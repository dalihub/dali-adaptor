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
#include <dali/internal/accessibility/bridge/bridge-accessible.h>

// EXTERNAL INCLUDES
#include <iostream>

//comment out 2 lines below to get more logs
#undef LOG
#define LOG() _LoggerEmpty()

using namespace Dali::Accessibility;

#define GET_NAVIGABLE_AT_POINT_MAX_RECURSION_DEPTH 10000

BridgeAccessible::BridgeAccessible()
{
}

void BridgeAccessible::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{AtspiDbusInterfaceAccessible};
  AddGetPropertyToInterface( desc, "ChildCount", &BridgeAccessible::GetChildCount );
  AddGetPropertyToInterface( desc, "Name", &BridgeAccessible::GetName );
  AddGetPropertyToInterface( desc, "Description", &BridgeAccessible::GetDescription );
  AddGetPropertyToInterface( desc, "Parent", &BridgeAccessible::GetParent );
  AddFunctionToInterface( desc, "GetRole", &BridgeAccessible::GetRole );
  AddFunctionToInterface( desc, "GetRoleName", &BridgeAccessible::GetRoleName );
  AddFunctionToInterface( desc, "GetLocalizedRoleName", &BridgeAccessible::GetLocalizedRoleName );
  AddFunctionToInterface( desc, "GetState", &BridgeAccessible::GetStates );
  AddFunctionToInterface( desc, "GetAttributes", &BridgeAccessible::GetAttributes );
  AddFunctionToInterface( desc, "GetInterfaces", &BridgeAccessible::GetInterfaces );
  AddFunctionToInterface( desc, "GetChildAtIndex", &BridgeAccessible::GetChildAtIndex );
  AddFunctionToInterface( desc, "GetChildren", &BridgeAccessible::GetChildren );
  AddFunctionToInterface( desc, "GetIndexInParent", &BridgeAccessible::GetIndexInParent );
  AddFunctionToInterface( desc, "GetNavigableAtPoint", &BridgeAccessible::GetNavigableAtPoint );
  AddFunctionToInterface( desc, "GetNeighbor", &BridgeAccessible::GetNeighbor );
  AddFunctionToInterface( desc, "GetDefaultLabelInfo", &BridgeAccessible::GetDefaultLabelInfo );
  AddFunctionToInterface( desc, "DoGesture", &BridgeAccessible::DoGesture );
  AddFunctionToInterface( desc, "GetReadingMaterial", &BridgeAccessible::GetReadingMaterial );
  AddFunctionToInterface( desc, "GetRelationSet", &BridgeAccessible::GetRelationSet );
  dbusServer.addInterface( "/", desc, true );
}

static bool AcceptObjectCheckRole( Component* obj )
{
  if( !obj )
    return false;
  switch( obj->GetRole() )
  {
    case Role::APPLICATION:
    case Role::FILLER:
    case Role::SCROLL_PANE:
    case Role::SPLIT_PANE:
    case Role::WINDOW:
    case Role::IMAGE:
    case Role::IMAGE_MAP:
    case Role::LIST:
    case Role::ICON:
    case Role::TOOL_BAR:
    case Role::REDUNDANT_OBJECT:
    case Role::COLOR_CHOOSER:
    case Role::TREE_TABLE:
    case Role::PAGE_TAB_LIST:
    case Role::PAGE_TAB:
    case Role::SPIN_BUTTON:
    case Role::INPUT_METHOD_WINDOW:
    case Role::EMBEDDED:
    case Role::INVALID:
    case Role::NOTIFICATION:
    case Role::DATE_EDITOR:
    case Role::TABLE:
    {
      return false;
    }
    default:
    {
      break;
    }
  }

  return true;
}

static bool AcceptObjectCheckRelations( Component* obj)
{
  auto r = obj->GetRelationSet();

  for (const auto& it : r)
    if (it.relationType == RelationType::CONTROLLED_BY)
      return false;

  return true;
}

static Component* GetScrollableParent( Accessible* obj )
{
  while( obj )
  {
    obj = obj->GetParent();
    auto comp = dynamic_cast< Component* >( obj );
    if( comp && comp->IsScrollable() )
      return comp;
  }
  return nullptr;
}

