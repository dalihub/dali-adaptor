#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACCESSIBLE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACCESSIBLE_H

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

// EXTERNAL INCLUDES
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// INTERNAL INCLUDES
#include "bridge-base.h"

class BridgeAccessible : public virtual BridgeBase
{
protected:
  BridgeAccessible();

  void RegisterInterfaces();

public:
  enum class GetNeighborSearchMode
  {
    normal = 0,
    recurseFromRoot = 1,
    continueAfterFailedRecursion = 2,
    recurseToOutside = 3,
  };
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
  DBus::ValueOrError< Dali::Accessibility::Accessible*, uint8_t, Dali::Accessibility::Accessible* > GetNavigableAtPoint( int32_t x, int32_t y, uint32_t coordType );
  DBus::ValueOrError< Dali::Accessibility::Accessible*, uint8_t > GetNeighbor( std::string root_path, int32_t direction, int32_t search_mode );
  DBus::ValueOrError< Dali::Accessibility::Accessible*, uint32_t , std::unordered_map< std::string, std::string > > GetDefaultLabelInfo();
  using ReadingMaterialType = DBus::ValueOrError<
      std::unordered_map< std::string, std::string >, // attributes
      std::string,                                    // name
      std::string,                                    // labeledByName
      std::string,                                    // textIfceName
      uint32_t,
      Dali::Accessibility::States,
      std::string,                      // localized name
      int32_t,                          // child count
      double,                           // current value
      double,                           // minimum increment
      double,                           // maximum value
      double,                           // minimum value
      std::string,                      // description
      int32_t,                          // index in parent
      bool,                             // isSelectedInParent
      bool,                             // hasCheckBoxChild
      int32_t,                          // listChildrenCount
      int32_t,                          // firstSelectedChildIndex
      Dali::Accessibility::Accessible*, // parent
      Dali::Accessibility::States,      // parentStateSet
      int32_t,                          // parentChildCount
      uint32_t,                         // parentRole
      int32_t,                          // selectedChildCount,
      Dali::Accessibility::Accessible*  // describedByObject
      >;

  ReadingMaterialType GetReadingMaterial();

  DBus::ValueOrError< bool > DoGesture( Dali::Accessibility::Gesture type, int32_t xBeg, int32_t xEnd, int32_t yBeg, int32_t yEnd, Dali::Accessibility::GestureState state, uint32_t eventTime );

  using Relation = std::tuple< uint32_t, std::vector< Dali::Accessibility::Address > >;
  DBus::ValueOrError<std::vector< Relation >> GetRelationSet();

private:
  Dali::Accessibility::Accessible* CalculateNeighbor( Dali::Accessibility::Accessible* root, Dali::Accessibility::Accessible* start, unsigned char forward, GetNeighborSearchMode search_mode );
  std::vector< Dali::Accessibility::Accessible* > ValidChildrenGet( const std::vector< Dali::Accessibility::Accessible* >& children, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root );
  Dali::Accessibility::Accessible* GetCurrentlyHighlighted();
  Dali::Accessibility::Accessible* DirectionalDepthFirstSearchTryNonDefunctSibling( bool& all_children_visited, Dali::Accessibility::Accessible* node, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root, unsigned char forward );
  Dali::Accessibility::Accessible* GetNextNonDefunctSibling( Dali::Accessibility::Accessible* obj, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root, unsigned char forward );
  Dali::Accessibility::Component* CalculateNavigableAccessibleAtPoint( Dali::Accessibility::Accessible* root, Dali::Accessibility::Point p, Dali::Accessibility::CoordType cType, unsigned int maxRecursionDepth );
  Dali::Accessibility::Component * GetObjectInRelation(Dali::Accessibility::Accessible * obj, Dali::Accessibility::RelationType ralationType);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACCESSIBLE_H
