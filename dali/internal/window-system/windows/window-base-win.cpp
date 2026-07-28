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
#include <dali/internal/window-system/windows/window-base-win.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>
#include <dali/integration-api/string-utils.h>
#include <dali/public-api/object/any.h>
#include <windows.h>
#include <windowsx.h>
#include <atomic>
#include <string>

// WinUser.h compatibility aliases collide with WindowBase virtual methods.
#ifdef IsMaximized
#undef IsMaximized
#endif
#ifdef IsMinimized
#undef IsMinimized
#endif

// INTERNAL HEADERS
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/internal/input/windows/input-method-context-impl-win.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

using Dali::Integration::ToDaliString;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const Device::Class::Type    KEYBOARD_DEVICE_CLASS   = Device::Class::KEYBOARD;
const Device::Subclass::Type DEFAULT_DEVICE_SUBCLASS = Device::Subclass::NONE;

constexpr uint32_t SHIFT_MODIFIER  = 0x001u;
constexpr uint32_t CTRL_MODIFIER   = 0x002u;
constexpr uint32_t ALT_MODIFIER    = 0x004u;
constexpr uint32_t WIN_MODIFIER    = 0x008u;
constexpr uint32_t SCROLL_MODIFIER = 0x010u;
constexpr uint32_t NUM_MODIFIER    = 0x020u;
constexpr uint32_t CAPS_MODIFIER   = 0x040u;
constexpr uint32_t ALTGR_MODIFIER  = 0x400u;

std::atomic<int> gNextNativeWindowId{1};

int GetNextNativeWindowId()
{
  int id = gNextNativeWindowId.fetch_add(1, std::memory_order_relaxed);
  if(id <= 0)
  {
    gNextNativeWindowId.store(2, std::memory_order_relaxed);
    id = 1;
  }
  return id;
}

uint32_t GetKeyModifiers(uintptr_t wParam = 0u)
{
  const auto keyState = static_cast<WPARAM>(wParam);
  uint32_t   modifiers{0u};
  if((keyState & MK_SHIFT) != 0 || (GetKeyState(VK_SHIFT) & 0x8000) != 0)
  {
    modifiers |= SHIFT_MODIFIER;
  }
  if((keyState & MK_CONTROL) != 0 || (GetKeyState(VK_CONTROL) & 0x8000) != 0)
  {
    modifiers |= CTRL_MODIFIER;
  }
  if((GetKeyState(VK_MENU) & 0x8000) != 0)
  {
    modifiers |= ALT_MODIFIER;
  }
  if((GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0)
  {
    modifiers |= WIN_MODIFIER;
  }
  if((GetKeyState(VK_SCROLL) & 0x0001) != 0)
  {
    modifiers |= SCROLL_MODIFIER;
  }
  if((GetKeyState(VK_NUMLOCK) & 0x0001) != 0)
  {
    modifiers |= NUM_MODIFIER;
  }
  if((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
  {
    modifiers |= CAPS_MODIFIER;
  }
  if((GetKeyState(VK_RMENU) & 0x8000) != 0 && (GetKeyState(VK_CONTROL) & 0x8000) != 0)
  {
    modifiers |= ALTGR_MODIFIER;
  }
  return modifiers;
}

MouseButton::Type GetMouseButton(uint32_t message, uintptr_t buttonState)
{
  if(message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || (buttonState & MK_LBUTTON) != 0u)
  {
    return MouseButton::PRIMARY;
  }
  if(message == WM_MBUTTONDOWN || message == WM_MBUTTONUP || (buttonState & MK_MBUTTON) != 0u)
  {
    return MouseButton::TERTIARY;
  }
  if(message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || (buttonState & MK_RBUTTON) != 0u)
  {
    return MouseButton::SECONDARY;
  }
  return MouseButton::INVALID;
}

std::string WideStringToUtf8(const wchar_t* text, int length)
{
  if(!text || length <= 0)
  {
    return {};
  }

  const int required = WideCharToMultiByte(CP_UTF8, 0, text, length, nullptr, 0, nullptr, nullptr);
  if(required <= 0)
  {
    return {};
  }

  std::string utf8(static_cast<size_t>(required), '\0');
  WideCharToMultiByte(CP_UTF8, 0, text, length, utf8.data(), required, nullptr, nullptr);
  return utf8;
}

std::string GetKeyString(uintptr_t virtualKey, intptr_t lParam)
{
  BYTE keyboardState[256]{};
  if(!GetKeyboardState(keyboardState))
  {
    return {};
  }

  wchar_t    text[8]{};
  const UINT scanCode = static_cast<UINT>((static_cast<uintptr_t>(lParam) >> 16u) & 0xffu);
  // Bit 2 keeps ToUnicodeEx from changing the keyboard buffer, which avoids
  // consuming dead-key state while DALi inspects the translated text.
  const int length = ToUnicodeEx(static_cast<UINT>(virtualKey), scanCode, keyboardState, text, static_cast<int>(sizeof(text) / sizeof(text[0])), 0x04u, GetKeyboardLayout(0));
  return length > 0 ? WideStringToUtf8(text, length) : std::string{};
}

std::string GetLogicalKey(const std::string& keyName, const std::string& keyString)
{
  if(keyString.size() != 1u)
  {
    return keyString.empty() ? keyName : keyString;
  }

  const auto character = static_cast<unsigned char>(keyString[0]);
  if(character < 0x20u || character == 0x7fu)
  {
    return keyName;
  }

  switch(keyString[0])
  {
    case ' ':
      return "space";
    case '!':
      return "exclam";
    case '"':
      return "quotedbl";
    case '#':
      return "numbersign";
    case '$':
      return "dollar";
    case '%':
      return "percent";
    case '&':
      return "ampersand";
    case '\'':
      return "apostrophe";
    case '(':
      return "parenleft";
    case ')':
      return "parenright";
    case '*':
      return "asterisk";
    case '+':
      return "plus";
    case ',':
      return "comma";
    case '-':
      return "minus";
    case '.':
      return "period";
    case '/':
      return "slash";
    case ':':
      return "colon";
    case ';':
      return "semicolon";
    case '<':
      return "less";
    case '=':
      return "equal";
    case '>':
      return "greater";
    case '?':
      return "question";
    case '@':
      return "at";
    case '[':
      return "bracketleft";
    case '\\':
      return "backslash";
    case ']':
      return "bracketright";
    case '^':
      return "asciicircum";
    case '_':
      return "underscore";
    case '`':
      return "grave";
    case '{':
      return "braceleft";
    case '|':
      return "bar";
    case '}':
      return "braceright";
    case '~':
      return "asciitilde";
    default:
      return keyString;
  }
}

std::wstring Utf8ToWideString(const std::string& text)
{
  if(text.empty())
  {
    return {};
  }

  const int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);
  if(required <= 0)
  {
    return {};
  }

  std::wstring wide(static_cast<size_t>(required), L'\0');
  MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), wide.data(), required);
  return wide;
}

