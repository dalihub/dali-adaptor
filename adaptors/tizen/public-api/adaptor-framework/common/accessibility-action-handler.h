#ifndef __DALI_ACCESSIBILITY_ACTION_HANDLER_H__
#define __DALI_ACCESSIBILITY_ACTION_HANDLER_H__

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

// INTERNAL INCLUDES

namespace Dali DALI_IMPORT_API
{

/**
 * AccessibilityActionHandler is an abstract interface, used by Dali to handle accessibility actions
 * passed by the accessibility manager.
 */
class AccessibilityActionHandler
{
public:

  /**
   * Change the accessibility status when Accessibility feature(screen-reader) turned on or off.
   * @return whether the status is changed or not.
   */
  virtual bool ChangeAccessibilityStatus() = 0;

  /**
   * Clear the accessibility focus from the current focused actor.
   * @return whether the focus is cleared or not.
   */
  virtual bool ClearAccessibilityFocus() = 0;

  /**
   * Perform the accessibility action to move focus to the previous focusable actor (by one finger flick up).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionPrevious() = 0;

  /**
   * Perform the accessibility action to move focus to the next focusable actor (by one finger flick down).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionNext() = 0;

  /**
   * Perform the accessibility action to move focus to the previous focusable actor (by one finger flick left).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionReadPrevious() = 0;

  /**
   * Perform the accessibility action to move focus to the next focusable actor (by one finger flick right).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionReadNext() = 0;

  /**
   * Perform the accessibility action to focus and read the actor (by one finger tap or move).
   * @param allowReadAgain true if the action read again the same object (i.e. read action)
   *                       false if the action just read when the focus object is changed (i.e. over action)
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionRead(bool allowReadAgain) = 0;

  /**
   * Perform the accessibility action to activate the current focused actor (by one finger double tap).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionActivate() = 0;

  /**
   * Perform the accessibility action to change the value when the current focused actor is a slider
   * (by double finger down and move up and right).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionUp() = 0;

  /**
   * Perform the accessibility action to change the value when the current focused actor is a slider
   * (by double finger down and move down and left).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionDown() = 0;

  /**
   * Perform the accessibility action to navigate back (by two fingers circle draw).
   * @return whether the accessibility action is performed or not.
   */
  virtual bool AccessibilityActionBack() = 0;

}; // class AccessibilityActionHandler

} // namespace Dali

#endif // __DALI_ACCESSIBILITY_ACTION_HANDLER_H__
