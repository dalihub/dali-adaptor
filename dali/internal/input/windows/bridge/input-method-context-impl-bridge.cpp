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
#include <dali/internal/input/windows/bridge/input-method-context-impl-bridge.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/window.h>

// INTERNAL INCLUDES
#include <dali/internal/input/windows/bridge/ime-bridge-host-registry.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
InputMethodContextPtr InputMethodContextBridge::New(Dali::Actor actor)
{
  InputMethodContextPtr manager;

  if(actor && Dali::Adaptor::IsAvailable())
  {
    InputMethodContextBridge* context = new InputMethodContextBridge(actor);
    manager                           = context;

    // Fix (task-3 final review, I9): the attach used to happen in the
    // constructor, where the RefObject count is still zero -- a bridge that
    // called back synchronously would make one of the IME-to-application
    // paths build a `Dali::InputMethodContext handle(this)`, taking the count
    // 0 -> 1 and then 1 -> 0 on destruction, deleting the object inside its
    // own constructor. `manager` already holds the first reference here, so
    // it is safe now. See RegisterWithBridge()'s documentation.
    context->RegisterWithBridge();
  }

  return manager;
}

// Fix (task-3 final review, I2): mTextPredictionAllowed starts false, not
// true. D3 asks for generic-equivalent behaviour with no bridge, and every
// other backend reports false when nothing is attached: the base
// (input-method-context-impl.h:388-391), generic
// (input-method-context-impl-generic.cpp:297-302), and ecore-wl, which starts
// from a local `false` and only overwrites it from a live IMF context
// (input-method-context-impl-ecore-wl.cpp:1584-1593); windows does not
// override it at all. `true` also made FlushPendingState() push
// SetTextPrediction(this, true) to a newly attached bridge for an application
// that never asked for prediction. mReturnKeyEnabled stays true: base
// (input-method-context-impl.h:309-312) and generic
// (input-method-context-impl-generic.cpp:235-239) both report true there.
InputMethodContextBridge::InputMethodContextBridge(Dali::Actor actor)
: mActor(actor),
  mReturnKeyEnabled(true),
  mTextPredictionAllowed(false),
  mMultiLine(false),
  mPanelPositionSet(false),
  mPanelShowPending(false),
  mInputPanelAutoShow(true)
{
  // Member initialisation only -- see RegisterWithBridge().
}

void InputMethodContextBridge::RegisterWithBridge()
{
  ImeBridgeHostRegistry::Add(this);

  // A bridge may already be registered; attach right away. The reverse
  // order (bridge registers later) is handled by RegisterImeBridge().
  if(auto* bridge = Bridge())
  {
    bridge->AttachHost(this);
    OnBridgeAttached();
  }
}

InputMethodContextBridge::~InputMethodContextBridge()
{
  if(auto* bridge = Bridge())
  {
    bridge->DetachHost(this);
  }
  ImeBridgeHostRegistry::Remove(this);
}

Dali::Integration::ImeBridge* InputMethodContextBridge::Bridge() const
{
  return Dali::Integration::GetImeBridge();
}

void InputMethodContextBridge::Initialize()
{
}

void InputMethodContextBridge::Finalize()
{
}

void InputMethodContextBridge::ConnectCallbacks()
{
}

void InputMethodContextBridge::DisconnectCallbacks()
{
}

// -- application to IME --

void InputMethodContextBridge::Activate()
{
  if(auto* bridge = Bridge())
  {
    bridge->Activate(this);
  }

  // With the input panel enabled (the default), gaining focus shows the
  // panel, as on the platform backends. With no bridge yet this records a
  // pending show that OnBridgeAttached() replays.
  if(mInputPanelAutoShow)
  {
    ShowInputPanel();
  }

  // Fix (task-3 final review, I3): Dali::InputMethodContext::ActivatedSignal()
  // is public API and four of the five platform backends emit it from
  // Activate() (input-method-context-impl-ecore-wl.cpp:702,
  // input-method-context-impl-tcore-wl.cpp:741,
  // input-method-context-impl-win.cpp:357, input-method-context-impl-x.cpp:492).
  // The windows shape is followed here, including its
  // Dali::Adaptor::IsAvailable() guard.
  if(Dali::Adaptor::IsAvailable())
  {
    Dali::InputMethodContext handle(this);
    mActivatedSignal.Emit(handle);
  }
}

