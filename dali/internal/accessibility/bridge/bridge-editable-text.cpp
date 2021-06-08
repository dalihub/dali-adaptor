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
#include <dali/internal/accessibility/bridge/bridge-editable-text.h>

// INTERNAL INCLUDES
#include <dali/internal/clipboard/common/clipboard-impl.h>

using namespace Dali::Accessibility;

void BridgeEditableText::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceEditableText};
  AddFunctionToInterface(desc, "CopyText", &BridgeEditableText::CopyText);
  AddFunctionToInterface(desc, "CutText", &BridgeEditableText::CutText);
  AddFunctionToInterface(desc, "DeleteText", &BridgeEditableText::DeleteText);
  AddFunctionToInterface(desc, "InsertText", &BridgeEditableText::InsertText);
  AddFunctionToInterface(desc, "PasteText", &BridgeEditableText::PasteText);
  AddFunctionToInterface(desc, "SetTextContents", &BridgeEditableText::SetTextContents);
  dbusServer.addInterface("/", desc, true);
}

EditableText* BridgeEditableText::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert(s);
  auto s2 = dynamic_cast<EditableText*>(s);
  if(!s2)
    throw std::domain_error{"object " + s->GetAddress().ToString() + " doesn't have Text interface"};
  return s2;
}

DBus::ValueOrError<bool> BridgeEditableText::CopyText(int32_t startPos, int32_t endPos)
{
  return FindSelf()->CopyText(startPos, endPos);
}

DBus::ValueOrError<bool> BridgeEditableText::CutText(int32_t startPos, int32_t endPos)
{
  return FindSelf()->CutText(startPos, endPos);
}

DBus::ValueOrError<bool> BridgeEditableText::DeleteText(int32_t startPos, int32_t endPos)
{
  return FindSelf()->DeleteText(startPos, endPos);
}

DBus::ValueOrError<bool> BridgeEditableText::InsertText(int32_t startPos, std::string text, [[maybe_unused]] int32_t length)
{
  return FindSelf()->InsertText(startPos, std::move(text));
}

DBus::ValueOrError<bool> BridgeEditableText::PasteText(int32_t pos)
{
  // auto imfManager = Dali::Internal::Adaptor::ImfManager::Get();
  // imfManager.SetCursorPosition( pos );
  // auto clipboard = Dali::Internal::Adaptor::Clipboard::Get();
  // clipboard.RequestItem();
  // return true;
  return false;
}

DBus::ValueOrError<bool> BridgeEditableText::SetTextContents(std::string newContents)
{
  return FindSelf()->SetTextContents(std::move(newContents));
}