static bool ObjectIsItem( Component* obj )
{
  if( !obj )
    return false;
  auto role = obj->GetRole();
  return role == Role::LIST_ITEM || role == Role::MENU_ITEM;
}

static bool ObjectIsCollapsed( Component* obj )
{
  if( !obj )
    return false;
  const auto states = obj->GetStates();
  return states[State::EXPANDABLE] && !states[State::EXPANDED];
}

static bool OobjectIsZeroSize( Component* obj )
{
  if( !obj )
    return false;
  auto extents = obj->GetExtents( CoordType::WINDOW );
  return extents.height == 0 || extents.width == 0;
}

static bool AcceptObject( Component* obj )
{
  if( !obj )
    return false;
  const auto states = obj->GetStates();
  if( !states[State::VISIBLE] )
    return false;
  if( !AcceptObjectCheckRole( obj ) )
    return false;
  if ( !AcceptObjectCheckRelations( obj ) )
    return false;
  if( !states[State::HIGHLIGHTABLE] )
    return false;

  if( GetScrollableParent( obj ) != nullptr )
  {
    auto parent = dynamic_cast< Component* >( obj->GetParent() );

    if( parent )
    {
      return !ObjectIsItem( obj ) || !ObjectIsCollapsed( parent );
    }
  }
  else
  {
    if( OobjectIsZeroSize( obj ) )
    {
      return false;
    }
    if( !states[State::SHOWING] )
    {
      return false;
    }
  }
  return true;
}

static bool AcceptObject( Accessible* obj )
{
  auto c = dynamic_cast< Component* >( obj );
  return AcceptObject( c );
}

static std::string objDump( Component* obj )
{
  if ( !obj )
    return "nullptr";
  std::ostringstream o;
  auto e = obj->GetExtents( CoordType::SCREEN );
  o << "name: " << obj->GetName() << " extent: (" << e.x << ", "
      << e.y << "), [" << e.width << ", " << e.height << "]";
  return o.str();
}

Component * BridgeAccessible::GetObjectInRelation( Accessible * obj, RelationType ralationType )
{
  if ( !obj )
    return nullptr;
  for ( auto &relation : obj->GetRelationSet() )
  {
    if ( relation.relationType == ralationType )
    {
      for ( auto &address : relation.targets )
      {
        auto component = dynamic_cast<Component*>( Find( address ) );
        if ( component )
          return component;
      }
    }
  }
  return nullptr;
}

static std::string makeIndent( unsigned int maxRecursionDepth )
{
  return std::string( GET_NAVIGABLE_AT_POINT_MAX_RECURSION_DEPTH - maxRecursionDepth, ' ' );
}

Component* BridgeAccessible::CalculateNavigableAccessibleAtPoint( Accessible* root, Point p, CoordType cType, unsigned int maxRecursionDepth )
{
  if( !root || maxRecursionDepth == 0 )
    return nullptr;
  auto root_component = dynamic_cast< Component* >( root );
  LOG() << "CalculateNavigableAccessibleAtPoint: checking: " << makeIndent(maxRecursionDepth) << objDump(root_component);

  if( root_component && !root_component->Contains( p, cType ) )
    return nullptr;

  auto children = root->GetChildren();
  for( auto childIt = children.rbegin(); childIt != children.rend(); childIt++ )
  {
    //check recursively all children first
    auto result = CalculateNavigableAccessibleAtPoint( *childIt, p, cType, maxRecursionDepth - 1 );
    if( result )
      return result;
  }
  if( root_component )
  {
    //Found a candidate, all its children are already checked
    auto controledBy = GetObjectInRelation( root_component, RelationType::CONTROLLED_BY );
    if ( !controledBy )
      controledBy = root_component;

    if ( controledBy->IsProxy() || AcceptObject( controledBy ) )
    {
      LOG() << "CalculateNavigableAccessibleAtPoint: found:    " << makeIndent(maxRecursionDepth) << objDump( root_component );
      return controledBy;
    }
  }
  return nullptr;
}

