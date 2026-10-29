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

// NOTE: the Ubuntu UTC executable compiles the production ime-bridge.cpp and
// input-method-context-impl-bridge.cpp from dali/internal/input/windows/bridge/.
// It links the installed platform libdali2-adaptor.so for the remaining adaptor
// implementation, without compiling the bridge factory or replacing Ubuntu's
// input backend. The registry tests in utc-Dali-ImeBridge.cpp use the same
// bridge implementation in this executable.

#include <adaptor-test-application.h>
#include <dali-test-suite-utils.h>
#include <dali/integration-api/adaptor-framework/ime-bridge.h>
#include <dali/internal/input/windows/bridge/ime-bridge-host-registry.h>
#include <dali/internal/input/windows/bridge/input-method-context-impl-bridge.h>
#include <dali/public-api/object/property-map.h>

#include <algorithm>
#include <vector>

using namespace Dali;

namespace
{
/// Bridge that records calls.
class RecordingBridge : public Dali::Integration::ImeBridge
{
public:
  std::vector<Dali::Integration::ImeBridgeHost*> attached;
  int                                             showCount{0};
  int                                             hideCount{0};
  int                                             filterCount{0};
  int                                             returnKeyCount{0};
  int                                             panelPositionCount{0};
  int                                             activateCount{0};
  int                                             deactivateCount{0};
  int                                             resetCount{0};
  int                                             cursorPositionCount{0};
  int                                             surroundingTextCount{0};
  uint32_t                                        lastCursorPosition{0u};
  Dali::String                                    lastSurroundingText;
  bool                                            filterResult{false};
  bool                                            hideResult{false};
  Dali::InputMethod::PanelLayout                  lastLayout{Dali::InputMethod::PanelLayout::NORMAL};
  Dali::InputMethod::PanelLayoutVariation         lastVariation{Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL};
  Dali::InputMethod::ReturnKeyType                lastReturnKeyType{Dali::InputMethod::ReturnKeyType::DEFAULT};
  bool                                            lastReturnKeyEnabled{true};
  bool                                            lastMultiLine{false};
  int32_t                                         lastPanelPositionX{-1};
  int32_t                                         lastPanelPositionY{-1};
  Dali::Integration::InputMethodContext::InputPanelLanguage lastLanguage{
    Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC};
  bool lastTextPrediction{true};
  int          userDataSetCount{0};
  int          userDataGetCount{0};
  Dali::String lastUserDataSet;
  Dali::String userDataToReturn;

