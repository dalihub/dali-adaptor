#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACCESSIBLE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACCESSIBLE_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <map>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/internal/accessibility/bridge/bridge-base.h>

/**
 * @brief The BridgeAccessible class is to correspond with Dali::Accessibility::Accessible.
 */
class BridgeAccessible : public virtual BridgeBase
{
protected:
  /**
   * @brief Constructor.
   */
  BridgeAccessible();

  /**
   * @brief Registers Accessible functions to dbus interfaces.
   */
  void RegisterInterfaces();

  /**
   * @brief Returns the Accessible object of the currently executed DBus method call.
   *
   * @return The Accessible object
   */
  Dali::Accessibility::Accessible* FindSelf() const;

public:
  /**
   * @brief Enumeration for NeighborSearchMode.
   */
  enum class NeighborSearchMode
  {
    NORMAL                          = 0, ///< Normal
    RECURSE_FROM_ROOT               = 1, ///< Recurse from root
    CONTINUE_AFTER_FAILED_RECURSION = 2, ///< Continue after failed recursion
    RECURSE_TO_OUTSIDE              = 3, ///< Recurse to outside
  };

  using ReadingMaterialType = DBus::ValueOrError<
    std::map<std::string, std::string>, // attributes
    std::string,                        // name
    std::string,                        // labeledByName
    std::string,                        // textIfceName
    uint32_t,
    Dali::Accessibility::States,
    std::string,                      // localized name
    int32_t,                          // child count
    double,                           // current value
    std::string,                      // formatted current value
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

  using NodeInfoType = DBus::ValueOrError<
    std::string,                                    // role name
    std::string,                                    // name
    std::string,                                    // toolkit name
    std::map<std::string, std::string>,             // attributes
    Dali::Accessibility::States,                    // states
    std::tuple<int32_t, int32_t, int32_t, int32_t>, // screen extents
    std::tuple<int32_t, int32_t, int32_t, int32_t>, // window extents
    double,                                         // current value
    double,                                         // minimum increment
    double,                                         // maximum value
    double                                          // minimum value
    >;

  using Relation = std::tuple<uint32_t, std::vector<Dali::Accessibility::Accessible*>>;

  /**
   * @copydoc Dali::Accessibility::Accessible::GetChildCount()
   */
  int GetChildCount();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetChildAtIndex()
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*> GetChildAtIndex(int index);

  /**
   * @copydoc Dali::Accessibility::Accessible::GetParent()
   */
  Dali::Accessibility::Accessible* GetParent();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetChildren()
   */
  DBus::ValueOrError<std::vector<Dali::Accessibility::Accessible*>> GetChildren();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetName()
   */
  std::string GetName();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetDescription()
   */
  std::string GetDescription();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetRole()
   */
  DBus::ValueOrError<uint32_t> GetRole();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetRoleName()
   */
  DBus::ValueOrError<std::string> GetRoleName();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetLocalizedRoleName()
   */
  DBus::ValueOrError<std::string> GetLocalizedRoleName();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetIndexInParent()
   */
  DBus::ValueOrError<int32_t> GetIndexInParent();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetStates()
   */
  DBus::ValueOrError<std::array<uint32_t, 2>> GetStates();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetAttributes()
   */
  DBus::ValueOrError<std::map<std::string, std::string>> GetAttributes();

  /**
   * @copydoc Dali::Accessibility::Accessible::GetInterfacesAsStrings()
   */
  DBus::ValueOrError<std::vector<std::string>> GetInterfacesAsStrings();

  /**
   * @brief Gets Accessible object on which surface lies the point with given coordinates.
   *
   * @param[in] x X coordinate of a point
   * @param[in] y Y coordinate of a point
   * @param[in] coordinateType The coordinate type
   * @return The array containing the Accessible object, recursive status, and deputy Accessible
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*, uint8_t, Dali::Accessibility::Accessible*> GetNavigableAtPoint(int32_t x, int32_t y, uint32_t coordinateType);

  /**
   * @brief Gets Accessible object that, dependently to the given direction parameter,
   * stands in navigation order immediately before/after the Accessible object being a target of this dbus call.
   *
   * The 'direction' parameter denotes if the neighbor object search is done forward or backward in UI elements navigation order.
   * @param[in] rootPath The path of root Accessible object
   * @param[in] direction 1 is forward, 0 is backward
   * @param[in] searchMode BridgeAccessible::NeighborSearchMode enum
   * @return The array containing the neighbor Accessible object and recursive status
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*, uint8_t> GetNeighbor(std::string rootPath, int32_t direction, int32_t searchMode);

  /**
   * @brief Gets the default label information.
   *
   * The "Default label" is a text that could be read by screen-reader immediately
   * after the navigation context has changed (window activates, popup shows up, tab changes) and before first UI element is highlighted.
   * @return The array containing the Accessible object being a source of default label text, its role, and its attributes
   * @note This is a Tizen only feature not present in upstream ATSPI.
   * Feature can be enabled/disabled for particular context root object by setting value of its accessibility attribute "default_label".
   */
  DBus::ValueOrError<Dali::Accessibility::Accessible*, uint32_t, std::map<std::string, std::string>> GetDefaultLabelInfo();