void InputMethodContextBridge::Deactivate()
{
  // The hide is requested before focus leaves, as on the platform backends.
  if(mInputPanelAutoShow)
  {
    HideInputPanel();
  }

  if(auto* bridge = Bridge())
  {
    bridge->Deactivate(this);
  }
}

void InputMethodContextBridge::Reset()
{
  if(auto* bridge = Bridge())
  {
    bridge->Reset(this);
  }
}

bool InputMethodContextBridge::ShowInputPanel()
{
  auto* bridge = Bridge();
  if(!bridge)
  {
    // D4: replay once a bridge registers.
    // NOTE (M1, deliberate divergence from generic): the generic backend
    // returns `true` unconditionally here (input-method-context-impl-generic.cpp:247-257),
    // but that `true` carries no information -- generic never actually shows
    // anything either. We return `false` because D4's replay-on-attach
    // (FlushPendingShow(), driven by mPanelShowPending) is predicated on the
    // caller being able to tell this call did not go through yet.
    mPanelShowPending = true;
    return false;
  }

  mPanelShowPending = false;
  return bridge->ShowInputPanel(this);
}

bool InputMethodContextBridge::HideInputPanel()
{
  mPanelShowPending = false;

  auto* bridge = Bridge();
  // NOTE (M1, deliberate divergence from generic): see ShowInputPanel()
  // above -- generic always returns `true`
  // (input-method-context-impl-generic.cpp:252-255); we return `false` when
  // there is no bridge to actually forward the request to.
  return bridge ? bridge->HideInputPanel(this) : false;
}

bool InputMethodContextBridge::AutoEnableInputPanel(bool enabled)
{
  // Only decides whether focus changes show and hide the panel by themselves.
  // An explicit ShowInputPanel()/HideInputPanel() reaches the bridge either
  // way.
  mInputPanelAutoShow = enabled;
  return true;
}

bool InputMethodContextBridge::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  auto* bridge = Bridge();
  return bridge ? bridge->FilterEventKey(this, keyEvent) : false;
}

void InputMethodContextBridge::SetCursorPosition(unsigned int cursorPosition)
{
  mCursorPosition = static_cast<int32_t>(cursorPosition);
  if(auto* bridge = Bridge())
  {
    bridge->SetCursorPosition(this, cursorPosition);
  }
}

unsigned int InputMethodContextBridge::GetCursorPosition() const
{
  return static_cast<unsigned int>(mCursorPosition);
}

void InputMethodContextBridge::SetSurroundingText(const Dali::String& text)
{
  mSurroundingText = text;
  if(auto* bridge = Bridge())
  {
    bridge->SetSurroundingText(this, text);
  }
}

Dali::String InputMethodContextBridge::GetSurroundingText() const
{
  return mSurroundingText;
}

void InputMethodContextBridge::NotifyCursorPosition()
{
  // Fix (task-3 final review, I1): this was empty, and it is the only thing
  // Commit() and PreEdit() call after writing the application's post-event
  // cursor and text back from the returned CallbackData. The IME therefore
  // composed against a stale cursor after every commit. ecore-wl pushes the
  // cursor down here in exactly this way
  // (input-method-context-impl-ecore-wl.cpp:1224-1232).
  auto* bridge = Bridge();
  if(!bridge)
  {
    return;
  }

  bridge->SetCursorPosition(this, static_cast<uint32_t>(mCursorPosition));

  // ecore-wl pushes only the cursor here because Ecore_IMF pulls the
  // surrounding text back on demand through its retrieve-surrounding
  // callback. The bridge is push-only for text, so the text that came back
  // with the cursor goes with it -- otherwise the IME holds the pre-commit
  // string against the post-commit cursor.
  bridge->SetSurroundingText(this, mSurroundingText);
}

bool InputMethodContextBridge::SetInputPanelLayout(Dali::InputMethod::PanelLayout layout)
{
  mPanelLayout = layout;
  if(auto* bridge = Bridge())
  {
    bridge->SetPanelLayout(this, mPanelLayout, mPanelLayoutVariation);
  }
  return true;
}

Dali::InputMethod::PanelLayout InputMethodContextBridge::GetInputPanelLayout() const
{
  return mPanelLayout;
}

bool InputMethodContextBridge::SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation)
{
  mPanelLayoutVariation = variation;
  if(auto* bridge = Bridge())
  {
    bridge->SetPanelLayout(this, mPanelLayout, mPanelLayoutVariation);
  }
  return true;
}

