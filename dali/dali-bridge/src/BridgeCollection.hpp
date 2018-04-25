#ifndef BRIDGE_COLLECTION_HPP
#define BRIDGE_COLLECTION_HPP

#include "BridgeBase.hpp"

#include <array>
#include <tuple>
#include <unordered_map>
#include <vector>

class BridgeCollection : public virtual BridgeBase
{
private:
  struct Comparer;
  static void VisitNodes( Dali::Accessibility::Accessible* obj, std::vector< Dali::Accessibility::Accessible* >& result, Comparer& cmp, size_t maxCount );

protected:
  BridgeCollection() = default;

  void RegisterInterfaces();

  Dali::Accessibility::Collection* FindSelf() const;

public:
  using MatchRule = std::tuple<
      std::array< int32_t, 2 >, int32_t,
      std::unordered_map< std::string, std::string >, int32_t,
      std::array< int32_t, 4 >, int32_t,
      std::vector< std::string >, int32_t,
      bool >;
  struct Index
  {
    enum
    {
      States,
      StatesMatchType,
      Attributes,
      AttributesMatchType,
      Roles,
      RolesMatchType,
      Interfaces,
      InterfacesMatchType,
    };
  };

  DBus::ValueOrError< std::vector< Dali::Accessibility::Accessible* > > GetMatches( MatchRule rule, uint32_t sortBy, int32_t count, bool traverse );
};

#endif
