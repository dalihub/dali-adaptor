#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SELECTION_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SELECTION_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/selection.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeSelection class is to correspond with Dali::Accessibility::Selection.
 */
class BridgeSelection : public virtual BridgeBase
{
protected:
  BridgeSelection() = default;

  /**
   * @brief Registers Selection functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Selection object of the currently executed DBus method call.
   *
   * @return The Selection object
   */
  std::shared_ptr<Dali::Accessibility::Selection> FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::Selection::GetSelectedChildrenCount()
   */
  DBus::ValueOrError<int32_t> GetSelectedChildrenCount();

  /**
   * @copydoc Dali::Accessibility::Selection::GetSelectedChild()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetSelectedChild(int32_t selectedChildIndex);

  /**
   * @copydoc Dali::Accessibility::Selection::SelectChild()
   */
  DBus::ValueOrError<bool> SelectChild(int32_t childIndex);

  /**
   * @copydoc Dali::Accessibility::Selection::DeselectSelectedChild()
   */
  DBus::ValueOrError<bool> DeselectSelectedChild(int32_t selectedChildIndex);

  /**
   * @copydoc Dali::Accessibility::Selection::IsChildSelected()
   */
  DBus::ValueOrError<bool> IsChildSelected(int32_t childIndex);

  /**
   * @copydoc Dali::Accessibility::Selection::SelectAll()
   */
  DBus::ValueOrError<bool> SelectAll();

  /**
   * @copydoc Dali::Accessibility::Selection::ClearSelection()
   */
  DBus::ValueOrError<bool> ClearSelection();

  /**
   * @copydoc Dali::Accessibility::Selection::DeselectChild()
   */
  DBus::ValueOrError<bool> DeselectChild(int32_t childIndex);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_SELECTION_H
