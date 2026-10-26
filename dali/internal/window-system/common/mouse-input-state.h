#ifndef DALI_INTERNAL_WINDOW_SYSTEM_COMMON_MOUSE_INPUT_STATE_H
#define DALI_INTERNAL_WINDOW_SYSTEM_COMMON_MOUSE_INPUT_STATE_H

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
#include <dali/integration-api/events/point.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/dali-namespace.h>
#include <cstdint>
#include <limits>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Tracks native mouse input sequences across input mode changes.
 *
 * Button masks are supplied by the native backend. A button held when input is
 * enabled cannot resume a cancelled drag or generate an unmatched release.
 * Access is confined to the event thread.
 */
class MouseInputState
{
public:
  static constexpr uint32_t UNKNOWN_BUTTONS = std::numeric_limits<uint32_t>::max();

  MouseInputState()                                          = default;
  ~MouseInputState()                                         = default;
  MouseInputState(const MouseInputState& rhs)                = default;
  MouseInputState& operator=(const MouseInputState& rhs)     = default;
  MouseInputState(MouseInputState&& rhs) noexcept            = default;
  MouseInputState& operator=(MouseInputState&& rhs) noexcept = default;

  /**
   * @brief Changes input mode and returns whether the mode changed.
   * @param[in] enabled Whether native mouse input is enabled
   * @param[in] pressedButtons The native buttons currently held down
   * @return True if the mode changed
   */
  bool SetEnabled(bool enabled, uint32_t pressedButtons)
  {
    if(mEnabled == enabled)
    {
      return false;
    }

    mSuppressedButtons |= mPressedButtons;
    if(enabled && pressedButtons != UNKNOWN_BUTTONS)
    {
      mSuppressedButtons = pressedButtons;
    }
    mEnabled                  = enabled;
    mPressedButtons           = 0u;
    mVerticalWheelRemainder   = 0;
    mHorizontalWheelRemainder = 0;
    return true;
  }

  /**
   * @brief Returns whether native mouse input is enabled.
   * @return The enabled state
   */
  bool IsEnabled() const
  {
    return mEnabled;
  }

  /**
   * @brief Starts a button sequence if input is enabled.
   * @param[in] button The native button mask
   * @return Whether the press should be forwarded
   */
  bool OnButtonDown(uint32_t button)
  {
    if(!mEnabled)
    {
      mSuppressedButtons |= button;
      return false;
    }

    mSuppressedButtons &= ~button;
    mPressedButtons |= button;
    return true;
  }

  /**
   * @brief Finishes a button sequence only if its press was forwarded.
   * @param[in] button The native button mask
   * @return Whether the release should be forwarded
   */
  bool OnButtonUp(uint32_t button)
  {
    const bool forward = mEnabled && (mPressedButtons & button) != 0u;
    mPressedButtons &= ~button;
    mSuppressedButtons &= ~button;
    return forward;
  }

  /**
   * @brief Filters motion from button sequences interrupted by a mode change.
   * @param[in] pressedButtons The native buttons held for this motion
   * @return Whether the motion should be forwarded
   */
  bool OnMotion(uint32_t pressedButtons)
  {
    if(pressedButtons != UNKNOWN_BUTTONS)
    {
      mSuppressedButtons &= pressedButtons;
    }
    return mEnabled && mSuppressedButtons == 0u;
  }

  /**
   * @brief Updates held buttons using a native snapshot when input is enabled.
   * @param[in] pressedButtons The normalized buttons currently held down
   */
  void SetSuppressedButtons(uint32_t pressedButtons)
  {
    mSuppressedButtons = pressedButtons;
  }

  /**
   * @brief Converts a DALi mouse button to a shared button mask.
   * @param[in] button The button identifier
   * @return The button mask, or zero for an invalid button
   */
  static uint32_t GetButtonMask(MouseButton::Type button)
  {
    const int32_t index = static_cast<int32_t>(button);
    return index > 0 && index < 32 ? 1u << index : 0u;
  }

  /**
   * @brief Filters native points from every device and remembers the last forwarded point.
   * @param[in] point The native point
   * @param[in] pressedButtons A normalized native button snapshot, if available
   * @return Whether the point should be forwarded
   */
  bool OnPoint(const Integration::Point& point, uint32_t pressedButtons = UNKNOWN_BUTTONS)
  {
    bool forward = mEnabled;
    if(point.GetState() == PointState::INTERRUPTED)
    {
      mPressedButtons    = 0u;
      mSuppressedButtons = 0u;
    }
    else if(point.GetDeviceClass() == Device::Class::MOUSE)
    {
      // Button bookkeeping is mouse-specific; the enabled state applies to all devices.
      switch(point.GetState())
      {
        case PointState::DOWN:
        {
          forward = OnButtonDown(GetButtonMask(point.GetMouseButton()));
          break;
        }
        case PointState::UP:
        {
          forward = OnButtonUp(GetButtonMask(point.GetMouseButton()));
          break;
        }
        case PointState::MOTION:
        {
          forward = OnMotion(pressedButtons);
          break;
        }
        default:
        {
          break;
        }
      }
    }
    if(forward)
    {
      mLastPoint = point;
      mHasPoint  = point.GetState() != PointState::INTERRUPTED;
    }
    return forward;
  }

  /**
   * @brief Takes a terminal point for the last forwarded native input sequence.
   * @param[out] point The interruption point
   * @return Whether a native point was available
   */
  bool TakeInterruptedPoint(Integration::Point& point)
  {
    if(!mHasPoint)
    {
      return false;
    }
    point = mLastPoint;
    point.SetState(PointState::INTERRUPTED);
    point.SetPressure(0.0f);
    mHasPoint = false;
    return true;
  }

  /**
   * @brief Accumulates wheel motion only while input is enabled.
   * @param[in] delta The native delta, with its sign converted to DALi coordinates
   * @param[in] horizontal Whether the delta belongs to the horizontal wheel
   * @param[in] deltaPerStep The native delta corresponding to one wheel step
   * @return The number of complete wheel steps to forward
   */
  int32_t OnWheel(int32_t delta, bool horizontal, int32_t deltaPerStep)
  {
    DALI_ASSERT_DEBUG(deltaPerStep > 0);
    if(!mEnabled)
    {
      return 0;
    }

    int32_t& remainder = horizontal ? mHorizontalWheelRemainder : mVerticalWheelRemainder;
    remainder += delta;
    const int32_t steps = remainder / deltaPerStep;
    remainder %= deltaPerStep;
    return steps;
  }

private:
  Integration::Point mLastPoint;
  bool               mHasPoint{false};
  int32_t            mVerticalWheelRemainder{0};
  int32_t            mHorizontalWheelRemainder{0};
  uint32_t           mPressedButtons{0u};
  uint32_t           mSuppressedButtons{0u};
  bool               mEnabled{true};
};

} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_WINDOW_SYSTEM_COMMON_MOUSE_INPUT_STATE_H