Dali::InputMethod::PanelLayoutVariation InputMethodContextBridge::GetInputPanelLayoutVariation() const
{
  return mPanelLayoutVariation;
}

bool InputMethodContextBridge::SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType action)
{
  mReturnKeyType = action;
  if(auto* bridge = Bridge())
  {
    bridge->SetReturnKey(this, mReturnKeyType, mReturnKeyEnabled);
  }
  return true;
}

Dali::InputMethod::ReturnKeyType InputMethodContextBridge::GetInputPanelReturnKeyType() const
{
  return mReturnKeyType;
}

bool InputMethodContextBridge::SetReturnKeyState(bool visible)
{
  mReturnKeyEnabled = visible;
  if(auto* bridge = Bridge())
  {
    bridge->SetReturnKey(this, mReturnKeyType, mReturnKeyEnabled);
  }
  return true;
}

bool InputMethodContextBridge::IsReturnKeyEnabled() const
{
  return mReturnKeyEnabled;
}

bool InputMethodContextBridge::SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType autoCapital)
{
  mAutoCapitalType = autoCapital;
  if(auto* bridge = Bridge())
  {
    bridge->SetAutoCapital(this, autoCapital);
  }
  return true;
}

Dali::InputMethod::AutoCapitalType InputMethodContextBridge::GetInputPanelAutoCapitalType() const
{
  return mAutoCapitalType;
}

bool InputMethodContextBridge::AllowTextPrediction(bool prediction)
{
  mTextPredictionAllowed = prediction;
  if(auto* bridge = Bridge())
  {
    bridge->SetTextPrediction(this, prediction);
  }
  return true;
}

bool InputMethodContextBridge::IsTextPredictionAllowed() const
{
  return mTextPredictionAllowed;
}

bool InputMethodContextBridge::SetInputPanelUserData(const Dali::String& data)
{
  mUserData = data;
  if(auto* bridge = Bridge())
  {
    bridge->SetUserData(this, data);
  }
  return true;
}

Dali::String InputMethodContextBridge::GetInputPanelUserData() const
{
  // Asks the IME rather than echoing mUserData. The other backends pull too
  // (the X11 one calls ecore_imf_context_input_panel_imdata_get()), and the
  // IME is free to revise the data it was given, so a cache read reports what
  // the application asked for instead of what is actually in effect.
  //
  // An empty answer falls back to the cache: an IME that never registered an
  // imdata-requested handler cannot answer at all, and the last value the
  // application set is both the best available answer and what this backend
  // reported before.
  if(auto* bridge = Bridge())
  {
    Dali::String fromIme = bridge->GetUserData(const_cast<InputMethodContextBridge*>(this));
    if(!fromIme.Empty())
    {
      return fromIme;
    }
  }

  return mUserData;
}

bool InputMethodContextBridge::SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language)
{
  mInputPanelLanguage = language;
  if(auto* bridge = Bridge())
  {
    bridge->SetLanguage(this, language);
  }
  return true;
}

Dali::Integration::InputMethodContext::InputPanelLanguage InputMethodContextBridge::GetInputPanelLanguage() const
{
  return mInputPanelLanguage;
}

void InputMethodContextBridge::SetContentMimeTypes(const Dali::String& mimeTypes)
{
  mContentMimeTypes = mimeTypes;
  if(auto* bridge = Bridge())
  {
    bridge->SetContentMimeTypes(this, mimeTypes);
  }
}

bool InputMethodContextBridge::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  // Fix (task-3 final review, I8): the sim used to inherit the base's
  // unconditional `true` (input-method-context-impl.h:426-431) while nothing
  // happened and the bridge was never told, even though its sibling
  // SetInputPanelPositionAlign() below is forwarded and reports honestly.
  // Note the base signature takes unsigned int; the bridge, like
  // SetPanelPositionAlign(), takes int32_t.
  mPanelPositionX   = static_cast<int32_t>(x);
  mPanelPositionY   = static_cast<int32_t>(y);
  mPanelPositionSet = true;

  auto* bridge = Bridge();
  if(!bridge)
  {
    return false;
  }

  bridge->SetPanelPosition(this, mPanelPositionX, mPanelPositionY);
  return true;
}

bool InputMethodContextBridge::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  auto* bridge = Bridge();
  if(!bridge)
  {
    return false;
  }

  bridge->SetPanelPositionAlign(this, x, y, align);
  return true;
}

