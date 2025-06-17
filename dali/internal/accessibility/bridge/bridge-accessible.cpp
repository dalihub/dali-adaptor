/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/events/hit-test-algorithm.h>
#include <dali/public-api/math/math-utils.h>

#include <algorithm>
#include <iostream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/devel-api/atspi-interfaces/selection.h>
#include <dali/devel-api/atspi-interfaces/text.h>
#include <dali/devel-api/atspi-interfaces/value.h>

//comment out 2 lines below to get more logs
#undef LOG
#define LOG() _LoggerEmpty()

using namespace Dali::Accessibility;

#define GET_NAVIGABLE_AT_POINT_MAX_RECURSION_DEPTH 10000

namespace
{
constexpr const char* FORCE_CHILD_SEARCH_ATTR{"forceChildSearch"};
constexpr const char* VALUE_FORMAT_KEY      = "value_format";
constexpr const char* VALUE_FORMAT_TEXT_VAL = "text";

bool IsSubWin(Accessible* accessible)
{
  auto baseHandle = BridgeBase::GetWindow(accessible).GetBaseHandle();
  if(baseHandle)
  {
    auto window = Dali::Window::DownCast(baseHandle);
    return DALI_LIKELY(Dali::DevelWindow::GetParent(window));
  }
  return false;
}

bool SortVertically(Component* lhs, Component* rhs)
{
  auto leftRect  = lhs->GetExtents(CoordinateType::WINDOW);
  auto rightRect = rhs->GetExtents(CoordinateType::WINDOW);

  return leftRect.y < rightRect.y;
}

bool SortHorizontally(Component* lhs, Component* rhs)
{
  auto leftRect  = lhs->GetExtents(CoordinateType::WINDOW);
  auto rightRect = rhs->GetExtents(CoordinateType::WINDOW);

  return leftRect.x < rightRect.x;
}

std::vector<std::vector<Component*>> SplitLines(const std::vector<Component*>& children)
{
  // Find first with non-zero area
  auto first = std::find_if(children.begin(), children.end(), [](Component* child) -> bool {
    auto extents = child->GetExtents(CoordinateType::WINDOW);
    return !Dali::EqualsZero(extents.height) && !Dali::EqualsZero(extents.width);
  });

  if(first == children.end())
  {
    return {};
  }

  std::vector<std::vector<Component*>> lines(1);
  Dali::Rect<>                         lineRect = (*first)->GetExtents(CoordinateType::WINDOW);
  Dali::Rect<>                         rect;

  // Split into lines
  for(auto it = first; it != children.end(); ++it)
  {
    auto child = *it;

    rect = child->GetExtents(CoordinateType::WINDOW);
    if(Dali::EqualsZero(rect.height) || Dali::EqualsZero(rect.width))
    {
      // Zero area, ignore
      continue;
    }

    if(lineRect.y + (0.5 * lineRect.height) >= rect.y + (0.5 * rect.height))
    {
      // Same line
      lines.back().push_back(child);
    }
    else
    {
      // Start a new line
      lineRect = rect;
      lines.emplace_back();
      lines.back().push_back(child);
    }
  }

  return lines;
}

static bool AcceptObjectCheckRelations(Component* obj)
{
  auto relations = obj->GetRelationSet();

  for(const auto& it : relations)
  {
    if(it.mRelationType == RelationType::CONTROLLED_BY)
    {
      return false;
    }
  }
  return true;
}

static Component* GetScrollableParent(Accessible* obj)
{
  while(obj)
  {
    obj       = obj->GetParent();
    auto comp = dynamic_cast<Component*>(obj);
    if(comp && comp->IsScrollable())
    {
      return comp;
    }
  }
  return nullptr;
}

static bool IsObjectItem(Component* obj)
{
  if(!obj)
  {
    return false;
  }
  auto role = obj->GetRole();
  return role == Role::LIST_ITEM || role == Role::MENU_ITEM;
}

static bool IsObjectCollapsed(Component* obj)
{
  if(!obj)
  {
    return false;
  }
  const auto states = obj->GetStates();
  return states[State::EXPANDABLE] && !states[State::EXPANDED];
}

static bool IsObjectZeroSize(Component* obj)
{
  if(!obj)
  {
    return false;
  }
  auto extents = obj->GetExtents(CoordinateType::WINDOW);
  return Dali::EqualsZero(extents.height) || Dali::EqualsZero(extents.width);
}

static bool IsVisibleInScrollableParent(Accessible* accessible)
{
  auto scrollableParent = GetScrollableParent(accessible);
  if(scrollableParent == nullptr)
  {
    return true;
  }

  auto scrollableParentExtents = scrollableParent->GetExtents(CoordinateType::WINDOW);
  auto component               = dynamic_cast<Component*>(accessible);

  if(component && !scrollableParentExtents.Intersects(component->GetExtents(CoordinateType::WINDOW)))
  {
    return false;
  }

  return true;
}

static bool IsChildVisibleInScrollableParent(Accessible* start, Accessible* accessible)
{
  return IsVisibleInScrollableParent(start) || IsVisibleInScrollableParent(accessible);
}

static bool IsObjectAcceptable(Component* obj)
{
  if(!obj)
  {
    return false;
  }

  const auto states = obj->GetStates();
  if(!states[State::VISIBLE])
  {
    return false;
  }
  if(!AcceptObjectCheckRelations(obj))
  {
    return false;
  }
  if(!states[State::HIGHLIGHTABLE])
  {
    return false;
  }

  if(GetScrollableParent(obj) != nullptr)
  {
    auto parent = dynamic_cast<Component*>(obj->GetParent());

    if(parent)
    {
      return !IsObjectItem(obj) || !IsObjectCollapsed(parent);
    }
  }
  else
  {
    if(IsObjectZeroSize(obj))
    {
      return false;
    }
    if(!states[State::SHOWING])
    {
      return false;
    }
  }
  return true;
}

static bool IsObjectAcceptable(Accessible* obj)
{
  auto component = dynamic_cast<Component*>(obj);
  return IsObjectAcceptable(component);
}

static int32_t GetItemCountOfContainer(Accessible* obj, Dali::Accessibility::Role containerRole, Dali::Accessibility::Role itemRole, bool isDirectChild)
{
  int32_t itemCount = 0;
  if(obj && (!isDirectChild || obj->GetRole() == containerRole))
  {
    for(auto i = 0u; i < static_cast<size_t>(obj->GetChildCount()); ++i)
    {
      auto child = obj->GetChildAtIndex(i);
      if(child && child->GetRole() == itemRole)
      {
        itemCount++;
      }
    }
  }
  return itemCount;
}

static int32_t GetItemCountOfFirstDescendantContainer(Accessible* obj, Dali::Accessibility::Role containerRole, Dali::Accessibility::Role itemRole, bool isDirectChild)
{
  int32_t itemCount = 0;
  itemCount         = GetItemCountOfContainer(obj, containerRole, itemRole, isDirectChild);
  if(itemCount > 0 || !obj)
  {
    return itemCount;
  }

  for(auto i = 0u; i < static_cast<size_t>(obj->GetChildCount()); ++i)
  {
    auto child = obj->GetChildAtIndex(i);
    itemCount  = GetItemCountOfFirstDescendantContainer(child, containerRole, itemRole, isDirectChild);
    if(itemCount > 0)
    {
      return itemCount;
    }
  }
  return itemCount;
}

static std::string GetComponentInfo(Component* obj)
{
  if(!obj)
  {
    return "nullptr";
  }

  std::ostringstream object;
  auto               extent = obj->GetExtents(CoordinateType::SCREEN);
  object << "name: " << obj->GetName() << " extent: (" << extent.x << ", "
         << extent.y << "), [" << extent.width << ", " << extent.height << "]";
  return object.str();
}

static std::string MakeIndent(unsigned int maxRecursionDepth)
{
  return std::string(GET_NAVIGABLE_AT_POINT_MAX_RECURSION_DEPTH - maxRecursionDepth, ' ');
}

static bool IsRoleAcceptableWhenNavigatingNextPrev(Accessible* obj)
{
  if(!obj)
  {
    return false;
  }
  auto role = obj->GetRole();
  return role != Role::POPUP_MENU && role != Role::DIALOG;
}

static Accessible* FindNonDefunctChild(const std::vector<Component*>& children, unsigned int currentIndex, unsigned char forward, Accessible* start)
{
  unsigned int childrenCount = children.size();
  for(; currentIndex < childrenCount; forward ? ++currentIndex : --currentIndex)
  {
    Accessible* object = children[currentIndex];
    if(object && !object->GetStates()[State::DEFUNCT] && IsChildVisibleInScrollableParent(start, object))
    {
      return object;
    }
  }
  return nullptr;
}

// The auxiliary method for Depth-First Search (DFS) algorithm to find non defunct child directionally
static Accessible* FindNonDefunctChildWithDepthFirstSearch(Accessible* node, const std::vector<Component*>& children, unsigned char forward, Accessible* start)
{
  if(!node)
  {
    return nullptr;
  }

  auto childrenCount = children.size();
  if(childrenCount > 0)
  {
    const bool isShowing = GetScrollableParent(node) == nullptr ? node->GetStates()[State::SHOWING] : true;
    if(isShowing)
    {
      return FindNonDefunctChild(children, forward ? 0 : childrenCount - 1, forward, start);
    }
  }
  return nullptr;
}

static bool CheckChainEndWithAttribute(Accessible* obj, unsigned char forward)
{
  if(!obj)
  {
    return false;
  }

  auto attrs = obj->GetAttributes();
  for(auto& attr : attrs)
  {
    if(attr.first == "relation_chain_end")
    {
      if((attr.second == "prev,end" && forward == 0) || (attr.second == "next,end" && forward == 1) || attr.second == "prev,next,end")
      {
        return true;
      }
    }
  }
  return false;
}

static std::vector<Component*> GetScrollableParents(Accessible* accessible)
{
  std::vector<Component*> scrollableParents;

  while(accessible)
  {
    accessible     = accessible->GetParent();
    auto component = dynamic_cast<Component*>(accessible);
    if(component && component->IsScrollable())
    {
      scrollableParents.push_back(component);
    }
  }
  return scrollableParents;
}

static std::vector<Component*> GetNonDuplicatedScrollableParents(Accessible* child, Accessible* start)
{
  auto scrollableParentsOfChild = GetScrollableParents(child);
  auto scrollableParentsOfStart = GetScrollableParents(start);

  // find the first different scrollable parent by comparing from top to bottom.
  // since it can not be the same after that, there is no need to compare.
  while(!scrollableParentsOfChild.empty() && !scrollableParentsOfStart.empty() && scrollableParentsOfChild.back() == scrollableParentsOfStart.back())
  {
    scrollableParentsOfChild.pop_back();
    scrollableParentsOfStart.pop_back();
  }

  return scrollableParentsOfChild;
}

bool IsActorAccessibleFunction(Dali::Actor actor, Dali::HitTestAlgorithm::TraverseType type)
{
  bool hittable = false;
  switch(type)
  {
    case Dali::HitTestAlgorithm::CHECK_ACTOR:
    {
      // Check whether the actor is visible and not fully transparent.
      if(actor.GetCurrentProperty<bool>(Dali::Actor::Property::VISIBLE) && actor.GetCurrentProperty<Dali::Vector4>(Dali::Actor::Property::WORLD_COLOR).a > 0.01f) // not FULLY_TRANSPARENT
      {
        if(actor)
        {
          if(auto accessible = Accessible::Get(actor))
          {
            auto states = accessible->GetStates();
            if(states[State::HIGHLIGHTABLE])
            {
              hittable = true;
            }
          }
        }
      }
      break;
    }
    case Dali::HitTestAlgorithm::DESCEND_ACTOR_TREE:
    {
      if(actor.GetCurrentProperty<bool>(Dali::Actor::Property::VISIBLE)) // Actor is visible, if not visible then none of its children are visible.
      {
        hittable = true;
      }
      break;
    }
    default:
    {
      break;
    }
  }
  return hittable;
};

} // anonymous namespace

