/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/actor-accessible.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/actors/actor-devel.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>

namespace Dali::Accessibility
{
ActorAccessible::ActorAccessible(Actor actor)
: Dali::BaseObjectObserver(actor),
  mSelf(actor),
  mChildrenDirty{true}, // to trigger the initial UpdateChildren()
  mActorId{static_cast<uint32_t>(actor.GetProperty<int>(Dali::Actor::Property::ID))}
{
  // Select the right overload manually because Connect(this, &OnChildrenChanged) is ambiguous.
  void (ActorAccessible::*handler)(Dali::Actor) = &ActorAccessible::OnChildrenChanged;

  Dali::DevelActor::ChildAddedSignal(actor).Connect(this, handler);
  Dali::DevelActor::ChildRemovedSignal(actor).Connect(this, handler);
  Dali::DevelActor::ChildOrderChangedSignal(actor).Connect(this, handler);
}

void ActorAccessible::ObjectDestroyed()
{
  Bridge::GetCurrentBridge()->RemoveAccessible(mActorId);
}

std::string ActorAccessible::GetName() const
{
  return Self().GetProperty<std::string>(Dali::Actor::Property::NAME);
}

std::string ActorAccessible::GetDescription() const
{
  return {};
}

std::string ActorAccessible::GetValue() const
{
  return {};
}

Accessible* ActorAccessible::GetParent()
{
  if(IsOnRootLevel())
  {
    auto data = GetBridgeData();
    return data->mBridge->GetApplication();
  }

  return Get(Self().GetParent());
}

std::size_t ActorAccessible::GetChildCount() const
{
  // ActorAccessible is never stored in const memory and it is an implementation detail that
  // children are recalculated lazily.
  const_cast<ActorAccessible*>(this)->UpdateChildren();

  return mChildren.size();
}

std::vector<Accessible*> ActorAccessible::GetChildren()
{
  UpdateChildren();

  return mChildren;
}

Accessible* ActorAccessible::GetChildAtIndex(std::size_t index)
{
  auto childCount = GetChildCount(); // calls UpdateChildren()
  if(DALI_UNLIKELY(index >= childCount))
  {
    throw std::domain_error{"invalid index " + std::to_string(index) + " for object with " + std::to_string(childCount) + " children"};
  }

  return mChildren[index];
}

std::size_t ActorAccessible::GetIndexInParent()
{
  auto* parent = GetParent();

  if(DALI_UNLIKELY(!parent))
  {
    throw std::domain_error{"can't call GetIndexInParent on object without parent"};
  }

  // Avoid calling parent->GetChildren() in order not to make an unnecessary copy
  auto childCount = parent->GetChildCount();
  for(auto i = 0u; i < childCount; ++i)
  {
    if(parent->GetChildAtIndex(i) == this)
    {
      return i;
    }
  }

  throw std::domain_error{"actor is not a child of its parent"};
}

Dali::Actor ActorAccessible::GetInternalActor() const
{
  return Self();
}

ComponentLayer ActorAccessible::GetLayer() const
{
  return ComponentLayer::WINDOW;
}

std::int16_t ActorAccessible::GetMdiZOrder() const
{
  return 0;
}

double ActorAccessible::GetAlpha() const
{
  return 0;
}

bool ActorAccessible::IsScrollable() const
{
  return false;
}

Dali::Rect<> ActorAccessible::GetExtents(CoordinateType type) const
{
  Dali::Actor actor                   = Self();
  Vector2     screenPosition          = actor.GetProperty<Vector2>(Actor::Property::SCREEN_POSITION);
  Vector3     size                    = actor.GetCurrentProperty<Vector3>(Actor::Property::SIZE) * actor.GetCurrentProperty<Vector3>(Actor::Property::WORLD_SCALE);
  bool        positionUsesAnchorPoint = actor.GetProperty<bool>(Actor::Property::POSITION_USES_ANCHOR_POINT);
  Vector3     anchorPointOffSet       = size * (positionUsesAnchorPoint ? actor.GetCurrentProperty<Vector3>(Actor::Property::ANCHOR_POINT) : AnchorPoint::TOP_LEFT);
  Vector2     position                = Vector2(screenPosition.x - anchorPointOffSet.x, screenPosition.y - anchorPointOffSet.y);

  if(type == CoordinateType::WINDOW)
  {
    return {position.x, position.y, size.x, size.y};
  }
  else // CoordinateType::SCREEN
  {
    auto window         = Dali::DevelWindow::Get(actor);
    auto windowPosition = window.GetPosition();
    return {position.x + windowPosition.GetX(), position.y + windowPosition.GetY(), size.x, size.y};
  }
}

void ActorAccessible::OnChildrenChanged()
{
  mChildrenDirty = true;
}

void ActorAccessible::OnChildrenChanged(Dali::Actor)
{
  OnChildrenChanged();
}

void ActorAccessible::DoGetChildren(std::vector<Accessible*>& children)
{
  auto self       = Self();
  auto childCount = self.GetChildCount();

  children.reserve(childCount);

  for(auto i = 0u; i < childCount; ++i)
  {
    children.push_back(Accessible::Get(self.GetChildAt(i)));
  }
}

void ActorAccessible::UpdateChildren()
{
  if(!mChildrenDirty)
  {
    return;
  }

  // Set to false before DoGetChildren() to prevent recursion
  // in case DoGetChildren() does something strange.
  mChildrenDirty = false;

  mChildren.clear();
  DoGetChildren(mChildren);

  // Erase-remove idiom
  // TODO (C++20): Replace with std::erase_if
  auto it = std::remove_if(mChildren.begin(), mChildren.end(), [](const Accessible* child) {
    return !child || child->IsHidden();
  });
  mChildren.erase(it, mChildren.end());
  mChildren.shrink_to_fit();
}

} // namespace Dali::Accessibility
