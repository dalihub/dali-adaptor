#ifndef __DALI_ACCESSIBILITY_MANAGER_H__
#define __DALI_ACCESSIBILITY_MANAGER_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <boost/function.hpp>

#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal-v2.h>
#include <dali/public-api/events/touch-event.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class AccessibilityManager;
}
}

class AccessibilityActionHandler;
class AccessibilityGestureHandler;
class TouchPoint;

/**
 * @brief The AccessibilityManager provides signals when accessibility & screen reader feature turned on in device.
 */
class DALI_IMPORT_API AccessibilityManager : public BaseHandle
{
public:

  // Typedefs

  /**
   * @brief Accessibility Action Signal.
   *
   * Signal connected callback should return the result
   */
  typedef SignalV2< bool ( AccessibilityManager& ) > AccessibilityActionSignalV2; ///< Generic signal type
  typedef SignalV2< bool (AccessibilityManager&, const Dali::TouchEvent&)> AccessibilityActionScrollSignalV2; ///< Scroll signal type

  /**
   * @brief Create an uninitialized handle.
   *
   * This can be initialized by calling getting the manager from Dali::Adaptor.
   */
  AccessibilityManager();

  /**
   * @brief Retrieve a handle to the AccessibilityManager.
   *
   * @return A handle to the AccessibilityManager.
   */
  static AccessibilityManager Get();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~AccessibilityManager();

  /**
   * @brief Returns the current position of the read action.
   * @return The current event position.
   */
  Vector2 GetReadPosition() const;

  /**
   * @brief Query whether the accessibility(screen-reader) is enabled.
   *
   * The accessibility will be enabled by system setting.
   * @return True if the accessibility(screen-reader) is enabled.
   */
  bool IsEnabled() const;

  /**
   * @brief Set the handler to handle accessibility actions.
   *
   * @param[in] handler The Accessibility action handler.
   * @note Handlers should remove themselves when they are destroyed.
   */
  void SetActionHandler(AccessibilityActionHandler& handler);

  /**
   * @brief Set the handler to handle accessibility gestures.
   *
   * @param[in] handler The Accessibility gesture handler.
   * @note Handlers should remove themselves when they are destroyed.
   */
  void SetGestureHandler(AccessibilityGestureHandler& handler);

  /**
   * @brief Handle the accessibility action to move focus to the next focusable actor
   * (by one finger flick down).
   *
   * @param allowEndFeedback true if end of list feedback should be played when the focus is alread reached to the end
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionNextEvent(bool allowEndFeedback = true);

  /**
   * @brief Handle the accessibility action to move focus to the previous focusable actor
   * (by one finger flick up).
   *
   * @param allowEndFeedback true if end of list feedback should be played when the focus is alread reached to the end
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionPreviousEvent(bool allowEndFeedback = true);

  /**
   * @brief Handle the accessibility action to activate the current focused actor (by one
   * finger double tap)
   *
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionActivateEvent();

  /**
   * @brief Handle the accessibility action to focus and read the actor (by one finger tap or move).
   *
   * @param x x position of event
   * @param y y position of event
   * @param allowReadAgain true if the action read again the same object (i.e. read action)
   *                       false if the action just read when the focus object is changed (i.e. over action)
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionReadEvent(unsigned int x, unsigned int y, bool allowReadAgain);

  /**
   * @brief Handle the accessibility action to move focus to the next focusable actor
   * (by one finger flick right).
   *
   * @param allowEndFeedback true if end of list feedback should be played when the focus is alread reached to the end
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionReadNextEvent(bool allowEndFeedback = true);

  /**
   * @brief Handle the accessibility action to move focus to the previous focusable actor
   * (by one finger flick up).
   *
   * @param allowEndFeedback true if end of list feedback should be played when the focus is alread reached to the front
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionReadPreviousEvent(bool allowEndFeedback = true);

  /**
   * @brief Handle the accessibility action to change the value when the current focused
   * actor is a slider (by double finger down and move up and right).
   *
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionUpEvent();

  /**
   * @brief Handle the accessibility action to change the value when the current focused
   * actor is a slider (by double finger down and move down and left).
   *
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionDownEvent();

  /**
   * @brief Handle the accessibility action to clear the focus from the current focused
   * actor if any, so that no actor is focused in the focus chain.
   *
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionClearFocusEvent();

  /**
   * @brief Handle the accessibility action to scroll when there is a scroller on the touched position
   * (by 2 finger touch & move, 2 finger flick).
   *
   * @param[in]  point      The touch point information.
   * @param[in]  timeStamp  The time the touch occurred.
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionScrollEvent(const TouchPoint& point, unsigned long timeStamp);

  /**
   * @brief Handle the accessibility action to move for the current focused actor
   * (by 1 finger tap & hold and move).
   *
   * @param[in]  point      The touch point information.
   * @param[in]  timeStamp  The time the touch occurred.
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionTouchEvent(const TouchPoint& point, unsigned long timeStamp);

  /**
   * @brief Handle the accessibility action to navigate back (by two fingers circle draw).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionBackEvent();

  /**
   * @brief Handle the accessibility action to enable the feature.
   */
  void HandleActionEnableEvent();

