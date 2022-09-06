#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TABLE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TABLE_H

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
#include <dali/devel-api/atspi-interfaces/table.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeTable class contains glue code for Accessibility::Table.
 */
class BridgeTable : public virtual BridgeBase
{
protected:
  BridgeTable() = default;

  /**
   * @brief Registers Table methods as a DBus interface.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Table object of the currently executed DBus method call.
   *
   * @return The Table object
   */
  Dali::Accessibility::Table* FindSelf() const;

public:
  using RowColumnSpanType = DBus::ValueOrError<
    bool,         // success
    std::int32_t, // row
    std::int32_t, // column
    std::int32_t, // rowSpan
    std::int32_t, // columnSpan
    bool          // isSelected
    >;

  /**
   * @copydoc Dali::Accessibility::Table::GetRowCount()
   */
  DBus::ValueOrError<std::int32_t> GetRowCount();

  /**
   * @copydoc Dali::Accessibility::Table::GetColumnCount()
   */
  DBus::ValueOrError<std::int32_t> GetColumnCount();

  /**
   * @copydoc Dali::Accessibility::Table::GetSelectedRowCount()
   */
  DBus::ValueOrError<std::int32_t> GetSelectedRowCount();

  /**
   * @copydoc Dali::Accessibility::Table::GetSelectedColumnCount()
   */
  DBus::ValueOrError<std::int32_t> GetSelectedColumnCount();

  /**
   * @copydoc Dali::Accessibility::Table::GetCaption()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetCaption();

  /**
   * @copydoc Dali::Accessibility::Table::GetSummary()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetSummary();

  /**
   * @copydoc Dali::Accessibility::Table::GetCell()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetCell(std::int32_t row, std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetChildIndex()
   */
  DBus::ValueOrError<std::int32_t> GetChildIndex(std::int32_t row, std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetRowByChildIndex()
   */
  DBus::ValueOrError<std::int32_t> GetRowByChildIndex(std::int32_t childIndex);

  /**
   * @copydoc Dali::Accessibility::Table::GetColumnByChildIndex()
   */
  DBus::ValueOrError<std::int32_t> GetColumnByChildIndex(std::int32_t childIndex);

  /**
   * @copydoc Dali::Accessibility::Table::GetRowDescription()
   */
  DBus::ValueOrError<std::string> GetRowDescription(std::int32_t row);

  /**
   * @copydoc Dali::Accessibility::Table::GetColumnDescription()
   */
  DBus::ValueOrError<std::string> GetColumnDescription(std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetRowSpan()
   */
  DBus::ValueOrError<std::int32_t> GetRowSpan(std::int32_t row, std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetColumnSpan()
   */
  DBus::ValueOrError<std::int32_t> GetColumnSpan(std::int32_t row, std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetRowHeader()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetRowHeader(std::int32_t row);

  /**
   * @copydoc Dali::Accessibility::Table::GetColumnHeader()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetColumnHeader(std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetSelectedRows()
   */
  DBus::ValueOrError<std::vector<std::int32_t>> GetSelectedRows();

  /**
   * @copydoc Dali::Accessibility::Table::GetSelectedColumns()
   */
  DBus::ValueOrError<std::vector<std::int32_t>> GetSelectedColumns();

  /**
   * @copydoc Dali::Accessibility::Table::IsRowSelected()
   */
  DBus::ValueOrError<bool> IsRowSelected(std::int32_t row);

  /**
   * @copydoc Dali::Accessibility::Table::IsColumnSelected()
   */
  DBus::ValueOrError<bool> IsColumnSelected(std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::IsCellSelected()
   */
  DBus::ValueOrError<bool> IsCellSelected(std::int32_t row, std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::AddRowSelection()
   */
  DBus::ValueOrError<bool> AddRowSelection(std::int32_t row);

  /**
   * @copydoc Dali::Accessibility::Table::AddColumnSelection()
   */
  DBus::ValueOrError<bool> AddColumnSelection(std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::RemoveRowSelection()
   */
  DBus::ValueOrError<bool> RemoveRowSelection(std::int32_t row);

  /**
   * @copydoc Dali::Accessibility::Table::RemoveColumnSelection()
   */
  DBus::ValueOrError<bool> RemoveColumnSelection(std::int32_t column);

  /**
   * @copydoc Dali::Accessibility::Table::GetRowColumnSpan()
   */
  RowColumnSpanType GetRowColumnSpan(std::int32_t childIndex);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_TABLE_H
