#ifndef DALI_INTERNAL_WIDGET_APPLICATION_IMPL_UBUNTU_H
#define DALI_INTERNAL_WIDGET_APPLICATION_IMPL_UBUNTU_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/common/application-impl.h>
#include <dali/internal/system/common/widget-application-impl.h>
#include <dali/public-api/adaptor-framework/widget-application.h>

namespace Dali
{
class Widget;

namespace Internal
{
namespace Adaptor
{
/**
 * Implementation of the WidgetApplicationUbuntu class.
 */
class WidgetApplicationUbuntu : public WidgetApplication
{
public:
  typedef std::pair<const std::string, Dali::WidgetApplication::CreateWidgetFunction> CreateWidgetFunctionPair;
  typedef std::vector<CreateWidgetFunctionPair>                                       CreateWidgetFunctionContainer;

  /**
   * Create a new widget application
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   * @param[in]  windowData   The window data
   */
  static WidgetApplicationPtr New(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData);

public:
  /**
   * @copydoc Dali::WidgetApplication::RegisterWidgetCreator()
   */
  void RegisterWidgetCreatingFunction(const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction) override;

protected:
  /**
   * Private Constructor
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   * @param[in]  windowData   The window data
   */
  WidgetApplicationUbuntu(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData);

  /**
   * Destructor
   */
  virtual ~WidgetApplicationUbuntu();

  WidgetApplicationUbuntu(const Application&)      = delete;
  WidgetApplicationUbuntu& operator=(Application&) = delete;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WIDGET_APPLICATION_IMPL_UBUNTU_H
