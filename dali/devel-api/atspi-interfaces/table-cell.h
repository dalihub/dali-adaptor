#ifndef DALI_ADAPTOR_ATSPI_TABLE_CELL_H
#define DALI_ADAPTOR_ATSPI_TABLE_CELL_H

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
#include <utility>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>

namespace Dali::Accessibility
{
class Table;

/**
 * @brief Interface representing a table cell.
 *
 * @see Dali::Accessibility::Table
 */
class DALI_ADAPTOR_API TableCell
{
public:
  /**
   * @brief Cell information type.
   *
   * @see TableCell:GetCellRowColumnSpan()
   */
  struct RowColumnSpanType
  {
    int row;        ///< Row index
    int column;     ///< Column index
    int rowSpan;    ///< Row span
    int columnSpan; ///< Column span
  };

  /**
   * @brief Returns the table this cell belongs to.
   *
   * @return The table
   */
  virtual Table* GetTable() const;

  /**
   * @brief Returns the position of this cell in the table.
   *
   * @return A pair of integers (row index, column index)
   */
  virtual std::pair<int, int> GetCellPosition() const;

  /**
   * @brief Returns the number of rows occupied by this cell.
   *
   * @return Number of rows
   */
  virtual int GetCellRowSpan() const;

  /**
   * @brief Returns the number of columns occupied by this cell.
   *
   * @return Number of columns
   */
  virtual int GetCellColumnSpan() const;

  /**
   * @brief Returns the position, row span, and column span of this cell.
   *
   * @return Cell information
   *
   * @see TableCell::RowColumnSpanType
   * @see TableCell::GetCellPosition()
   * @see TableCell::GetCellRowSpan()
   * @see TableCell::GetCellColumnSpan()
   */
  virtual RowColumnSpanType GetCellRowColumnSpan() const;
};

namespace Internal
{
template<>
struct AtspiInterfaceTypeHelper<AtspiInterface::TABLE_CELL>
{
  using Type = TableCell;
};
} // namespace Internal

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_ATSPI_TABLE_CELL_H