BridgeAccessible::ReadingMaterialType BridgeAccessible::GetReadingMaterial()
{
  auto self = FindSelf();
  auto attributes = self->GetAttributes();
  auto name = self->GetName();
  std::string labeledByName = "";
  std::string textIfceName = "";
  auto role = static_cast< uint32_t >( self->GetRole() );
  auto states = self->GetStates();
  auto localizedName = self->GetLocalizedRoleName();
  auto childCount = static_cast< int32_t >( self->GetChildCount() );

  double currentValue = 0.0;
  double minimumIncrement = 0.0;
  double maximumValue = 0.0;
  double minimumValue = 0.0;

  auto *value = dynamic_cast<Dali::Accessibility::Value *>(self);
  if (value)
  {
    currentValue = value->GetCurrent();
    minimumIncrement = value->GetMinimumIncrement();
    maximumValue = value->GetMaximum();
    minimumValue = value->GetMinimum();
  }

  auto description = self->GetDescription();
  auto indexInParent = static_cast< int32_t >( self->GetIndexInParent() );
  bool isSelectedInParent = false;
  bool hasCheckBoxChild = false;
  int32_t firstSelectedChildIndex = -1;
  int32_t selectedChildCount = 0;

  for( auto i = 0u; i < static_cast< size_t >( childCount ); ++i )
  {
    auto q = self->GetChildAtIndex( i );
    auto s = q->GetStates();
    if( s[State::SELECTABLE] )
    {
      if( s[State::SELECTED] )
      {
        ++selectedChildCount;
        if( firstSelectedChildIndex < 0 )
          firstSelectedChildIndex = static_cast< int32_t >( i );
      }
    }
    if( q->GetRole() == Role::CHECK_BOX )
      hasCheckBoxChild = true;
  }

  int32_t listChildrenCount = 0;
  Accessible* parent = self->GetParent();
  auto parentStateSet = parent ? parent->GetStates() : States{};
  auto parentChildCount = parent ? static_cast< int32_t >( parent->GetChildCount() ) : 0;
  auto parentRole = static_cast< uint32_t >( parent ? parent->GetRole() : Role{} );
  Accessible* describedByObject = nullptr;

  return {
      attributes,
      name,
      labeledByName,
      textIfceName,
      role,
      states,
      localizedName,
      childCount,
      currentValue,
      minimumIncrement,
      maximumValue,
      minimumValue,
      description,
      indexInParent,
      isSelectedInParent,
      hasCheckBoxChild,
      listChildrenCount,
      firstSelectedChildIndex,
      parent,
      parentStateSet,
      parentChildCount,
      parentRole,
      selectedChildCount,
      describedByObject};
}

DBus::ValueOrError< bool > BridgeAccessible::DoGesture( Dali::Accessibility::Gesture type, int32_t xBeg, int32_t xEnd, int32_t yBeg, int32_t yEnd, Dali::Accessibility::GestureState state, uint32_t eventTime )
{
 return FindSelf()->DoGesture( Dali::Accessibility::GestureInfo {type, xBeg, xEnd, yBeg, yEnd, state, eventTime});
}

DBus::ValueOrError< Accessible*, uint8_t, Accessible* > BridgeAccessible::GetNavigableAtPoint( int32_t x, int32_t y, uint32_t coordType )
{
  Accessible* deputy = nullptr;
  auto accessible = FindSelf();
  auto cType = static_cast< CoordType >( coordType );
  LOG() << "GetNavigableAtPoint: " << x << ", " << y << " type: " << coordType;
  auto component = CalculateNavigableAccessibleAtPoint( accessible, {x, y}, cType, GET_NAVIGABLE_AT_POINT_MAX_RECURSION_DEPTH );
  bool recurse = false;
  if( component )
  {
    recurse = component->IsProxy();
  }
  //TODO: add deputy
  return {component, recurse, deputy};
}

static bool CheckChainEndWithAttribute( Accessible* obj, unsigned char forward )
{
  if( !obj )
    return false;
  auto attrs = obj->GetAttributes();
  for( auto& attr : attrs )
  {
    if( attr.first == "relation_chain_end" )
    {
      if( ( attr.second == "prev,end" && forward == 0 ) || ( attr.second == "next,end" && forward == 1 ) || attr.second == "prev,next,end" )
      {
        return true;
      }
    }
  }
  return false;
}

static Accessible* DeputyOfProxyInParentGet( Accessible* obj )
{
  return nullptr;
}