bool GetNativePositionSize(WinWindowHandle handle, PositionSize& positionSize)
{
  const HWND window = reinterpret_cast<HWND>(handle);
  RECT       windowRect{};
  RECT       clientRect{};
  if(!window || !GetWindowRect(window, &windowRect) || !GetClientRect(window, &clientRect))
  {
    return false;
  }

  positionSize.x      = windowRect.left;
  positionSize.y      = windowRect.top;
  positionSize.width  = clientRect.right - clientRect.left;
  positionSize.height = clientRect.bottom - clientRect.top;
  return true;
}

void SetNativePositionSize(WinWindowHandle handle, const PositionSize& positionSize, bool move, bool resize)
{
  const HWND window = reinterpret_cast<HWND>(handle);
  if(!window)
  {
    return;
  }

  int width  = positionSize.width;
  int height = positionSize.height;
  if(resize)
  {
    const DWORD style   = static_cast<DWORD>(GetWindowLongPtr(window, GWL_STYLE));
    const DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(window, GWL_EXSTYLE));
    RECT        frame   = {0, 0, (std::max)(width, 1), (std::max)(height, 1)};
    if(AdjustWindowRectEx(&frame, style, GetMenu(window) != nullptr, exStyle))
    {
      width  = frame.right - frame.left;
      height = frame.bottom - frame.top;
    }
  }

  UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
  if(!move)
  {
    flags |= SWP_NOMOVE;
  }
  if(!resize)
  {
    flags |= SWP_NOSIZE;
  }

  if(!SetWindowPos(window, nullptr, positionSize.x, positionSize.y, width, height, flags))
  {
    DALI_LOG_ERROR("Failed to update DALi window geometry, error %lu\n", static_cast<unsigned long>(GetLastError()));
  }
}

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW_BASE");
#endif

} // unnamed namespace

