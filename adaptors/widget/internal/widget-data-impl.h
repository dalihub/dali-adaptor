#ifndef __DALI_INTERNAL_WIDGET_DATA_H__
#define __DALI_INTERNAL_WIDGET_DATA_H__

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

// INTERNAL INCLUDES
#include <widget-data.h>
#include <window.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class WidgetData;
typedef IntrusivePtr<WidgetData> WidgetDataPtr;

/**
 * Implementation of the Application class.
 */
class WidgetData : public BaseObject
{
public:

  /**
   * Create a new WidgetData
   * @param[in]      instanceId for widget instance
   * @param[in]      args for widget instance
   * @param[in]      content for widget instance
   * @return A handle to the WidgetData
   */
  static Dali::WidgetData New( const char* instanceId, bundle* args, char* content );

  /**
   * @brief Get widget instance id
   * @SINCE_1_2.47
   * @return Id of widget instance
   */
  const char* GetInstanceId();

  /**
   * @brief Get widget instance arguments
   * @SINCE_1_2.47
   * @return Bundle which contains widget instance arguments
   */
  bundle* GetArgs();

  /**
   * @brief Get widget instance content
   * @SINCE_1_2.47
   * @return Content of widget instance
   */
  char* GetContent();

  /**
   * @brief Get widget instance Window
   * @SINCE_1_2.47
   * @return Window of widget instance
   */
  Dali::Window GetWindow();

  /**
   * @brief Set widget instance arguments
   * @SINCE_1_2.47
   * @param[in] args arguments of widget instance
   */
  void SetArgs( bundle* args );

  /**
   * @brief Set widget instance arguments
   * @SINCE_1_2.47
   * @param[in] content content of widget instance
   */
  void SetContent( char* content );

  /**
   * @brief Set widget instance arguments
   * @SINCE_1_2.47
   * @param[in] window Window of widget instance
   */
  void SetWindow( Dali::Window window );

protected:

  /**
   * Private Constructor
   * @param[in]      instanceId for widget instance
   * @param[in]      args for widget instance
   * @param[in]      content for widget instance
   */
  WidgetData( const char* instanceId, bundle* args, char* content );

  /**
   * Destructor
   */
  virtual ~WidgetData();

  // Undefined
  WidgetData(const WidgetData&);
  WidgetData& operator=(WidgetData&);

private:

  Dali::Window   mWindow;
  bundle*        mArgs;
  const char*    mInstanceId;
  char*          mContent;

};

inline WidgetData& GetImplementation(Dali::WidgetData& widgetData)
{
  DALI_ASSERT_ALWAYS(widgetData && "widget data handle is empty");

  BaseObject& handle = widgetData.GetBaseObject();

  return static_cast<Internal::Adaptor::WidgetData&>(handle);
}

inline const WidgetData& GetImplementation(const Dali::WidgetData& widgetData)
{
  DALI_ASSERT_ALWAYS(widgetData && "widget data handle is empty");

  const BaseObject& handle = widgetData.GetBaseObject();

  return static_cast<const Internal::Adaptor::WidgetData&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_WIDGET_DATA_H__