  /**
   * @brief Gets Reading material information of the self object.
   * @return Reading material information
   */
  ReadingMaterialType GetReadingMaterial();

  /**
   * @copydoc Dali::Accessibility::Accessible::DoGesture()
   */
  DBus::ValueOrError<bool> DoGesture(Dali::Accessibility::Gesture type, int32_t startPositionX, int32_t startPositionY, int32_t endPositionX, int32_t endPositionY, Dali::Accessibility::GestureState state, uint32_t eventTime);

  /**
   * @copydoc Dali::Accessibility::Accessible::GetRelationSet()
   */
  DBus::ValueOrError<std::vector<Relation>> GetRelationSet();

  /**
   * @copydoc Dali::Accessibility::Accessible::SetListenPostRender()
   */
  DBus::ValueOrError<void> SetListenPostRender(bool enabled);

  /**
   * @brief Gets Node information of the self object.
   * @return Node information
   */
  NodeInfoType GetNodeInfo();

private:
  /**
   * @brief Calculates Neighbor candidate object in root node.
   *
   * The DFS algorithm in the method is implemented in iterative way.
   * @param root The accessible root object
   * @param start The start node
   * @param forward If forward is 1, then it navigates forward, otherwise backward.
   * @param searchMode BridgeAccessible::NeighborSearchMode  enum
   * @return The neighbor Accessible object
   */
  Dali::Accessibility::Accessible* CalculateNeighbor(Dali::Accessibility::Accessible* root, Dali::Accessibility::Accessible* start, unsigned char forward, NeighborSearchMode searchMode);

  /**
   * @brief Gets valid children accessible.
   *
   * @param[in] children Children accessible objects
   * @param start The start node
   * @return The valid children
   */
  std::vector<Dali::Accessibility::Component*> GetValidChildren(const std::vector<Dali::Accessibility::Accessible*>& children, Dali::Accessibility::Accessible* start);

  /**
   * @brief Sorts children accessible based on the coordinates from top-left to bottom-right.
   *
   * @param[in,out] children Children accessible objects
   * @return The sorted children
   */
  void SortChildrenFromTopLeft(std::vector<Dali::Accessibility::Component*>& children);

  /**
   * @brief Gets the currently highlighted accessible.
   *
   * @return The highlighted accessible
   * @remarks This is an experimental feature and might not be supported now.
   */
  Dali::Accessibility::Accessible* GetCurrentlyHighlighted();

  /**
   * @brief Finds the non defunct sibling of the node.
   *
   * @param[out] areAllChildrenVisited True if all children are visited
   * @param[in] node The accessible object to find its non defunct sibling
   * @param[in] start The start node
   * @param[in] root The root node
   * @param[in] forward If forward is 1, then it navigates forward, otherwise backward.
   * @return The non defunct sibling accessible
   *
   * @note This function performs a Depth-First Search (DFS) on all children within the node.
   */
  Dali::Accessibility::Accessible* FindNonDefunctSibling(bool& areAllChildrenVisited, Dali::Accessibility::Accessible* node, Dali::Accessibility::Accessible* start, Dali::Accessibility::Accessible* root, unsigned char forward);

  /**
   * @brief Gets the next non defunct sibling.
   *
   * @param obj The accessible object to find its non defunct sibling
   * @param start The start node
   * @param forward If forward is 1, then it navigates forward, otherwise backward.
   * @return The non defunct sibling accessible
   */
  Dali::Accessibility::Accessible* GetNextNonDefunctSibling(Dali::Accessibility::Accessible* obj, Dali::Accessibility::Accessible* start, unsigned char forward);

  /**
   * @brief Gets Component object in relation.
   * @param obj The accessible object to find component in its address
   * @param relationType Accessibility::RelationType enum
   * @return The Component object
   */
  Dali::Accessibility::Component* GetObjectInRelation(Dali::Accessibility::Accessible* obj, Dali::Accessibility::RelationType relationType);

  /**
   * @brief Calculates and gets Component object that can be navigable at given coordinates.
   *
   * This method calculates which Component should be addressed if user hits certain point in a screen.
   * @param root The root node
   * @param point The coordinate of a point
   * @param type This type says if the coordinates of a point refer to the device screen or current window.
   * @param maxRecursionDepth The maximum recursion depth count
   * @return The Component object
   */
  Dali::Accessibility::Component* CalculateNavigableAccessibleAtPoint(Dali::Accessibility::Accessible* root, Dali::Accessibility::Point point, Dali::Accessibility::CoordinateType type, unsigned int maxRecursionDepth);
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_ACCESSIBLE_H
