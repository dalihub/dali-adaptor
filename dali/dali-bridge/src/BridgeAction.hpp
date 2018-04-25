#ifndef BRIDGE_ACTION_HPP
#define BRIDGE_ACTION_HPP

#include "BridgeBase.hpp"

#include <tuple>
#include <vector>

class BridgeAction : public virtual BridgeBase
{
protected:
  BridgeAction() = default;

  void RegisterInterfaces();

  Dali::Accessibility::Action* FindSelf() const;

public:
  DBus::ValueOrError< std::string > GetActionName( int32_t index );
  DBus::ValueOrError< std::string > GetLocalizedActionName( int32_t index );
  DBus::ValueOrError< std::string > GetActionDescription( int32_t index );
  DBus::ValueOrError< std::string > GetActionKeyBinding( int32_t index );
  DBus::ValueOrError< int32_t > GetActionCount();
  DBus::ValueOrError< bool > DoAction( int32_t index );
  DBus::ValueOrError< bool > DoActionName( std::string name );
};

#endif
