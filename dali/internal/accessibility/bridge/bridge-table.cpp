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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-table.h>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/table-cell.h>

using namespace Dali::Accessibility;

void BridgeTable::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::TABLE)};

  AddGetPropertyToInterface(desc, "NRows", &BridgeTable::GetRowCount);
  AddGetPropertyToInterface(desc, "NColumns", &BridgeTable::GetColumnCount);
  AddGetPropertyToInterface(desc, "NSelectedRows", &BridgeTable::GetSelectedRowCount);
  AddGetPropertyToInterface(desc, "NSelectedColumns", &BridgeTable::GetSelectedColumnCount);
  AddGetPropertyToInterface(desc, "Caption", &BridgeTable::GetCaption);
  AddGetPropertyToInterface(desc, "Summary", &BridgeTable::GetSummary);
  AddFunctionToInterface(desc, "GetAccessibleAt", &BridgeTable::GetCell);
  AddFunctionToInterface(desc, "GetIndexAt", &BridgeTable::GetChildIndex);
  AddFunctionToInterface(desc, "GetRowAtIndex", &BridgeTable::GetRowByChildIndex);
  AddFunctionToInterface(desc, "GetColumnAtIndex", &BridgeTable::GetColumnByChildIndex);
  AddFunctionToInterface(desc, "GetRowDescription", &BridgeTable::GetRowDescription);
  AddFunctionToInterface(desc, "GetColumnDescription", &BridgeTable::GetColumnDescription);
  AddFunctionToInterface(desc, "GetRowExtentAt", &BridgeTable::GetRowSpan);
  AddFunctionToInterface(desc, "GetColumnExtentAt", &BridgeTable::GetColumnSpan);
  AddFunctionToInterface(desc, "GetRowHeader", &BridgeTable::GetRowHeader);
  AddFunctionToInterface(desc, "GetColumnHeader", &BridgeTable::GetColumnHeader);
  AddFunctionToInterface(desc, "GetSelectedRows", &BridgeTable::GetSelectedRows);
  AddFunctionToInterface(desc, "GetSelectedColumns", &BridgeTable::GetSelectedColumns);
  AddFunctionToInterface(desc, "IsRowSelected", &BridgeTable::IsRowSelected);
  AddFunctionToInterface(desc, "IsColumnSelected", &BridgeTable::IsColumnSelected);
  AddFunctionToInterface(desc, "IsSelected", &BridgeTable::IsCellSelected);
  AddFunctionToInterface(desc, "AddRowSelection", &BridgeTable::AddRowSelection);
  AddFunctionToInterface(desc, "AddColumnSelection", &BridgeTable::AddColumnSelection);
  AddFunctionToInterface(desc, "RemoveRowSelection", &BridgeTable::RemoveRowSelection);
  AddFunctionToInterface(desc, "RemoveColumnSelection", &BridgeTable::RemoveColumnSelection);
  AddFunctionToInterface(desc, "GetRowColumnExtentsAtIndex", &BridgeTable::GetRowColumnSpan);

  mDbusServer.addInterface("/", desc, true);
}

Table* BridgeTable::FindSelf() const
{
  return FindCurrentObjectWithInterface<AtspiInterface::TABLE>();
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetRowCount()
{
  return FindSelf()->GetRowCount();
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetColumnCount()
{
  return FindSelf()->GetColumnCount();
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetSelectedRowCount()
{
  return FindSelf()->GetSelectedRowCount();
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetSelectedColumnCount()
{
  return FindSelf()->GetSelectedColumnCount();
}

DBus::ValueOrError<Accessible*> BridgeTable::GetCaption()
{
  return FindSelf()->GetCaption();
}

DBus::ValueOrError<Accessible*> BridgeTable::GetSummary()
{
  return FindSelf()->GetSummary();
}

DBus::ValueOrError<Accessible*> BridgeTable::GetCell(std::int32_t row, std::int32_t column)
{
  return FindSelf()->GetCell(row, column);
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetChildIndex(std::int32_t row, std::int32_t column)
{
  return static_cast<std::int32_t>(FindSelf()->GetChildIndex(row, column));
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetRowByChildIndex(std::int32_t childIndex)
{
  return FindSelf()->GetRowByChildIndex(static_cast<std::size_t>(childIndex));
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetColumnByChildIndex(std::int32_t childIndex)
{
  return FindSelf()->GetColumnByChildIndex(static_cast<std::size_t>(childIndex));
}

DBus::ValueOrError<std::string> BridgeTable::GetRowDescription(std::int32_t row)
{
  return FindSelf()->GetRowDescription(row);
}

DBus::ValueOrError<std::string> BridgeTable::GetColumnDescription(std::int32_t column)
{
  return FindSelf()->GetColumnDescription(column);
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetRowSpan(std::int32_t row, std::int32_t column)
{
  return FindSelf()->GetRowSpan(row, column);
}

DBus::ValueOrError<std::int32_t> BridgeTable::GetColumnSpan(std::int32_t row, std::int32_t column)
{
  return FindSelf()->GetColumnSpan(row, column);
}

DBus::ValueOrError<Accessible*> BridgeTable::GetRowHeader(std::int32_t row)
{
  return FindSelf()->GetRowHeader(row);
}

DBus::ValueOrError<Accessible*> BridgeTable::GetColumnHeader(std::int32_t column)
{
  return FindSelf()->GetColumnHeader(column);
}

DBus::ValueOrError<std::vector<std::int32_t>> BridgeTable::GetSelectedRows()
{
  return FindSelf()->GetSelectedRows();
}

DBus::ValueOrError<std::vector<std::int32_t>> BridgeTable::GetSelectedColumns()
{
  return FindSelf()->GetSelectedColumns();
}

DBus::ValueOrError<bool> BridgeTable::IsRowSelected(std::int32_t row)
{
  return FindSelf()->IsRowSelected(row);
}

DBus::ValueOrError<bool> BridgeTable::IsColumnSelected(std::int32_t column)
{
  return FindSelf()->IsColumnSelected(column);
}

DBus::ValueOrError<bool> BridgeTable::IsCellSelected(std::int32_t row, std::int32_t column)
{
  return FindSelf()->IsCellSelected(row, column);
}

DBus::ValueOrError<bool> BridgeTable::AddRowSelection(std::int32_t row)
{
  return FindSelf()->AddRowSelection(row);
}

DBus::ValueOrError<bool> BridgeTable::AddColumnSelection(std::int32_t column)
{
  return FindSelf()->AddColumnSelection(column);
}

DBus::ValueOrError<bool> BridgeTable::RemoveRowSelection(std::int32_t row)
{
  return FindSelf()->RemoveRowSelection(row);
}

DBus::ValueOrError<bool> BridgeTable::RemoveColumnSelection(std::int32_t column)
{
  return FindSelf()->RemoveColumnSelection(column);
}

BridgeTable::RowColumnSpanType BridgeTable::GetRowColumnSpan(std::int32_t childIndex)
{
  Table::RowColumnSpanType span = FindSelf()->GetRowColumnSpan(static_cast<std::size_t>(childIndex));

  return {span.success, span.row, span.column, span.rowSpan, span.columnSpan, span.isSelected};
}
