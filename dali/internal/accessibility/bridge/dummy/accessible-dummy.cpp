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

//INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>

// ALL API WHAT ACCESSIBLE INHERITTED
namespace Dali::Accessibility
{
// Action
std::string Action::GetActionName(std::size_t index) const
{
  return {};
}
std::string Action::GetLocalizedActionName(std::size_t index) const
{
  return {};
}
std::string Action::GetActionDescription(std::size_t index) const
{
  return {};
}
std::string Action::GetActionKeyBinding(std::size_t index) const
{
  return {};
}
std::size_t Action::GetActionCount() const
{
  return 0;
}
bool Action::DoAction(std::size_t index)
{
  return false;
}
bool Action::DoAction(const std::string& name)
{
  return false;
}

// Application
std::string Application::GetToolkitName() const
{
  return {};
}
std::string Application::GetVersion() const
{
  return {};
}
bool Application::GetIncludeHidden() const
{
  return false;
}
bool Application::SetIncludeHidden(bool includeHidden)
{
  return false;
}

// Component
Dali::Rect<float> Component::GetExtents(Accessibility::CoordinateType type) const
{
  return {};
}
Accessibility::ComponentLayer Component::GetLayer() const
{
  return {};
}
bool Component::ClearHighlight()
{
  return false;
}
bool Component::GrabHighlight()
{
  return false;
}
int16_t Component::GetMdiZOrder() const
{
  return 0;
}
bool Component::GrabFocus()
{
  return false;
}
double Component::GetAlpha() const
{
  return 0;
}

bool Component::IsScrollable() const
{
  return false;
}

// Text
std::string Text::GetText(std::size_t startOffset, std::size_t endOffset) const
{
  return {};
}
std::size_t Text::GetCharacterCount() const
{
  return 0;
}
std::size_t Text::GetCursorOffset() const
{
  return 0;
}
bool Text::SetCursorOffset(std::size_t offset)
{
  return false;
}
Accessibility::Range Text::GetTextAtOffset(std::size_t offset, Accessibility::TextBoundary boundary) const
{
  return {};
}
Accessibility::Range Text::GetRangeOfSelection(std::size_t selectionIndex) const
{
  return {};
}
bool Text::RemoveSelection(std::size_t selectionIndex)
{
  return false;
}
bool Text::SetRangeOfSelection(std::size_t selectionIndex, std::size_t startOffset, std::size_t endOffset)
{
  return false;
}
Rect<float> Text::GetRangeExtents(std::size_t startOffset, std::size_t endOffset, CoordinateType type)
{
  return {};
}

// EditableText
bool EditableText::CopyText(std::size_t startPosition, std::size_t endPosition)
{
  return false;
}
bool EditableText::CutText(std::size_t startPosition, std::size_t endPosition)
{
  return false;
}
bool EditableText::DeleteText(std::size_t startPosition, std::size_t endPosition)
{
  return false;
}
bool EditableText::InsertText(std::size_t startPosition, std::string text)
{
  return false;
}
bool EditableText::SetTextContents(std::string newContents)
{
  return false;
}

// Hyperlink
std::int32_t Hyperlink::GetEndIndex() const
{
  return 0;
}
std::int32_t Hyperlink::GetStartIndex() const
{
  return 0;
}
std::int32_t Hyperlink::GetAnchorCount() const
{
  return 0;
}
Accessible* Hyperlink::GetAnchorAccessible(std::int32_t anchorIndex) const
{
  return nullptr;
}
std::string Hyperlink::GetAnchorUri(std::int32_t anchorIndex) const
{
  return {};
}
bool Hyperlink::IsValid() const
{
  return false;
}

// Hypertext
Hyperlink* Hypertext::GetLink(std::int32_t linkIndex) const
{
  return nullptr;
}
std::int32_t Hypertext::GetLinkIndex(std::int32_t characterOffset) const
{
  return 0;
}
std::int32_t Hypertext::GetLinkCount() const
{
  return 0;
}

// Selection
int Selection::GetSelectedChildrenCount() const
{
  return 0;
}
Accessible* Selection::GetSelectedChild(int selectedChildIndex)
{
  return nullptr;
}
bool Selection::SelectChild(int childIndex)
{
  return false;
}
bool Selection::DeselectSelectedChild(int selectedChildIndex)
{
  return false;
}
bool Selection::IsChildSelected(int childIndex) const
{
  return false;
}
bool Selection::SelectAll()
{
  return false;
}
bool Selection::ClearSelection()
{
  return false;
}
bool Selection::DeselectChild(int childIndex)
{
  return false;
}

// Socket
Address Socket::Embed(Address plug)
{
  return {};
}
void Socket::Unembed(Address plug)
{
}
void Socket::SetOffset(std::int32_t x, std::int32_t y)
{
}

// TableCell
Table* TableCell::GetTable() const
{
  return nullptr;
}
std::pair<int, int> TableCell::GetCellPosition() const
{
  return {};
}
int TableCell::GetCellRowSpan() const
{
  return 0;
}
int TableCell::GetCellColumnSpan() const
{
  return 0;
}
TableCell::RowColumnSpanType TableCell::GetCellRowColumnSpan() const
{
  return {};
}

// Table
int Table::GetRowCount() const
{
  return 0;
}
int Table::GetColumnCount() const
{
  return 0;
}
int Table::GetSelectedRowCount() const
{
  return 0;
}
int Table::GetSelectedColumnCount() const
{
  return 0;
}
Accessible* Table::GetCaption() const
{
  return nullptr;
}
Accessible* Table::GetSummary() const
{
  return nullptr;
}
TableCell* Table::GetCell(int row, int column) const
{
  return nullptr;
}
std::size_t Table::GetChildIndex(int row, int column) const
{
  return 0;
}
int Table::GetRowByChildIndex(std::size_t childIndex) const
{
  return 0;
}
int Table::GetColumnByChildIndex(std::size_t childIndex) const
{
  return 0;
}
std::string Table::GetRowDescription(int row) const
{
  return {};
}
std::string Table::GetColumnDescription(int column) const
{
  return {};
}
int Table::GetRowSpan(int row, int column) const
{
  return 0;
}
int Table::GetColumnSpan(int row, int column) const
{
  return 0;
}
Accessible* Table::GetRowHeader(int row) const
{
  return nullptr;
}
Accessible* Table::GetColumnHeader(int column) const
{
  return nullptr;
}
std::vector<int> Table::GetSelectedRows() const
{
  return {};
}
std::vector<int> Table::GetSelectedColumns() const
{
  return {};
}
bool Table::IsRowSelected(int row) const
{
  return false;
}
bool Table::IsColumnSelected(int column) const
{
  return false;
}
bool Table::IsCellSelected(int row, int column) const
{
  return false;
}
bool Table::AddRowSelection(int row)
{
  return false;
}
bool Table::AddColumnSelection(int column)
{
  return false;
}
bool Table::RemoveRowSelection(int row)
{
  return false;
}
bool Table::RemoveColumnSelection(int column)
{
  return false;
}
Table::RowColumnSpanType Table::GetRowColumnSpan(std::size_t childIndex) const
{
  return {};
}

// Value
double Value::GetMinimum() const
{
  return 0;
}
double Value::GetCurrent() const
{
  return 0;
}
std::string Value::GetValueText() const
{
  return {};
}
double Value::GetMaximum() const
{
  return 0;
}
bool Value::SetCurrent(double value)
{
  return false;
}
double Value::GetMinimumIncrement() const
{
  return 0;
}
} //namespace Dali::Accessibility
