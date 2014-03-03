#ifndef __DALI_ACCESSIBILITY_MANAGER_H__
#define __DALI_ACCESSIBILITY_MANAGER_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//


/**
 * @addtogroup CAPI_DALI_FRAMEWORK
 * @{
 */

// EXTERNAL INCLUDES
#include <boost/function.hpp>

#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal-v2.h>

namespace Dali DALI_IMPORT_API
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
 * The AccessibilityManager provides signals when accessibility & screen reader feature turned on in device.
 */
class AccessibilityManager : public BaseHandle
{
public:

  // Typedefs

  /**
   * Accessibility Action Signal
   * Signal connected callback should return the result
   */
  typedef SignalV2< bool ( AccessibilityManager& ) > AccessibilityActionSignalV2;

  // Signal Names
  static const char* const SIGNAL_STATUS_CHANGED;
  static const char* const SIGNAL_ACTION_NEXT;
  static const char* const SIGNAL_ACTION_PREVIOUS;
  static const char* const SIGNAL_ACTION_ACTIVATE;
  static const char* const SIGNAL_ACTION_OVER;
  static const char* const SIGNAL_ACTION_READ;
  static const char* const SIGNAL_ACTION_READ_NEXT;
  static const char* const SIGNAL_ACTION_READ_PREVIOUS;
  static const char* const SIGNAL_ACTION_UP;
  static const char* const SIGNAL_ACTION_DOWN;
  static const char* const SIGNAL_ACTION_CLEAR_FOCUS;
  static const char* const SIGNAL_ACTION_BACK;

  /**
   * Create an uninitialized handle.
   * This can be initialized by calling getting the manager from Dali::Adaptor.
   */
  AccessibilityManager();

  /**
   * Retrieve a handle to the AccessibilityManager.
   * @return A handle to the AccessibilityManager.
   */
  static AccessibilityManager Get();

  /**
   * Virtual Destructor.
   */
  virtual ~AccessibilityManager();

  /**
   * Returns the current position of the read action.
   * @return The current event position.
   */
  Vector2 GetReadPosition() const;

  /**
   * Query whether the accessibility(screen-reader) is enabled.
   * The accessibility will be enabled by system setting.
   * @return True if the accessibility(screen-reader) is enabled.
   */
  bool IsEnabled() const;

  /**
   * Set the handler to handle accessibility actions.
   * @param[in] handler The Accessibility action handler.
   * @note Handlers should remove themselves when they are destroyed.
   */
  void SetActionHandler(AccessibilityActionHandler& handler);

  /**
   * Set the handler to handle accessibility gestures.
   * @param[in] handler The Accessibility gesture handler.
   * @note Handlers should remove themselves when they are destroyed.
   */
  void SetGestureHandler(AccessibilityGestureHandler& handler);

  /**
   * Handle the accessibility action to move focus to the next focusable actor
   * (by one finger flick down).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionNextEvent();

  /**
   * Handle the accessibility action to move focus to the previous focusable actor
   * (by one finger flick up).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionPreviousEvent();

  /**
   * Handle the accessibility action to activate the current focused actor (by one
   * finger double tap)
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionActivateEvent();

  /**
   * Handle the accessibility action to focus and read the actor (by one finger tap or move).
   * @param x x position of event
   * @param y y position of event
   * @param allowReadAgain true if the action read again the same object (i.e. read action)
   *                       false if the action just read when the focus object is changed (i.e. over action)
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionReadEvent(unsigned int x, unsigned int y, bool allowReadAgain);

  /**
   * Handle the accessibility action to move focus to the next focusable actor
   * (by one finger flick right).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionReadNextEvent();

  /**
   * Handle the accessibility action to move focus to the previous focusable actor
   * (by one finger flick up).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionReadPreviousEvent();

  /**
   * Handle the accessibility action to change the value when the current focused
   * actor is a slider (by double finger down and move up and right).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionUpEvent();

  /**
   * Handle the accessibility action to change the value when the current focused
   * actor is a slider (by double finger down and move down and left).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionDownEvent();

  /**
   * Handle the accessibility action to clear the focus from the current focused
   * actor if any, so that no actor is focused in the focus chain.
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionClearFocusEvent();

  /**
   * Handle the accessibility action to scroll when the current focused actor is
   * a scrollable control or its child (by 2 finger touch & move, 2 finger flick).
   * @param[in]  point      The touch point information.
   * @param[in]  timeStamp  The time the touch occurred.
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionScrollEvent(TouchPoint& point, unsigned long timeStamp);

  /**
   * Handle the accessibility action to navigate back (by two fingers circle draw).
   * @return Whether the action is performed successfully or not.
   */
  bool HandleActionBackEvent();

  /**
   * Handle the accessibility action to enable the feature
   */
  void HandleActionEnableEvent();

  /**
   * Handle the accessibility action to disable the feature
   */
  void HandleActionDisableEvent();

public:  // Signals

  /**
   * This is emitted when accessibility(screen-reader) feature turned on or off
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& StatusChangedSignal();

  /**
   * This is emitted when accessibility action is received to move focus to the next
   * focusable actor (by one finger flick down).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionNextSignal();

  /**
   * This is emitted when accessibility action is received to move focus to the previous
   * focusable actor (by one finger flick up).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionPreviousSignal();

  /**
   * This is emitted when accessibility action is received to activate the current focused
   * actor (by one finger double tap)
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionActivateSignal();

  /**
   * This is emitted when accessibility action is received to focus and read the actor
   * (by one finger tap).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionReadSignal();

  /**
   * This is emitted when accessibility action is received to focus and read the actor
   * (by one finger move).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionOverSignal();

  /**
   * This is emitted when accessibility action is received to move focus to the next
   * focusable actor (by one finger flick right).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionReadNextSignal();

  /**
   * This is emitted when accessibility action is received to move focus to the previous
   * focusable actor (by one finger flick left).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionReadPreviousSignal();

  /**
   * This is emitted when accessibility action is received to change the value when the
   * current focused actor is a slider (by double finger down and move up and right).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionUpSignal();

  /**
   * This is emitted when accessibility action is received to change the value when the
   * current focused actor is a slider (by double finger down and move down and left).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionDownSignal();

  /**
   * This is emitted when accessibility action is received to clear the focus from the
   * current focused actor if any, so that no actor is focused in the focus chain.
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionClearFocusSignal();

  /**
   * This is emitted when accessibility action is received to navigate back (by two
   * fingers circle draw).
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallback( AccessibilityManager& manager );
   * @endcode
   * @return The signal to connect to.
   */
  AccessibilityActionSignalV2& ActionBackSignal();

public: // Not intended for application developers

  /**
   * Creates a handle using the Adaptor::Internal implementation.
   * @param[in] manager The AccessibilityManager implementation.
   */
  AccessibilityManager( Internal::Adaptor::AccessibilityManager& manager );

  /**
   * This constructor is used by AccessibilityManager::Get().
   * @param[in] manager A pointer to the accessibility manager.
   */
  AccessibilityManager( Internal::Adaptor::AccessibilityManager* manager );
};

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_ACCESSIBILITY_MANAGER_H__
