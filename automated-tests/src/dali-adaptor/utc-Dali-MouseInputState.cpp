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

// EXTERNAL INCLUDES
#include <dali-test-suite-utils.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/touch-event-combiner.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/mouse-input-state.h>

using Dali::Internal::Adaptor::MouseInputState;

void utc_dali_mouse_input_state_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_mouse_input_state_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliMouseInputStateDisablesInput(void)
{
  MouseInputState state;
  DALI_TEST_CHECK(state.IsEnabled());
  DALI_TEST_CHECK(state.OnMotion(0u));
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  DALI_TEST_CHECK(state.OnButtonUp(1u));

  DALI_TEST_CHECK(state.SetEnabled(false, 0u));
  DALI_TEST_CHECK(!state.IsEnabled());
  DALI_TEST_CHECK(!state.OnMotion(0u));
  DALI_TEST_CHECK(!state.OnMotion(1u));
  DALI_TEST_CHECK(!state.OnButtonDown(1u));
  DALI_TEST_CHECK(!state.OnButtonUp(1u));

  DALI_TEST_CHECK(state.SetEnabled(true, 0u));
  DALI_TEST_CHECK(state.OnMotion(0u));
  DALI_TEST_CHECK(!state.OnButtonUp(1u));
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  DALI_TEST_CHECK(state.OnButtonUp(1u));
  END_TEST;
}

int UtcDaliMouseInputStateCancelsHeldButtons(void)
{
  MouseInputState state;
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  DALI_TEST_CHECK(state.SetEnabled(false, 1u));
  DALI_TEST_CHECK(state.SetEnabled(true, 1u | 2u));

  DALI_TEST_CHECK(!state.OnMotion(1u | 2u));
  DALI_TEST_CHECK(!state.OnButtonUp(1u));
  DALI_TEST_CHECK(!state.OnMotion(2u));
  DALI_TEST_CHECK(!state.OnButtonUp(2u));
  DALI_TEST_CHECK(state.OnMotion(0u));

  DALI_TEST_CHECK(state.OnButtonDown(2u));
  DALI_TEST_CHECK(state.OnMotion(2u));
  DALI_TEST_CHECK(state.OnButtonUp(2u));
  END_TEST;
}

int UtcDaliMouseInputStateObservesReleaseOutsideWindow(void)
{
  MouseInputState state;
  DALI_TEST_CHECK(state.SetEnabled(false, 0u));
  DALI_TEST_CHECK(state.SetEnabled(true, 1u));
  DALI_TEST_CHECK(!state.OnMotion(1u));

  // The button was released outside the window, so no button-up was received.
  DALI_TEST_CHECK(state.OnMotion(0u));
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  DALI_TEST_CHECK(state.OnMotion(1u));
  DALI_TEST_CHECK(state.OnButtonUp(1u));
  END_TEST;
}

int UtcDaliMouseInputStateRepeatedCallsPreserveSequences(void)
{
  MouseInputState state;
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  DALI_TEST_CHECK(!state.SetEnabled(true, 1u));
  DALI_TEST_CHECK(state.OnMotion(1u));
  DALI_TEST_CHECK(state.OnButtonUp(1u));

  DALI_TEST_CHECK(state.SetEnabled(false, 0u));
  DALI_TEST_CHECK(!state.SetEnabled(false, 0u));
  DALI_TEST_CHECK(state.SetEnabled(true, 1u));
  DALI_TEST_CHECK(!state.SetEnabled(true, 0u));
  DALI_TEST_CHECK(!state.OnMotion(1u));
  DALI_TEST_CHECK(!state.OnButtonUp(1u));
  DALI_TEST_CHECK(state.OnMotion(0u));
  END_TEST;
}

int UtcDaliMouseInputStateWindowsAreIndependent(void)
{
  MouseInputState first;
  MouseInputState second;
  DALI_TEST_CHECK(first.OnButtonDown(1u));
  DALI_TEST_CHECK(second.OnButtonDown(2u));
  DALI_TEST_CHECK(first.SetEnabled(false, 0u));
  DALI_TEST_CHECK(!first.OnMotion(1u));
  DALI_TEST_CHECK(second.IsEnabled());
  DALI_TEST_CHECK(second.OnMotion(2u));
  DALI_TEST_CHECK(second.OnButtonUp(2u));
  END_TEST;
}