  void AttachHost(Dali::Integration::ImeBridgeHost* host) override { attached.push_back(host); }
  void DetachHost(Dali::Integration::ImeBridgeHost* host) override
  {
    attached.erase(std::remove(attached.begin(), attached.end(), host), attached.end());
  }
  void Activate(Dali::Integration::ImeBridgeHost*) override { ++activateCount; }
  void Deactivate(Dali::Integration::ImeBridgeHost*) override { ++deactivateCount; }
  void Reset(Dali::Integration::ImeBridgeHost*) override { ++resetCount; }
  bool ShowInputPanel(Dali::Integration::ImeBridgeHost*) override
  {
    ++showCount;
    return true;
  }
  bool HideInputPanel(Dali::Integration::ImeBridgeHost*) override
  {
    ++hideCount;
    return hideResult;
  }
  bool FilterEventKey(Dali::Integration::ImeBridgeHost*, const Dali::KeyEvent&) override
  {
    ++filterCount;
    return filterResult;
  }
  void SetCursorPosition(Dali::Integration::ImeBridgeHost*, uint32_t position) override
  {
    ++cursorPositionCount;
    lastCursorPosition = position;
  }
  void SetSurroundingText(Dali::Integration::ImeBridgeHost*, const Dali::String& text) override
  {
    ++surroundingTextCount;
    lastSurroundingText = text;
  }
  void SetPanelLayout(Dali::Integration::ImeBridgeHost*, Dali::InputMethod::PanelLayout layout, Dali::InputMethod::PanelLayoutVariation variation) override
  {
    lastLayout    = layout;
    lastVariation = variation;
  }
  void SetReturnKey(Dali::Integration::ImeBridgeHost*, Dali::InputMethod::ReturnKeyType type, bool enabled) override
  {
    ++returnKeyCount;
    lastReturnKeyType    = type;
    lastReturnKeyEnabled = enabled;
  }
  void SetAutoCapital(Dali::Integration::ImeBridgeHost*, Dali::InputMethod::AutoCapitalType) override {}
  void SetTextPrediction(Dali::Integration::ImeBridgeHost*, bool allowed) override { lastTextPrediction = allowed; }
  void SetUserData(Dali::Integration::ImeBridgeHost*, const Dali::String& data) override
  {
    ++userDataSetCount;
    lastUserDataSet = data;
  }
  Dali::String GetUserData(Dali::Integration::ImeBridgeHost*) override
  {
    ++userDataGetCount;
    return userDataToReturn;
  }
  void SetLanguage(Dali::Integration::ImeBridgeHost*, Dali::Integration::InputMethodContext::InputPanelLanguage language) override { lastLanguage = language; }
  void SetContentMimeTypes(Dali::Integration::ImeBridgeHost*, const Dali::String&) override {}
  void SetPanelPosition(Dali::Integration::ImeBridgeHost*, int32_t x, int32_t y) override
  {
    ++panelPositionCount;
    lastPanelPositionX = x;
    lastPanelPositionY = y;
  }
  void SetPanelPositionAlign(Dali::Integration::ImeBridgeHost*, int32_t, int32_t, Dali::InputMethodContext::InputPanelAlign) override {}
  void SetMultiLine(Dali::Integration::ImeBridgeHost*, bool multiLine) override { lastMultiLine = multiLine; }
};

/// Construct the backend directly. InputMethodContextBridge::New() requires an
/// Adaptor and a non-empty Actor, but the app->IME delegation path does not
/// touch the Adaptor. Since the constructor does not use the actor, an empty
/// handle is sufficient (Core initialization is not required).
///
/// Fix (task-3 final review, I9): the bridge attach no longer happens in the
/// constructor -- it would run with the RefObject count still at zero. New()
/// does it through RegisterWithBridge() once an InputMethodContextPtr holds
/// the first reference, and this helper calls the very same function in the
/// very same order, so the tests still exercise the production attach path.
///
/// This helper exists for the app->IME tests, which want neither an Adaptor
/// nor a Core. `InputMethodContextBridge::New()` needs both (a non-empty
/// Dali::Actor requires an initialised Core), so the IME->app tests at the
/// bottom of this file stand up an AdaptorTestApplication and call New()
/// directly instead of using this.
Dali::Internal::Adaptor::InputMethodContextPtr MakeBridgeContext()
{
  auto* sim = new Dali::Internal::Adaptor::InputMethodContextBridge(Dali::Actor());

  Dali::Internal::Adaptor::InputMethodContextPtr context(sim);
  sim->RegisterWithBridge();

  return context;
}

// Free functions for signal-connect tests: DALi's Signal<>::Connect() only
// takes a plain function pointer without a ConnectionTracker (see e.g.
// utc-Dali-Application.cpp's *SignalCallback functions for the same idiom).
bool                             gStatusChangedFired = false;
Dali::InputMethodContext::State  gLastStatusChangedState{Dali::InputMethodContext::State::HIDE};
bool                             gResizedFired = false;

void RecordStatusChanged(Dali::InputMethodContext, Dali::InputMethodContext::State state)
{
  gStatusChangedFired    = true;
  gLastStatusChangedState = state;
}

void RecordResized(Dali::InputMethodContext)
{
  gResizedFired = true;
}

bool                                   gLanguageChangedFired = false;
bool                                   gKeyboardTypeChangedFired = false;
Dali::InputMethodContext::KeyboardType gLastKeyboardType{Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD};

void RecordLanguageChanged(Dali::InputMethodContext)
{
  gLanguageChangedFired = true;
}

void RecordKeyboardTypeChanged(Dali::InputMethodContext, Dali::InputMethodContext::KeyboardType type)
{
  gKeyboardTypeChangedFired = true;
  gLastKeyboardType         = type;
}

// ---------------------------------------------------------------------------
// Recorders for the IME->application direction.
//
// All ten of those ImeBridgeHost methods early-return unless
// Dali::Adaptor::IsAvailable(), so the tests below stand up an
// AdaptorTestApplication -- the same thing utc-Dali-TiltSensor.cpp does in this
// suite (:112,127,144,158,172,186,207,224,241). It makes
// Internal::Adaptor::Adaptor::mAvailable true
// (adaptor-test-application.h:40), and adaptor-test-adaptor-impl.cpp -- which
// is compiled into this executable (CMakeLists.txt:94) -- preempts the
// adaptor's own IsAvailable(). That preemption demonstrably works:
// utc-Dali-Timer.cpp's UtcDaliTimerSetInterval02 asserts
// timer.IsRunning() == true, and TimerEcore only assigns the id it reports
// inside if(Dali::Adaptor::IsAvailable()) (timer-impl-ecore.cpp:114-117).
// ---------------------------------------------------------------------------

int                                                 gEventReceivedCount = 0;
int                                                 gKeyboardEventCount = 0;
Dali::Integration::InputMethodContext::EventType    gLastEventName{Dali::Integration::InputMethodContext::VOID};
Dali::String                                        gLastPredictiveString;
int32_t                                             gLastCursorOffset  = 0;
int32_t                                             gLastNumberOfChars = 0;
int32_t                                             gLastStartIndex    = 0;
int32_t                                             gLastEndIndex      = 0;
Dali::Integration::InputMethodContext::CallbackData gCallbackDataToReturn;

bool         gContentReceivedFired = false;
Dali::String gLastContentUri;
Dali::String gLastContentDescription;
Dali::String gLastContentMimeTypes;

bool         gPrivateCommandFired = false;
Dali::String gLastPrivateCommand;
int          gEventCountWhenPrivateCommandRan    = -1;
int          gKeyboardCountWhenPrivateCommandRan = -1;

bool gActivatedFired = false;

Dali::Integration::InputMethodContext::CallbackData RecordEventReceived(
  Dali::InputMethodContext,
  const Dali::Integration::InputMethodContext::EventData&)
{
  ++gEventReceivedCount;
  // EventReceivedSignal's return value is discarded by every backend; only
  // KeyboardEventReceivedSignal's is read.
  return Dali::Integration::InputMethodContext::CallbackData();
}

Dali::Integration::InputMethodContext::CallbackData RecordKeyboardEvent(
  Dali::InputMethodContext,
  const Dali::Integration::InputMethodContext::EventData& data)
{
  ++gKeyboardEventCount;
  gLastEventName        = data.eventName;
  gLastPredictiveString = data.predictiveString;
  gLastCursorOffset     = data.cursorOffset;
  gLastNumberOfChars    = data.numberOfChars;
  gLastStartIndex       = data.startIndex;
  gLastEndIndex         = data.endIndex;
  return gCallbackDataToReturn;
}

void RecordContentReceived(Dali::InputMethodContext,
                           const Dali::Integration::InputMethodContext::ContentReceivedData& data)
{
  gContentReceivedFired   = true;
  gLastContentUri         = data.contentUri;
  gLastContentDescription = data.description;
  gLastContentMimeTypes   = data.mimeTypes;
}

void RecordPrivateCommand(Dali::InputMethodContext, const Dali::String& command)
{
  gPrivateCommandFired                = true;
  gLastPrivateCommand                 = command;
  gEventCountWhenPrivateCommandRan    = gEventReceivedCount;
  gKeyboardCountWhenPrivateCommandRan = gKeyboardEventCount;
}

void RecordActivated(Dali::InputMethodContext)
{
  gActivatedFired = true;
}

void ResetHostRecorders()
{
  gEventReceivedCount   = 0;
  gKeyboardEventCount   = 0;
  gLastEventName        = Dali::Integration::InputMethodContext::VOID;
  gLastPredictiveString = Dali::String();
  gLastCursorOffset     = 0;
  gLastNumberOfChars    = 0;
  gLastStartIndex       = 0;
  gLastEndIndex         = 0;
  gCallbackDataToReturn = Dali::Integration::InputMethodContext::CallbackData();

  gContentReceivedFired   = false;
  gLastContentUri         = Dali::String();
  gLastContentDescription = Dali::String();
  gLastContentMimeTypes   = Dali::String();

  gPrivateCommandFired                = false;
  gLastPrivateCommand                 = Dali::String();
  gEventCountWhenPrivateCommandRan    = -1;
  gKeyboardCountWhenPrivateCommandRan = -1;

  gActivatedFired = false;
}

/// Connects the two low-level event signals and clears the bridge's
/// push counters, which FlushPendingState() has already bumped once on attach.
void ArmHostRecorders(Dali::Internal::Adaptor::InputMethodContext& impl, RecordingBridge& bridge)
{
  ResetHostRecorders();
  impl.EventReceivedSignal().Connect(&RecordEventReceived);
  impl.KeyboardEventReceivedSignal().Connect(&RecordKeyboardEvent);

  bridge.cursorPositionCount  = 0;
  bridge.surroundingTextCount = 0;
  bridge.resetCount           = 0;
}

} // unnamed namespace

