#ifndef DALI_ADAPTOR_ATSPI_VALUE_H
#define DALI_ADAPTOR_ATSPI_VALUE_H

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
 */

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface representing objects which can store numeric value.
 */
class DALI_ADAPTOR_API Value
{
public:
  /**
   * @brief Gets the lowest possible value.
   *
   * @return The minimum value
   */
  virtual double GetMinimum() const;

  /**
   * @brief Gets the current value.
   *
   * The application may set the "value_format" attribute to one of the
   * following values in order to customize what is read by the Screen Reader:
   * 1. "percent" (the default) - GetCurrent() normalized as a percentage
   *    of the range [GetMinimum(), GetMaximum()],
   * 2. "number" - GetCurrent() verbatim
   * 3. "text" - GetValueText() is used instead of GetCurrent()
   *
   * @return The current value
   *
   * @see Value::GetMinimum()
   * @see Value::GetMaximum()
   * @see Value::GetValueText()
   * @see Dali::Toolkit::DevelControl::AppendAccessibilityAttribute()
   */
  virtual double GetCurrent() const;

  /**
   * @brief Gets the formatted current value.
   *
   * This does not have to be GetCurrent() formatted in any particular way,
   * i.e. it may be an arbitrary string, e.g. "small font size" for the
   * numeric value 10.0.
   *
   * @return The current value as text
   *
   * @note Only used if the "value_format" attribute is "text"
   * @see Value::GetCurrent()
   */
  virtual std::string GetValueText() const;

  /**
   * @brief Gets the highest possible value.
   *
   * @return The highest value.
   */
  virtual double GetMaximum() const;

  /**
   * @brief Sets the current value.
   *
   * @param[in] value The current value to set
   *
   * @return true if value could have been assigned, false otherwise
   */
  virtual bool SetCurrent(double value);

  /**
   * @brief Gets the lowest increment that can be distinguished.
   *
   * @return The lowest increment
   */
  virtual double GetMinimumIncrement() const;
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::VALUE>
{
  using Type = Value;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_VALUE_H
