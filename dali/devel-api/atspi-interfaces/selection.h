#ifndef DALI_ADAPTOR_ATSPI_SELECTION_H
#define DALI_ADAPTOR_ATSPI_SELECTION_H

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
 * @brief Interface representing objects which can store a set of selected items.
 */
class DALI_ADAPTOR_API Selection : public virtual Accessible
{
public:
  /**
   * @brief Gets the number of selected children.
   *
   * @return The number of selected children (zero if none)
   */
  virtual int GetSelectedChildrenCount() const = 0;

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
  virtual Accessible* GetSelectedChild(int selectedChildIndex) = 0;

  /**
   * @brief Selects a child.
   *
   * @param childIndex The index of the child
   *
   * @return true on success, false otherwise
   */
  virtual bool SelectChild(int childIndex) = 0;

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
  virtual bool DeselectSelectedChild(int selectedChildIndex) = 0;

  /**
   * @brief Checks whether a child is selected.
   *
   * @param childIndex The index of the child
   *
   * @return true if given child is selected, false otherwise
   */
  virtual bool IsChildSelected(int childIndex) const = 0;

  /**
   * @brief Selects all children.
   *
   * @return true on success, false otherwise
   */
  virtual bool SelectAll() = 0;

  /**
   * @brief Deselects all children.
   *
   * @return true on success, false otherwise
   */
  virtual bool ClearSelection() = 0;

  /**
   * @brief Deselects a child.
   *
   * @param childIndex The index of the child.
   *
   * @return true on success, false otherwise
   *
   * @see Dali::Accessibility::Selection::DeselectSelectedChild
   */
  virtual bool DeselectChild(int childIndex) = 0;

  /**
   * @brief Downcasts an Accessible to a Selection.
   *
   * @param obj The Accessible
   * @return A Selection or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Selection* DownCast(Accessible* obj);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::SELECTION>
{
  using Type = Selection;
};
} // namespace Internal

inline Selection* Selection::DownCast(Accessible* obj)
{
  return Accessible::DownCast<AtspiInterface::SELECTION>(obj);
}

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_SELECTION_H
