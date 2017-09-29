#ifndef __DALI_INTERNAL_WIDGET_H__
#define __DALI_INTERNAL_WIDGET_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-object.h>
#include <utility>
#include <dali/public-api/common/vector-wrapper.h>
#include <widget_base.h>

// INTERNAL INCLUDES
#include <widget.h>
#include <widget-application.h>
#include <widget-application-impl.h>
#include <window.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class Widget;
typedef IntrusivePtr<Widget> WidgetPtr;

/**
 * Implementation of the Application class.
 */
class Widget : public BaseObject
{
public:
  typedef Dali::Widget::WidgetCreateSignalType    WidgetCreateSignalType;
  typedef Dali::Widget::WidgetTerminateSignalType WidgetTerminateSignalType;
  typedef Dali::Widget::WidgetPauseSignalType     WidgetPauseSignalType;
  typedef Dali::Widget::WidgetResumeSignalType    WidgetResumeSignalType;
  typedef Dali::Widget::WidgetResizeSignalType    WidgetResizeSignalType;
  typedef Dali::Widget::WidgetUpdateSignalType    WidgetUpdateSignalType;

  /**
   * Create a new Widget
   * @param[in]  application  WidgetApplication for managing widget instance
   * @param[in]  id           Id for widget class
   */
  static Dali::Widget New( const std::string& id );

public:  // Signals

  /**
   * @copydoc Dali::Widget::InitSignal()
   */
  WidgetCreateSignalType& CreateSignal(){ return mCreateSignal; }

  /**
   * @copydoc Dali::Widget::TerminateSignal()
   */
  WidgetTerminateSignalType& TerminateSignal(){ return mTerminateSignal; }

  /**
   * @copydoc Dali::Widget::PauseSignal()
   */
  WidgetPauseSignalType& PauseSignal(){ return mPauseSignal; }

  /**
   * @copydoc Dali::Widget::ResumeSignal()
   */
  WidgetResumeSignalType& ResumeSignal(){ return mResumeSignal; }

  /**
   * @copydoc Dali::Widget::ResizeSignal()
   */
  WidgetResizeSignalType& ResizeSignal(){ return mResizeSignal; }

  /**
   * @copydoc Dali::Widget::UpdateSignal()
   */
  WidgetUpdateSignalType& UpdateSignal(){ return mUpdateSignal; }

protected:

  /**
   * Private Constructor
   * @param[in]  id           Id for widget class
   */
  Widget( const std::string& id );

  /**
   * Destructor
   */
  virtual ~Widget();

  // Undefined
  Widget(const Widget&);
  Widget& operator=(Widget&);

public:

  WidgetCreateSignalType        mCreateSignal;
  WidgetTerminateSignalType     mTerminateSignal;
  WidgetPauseSignalType         mPauseSignal;
  WidgetResumeSignalType        mResumeSignal;
  WidgetResizeSignalType        mResizeSignal;
  WidgetUpdateSignalType        mUpdateSignal;
  const std::string             mClassId;

private:
  SlotDelegate< Widget >                mSlotDelegate;
};

inline Widget& GetImplementation(Dali::Widget& widget)
{
  DALI_ASSERT_ALWAYS(widget && "widget handle is empty");

  BaseObject& handle = widget.GetBaseObject();

  return static_cast<Internal::Adaptor::Widget&>(handle);
}

inline const Widget& GetImplementation(const Dali::Widget& widget)
{
  DALI_ASSERT_ALWAYS(widget && "widget handle is empty");

  const BaseObject& handle = widget.GetBaseObject();

  return static_cast<const Internal::Adaptor::Widget&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
#endif // __DALI_INTERNAL_WIDGET_H__
