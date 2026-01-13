/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/object/type-info.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>

namespace Dali::Accessibility
{

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

Address GetAddressByActorId(uint32_t actorId)
{
  if(auto bridge = Bridge::GetCurrentBridge())
  {
    return {bridge->GetBusName(), std::to_string(actorId)};
  }
  return {};
}

} // namespace

ActorAccessible::ActorAccessible(Actor actor)
: Dali::BaseObjectObserver(actor),
  mSelf(actor),
  mChildrenDirty{true}, // to trigger the initial UpdateChildren()
  mIsBeingDestroyed{false},
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
  mIsBeingDestroyed = true;
  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->RemoveAccessible(mActorId);
  }
}

Address ActorAccessible::GetAddress() const
{
  return GetAddressByActorId(mActorId);
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
    if(auto bridge = Bridge::GetCurrentBridge())
    {
      return bridge->GetApplication();
    }
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

std::string ActorAccessible::GetStringProperty(std::string propertyName) const
{
  return {};
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

Dali::Rect<float> ActorAccessible::GetExtents(CoordinateType type) const
{
  Dali::Actor actor   = Self();
  auto        extents = DevelActor::CalculateScreenExtents(actor);

  if(Dali::EqualsZero(extents.width) && Dali::EqualsZero(extents.height) && CanAcceptZeroSize())
  {
    extents.width  = 1.f;
    extents.height = 1.f;
  }

  auto rounded = Dali::Rect<float>{std::round(extents.x), std::round(extents.y), std::round(extents.width), std::round(extents.height)};

  if(type == CoordinateType::WINDOW)
  {
    return rounded;
  }
  else // CoordinateType::SCREEN
  {
    auto window         = Dali::DevelWindow::Get(actor);
    auto windowPosition = window.GetPosition();
    return {rounded.x + windowPosition.GetX(), rounded.y + windowPosition.GetY(), rounded.width, rounded.height};
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

AtspiInterfaces ActorAccessible::DoGetInterfaces() const
{
  AtspiInterfaces interfaces             = Accessible::DoGetInterfaces();
  interfaces[AtspiInterface::COLLECTION] = true;

  return interfaces;
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

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    const bool shouldIncludeHidden = bridge->ShouldIncludeHidden();

    // Erase-remove idiom
    // TODO (C++20): Replace with std::erase_if
    auto it = std::remove_if(mChildren.begin(), mChildren.end(), [shouldIncludeHidden](const Accessible* child)
    { return !child || (!shouldIncludeHidden && child->IsHidden()); });
    mChildren.erase(it, mChildren.end());
    mChildren.shrink_to_fit();
  }
}

void ActorAccessible::EmitActiveDescendantChanged(Accessible* child)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitActiveDescendantChanged(this, child);
  }
}

void ActorAccessible::EmitStateChanged(State state, int newValue, int reserved)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bool shouldEmit{false};

    switch(state)
    {
      case State::CHECKED:
      case State::SELECTED:
      {
        shouldEmit = true;
        break;
      }
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
        bridge->EmitStateChanged(shared_from_this(), state, newValue, reserved);
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
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitTextChanged(this, TextChangedState::INSERTED, position, length, content);
  }
}
void ActorAccessible::EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitTextChanged(this, TextChangedState::DELETED, position, length, content);
  }
}
void ActorAccessible::EmitTextCursorMoved(unsigned int cursorPosition)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitCursorMoved(this, cursorPosition);
  }
}

void ActorAccessible::EmitMovedOutOfScreen(ScreenRelativeMoveType type)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitMovedOutOfScreen(this, type);
  }
}

void ActorAccessible::EmitScrollStarted()
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitScrollStarted(this);
  }
}

void ActorAccessible::EmitScrollFinished()
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->EmitScrollFinished(this);
  }
}

void ActorAccessible::Emit(WindowEvent event, unsigned int detail)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    bridge->Emit(this, event, detail);
  }
}

void ActorAccessible::Emit(ObjectPropertyChangeEvent event)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    try
    {
      bridge->Emit(shared_from_this(), event);
    }
    catch(const std::bad_weak_ptr& e)
    {
      DALI_LOG_ERROR("bad_weak_ptr exception caught: %s", e.what());
    }
  }
}

void ActorAccessible::EmitBoundsChanged(Rect<int> rect)
{
  if(mIsBeingDestroyed)
  {
    return;
  }

  if(auto bridge = Bridge::GetCurrentBridge())
  {
    try
    {
      bridge->EmitBoundsChanged(shared_from_this(), rect);
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

bool ActorAccessible::CanAcceptZeroSize() const
{
  auto layer = Self().GetLayer();
  return layer && layer.GetProperty<Dali::Layer::Behavior>(Dali::Layer::Property::BEHAVIOR) == Dali::Layer::Behavior::LAYER_3D;
}

} // namespace Dali::Accessibility
