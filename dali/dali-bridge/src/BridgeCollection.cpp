#include "BridgeCollection.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace Dali::Accessibility;

void BridgeCollection::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{ATSPI_DBUS_INTERFACE_COLLECTION};
  AddFunctionToInterface( desc, "GetMatches", &BridgeCollection::GetMatches );
  dbusServer.addInterface( "/", desc, true );
}

Collection* BridgeCollection::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Collection* >( s );
  if( !s2 )
    throw AccessibleError{"object " + s->GetAddress().ToString() + " doesn't have Collection interface"};
  return s2;
}

enum
{
  ATSPI_Collection_MATCH_INVALID,
  ATSPI_Collection_MATCH_ALL,
  ATSPI_Collection_MATCH_ANY,
  ATSPI_Collection_MATCH_NONE,
  ATSPI_Collection_MATCH_EMPTY,
  ATSPI_Collection_MATCH_LAST_DEFINED,
};

struct BridgeCollection::Comparer
{
  using Mode = MatchType;

  enum class CompareFuncExit
  {
    firstFound,
    firstNotFound
  };

  static Mode ConvertToMatchType( int32_t mode )
  {
    switch( mode )
    {
      case ATSPI_Collection_MATCH_INVALID:
      {
        return Mode::Invalid;
      }
      case ATSPI_Collection_MATCH_ALL:
      {
        return Mode::All;
      }
      case ATSPI_Collection_MATCH_ANY:
      {
        return Mode::Any;
      }
      case ATSPI_Collection_MATCH_NONE:
      {
        return Mode::None;
      }
      case ATSPI_Collection_MATCH_EMPTY:
      {
        return Mode::Empty;
      }
    }
    return Mode::Invalid;
  }

  struct ComparerInterfaces
  {
    std::unordered_set< std::string > object;
    std::vector< std::string > requested;
    Mode mode = Mode::Invalid;

    ComparerInterfaces( MatchRule* rule ) : mode( ConvertToMatchType( std::get< Index::InterfacesMatchType >( *rule ) ) )
    {
      requested = {std::get< Index::Interfaces >( *rule ).begin(), std::get< Index::Interfaces >( *rule ).end()};
    }
    void Update( Accessible* obj )
    {
      object.clear();
      for( auto& q : obj->GetInterfaces() )
        object.insert( std::move( q ) );
    }
    bool RequestEmpty() const { return requested.empty(); }
    bool ObjectEmpty() const { return object.empty(); }
    bool Compare( CompareFuncExit exit )
    {
      bool foundAny = false;
      for( auto& iname : requested )
      {
        bool found = ( object.find( iname ) != object.end() );
        if( found )
          foundAny = true;
        if( found == ( exit == CompareFuncExit::firstFound ) )
          return found;
      }
      return foundAny;
    }
  };
  struct ComparerAttributes
  {
    std::unordered_map< std::string, std::string > requested, object;
    Mode mode = Mode::Invalid;

    ComparerAttributes( MatchRule* rule ) : mode( ConvertToMatchType( std::get< Index::AttributesMatchType >( *rule ) ) )
    {
      requested = std::get< Index::Attributes >( *rule );
    }
    void Update( Accessible* obj )
    {
      object = obj->GetAttributes();
    }
    bool RequestEmpty() const { return requested.empty(); }
    bool ObjectEmpty() const { return object.empty(); }
    bool Compare( CompareFuncExit exit )
    {
      bool foundAny = false;
      for( auto& iname : requested )
      {
        auto it = object.find( iname.first );
        bool found = it != object.end() && iname.second == it->second;
        if( found )
          foundAny = true;
        if( found == ( exit == CompareFuncExit::firstFound ) )
        {
          return found;
        }
      }
      return foundAny;
    }
  };
  struct ComparerRoles
  {
    using Roles = BitStates< 4, Role >;
    Roles requested, object;
    Mode mode = Mode::Invalid;

    ComparerRoles( MatchRule* rule ) : mode( ConvertToMatchType( std::get< Index::RolesMatchType >( *rule ) ) )
    {
      requested = Roles{std::get< Index::Roles >( *rule )};
    }
    void Update( Accessible* obj )
    {
      object = {};
      object[obj->GetRole()] = true;
      assert( object );
    }
    bool RequestEmpty() const { return !requested; }
    bool ObjectEmpty() const { return !object; }
    bool Compare( CompareFuncExit exit )
    {
      switch( mode )
      {
        case Mode::Invalid:
        {
          return true;
        }
        case Mode::Empty:
        case Mode::All:
        {
          return requested == ( object & requested );
        }
        case Mode::Any:
        {
          return bool( object & requested );
        }
        case Mode::None:
        {
          return bool( object & requested );
        }
      }
      return false;
    }
  };
  struct ComparerStates
  {
    States requested, object;
    Mode mode = Mode::Invalid;

