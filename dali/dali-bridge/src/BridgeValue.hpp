#ifndef BRIDGE_VALUE_HPP
#define BRIDGE_VALUE_HPP

#include "BridgeBase.hpp"
#include <array>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class BridgeValue : public virtual BridgeBase
{
protected:
  BridgeValue();

  void RegisterInterfaces();

  Dali::Accessibility::Value* FindSelf() const;

public:
  double GetCurrentValue();
  void SetCurrentValue( double new_value );
  double GetMaximumValue();
  double GetMinimumIncrement();
  double GetMinimumValue();
};

#endif