  /**
   * @brief Handle the accessibility action to disable the feature.
   */
  void HandleActionDisableEvent();

public:  // Signals

  /**
   * @brief This is emitted when accessibility(screen-reader) feature turned on or off.
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& StatusChangedSignal();

  /**
   * @brief This is emitted when accessibility action is received to move focus to the next
   * focusable actor (by one finger flick down).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionNextSignal();

  /**
   * @brief This is emitted when accessibility action is received to move focus to the previous
   * focusable actor (by one finger flick up).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionPreviousSignal();

  /**
   * @brief This is emitted when accessibility action is received to activate the current focused
   * actor (by one finger double tap).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionActivateSignal();

  /**
   * @brief This is emitted when accessibility action is received to focus and read the actor
   * (by one finger tap).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionReadSignal();

  /**
   * @brief This is emitted when accessibility action is received to focus and read the actor
   * (by one finger move).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionOverSignal();

  /**
   * @brief This is emitted when accessibility action is received to move focus to the next
   * focusable actor (by one finger flick right).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionReadNextSignal();

  /**
   * @brief This is emitted when accessibility action is received to move focus to the previous
   * focusable actor (by one finger flick left).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionReadPreviousSignal();

  /**
   * @brief This is emitted when accessibility action is received to change the value when the
   * current focused actor is a slider (by double finger down and move up and right).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionUpSignal();

  /**
   * @brief This is emitted when accessibility action is received to change the value when the
   * current focused actor is a slider (by double finger down and move down and left).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionDownSignal();

  /**
   * @brief This is emitted when accessibility action is received to clear the focus from the
   * current focused actor if any, so that no actor is focused in the focus chain.
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionClearFocusSignal();

  /**
   * @brief This is emitted when accessibility action is received to navigate back (by two
   * fingers circle draw).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionBackSignal();

  /**
   * @brief This is emitted when accessibility action is received to handle scroll event (by two
   * fingers drag).
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager, const TouchEvent& event );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionScrollSignalV2& ActionScrollSignal();

public: // Not intended for application developers

  /**
   * @brief Creates a handle using the Adaptor::Internal implementation.
   *
   * @param[in] manager The AccessibilityManager implementation.
   */
  DALI_INTERNAL AccessibilityManager( Internal::Adaptor::AccessibilityManager& manager );

  /**
   * @brief This constructor is used by AccessibilityManager::Get().
   *
   * @param[in] manager A pointer to the accessibility manager.
   */
  explicit DALI_INTERNAL AccessibilityManager( Internal::Adaptor::AccessibilityManager* manager );
};

} // namespace Dali

#endif // __DALI_ACCESSIBILITY_MANAGER_H__