Accessible* BridgeAccessible::GetCurrentlyHighlighted()
{
  //TODO: add currently highlighted object
  return nullptr;
}

std::vector< Accessible* > BridgeAccessible::ValidChildrenGet( const std::vector< Accessible* >& children, Accessible* start, Accessible* root )
{
  return children;
}

static bool DeputyIs( Accessible* obj )
{
  //TODO: add deputy
  return false;
}

static Accessible* ProxyInParentGet( Accessible* obj )
{
  if( !obj )
    return nullptr;
  auto children = obj->GetChildren();
  for( auto& child : children )
  {
    if( child->IsProxy() )
      return child;
  }
  return nullptr;
}

static bool ObjectRoleIsAcceptableWhenNavigatingNextPrev( Accessible* obj )
{
  if( !obj )
    return false;
  auto role = obj->GetRole();
  return role != Role::POPUP_MENU && role != Role::DIALOG;
}

template < class T >
struct CycleDetection
{
  CycleDetection( const T value ) : key( value ), currentSearchSize( 1 ), counter( 1 ) {}
  bool check( const T value )
  {
    if( key == value )
      return true;
    if( --counter == 0 )
    {
      currentSearchSize <<= 1;
      if( currentSearchSize == 0 )
        return true; // UNDEFINED BEHAVIOR
      counter = currentSearchSize;
      key = value;
    }
    return false;
  }
  T key;
  unsigned int currentSearchSize;
  unsigned int counter;
};

static Accessible* FindNonDefunctChild( const std::vector< Accessible* >& children, unsigned int currentIndex, unsigned char forward )
{
  unsigned int childrenCount = children.size();
  for( ; currentIndex < childrenCount; forward ? ++currentIndex : --currentIndex )
  {
    Accessible* n = children[currentIndex];
    if( n && !n->GetStates()[State::DEFUNCT] )
      return n;
  }
  return nullptr;
}

static Accessible* DirectionalDepthFirstSearchTryNonDefunctChild( Accessible* node, const std::vector< Accessible* >& children, unsigned char forward )
{
  if( !node )
    return nullptr;
  auto childrenCount = children.size();
  if( childrenCount > 0 )
  {
    const bool isShowing = GetScrollableParent( node ) == nullptr ? node->GetStates()[State::SHOWING] : true;
    if( isShowing )
    {
      return FindNonDefunctChild( children, forward ? 0 : childrenCount - 1, forward );
    }
  }
  return nullptr;
}

Accessible* BridgeAccessible::GetNextNonDefunctSibling( Accessible* obj, Accessible* start, Accessible* root, unsigned char forward )
{
  if( !obj )
    return nullptr;
  auto parent = obj->GetParent();
  if( !parent )
    return nullptr;

  auto children = ValidChildrenGet( parent->GetChildren(), start, root );

  unsigned int children_count = children.size();
  if( children_count == 0 )
  {
    return nullptr;
  }
  unsigned int current = 0;
  for( ; current < children_count && children[current] != obj; ++current )
    ;
  if( current >= children_count )
  {
    return nullptr;
  }
  forward ? ++current : --current;
  auto ret = FindNonDefunctChild( children, current, forward );
  return ret;
}

Accessible* BridgeAccessible::DirectionalDepthFirstSearchTryNonDefunctSibling( bool& all_children_visited, Accessible* node, Accessible* start, Accessible* root, unsigned char forward )
{
  while( true )
  {
    Accessible* sibling = GetNextNonDefunctSibling( node, start, root, forward );
    if( sibling )
    {
      node = sibling;
      all_children_visited = false;
      break;
    }
    // walk up...
    node = node->GetParent();
    if( node == nullptr || node == root )
      return nullptr;

    // in backward traversing stop the walk up on parent
    if( !forward )
      break;
  }
  return node;
}

