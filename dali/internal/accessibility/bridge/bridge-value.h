#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_VALUE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_VALUE_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <array>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/value.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeValue class is to correspond with Dali::Accessibility::Value.
 */
class BridgeValue : public virtual BridgeBase
{
protected:
  /**
   * @brief Constructor.
   */
  BridgeValue();

  /**
   * @brief Registers Value functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns Value Action object of the currently executed DBus method call.
   *
   * @return The Value object
   */
  std::shared_ptr<Dali::Accessibility::Value> FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Value::GetCurrent()
   */
  double GetCurrentValue();

  /**
   * @copydoc Dali::Accessibility::Value::SetCurrent()
   */
  void SetCurrentValue(double newValue);

  /**
   * @copydoc Dali::Accessibility::Value::GetValueText()
   */
  std::string GetCurrentValueText();

  /**
   * @copydoc Dali::Accessibility::Value::GetMaximum()
   */
  double GetMaximumValue();

  /**
   * @copydoc Dali::Accessibility::Value::GetMinimumIncrement()
   */
  double GetMinimumIncrement();

  /**
   * @copydoc Dali::Accessibility::Value::GetMinimum()
   */
  double GetMinimumValue();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_VALUE_H
