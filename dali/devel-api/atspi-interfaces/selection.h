#ifndef DALI_ADAPTOR_ATSPI_SELECTION_H
#define DALI_ADAPTOR_ATSPI_SELECTION_H

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
 * @brief Interface representing objects which can store a set of selected items.
 */
class DALI_ADAPTOR_API Selection
{
public:
  /**
   * @brief Gets the number of selected children.
   *
   * @return The number of selected children (zero if none)
   */
  virtual int GetSelectedChildrenCount() const;

  /**
   * @brief Gets a specific selected child.
   *
   * @param selectedChildIndex The index of the selected child
   *
   * @note @p selectedChildIndex refers to the list of selected children,
   * not the list of all children
   *
   * @return The selected child or nullptr if index is invalid
   */
  virtual Accessible* GetSelectedChild(int selectedChildIndex);

  /**
   * @brief Selects a child.
   *
   * @param childIndex The index of the child
   *
   * @return true on success, false otherwise
   */
  virtual bool SelectChild(int childIndex);

  /**
   * @brief Deselects a selected child.
   *
   * @param selectedChildIndex The index of the selected child
   *
   * @note @p selectedChildIndex refers to the list of selected children,
   * not the list of all children
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::Selection::DeselectChild
   */
  virtual bool DeselectSelectedChild(int selectedChildIndex);

  /**
   * @brief Checks whether a child is selected.
   *
   * @param childIndex The index of the child
   *
   * @return true if given child is selected, false otherwise
   */
  virtual bool IsChildSelected(int childIndex) const;

  /**
   * @brief Selects all children.
   *
   * @return true on success, false otherwise
   */
  virtual bool SelectAll();

  /**
   * @brief Deselects all children.
   *
   * @return true on success, false otherwise
   */
  virtual bool ClearSelection();

  /**
   * @brief Deselects a child.
   *
   * @param childIndex The index of the child.
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::Selection::DeselectSelectedChild
   */
  virtual bool DeselectChild(int childIndex);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::SELECTION>
{
  using Type = Selection;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_SELECTION_H
