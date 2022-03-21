#ifndef DALI_WIDGET_CONTROLLER_WIN_H
#define DALI_WIDGET_CONTROLLER_WIN_H

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
#include <dali/internal/system/common/widget-controller.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Holds the Implementation for the internal WidgetImpl class
 */
class WidgetImplWin : public Widget::Impl
{
public:
  /**
   * Constructor
   */
  WidgetImplWin();

  /**
   * Destructor
   */
  ~WidgetImplWin() override;

public:
  /**
   * Set content information to widget framework
   */
  void SetContentInfo(const std::string& contentInfo) override;

  /**
   * Check Widget is using key
   */
  bool IsKeyEventUsing() const override;

  /**
   * Set the flag that widget is using keyEvent
   */
  void SetUsingKeyEvent(bool flag) override;

  /**
   * Set the Information of widget
   */
  void SetInformation(Dali::Window window, const std::string& widgetId) override;

  /**
   * Get the window
   */
  Dali::Window GetWindow() const override;

  /**
   * Get the widget id
   */
  std::string GetWidgetId() const override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_WIDGET_CONTROLLER_UBUNTU_H
