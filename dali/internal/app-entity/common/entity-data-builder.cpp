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
#include <dali/internal/app-entity/common/entity-data-builder.h>

// EXTERNAL INCLUDES
#include <cmath>
#include <string>
#include <utility>

// INTERNAL INCLUDES
#include <dali/devel-api/actors/actor-devel.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-bridge.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/math/rect.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
using Dali::Integration::FocusedActorProvider;
namespace
{
#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ENTITY_DATA");
#endif

/**
 * @brief Returns the Accessible for an Actor via the current bridge, or null.
 *
 * The returned SharedPtr keeps the Accessible alive for the caller's scope.
 */
Dali::SharedPtr<Dali::Accessibility::Accessible> GetAccessibleForActor(Dali::Actor actor)
{
  auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge();
  if(!bridge)
  {
    return {};
  }
  return bridge->GetAccessible(actor);
}

} // unnamed namespace

bool ReadActorAnnotation(Dali::Actor actor, EntityData::Annotation& annotation)
{
  annotation = {};
  if(!actor)
  {
    return false;
  }

  Dali::String entityId;
  Dali::String entityType;
  Dali::String entityInfo;
  if(!actor.GetAnnotation(entityId, entityType, entityInfo))
  {
    return false;
  }

  annotation.entityId   = entityId.CStr();
  annotation.entityType = entityType.CStr();
  annotation.entityInfo = entityInfo.CStr();

  const bool hasAnnotation = !annotation.entityId.empty() && !annotation.entityType.empty();
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData ReadActorAnnotation: actor=%d has=%d entityId=%s entityType=%s entityInfo=%s\n",
                actor.GetProperty<int>(Dali::Actor::Property::ID), hasAnnotation,
                annotation.entityId.c_str(), annotation.entityType.c_str(),
                annotation.entityInfo.c_str());
  return hasAnnotation;
}

EntityData MakeEntityData(Dali::Actor actor, FocusedActorProvider* provider)
{
  EntityData::Annotation annotation;
  ReadActorAnnotation(actor, annotation);
  return MakeEntityData(actor, provider, std::move(annotation));
}

EntityData MakeEntityData(Dali::Actor actor, FocusedActorProvider* provider, EntityData::Annotation annotation)
{
  EntityData data;

  if(!actor)
  {
    return data;
  }

  // Id — Actor ID as string. Stable only for the lifetime of the Actor.
  data.id = std::to_string(actor.GetProperty<int>(Dali::Actor::Property::ID));

  // Type — always the runtime Actor type name.
  data.type = actor.GetTypeName().CStr();

  // Try the Accessibility Bridge first for description / bounds / state.
  Dali::SharedPtr<Dali::Accessibility::Accessible> accessible = GetAccessibleForActor(actor);

  if(accessible)
  {
    data.description = accessible->GetDescription();

    // Accessible derives from Component, so bounds are available directly.
    // Both coordinate systems are reported: absolute screen and window-relative.
    const auto screenExtents = accessible->GetExtents(Dali::Devel::Accessibility::CoordinateType::SCREEN);
    data.screenBounds.x      = screenExtents.x;
    data.screenBounds.y      = screenExtents.y;
    data.screenBounds.width  = screenExtents.width;
    data.screenBounds.height = screenExtents.height;

    const auto windowExtents = accessible->GetExtents(Dali::Devel::Accessibility::CoordinateType::WINDOW);
    data.windowBounds.x      = windowExtents.x;
    data.windowBounds.y      = windowExtents.y;
    data.windowBounds.width  = windowExtents.width;
    data.windowBounds.height = windowExtents.height;

    auto states    = accessible->GetStates();
    data.isEnabled = states[Dali::Integration::Accessibility::State::ENABLED];

    if(provider)
    {
      data.isFocused = (provider->GetFocusedActor() == actor);
    }
    else
    {
      data.isFocused = states[Dali::Integration::Accessibility::State::FOCUSED];
    }
  }
  else
  {
    // Match ActorAccessible::GetExtents(): CalculateCurrentScreenExtents() is
    // window-relative, and screen coordinates add the window origin.
    data.description = "";

    const Dali::Bounds extents = Dali::DevelActor::CalculateCurrentScreenExtents(actor);
    const Dali::Bounds rounded{std::round(extents.x), std::round(extents.y),
                               std::round(extents.width), std::round(extents.height)};
    data.windowBounds.x      = rounded.x;
    data.windowBounds.y      = rounded.y;
    data.windowBounds.width  = rounded.width;
    data.windowBounds.height = rounded.height;

    data.screenBounds = data.windowBounds;
    if(Dali::Window window = Dali::Window::Get(actor))
    {
      const Dali::PositionSize positionSize = window.GetPositionSize();
      data.screenBounds.x += static_cast<float>(positionSize.x);
      data.screenBounds.y += static_cast<float>(positionSize.y);
    }

    data.isEnabled = actor.GetProperty<bool>(Dali::Actor::Property::ENABLED);
    data.isFocused = provider ? (provider->GetFocusedActor() == actor) : false;
  }

  data.annotation = std::move(annotation);

  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData MakeEntityData: id=%s type=%s accessible=%d screenBounds=%f,%f,%f,%f windowBounds=%f,%f,%f,%f entityId=%s\n",
                data.id.c_str(), data.type.c_str(), accessible ? 1 : 0,
                data.screenBounds.x, data.screenBounds.y, data.screenBounds.width, data.screenBounds.height,
                data.windowBounds.x, data.windowBounds.y, data.windowBounds.width, data.windowBounds.height,
                data.annotation.entityId.c_str());
  return data;
}

bool IsActorVisible(Dali::Actor actor)
{
  if(!actor)
  {
    return false;
  }

  if(Dali::SharedPtr<Dali::Accessibility::Accessible> accessible = GetAccessibleForActor(actor))
  {
    auto states = accessible->GetStates();
    return states[Dali::Integration::Accessibility::State::SHOWING] &&
           states[Dali::Integration::Accessibility::State::VISIBLE];
  }

  return actor.GetProperty<bool>(Dali::Actor::Property::VISIBLE);
}

bool IsActorHierarchyVisible(Dali::Actor actor)
{
  for(Dali::Actor current = actor; current; current = current.GetParent())
  {
    if(!IsActorVisible(current))
    {
      return false;
    }
  }
  return static_cast<bool>(actor);
}

Dali::Bounds GetActorWindowBounds(Dali::Actor actor)
{
  Dali::Bounds bounds;
  if(!actor)
  {
    return bounds;
  }

  if(Dali::SharedPtr<Dali::Accessibility::Accessible> accessible = GetAccessibleForActor(actor))
  {
    const auto extents = accessible->GetExtents(Dali::Devel::Accessibility::CoordinateType::WINDOW);
    bounds.x           = extents.x;
    bounds.y           = extents.y;
    bounds.width       = extents.width;
    bounds.height      = extents.height;
    return bounds;
  }

  // Match ActorAccessible::GetExtents(): CalculateCurrentScreenExtents() is window-relative.
  const Dali::Bounds extents = Dali::DevelActor::CalculateCurrentScreenExtents(actor);
  bounds.x                   = std::round(extents.x);
  bounds.y                   = std::round(extents.y);
  bounds.width               = std::round(extents.width);
  bounds.height              = std::round(extents.height);
  return bounds;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
