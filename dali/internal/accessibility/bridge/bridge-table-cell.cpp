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

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-table-cell.h>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/table.h>

using namespace Dali::Accessibility;

void BridgeTableCell::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::TABLE_CELL)};

  AddGetPropertyToInterface(desc, "Table", &BridgeTableCell::GetTable);
  AddGetPropertyToInterface(desc, "Position", &BridgeTableCell::GetCellPosition);
  AddGetPropertyToInterface(desc, "RowSpan", &BridgeTableCell::GetCellRowSpan);
  AddGetPropertyToInterface(desc, "ColumnSpan", &BridgeTableCell::GetCellColumnSpan);
  AddFunctionToInterface(desc, "GetRowColumnSpan", &BridgeTableCell::GetCellRowColumnSpan);

  mDbusServer.addInterface("/", desc, true);
}

TableCell* BridgeTableCell::FindSelf() const
{
  return FindCurrentObjectWithInterface<AtspiInterface::TABLE_CELL>();
}

DBus::ValueOrError<Accessible*> BridgeTableCell::GetTable()
{
  return dynamic_cast<Dali::Accessibility::Accessible*>(FindSelf()->GetTable());
}

DBus::ValueOrError<std::int32_t, std::int32_t> BridgeTableCell::GetCellPosition()
{
  std::pair<int, int> position = FindSelf()->GetCellPosition();

  return {position.first, position.second};
}

DBus::ValueOrError<std::int32_t> BridgeTableCell::GetCellRowSpan()
{
  return FindSelf()->GetCellRowSpan();
}

DBus::ValueOrError<std::int32_t> BridgeTableCell::GetCellColumnSpan()
{
  return FindSelf()->GetCellColumnSpan();
}

DBus::ValueOrError<std::int32_t, std::int32_t, std::int32_t, std::int32_t> BridgeTableCell::GetCellRowColumnSpan()
{
  TableCell::RowColumnSpanType span = FindSelf()->GetCellRowColumnSpan();

  return {span.row, span.column, span.rowSpan, span.columnSpan};
}