Accessible* BridgeAccessible::CalculateNeighbor( Accessible* root, Accessible* start, unsigned char forward, BridgeAccessible::GetNeighborSearchMode search_mode )
{
  if( start && CheckChainEndWithAttribute( start, forward ) )
    return start;
  if( root && root->GetStates()[State::DEFUNCT] )
    return NULL;
  if( start && start->GetStates()[State::DEFUNCT] )
  {
    start = NULL;
    forward = 1;
  }

  if( search_mode == BridgeAccessible::GetNeighborSearchMode::recurseToOutside )
  {
    // This only works if we navigate backward, and it is not possible to
    // find in embedded process. In this case the deputy should be used */
    return DeputyOfProxyInParentGet( start );
  }

  Accessible* node = start ? start : root;
  if( !node )
    return nullptr;

  // initialization of all-children-visited flag for start node - we assume
  // that when we begin at start node and we navigate backward, then all children
  // are visited, so navigation will ignore start's children and go to
  // previous sibling available.
  // Regarding condtion (start != root):
  // The last object can be found only if all_children_visited is false.
  // The start is same with root, when looking for the last object.
  bool all_children_visited = ( start != root ) && ( search_mode != BridgeAccessible::GetNeighborSearchMode::recurseFromRoot && !forward );
  // true, if starting element should be ignored. this is only used in rare case of
  // recursive search failing to find an object.
  // consider tree, where element A on bus BUS_A has child B on bus BUS_B. when going "next" from
  // element A algorithm has to descend into BUS_B and search element B and its children. this is done
  // by returning to our caller object B with special flag set (meaning - continue the search from B on bus BUS_B).
  // if next object will be found there (on BUS_B), then search ends. but if not, then our caller will find it out
  // and will call us again with object A and flag search_mode set to NEIGHBOR_SEARCH_MODE_CONTINUE_AFTER_FAILED_RECURSING.
  // this flag means, that object A was already checked previously and we should skip it and its children.
  bool force_next = ( search_mode == BridgeAccessible::GetNeighborSearchMode::continueAfterFailedRecursion );

  CycleDetection< Accessible* > cycleDetection( node );
  while( node )
  {
    if( node->GetStates()[State::DEFUNCT] )
      return nullptr;

    // always accept proxy object from different world
    if( !force_next && node->IsProxy() )
      return node;

    auto children = node->GetChildren();
    children = ValidChildrenGet( children, start, root );

    // do accept:
    // 1. not start node
    // 2. parent after all children in backward traversing
    // 3. Nodes with roles: ATSPI_ROLE_PAGE_TAB, ATSPI_ROLE_POPUP_MENU and ATSPI_ROLE_DIALOG, only when looking for first or last element.
    //    Objects with those roles shouldnt be reachable, when navigating next / prev.
    bool all_children_visited_or_moving_forward = ( children.size() == 0 || forward || all_children_visited );
    if( !force_next && node != start && all_children_visited_or_moving_forward && AcceptObject( node ) )
    {
      if( start == NULL || ObjectRoleIsAcceptableWhenNavigatingNextPrev( node ) )
        return node;
    }

    Accessible* next_related_in_direction = !force_next ? GetObjectInRelation( node, forward ? RelationType::FLOWS_TO : RelationType::FLOWS_FROM ) : nullptr;
    // force_next means that the search_mode is NEIGHBOR_SEARCH_MODE_CONTINUE_AFTER_FAILED_RECURSING
    // in this case the node is elm_layout which is parent of proxy object.
    // There is an access object working for the proxy object, and the access
    // object could have relation information. This relation information should
    // be checked first before using the elm_layout as a node.
    if( force_next && forward )
    {
      auto deputy = DeputyOfProxyInParentGet( node );
      next_related_in_direction =
          GetObjectInRelation( deputy, RelationType::FLOWS_TO );
    }

    if( next_related_in_direction && start && start->GetStates()[State::DEFUNCT] )
    {
      next_related_in_direction = NULL;
    }

    unsigned char want_cycle_detection = 0;
    if( next_related_in_direction )
    {
      // Check next_related_in_direction is deputy object
      Accessible* parent;
      if( !forward )
      {
        // If the prev object is deputy, then go to inside of its proxy first
        if( DeputyIs( next_related_in_direction ) )
        {
          parent = next_related_in_direction->GetParent();
          next_related_in_direction = ProxyInParentGet( parent );
        }
      }
      else
      {
        // If current object is deputy, and it has relation next object,
        // then do not use the relation next object, and use proxy first
        if( DeputyIs( node ) )
        {
          parent = node->GetParent();
          next_related_in_direction = ProxyInParentGet( parent );
        }
      }
      node = next_related_in_direction;
      want_cycle_detection = 1;
    }
    else
    {
      auto child = !force_next && !all_children_visited ? DirectionalDepthFirstSearchTryNonDefunctChild( node, children, forward ) : nullptr;
      if( child )
      {
        want_cycle_detection = 1;
      }
      else
      {
        if( !force_next && node == root )
          return NULL;
        all_children_visited = true;
        child = DirectionalDepthFirstSearchTryNonDefunctSibling( all_children_visited, node, start, root, forward );
      }
      node = child;
    }
    force_next = 0;
    if( want_cycle_detection && cycleDetection.check( node ) )
    {
      return NULL;
    }
  }
  return NULL;
}

