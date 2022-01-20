#ifndef DALI_WIDGET_CONTROLLER_H
#define DALI_WIDGET_CONTROLLER_H

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
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/widget-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Holds the Implementation for the internal WidgetImpl class
 */
class Widget::Impl : public Dali::ConnectionTracker
{
public:
  /**
   * Set content information to widget framework
   */
  virtual void SetContentInfo(const std::string& contentInfo) = 0;

  /**
   * Check Widget is using key
   */
  virtual bool IsKeyEventUsing() const = 0;

  /**
   * Set the flag that widget is using keyEvent
   */
  virtual void SetUsingKeyEvent(bool flag) = 0;

  /**
   * Set the Information of widget
   */
  virtual void SetInformation(Dali::Window window, const std::string& widgetId) = 0;

  /**
   * Get the window
   */
  virtual Dali::Window GetWindow() const = 0;

  /**
   * Get the widget id
   */
  virtual std::string GetWidgetId() const = 0;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_WIDGET_CONTROLLER_H
