/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include <dali/internal/input/windows/input-method-context-impl-win.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/key-event.h>
#include <windows.h>
#include <imm.h>

// WinUser.h defines CreateWindow as a macro (CreateWindowA/W). It collides with the
// WindowBase::CreateWindow() virtual method that is pulled in transitively by the DALi
// headers included below. Undef it here, mirroring window-base-win.cpp.
#ifdef CreateWindow
#undef CreateWindow
#endif

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/virtual-keyboard-impl.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/public-api/adaptor-framework/key.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_INPUT_METHOD_CONTEXT");
#endif

std::unordered_map<WinWindowHandle, InputMethodContextWin*> gActiveInputContexts;

class ScopedInputContext
{
public:
  explicit ScopedInputContext(WinWindowHandle window)
  : mWindow(reinterpret_cast<HWND>(window)),
    mContext(mWindow ? ImmGetContext(mWindow) : nullptr)
  {
  }

  ~ScopedInputContext()
  {
    if(mContext)
    {
      ImmReleaseContext(mWindow, mContext);
    }
  }

  HIMC Get() const
  {
    return mContext;
  }

  ScopedInputContext(const ScopedInputContext&)            = delete;
  ScopedInputContext& operator=(const ScopedInputContext&) = delete;

private:
  HWND mWindow;
  HIMC mContext;
};

std::string WideStringToUtf8(const std::wstring& text)
{
  if(text.empty())
  {
    return {};
  }

  const int required = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
  if(required <= 0)
  {
    return {};
  }

  std::string utf8(static_cast<size_t>(required), '\0');
  WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), utf8.data(), required, nullptr, nullptr);
  return utf8;
}

std::wstring Utf8StringToWide(const char* text)
{
  if(!text || !*text)
  {
    return {};
  }

  const int length   = static_cast<int>(strlen(text));
  const int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, length, nullptr, 0);
  if(required <= 0)
  {
    return {};
  }

  std::wstring wide(static_cast<size_t>(required), L'\0');
  MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, length, wide.data(), required);
  return wide;
}

std::wstring ReadCompositionString(HIMC context, DWORD index)
{
  const LONG byteLength = ImmGetCompositionStringW(context, index, nullptr, 0u);
  if(byteLength <= 0)
  {
    return {};
  }

  std::wstring value(static_cast<size_t>(byteLength) / sizeof(wchar_t), L'\0');
  const LONG   copied = ImmGetCompositionStringW(context, index, value.data(), static_cast<DWORD>(byteLength));
  if(copied <= 0)
  {
    return {};
  }
  value.resize(static_cast<size_t>(copied) / sizeof(wchar_t));
  return value;
}

std::vector<uint8_t> ReadCompositionAttributes(HIMC context)
{
  const LONG byteLength = ImmGetCompositionStringW(context, GCS_COMPATTR, nullptr, 0u);
  if(byteLength <= 0)
  {
    return {};
  }

  std::vector<uint8_t> attributes(static_cast<size_t>(byteLength));
  const LONG           copied = ImmGetCompositionStringW(context, GCS_COMPATTR, attributes.data(), static_cast<DWORD>(attributes.size()));
  if(copied <= 0)
  {
    return {};
  }
  attributes.resize(static_cast<size_t>(copied));
  return attributes;
}

uint32_t Utf16OffsetToCharacterIndex(const std::wstring& text, size_t offset)
{
  const size_t limit = (std::min)(offset, text.size());
  uint32_t     index = 0u;
  for(size_t position = 0u; position < limit; ++position, ++index)
  {
    const wchar_t value = text[position];
    if(value >= 0xd800 && value <= 0xdbff && position + 1u < limit)
    {
      const wchar_t next = text[position + 1u];
      if(next >= 0xdc00 && next <= 0xdfff)
      {
        ++position;
      }
    }
  }
  return index;
}

LCID GetKeyboardLocale()
{
  const auto layout = reinterpret_cast<ULONG_PTR>(GetKeyboardLayout(0));
  return MAKELCID(LOWORD(layout), SORT_DEFAULT);
}
} // namespace

InputMethodContextPtr InputMethodContextWin::New(Dali::Actor actor)
{
  InputMethodContextPtr manager;

  if(actor && Adaptor::IsAvailable())
  {
    manager = new InputMethodContextWin(actor);
  }

  return manager;
}

