#ifndef BRIDGE_OBJECT_HPP
#define BRIDGE_OBJECT_HPP

#include "BridgeBase.hpp"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class BridgeObject : public virtual BridgeBase
{
protected:
  BridgeObject();

  void RegisterInterfaces();

  DBus::DBusInterfaceDescription::SignalId stateChanged;

  void EmitStateChanged( Dali::Accessibility::Accessible* obj, Dali::Accessibility::State state, int val1, int val2 ) override;
  void Emit( Dali::Accessibility::Accessible* obj, Dali::Accessibility::WindowEvent we, unsigned int detail1 ) override;

public:
  int GetChildCount();
  DBus::ValueOrError< Dali::Accessibility::Accessible* > GetChildAtIndex( int index );
  Dali::Accessibility::Accessible* GetParent();
  DBus::ValueOrError< std::vector< Dali::Accessibility::Accessible* > > GetChildren();
  std::string GetName();
  std::string GetDescription();
  DBus::ValueOrError< uint32_t > GetRole();
  DBus::ValueOrError< std::string > GetRoleName();
  DBus::ValueOrError< std::string > GetLocalizedRoleName();
  DBus::ValueOrError< int32_t > GetIndexInParent();
  DBus::ValueOrError< std::array< uint32_t, 2 > > GetStates();
  DBus::ValueOrError< std::unordered_map< std::string, std::string > > GetAttributes();
  DBus::ValueOrError< std::vector< std::string > > GetInterfaces();
};

#endif