void InputMethodContextBridge::NotifyTextInputMultiLine(bool multiLine)
{
  // Fix (task-3 final review, I8): same shape as SetInputPanelPosition() --
  // the base is a no-op (input-method-context-impl.h:247-249) but the value
  // is not cosmetic: ecore-wl turns it into the MULTILINE input hint
  // (input-method-context-impl-ecore-wl.cpp:1261-1269), which is what decides
  // whether the return key inserts a newline or performs the return-key
  // action. An in-process IME needs it for the same reason, so it gets a
  // bridge method rather than being dropped.
  mMultiLine = multiLine;
  if(auto* bridge = Bridge())
  {
    bridge->SetMultiLine(this, multiLine);
  }
}

void InputMethodContextBridge::ApplyOptions(const Dali::Integration::InputMethodOptions& options)
{
  // NOTE: deviation from the brief's "store into mOptions, then CompareAndSet"
  // description. InputMethodOptions holds a std::unique_ptr<Impl> member and
  // declares no copy assignment operator, so the implicit one is deleted
  // (input-method-options.h:55-101); `mOptions = options;` does not compile.
  // CompareAndSet() itself copies only the changed fields from `options` into
  // `mOptions`, matching InputMethodContextWin::ApplyOptions's structure
  // (input-method-context-impl-win.cpp:616-634), which never assigns mOptions
  // wholesale either.
  using namespace Dali::Integration::InputMethod::Category;

  int index = 0;

  if(mOptions.CompareAndSet(PANEL_LAYOUT, options, index))
  {
    // Fix (task-3 review round 1, I1): CompareAndSet(PANEL_LAYOUT, ...)
    // already updated mOptions' own cached variation to match the new
    // layout (input-method-options.cpp:317-318), even when the *raw*
    // variation value did not change (e.g. NORMAL -> PASSWORD both at raw
    // variation 0 map to different PanelLayoutVariation values, so the
    // VARIATION branch below never fires for this transition). Sync
    // mPanelLayoutVariation from mOptions here, before calling
    // SetInputPanelLayout(), so the bridge is sent a consistent
    // (layout, variation) pair instead of pairing the new layout with our
    // still-stale cached variation.
    mPanelLayoutVariation = mOptions.GetPanelLayoutVariation();
    SetInputPanelLayout(mOptions.GetPanelLayout());
  }
  if(mOptions.CompareAndSet(BUTTON_ACTION, options, index))
  {
    SetInputPanelReturnKeyType(static_cast<Dali::InputMethod::ReturnKeyType>(index));
  }
  if(mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index))
  {
    SetInputPanelAutoCapitalType(static_cast<Dali::InputMethod::AutoCapitalType>(index));
  }
  if(mOptions.CompareAndSet(VARIATION, options, index))
  {
    SetInputPanelLayoutVariation(mOptions.GetPanelLayoutVariation());
  }
}

// -- cached reads: never reach the bridge (spec D2) --

Dali::BoundsInteger InputMethodContextBridge::GetInputPanelArea()
{
  return mInputPanelArea;
}

Dali::InputMethodContext::State InputMethodContextBridge::GetInputPanelState()
{
  return mInputPanelState;
}

Dali::InputMethodContext::KeyboardType InputMethodContextBridge::GetKeyboardType()
{
  // Fix (task-3 final review, signal surface): this used to hardcode
  // SOFTWARE_KEYBOARD, so KeyboardTypeChangedSignal() could never fire and
  // the getter could never disagree with it. It now reports whatever the IME
  // last announced through KeyboardTypeChanged(), defaulting to
  // SOFTWARE_KEYBOARD.
  return mKeyboardType;
}

void InputMethodContextBridge::GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  attrs = mPreeditAttrs;
}

// -- IME to application --

void InputMethodContextBridge::Commit(const Dali::String& text)
{
  if(text.Empty() || !Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::COMMIT, text, 0, 0);

  mEventSignal.Emit(handle, eventData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

  if(callbackData.update)
  {
    mCursorPosition  = callbackData.cursorPosition;
    mSurroundingText = callbackData.currentText;
    NotifyCursorPosition();
  }
}

void InputMethodContextBridge::PreEdit(const Dali::String&                                                        text,
                                    int32_t                                                                     cursorPosition,
                                    const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs)
{
  mPreeditAttrs = attrs;

  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::PRE_EDIT, text, cursorPosition, 0);

  mEventSignal.Emit(handle, eventData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

  if(callbackData.update)
  {
    mCursorPosition  = callbackData.cursorPosition;
    mSurroundingText = callbackData.currentText;
    NotifyCursorPosition();
  }

  if(callbackData.preeditResetRequired)
  {
    Reset();
  }
}

void InputMethodContextBridge::PreEditStart()
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::PRE_EDIT, Dali::String(), 0, 0);

  mEventSignal.Emit(handle, eventData);
  mKeyboardEventSignal.Emit(handle, eventData);
}