    ComparerStates( MatchRule* rule ) : mode( ConvertToMatchType( std::get< Index::StatesMatchType >( *rule ) ) )
    {
      requested = States{std::get< Index::States >( *rule )};
    }
    void Update( Accessible* obj )
    {
      object = obj->GetStates();
    }
    bool RequestEmpty() const { return !requested; }
    bool ObjectEmpty() const { return !object; }
    bool Compare( CompareFuncExit exit )
    {
      switch( mode )
      {
        case Mode::Invalid:
        {
          return true;
        }
        case Mode::Empty:
        case Mode::All:
        {
          return requested == ( object & requested );
        }
        case Mode::Any:
        {
          return bool( object & requested );
        }
        case Mode::None:
        {
          return bool( object & requested );
        }
      }
      return false;
    }
  };

  template < typename T >
  bool compareFunc( T& cmp, Accessible* obj )
  {
    if( cmp.mode == Mode::Invalid )
      return true;
    cmp.Update( obj );
    switch( cmp.mode )
    {
      case Mode::Any:
      {
        if( cmp.RequestEmpty() || cmp.ObjectEmpty() )
          return false;
        break;
      }
      case Mode::All:
      {
        if( cmp.RequestEmpty() )
          return true;
        if( cmp.ObjectEmpty() )
          return false;
        break;
      }
      case Mode::None:
      {
        if( cmp.RequestEmpty() || cmp.ObjectEmpty() )
          return true;
        break;
      }
      case Mode::Empty:
      {
        if( cmp.RequestEmpty() && cmp.ObjectEmpty() )
          return true;
        if( cmp.RequestEmpty() || cmp.ObjectEmpty() )
          return false;
        break;
      }
      case Mode::Invalid:
      {
        return true;
      }
    }

    switch( cmp.mode )
    {
      case Mode::Empty:
      case Mode::All:
      {
        if( !cmp.Compare( CompareFuncExit::firstNotFound ) )
          return false;
        break;
      }
      case Mode::Any:
      {
        if( cmp.Compare( CompareFuncExit::firstFound ) )
          return true;
        break;
      }
      case Mode::None:
      {
        if( cmp.Compare( CompareFuncExit::firstFound ) )
          return false;
        break;
      }
      case Mode::Invalid:
      {
        return true;
      }
    }
    switch( cmp.mode )
    {
      case Mode::Empty:
      case Mode::All:
      case Mode::None:
      {
        return true;
      }
      case Mode::Any:
      {
        return false;
      }
      case Mode::Invalid:
      {
        return true;
      }
    }
    return false;
  }

  ComparerInterfaces ci;
  ComparerAttributes ca;
  ComparerRoles cr;
  ComparerStates cs;

  Comparer( MatchRule* mr ) : ci( mr ), ca( mr ), cr( mr ), cs( mr ) {}

  bool operator()( Accessible* obj )
  {
    return compareFunc( ci, obj ) &&
           compareFunc( ca, obj ) &&
           compareFunc( cr, obj ) &&
           compareFunc( cs, obj );
  }
};

void BridgeCollection::VisitNodes( Accessible* obj, std::vector< Accessible* >& result, Comparer& cmp, size_t maxCount )
{
  if( result.size() >= maxCount )
    return;

  if( cmp( obj ) )
    result.emplace_back( obj );

  for( auto i = 0u; i < obj->GetChildCount(); ++i )
    VisitNodes( obj->GetChildAtIndex( i ), result, cmp, maxCount );
}

DBus::ValueOrError< std::vector< Accessible* > > BridgeCollection::GetMatches( MatchRule rule, uint32_t sortBy, int32_t count, bool traverse )
{
  std::vector< Accessible* > res;
  auto self = BridgeBase::FindSelf();
  auto matcher = Comparer{&rule};
  VisitNodes( self, res, matcher, count );

  switch( static_cast< SortOrder >( sortBy ) )
  {
    case SortOrder::Canonical:
    {
      break;
    }

    case SortOrder::ReverseCanonical:
    {
      std::reverse( res.begin(), res.end() );
      break;
    }

    default:
    {
      throw AccessibleError{"unsupported sorting order"};
    }
      //TODO: other cases
  }

  return res;
}