WindowBaseWin::WindowBaseWin(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mWin32Window(0),
  mNativeWindowId(GetNextNativeWindowId()),
  mVerticalWheelRemainder(0),
  mHorizontalWheelRemainder(0),
  mOwnSurface(false),
  mIsTransparent(false),
  mIsIconifiedState(false),
  mIsMaximizedState(false)
{
  Initialize(positionSize, surface, isTransparent);
}

WindowBaseWin::~WindowBaseWin()
{
  const WinWindowHandle nativeWindow = mWin32Window;

  // Stop callbacks before the C++ object starts disappearing.  External
  // windows are only detached; DALi-owned windows are destroyed synchronously.
  mWindowImpl.SetListener(nullptr);
  mWindowImpl.DetachWindow();
  if(mOwnSurface && nativeWindow != 0)
  {
    WindowsPlatform::WindowImpl::DestroyHWnd(nativeWindow);
  }
  mWin32Window = 0;
}

void WindowBaseWin::Initialize(PositionSize positionSize, Any surface, bool isTransparent)
{
  // see if there is a surface in Any surface
  uintptr_t surfaceId = GetSurfaceId(surface);

  // if the surface is empty, create a new one.
  if(surfaceId == 0)
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateWinWindow(positionSize, isTransparent);
  }
  else
  {
    SetWinWindow(surfaceId);
  }

  mIsIconifiedState = IsIconic(reinterpret_cast<HWND>(mWin32Window)) != 0;
  mIsMaximizedState = IsZoomed(reinterpret_cast<HWND>(mWin32Window)) != 0;
  mWindowImpl.SetListener(MakeCallback(this, &WindowBaseWin::EventEntry));
}

void WindowBaseWin::OnDeleteRequest()
{
  if(Dali::Adaptor::IsAvailable())
  {
    mDeleteRequestSignal.Emit();
  }
}

void WindowBaseWin::OnFocusIn(int, TWinEventInfo* event)
{
  if(event->mWindow == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    mFocusChangedSignal.Emit(true);
  }
}

void WindowBaseWin::OnFocusOut(int, TWinEventInfo* event)
{
  if(event->mWindow == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    mFocusChangedSignal.Emit(false);
  }
}

void WindowBaseWin::OnWindowDamaged(int, TWinEventInfo* event)
{
  if(event->mWindow == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    RECT updateRect{};
    HWND window = reinterpret_cast<HWND>(mWin32Window);
    if(!GetUpdateRect(window, &updateRect, 0))
    {
      GetClientRect(window, &updateRect);
    }

    DamageArea area;
    area.x      = updateRect.left;
    area.y      = updateRect.top;
    area.width  = updateRect.right - updateRect.left;
    area.height = updateRect.bottom - updateRect.top;

    mWindowDamagedSignal.Emit(area);
  }
}

void WindowBaseWin::OnWindowPositionSizeChanged(TWinEventInfo* event)
{
  if(event->mWindow == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    PositionSize positionSize;
    if(GetNativePositionSize(mWin32Window, positionSize))
    {
      mUpdatePositionSizeSignal.Emit(positionSize);
    }
  }
}

