#ifndef __DALI_INTERNAL_ACCESSIBILITY_ADAPTOR_MOBILE_H__
#define __DALI_INTERNAL_ACCESSIBILITY_ADAPTOR_MOBILE_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <accessibility-adaptor.h>
#include <accessibility-action-handler.h>
#include <accessibility-gesture-handler.h>
#include <indicator-interface.h>
#include <accessibility-gesture-detector.h>
#include <accessibility-adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * This mobile version is different since it forwards events to the indicator.
 */
class AccessibilityAdaptorMobile : public AccessibilityAdaptor
{
public:

  /**
   * Constructor.
   */
  AccessibilityAdaptorMobile();

  // From AccessibilityAdaptor base class

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionNextEvent()
   */
  virtual bool HandleActionNextEvent( bool allowEndFeedback );

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionPreviousEvent()
   */
  virtual bool HandleActionPreviousEvent( bool allowEndFeedback );

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionActivateEvent()
   */
  virtual bool HandleActionActivateEvent();

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionReadEvent()
   */
  virtual bool HandleActionReadEvent( unsigned int x, unsigned int y, bool allowReadAgain );

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionReadNextEvent()
   */
  virtual bool HandleActionReadNextEvent( bool allowEndFeedback );

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionReadPreviousEvent()
   */
  virtual bool HandleActionReadPreviousEvent( bool allowEndFeedback );

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionUpEvent()
   */
  virtual bool HandleActionUpEvent();

  /**
   * @copydoc Dali::AccessibilityAdaptor::HandleActionDownEvent()
   */
  virtual bool HandleActionDownEvent();

private:

  /**
   * Destructor.
   */
  virtual ~AccessibilityAdaptorMobile();

  // Undefined
  AccessibilityAdaptorMobile( const AccessibilityAdaptorMobile& );
  AccessibilityAdaptorMobile& operator=( AccessibilityAdaptorMobile& );

public:

  // Helpers for public-api forwarding methods

  inline static Internal::Adaptor::AccessibilityAdaptorMobile& GetImplementation(Dali::AccessibilityAdaptor& adaptor)
  {
    DALI_ASSERT_ALWAYS( adaptor && "AccessibilityAdaptorMobile handle is empty" );

    BaseObject& handle = adaptor.GetBaseObject();

    return static_cast<Internal::Adaptor::AccessibilityAdaptorMobile&>(handle);
  }

  inline static const Internal::Adaptor::AccessibilityAdaptorMobile& GetImplementation(const Dali::AccessibilityAdaptor& adaptor)
  {
    DALI_ASSERT_ALWAYS( adaptor && "AccessibilityAdaptorMobile handle is empty" );

    const BaseObject& handle = adaptor.GetBaseObject();

    return static_cast<const Internal::Adaptor::AccessibilityAdaptorMobile&>(handle);
  }

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_ACCESSIBILITY_ADAPTOR_MOBILE_H__
