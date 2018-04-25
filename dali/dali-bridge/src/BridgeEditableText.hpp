#ifndef BRIDGE_EDITABLE_TEXT_HPP
#define BRIDGE_EDITABLE_TEXT_HPP

#include "BridgeBase.hpp"

class BridgeEditableText : public virtual BridgeBase
{
protected:
  BridgeEditableText() = default;

  void RegisterInterfaces();

  Dali::Accessibility::EditableText* FindSelf() const;

public:
  DBus::ValueOrError< bool > CopyText( int32_t startPos, int32_t endPos );
  DBus::ValueOrError< bool > CutText( int32_t startPos, int32_t endPos );
  DBus::ValueOrError< bool > PasteText( int32_t pos );
};

#endif
