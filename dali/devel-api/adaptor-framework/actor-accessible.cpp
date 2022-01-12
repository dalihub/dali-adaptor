/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>

namespace Dali::Accessibility
{
ActorAccessible::ActorAccessible(Actor actor)
: mSelf(actor)
{
}

std::string ActorAccessible::GetName() const
{
  return Self().GetProperty<std::string>(Dali::Actor::Property::NAME);
}

std::string ActorAccessible::GetDescription() const
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
  return static_cast<std::size_t>(Self().GetChildCount());
}

std::vector<Accessible*> ActorAccessible::GetChildren()
{
  std::vector<Accessible*> tmp(GetChildCount());
  for(auto i = 0u; i < tmp.size(); ++i)
  {
    tmp[i] = GetChildAtIndex(i);
  }

  return tmp;
}

Accessible* ActorAccessible::GetChildAtIndex(std::size_t index)
{
  auto numberOfChildren = GetChildCount();
  if(DALI_UNLIKELY(index >= numberOfChildren))
  {
    throw std::domain_error{"invalid index " + std::to_string(index) + " for object with " + std::to_string(numberOfChildren) + " children"};
  }

  return Get(Self().GetChildAt(static_cast<std::uint32_t>(index)));
}

std::size_t ActorAccessible::GetIndexInParent()
{
  auto self   = Self();
  auto parent = self.GetParent();

  if(DALI_UNLIKELY(!parent))
  {
    throw std::domain_error{"can't call GetIndexInParent on object without parent"};
  }

  auto size = static_cast<std::size_t>(parent.GetChildCount());
  for(auto i = 0u; i < size; ++i)
  {
    if(parent.GetChildAt(i) == self)
    {
      return i;
    }
  }

  throw std::domain_error{"actor is not a child of its parent"};
}

Dali::Actor ActorAccessible::GetInternalActor()
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

} // namespace Dali::Accessibility