int UtcDaliImeBridgeContextAttachesOnCreation(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  {
    auto context = MakeBridgeContext();
    DALI_TEST_EQUALS(bridge.attached.size(), 1u, TEST_LOCATION);
  }
  // It detaches when destroyed.
  DALI_TEST_EQUALS(bridge.attached.size(), 0u, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextForwardsShowInputPanel(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  DALI_TEST_EQUALS(impl.ShowInputPanel(), true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.showCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextForwardsHideInputPanel(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  bridge.hideResult = true;
  DALI_TEST_EQUALS(impl.HideInputPanel(), true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.hideCount, 1, TEST_LOCATION);

  bridge.hideResult = false;
  DALI_TEST_EQUALS(impl.HideInputPanel(), false, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.hideCount, 2, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextAutoShowsOnActivateAndHidesOnDeactivateByDefault(void)
{
  // The input panel starts enabled, as on the platform backends: focus alone
  // shows and hides it.
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  impl.Activate();
  DALI_TEST_EQUALS(bridge.activateCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.showCount, 1, TEST_LOCATION);

  impl.Deactivate();
  DALI_TEST_EQUALS(bridge.hideCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.deactivateCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextAutoShowDisabledLeavesFocusChangesAlone(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  DALI_TEST_EQUALS(impl.AutoEnableInputPanel(false), true, TEST_LOCATION);

  impl.Activate();
  impl.Deactivate();
  DALI_TEST_EQUALS(bridge.activateCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.deactivateCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.showCount, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.hideCount, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextExplicitShowAndHideIgnoreAutoShow(void)
{
  // Disabling the input panel only stops focus changes from showing it. An
  // application that asks for the panel still gets it.
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  impl.AutoEnableInputPanel(false);

  DALI_TEST_EQUALS(impl.ShowInputPanel(), true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.showCount, 1, TEST_LOCATION);

  bridge.hideResult = true;
  DALI_TEST_EQUALS(impl.HideInputPanel(), true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.hideCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextAutoShowBeforeRegistrationIsReplayed(void)
{
  // Declared before the context so it outlives it; registered only later.
  RecordingBridge bridge;

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  impl.Activate(); // no bridge yet: the automatic show is kept pending

  Dali::Integration::RegisterImeBridge(&bridge);
  DALI_TEST_EQUALS(bridge.showCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextForwardsFilterEventKeyResult(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  // An empty KeyEvent is sufficient; the bridge only determines the return value.
  Dali::KeyEvent keyEvent;

  bridge.filterResult = true;
  DALI_TEST_EQUALS(impl.FilterEventKey(keyEvent), true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.filterCount, 1, TEST_LOCATION);

  bridge.filterResult = false;
  DALI_TEST_EQUALS(impl.FilterEventKey(keyEvent), false, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.filterCount, 2, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextForwardsPanelLayout(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  impl.SetInputPanelLayout(Dali::InputMethod::PanelLayout::NUMBER);

  DALI_TEST_EQUALS(static_cast<int>(bridge.lastLayout),
                   static_cast<int>(Dali::InputMethod::PanelLayout::NUMBER),
                   TEST_LOCATION);
  // The cache is also updated so the app can read it.
  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelLayout()),
                   static_cast<int>(Dali::InputMethod::PanelLayout::NUMBER),
                   TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextForwardsReturnKeyTypeAndState(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  impl.SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType::DONE);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastReturnKeyType),
                   static_cast<int>(Dali::InputMethod::ReturnKeyType::DONE),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastReturnKeyEnabled, true, TEST_LOCATION); // default IsReturnKeyEnabled()

  // Changing only the enabled state must keep sending the current type
  // alongside it -- the bridge API takes both together.
  impl.SetReturnKeyState(false);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastReturnKeyType),
                   static_cast<int>(Dali::InputMethod::ReturnKeyType::DONE),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastReturnKeyEnabled, false, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextSetInputPanelPositionAlignReturnsTrueWhenBridged(void)
{
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  // Task 2's skeleton returned false unconditionally; with a bridge attached
  // this must now report that the request was forwarded.
  DALI_TEST_EQUALS(impl.SetInputPanelPositionAlign(10, 20, Dali::InputMethodContext::InputPanelAlign::TOP_LEFT),
                   true,
                   TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextApplyOptionsSendsConsistentLayoutAndVariation(void)
{
  // Regression test for review round 1 finding I1: CompareAndSet(PANEL_LAYOUT)
  // updates mOptions' cached variation even when the *raw* variation value is
  // unchanged (NORMAL -> PASSWORD both start at raw variation 0, but map to
  // different PanelLayoutVariation values), so the VARIATION category never
  // fires for this transition. If ApplyOptions() forwards the new layout
  // paired with the context's stale cached variation, this test fails.
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  Dali::Property::Map settings;
  settings["PANEL_LAYOUT"]    = 10; // PASSWORD, see NormalizePanelLayout() in input-method-options.cpp
  settings["BUTTON_ACTION"]   = static_cast<int>(Dali::InputMethod::ReturnKeyType::DEFAULT);
  settings["AUTO_CAPITALIZE"] = static_cast<int>(Dali::InputMethod::AutoCapitalType::SENTENCE);
  settings["VARIATION"]       = 0;

  Dali::Integration::InputMethodOptions options;
  options.ApplyProperty(settings);

  impl.ApplyOptions(options);

  DALI_TEST_EQUALS(static_cast<int>(bridge.lastLayout),
                   static_cast<int>(Dali::InputMethod::PanelLayout::PASSWORD),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastVariation),
                   static_cast<int>(Dali::InputMethod::PanelLayoutVariation::PASSWORD_NORMAL),
                   TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextHostCallbacksUpdateCacheAndEmitSignals(void)
{
  // The two ungated host callbacks (InputPanelGeometryChanged/
  // InputPanelStateChanged) need no bridge and no Adaptor -- this checks both
  // the cache round-trip and, per review round 1 finding I5, that the
  // corresponding app-facing signal actually fires.
  gStatusChangedFired = false;
  gResizedFired        = false;

  auto context = MakeBridgeContext();

  // Fix (task-3 review round 2): InputMethodContext and ImeBridgeHost are
  // unrelated sibling bases, joined only at InputMethodContextBridge
  // (input-method-context-impl-bridge.h:45); static_cast between them directly
  // is an ill-formed cross-cast. Go down to the most-derived type once, then
  // let both references bind by implicit (well-formed) upcast.
  auto&                                         sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext&  impl = sim;
  Dali::Integration::ImeBridgeHost&             host = sim;

  impl.StatusChangedSignal().Connect(&RecordStatusChanged);
  impl.ResizedSignal().Connect(&RecordResized);

  const Dali::BoundsInteger area(1, 2, 3, 4);
  host.InputPanelGeometryChanged(area);

  const Dali::BoundsInteger readBack = impl.GetInputPanelArea();
  DALI_TEST_EQUALS(readBack.x, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(readBack.y, 2, TEST_LOCATION);
  DALI_TEST_EQUALS(readBack.width, 3, TEST_LOCATION);
  DALI_TEST_EQUALS(readBack.height, 4, TEST_LOCATION);
  DALI_TEST_CHECK(gResizedFired);

  host.InputPanelStateChanged(Dali::InputMethodContext::State::SHOW);
  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelState()),
                   static_cast<int>(Dali::InputMethodContext::State::SHOW),
                   TEST_LOCATION);
  DALI_TEST_CHECK(gStatusChangedFired);
  DALI_TEST_EQUALS(static_cast<int>(gLastStatusChangedState),
                   static_cast<int>(Dali::InputMethodContext::State::SHOW),
                   TEST_LOCATION);

  END_TEST;
}

int UtcDaliImeBridgeLateRegistrationReplaysPendingShow(void)
{
  // Context first, bridge later. The show request must not be lost.
  //
  // Fix (task-3 final review, M5): the bridge is *declared* before the
  // context, so it is destroyed after it, and only *registered* afterwards --
  // which is the ordering this test is about. Declaring it after the context
  // relied on the explicit UnregisterImeBridge() below running first for the
  // teardown to be safe.
  RecordingBridge bridge;

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  DALI_TEST_EQUALS(impl.ShowInputPanel(), false, TEST_LOCATION); // no bridge

  Dali::Integration::RegisterImeBridge(&bridge);

  DALI_TEST_EQUALS(bridge.attached.size(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.showCount, 1, TEST_LOCATION); // OnBridgeAttached replays it

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeLateRegistrationReplaysCachedConfiguration(void)
{
  // Regression test for review finding I10: FlushPendingState() had no
  // coverage whatsoever -- deleting the call from OnBridgeAttached() left all
  // fourteen tests passing, because every layout/return-key/options test set
  // its value only *after* the bridge was registered, and the
  // late-registration test asserted the show count alone. Configuring the
  // context with no bridge and only then registering one is exactly the
  // ordering D4 exists for, and none of it may be lost.
  RecordingBridge bridge; // declared before the context (M5), registered later

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  DALI_TEST_EQUALS(bridge.attached.size(), 0u, TEST_LOCATION);

  impl.SetInputPanelLayout(Dali::InputMethod::PanelLayout::NUMBER);
  impl.SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType::SEARCH);
  impl.SetReturnKeyState(false);
  impl.SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage::ALPHABET);
  impl.NotifyTextInputMultiLine(true);
  impl.SetInputPanelPosition(11u, 22u);

  // None of it reached the bridge: it is not registered yet.
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastLayout),
                   static_cast<int>(Dali::InputMethod::PanelLayout::NORMAL),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.returnKeyCount, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.panelPositionCount, 0, TEST_LOCATION);

  Dali::Integration::RegisterImeBridge(&bridge);

  DALI_TEST_EQUALS(bridge.attached.size(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastLayout),
                   static_cast<int>(Dali::InputMethod::PanelLayout::NUMBER),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastReturnKeyType),
                   static_cast<int>(Dali::InputMethod::ReturnKeyType::SEARCH),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastReturnKeyEnabled, false, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastLanguage),
                   static_cast<int>(Dali::Integration::InputMethodContext::InputPanelLanguage::ALPHABET),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastMultiLine, true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.panelPositionCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastPanelPositionX, 11, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastPanelPositionY, 22, TEST_LOCATION);

  // D3/I2: with no bridge the context must report prediction as disallowed,
  // and must not tell a newly attached bridge otherwise.
  DALI_TEST_EQUALS(bridge.lastTextPrediction, false, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextForwardsPanelPosition(void)
{
  // Regression test for review finding I8: SetInputPanelPosition() used to
  // fall through to the base's unconditional `true` while nothing happened
  // and the bridge was never told, unlike its sibling
  // SetInputPanelPositionAlign().
  RecordingBridge bridge;

  {
    // No bridge: report the failure rather than claiming success.
    auto  unbridged = MakeBridgeContext();
    auto& impl      = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*unbridged);
    DALI_TEST_EQUALS(impl.SetInputPanelPosition(1u, 2u), false, TEST_LOCATION);
  }

  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  DALI_TEST_EQUALS(impl.SetInputPanelPosition(30u, 40u), true, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.panelPositionCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastPanelPositionX, 30, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.lastPanelPositionY, 40, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextLanguageAndKeyboardTypeCallbacks(void)
{
  // Regression test for the review's dead-signal enumeration: without
  // ImeBridgeHost::InputPanelLanguageChanged()/KeyboardTypeChanged() the IME
  // could not report either event, so LanguageChangedSignal and
  // KeyboardTypeChangedSignal (and their legacy counterparts) were
  // permanently dead, GetKeyboardType() was hardcoded, and
  // GetInputPanelLanguage() only echoed the application. Like the two
  // pre-existing host callbacks these are ungated, so no Adaptor is needed.
  gLanguageChangedFired     = false;
  gKeyboardTypeChangedFired = false;

  auto context = MakeBridgeContext();

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  impl.LanguageChangedSignal().Connect(&RecordLanguageChanged);
  impl.KeyboardTypeChangedSignal().Connect(&RecordKeyboardTypeChanged);

  // The default is what the application last set, i.e. AUTOMATIC.
  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelLanguage()),
                   static_cast<int>(Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC),
                   TEST_LOCATION);

  host.InputPanelLanguageChanged(Dali::Integration::InputMethodContext::InputPanelLanguage::ALPHABET);

  DALI_TEST_CHECK(gLanguageChangedFired);
  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelLanguage()),
                   static_cast<int>(Dali::Integration::InputMethodContext::InputPanelLanguage::ALPHABET),
                   TEST_LOCATION);

  DALI_TEST_EQUALS(static_cast<int>(impl.GetKeyboardType()),
                   static_cast<int>(Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD),
                   TEST_LOCATION);

  host.KeyboardTypeChanged(Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD);

  DALI_TEST_CHECK(gKeyboardTypeChangedFired);
  DALI_TEST_EQUALS(static_cast<int>(gLastKeyboardType),
                   static_cast<int>(Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(impl.GetKeyboardType()),
                   static_cast<int>(Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD),
                   TEST_LOCATION);

  END_TEST;
}

int UtcDaliImeBridgeContextOnBridgeDetachedResetsPanelState(void)
{
  // Regression test for review finding I6: unregistering the bridge used to
  // leave the context reporting a vanished input panel as visible, with
  // geometry, for ever and with no StatusChangedSignal.
  RecordingBridge bridge; // declared before the context (M5)

  Dali::Integration::RegisterImeBridge(&bridge);

  auto context = MakeBridgeContext();

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  host.InputPanelGeometryChanged(Dali::BoundsInteger(0, 100, 480, 300));
  host.InputPanelStateChanged(Dali::InputMethodContext::State::SHOW);

  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelState()),
                   static_cast<int>(Dali::InputMethodContext::State::SHOW),
                   TEST_LOCATION);

  // Connect only now, so the assertions below cannot be satisfied by the
  // emits that set the state up.
  gStatusChangedFired     = false;
  gLastStatusChangedState = Dali::InputMethodContext::State::SHOW;
  gResizedFired           = false;
  impl.StatusChangedSignal().Connect(&RecordStatusChanged);
  impl.ResizedSignal().Connect(&RecordResized);

  Dali::Integration::UnregisterImeBridge(&bridge);

  DALI_TEST_EQUALS(bridge.attached.size(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelState()),
                   static_cast<int>(Dali::InputMethodContext::State::HIDE),
                   TEST_LOCATION);

  const Dali::BoundsInteger area = impl.GetInputPanelArea();
  DALI_TEST_EQUALS(area.x, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(area.y, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(area.width, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(area.height, 0, TEST_LOCATION);

  DALI_TEST_CHECK(gResizedFired);
  DALI_TEST_CHECK(gStatusChangedFired);
  DALI_TEST_EQUALS(static_cast<int>(gLastStatusChangedState),
                   static_cast<int>(Dali::InputMethodContext::State::HIDE),
                   TEST_LOCATION);

  END_TEST;
}

int UtcDaliImeBridgeContextWithoutBridgeIsSafe(void)
{
  // D3: Without a bridge, behave like the generic backend.
  auto  context = MakeBridgeContext();
  auto& impl    = static_cast<Dali::Internal::Adaptor::InputMethodContext&>(*context);

  DALI_TEST_EQUALS(impl.ShowInputPanel(), false, TEST_LOCATION);
  DALI_TEST_EQUALS(impl.HideInputPanel(), false, TEST_LOCATION);

  Dali::KeyEvent keyEvent;
  DALI_TEST_EQUALS(impl.FilterEventKey(keyEvent), false, TEST_LOCATION);

  DALI_TEST_EQUALS(static_cast<int>(impl.GetInputPanelState()),
                   static_cast<int>(Dali::InputMethodContext::State::HIDE),
                   TEST_LOCATION);

  const Dali::BoundsInteger area = impl.GetInputPanelArea();
  DALI_TEST_EQUALS(area.width, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(area.height, 0, TEST_LOCATION);

  END_TEST;
}

// ===========================================================================
// IME -> application direction.
//
// Ten of the sixteen ImeBridgeHost methods early-return unless
// Dali::Adaptor::IsAvailable(), so the tests from here down stand up an
// AdaptorTestApplication and go through the production factory,
// InputMethodContextBridge::New(), which needs the same thing plus a non-empty
// Actor (input-method-context-impl-bridge.cpp:34-54). Each test case runs in its
// own forked process (test-harness.cpp:312-345), so the one-way
// Adaptor::SetAvailable() cannot leak into the Adaptor-free tests above.
// ===========================================================================

int UtcDaliImeBridgeContextActivateEmitsActivatedSignal(void)
{
  // Regression test for review finding I3: Activate() only forwarded to the
  // bridge, so Dali::InputMethodContext::ActivatedSignal() -- public API, and
  // emitted from Activate() by four of the five platform backends -- could
  // never fire in the simulator.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context); // New() returns an empty pointer with no Adaptor

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;

  ResetHostRecorders();
  impl.ActivatedSignal().Connect(&RecordActivated);

  DALI_TEST_EQUALS(bridge.activateCount, 0, TEST_LOCATION);
  DALI_TEST_CHECK(!gActivatedFired);

  impl.Activate();

  DALI_TEST_EQUALS(bridge.activateCount, 1, TEST_LOCATION); // still forwarded
  DALI_TEST_CHECK(gActivatedFired);                         // and now emitted

  // Deactivate() is forwarded and emits nothing, as on every backend.
  impl.Deactivate();
  DALI_TEST_EQUALS(bridge.deactivateCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextCommitPushesReturnedCursorAndTextToBridge(void)
{
  // Regression test for review finding I1: NotifyCursorPosition() had an empty
  // body, and it is the only thing Commit() calls after writing the
  // application's post-commit cursor and text back from the returned
  // CallbackData -- so those values were absorbed into the cache and dropped,
  // leaving the IME composing against a stale cursor after every commit.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);
  gCallbackDataToReturn = Dali::Integration::InputMethodContext::CallbackData(
    true /* update */, 7 /* cursorPosition */, Dali::String("hello world"), false /* preeditResetRequired */);

  host.Commit(Dali::String("hello"));

  // The application saw a COMMIT carrying the committed text, on both signals.
  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::COMMIT),
                   TEST_LOCATION);
  DALI_TEST_CHECK(gLastPredictiveString == "hello");

  // ...and what it returned reached the IME. This is the I1 assertion: it
  // fails if NotifyCursorPosition() is emptied again.
  DALI_TEST_EQUALS(bridge.cursorPositionCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastCursorPosition), 7, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.surroundingTextCount, 1, TEST_LOCATION);
  DALI_TEST_CHECK(bridge.lastSurroundingText == "hello world");

  // The cache the application can read back agrees with what was pushed.
  DALI_TEST_EQUALS(impl.GetCursorPosition(), 7u, TEST_LOCATION);
  DALI_TEST_CHECK(impl.GetSurroundingText() == "hello world");

  // update == false must push nothing, and an empty commit must not even
  // reach the application (input-method-context-impl-bridge.cpp:503-507).
  gCallbackDataToReturn = Dali::Integration::InputMethodContext::CallbackData();
  host.Commit(Dali::String("again"));
  DALI_TEST_EQUALS(gKeyboardEventCount, 2, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.cursorPositionCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.surroundingTextCount, 1, TEST_LOCATION);

  host.Commit(Dali::String());
  DALI_TEST_EQUALS(gEventReceivedCount, 2, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 2, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextPreEditDeliversTextStyleAndResetRequest(void)
{
  // PreEdit() carries the composing string and its cursor, caches the style
  // runs for GetPreeditStyle(), pushes the returned cursor/text back to the
  // IME (I1, same path as Commit()) and honours preeditResetRequired by
  // calling Reset() on the bridge.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);
  gCallbackDataToReturn = Dali::Integration::InputMethodContext::CallbackData(
    true /* update */, 2 /* cursorPosition */, Dali::String("ab"), true /* preeditResetRequired */);

  Dali::Integration::InputMethodContext::PreEditAttributeDataContainer attrs;
  Dali::Integration::InputMethodContext::PreeditAttributeData          run;
  run.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::UNDERLINE;
  run.startIndex  = 0u;
  run.endIndex    = 2u;
  attrs.PushBack(run);

  host.PreEdit(Dali::String("ab"), 2, attrs);

  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::PRE_EDIT),
                   TEST_LOCATION);
  DALI_TEST_CHECK(gLastPredictiveString == "ab");
  DALI_TEST_EQUALS(gLastCursorOffset, 2, TEST_LOCATION);

  // The style runs are readable back through GetPreeditStyle().
  Dali::Integration::InputMethodContext::PreEditAttributeDataContainer readBack;
  impl.GetPreeditStyle(readBack);
  DALI_TEST_EQUALS(readBack.Count(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(readBack[0].preeditType),
                   static_cast<int>(Dali::Integration::InputMethodContext::PreeditStyle::UNDERLINE),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(readBack[0].endIndex, 2u, TEST_LOCATION);

  // I1 again: the returned cursor/text reached the IME.
  DALI_TEST_EQUALS(bridge.cursorPositionCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(bridge.lastCursorPosition), 2, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.surroundingTextCount, 1, TEST_LOCATION);
  DALI_TEST_CHECK(bridge.lastSurroundingText == "ab");

  // preeditResetRequired asks the IME to abandon the composition.
  DALI_TEST_EQUALS(bridge.resetCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextPreEditStartAndEndDeliverEmptyPreEdit(void)
{
  // Regression test for review finding I4: PreEditEnd() used to send
  // EventType::VOID, which the toolkit's text controller handles with
  // "do nothing", so a composition the IME abandoned without committing was
  // never cleared. It must deliver a PRE_EDIT carrying an empty string, which
  // is what removes the pre-edit run. Asserted on the EventData the handler
  // actually receives, not on the toolkit.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);

  host.PreEditStart();

  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::PRE_EDIT),
                   TEST_LOCATION);
  DALI_TEST_CHECK(gLastPredictiveString.Empty());

  // Now the one that was broken. Re-arm so the assertions cannot be satisfied
  // by PreEditStart()'s event above.
  ResetHostRecorders();

  host.PreEditEnd();

  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::PRE_EDIT),
                   TEST_LOCATION); // I4: was VOID
  DALI_TEST_CHECK(gLastPredictiveString.Empty());

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextRetrieveSurrounding(void)
{
  // The one documented-synchronous host method (ime-bridge.h:108-118): the IME
  // waits on the reply, so both the "application declined" and the
  // "application answered" contracts matter.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);

  // update == false: no result, and the out parameters are left alone.
  Dali::String text("untouched");
  int32_t      cursorPosition = -1;

  DALI_TEST_EQUALS(host.RetrieveSurrounding(text, cursorPosition), false, TEST_LOCATION);
  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::GET_SURROUNDING),
                   TEST_LOCATION);
  DALI_TEST_CHECK(text == "untouched");
  DALI_TEST_EQUALS(cursorPosition, -1, TEST_LOCATION);

  // update == true: the reply is handed back and cached.
  gCallbackDataToReturn = Dali::Integration::InputMethodContext::CallbackData(
    true, 3, Dali::String("abcdef"), false);

  DALI_TEST_EQUALS(host.RetrieveSurrounding(text, cursorPosition), true, TEST_LOCATION);
  DALI_TEST_CHECK(text == "abcdef");
  DALI_TEST_EQUALS(cursorPosition, 3, TEST_LOCATION);
  DALI_TEST_CHECK(impl.GetSurroundingText() == "abcdef");
  DALI_TEST_EQUALS(impl.GetCursorPosition(), 3u, TEST_LOCATION);

  // Reading has no side effects on the IME: unlike Commit()/PreEdit() this
  // must not push anything back down the bridge (ime-bridge.h:110-113).
  DALI_TEST_EQUALS(bridge.cursorPositionCount, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(bridge.surroundingTextCount, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextDeleteSurroundingAndSetSelection(void)
{
  // Two host methods whose whole payload is the numbers they carry, so a
  // mis-ordered EventData constructor would be invisible without this.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);

  host.DeleteSurrounding(-2, 3);

  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::DELETE_SURROUNDING),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(gLastCursorOffset, -2, TEST_LOCATION);
  DALI_TEST_EQUALS(gLastNumberOfChars, 3, TEST_LOCATION);

  ResetHostRecorders();

  host.SetSelection(4, 9);

  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::SELECTION_SET),
                   TEST_LOCATION);
  DALI_TEST_EQUALS(gLastStartIndex, 4, TEST_LOCATION);
  DALI_TEST_EQUALS(gLastEndIndex, 9, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextSendPrivateCommandEmitsBetweenTheEventSignals(void)
{
  // Covers review round 1 finding I2, which added
  // mPrivateCommandReceivedSignal: both Tizen backends emit it *between* the
  // two low-level event signals, and that ordering is what the assertions on
  // the counters captured at emit time check.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);
  impl.PrivateCommandReceivedSignal().Connect(&RecordPrivateCommand);

  host.SendPrivateCommand(Dali::String("ise.command.42"));

  DALI_TEST_EQUALS(gEventReceivedCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<int>(gLastEventName),
                   static_cast<int>(Dali::Integration::InputMethodContext::PRIVATE_COMMAND),
                   TEST_LOCATION);

  DALI_TEST_CHECK(gPrivateCommandFired);
  DALI_TEST_CHECK(gLastPrivateCommand == "ise.command.42");

  // Ordering: after EventReceivedSignal, before KeyboardEventReceivedSignal.
  DALI_TEST_EQUALS(gEventCountWhenPrivateCommandRan, 1, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardCountWhenPrivateCommandRan, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextCommitContentEmitsContentReceived(void)
{
  // CommitContent() delivers non-text content and must not go through the
  // EventData path at all -- it raises ContentReceivedSignal only.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);
  impl.ContentReceivedSignal().Connect(&RecordContentReceived);

  host.CommitContent(Dali::String("file:///tmp/sticker.png"),
                     Dali::String("a sticker"),
                     Dali::String("image/png"));

  DALI_TEST_CHECK(gContentReceivedFired);
  DALI_TEST_CHECK(gLastContentUri == "file:///tmp/sticker.png");
  DALI_TEST_CHECK(gLastContentDescription == "a sticker");
  DALI_TEST_CHECK(gLastContentMimeTypes == "image/png");

  // Not an EventData event.
  DALI_TEST_EQUALS(gEventReceivedCount, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextUserDataIsReadFromTheIme(void)
{
  // GetInputPanelUserData() must report what the IME holds, not echo the last
  // SetInputPanelUserData(): the IME is free to revise the data it was given,
  // and every other backend pulls rather than reading a cache.
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = *context;

  const int userDataSetCountBefore = bridge.userDataSetCount;
  DALI_TEST_CHECK(impl.SetInputPanelUserData(Dali::String("app-asked-for-this")));
  DALI_TEST_EQUALS(bridge.userDataSetCount, userDataSetCountBefore + 1, TEST_LOCATION);
  DALI_TEST_CHECK(bridge.lastUserDataSet == "app-asked-for-this");

  bridge.userDataToReturn = Dali::String("ime-actually-has-this");

  DALI_TEST_CHECK(impl.GetInputPanelUserData() == "ime-actually-has-this");
  DALI_TEST_EQUALS(bridge.userDataGetCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}

int UtcDaliImeBridgeContextUserDataFallsBackToTheCache(void)
{
  // An IME that registered no imdata-requested handler cannot answer at all.
  // The value the application set is then the only thing known, and is what
  // this backend reported before it started asking.
  RecordingBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);

  auto  context = MakeBridgeContext();
  auto& impl    = *context;

  impl.SetInputPanelUserData(Dali::String("cached"));
  bridge.userDataToReturn = Dali::String(); // the IME has no answer

  DALI_TEST_CHECK(impl.GetInputPanelUserData() == "cached");
  DALI_TEST_EQUALS(bridge.userDataGetCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);

  // With no bridge at all there is nothing to ask, and the cache still answers.
  DALI_TEST_CHECK(impl.GetInputPanelUserData() == "cached");
  DALI_TEST_EQUALS(bridge.userDataGetCount, 1, TEST_LOCATION);
  END_TEST;
}

int UtcDaliImeBridgeContextRecaptureStringIsNotDecomposed(void)
{
  // RecaptureString() replaces a range atomically. This backend cannot do
  // that -- EventType has no such event -- and must drop it rather than
  // synthesise one from DELETE_SURROUNDING plus COMMIT, which would let the
  // application observe the range deleted and not yet replaced.
  AdaptorTestApplication application;
  RecordingBridge        bridge;

  Dali::Integration::RegisterImeBridge(&bridge);

  Dali::Actor actor   = Dali::Actor::New();
  auto        context = Dali::Internal::Adaptor::InputMethodContextBridge::New(actor);
  DALI_TEST_CHECK(context);

  auto&                                        sim  = static_cast<Dali::Internal::Adaptor::InputMethodContextBridge&>(*context);
  Dali::Internal::Adaptor::InputMethodContext& impl = sim;
  Dali::Integration::ImeBridgeHost&            host = sim;

  ArmHostRecorders(impl, bridge);

  Dali::Integration::InputMethodContext::PreEditAttributeDataContainer attrs;
  host.RecaptureString(-3, 3, Dali::String("pre"), Dali::String("commit"), attrs);

  DALI_TEST_EQUALS(gEventReceivedCount, 0, TEST_LOCATION);
  DALI_TEST_EQUALS(gKeyboardEventCount, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  END_TEST;
}