void InputMethodContextBridge::PreEditEnd()
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  // Fix (task-3 final review, I4): this used to send EventType::VOID, which
  // the only consumer handles with "// do nothing"
  // (dali-toolkit/internal/text/controller/text-controller-event-handler.cpp:930-934,
  // branch tizen), so a composition the IME abandoned without committing --
  // Escape, focus loss, Reset -- stayed in the text field for ever; nothing
  // else in this backend clears one.
  //
  // An empty PRE_EDIT is what actually clears it: the toolkit's PRE_EDIT case
  // calls TextUpdater::InsertText(..., PRE_EDIT)
  // (text-controller-event-handler.cpp:869-875), whose first act is to remove
  // the previous pre-edit run and put the cursor back where the composition
  // started (text-controller-text-updater.cpp:224-240). The empty string then
  // inserts nothing, and the relayout/cursor update still happens
  // (text-controller-text-updater.cpp:455-484). This is also the exact event
  // PreEditStart() already sends, so it is a state the toolkit is known to
  // tolerate. See the report for the one residual: with a non-empty field the
  // toolkit leaves mPreEditFlag set (with mPreEditLength 0), which it also
  // does after PreEditStart(); only the next COMMIT or key event clears it.
  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::PRE_EDIT, Dali::String(), 0, 0);

  mEventSignal.Emit(handle, eventData);
  mKeyboardEventSignal.Emit(handle, eventData);
}

void InputMethodContextBridge::DeleteSurrounding(int32_t cursorOffset, int32_t numberOfChars)
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::DELETE_SURROUNDING, Dali::String(), cursorOffset, numberOfChars);

  mEventSignal.Emit(handle, eventData);
  mKeyboardEventSignal.Emit(handle, eventData);
}

void InputMethodContextBridge::SetSelection(int32_t startIndex, int32_t endIndex)
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::SELECTION_SET, startIndex, endIndex);

  mEventSignal.Emit(handle, eventData);
  mKeyboardEventSignal.Emit(handle, eventData);
}

void InputMethodContextBridge::SendPrivateCommand(const Dali::String& command)
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData eventData(
    Dali::Integration::InputMethodContext::PRIVATE_COMMAND, command, 0, 0);

  // Fix (task-3 review round 1, I2): the brief's table omitted this signal.
  // Both Tizen backends emit it between the two event signals
  // (input-method-context-impl-ecore-wl.cpp:1023,
  // input-method-context-impl-tcore-wl.cpp:1084).
  mEventSignal.Emit(handle, eventData);
  mPrivateCommandReceivedSignal.Emit(handle, command);
  mKeyboardEventSignal.Emit(handle, eventData);
}

void InputMethodContextBridge::CommitContent(const Dali::String& contentUri, const Dali::String& description, const Dali::String& mimeTypes)
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  // Emits both the public and legacy content-received signals
  // (input-method-context-impl.h:656).
  EmitContentReceivedSignal(contentUri, description, mimeTypes);
}

void InputMethodContextBridge::RecaptureString(int32_t                                                                     cursorOffset,
                                            int32_t                                                                     numberOfChars,
                                            const Dali::String&                                                         preedit,
                                            const Dali::String&                                                         commit,
                                            const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs)
{
  // Received and reported, not performed. EventType has no atomic
  // replace-a-range event, and the only way to build one from what it does
  // have -- DELETE_SURROUNDING then COMMIT then PRE_EDIT -- is exactly the
  // decomposition ImeBridgeHost::RecaptureString() tells implementations not
  // to do: the application would see, and could act on, a state in which the
  // old text is gone and the new text has not arrived.
  //
  // Closing this needs a new integration event type and a toolkit that
  // handles it; until then an error is better than silence, because the IME
  // has no way to find out its request did nothing.
  (void)preedit;
  (void)commit;
  (void)attrs;

  DALI_LOG_ERROR("InputMethodContextBridge::RecaptureString is not supported; dropped (offset %d, %d chars)\n",
                 cursorOffset,
                 numberOfChars);
}

