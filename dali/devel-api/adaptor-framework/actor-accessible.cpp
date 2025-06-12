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
#include <dali/devel-api/adaptor-framework/actor-accessible.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/actors/actor-devel.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>

using namespace Dali::Accessibility;

namespace
{
bool UpdateLastEmitted(std::map<State, int>& lastEmitted, State state, int newValue)
{
  bool updated                = false;
  const auto [iter, inserted] = lastEmitted.emplace(state, newValue);
  if(!inserted && iter->second != newValue)
  {
    iter->second = newValue;
    updated      = true;
  }

  return inserted || updated;
}

bool IsModalRole(Role role)
{
  return role == Role::POPUP_MENU || role == Role::PANEL || role == Role::DIALOG || role == Role::PAGE_TAB;
}

bool IsWindowRole(Role role)
{
  return role == Role::WINDOW || role == Role::FRAME || role == Role::INPUT_METHOD_WINDOW;
}

bool ShouldEmitVisible(Accessible* accessible)
{
  Role role = accessible->GetRole();
  return IsWindowRole(role);
}

bool ShouldEmitShowing(Accessible* accessible, bool showing)
{
  Role role = accessible->GetRole();
  return IsWindowRole(role) || IsModalRole(role) || (showing && role == Role::NOTIFICATION) ||
         (!showing && accessible->IsHighlighted()) || accessible->GetStates()[State::MODAL];
}

} // namespace

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

  const bool shouldIncludeHidden = Bridge::GetCurrentBridge()->ShouldIncludeHidden();

  // Erase-remove idiom
  // TODO (C++20): Replace with std::erase_if
  auto it = std::remove_if(mChildren.begin(), mChildren.end(), [shouldIncludeHidden](const Accessible* child) { return !child || (!shouldIncludeHidden && child->IsHidden()); });
  mChildren.erase(it, mChildren.end());
  mChildren.shrink_to_fit();
}

void ActorAccessible::EmitActiveDescendantChanged(Accessible* child)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitActiveDescendantChanged(this, child);
  }
}

void ActorAccessible::EmitStateChanged(State state, int newValue, int reserved)
{
  if(auto bridgeData = GetBridgeData())
  {
    bool shouldEmit{false};

    switch(state)
    {
      case State::SHOWING:
      {
        shouldEmit = ShouldEmitShowing(this, static_cast<bool>(newValue));
        break;
      }
      case State::VISIBLE:
      {
        shouldEmit = ShouldEmitVisible(this);
        break;
      }
      default:
      {
        shouldEmit = UpdateLastEmitted(mLastEmittedState, state, newValue);
        break;
      }
    }

    if(shouldEmit)
    {
      try
      {
        bridgeData->mBridge->EmitStateChanged(shared_from_this(), state, newValue, reserved);
      }
      catch(const std::bad_weak_ptr& e)
      {
        DALI_LOG_ERROR("bad_weak_ptr exception caught: %s", e.what());
      }
    }
  }
}

void ActorAccessible::EmitShowing(bool isShowing)
{
  EmitStateChanged(State::SHOWING, isShowing ? 1 : 0);
}

void ActorAccessible::EmitVisible(bool isVisible)
{
  EmitStateChanged(State::VISIBLE, isVisible ? 1 : 0);
}

void ActorAccessible::EmitHighlighted(bool isHighlighted)
{
  EmitStateChanged(State::HIGHLIGHTED, isHighlighted ? 1 : 0);
}

void ActorAccessible::EmitFocused(bool isFocused)
{
  EmitStateChanged(State::FOCUSED, isFocused ? 1 : 0);
}

void ActorAccessible::EmitTextInserted(unsigned int position, unsigned int length, const std::string& content)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitTextChanged(this, TextChangedState::INSERTED, position, length, content);
  }
}
void ActorAccessible::EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitTextChanged(this, TextChangedState::DELETED, position, length, content);
  }
}
void ActorAccessible::EmitTextCursorMoved(unsigned int cursorPosition)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitCursorMoved(this, cursorPosition);
  }
}

void ActorAccessible::EmitMovedOutOfScreen(ScreenRelativeMoveType type)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitMovedOutOfScreen(this, type);
  }
}

void ActorAccessible::EmitScrollStarted()
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitScrollStarted(this);
  }
}

void ActorAccessible::EmitScrollFinished()
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->EmitScrollFinished(this);
  }
}

void ActorAccessible::Emit(WindowEvent event, unsigned int detail)
{
  if(auto bridgeData = GetBridgeData())
  {
    bridgeData->mBridge->Emit(this, event, detail);
  }
}
void ActorAccessible::Emit(ObjectPropertyChangeEvent event)
{
  if(auto bridgeData = GetBridgeData())
  {
    try
    {
      bridgeData->mBridge->Emit(shared_from_this(), event);
    }
    catch(const std::bad_weak_ptr& e)
    {
      DALI_LOG_ERROR("bad_weak_ptr exception caught: %s", e.what());
    }
  }
}

void ActorAccessible::EmitBoundsChanged(Rect<> rect)
{
  if(auto bridgeData = GetBridgeData())
  {
    try
    {
      bridgeData->mBridge->EmitBoundsChanged(shared_from_this(), rect);
    }
    catch(const std::bad_weak_ptr& e)
    {
      DALI_LOG_ERROR("bad_weak_ptr exception caught: %s", e.what());
    }
  }
}

void ActorAccessible::NotifyAccessibilityStateChange(Dali::Accessibility::States states, bool isRecursive)
{
  if(Accessibility::IsUp())
  {
    const auto newStates = GetStates();
    for(auto i = 0u; i < static_cast<unsigned int>(Dali::Accessibility::State::MAX_COUNT); i++)
    {
      const auto index = static_cast<Dali::Accessibility::State>(i);
      if(states[index])
      {
        EmitStateChanged(index, newStates[index]);
      }
    }

    if(isRecursive)
    {
      auto children = GetChildren();
      for(auto child : children)
      {
        if(auto accessible = dynamic_cast<ActorAccessible*>(child))
        {
          accessible->NotifyAccessibilityStateChange(states, isRecursive);
        }
      }
    }
  }
}

void ActorAccessible::ClearCache()
{
  mLastEmittedState.clear();
}

} // namespace Dali::Accessibility
