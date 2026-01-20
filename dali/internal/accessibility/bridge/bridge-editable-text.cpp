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
#include <dali/internal/accessibility/bridge/bridge-editable-text.h>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>

using namespace Dali::Accessibility;

void BridgeEditableText::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::EDITABLE_TEXT)};
  AddFunctionToInterface(desc, "CopyText", &BridgeEditableText::CopyText);
  AddFunctionToInterface(desc, "CutText", &BridgeEditableText::CutText);
  AddFunctionToInterface(desc, "DeleteText", &BridgeEditableText::DeleteText);
  AddFunctionToInterface(desc, "InsertText", &BridgeEditableText::InsertText);
  AddFunctionToInterface(desc, "PasteText", &BridgeEditableText::PasteText);
  AddFunctionToInterface(desc, "SetTextContents", &BridgeEditableText::SetTextContents);
  mDbusServer.addInterface("/", desc, true);
}

std::shared_ptr<EditableText> BridgeEditableText::FindSelf() const
{
  return FindCurrentObjectWithInterface<Dali::Accessibility::AtspiInterface::EDITABLE_TEXT>();
}

DBus::ValueOrError<bool> BridgeEditableText::CopyText(int32_t startPosition, int32_t endPosition)
{
  return FindSelf()->CopyText(startPosition, endPosition);
}

DBus::ValueOrError<bool> BridgeEditableText::CutText(int32_t startPosition, int32_t endPosition)
{
  return FindSelf()->CutText(startPosition, endPosition);
}

DBus::ValueOrError<bool> BridgeEditableText::DeleteText(int32_t startPosition, int32_t endPosition)
{
  return FindSelf()->DeleteText(startPosition, endPosition);
}

DBus::ValueOrError<bool> BridgeEditableText::InsertText(int32_t startPosition, std::string text, [[maybe_unused]] int32_t length)
{
  return FindSelf()->InsertText(startPosition, std::move(text));
}

DBus::ValueOrError<bool> BridgeEditableText::PasteText(int32_t position)
{
  // auto imfManager = Dali::Internal::Adaptor::ImfManager::Get();
  // imfManager.SetCursorPosition( position );
  // auto clipboard = Dali::Internal::Adaptor::TextClipboard::Get();
  // clipboard.RequestItem();
  // return true;
  return false;
}

DBus::ValueOrError<bool> BridgeEditableText::SetTextContents(std::string newContents)
{
  return FindSelf()->SetTextContents(std::move(newContents));
}