bool InputMethodContextBridge::RetrieveSurrounding(Dali::String& text, int32_t& cursorPosition)
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return false;
  }

  Dali::InputMethodContext                         handle(this);
  Dali::Integration::InputMethodContext::EventData imfData(
    Dali::Integration::InputMethodContext::GET_SURROUNDING, Dali::String(), 0, 0);

  mEventSignal.Emit(handle, imfData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

  if(!callbackData.update)
  {
    return false;
  }

  mSurroundingText = callbackData.currentText;
  mCursorPosition  = callbackData.cursorPosition;
  text             = mSurroundingText;
  cursorPosition   = mCursorPosition;
  return true;
}

void InputMethodContextBridge::FeedKeyEvent(const Dali::KeyEvent& keyEvent)
{
  // Fix (task-3 review round 1, I3): the original implementation always went
  // through Dali::Adaptor::FeedKeyEvent(), which is
  // mWindows.front()->FeedKeyEvent(...)
  // (dali/internal/adaptor/common/adaptor-impl.cpp:598-602) -- the *first*
  // window, regardless of which one actually owns the focused control. Fixed
  // per review: resolve the actor's own window first via Dali::Window::Get()
  // (dali/public-api/adaptor-framework/window.h:184) and feed it there
  // directly (window.h:1010, takes a const KeyEvent&, no copy needed). Only
  // fall back to the process-wide entry point when the actor is not (yet)
  // staged on any window.
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  Dali::Actor actor = mActor.GetHandle();
  if(actor)
  {
    Dali::Window window = Dali::Window::Get(actor);
    if(window)
    {
      window.FeedKeyEvent(keyEvent);
      return;
    }
  }

  // Fallback: the actor is not on any window (not yet staged, or this
  // context was constructed with an empty actor). This targets
  // mWindows.front() (adaptor-impl.cpp:598-602), which is only a guess, but
  // it is the best process-wide guess available without a window to target.
  Dali::KeyEvent event = keyEvent;
  Dali::Adaptor::Get().FeedKeyEvent(event);
}

void InputMethodContextBridge::InputPanelGeometryChanged(const Dali::BoundsInteger& area)
{
  mInputPanelArea = area;

  // Fix (task-3 review round 1, I5): the plan's premise that no backend
  // emits status/resize from its impl was wrong. ecore-wl and tcore-wl both
  // call EmitKeyboardResizedSignal() directly from their geometry-changed
  // callback, ungated by Dali::Adaptor::IsAvailable()
  // (input-method-context-impl-ecore-wl.cpp:190,
  // input-method-context-impl-tcore-wl.cpp:207) -- matched here. Those
  // callbacks receive an opaque ecore int unrelated to any rect, so there is
  // no literal value to reuse; height is passed here as the closest
  // equivalent to the legacy resize signal's traditional meaning (how much
  // vertical space the input panel occupies).
  EmitKeyboardResizedSignal(area.height);
}

void InputMethodContextBridge::InputPanelStateChanged(Dali::InputMethodContext::State state)
{
  mInputPanelState = state;

  // Fix (task-3 review round 1, I5): see InputPanelGeometryChanged() above.
  // ecore-wl/tcore-wl call EmitStatusChangedSignal() directly from their
  // state-changed callback (input-method-context-impl-ecore-wl.cpp:150,155,160,
  // input-method-context-impl-tcore-wl.cpp:167,172,177), ungated -- matched here.
  EmitStatusChangedSignal(state);
}

void InputMethodContextBridge::InputPanelLanguageChanged(Dali::Integration::InputMethodContext::InputPanelLanguage language)
{
  // Fix (task-3 final review, signal surface): LanguageChangedSignal() and
  // LegacyLanguageChangedSignal() were permanently dead here because
  // ImeBridgeHost had no way for the IME to report the change, and
  // GetInputPanelLanguage() only ever echoed what the application had set.
  // The cache is updated first so a handler reading it back during the emit
  // sees the new value, matching ecore-wl's ordering
  // (input-method-context-impl-ecore-wl.cpp:170-179) -- and, like ecore-wl's
  // callback and the two callbacks above, ungated by
  // Dali::Adaptor::IsAvailable().
  mInputPanelLanguage = language;

  // Platform backends pass an opaque platform language id to the legacy
  // signal. There is no such id for an in-process IME, so the DALi enum
  // value is what the legacy signal carries.
  EmitLanguageChangedSignal(static_cast<int>(language));
}