void WindowBaseWin::OnMouseButtonDown(int type, TWinEventInfo* event)
{
  Event_Mouse_Button touchEvent = {};
  touchEvent.window             = event->mWindow;
  touchEvent.timestamp          = event->timestamp;
  touchEvent.x                  = GET_X_LPARAM(static_cast<LPARAM>(event->lParam));
  touchEvent.y                  = GET_Y_LPARAM(static_cast<LPARAM>(event->lParam));
  touchEvent.multi.device       = DEVICE_MOUSE;
  touchEvent.multi.pressure     = 1.0;

  if(touchEvent.window == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    SetCapture(reinterpret_cast<HWND>(mWin32Window));

    Integration::Point point;
    point.SetDeviceId(touchEvent.multi.device);
    point.SetState(PointState::DOWN);
    point.SetDeviceClass(Device::Class::MOUSE);
    point.SetScreenPosition(Vector2(static_cast<float>(touchEvent.x), static_cast<float>(touchEvent.y)));
    point.SetRadius(touchEvent.multi.radius, Vector2(touchEvent.multi.radius_x, touchEvent.multi.radius_y));
    point.SetPressure(touchEvent.multi.pressure);
    point.SetAngle(Degree(touchEvent.multi.angle));
    point.SetMouseButton(GetMouseButton(static_cast<uint32_t>(type), event->wParam));

    mTouchEventSignal.Emit(point, touchEvent.timestamp);

    mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseWin::OnMouseButtonUp(int type, TWinEventInfo* event)
{
  Event_Mouse_Button touchEvent = {};
  touchEvent.window             = event->mWindow;
  touchEvent.timestamp          = event->timestamp;
  touchEvent.x                  = GET_X_LPARAM(static_cast<LPARAM>(event->lParam));
  touchEvent.y                  = GET_Y_LPARAM(static_cast<LPARAM>(event->lParam));
  touchEvent.multi.device       = DEVICE_MOUSE;
  touchEvent.multi.pressure     = 1.0;

  if(touchEvent.window == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    if((event->wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0u && GetCapture() == reinterpret_cast<HWND>(mWin32Window))
    {
      ReleaseCapture();
    }

    Integration::Point point;
    point.SetDeviceId(touchEvent.multi.device);
    point.SetState(PointState::UP);
    point.SetDeviceClass(Device::Class::MOUSE);
    point.SetScreenPosition(Vector2(static_cast<float>(touchEvent.x), static_cast<float>(touchEvent.y)));
    point.SetRadius(touchEvent.multi.radius, Vector2(touchEvent.multi.radius_x, touchEvent.multi.radius_y));
    point.SetPressure(touchEvent.multi.pressure);
    point.SetAngle(Degree(touchEvent.multi.angle));
    point.SetMouseButton(GetMouseButton(static_cast<uint32_t>(type), event->wParam));

    mTouchEventSignal.Emit(point, touchEvent.timestamp);

    mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseWin::OnMouseButtonMove(int, TWinEventInfo* event)
{
  Event_Mouse_Button touchEvent = {};
  touchEvent.window             = event->mWindow;
  touchEvent.timestamp          = event->timestamp;
  touchEvent.x                  = GET_X_LPARAM(static_cast<LPARAM>(event->lParam));
  touchEvent.y                  = GET_Y_LPARAM(static_cast<LPARAM>(event->lParam));
  touchEvent.multi.device       = DEVICE_MOUSE;
  touchEvent.multi.pressure     = (event->wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) != 0u ? 1.0 : 0.0;

  if(touchEvent.window == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    Integration::Point point;
    point.SetDeviceId(touchEvent.multi.device);
    point.SetState(PointState::MOTION);
    point.SetDeviceClass(Device::Class::MOUSE);
    point.SetScreenPosition(Vector2(static_cast<float>(touchEvent.x), static_cast<float>(touchEvent.y)));
    point.SetRadius(touchEvent.multi.radius, Vector2(touchEvent.multi.radius_x, touchEvent.multi.radius_y));
    point.SetPressure(touchEvent.multi.pressure);
    point.SetAngle(Degree(touchEvent.multi.angle));
    const MouseButton::Type mouseButton = GetMouseButton(WM_MOUSEMOVE, event->wParam);
    if(mouseButton != MouseButton::INVALID)
    {
      point.SetMouseButton(mouseButton);
    }

    mTouchEventSignal.Emit(point, touchEvent.timestamp);

    mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseWin::OnMouseWheel(int type, TWinEventInfo* event)
{
  Event_Mouse_Wheel mouseWheelEvent = {};
  mouseWheelEvent.window            = event->mWindow;
  mouseWheelEvent.timestamp         = event->timestamp;
  mouseWheelEvent.modifiers         = GetKeyModifiers(GET_KEYSTATE_WPARAM(static_cast<WPARAM>(event->wParam)));
  mouseWheelEvent.direction         = type == WM_MOUSEHWHEEL ? 1 : 0;

  POINT point{GET_X_LPARAM(static_cast<LPARAM>(event->lParam)), GET_Y_LPARAM(static_cast<LPARAM>(event->lParam))};
  ScreenToClient(reinterpret_cast<HWND>(mWin32Window), &point);
  mouseWheelEvent.x = point.x;
  mouseWheelEvent.y = point.y;

  int32_t&      remainder   = type == WM_MOUSEHWHEEL ? mHorizontalWheelRemainder : mVerticalWheelRemainder;
  const int32_t nativeDelta = GET_WHEEL_DELTA_WPARAM(static_cast<WPARAM>(event->wParam));
  // Win32 vertical positive means up, while DALi/X11 represents up as -1.
  // Horizontal positive already means right on both backends.
  remainder += type == WM_MOUSEHWHEEL ? nativeDelta : -nativeDelta;
  mouseWheelEvent.z = remainder / WHEEL_DELTA;
  remainder %= WHEEL_DELTA;

  if(mouseWheelEvent.window == mWin32Window && mouseWheelEvent.z != 0 && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent.direction, mouseWheelEvent.modifiers, mouseWheelEvent.x, mouseWheelEvent.y, mouseWheelEvent.z);

    Integration::WheelEvent wheelEvent(Integration::WheelEvent::MOUSE_WHEEL, mouseWheelEvent.direction, mouseWheelEvent.modifiers, Vector2(mouseWheelEvent.x, mouseWheelEvent.y), mouseWheelEvent.z, mouseWheelEvent.timestamp);

    mWheelEventSignal.Emit(wheelEvent);
  }
}

void WindowBaseWin::OnKeyDown(int, TWinEventInfo* event)
{
  if(event->mWindow == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnKeyDown\n");

    const int         keyCode       = static_cast<int>(event->wParam);
    const std::string utf8KeyName   = WindowsPlatform::GetKeyName(keyCode, event->lParam);
    const std::string utf8KeyString = GetKeyString(event->wParam, event->lParam);
    const String      keyName(ToDaliString(utf8KeyName));
    const String      logicalKey(ToDaliString(GetLogicalKey(utf8KeyName, utf8KeyString)));
    const String      keyString(ToDaliString(utf8KeyString));
    const String      compose; ///< Raw key events carry no compose string; IME composition arrives via the WM_IME_* path.
    const String      deviceName(ToDaliString(std::string("keyboard")));
    const int         modifier = static_cast<int>(GetKeyModifiers());
    const auto        time     = static_cast<unsigned long>(event->timestamp);

    Integration::KeyEvent keyEvent(keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::DOWN, compose, deviceName, KEYBOARD_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS);
    keyEvent.isRepeat    = (static_cast<uintptr_t>(event->lParam) & (1u << 30u)) != 0u;
    keyEvent.windowId    = GetNativeWindowId();
    keyEvent.receiveTime = GetTickCount();

    mKeyEventSignal.Emit(keyEvent);
  }
}

void WindowBaseWin::OnKeyUp(int, TWinEventInfo* event)
{
  if(event->mWindow == mWin32Window && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnKeyUp\n");

    const int         keyCode       = static_cast<int>(event->wParam);
    const std::string utf8KeyName   = WindowsPlatform::GetKeyName(keyCode, event->lParam);
    const std::string utf8KeyString = GetKeyString(event->wParam, event->lParam);
    const String      keyName(ToDaliString(utf8KeyName));
    const String      logicalKey(ToDaliString(GetLogicalKey(utf8KeyName, utf8KeyString)));
    const String      keyString(ToDaliString(utf8KeyString));
    const String      compose; ///< Raw key events carry no compose string; IME composition arrives via the WM_IME_* path.
    const String      deviceName(ToDaliString(std::string("keyboard")));
    const int         modifier = static_cast<int>(GetKeyModifiers());
    const auto        time     = static_cast<unsigned long>(event->timestamp);

    // Match the Linux backends: the released key still carries its translated string
    // (keyEvent->string), so populate keyString on UP just like DOWN.
    Integration::KeyEvent keyEvent(keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::UP, compose, deviceName, KEYBOARD_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS);
    keyEvent.windowId    = GetNativeWindowId();
    keyEvent.receiveTime = GetTickCount();

    mKeyEventSignal.Emit(keyEvent);
  }
}

Any WindowBaseWin::GetNativeWindow()
{
  return mWin32Window;
}

void* WindowBaseWin::GetNativeRawHandle()
{
  return reinterpret_cast<void*>(mWin32Window);
}

int WindowBaseWin::GetNativeWindowId()
{
  return mNativeWindowId;
}

std::string WindowBaseWin::GetNativeWindowResourceId()
{
  return std::string();
}

Dali::Any WindowBaseWin::CreateWindow(int width, int height)
{
  return reinterpret_cast<void*>(mWin32Window);
}

void WindowBaseWin::DestroyWindow()
{
  // This method releases graphics-window wrapper resources on backends that
  // have one.  The HWND itself is owned by WindowBaseWin and is released in
  // the destructor so surface replacement does not destroy the native window.
}

void WindowBaseWin::SetWindowRotation(int angle)
{
}

void WindowBaseWin::SetWindowBufferTransform(int angle)
{
}

void WindowBaseWin::SetWindowTransform(int angle)
{
}

void WindowBaseWin::ResizeWindow(PositionSize positionSize)
{
  // ANGLE renders directly to the HWND; there is no separate native wrapper
  // to resize here.  Resize() updates the HWND client area.
}

bool WindowBaseWin::IsWindowRotationSupported()
{
  return false;
}

void WindowBaseWin::Move(PositionSize positionSize)
{
  SetNativePositionSize(mWin32Window, positionSize, true, false);
}

void WindowBaseWin::Resize(PositionSize positionSize)
{
  SetNativePositionSize(mWin32Window, positionSize, false, true);
}

void WindowBaseWin::MoveResize(PositionSize positionSize)
{
  SetNativePositionSize(mWin32Window, positionSize, true, true);
}

void WindowBaseWin::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
}

void WindowBaseWin::SetClass(const std::string& name, const std::string& className)
{
  const std::wstring title = Utf8ToWideString(name.empty() ? className : name);
  if(!SetWindowTextW(reinterpret_cast<HWND>(mWin32Window), title.c_str()))
  {
    DALI_LOG_ERROR("Failed to set DALi window title, error %lu\n", static_cast<unsigned long>(GetLastError()));
  }
}

void WindowBaseWin::Raise()
{
  SetWindowPos(reinterpret_cast<HWND>(mWin32Window), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void WindowBaseWin::Lower()
{
  SetWindowPos(reinterpret_cast<HWND>(mWin32Window), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void WindowBaseWin::Activate()
{
  HWND window = reinterpret_cast<HWND>(mWin32Window);
  ShowWindow(window, SW_SHOW);
  SetForegroundWindow(window);
  SetFocus(window);
}

void WindowBaseWin::Maximize(bool maximize)
{
  HWND window = reinterpret_cast<HWND>(mWin32Window);
  if(maximize)
  {
    ShowWindow(window, SW_MAXIMIZE);
  }
  else if(IsZoomed(window))
  {
    ShowWindow(window, SW_RESTORE);
  }
}

bool WindowBaseWin::IsMaximized() const
{
  return IsZoomed(reinterpret_cast<HWND>(mWin32Window)) != 0;
}

void WindowBaseWin::SetMaximumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseWin::Minimize(bool minimize)
{
  HWND window = reinterpret_cast<HWND>(mWin32Window);
  if(minimize)
  {
    ShowWindow(window, SW_MINIMIZE);
  }
  else if(IsIconic(window))
  {
    ShowWindow(window, SW_RESTORE);
  }
}

bool WindowBaseWin::IsMinimized() const
{
  return IsIconic(reinterpret_cast<HWND>(mWin32Window)) != 0;
}

void WindowBaseWin::SetMinimumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseWin::MaximizeWithRestoreSize(bool maximize, Dali::Window::WindowSize size)
{
  Maximize(maximize);
  if(!maximize)
  {
    PositionSize positionSize;
    if(GetNativePositionSize(mWin32Window, positionSize))
    {
      positionSize.width  = static_cast<int>(size.GetWidth());
      positionSize.height = static_cast<int>(size.GetHeight());
      Resize(positionSize);
    }
  }
}

void WindowBaseWin::SetAvailableAnlges(const std::vector<int>& angles)
{
}

void WindowBaseWin::SetPreferredAngle(int angle)
{
}

void WindowBaseWin::SetAcceptFocus(bool accept)
{
  const HWND window = reinterpret_cast<HWND>(mWin32Window);
  if(!window)
  {
    return;
  }

  LONG_PTR style = GetWindowLongPtr(window, GWL_EXSTYLE);
  style          = accept ? (style & ~static_cast<LONG_PTR>(WS_EX_NOACTIVATE)) : (style | WS_EX_NOACTIVATE);
  SetLastError(ERROR_SUCCESS);
  if(SetWindowLongPtr(window, GWL_EXSTYLE, style) == 0 && GetLastError() != ERROR_SUCCESS)
  {
    DALI_LOG_ERROR("Failed to update DALi window focus policy, error %lu\n", static_cast<unsigned long>(GetLastError()));
  }
}

void WindowBaseWin::Show()
{
  ShowWindow(reinterpret_cast<HWND>(mWin32Window), SW_SHOW);
}

void WindowBaseWin::Hide()
{
  ShowWindow(reinterpret_cast<HWND>(mWin32Window), SW_HIDE);
}

unsigned int WindowBaseWin::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseWin::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return std::string();
}

unsigned int WindowBaseWin::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  return 0;
}

bool WindowBaseWin::RemoveAuxiliaryHint(unsigned int id)
{
  return false;
}

bool WindowBaseWin::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  return false;
}

std::string WindowBaseWin::GetAuxiliaryHintValue(unsigned int id) const
{
  return std::string();
}

unsigned int WindowBaseWin::GetAuxiliaryHintId(const std::string& hint) const
{
  return 0;
}

void WindowBaseWin::SetInputRegion(const BoundsInteger& inputRegion)
{
}

void WindowBaseWin::SetType(Dali::WindowType type)
{
}

Dali::WindowType WindowBaseWin::GetType() const
{
  return Dali::WindowType::NORMAL;
}

Dali::WindowOperationResult WindowBaseWin::SetNotificationLevel(Dali::WindowNotificationLevel level)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

Dali::WindowNotificationLevel WindowBaseWin::GetNotificationLevel() const
{
  return Dali::WindowNotificationLevel::NONE;
}

void WindowBaseWin::SetOpaqueState(bool opaque)
{
}

Dali::WindowOperationResult WindowBaseWin::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

WindowScreenOffMode WindowBaseWin::GetScreenOffMode() const
{
  return WindowScreenOffMode::TIMEOUT;
}

Dali::WindowOperationResult WindowBaseWin::SetBrightness(int brightness)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

int WindowBaseWin::GetBrightness() const
{
  return 0;
}

bool WindowBaseWin::GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode)
{
  return false;
}

bool WindowBaseWin::UngrabKey(Dali::KEY key)
{
  return false;
}

bool WindowBaseWin::GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result)
{
  return false;
}

bool WindowBaseWin::UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result)
{
  return false;
}

void WindowBaseWin::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  float xres, yres;
  mWindowImpl.GetDPI(xres, yres);

  dpiHorizontal = static_cast<unsigned int>(xres + 0.5f);
  dpiVertical   = static_cast<unsigned int>(yres + 0.5f);
}

int WindowBaseWin::GetScreenRotationAngle(const bool update)
{
  return 0;
}

void WindowBaseWin::SetWindowRotationAngle(int degree)
{
}

void WindowBaseWin::WindowRotationCompleted(int degree, int width, int height)
{
}

void WindowBaseWin::SetTransparency(bool transparent)
{
  mIsTransparent = transparent;
}

int WindowBaseWin::GetWindowRotationAngle() const
{
  return 0;
}

uintptr_t WindowBaseWin::GetSurfaceId(Any surface) const
{
  uintptr_t surfaceId = 0;

  if(surface.Empty() == false)
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS((surface.IsType<WinWindowHandle>()) && "Surface type is invalid");

    surfaceId = AnyCast<WinWindowHandle>(surface);
  }
  return surfaceId;
}

void WindowBaseWin::CreateWinWindow(PositionSize positionSize, bool isTransparent)
{
  mWin32Window   = WindowsPlatform::WindowImpl::CreateHwnd("DALi", positionSize.x, positionSize.y, positionSize.width, positionSize.height, 0);
  mIsTransparent = isTransparent;

  DALI_ASSERT_ALWAYS(mWin32Window != 0 && "There is no Windows window");
  mWindowImpl.SetHWND(mWin32Window);
}

void WindowBaseWin::SetWinWindow(uintptr_t surfaceId)
{
  mWin32Window = static_cast<WinWindowHandle>(surfaceId);

  mWindowImpl.SetHWND(mWin32Window);
  if(!mWindowImpl.SetWinProc())
  {
    DALI_LOG_ERROR("Failed to install DALi event handler on external HWND\n");
  }
}

void WindowBaseWin::EventEntry(TWinEventInfo* event)
{
  unsigned int uMsg = event->uMsg;

  InputMethodContextWin::ProcessWindowMessage(event->mWindow, uMsg, event->wParam, event->lParam);

  switch(uMsg)
  {
    case WM_CLOSE:
    {
      OnDeleteRequest();
      break;
    }

    case WM_SETFOCUS:
    {
      OnFocusIn(uMsg, event);
      break;
    }

    case WM_KILLFOCUS:
    {
      OnFocusOut(uMsg, event);
      break;
    }

    case WM_PAINT:
    {
      OnWindowDamaged(uMsg, event);
      break;
    }

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
      OnMouseButtonDown(uMsg, event);
      break;
    }

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
      OnMouseButtonUp(uMsg, event);
      break;
    }

    case WM_MOUSEMOVE:
    {
      OnMouseButtonMove(uMsg, event);
      break;
    }

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    {
      OnMouseWheel(uMsg, event);
      break;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
      OnKeyDown(uMsg, event);
      break;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
      OnKeyUp(uMsg, event);
      break;
    }

    case WM_WINDOWPOSCHANGED:
    {
      const auto* windowPosition = reinterpret_cast<const WINDOWPOS*>(event->lParam);
      if(!windowPosition || (windowPosition->flags & SWP_NOMOVE) == 0u || (windowPosition->flags & SWP_NOSIZE) == 0u)
      {
        OnWindowPositionSizeChanged(event);
      }
      break;
    }

    case WM_SIZE:
    {
      if(Dali::Adaptor::IsAvailable())
      {
        const bool minimized = event->wParam == SIZE_MINIMIZED;
        const bool maximized = event->wParam == SIZE_MAXIMIZED;
        if(minimized != mIsIconifiedState)
        {
          mIsIconifiedState = minimized;
          mIconifyChangedSignal.Emit(minimized);
        }
        if(maximized != mIsMaximizedState)
        {
          mIsMaximizedState = maximized;
          mMaximizeChangedSignal.Emit(maximized);
        }
      }
      break;
    }

    default:
      break;
  }
}

