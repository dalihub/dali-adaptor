#ifndef DALI_ADAPTOR_ATSPI_ACTION_H
#define DALI_ADAPTOR_ATSPI_ACTION_H

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

// EXTERNAL INCLUDES
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>

namespace Dali::Accessibility
{
/**
 * @brief Interface enabling to perform provided actions.
 */
class DALI_ADAPTOR_API Action
{
public:
  /**
   * @brief Gets name of action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with name of action
   */
  virtual std::string GetActionName(std::size_t index) const;

  /**
   * @brief Gets translated name of action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with name of action translated according to current translation domain
   *
   * @note The translation is not supported in this version
   */
  virtual std::string GetLocalizedActionName(std::size_t index) const;

  /**
   * @brief Gets description of action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with description of action
   */
  virtual std::string GetActionDescription(std::size_t index) const;

  /**
   * @brief Gets key code binded to action with given index.
   *
   * @param[in] index The index of action
   *
   * @return The string with key name
   */
  virtual std::string GetActionKeyBinding(std::size_t index) const;

  /**
   * @brief Gets number of provided actions.
   *
   * @return The number of actions
   */
  virtual std::size_t GetActionCount() const;

  /**
   * @brief Performs an action with given index.
   *
   * @param index The index of action
   *
   * @return true on success, false otherwise
   */
  virtual bool DoAction(std::size_t index);

  /**
   * @brief Performs an action with given name.
   *
   * @param name The name of action
   *
   * @return true on success, false otherwise
   */
  virtual bool DoAction(const std::string& name);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::ACTION>
{
  using Type = Action;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_ACTION_H