BridgeAccessible::BridgeAccessible()
{
}

void BridgeAccessible::RegisterInterfaces()
{
  DBus::DBusInterfaceDescription desc{Accessible::GetInterfaceName(AtspiInterface::ACCESSIBLE)};
  AddGetPropertyToInterface(desc, "ChildCount", &BridgeAccessible::GetChildCount);
  AddGetPropertyToInterface(desc, "Name", &BridgeAccessible::GetName);
  AddGetPropertyToInterface(desc, "Description", &BridgeAccessible::GetDescription);
  AddGetPropertyToInterface(desc, "Parent", &BridgeAccessible::GetParent);
  AddFunctionToInterface(desc, "GetRole", &BridgeAccessible::GetRole);
  AddFunctionToInterface(desc, "GetRoleName", &BridgeAccessible::GetRoleName);
  AddFunctionToInterface(desc, "GetLocalizedRoleName", &BridgeAccessible::GetLocalizedRoleName);
  AddFunctionToInterface(desc, "GetState", &BridgeAccessible::GetStates);
  AddFunctionToInterface(desc, "GetAttributes", &BridgeAccessible::GetAttributes);
  AddFunctionToInterface(desc, "GetInterfaces", &BridgeAccessible::GetInterfacesAsStrings);
  AddFunctionToInterface(desc, "GetChildAtIndex", &BridgeAccessible::GetChildAtIndex);
  AddFunctionToInterface(desc, "GetChildren", &BridgeAccessible::GetChildren);
  AddFunctionToInterface(desc, "GetIndexInParent", &BridgeAccessible::GetIndexInParent);
  AddFunctionToInterface(desc, "GetNavigableAtPoint", &BridgeAccessible::GetNavigableAtPoint);
  AddFunctionToInterface(desc, "GetNeighbor", &BridgeAccessible::GetNeighbor);
  AddFunctionToInterface(desc, "GetDefaultLabelInfo", &BridgeAccessible::GetDefaultLabelInfo);
  AddFunctionToInterface(desc, "DoGesture", &BridgeAccessible::DoGesture);
  AddFunctionToInterface(desc, "GetReadingMaterial", &BridgeAccessible::GetReadingMaterial);
  AddFunctionToInterface(desc, "GetRelationSet", &BridgeAccessible::GetRelationSet);
  AddFunctionToInterface(desc, "SetListenPostRender", &BridgeAccessible::SetListenPostRender);
  AddFunctionToInterface(desc, "GetNodeInfo", &BridgeAccessible::GetNodeInfo);
  mDbusServer.addInterface("/", desc, true);
}

