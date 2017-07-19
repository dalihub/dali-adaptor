#ifndef __DALI_WIDGET_DATA_H__
#define __DALI_WIDGET_DATA_H__

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
#include <dali/public-api/object/base-handle.h>
#include <bundle.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class WidgetData;
}
}

class Window;

class DALI_IMPORT_API WidgetData : public BaseHandle
{
public:

  /**
   * @brief This is the constructor for WidgetApplications with a name.
   *
   * @param[in]      instanceId for widget instance
   * @param[in]      args for widget instance
   * @param[in]      content for widget instance
   * @param[in]      window for widget instance
   * @return A handle to the WidgetData
   */
  static WidgetData New( const char* instanceId, bundle* args, char* content );

  /**
   * @brief The default constructor.
   *
   */
  WidgetData();

  /**
   * @brief Copy Constructor.
   * @param[in] WidgetData Handle to an object
   */
  WidgetData( const WidgetData& widgetData );

  /**
   * @brief Assignment operator.
   * @param[in] WidgetData Handle to an object
   * @return A reference to this
   */
  WidgetData& operator=( const WidgetData& widgetData );

  /**
   * @brief Destructor
   *
   */
  ~WidgetData();

  /**
   * @brief Get widget instance id
   * @return Id of widget instance
   */
  const char* GetInstanceId();

  /**
   * @brief Get widget instance arguments
   * @return Bundle which contains widget instance arguments
   */
  bundle* GetArgs();

  /**
   * @brief Get widget instance content
   * @return Content of widget instance
   */
  char* GetContent();

  /**
   * @brief Get widget instance Window
   * @return Window of widget instance
   */
  Window GetWindow();

  /**
   * @brief Set widget instance arguments
   * @param[in] args arguments of widget instance
   */
  void SetArgs( bundle* args );

  /**
   * @brief Set widget instance arguments
   * @param[in] content content of widget instance
   */
  void SetContent( char* content );

  /**
   * @brief Set widget instance arguments
   * @param[in] window Window of widget instance
   */
  void SetWindow( Window window );

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL WidgetData(Internal::Adaptor::WidgetData* widgetData);
  /// @endcond
};

} // namespace Dali

#endif // ___DALI_WIDGET_DATA_H__
