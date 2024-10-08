#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_APPLICATION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_APPLICATION_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/application.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeApplication class is to correspond with Dali::Accessibility::Application.
 *
 * Implementation of org.a11y.atspi.Application interface
 */
class BridgeApplication : public virtual BridgeBase
{
protected:
  BridgeApplication() = default;

  /**
   * @brief Registers Application functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Application object of the currently executed DBus method call.
   *
   * @return The Application object
   */
  Dali::Accessibility::Application* FindSelf() const;

public:
  /**
   * @brief Gets name of graphic user interface framework used by an application.
   *
   * @return String with name
   */
  std::string GetToolkitName();

  /**
   * @brief Gets version of graphic user interface framework used by an application.
   *
   * @return String with version
   */
  std::string GetVersion();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_APPLICATION_H