DBus::ValueOrError< Accessible*, uint8_t > BridgeAccessible::GetNeighbor( std::string rootPath, int32_t direction, int32_t search_mode )
{
  auto start = FindSelf();
  rootPath = StripPrefix( rootPath );
  auto root = !rootPath.empty() ? Find( rootPath ) : nullptr;
  auto accessible = CalculateNeighbor( root, start, direction == 1, static_cast< GetNeighborSearchMode >( search_mode ) );
  unsigned char recurse = 0;
  if( accessible )
  {
    recurse = accessible->IsProxy();
  }
  return {accessible, recurse};
}

Accessible* BridgeAccessible::GetParent()
{
  // NOTE: currently bridge supports single application root element.
  // only element set as application root might return nullptr from GetParent
  // if you want more, then you need to change setApplicationRoot to
  // add/remove ApplicationRoot and make roots a vector.
  auto p = FindSelf()->GetParent();
  assert( p );
  return p;
}
DBus::ValueOrError< std::vector< Accessible* > > BridgeAccessible::GetChildren()
{
  return FindSelf()->GetChildren();
}
std::string BridgeAccessible::GetDescription()
{
  return FindSelf()->GetDescription();
}
DBus::ValueOrError< uint32_t > BridgeAccessible::GetRole()
{
  return static_cast< unsigned int >( FindSelf()->GetRole() );
}
DBus::ValueOrError< std::string > BridgeAccessible::GetRoleName()
{
  return FindSelf()->GetRoleName();
}
DBus::ValueOrError< std::string > BridgeAccessible::GetLocalizedRoleName()
{
  return FindSelf()->GetLocalizedRoleName();
}
DBus::ValueOrError< int32_t > BridgeAccessible::GetIndexInParent()
{
  return FindSelf()->GetIndexInParent();
}
DBus::ValueOrError< std::array< uint32_t, 2 > > BridgeAccessible::GetStates()
{
  return FindSelf()->GetStates().GetRawData();
}
DBus::ValueOrError< std::unordered_map< std::string, std::string > > BridgeAccessible::GetAttributes()
{
  return FindSelf()->GetAttributes();
}
DBus::ValueOrError< std::vector< std::string > > BridgeAccessible::GetInterfaces()
{
  return FindSelf()->GetInterfaces();
}
int BridgeAccessible::GetChildCount()
{
  return FindSelf()->GetChildCount();
}
DBus::ValueOrError< Accessible* > BridgeAccessible::GetChildAtIndex( int index )
{
  if( index < 0 )
    throw std::domain_error{"negative index (" + std::to_string( index ) + ")"};
  return FindSelf()->GetChildAtIndex( static_cast< size_t >( index ) );
}

std::string BridgeAccessible::GetName()
{
  return FindSelf()->GetName();
}

DBus::ValueOrError< Accessible*, uint32_t , std::unordered_map< std::string, std::string > > BridgeAccessible::GetDefaultLabelInfo()
{
  auto defaultLabel = FindSelf()->GetDefaultLabel();
  return {defaultLabel, static_cast< uint32_t >( defaultLabel->GetRole() ) , defaultLabel->GetAttributes()};
}

DBus::ValueOrError<std::vector< BridgeAccessible::Relation >> BridgeAccessible::GetRelationSet()
{
  auto relations = FindSelf()->GetRelationSet();
  std::vector< BridgeAccessible::Relation > ret;

  for (auto &it : relations)
    ret.emplace_back(Relation{static_cast<uint32_t>(it.relationType), it.targets});

  return ret;
}