void WindowBaseWin::SetParent(WindowBase* parentWinBase, bool belowParent)
{
  HWND window = reinterpret_cast<HWND>(mWin32Window);
  HWND parent = parentWinBase ? static_cast<HWND>(parentWinBase->GetNativeRawHandle()) : nullptr;
  SetLastError(ERROR_SUCCESS);
  const HWND previousParent = ::SetParent(window, parent);
  if(previousParent == nullptr && GetLastError() != ERROR_SUCCESS)
  {
    DALI_LOG_ERROR("Failed to set DALi parent window, error %lu\n", static_cast<unsigned long>(GetLastError()));
    return;
  }

  LONG_PTR style = GetWindowLongPtr(window, GWL_STYLE);
  style &= ~(static_cast<LONG_PTR>(WS_CHILD) | static_cast<LONG_PTR>(WS_POPUP));
  style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;
  SetWindowLongPtr(window, GWL_STYLE, style);
  SetWindowPos(window,
               belowParent && parent ? HWND_BOTTOM : nullptr,
               0,
               0,
               0,
               0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED | (belowParent && parent ? 0u : SWP_NOZORDER));
}

int WindowBaseWin::CreateFrameRenderedSyncFence()
{
  return -1;
}

int WindowBaseWin::CreateFramePresentedSyncFence()
{
  return -1;
}

