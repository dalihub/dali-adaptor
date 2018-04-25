#ifndef BRIDGE_ACCESSIBLE_HPP
#define BRIDGE_ACCESSIBLE_HPP

#include "BridgeBase.hpp"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

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
  DBus::ValueOrError< Dali::Accessibility::Accessible*, uint32_t > GetDefaultLabelInfo();
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

  DBus::ValueOrError< bool > DoGesture( int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t );

private:
  Dali::Accessibility::Accessible* CalculateNeighbor( Dali::Accessibility::Accessible* root, Dali::Accessibility::Accessible* start, unsigned char forward, GetNeighborSearchMode search_mode );
  std::vector< Dali::Accessibility::Accessible* > ValidChildrenGet( const std::vector< Dali::Accessibility::Accessible* >& children, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root );
  Dali::Accessibility::Accessible* GetCurrentlyHighlighted();
  Dali::Accessibility::Accessible* DirectionalDepthFirstSearchTryNonDefunctSibling( bool& all_children_visited, Dali::Accessibility::Accessible* node, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root, unsigned char forward );
  Dali::Accessibility::Accessible* GetNextNonDefunctSibling( Dali::Accessibility::Accessible* obj, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root, unsigned char forward );
};

#endif
