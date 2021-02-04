#ifndef DALI_INTERNAL_WIDGET_APPLICATION_TIZEN_H
#define DALI_INTERNAL_WIDGET_APPLICATION_TIZEN_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <widget_base.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/adaptor/common/application-impl.h>
#include <dali/internal/system/common/widget-application-impl.h>

namespace Dali
{
class Widget;

namespace Internal
{
namespace Adaptor
{
class WidgetApplication;
typedef IntrusivePtr<WidgetApplication> WidgetApplicationPtr;

/**
 * Implementation of the WidgetApplication class.
 */
class WidgetApplicationTizen : public WidgetApplication
{
public:
  typedef std::pair<const std::string, Dali::WidgetApplication::CreateWidgetFunction> CreateWidgetFunctionPair;
  typedef std::vector<CreateWidgetFunctionPair>                                       CreateWidgetFunctionContainer;

  /**
   * Create a new widget application
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   */
  static WidgetApplicationPtr New(int* argc, char** argv[], const std::string& stylesheet);

public:
  /**
   * @copydoc Dali::Internal::Adaptor::Application::OnInit()
   */
  void OnInit() override;

  /**
   * @copydoc Dali::WidgetApplication::RegisterWidgetCreator()
   */
  void RegisterWidgetCreatingFunction(const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction) override;

  /**
   * Add widget name - CreateWidgetFunction pair to container.
   */
  void AddWidgetCreatingFunctionPair(CreateWidgetFunctionPair pair);

  /**
   * Find and get CreateWidgetFunctionPair in container by widget name.
   */
  CreateWidgetFunctionPair GetWidgetCreatingFunctionPair(const std::string& widgetName);

  /**
   * Add widget_base_instance_h - Widget instance pair to container.
   */
  void AddWidget(widget_base_instance_h widgetBaseInstance, Dali::Widget widget, Dali::Window window);

  /**
   * Find and get Widget instance in container by widget_base_instance_h.
   */
  Dali::Widget GetWidget(widget_base_instance_h widgetBaseInstance) const;

  /**
   * Delete widget_base_instance_h - Widget instance pair in container.
   */
  void DeleteWidget(widget_base_instance_h widgetBaseInstance);

  /**
   * Find and get Window instance in container by widget_base_instance_h.
   */
  Dali::Window GetWindowFromWidget(widget_base_instance_h widgetBaseInstance) const;

  /**
   * Get the number of created widget.
   */
  int32_t GetWidgetCount();

protected:
  /**
   * Private Constructor
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   */
  WidgetApplicationTizen(int* argc, char** argv[], const std::string& stylesheet);

  /**
   * Destructor
   */
  virtual ~WidgetApplicationTizen();

  WidgetApplicationTizen(const Application&) = delete;
  WidgetApplicationTizen& operator=(Application&) = delete;

private:
  typedef std::pair<widget_base_instance_h, Dali::Widget> WidgetInstancePair;
  typedef std::vector<WidgetInstancePair>                 WidgetInstanceContainer;

  CreateWidgetFunctionContainer mCreateWidgetFunctionContainer;
  WidgetInstanceContainer       mWidgetInstanceContainer;

  typedef std::pair<widget_base_instance_h, Dali::Window> WindowInstancePair;
  typedef std::vector<WindowInstancePair>                 WindowInstanceContainer;
  WindowInstanceContainer                                 mWindowInstanceContainer;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WIDGET_APPLICATION_TIZEN_H