Accessible* BridgeAccessible::FindSelf() const
{
  return FindCurrentObject();
}

Component* BridgeAccessible::GetObjectInRelation(Accessible* obj, RelationType relationType)
{
  if(!obj)
  {
    return nullptr;
  }

  for(auto& relation : obj->GetRelationSet())
  {
    if(relation.mRelationType == relationType)
    {
      for(auto& target : relation.mTargets)
      {
        auto component = dynamic_cast<Component*>(target);
        if(component)
        {
          return component;
        }
      }
    }
  }
  return nullptr;
}

Component* BridgeAccessible::CalculateNavigableAccessibleAtPoint(Accessible* root, Point point, CoordinateType type, unsigned int maxRecursionDepth)
{
  if(!root || maxRecursionDepth == 0)
  {
    return nullptr;
  }

  // When the layer is 3D behaviour, hit test algorithm needs to be used to find the correct actor. This is because the z-order is not considered in the normal way.
  auto actor = root->GetInternalActor();
  if(actor)
  {
    Dali::Actor layer = actor.GetLayer();
    if(layer && layer.GetProperty<Dali::Layer::Behavior>(Dali::Layer::Property::BEHAVIOR) == Dali::Layer::Behavior::LAYER_3D)
    {
      Dali::HitTestAlgorithm::Results hitTestResults;
      Dali::HitTestAlgorithm::HitTest(Dali::Stage::GetCurrent(), Dali::Vector2(point.x, point.y), hitTestResults, IsActorAccessibleFunction);
      if(hitTestResults.actor)
      {
        return dynamic_cast<Component*>(Accessible::Get(hitTestResults.actor));
      }
      else
      {
        return nullptr;
      }
    }
  }

  auto rootComponent = dynamic_cast<Component*>(root);
  LOG() << "CalculateNavigableAccessibleAtPoint: checking: " << MakeIndent(maxRecursionDepth) << GetComponentInfo(rootComponent);

  bool        forceChildSearch     = false;
  const auto& attributes           = root->GetAttributes();
  auto        forceChildSearchAttr = attributes.find(FORCE_CHILD_SEARCH_ATTR);
  if(forceChildSearchAttr != attributes.end())
  {
    DALI_LOG_RELEASE_INFO("Force child search attr is set.");
    forceChildSearch = std::atoi(forceChildSearchAttr->second.c_str()) == 1;
  }

  if(rootComponent && !forceChildSearch && !rootComponent->IsAccessibleContainingPoint(point, type))
  {
    return nullptr;
  }

  auto children = root->GetChildren();
  for(auto childIt = children.rbegin(); childIt != children.rend(); childIt++)
  {
    //check recursively all children first
    auto result = CalculateNavigableAccessibleAtPoint(*childIt, point, type, maxRecursionDepth - 1);
    if(result)
    {
      return result;
    }
  }

  if(rootComponent)
  {
    //Found a candidate, all its children are already checked
    auto controledBy = GetObjectInRelation(rootComponent, RelationType::CONTROLLED_BY);
    if(!controledBy)
    {
      controledBy = rootComponent;
    }

    if(controledBy->IsProxy() || IsObjectAcceptable(controledBy))
    {
      LOG() << "CalculateNavigableAccessibleAtPoint: found:    " << MakeIndent(maxRecursionDepth) << GetComponentInfo(rootComponent) << " " << controledBy->IsProxy();
      return controledBy;
    }
  }
  return nullptr;
}

