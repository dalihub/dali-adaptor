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
 *
 */

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-text.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-factory.h>

using namespace Dali::Accessibility;

void BridgeText::RegisterInterfaces()
{
  // The second arguments below are the names (or signatures) of DBus methods.
  // Screen Reader will call the methods with the exact names as specified in the AT-SPI Text interface:
  // https://gitlab.gnome.org/GNOME/at-spi2-core/-/blob/master/xml/Text.xml

  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::TEXT)};
  AddFunctionToInterface(desc, "GetText", &BridgeText::GetText);
  AddGetPropertyToInterface(desc, "CharacterCount", &BridgeText::GetCharacterCount);
  AddGetPropertyToInterface(desc, "CaretOffset", &BridgeText::GetCursorOffset);
  AddFunctionToInterface(desc, "SetCaretOffset", &BridgeText::SetCursorOffset);
  AddFunctionToInterface(desc, "GetTextAtOffset", &BridgeText::GetTextAtOffset);
  AddFunctionToInterface(desc, "GetSelection", &BridgeText::GetRangeOfSelection);
  AddFunctionToInterface(desc, "SetSelection", &BridgeText::SetRangeOfSelection);
  AddFunctionToInterface(desc, "RemoveSelection", &BridgeText::RemoveSelection);
  mDbusServer.addInterface("/", desc, true);
}

Text* BridgeText::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::TEXT>();
}

DBus::ValueOrError<std::string> BridgeText::GetText(int startOffset, int endOffset)
{
  return FindSelf()->GetText(startOffset, endOffset);
}

DBus::ValueOrError<int32_t> BridgeText::GetCharacterCount()
{
  return FindSelf()->GetCharacterCount();
}

DBus::ValueOrError<int32_t> BridgeText::GetCursorOffset()
{
  return FindSelf()->GetCursorOffset();
}

DBus::ValueOrError<bool> BridgeText::SetCursorOffset(int32_t offset)
{
  return FindSelf()->SetCursorOffset(offset);
}

DBus::ValueOrError<std::string, int, int> BridgeText::GetTextAtOffset(int32_t offset, uint32_t boundary)
{
  auto range = FindSelf()->GetTextAtOffset(offset, static_cast<TextBoundary>(boundary));
  return {range.content, static_cast<int>(range.startOffset), static_cast<int>(range.endOffset)};
}

DBus::ValueOrError<int, int> BridgeText::GetRangeOfSelection(int32_t selectionIndex)
{
  auto range = FindSelf()->GetRangeOfSelection(selectionIndex);
  return {static_cast<int>(range.startOffset), static_cast<int>(range.endOffset)};
}

DBus::ValueOrError<bool> BridgeText::RemoveSelection(int32_t selectionIndex)
{
  return FindSelf()->RemoveSelection(selectionIndex);
}

DBus::ValueOrError<bool> BridgeText::SetRangeOfSelection(int32_t selectionIndex, int32_t startOffset, int32_t endOffset)
{
  return FindSelf()->SetRangeOfSelection(selectionIndex, startOffset, endOffset);
}