void InputMethodContextWin::ProcessWindowMessage(WinWindowHandle window, uint32_t message, uintptr_t wParam, intptr_t lParam)
{
  const auto active = gActiveInputContexts.find(window);
  if(active == gActiveInputContexts.end() || !active->second)
  {
    return;
  }

  InputMethodContextWin& inputContext = *active->second;
  switch(message)
  {
    case WM_IME_STARTCOMPOSITION:
    {
      inputContext.mIsComposing = true;
      break;
    }
    case WM_IME_COMPOSITION:
    {
      const HWND nativeWindow = reinterpret_cast<HWND>(window);
      const HIMC context      = ImmGetContext(nativeWindow);
      if(!context)
      {
        break;
      }

      const auto compositionFlags = static_cast<LPARAM>(lParam);
      if((compositionFlags & GCS_RESULTSTR) != 0)
      {
        inputContext.mIsComposing = false;
        inputContext.HandleCommit(WideStringToUtf8(ReadCompositionString(context, GCS_RESULTSTR)));
        inputContext.HandlePreEdit({}, {}, 0u);
      }
      if((compositionFlags & GCS_COMPSTR) != 0)
      {
        const std::wstring composition = ReadCompositionString(context, GCS_COMPSTR);
        const LONG         cursor      = ImmGetCompositionStringW(context, GCS_CURSORPOS, nullptr, 0u);
        inputContext.mIsComposing      = true;
        inputContext.HandlePreEdit(composition,
                                   ReadCompositionAttributes(context),
                                   Utf16OffsetToCharacterIndex(composition, cursor < 0 ? 0u : static_cast<size_t>(cursor)));
      }

      ImmReleaseContext(nativeWindow, context);
      break;
    }
    case WM_IME_ENDCOMPOSITION:
    {
      if(inputContext.mIsComposing)
      {
        inputContext.mIsComposing = false;
        inputContext.HandlePreEdit({}, {}, 0u);
      }
      break;
    }
    case WM_IME_NOTIFY:
    {
      if(wParam == IMN_SETOPENSTATUS)
      {
        const HWND nativeWindow = reinterpret_cast<HWND>(window);
        const HIMC context      = ImmGetContext(nativeWindow);
        if(context)
        {
          // TRUE/FALSE are #undef'd by the DALi EGL headers, so compare against the literal Win32 BOOL value.
          inputContext.HandleInputPanelState(ImmGetOpenStatus(context) != 0);
          ImmReleaseContext(nativeWindow, context);
        }
      }
      break;
    }
    case WM_INPUTLANGCHANGE:
    {
      if(Dali::Adaptor::IsAvailable())
      {
        inputContext.EmitLanguageChangedSignal(static_cast<int>(LOWORD(static_cast<ULONG_PTR>(lParam))));
      }
      break;
    }
    default:
      break;
  }
}

void InputMethodContextWin::Finalize()
{
  Deactivate();
  DisconnectCallbacks();
  DeleteContext();
}

InputMethodContextWin::InputMethodContextWin(Dali::Actor actor)
: mOptions(),
  mPreeditAttrs(),
  mSurroundingText(),
  mWin32Window(0),
  mIMFCursorPosition(0),
  mRestoreAfterFocusLost(false),
  mIdleCallbackConnected(false),
  mIsComposing(false)
{
  actor.SceneConnectedSignal().Connect(this, &InputMethodContextWin::OnStaged);
}

InputMethodContextWin::~InputMethodContextWin()
{
  Finalize();
}

void InputMethodContextWin::Initialize()
{
  CreateContext(mWin32Window);
  ConnectCallbacks();
}

void InputMethodContextWin::CreateContext(WinWindowHandle winHandle)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::CreateContext\n");
}

void InputMethodContextWin::DeleteContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::DeleteContext\n");
}

// Callbacks for predicitive text support.
void InputMethodContextWin::ConnectCallbacks()
{
}

void InputMethodContextWin::DisconnectCallbacks()
{
}

void InputMethodContextWin::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
  if(mWin32Window == 0u)
  {
    return;
  }

  auto existing = gActiveInputContexts.find(mWin32Window);
  if(existing != gActiveInputContexts.end() && existing->second && existing->second != this)
  {
    existing->second->Reset();
  }
  gActiveInputContexts[mWin32Window] = this;

  if(Dali::Adaptor::IsAvailable())
  {
    Dali::InputMethodContext handle(this);
    mActivatedSignal.Emit(handle);
  }
}

void InputMethodContextWin::Deactivate()
{
  mIdleCallbackConnected = false;
  const auto active      = gActiveInputContexts.find(mWin32Window);
  if(active != gActiveInputContexts.end() && active->second == this)
  {
    Reset();
    gActiveInputContexts.erase(active);
  }
}

void InputMethodContextWin::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::Reset\n");
  const bool clearPreEdit = mIsComposing;
  mIsComposing            = false;

  ScopedInputContext context(mWin32Window);
  if(context.Get())
  {
    ImmNotifyIME(context.Get(), NI_COMPOSITIONSTR, CPS_CANCEL, 0u);
  }
  if(clearPreEdit)
  {
    HandlePreEdit({}, {}, 0u);
  }
}

ImfContext* InputMethodContextWin::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetContext\n");

  return nullptr;
}

