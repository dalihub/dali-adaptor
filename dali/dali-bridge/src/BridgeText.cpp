#include "BridgeText.hpp"

#include <dali/internal/input/common/imf-manager-impl.h>

using namespace Dali::Accessibility;

void BridgeText::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{ATSPI_DBUS_INTERFACE_TEXT};
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
    throw AccessibleError{"object " + s->GetAddress().ToString() + " doesn't have Text interface"};
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
  auto imfManager = Dali::Internal::Adaptor::ImfManager::Get();
  return imfManager.GetCursorPosition();
}

DBus::ValueOrError< bool > BridgeText::SetCaretOffset( int32_t offset )
{
  auto imfManager = Dali::Internal::Adaptor::ImfManager::Get();
  imfManager.SetCursorPosition( offset );
  return true;
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
