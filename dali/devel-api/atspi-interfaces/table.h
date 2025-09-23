#ifndef DALI_ADAPTOR_ATSPI_TABLE_H
#define DALI_ADAPTOR_ATSPI_TABLE_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

namespace Dali::Accessibility
{
class TableCell;

/**
 * @brief Interface representing a table.
 *
 * The selection methods extend the Selection interface,
 * so both should be implemented by table and grid controls.
 *
 * @see Dali::Accessibility::Selection
 * @see Dali::Accessibility::TableCell
 */
class DALI_ADAPTOR_API Table : public virtual Accessible
{
public:
  /**
   * @brief Cell information type.
   *
   * @see Table:GetRowColumnSpan()
   */
  struct RowColumnSpanType
  {
    int  row;        ///< Row index
    int  column;     ///< Column index
    int  rowSpan;    ///< Row span
    int  columnSpan; ///< Column span
    bool isSelected; ///< Whether the cell is selected
    bool success;    ///< Whether other fields are meaningful
  };

  /**
   * @brief Gets the number of rows.
   *
   * @return Number of rows
   */
  virtual int GetRowCount() const = 0;

  /**
   * @brief Gets the number of columns.
   *
   * @return Number of columns
   */
  virtual int GetColumnCount() const = 0;

  /**
   * @brief Gets the number of selected rows.
   *
   * @return Number of selected rows
   *
   * @see Table::GetSelectedRows()
   */
  virtual int GetSelectedRowCount() const = 0;

  /**
   * @brief Gets the number of selected columns.
   *
   * @return Number of selected columns
   *
   * @see Table::GetSelectedColumns()
   */
  virtual int GetSelectedColumnCount() const = 0;

  /**
   * @brief Gets the table's caption.
   *
   * @return The caption or null
   */
  virtual Accessible* GetCaption() const = 0;

  /**
   * @brief Gets the table's summary.
   *
   * @return The summary or null
   */
  virtual Accessible* GetSummary() const = 0;

  /**
   * @brief Gets the cell at the specified position
   *
   * @param[in] row Row number
   * @param[in] column Column number
   *
   * @return The cell or null
   */
  virtual TableCell* GetCell(int row, int column) const = 0;

  /**
   * @brief Gets the one-dimensional index of a cell
   *
   * The returned index should be such that:
   * @code GetChildAtIndex(GetChildIndex(row, column)) == GetCell(row, column) @endcode
   *
   * @param[in] row Row number
   * @param[in] column Column number
   *
   * @return The one-dimensional index
   *
   * @see Dali::Accessibility::Accessible::GetChildAtIndex()
   * @see Table::GetCell()
   */
  virtual std::size_t GetChildIndex(int row, int column) const = 0;

  /**
   * @brief Gets the row number of a cell
   *
   * @param[in] childIndex One-dimensional index of the cell
   *
   * @return The row number of the cell
   *
   * @see Table::GetChildIndex()
   */
  virtual int GetRowByChildIndex(std::size_t childIndex) const = 0;

  /**
   * @brief Gets the column number of a cell
   *
   * @param[in] childIndex One-dimensional index of the cell
   *
   * @return The column number of the cell
   *
   * @see Table::GetChildIndex()
   */
  virtual int GetColumnByChildIndex(std::size_t childIndex) const = 0;

  /**
   * @brief Gets the description of a row.
   *
   * @param[in] row Row number
   *
   * @return The description of the row
   */
  virtual std::string GetRowDescription(int row) const = 0;

  /**
   * @brief Gets the description of a column.
   *
   * @param[in] column Column number
   *
   * @return The description of the column
   */
  virtual std::string GetColumnDescription(int column) const = 0;