bool InputMethodContextWin::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

bool InputMethodContextWin::SetRestoreAfterFocusLost(bool toggle)
{
  mRestoreAfterFocusLost = toggle;
  return true;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextWin::PreEditChanged(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::PreEditChanged\n");
  HandlePreEdit(Utf8StringToWide(static_cast<const char*>(eventInfo)), {}, 0u);
}

void InputMethodContextWin::CommitReceived(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::CommitReceived\n");
  HandleCommit(eventInfo ? static_cast<const char*>(eventInfo) : "");
}

void InputMethodContextWin::HandlePreEdit(const std::wstring& text, const std::vector<uint8_t>& attributes, uint32_t cursorPosition)
{
  mPreeditAttrs.Clear();

  const size_t attributeCount = (std::min)(attributes.size(), text.size());
  if(attributeCount == 0u && !text.empty())
  {
    Dali::Integration::InputMethodContext::PreeditAttributeData data;
    data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::UNDERLINE;
    data.startIndex  = 0u;
    data.endIndex    = Utf16OffsetToCharacterIndex(text, text.size());
    mPreeditAttrs.PushBack(data);
  }
  else
  {
    size_t runStart = 0u;
    while(runStart < attributeCount)
    {
      size_t runEnd = runStart + 1u;
      while(runEnd < attributeCount && attributes[runEnd] == attributes[runStart])
      {
        ++runEnd;
      }

      Dali::Integration::InputMethodContext::PreeditAttributeData data;
      switch(attributes[runStart])
      {
        case ATTR_TARGET_CONVERTED:
        case ATTR_TARGET_NOTCONVERTED:
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::HIGHLIGHT;
          break;
        case ATTR_INPUT_ERROR:
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::REVERSE;
          break;
        default:
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::UNDERLINE;
          break;
      }
      data.startIndex = Utf16OffsetToCharacterIndex(text, runStart);
      data.endIndex   = Utf16OffsetToCharacterIndex(text, runEnd);
      mPreeditAttrs.PushBack(data);
      runStart = runEnd;
    }
  }

  if(Dali::Adaptor::IsAvailable())
  {
    Dali::InputMethodContext                         handle(this);
    Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::PRE_EDIT, Dali::String(WideStringToUtf8(text).c_str()), static_cast<int32_t>(cursorPosition), 0);
    mEventSignal.Emit(handle, eventData);
    Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

    if(callbackData.update)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
      mSurroundingText   = callbackData.currentText;
      NotifyCursorPosition();
    }

    if(callbackData.preeditResetRequired)
    {
      Reset();
    }
  }
}