void InputMethodContextBridge::KeyboardTypeChanged(Dali::InputMethodContext::KeyboardType keyboardType)
{
  // Fix (task-3 final review, signal surface): as above for
  // KeyboardTypeChangedSignal()/LegacyKeyboardTypeChangedSignal(); ecore-wl
  // emits from its own keyboard-mode callback
  // (input-method-context-impl-ecore-wl.cpp:206,212).
  mKeyboardType = keyboardType;

  EmitKeyboardTypeChangedSignal(keyboardType);
}

void InputMethodContextBridge::OnBridgeAttached()
{
  // A synchronous bridge callback may release the last external reference.
  InputMethodContextPtr guard(this);

  // Runs on both attach paths (RegisterWithBridge() from New(), and late
  // RegisterImeBridge()). On the New() path there is nothing cached yet
  // beyond the member defaults, so this pushes the same defaults a freshly
  // constructed bridge-backed context would send anyway via its setters --
  // that duplication is intentional, not a bug.
  FlushPendingState();
  FlushPendingShow();
}

void InputMethodContextBridge::OnBridgeDetached()
{
  // Keep the context alive until both panel notifications have completed.
  InputMethodContextPtr guard(this);

  // Fix (task-3 final review, I6): the bridge is gone, so everything cached
  // on the IME's behalf describes an input panel that no longer exists.
  // Without this, a context whose panel was showing keeps answering SHOW from
  // GetInputPanelState() -- with geometry -- for ever, and D2's "the adaptor
  // stands on its own when the bridge unregisters" would not hold.
  mPanelShowPending = false;

  const Dali::BoundsInteger emptyArea(0, 0, 0, 0);

  const bool areaChanged  = (mInputPanelArea != emptyArea);
  const bool stateChanged = (mInputPanelState != Dali::InputMethodContext::State::HIDE);

  mInputPanelArea  = emptyArea;
  mInputPanelState = Dali::InputMethodContext::State::HIDE;

  // Emitted only on a real transition. Announcing HIDE to a context that was
  // already hidden would push a spurious `false` onto
  // LegacyStatusChangedSignal() (input-method-context-impl.cpp:154-161) every
  // time any bridge unregisters.
  if(areaChanged)
  {
    EmitKeyboardResizedSignal(mInputPanelArea.height);
  }
  if(stateChanged)
  {
    EmitStatusChangedSignal(mInputPanelState);
  }
}

void InputMethodContextBridge::FlushPendingShow()
{
  if(!mPanelShowPending)
  {
    return;
  }

  if(auto* bridge = Bridge())
  {
    mPanelShowPending = false;
    bridge->ShowInputPanel(this);
  }
}

void InputMethodContextBridge::FlushPendingState()
{
  // Fix (task-3 review round 1, I4): a TextField that configures the
  // context before a bridge registers (the ordering D4 exists for) used to
  // have every setting but the pending show silently dropped, because only
  // FlushPendingShow() replayed anything on attach. Replay the full cached
  // configuration here, called ahead of FlushPendingShow() from
  // OnBridgeAttached(). Never constructs a Dali handle or IntrusivePtr from
  // `this`.
  auto* bridge = Bridge();
  if(!bridge)
  {
    return;
  }

  bridge->SetPanelLayout(this, mPanelLayout, mPanelLayoutVariation);
  bridge->SetReturnKey(this, mReturnKeyType, mReturnKeyEnabled);
  bridge->SetAutoCapital(this, mAutoCapitalType);
  bridge->SetTextPrediction(this, mTextPredictionAllowed);
  bridge->SetUserData(this, mUserData);
  bridge->SetLanguage(this, mInputPanelLanguage);
  bridge->SetContentMimeTypes(this, mContentMimeTypes);
  bridge->SetMultiLine(this, mMultiLine);
  bridge->SetCursorPosition(this, static_cast<uint32_t>(mCursorPosition));
  bridge->SetSurroundingText(this, mSurroundingText);

  // Unlike the values above, (0, 0) is a meaningful panel position rather
  // than an "unset" default, so this is replayed only when the application
  // actually asked for one.
  if(mPanelPositionSet)
  {
    bridge->SetPanelPosition(this, mPanelPositionX, mPanelPositionY);
  }
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
