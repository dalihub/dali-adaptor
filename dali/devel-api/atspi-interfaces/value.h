#ifndef DALI_ADAPTOR_ATSPI_VALUE_H
#define DALI_ADAPTOR_ATSPI_VALUE_H

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
 */

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface representing objects which can store numeric value.
 */
class DALI_ADAPTOR_API Value : public virtual Accessible
{
public:
  /**
   * @brief Gets the lowest possible value.
   *
   * @return The minimum value
  */
  virtual double GetMinimum() const = 0;

  /**
   * @brief Gets the current value.
   *
   * @return The current value
  */
  virtual double GetCurrent() const = 0;

  /**
   * @brief Gets the highest possible value.
   *
   * @return The highest value.
  */
  virtual double GetMaximum() const = 0;

  /**
   * @brief Sets the current value.
   *
   * @param[in] value The current value to set
   *
   * @return true if value could have been assigned, false otherwise
  */
  virtual bool SetCurrent(double value) = 0;

  /**
   * @brief Gets the lowest increment that can be distinguished.
   *
   * @return The lowest increment
  */
  virtual double GetMinimumIncrement() const = 0;

  /**
   * @brief Downcasts an Accessible to a Value.
   *
   * @param obj The Accessible
   * @return A Value or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Value* DownCast(Accessible* obj);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::VALUE>
{
  using Type = Value;
};
} // namespace Internal

inline Value* Value::DownCast(Accessible* obj)
{
  return Accessible::DownCast<AtspiInterface::VALUE>(obj);
}

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_VALUE_H