void InputMethodContextWin::HandleCommit(const std::string& text)
{
  if(text.empty() || !Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::COMMIT, Dali::String(text.c_str()), 0, 0);
  mEventSignal.Emit(handle, eventData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

  if(callbackData.update)
  {
    mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
    mSurroundingText   = callbackData.currentText;
    NotifyCursorPosition();
  }
}

void InputMethodContextWin::HandleInputPanelState(bool shown)
{
  if(Dali::Adaptor::IsAvailable())
  {
    EmitStatusChangedSignal(shown ? Dali::InputMethodContext::State::SHOW : Dali::InputMethodContext::State::HIDE);
  }
}

/**
 * Called when an InputMethodContext retrieve surround event is received.
 * Here the InputMethodContext module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
bool InputMethodContextWin::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::RetrieveSurrounding\n");

  Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::GET_SURROUNDING, Dali::String(), 0, 0);
  Dali::InputMethodContext                         handle(this);
  mEventSignal.Emit(handle, imfData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

  if(callbackData.update)
  {
    mSurroundingText = callbackData.currentText;
    if(text)
    {
      *text = strdup(callbackData.currentText.CStr());
    }

    if(cursorPosition)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
      *cursorPosition    = mIMFCursorPosition;
    }
  }

  return true;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextWin::DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::DeleteSurrounding\n");
}

void InputMethodContextWin::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::NotifyCursorPosition\n");
}

void InputMethodContextWin::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetCursorPosition\n");

  mIMFCursorPosition = static_cast<int>(cursorPosition);
}

unsigned int InputMethodContextWin::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetCursorPosition\n");

  return static_cast<unsigned int>(mIMFCursorPosition);
}

void InputMethodContextWin::SetSurroundingText(const Dali::String& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetSurroundingText\n");

  mSurroundingText = text;
}

Dali::String InputMethodContextWin::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetSurroundingText\n");

  return mSurroundingText;
}

void InputMethodContextWin::NotifyTextInputMultiLine(bool multiLine)
{
}

Dali::Integration::InputMethodContext::TextDirection InputMethodContextWin::GetTextDirection()
{
  wchar_t readingLayout[4]{};
  if(GetLocaleInfoW(GetKeyboardLocale(), LOCALE_IREADINGLAYOUT, readingLayout, static_cast<int>(std::size(readingLayout))) > 0 &&
     readingLayout[0] == L'1')
  {
    return Dali::Integration::InputMethodContext::RIGHT_TO_LEFT;
  }
  return Dali::Integration::InputMethodContext::LEFT_TO_RIGHT;
}

BoundsInteger InputMethodContextWin::GetInputPanelArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  return BoundsInteger(xPos, yPos, width, height);
}

void InputMethodContextWin::ApplyOptions(const Dali::Integration::InputMethodOptions& options)
{
  using namespace Dali::Integration::InputMethod::Category;

  int index;

  if(mOptions.CompareAndSet(PANEL_LAYOUT, options, index))
  {
  }
  if(mOptions.CompareAndSet(BUTTON_ACTION, options, index))
  {
  }
  if(mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index))
  {
  }
  if(mOptions.CompareAndSet(VARIATION, options, index))
  {
  }
}

bool InputMethodContextWin::SetInputPanelUserData(const Dali::String& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelUserData\n");
  return true;
}

Dali::String InputMethodContextWin::GetInputPanelUserData() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelUserData\n");

  return Dali::String();
}

Dali::InputMethodContext::State InputMethodContextWin::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelState\n");
  ScopedInputContext context(mWin32Window);
  if(context.Get() && ImmGetOpenStatus(context.Get()))
  {
    return Dali::InputMethodContext::State::SHOW;
  }
  return Dali::InputMethodContext::State::HIDE;
}

bool InputMethodContextWin::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetReturnKeyState\n");
  return true;
}

bool InputMethodContextWin::IsReturnKeyEnabled() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::IsReturnKeyEnabled\n");
  return true;
}

bool InputMethodContextWin::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::AutoEnableInputPanel\n");
  return true;
}

bool InputMethodContextWin::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::ShowInputPanel\n");
  ScopedInputContext context(mWin32Window);
  // TRUE (1) is used as the literal Win32 BOOL value; the TRUE macro is #undef'd by the DALi EGL headers.
  return context.Get() && ImmSetOpenStatus(context.Get(), 1);
}

bool InputMethodContextWin::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::HideInputPanel\n");
  ScopedInputContext context(mWin32Window);
  // FALSE (0) is used as the literal Win32 BOOL value; the FALSE macro is #undef'd by the DALi EGL headers.
  return context.Get() && ImmSetOpenStatus(context.Get(), 0);
}

Dali::InputMethodContext::KeyboardType InputMethodContextWin::GetKeyboardType()
{
  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

bool InputMethodContextWin::SetInputPanelLanguageLocale(const Dali::String& locale)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelLanguageLocale\n");
  return false;
}

Dali::String InputMethodContextWin::GetInputPanelLanguageLocale() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelLanguageLocale\n");

  wchar_t localeName[LOCALE_NAME_MAX_LENGTH]{};
  if(LCIDToLocaleName(GetKeyboardLocale(), localeName, static_cast<int>(std::size(localeName)), 0u) == 0)
  {
    return Dali::String();
  }
  return Dali::String(WideStringToUtf8(localeName).c_str());
}

void InputMethodContextWin::SetContentMimeTypes(const Dali::String& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetContentMimeTypes\n");
}

bool InputMethodContextWin::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);

  if(!KeyLookup::IsDeviceButton(keyEvent.GetKeyName().CStr()))
  {
    //check whether it's key down or key up event
    if(keyEvent.GetState() == Dali::KeyEvent::DOWN)
    {
      eventHandled = ProcessEventKeyDown(keyEvent);
    }
    else if(keyEvent.GetState() == Dali::KeyEvent::UP)
    {
      eventHandled = ProcessEventKeyUp(keyEvent);
    }
  }

  return eventHandled;
}

bool InputMethodContextWin::SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelLanguage\n");
  return true;
}

Dali::Integration::InputMethodContext::InputPanelLanguage InputMethodContextWin::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelLanguage\n");
  return Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

bool InputMethodContextWin::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelPosition\n");
  return true;
}

bool InputMethodContextWin::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelPositionAlign\n");
  return false;
}

void InputMethodContextWin::GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetPreeditStyle\n");
  attrs = mPreeditAttrs;
}

bool InputMethodContextWin::ProcessEventKeyDown(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  return eventHandled;
}

bool InputMethodContextWin::ProcessEventKeyUp(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  return eventHandled;
}

void InputMethodContextWin::OnStaged(Dali::Actor actor)
{
  WinWindowHandle winWindow(AnyCast<WinWindowHandle>(Dali::Integration::SceneHolder::Get(actor).GetNativeHandle()));

  if(mWin32Window != winWindow)
  {
    Finalize();
    mWin32Window = winWindow;
    Initialize();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
