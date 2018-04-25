#ifndef BRIDGE_COMPONENT_HPP
#define BRIDGE_COMPONENT_HPP

#include "BridgeBase.hpp"
#include <array>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class BridgeComponent : public virtual BridgeBase
{
protected:
  BridgeComponent();

  void RegisterInterfaces();

  Dali::Accessibility::Component* FindSelf() const;

public:
  DBus::ValueOrError< bool > Contains( int32_t x, int32_t y, uint32_t coordType );
  DBus::ValueOrError< Dali::Accessibility::Accessible* > GetAccessibleAtPoint( int32_t x, int32_t y, uint32_t coordType );
  DBus::ValueOrError< std::tuple< int32_t, int32_t, int32_t, int32_t > > GetExtents( uint32_t coordType );
  DBus::ValueOrError< int32_t, int32_t > GetPosition( uint32_t coordType );
  DBus::ValueOrError< int32_t, int32_t > GetSize( uint32_t coordType );
  DBus::ValueOrError< Dali::Accessibility::ComponentLayer > GetLayer();
  DBus::ValueOrError< double > GetAlpha();
  DBus::ValueOrError< bool > GrabHighlight();
  DBus::ValueOrError< bool > ClearHighlight();
};

#endif