void WindowBaseWin::SetPositionSizeWithAngle(PositionSize positionSize, int angle)
{
}

void WindowBaseWin::InitializeIme()
{
}

void WindowBaseWin::ImeWindowReadyToRender()
{
}

void WindowBaseWin::RequestMoveToServer()
{
}

void WindowBaseWin::RequestResizeToServer(WindowResizeDirection direction)
{
}

void WindowBaseWin::EnableFloatingMode(bool enable)
{
}

bool WindowBaseWin::IsFloatingModeEnabled() const
{
  return false;
}

void WindowBaseWin::IncludeInputRegion(const BoundsInteger& inputRegion)
{
}

void WindowBaseWin::ExcludeInputRegion(const BoundsInteger& inputRegion)
{
}

bool WindowBaseWin::PointerConstraintsLock()
{
  return false;
}

bool WindowBaseWin::PointerConstraintsUnlock()
{
  return false;
}

void WindowBaseWin::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
}

void WindowBaseWin::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
}

bool WindowBaseWin::PointerWarp(int32_t x, int32_t y)
{
  if(!mWin32Window)
  {
    return false;
  }

  // DALi coordinates are client-relative; SetCursorPos expects screen coordinates.
  POINT screenPoint{static_cast<LONG>(x), static_cast<LONG>(y)};
  HWND  window = reinterpret_cast<HWND>(mWin32Window);
  if(!::ClientToScreen(window, &screenPoint))
  {
    return false;
  }
  return ::SetCursorPos(screenPoint.x, screenPoint.y) != 0;
}

