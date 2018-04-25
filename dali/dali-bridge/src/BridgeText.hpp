#ifndef BRIDGE_TEXT_HPP
#define BRIDGE_TEXT_HPP

#include "BridgeBase.hpp"

class BridgeText : public virtual BridgeBase
{
protected:
  BridgeText() = default;

  void RegisterInterfaces();

  Dali::Accessibility::Text* FindSelf() const;

public:
  DBus::ValueOrError< std::string > GetText( int startOffset, int endOffset );
  DBus::ValueOrError< int32_t > GetCharacterCount();
  DBus::ValueOrError< int32_t > GetCaretOffset();
  DBus::ValueOrError< bool > SetCaretOffset( int32_t offset );
  DBus::ValueOrError< std::string, int, int > GetTextAtOffset( int32_t offset, uint32_t boundary );
  DBus::ValueOrError< int, int > GetSelection( int32_t selectionNum );
  DBus::ValueOrError< bool > RemoveSelection( int32_t selectionNum );
  DBus::ValueOrError< bool > SetSelection( int32_t selectionNum, int32_t startOffset, int32_t endOffset );
};

#endif
