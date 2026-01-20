#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACTION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACTION_H

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
#include <tuple>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/action.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeAction class is to correspond with Dali::Accessibility::Action.
 */
class BridgeAction : public virtual BridgeBase
{
protected:
  BridgeAction() = default;

  /**
   * @brief Registers Action functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Action object of the currently executed DBus method call.
   *
   * @return The Action object
   */
  std::shared_ptr<Dali::Accessibility::Action> FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Action::GetActionName()
   */
  DBus::ValueOrError<std::string> GetActionName(int32_t index);

  /**
   * @copydoc Dali::Accessibility::Action::GetLocalizedActionName()
   */
  DBus::ValueOrError<std::string> GetLocalizedActionName(int32_t index);

  /**
   * @copydoc Dali::Accessibility::Action::GetActionDescription()
   */
  DBus::ValueOrError<std::string> GetActionDescription(int32_t index);

  /**
   * @copydoc Dali::Accessibility::Action::GetActionKeyBinding()
   */
  DBus::ValueOrError<std::string> GetActionKeyBinding(int32_t index);

  /**
   * @copydoc Dali::Accessibility::Action::GetActionCount()
   */
  DBus::ValueOrError<int32_t> GetActionCount();

  /**
   * @copydoc Dali::Accessibility::Action::DoAction()
   */
  DBus::ValueOrError<bool> DoAction(int32_t index);

  /**
   * @copydoc Dali::Accessibility::Action::DoAction()
   */
  DBus::ValueOrError<bool> DoActionName(std::string name);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACTION_H
