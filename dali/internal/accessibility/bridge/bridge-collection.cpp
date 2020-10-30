/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include <dali/internal/accessibility/bridge/bridge-collection.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace Dali::Accessibility;

void BridgeCollection::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceCollection};
  AddFunctionToInterface( desc, "GetMatches", &BridgeCollection::GetMatches );
  dbusServer.addInterface( "/", desc, true );
}

Collection* BridgeCollection::FindSelf() const
{
  auto s = BridgeBase::FindSelf();
  assert( s );
  auto s2 = dynamic_cast< Collection* >( s );
  if( !s2 )
    throw std::domain_error{"object " + s->GetAddress().ToString() + " doesn't have Collection interface"};
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
    FIRST_FOUND,
    FIRST_NOT_FOUND
  };

  static Mode ConvertToMatchType( int32_t mode )
  {
    switch( mode )
    {
      case ATSPI_Collection_MATCH_INVALID:
      {
        return Mode::INVALID;
      }
      case ATSPI_Collection_MATCH_ALL:
      {
        return Mode::ALL;
      }
      case ATSPI_Collection_MATCH_ANY:
      {
        return Mode::ANY;
      }
      case ATSPI_Collection_MATCH_NONE:
      {
        return Mode::NONE;
      }
      case ATSPI_Collection_MATCH_EMPTY:
      {
        return Mode::EMPTY;
      }
    }
    return Mode::INVALID;
  }

  struct ComparerInterfaces
  {
    std::unordered_set< std::string > object;
    std::vector< std::string > requested;
    Mode mode = Mode::INVALID;

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
        if( found == ( exit == CompareFuncExit::FIRST_FOUND ) )
          return found;
      }
      return foundAny;
    }
  };
  struct ComparerAttributes
  {
    std::unordered_map< std::string, std::string > requested, object;
    Mode mode = Mode::INVALID;

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
        if( found == ( exit == CompareFuncExit::FIRST_FOUND ) )
        {
          return found;
        }
      }
      return foundAny;
    }
  };
  struct ComparerRoles
  {
    using Roles = BitSets< 4, Role >;
    Roles requested, object;
    Mode mode = Mode::INVALID;

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
        case Mode::INVALID:
        {
          return true;
        }
        case Mode::EMPTY:
        case Mode::ALL:
        {
          return requested == ( object & requested );
        }
        case Mode::ANY:
        {
          return bool( object & requested );
        }
        case Mode::NONE:
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
    Mode mode = Mode::INVALID;

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
        case Mode::INVALID:
        {
          return true;
        }
        case Mode::EMPTY:
        case Mode::ALL:
        {
          return requested == ( object & requested );
        }
        case Mode::ANY:
        {
          return bool( object & requested );
        }
        case Mode::NONE:
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
    if( cmp.mode == Mode::INVALID )
      return true;
    cmp.Update( obj );
    switch( cmp.mode )
    {
      case Mode::ANY:
      {
        if( cmp.RequestEmpty() || cmp.ObjectEmpty() )
          return false;
        break;
      }
      case Mode::ALL:
      {
        if( cmp.RequestEmpty() )
          return true;
        if( cmp.ObjectEmpty() )
          return false;
        break;
      }
      case Mode::NONE:
      {
        if( cmp.RequestEmpty() || cmp.ObjectEmpty() )
          return true;
        break;
      }
      case Mode::EMPTY:
      {
        if( cmp.RequestEmpty() && cmp.ObjectEmpty() )
          return true;
        if( cmp.RequestEmpty() || cmp.ObjectEmpty() )
          return false;
        break;
      }
      case Mode::INVALID:
      {
        return true;
      }
    }

    switch( cmp.mode )
    {
      case Mode::EMPTY:
      case Mode::ALL:
      {
        if( !cmp.Compare( CompareFuncExit::FIRST_NOT_FOUND ) )
          return false;
        break;
      }
      case Mode::ANY:
      {
        if( cmp.Compare( CompareFuncExit::FIRST_FOUND ) )
          return true;
        break;
      }
      case Mode::NONE:
      {
        if( cmp.Compare( CompareFuncExit::FIRST_FOUND ) )
          return false;
        break;
      }
      case Mode::INVALID:
      {
        return true;
      }
    }
    switch( cmp.mode )
    {
      case Mode::EMPTY:
      case Mode::ALL:
      case Mode::NONE:
      {
        return true;
      }
      case Mode::ANY:
      {
        return false;
      }
      case Mode::INVALID:
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
  if( maxCount > 0 && result.size() >= maxCount )
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
    case SortOrder::CANONICAL:
    {
      break;
    }

    case SortOrder::REVERSE_CANONICAL:
    {
      std::reverse( res.begin(), res.end() );
      break;
    }

    default:
    {
      throw std::domain_error{"unsupported sorting order"};
    }
      //TODO: other cases
  }

  return res;
}