  /**
   * @brief Gets the row span of a cell
   *
   * The return value should be such that:
   * @code GetRowSpan(row, column) == GetCell(row, column)->GetCellRowSpan() @endcode
   *
   * @param[in] row Row number
   * @param[in] column Column number
   *
   * @return The row span of the cell
   *
   * @see Table::GetCell()
   * @see Dali::Accessibility::TableCell::GetCellRowSpan()
   */
  virtual int GetRowSpan(int row, int column) const = 0;

  /**
   * @brief Gets the column span of a cell
   *
   * The return value should be such that:
   * @code GetColumnSpan(row, column) == GetCell(row, column)->GetCellColumnSpan() @endcode
   *
   * @param[in] row Row number
   * @param[in] column Column number
   *
   * @return The column span of the cell
   *
   * @see Table::GetCell()
   * @see Dali::Accessibility::TableCell::GetCellColumnSpan()
   */
  virtual int GetColumnSpan(int row, int column) const = 0;

  /**
   * @brief Gets the header of a row.
   *
   * @param[in] row Row number
   *
   * @return The row header or null
   */
  virtual Accessible* GetRowHeader(int row) const = 0;

  /**
   * @brief Gets the header of a column.
   *
   * @param[in] column Column number
   *
   * @return The column header or null
   */
  virtual Accessible* GetColumnHeader(int column) const = 0;

  /**
   * @brief Gets all selected rows' numbers.
   *
   * @return Selected rows' numbers
   *
   * @see Table::GetSelectedRowCount()
   */
  virtual std::vector<int> GetSelectedRows() const = 0;

  /**
   * @brief Gets all selected columns' numbers.
   *
   * @return Selected columns' numbers
   *
   * @see Table::GetSelectedColumnCount()
   */
  virtual std::vector<int> GetSelectedColumns() const = 0;

  /**
   * @brief Checks if a row is selected.
   *
   * @param[in] row Row number
   *
   * @return True if the row is selected, false otherwise
   */
  virtual bool IsRowSelected(int row) const = 0;

  /**
   * @brief Checks if a column is selected.
   *
   * @param[in] column Column number
   *
   * @return True if the column is selected, false otherwise
   */
  virtual bool IsColumnSelected(int column) const = 0;

  /**
   * @brief Checks if a cell is selected.
   *
   * @param[in] row Row number of the cell
   * @param[in] column Column number of the cell
   *
   * @return True if the cell is selected, false otherwise
   */
  virtual bool IsCellSelected(int row, int column) const = 0;

  /**
   * @brief Selects a row.
   *
   * @param[in] row Row number
   *
   * @return True on success, false otherwise
   */
  virtual bool AddRowSelection(int row) = 0;

  /**
   * @brief Selects a column.
   *
   * @param[in] column Column number
   *
   * @return True on success, false otherwise
   */
  virtual bool AddColumnSelection(int column) = 0;

  /**
   * @brief Unselects a row.
   *
   * @param[in] row Row number
   *
   * @return True on success, false otherwise
   */
  virtual bool RemoveRowSelection(int row) = 0;

  /**
   * @brief Unselects a column.
   *
   * @param[in] column Column number
   *
   * @return True on success, false otherwise
   */
  virtual bool RemoveColumnSelection(int column) = 0;

  /**
   * @brief Returns position and span information about a cell.
   *
   * @param[in] childIndex One-dimensional index of the cell
   *
   * @return The 'status' field of the returned structure is true on success, false otherwise
   *
   * @see Table::RowColumnSpanType
   */
  virtual RowColumnSpanType GetRowColumnSpan(std::size_t childIndex) const = 0;

  /**
   * @brief Downcasts an Accessible to a Table.
   *
   * @param obj The Accessible
   * @return A Table or null
   *
   * @see Dali::Accessibility::Accessible::DownCast()
   */
  static inline Table* DownCast(Accessible* obj);
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::TABLE>
{
  using Type = Table;
};
} // namespace Internal

inline Table* Table::DownCast(Accessible* obj)
{
  return Accessible::DownCast<AtspiInterface::TABLE>(obj);
}

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_TABLE_H
