#include "BridgeEditableText.hpp"

#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/input/common/imf-manager-impl.h>

using namespace Dali::Accessibility;

void BridgeEditableText::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{ATSPI_DBUS_INTERFACE_EDITABLE_TEXT};
  AddFunctionToInterface( desc, "CopyText", &BridgeEditableText::CopyText );
  AddFunctionToInterface( desc, "CutText", &BridgeEditableText::CutText );
  AddFunctionToInterface( desc, "PasteText", &BridgeEditableText::PasteText );
  dbusServer.addInterface( "/", desc, true );
}

EditableText* BridgeEditableText::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< EditableText* >( s );
  if( !s2 )
    throw AccessibleError{"object " + s->GetAddress().ToString() + " doesn't have Text interface"};
  return s2;
}

DBus::ValueOrError< bool > BridgeEditableText::CopyText( int32_t startPos, int32_t endPos )
{
  return FindSelf()->CopyText( startPos, endPos );
}

DBus::ValueOrError< bool > BridgeEditableText::CutText( int32_t startPos, int32_t endPos )
{
  return FindSelf()->CutText( startPos, endPos );
}

DBus::ValueOrError< bool > BridgeEditableText::PasteText( int32_t pos )
{
  auto imfManager = Dali::Internal::Adaptor::ImfManager::Get();
  imfManager.SetCursorPosition( pos );
  auto clipboard = Dali::Internal::Adaptor::Clipboard::Get();
  clipboard.RequestItem();

  return true;
}