BridgeAccessible::ReadingMaterialType BridgeAccessible::GetReadingMaterial()
{
  auto self                     = FindSelf();
  auto findObjectByRelationType = [this, &self](RelationType relationType) {
    auto relations = self->GetRelationSet();
    auto relation  = std::find_if(relations.begin(),
                                 relations.end(),
                                 [relationType](const Dali::Accessibility::Relation& relation) -> bool {
                                   return relation.mRelationType == relationType;
                                 });
    return relations.end() != relation && !relation->mTargets.empty() ? relation->mTargets.back() : nullptr;
  };

  auto        labellingObject = findObjectByRelationType(RelationType::LABELLED_BY);
  std::string labeledByName   = labellingObject ? labellingObject->GetName() : "";

  auto describedByObject = findObjectByRelationType(RelationType::DESCRIBED_BY);
  auto attributes        = self->GetAttributes();

  double      currentValue = 0.0;
  std::string currentValueText;
  double      minimumIncrement = 0.0;
  double      maximumValue     = 0.0;
  double      minimumValue     = 0.0;
  auto*       valueInterface   = Value::DownCast(self);
  if(valueInterface)
  {
    currentValue     = valueInterface->GetCurrent();
    currentValueText = valueInterface->GetValueText();
    minimumIncrement = valueInterface->GetMinimumIncrement();
    maximumValue     = valueInterface->GetMaximum();
    minimumValue     = valueInterface->GetMinimum();
  }
  else
  {
    // value text support outside of IAtspiValue interface
    currentValueText = self->GetValue();
    if(!currentValueText.empty())
    {
      attributes.insert({VALUE_FORMAT_KEY, VALUE_FORMAT_TEXT_VAL});
    }
  }

  int32_t firstSelectedChildIndex = -1;
  int32_t selectedChildCount      = 0;
  auto*   selfSelectionInterface  = Selection::DownCast(self);
  if(selfSelectionInterface)
  {
    selectedChildCount      = selfSelectionInterface->GetSelectedChildrenCount();
    auto firstSelectedChild = selfSelectionInterface->GetSelectedChild(0);
    if(firstSelectedChild)
    {
      firstSelectedChildIndex = firstSelectedChild->GetIndexInParent();
    }
  }

  auto childCount       = static_cast<int32_t>(self->GetChildCount());
  bool hasCheckBoxChild = false;
  for(auto i = 0u; i < static_cast<size_t>(childCount); ++i)
  {
    auto child = self->GetChildAtIndex(i);
    if(child->GetRole() == Role::CHECK_BOX)
    {
      hasCheckBoxChild = true;
      break;
    }
  }

  auto    itemCount         = attributes.find("item_count");
  auto    atspiRole         = self->GetRole();
  int32_t listChildrenCount = 0;
  if(itemCount != attributes.end())
  {
    // "item_count" gives manual control to the application, so it has priority
    listChildrenCount = std::atoi(itemCount->second.c_str());
  }
  else if(atspiRole == Role::DIALOG)
  {
    listChildrenCount = GetItemCountOfFirstDescendantContainer(self, Role::LIST, Role::LIST_ITEM, true);
  }
  else if(atspiRole == Role::POPUP_MENU)
  {
    listChildrenCount = GetItemCountOfFirstDescendantContainer(self, Role::POPUP_MENU, Role::MENU_ITEM, false);
  }

  auto*       textInterface         = Text::DownCast(self);
  std::string nameFromTextInterface = "";
  if(textInterface)
  {
    nameFromTextInterface = textInterface->GetText(0, textInterface->GetCharacterCount());
  }

  auto name              = self->GetName();
  auto role              = static_cast<uint32_t>(atspiRole);
  auto states            = self->GetStates();
  auto localizedRoleName = self->GetLocalizedRoleName();
  auto description       = self->GetDescription();
  auto indexInParent     = static_cast<int32_t>(self->GetIndexInParent());

  auto  parent                   = self->GetParent();
  auto  parentRole               = static_cast<uint32_t>(parent ? parent->GetRole() : Role{});
  auto  parentChildCount         = parent ? static_cast<int32_t>(parent->GetChildCount()) : 0;
  auto  parentStateSet           = parent ? parent->GetStates() : States{};
  bool  isSelectedInParent       = false;
  auto* parentSelectionInterface = Selection::DownCast(parent);
  if(parentSelectionInterface)
  {
    isSelectedInParent = parentSelectionInterface->IsChildSelected(indexInParent);
  }

  return {
    attributes,
    name,
    labeledByName,
    nameFromTextInterface,
    role,
    states,
    localizedRoleName,
    childCount,
    currentValue,
    currentValueText,
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

BridgeAccessible::NodeInfoType BridgeAccessible::GetNodeInfo()
{
  auto self        = FindSelf();
  auto roleName    = self->GetRoleName();
  auto name        = self->GetName();
  auto toolkitName = "dali";
  auto attributes  = self->GetAttributes();
  auto states      = self->GetStates();

  auto*        component     = Component::DownCast(self);
  Dali::Rect<> screenExtents = {0, 0, 0, 0};
  Dali::Rect<> windowExtents = {0, 0, 0, 0};
  if(component)
  {
    screenExtents = component->GetExtents(CoordinateType::SCREEN);
    windowExtents = component->GetExtents(CoordinateType::WINDOW);

    screenExtents.x += mData->mExtentsOffset.first;
    screenExtents.y += mData->mExtentsOffset.second;
    windowExtents.x += mData->mExtentsOffset.first;
    windowExtents.y += mData->mExtentsOffset.second;
  }

  auto*  valueInterface   = Value::DownCast(self);
  double currentValue     = 0.0;
  double minimumIncrement = 0.0;
  double maximumValue     = 0.0;
  double minimumValue     = 0.0;
  if(valueInterface)
  {
    currentValue     = valueInterface->GetCurrent();
    minimumIncrement = valueInterface->GetMinimumIncrement();
    maximumValue     = valueInterface->GetMaximum();
    minimumValue     = valueInterface->GetMinimum();
  }

  return {
    roleName,
    name,
    toolkitName,
    attributes,
    states,
    {screenExtents.x, screenExtents.y, screenExtents.width, screenExtents.height},
    {windowExtents.x, windowExtents.y, windowExtents.width, windowExtents.height},
    currentValue,
    minimumIncrement,
    maximumValue,
    minimumValue};
}

DBus::ValueOrError<bool> BridgeAccessible::DoGesture(Dali::Accessibility::Gesture type, int32_t startPositionX, int32_t startPositionY, int32_t endPositionX, int32_t endPositionY, Dali::Accessibility::GestureState state, uint32_t eventTime)
{
  // Please be aware of sending GestureInfo point in the different order with parameters
  return FindSelf()->DoGesture(Dali::Accessibility::GestureInfo{type, startPositionX, endPositionX, startPositionY, endPositionY, state, eventTime});
}

DBus::ValueOrError<Accessible*, uint8_t, Accessible*> BridgeAccessible::GetNavigableAtPoint(int32_t x, int32_t y, uint32_t coordinateType)
{
  Accessible* deputy     = nullptr;
  auto        accessible = FindSelf();
  auto        cType      = static_cast<CoordinateType>(coordinateType);

  const bool hasExtentsOffset = mData->mExtentsOffset.first != 0 || mData->mExtentsOffset.second != 0;
  if(hasExtentsOffset && !IsSubWin(accessible))
  {
    x -= mData->mExtentsOffset.first;
    y -= mData->mExtentsOffset.second;
  }

  LOG() << "GetNavigableAtPoint: " << x << ", " << y << " type: " << coordinateType;
  auto component = CalculateNavigableAccessibleAtPoint(accessible, {x, y}, cType, GET_NAVIGABLE_AT_POINT_MAX_RECURSION_DEPTH);
  bool recurse   = false;
  if(component)
  {
    recurse = component->IsProxy();
    if(recurse)
    {
      Accessible* parent = component;
      do
      {
        parent = parent->GetParent();
        if(IsObjectAcceptable(parent))
        {
          deputy = parent;
          LOG() << "deputy:    " << GetComponentInfo(dynamic_cast<Component*>(deputy));
          break;
        }
      } while(parent && parent != accessible);
    }
  }
  return {component, recurse, deputy};
}

Accessible* BridgeAccessible::GetCurrentlyHighlighted()
{
  return Accessible::Get(mData->mCurrentlyHighlightedActor);
}

std::vector<Component*> BridgeAccessible::GetValidChildren(const std::vector<Accessible*>& children, Accessible* start)
{
  if(children.empty())
  {
    return {};
  }

  std::vector<Component*> vec;

  Dali::Rect<> scrollableParentExtents;
  auto         nonDuplicatedScrollableParents = GetNonDuplicatedScrollableParents(children.front(), start);
  if(!nonDuplicatedScrollableParents.empty())
  {
    scrollableParentExtents = nonDuplicatedScrollableParents.front()->GetExtents(CoordinateType::WINDOW);
  }

  for(auto child : children)
  {
    auto* component = dynamic_cast<Component*>(child);
    if(component)
    {
      if(nonDuplicatedScrollableParents.empty() || scrollableParentExtents.Intersects(component->GetExtents(CoordinateType::WINDOW)))
      {
        vec.push_back(component);
      }
    }
  }

  return vec;
}

void BridgeAccessible::SortChildrenFromTopLeft(std::vector<Dali::Accessibility::Component*>& children)
{
  if(children.empty())
  {
    return;
  }

  std::vector<Component*> sortedChildren;

  std::sort(children.begin(), children.end(), &SortVertically);

  for(auto& line : SplitLines(children))
  {
    std::sort(line.begin(), line.end(), &SortHorizontally);
    sortedChildren.insert(sortedChildren.end(), line.begin(), line.end());
  }

  children = sortedChildren;
}

template<class T>
struct CycleDetection
{
  CycleDetection(const T value)
  : mKey(value),
    mCurrentSearchSize(1),
    mCounter(1)
  {
  }

  bool Check(const T value)
  {
    if(mKey == value)
    {
      return true;
    }

    if(--mCounter == 0)
    {
      mCurrentSearchSize <<= 1;
      if(mCurrentSearchSize == 0)
      {
        return true; // UNDEFINED BEHAVIOR
      }
      mCounter = mCurrentSearchSize;
      mKey     = value;
    }
    return false;
  }

  T            mKey;
  unsigned int mCurrentSearchSize;
  unsigned int mCounter;
};

Accessible* BridgeAccessible::GetNextNonDefunctSibling(Accessible* obj, Accessible* start, unsigned char forward)
{
  if(!obj)
  {
    return nullptr;
  }

  auto parent = obj->GetParent();
  if(!parent)
  {
    return nullptr;
  }
  else if(parent->IsProxy())
  {
    return parent;
  }

  auto children = GetValidChildren(parent->GetChildren(), start);
  SortChildrenFromTopLeft(children);

  unsigned int childrenCount = children.size();
  if(childrenCount == 0)
  {
    return nullptr;
  }

  unsigned int current = 0;
  for(; current < childrenCount && children[current] != obj; ++current)
  {
  }

  if(current >= childrenCount)
  {
    return nullptr;
  }

  forward ? ++current : --current;
  auto ret = FindNonDefunctChild(children, current, forward, start);
  return ret;
}

Accessible* BridgeAccessible::FindNonDefunctSibling(bool& areAllChildrenVisited, Accessible* node, Accessible* start, Accessible* root, unsigned char forward)
{
  while(true)
  {
    Accessible* sibling = GetNextNonDefunctSibling(node, start, forward);
    if(sibling)
    {
      node                  = sibling;
      areAllChildrenVisited = false; // Note that this is passed by non-const reference, so it is the caller that can determine whether this search exhausted all children.
      break;
    }
    // walk up...
    node = node->GetParent();
    if(node == nullptr || node == root)
    {
      return nullptr;
    }

    // in backward traversing stop the walk up on parent
    if(!forward)
    {
      break;
    }
  }

  return node;
}

Accessible* BridgeAccessible::CalculateNeighbor(Accessible* root, Accessible* start, unsigned char forward, BridgeAccessible::NeighborSearchMode searchMode)
{
  if(start && CheckChainEndWithAttribute(start, forward))
  {
    return start;
  }
  if(root && root->GetStates()[State::DEFUNCT])
  {
    return NULL;
  }
  if(start && start->GetStates()[State::DEFUNCT])
  {
    start   = NULL;
    forward = 1;
  }

  if(searchMode == BridgeAccessible::NeighborSearchMode::RECURSE_TO_OUTSIDE)
  {
    searchMode = BridgeAccessible::NeighborSearchMode::CONTINUE_AFTER_FAILED_RECURSION;
  }

  Accessible* node = start ? start : root;
  if(!node)
  {
    return nullptr;
  }

  // initialization of all-children-visited flag for start node - we assume
  // that when we begin at start node and we navigate backward, then all children
  // are visited, so navigation will ignore start's children and go to
  // previous sibling available.
  // Regarding condtion (start != root):
  // The last object can be found only if areAllChildrenVisited is false.
  // The start is same with root, when looking for the last object.
  bool areAllChildrenVisited = (start != root) && (searchMode != BridgeAccessible::NeighborSearchMode::RECURSE_FROM_ROOT && !forward);

  // true, if starting element should be ignored. this is only used in rare case of
  // recursive search failing to find an object.
  // consider tree, where element A on bus BUS_A has child B on bus BUS_B. when going "next" from
  // element A algorithm has to descend into BUS_B and search element B and its children. this is done
  // by returning to our caller object B with special flag set (meaning - continue the search from B on bus BUS_B).
  // if next object will be found there (on BUS_B), then search ends. but if not, then our caller will find it out
  // and will call us again with object A and flag searchMode set to NEIGHBOR_SEARCH_MODE_CONTINUE_AFTER_FAILED_RECURSING.
  // this flag means, that object A was already checked previously and we should skip it and its children.
  bool forceNext = (searchMode == BridgeAccessible::NeighborSearchMode::CONTINUE_AFTER_FAILED_RECURSION);

  CycleDetection<Accessible*> cycleDetection(node);
  while(node)
  {
    if(node->GetStates()[State::DEFUNCT])
    {
      return nullptr;
    }

    // always accept proxy object from different world
    if(!forceNext && node->IsProxy())
    {
      return node;
    }

    auto children = GetValidChildren(node->GetChildren(), start);
    SortChildrenFromTopLeft(children);

    // do accept:
    // 1. not start node
    // 2. parent after all children in backward traversing
    // 3. Nodes with roles: ATSPI_ROLE_PAGE_TAB, ATSPI_ROLE_POPUP_MENU and ATSPI_ROLE_DIALOG, only when looking for first or last element.
    //    Objects with those roles shouldnt be reachable, when navigating next / prev.
    bool areAllChildrenVisitedOrMovingForward = (children.size() == 0 || forward || areAllChildrenVisited);

    if(!forceNext && node != start && areAllChildrenVisitedOrMovingForward && IsObjectAcceptable(node) && IsChildVisibleInScrollableParent(start, node))
    {
      if(start == NULL || IsRoleAcceptableWhenNavigatingNextPrev(node))
      {
        return node;
      }
    }

    Accessible* nextRelatedInDirection = !forceNext ? GetObjectInRelation(node, forward ? RelationType::FLOWS_TO : RelationType::FLOWS_FROM) : nullptr;
    if(nextRelatedInDirection && start && start->GetStates()[State::DEFUNCT])
    {
      nextRelatedInDirection = NULL;
    }

    unsigned char wantCycleDetection = 0;
    if(nextRelatedInDirection)
    {
      node               = nextRelatedInDirection;
      wantCycleDetection = 1;
    }
    else
    {
      auto child = !forceNext && !areAllChildrenVisited ? FindNonDefunctChildWithDepthFirstSearch(node, children, forward, start) : nullptr;
      if(child)
      {
        wantCycleDetection = 1;
      }
      else
      {
        if(!forceNext && node == root)
        {
          return NULL;
        }
        areAllChildrenVisited = true;
        child                 = FindNonDefunctSibling(areAllChildrenVisited, node, start, root, forward);
      }
      node = child;
    }

    forceNext = 0;
    if(wantCycleDetection && cycleDetection.Check(node))
    {
      return NULL;
    }
  }
  return NULL;
}

DBus::ValueOrError<Accessible*, uint8_t> BridgeAccessible::GetNeighbor(std::string rootPath, int32_t direction, int32_t searchMode)
{
  auto          start      = FindSelf();
  auto          root       = !rootPath.empty() ? Find(StripPrefix(rootPath)) : nullptr;
  auto          accessible = CalculateNeighbor(root, start, direction == 1, static_cast<NeighborSearchMode>(searchMode));
  unsigned char recurse    = 0;
  if(accessible)
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
  auto parent = FindSelf()->GetParent();

  return parent;
}

DBus::ValueOrError<std::vector<Accessible*>> BridgeAccessible::GetChildren()
{
  return FindSelf()->GetChildren();
}

std::string BridgeAccessible::GetDescription()
{
  return FindSelf()->GetDescription();
}

DBus::ValueOrError<uint32_t> BridgeAccessible::GetRole()
{
  return static_cast<unsigned int>(FindSelf()->GetRole());
}

DBus::ValueOrError<std::string> BridgeAccessible::GetRoleName()
{
  return FindSelf()->GetRoleName();
}

DBus::ValueOrError<std::string> BridgeAccessible::GetLocalizedRoleName()
{
  return FindSelf()->GetLocalizedRoleName();
}

DBus::ValueOrError<int32_t> BridgeAccessible::GetIndexInParent()
{
  return FindSelf()->GetIndexInParent();
}

DBus::ValueOrError<std::array<uint32_t, 2>> BridgeAccessible::GetStates()
{
  return FindSelf()->GetStates().GetRawData();
}

DBus::ValueOrError<std::map<std::string, std::string>> BridgeAccessible::GetAttributes()
{
  auto                               self       = FindSelf();
  std::map<std::string, std::string> attributes = self->GetAttributes();

  if(mIsScreenReaderSuppressed)
  {
    attributes.insert({"suppress-screen-reader", "true"});
  }

  auto* valueInterface = Value::DownCast(self);
  if(!valueInterface && !self->GetValue().empty())
  {
    attributes.insert({VALUE_FORMAT_KEY, VALUE_FORMAT_TEXT_VAL});
  }

  return attributes;
}

DBus::ValueOrError<std::vector<std::string>> BridgeAccessible::GetInterfacesAsStrings()
{
  return FindSelf()->GetInterfacesAsStrings();
}

int BridgeAccessible::GetChildCount()
{
  return FindSelf()->GetChildCount();
}

DBus::ValueOrError<Accessible*> BridgeAccessible::GetChildAtIndex(int index)
{
  if(index < 0)
  {
    throw std::domain_error{"negative index (" + std::to_string(index) + ")"};
  }
  return FindSelf()->GetChildAtIndex(static_cast<size_t>(index));
}

std::string BridgeAccessible::GetName()
{
  return FindSelf()->GetName();
}

DBus::ValueOrError<Accessible*, uint32_t, std::map<std::string, std::string>> BridgeAccessible::GetDefaultLabelInfo()
{
  auto* defaultLabel = GetDefaultLabel(FindSelf());
  DALI_ASSERT_DEBUG(defaultLabel);

  // By default, the text is taken from navigation context root's accessibility properties name and description.
  return {defaultLabel, static_cast<uint32_t>(defaultLabel->GetRole()), defaultLabel->GetAttributes()};
}

DBus::ValueOrError<std::vector<BridgeAccessible::Relation>> BridgeAccessible::GetRelationSet()
{
  auto                                    relations = FindSelf()->GetRelationSet();
  std::vector<BridgeAccessible::Relation> ret;

  for(auto& it : relations)
  {
    ret.emplace_back(Relation{static_cast<uint32_t>(it.mRelationType), it.mTargets});
  }

  return ret;
}

DBus::ValueOrError<void> BridgeAccessible::SetListenPostRender(bool enabled)
{
  FindSelf()->SetListenPostRender(enabled);
  return {};
}
