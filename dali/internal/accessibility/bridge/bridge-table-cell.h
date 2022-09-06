#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TABLE_CELL_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TABLE_CELL_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/atspi-interfaces/table-cell.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeTableCell class contains glue code for Accessibility::TableCell.
 */
class BridgeTableCell : public virtual BridgeBase
{
protected:
  BridgeTableCell() = default;

  /**
   * @brief Registers TableCell methods as a DBus interface.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the TableCell object of the currently executed DBus method call.
   *
   * @return The TableCell object
   */
  Dali::Accessibility::TableCell* FindSelf() const;

public:
  /**
   * @copydoc Dali::Accessibility::TableCell::GetTable()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetTable();

  /**
   * @copydoc Dali::Accessibility::TableCell::GetCellPosition()
   */
  DBus::ValueOrError<std::int32_t, std::int32_t> GetCellPosition();

  /**
   * @copydoc Dali::Accessibility::TableCell::GetCellRowSpan()
   */
  DBus::ValueOrError<std::int32_t> GetCellRowSpan();

  /**
   * @copydoc Dali::Accessibility::TableCell::GetCellColumnSpan()
   */
  DBus::ValueOrError<std::int32_t> GetCellColumnSpan();

  /**
   * @copydoc Dali::Accessibility::TableCell::GetCellRowColumnSpan()
   */
  DBus::ValueOrError<std::int32_t, std::int32_t, std::int32_t, std::int32_t> GetCellRowColumnSpan();
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TABLE_CELL_H
