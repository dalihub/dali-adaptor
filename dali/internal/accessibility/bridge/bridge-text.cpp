/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceText};
  AddFunctionToInterface( desc, "GetText", &BridgeText::GetText );
  AddGetPropertyToInterface( desc, "CharacterCount", &BridgeText::GetCharacterCount );
  AddGetPropertyToInterface( desc, "CaretOffset", &BridgeText::GetCaretOffset );
  AddFunctionToInterface( desc, "SetCaretOffset", &BridgeText::SetCaretOffset );
  AddFunctionToInterface( desc, "GetTextAtOffset", &BridgeText::GetTextAtOffset );
  AddFunctionToInterface( desc, "GetSelection", &BridgeText::GetSelection );
  AddFunctionToInterface( desc, "SetSelection", &BridgeText::SetSelection );
  AddFunctionToInterface( desc, "RemoveSelection", &BridgeText::RemoveSelection );
  dbusServer.addInterface( "/", desc, true );
}

Text* BridgeText::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Text* >( s );
  if( !s2 )
    throw std::domain_error{"object " + s->GetAddress().ToString() + " doesn't have Text interface"};
  return s2;
}

DBus::ValueOrError< std::string > BridgeText::GetText( int startOffset, int endOffset )
{
  return FindSelf()->GetText( startOffset, endOffset );
}

DBus::ValueOrError< int32_t > BridgeText::GetCharacterCount()
{
  return FindSelf()->GetCharacterCount();
}

DBus::ValueOrError< int32_t > BridgeText::GetCaretOffset()
{
  return FindSelf()->GetCaretOffset();
}

DBus::ValueOrError< bool > BridgeText::SetCaretOffset( int32_t offset )
{
  return FindSelf()->SetCaretOffset(offset);
}

DBus::ValueOrError< std::string, int, int > BridgeText::GetTextAtOffset( int32_t offset, uint32_t boundary )
{
  auto r = FindSelf()->GetTextAtOffset( offset, static_cast< TextBoundary >( boundary ) );
  return {r.content, static_cast< int >( r.startOffset ), static_cast< int >( r.endOffset )};
}

DBus::ValueOrError< int, int > BridgeText::GetSelection( int32_t selectionNum )
{
  auto r = FindSelf()->GetSelection( selectionNum );
  return {static_cast< int >( r.startOffset ), static_cast< int >( r.endOffset )};
}

DBus::ValueOrError< bool > BridgeText::RemoveSelection( int32_t selectionNum )
{
  return FindSelf()->RemoveSelection( selectionNum );
}

DBus::ValueOrError< bool > BridgeText::SetSelection( int32_t selectionNum, int32_t startOffset, int32_t endOffset )
{
  return FindSelf()->SetSelection( selectionNum, startOffset, endOffset );
}