void WindowBaseWin::CursorVisibleSet(bool visible)
{
}

bool WindowBaseWin::KeyboardGrab(Device::Subclass::Type deviceSubclass)
{
  return false;
}

bool WindowBaseWin::KeyboardUnGrab()
{
  return false;
}

void WindowBaseWin::SetFullScreen(bool fullscreen)
{
  return;
}

bool WindowBaseWin::GetFullScreen()
{
  return false;
}

void WindowBaseWin::SetFrontBufferRenderingEnabled(bool enable)
{
}

bool WindowBaseWin::IsFrontBufferRenderingEnabled() const
{
  return false;
}

void WindowBaseWin::SetWindowFrontBufferMode(bool enable)
{
}

void WindowBaseWin::SetModal(bool modal)
{
}

bool WindowBaseWin::IsModal()
{
  return false;
}

void WindowBaseWin::SetAlwaysOnTop(bool alwaysOnTop)
{
}

bool WindowBaseWin::IsAlwaysOnTop() const
{
  return false;
}

void WindowBaseWin::SetBottom(bool enable)
{
}

bool WindowBaseWin::IsBottom()
{
  return false;
}

Any WindowBaseWin::GetNativeBuffer() const
{
  return 0;
}

bool WindowBaseWin::RelativeMotionGrab(uint32_t boundary)
{
  return false;
}

bool WindowBaseWin::RelativeMotionUnGrab()
{
  return false;
}

void WindowBaseWin::SetBackgroundBlur(int blurRadius, int cornerRadius)
{
}

int WindowBaseWin::GetBackgroundBlur()
{
  return 0;
}

void WindowBaseWin::SetBehindBlur(int blurRadius)
{
}

int WindowBaseWin::GetBehindBlur()
{
  return 0;
}

void WindowBaseWin::SetBehindBlurDim(bool enable, Vector4& color)
{
}

Vector4 WindowBaseWin::GetBehindBlurDim(bool& enable)
{
  enable = false;
  return Vector4(0.0, 0.0, 0.0, 0.0);
}

Extents WindowBaseWin::GetInsets()
{
  return Extents(0, 0, 0, 0);
}

Extents WindowBaseWin::GetInsets(WindowInsetsPartFlags insetsFlags)
{
  return Extents(0, 0, 0, 0);
}

void WindowBaseWin::SetScreen(const std::string& screenName)
{
}

std::string WindowBaseWin::GetScreen() const
{
  return std::string();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