int UtcDaliMouseInputStateNewPressAfterSuppressedRelease(void)
{
  MouseInputState state;
  DALI_TEST_CHECK(state.SetEnabled(false, 0u));
  DALI_TEST_CHECK(state.SetEnabled(true, 1u | 2u));
  DALI_TEST_CHECK(!state.OnButtonUp(1u));
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  DALI_TEST_CHECK(!state.OnMotion(1u | 2u));
  DALI_TEST_CHECK(!state.OnButtonUp(2u));
  DALI_TEST_CHECK(state.OnMotion(1u));
  DALI_TEST_CHECK(state.OnButtonUp(1u));
  END_TEST;
}

int UtcDaliMouseInputInterruptionResetsCombiner(void)
{
  using namespace Dali;
  MouseInputState                 state;
  Integration::TouchEventCombiner combiner;
  Integration::TouchEvent         touch;
  Integration::HoverEvent         hover;
  Integration::Point              point;
  point.SetDeviceId(10);
  point.SetDeviceClass(Device::Class::MOUSE);
  point.SetScreenPosition(Vector2(20.0f, 30.0f));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(state.OnMotion(0u));
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(point, 10u, touch, hover), Integration::TouchEventCombiner::DISPATCH_HOVER, TEST_LOCATION);
  DALI_TEST_EQUALS(hover.points[0].GetState(), PointState::STARTED, TEST_LOCATION);

  point.SetState(PointState::INTERRUPTED);
  DALI_TEST_CHECK(state.SetEnabled(false, 0u));
  touch = Integration::TouchEvent();
  hover = Integration::HoverEvent();
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(point, 20u, touch, hover), Integration::TouchEventCombiner::DISPATCH_HOVER, TEST_LOCATION);
  DALI_TEST_EQUALS(hover.points[0].GetState(), PointState::INTERRUPTED, TEST_LOCATION);
  DALI_TEST_EQUALS(hover.points[0].GetDeviceClass(), Device::Class::MOUSE, TEST_LOCATION);

  DALI_TEST_CHECK(state.SetEnabled(true, 0u));
  DALI_TEST_CHECK(state.OnButtonDown(1u));
  point.SetState(PointState::DOWN);
  point.SetMouseButton(MouseButton::PRIMARY);
  touch = Integration::TouchEvent();
  hover = Integration::HoverEvent();
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(point, 30u, touch, hover), Integration::TouchEventCombiner::DISPATCH_TOUCH, TEST_LOCATION);

  DALI_TEST_CHECK(state.SetEnabled(false, 1u));
  point.SetState(PointState::INTERRUPTED);
  touch = Integration::TouchEvent();
  hover = Integration::HoverEvent();
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(point, 40u, touch, hover), Integration::TouchEventCombiner::DISPATCH_TOUCH, TEST_LOCATION);
  DALI_TEST_EQUALS(touch.points[0].GetState(), PointState::INTERRUPTED, TEST_LOCATION);
  DALI_TEST_CHECK(!state.OnButtonUp(1u));

  DALI_TEST_CHECK(state.SetEnabled(true, 0u));
  DALI_TEST_CHECK(state.OnMotion(0u));
  point.SetState(PointState::MOTION);
  point.SetMouseButton(MouseButton::INVALID);
  touch = Integration::TouchEvent();
  hover = Integration::HoverEvent();
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(point, 50u, touch, hover), Integration::TouchEventCombiner::DISPATCH_HOVER, TEST_LOCATION);
  DALI_TEST_EQUALS(hover.points[0].GetState(), PointState::STARTED, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMouseInputWheelDoesNotReplayAfterEnable(void)
{
  MouseInputState state;
  DALI_TEST_EQUALS(state.OnWheel(-60, false, 120), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(60, true, 120), 0, TEST_LOCATION);
  DALI_TEST_CHECK(state.SetEnabled(false, 0u));
  DALI_TEST_EQUALS(state.OnWheel(-240, false, 120), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(240, true, 120), 0, TEST_LOCATION);
  DALI_TEST_CHECK(state.SetEnabled(true, 0u));
  DALI_TEST_EQUALS(state.OnWheel(-60, false, 120), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(60, true, 120), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(-60, false, 120), -1, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(60, true, 120), 1, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMouseInputWheelPreservesAxesAndRepeatedEnable(void)
{
  MouseInputState state;
  DALI_TEST_EQUALS(state.OnWheel(60, true, 120), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(-60, false, 120), 0, TEST_LOCATION);
  DALI_TEST_CHECK(!state.SetEnabled(true, 0u));
  DALI_TEST_EQUALS(state.OnWheel(180, true, 120), 2, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(-180, false, 120), -2, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(-120, true, 120), -1, TEST_LOCATION);
  DALI_TEST_EQUALS(state.OnWheel(120, false, 120), 1, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMouseInputNativePointsKeepDeviceMetadata(void)
{
  using namespace Dali;
  MouseInputState    state;
  Integration::Point point;
  point.SetDeviceId(42);
  point.SetDeviceClass(Device::Class::MOUSE);
  point.SetScreenPosition(Vector2(20.0f, 30.0f));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(state.OnPoint(point));
  // Native-to-scene coordinate conversion must not alter the stored point.
  point.SetScreenPosition(Vector2(200.0f, 300.0f));
  DALI_TEST_CHECK(state.SetEnabled(false, MouseInputState::UNKNOWN_BUTTONS));
  Integration::Point interrupted;
  DALI_TEST_CHECK(state.TakeInterruptedPoint(interrupted));
  DALI_TEST_EQUALS(interrupted.GetDeviceId(), 42, TEST_LOCATION);
  DALI_TEST_EQUALS(interrupted.GetDeviceClass(), Device::Class::MOUSE, TEST_LOCATION);
  DALI_TEST_EQUALS(interrupted.GetState(), PointState::INTERRUPTED, TEST_LOCATION);
  DALI_TEST_EQUALS(interrupted.GetScreenPosition(), Vector2(20.0f, 30.0f), TEST_LOCATION);
  DALI_TEST_CHECK(!state.TakeInterruptedPoint(interrupted));
  DALI_TEST_CHECK(!state.OnPoint(point));

  DALI_TEST_CHECK(state.SetEnabled(true, MouseInputState::UNKNOWN_BUTTONS));
  DALI_TEST_CHECK(state.OnPoint(point));
  END_TEST;
}

int UtcDaliMouseInputBlocksAllDevicePoints(void)
{
  using namespace Dali;
  const Device::Class::Type devices[] = {Device::Class::MOUSE, Device::Class::TOUCH, Device::Class::PEN, Device::Class::NONE};
  for(Device::Class::Type device : devices)
  {
    MouseInputState    state;
    Integration::Point point;
    point.SetDeviceClass(device);
    point.SetState(PointState::DOWN);
    point.SetMouseButton(MouseButton::PRIMARY);
    DALI_TEST_CHECK(state.OnPoint(point));
    DALI_TEST_CHECK(state.SetEnabled(false, MouseInputState::UNKNOWN_BUTTONS));
    Integration::Point interrupted;
    DALI_TEST_CHECK(state.TakeInterruptedPoint(interrupted));
    DALI_TEST_EQUALS(interrupted.GetDeviceClass(), device, TEST_LOCATION);
    DALI_TEST_EQUALS(interrupted.GetState(), PointState::INTERRUPTED, TEST_LOCATION);
    DALI_TEST_CHECK(!state.TakeInterruptedPoint(interrupted));

    const PointState::Type blockedStates[] = {PointState::DOWN, PointState::MOTION, PointState::UP, PointState::LEAVE, PointState::INTERRUPTED};
    for(PointState::Type blocked : blockedStates)
    {
      point.SetState(blocked);
      DALI_TEST_CHECK(!state.OnPoint(point));
    }

    DALI_TEST_CHECK(state.SetEnabled(true, MouseInputState::UNKNOWN_BUTTONS));
    point.SetState(PointState::DOWN);
    DALI_TEST_CHECK(state.OnPoint(point));
    point.SetState(PointState::MOTION);
    DALI_TEST_CHECK(state.OnPoint(point));
    point.SetState(PointState::UP);
    DALI_TEST_CHECK(state.OnPoint(point));
  }
  END_TEST;
}

int UtcDaliMouseInputCancelsTouchAndHoverTogether(void)
{
  using namespace Dali;
  MouseInputState                 state;
  Integration::TouchEventCombiner combiner;
  Integration::Point              finger;
  finger.SetDeviceId(1);
  finger.SetDeviceClass(Device::Class::TOUCH);
  finger.SetState(PointState::DOWN);
  Integration::TouchEvent down;
  Integration::HoverEvent noHover;
  DALI_TEST_CHECK(state.OnPoint(finger));
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(finger, 10u, down, noHover), Integration::TouchEventCombiner::DISPATCH_TOUCH, TEST_LOCATION);

  Integration::Point mouse;
  mouse.SetDeviceId(2);
  mouse.SetDeviceClass(Device::Class::MOUSE);
  mouse.SetState(PointState::MOTION);
  DALI_TEST_CHECK(state.OnPoint(mouse));
  Integration::TouchEvent unchangedTouch;
  Integration::HoverEvent hover;
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(mouse, 20u, unchangedTouch, hover), Integration::TouchEventCombiner::DISPATCH_HOVER, TEST_LOCATION);

  DALI_TEST_CHECK(state.SetEnabled(false, MouseInputState::UNKNOWN_BUTTONS));
  Integration::Point interruption;
  DALI_TEST_CHECK(state.TakeInterruptedPoint(interruption));
  Integration::TouchEvent cancelledTouch;
  Integration::HoverEvent cancelledHover;
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(interruption, 30u, cancelledTouch, cancelledHover), Integration::TouchEventCombiner::DISPATCH_BOTH, TEST_LOCATION);
  DALI_TEST_EQUALS(cancelledTouch.points[0].GetState(), PointState::INTERRUPTED, TEST_LOCATION);
  DALI_TEST_EQUALS(cancelledHover.points[0].GetState(), PointState::INTERRUPTED, TEST_LOCATION);
  finger.SetState(PointState::MOTION);
  DALI_TEST_CHECK(!state.OnPoint(finger));
  finger.SetState(PointState::UP);
  DALI_TEST_CHECK(!state.OnPoint(finger));

  DALI_TEST_CHECK(state.SetEnabled(true, MouseInputState::UNKNOWN_BUTTONS));
  finger.SetState(PointState::DOWN);
  Integration::TouchEvent freshDown;
  Integration::HoverEvent freshHover;
  DALI_TEST_CHECK(state.OnPoint(finger));
  DALI_TEST_EQUALS(combiner.GetNextTouchEvent(finger, 40u, freshDown, freshHover), Integration::TouchEventCombiner::DISPATCH_TOUCH, TEST_LOCATION);
  END_TEST;
}

int UtcDaliMouseInputTracksButtonsWithoutNativeSnapshot(void)
{
  using namespace Dali;
  MouseInputState    state;
  Integration::Point point;
  point.SetDeviceClass(Device::Class::MOUSE);
  point.SetMouseButton(MouseButton::PRIMARY);
  point.SetState(PointState::DOWN);
  DALI_TEST_CHECK(state.OnPoint(point));
  DALI_TEST_CHECK(state.SetEnabled(false, MouseInputState::UNKNOWN_BUTTONS));
  Integration::Point interrupted;
  DALI_TEST_CHECK(state.TakeInterruptedPoint(interrupted));
  DALI_TEST_CHECK(state.SetEnabled(true, MouseInputState::UNKNOWN_BUTTONS));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(!state.OnPoint(point));
  point.SetState(PointState::UP);
  DALI_TEST_CHECK(!state.OnPoint(point));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(state.OnPoint(point));
  point.SetState(PointState::DOWN);
  DALI_TEST_CHECK(state.OnPoint(point));
  point.SetState(PointState::UP);
  DALI_TEST_CHECK(state.OnPoint(point));
  END_TEST;
}

int UtcDaliMouseInputObservesButtonsWhileDisabled(void)
{
  using namespace Dali;
  MouseInputState    state;
  Integration::Point point;
  point.SetDeviceClass(Device::Class::MOUSE);
  point.SetMouseButton(MouseButton::SECONDARY);
  DALI_TEST_CHECK(state.SetEnabled(false, MouseInputState::UNKNOWN_BUTTONS));
  point.SetState(PointState::DOWN);
  DALI_TEST_CHECK(!state.OnPoint(point));
  DALI_TEST_CHECK(state.SetEnabled(true, MouseInputState::UNKNOWN_BUTTONS));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(!state.OnPoint(point));
  point.SetState(PointState::UP);
  DALI_TEST_CHECK(!state.OnPoint(point));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(state.OnPoint(point));

  DALI_TEST_CHECK(state.SetEnabled(false, MouseInputState::UNKNOWN_BUTTONS));
  point.SetState(PointState::DOWN);
  DALI_TEST_CHECK(!state.OnPoint(point));
  point.SetState(PointState::INTERRUPTED);
  DALI_TEST_CHECK(!state.OnPoint(point));
  DALI_TEST_CHECK(state.SetEnabled(true, MouseInputState::UNKNOWN_BUTTONS));
  point.SetState(PointState::MOTION);
  DALI_TEST_CHECK(state.OnPoint(point));
  END_TEST;
}
