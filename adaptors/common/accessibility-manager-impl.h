#ifndef __DALI_INTERNAL_ACCESSIBILITY_MANAGER_H__
#define __DALI_INTERNAL_ACCESSIBILITY_MANAGER_H__

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
#include <string>

#include <dali/public-api/object/base-object.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/events/touch-point.h>
#include <dali/integration-api/events/touch-event-combiner.h>
#include <accessibility-manager.h>

// INTERNAL INCLUDES
#include <accessibility-action-handler.h>
#include <accessibility-gesture-handler.h>
#include <indicator-impl.h>
#include <accessibility-gesture-detector.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * This class detects to accessibility action
 */
class AccessibilityManager : public Dali::BaseObject
{
public:

  typedef Dali::AccessibilityManager::AccessibilityActionSignalType AccessibilityActionSignalType;
  typedef Dali::AccessibilityManager::AccessibilityActionScrollSignalType AccessibilityActionScrollSignalType;

  // Creation

  /**
   * Constructor.
   */
  AccessibilityManager();

  /**
   * Get an instance of the AccessibilityManager.
   * @return The instance of the AccessibilityManager.
   */
  static Dali::AccessibilityManager Get();

  // Public API

  /**
   * Turn on accessibility action
   * This method should be called by vconf callback
   */
  void EnableAccessibility();

  /**
   * Turn off accessibility action
   * This method should be called by vconf callback
   */
  void DisableAccessibility();

  /**
   * @copydoc Dali::AccessibilityManager::IsEnabled()
   */
  bool IsEnabled() const;

  /**
   * @copydoc Dali::AccessibilityManager::GetReadPosition() const
   */
  Vector2 GetReadPosition() const;

  /**
   * @copydoc Dali::AccessibilityManager::SetActionHandler()
   */
  void SetActionHandler(AccessibilityActionHandler& handler);

  /**
   * @copydoc Dali::AccessibilityManager::SetGestureHandler()
   */
  void SetGestureHandler(AccessibilityGestureHandler& handler);

  /**
   * Set the Indicator
   */
  void SetIndicator(Indicator* indicator);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionNextEvent()
   */
  bool HandleActionNextEvent(bool allowEndFeedback = true);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionPreviousEvent()
   */
  bool HandleActionPreviousEvent(bool allowEndFeedback = true);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionActivateEvent()
   */
  bool HandleActionActivateEvent();

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionReadEvent()
   */
  bool HandleActionReadEvent(unsigned int x, unsigned int y, bool allowReadAgain);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionReadNextEvent()
   */
  bool HandleActionReadNextEvent(bool allowEndFeedback = true);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionReadPreviousEvent()
   */
  bool HandleActionReadPreviousEvent(bool allowEndFeedback = true);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionUpEvent()
   */
  bool HandleActionUpEvent();

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionDownEvent()
   */
  bool HandleActionDownEvent();

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionClearFocusEvent()
   */
  bool HandleActionClearFocusEvent();

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionScrollEvent()
   */
  bool HandleActionScrollEvent(const TouchPoint& point, unsigned long timeStamp);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionTouchEvent()
   */
  bool HandleActionTouchEvent(const TouchPoint& point, unsigned long timeStamp);

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionBackEvent()
   */
  bool HandleActionBackEvent();

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionEnableEvent()
   */
  void HandleActionEnableEvent();

  /**
   * @copydoc Dali::AccessibilityManager::HandleActionDisableEvent()
   */
  void HandleActionDisableEvent();

public: // Signals

  /**
   * @copydoc Dali::AccessibilityManager::StatusChangedSignal
   */
  AccessibilityActionSignalType& StatusChangedSignal()
  {
    return mStatusChangedSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionNextSignal
   */
  AccessibilityActionSignalType& ActionNextSignal()
  {
    return mActionNextSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionPreviousSignal
   */
  AccessibilityActionSignalType& ActionPreviousSignal()
  {
    return mActionPreviousSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionActivateSignal
   */
  AccessibilityActionSignalType& ActionActivateSignal()
  {
    return mActionActivateSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionOverSignal
   */
  AccessibilityActionSignalType& ActionOverSignal()
  {
    return mActionOverSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionReadSignal
   */
  AccessibilityActionSignalType& ActionReadSignal()
  {
    return mActionReadSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionReadNextSignal
   */
  AccessibilityActionSignalType& ActionReadNextSignal()
  {
    return mActionReadNextSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionReadPreviousSignal
   */
  AccessibilityActionSignalType& ActionReadPreviousSignal()
  {
    return mActionReadPreviousSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionUpSignal
   */
  AccessibilityActionSignalType& ActionUpSignal()
  {
    return mActionUpSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionDownSignal
   */
  AccessibilityActionSignalType& ActionDownSignal()
  {
    return mActionDownSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionClearFocusSignal
   */
  AccessibilityActionSignalType& ActionClearFocusSignal()
  {
    return mActionClearFocusSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionBackSignal
   */
  AccessibilityActionSignalType& ActionBackSignal()
  {
    return mActionBackSignal;
  }

  /**
   * @copydoc Dali::AccessibilityManager::ActionScrollSignal
   */
  AccessibilityActionScrollSignalType& ActionScrollSignal()
  {
    return mActionScrollSignal;
  }

private:

  // Destruction

  /**
   * Destructor.
   */
  virtual ~AccessibilityManager();

  // Undefined
  AccessibilityManager( const AccessibilityManager& );
  AccessibilityManager& operator=( AccessibilityManager& );

private:

  Dali::Integration::TouchEventCombiner mCombiner; ///< Combines multi-touch events.

  bool mIsEnabled; ///< enable/disable the accessibility action
  Vector2 mReadPosition; ///< ActionRead position

  AccessibilityActionHandler* mActionHandler; ///< The pointer of accessibility action handler

  AccessibilityGestureDetectorPtr mAccessibilityGestureDetector; ///< The accessibility gesture detector

  Indicator* mIndicator; ///< The indicator
  bool mIndicatorFocused; ///< Whether the Indicator is focused

  AccessibilityActionSignalType mStatusChangedSignal;
  AccessibilityActionSignalType mActionNextSignal;
  AccessibilityActionSignalType mActionPreviousSignal;
  AccessibilityActionSignalType mActionActivateSignal;
  AccessibilityActionSignalType mActionOverSignal;
  AccessibilityActionSignalType mActionReadSignal;
  AccessibilityActionSignalType mActionReadNextSignal;
  AccessibilityActionSignalType mActionReadPreviousSignal;
  AccessibilityActionSignalType mActionUpSignal;
  AccessibilityActionSignalType mActionDownSignal;
  AccessibilityActionSignalType mActionClearFocusSignal;
  AccessibilityActionSignalType mActionBackSignal;
  AccessibilityActionScrollSignalType mActionScrollSignal;

public:

  // Helpers for public-api forwarding methods

  inline static Internal::Adaptor::AccessibilityManager& GetImplementation(Dali::AccessibilityManager& manager)
  {
    DALI_ASSERT_ALWAYS( manager && "AccessibilityManager handle is empty" );

    BaseObject& handle = manager.GetBaseObject();

    return static_cast<Internal::Adaptor::AccessibilityManager&>(handle);
  }

  inline static const Internal::Adaptor::AccessibilityManager& GetImplementation(const Dali::AccessibilityManager& manager)
  {
    DALI_ASSERT_ALWAYS( manager && "AccessibilityManager handle is empty" );

    const BaseObject& handle = manager.GetBaseObject();

    return static_cast<const Internal::Adaptor::AccessibilityManager&>(handle);
  }

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_ACCESSIBILITY_MANAGER_H__
