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
#include <appcore_multiwindow_base.h>
#include <utility>
#include <dali/public-api/common/vector-wrapper.h>

// INTERNAL INCLUDES
#include <widget.h>
#include <widget-data.h>
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
  typedef Dali::Widget::WidgetInstanceCreateSignalType    WidgetInstanceCreateSignalType;
  typedef Dali::Widget::WidgetInstanceTerminateSignalType WidgetInstanceTerminateSignalType;
  typedef Dali::Widget::WidgetInstancePauseSignalType     WidgetInstancePauseSignalType;
  typedef Dali::Widget::WidgetInstanceResumeSignalType    WidgetInstanceResumeSignalType;
  typedef Dali::Widget::WidgetInstanceResizeSignalType    WidgetInstanceResizeSignalType;
  typedef Dali::Widget::WidgetInstanceUpdateSignalType    WidgetInstanceUpdateSignalType;


  typedef std::pair< const char*, Dali::WidgetData > IdWidgetDataPair;
  typedef std::vector< IdWidgetDataPair > IdWidgetDataContainer;

  /**
   * Create a new Widget
   * @param[in]  id         Id of widget instance
   */
  static Dali::Widget New( const std::string& id );

public:  // Signals

  /**
   * @copydoc Dali::Widget::InitSignal()
   */
  WidgetInstanceCreateSignalType& CreateSignal(){ return mCreateSignal; }

  /**
   * @copydoc Dali::Widget::TerminateSignal()
   */
  WidgetInstanceTerminateSignalType& TerminateSignal(){ return mTerminateSignal; }

  /**
   * @copydoc Dali::Widget::PauseSignal()
   */
  WidgetInstancePauseSignalType& PauseSignal(){ return mPauseSignal; }

  /**
   * @copydoc Dali::Widget::ResumeSignal()
   */
  WidgetInstanceResumeSignalType& ResumeSignal(){ return mResumeSignal; }

  /**
   * @copydoc Dali::Widget::ResizeSignal()
   */
  WidgetInstanceResizeSignalType& ResizeSignal(){ return mResizeSignal; }

  /**
   * @copydoc Dali::Widget::UpdateSignal()
   */
  WidgetInstanceUpdateSignalType& UpdateSignal(){ return mUpdateSignal; }

public:

  /**
   * Called when the widget is resized.
   */
  void OnResize(appcore_multiwindow_base_instance_h, Dali::Widget::WindowSize windowSize);

  /**
   * Called when the widget is updated.
   */
  void OnUpdate(appcore_multiwindow_base_instance_h, bundle* content, int force);

  /**
   * Called when Create Widget Instance.
   */
  void InsertWidgetData( const char* id, Dali::WidgetData widgetData );

  /**
   * Called when finding widget data by instance id.
   */
  Dali::WidgetData* FindWidgetData( const char* instanceId );

  /**
   * Called when deleting widget  data by instance id.
   */
  void DeleteWidgetData( const char* instanceId );

protected:

  /**
   * Private Constructor
   * @param[in]  id         Id for widget instance
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

  WidgetInstanceCreateSignalType        mCreateSignal;
  WidgetInstanceTerminateSignalType     mTerminateSignal;
  WidgetInstancePauseSignalType         mPauseSignal;
  WidgetInstanceResumeSignalType        mResumeSignal;
  WidgetInstanceResizeSignalType        mResizeSignal;
  WidgetInstanceUpdateSignalType        mUpdateSignal;
  const std::string                     mClassId;

private:
  SlotDelegate< Widget >                mSlotDelegate;
  IdWidgetDataContainer                 mIdWidgetContainer;
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
