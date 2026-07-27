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
#include <dali/internal/app-entity/common/entity-data-service.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <sstream>
#include <unordered_set>
#include <utility>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/adaptor-framework/window.h>

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
 * @brief Parses a decimal Actor ID string. Returns false on malformed input.
 */
bool ParseActorId(const std::string& id, uint32_t& outId)
{
  if(id.empty())
  {
    return false;
  }

  try
  {
    size_t             consumed = 0;
    const unsigned long value    = std::stoul(id, &consumed);
    if(consumed != id.size() || value > std::numeric_limits<uint32_t>::max())
    {
      return false;
    }
    outId = static_cast<uint32_t>(value);
    return true;
  }
  catch(...)
  {
    return false;
  }
}

/**
 * @brief Total ordering of Actor-ID strings.
 *
 * Plain decimal IDs sort before malformed IDs, then by numeric value. Exact
 * string order breaks ties and orders malformed IDs.
 */
bool IdLess(const std::string& lhs, const std::string& rhs)
{
  uint32_t leftActorId  = 0;
  uint32_t rightActorId = 0;
  const bool leftIsActorId  = ParseActorId(lhs, leftActorId);
  const bool rightIsActorId = ParseActorId(rhs, rightActorId);
  if(leftIsActorId != rightIsActorId)
  {
    return leftIsActorId;
  }
  if(leftIsActorId && leftActorId != rightActorId)
  {
    return leftActorId < rightActorId;
  }
  return lhs < rhs;
}

/**
 * @brief Returns a deterministic ordering for every floating-point value.
 *
 * Normal numeric ordering is used for finite values and infinities. All NaNs
 * are equivalent and sort after numeric values, preserving the strict weak
 * ordering required by std::sort.
 */
int CompareCoordinate(float lhs, float rhs)
{
  const bool lhsIsNan = std::isnan(lhs);
  const bool rhsIsNan = std::isnan(rhs);
  if(lhsIsNan || rhsIsNan)
  {
    if(lhsIsNan && rhsIsNan)
    {
      return 0;
    }
    return lhsIsNan ? 1 : -1;
  }

  if(lhs < rhs)
  {
    return -1;
  }
  if(rhs < lhs)
  {
    return 1;
  }
  return 0;
}

bool AreBoundsFinite(const Dali::Bounds& bounds)
{
  return std::isfinite(bounds.x) && std::isfinite(bounds.y) &&
         std::isfinite(bounds.width) && std::isfinite(bounds.height);
}

/**
 * @brief Escapes a string for embedding in a JSON string literal.
 */
std::string JsonEscape(const std::string& in)
{
  static constexpr char HEX_DIGITS[] = "0123456789abcdef";

  std::string out;
  out.reserve(in.size() + 2);
  for(const unsigned char c : in)
  {
    switch(c)
    {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if(c < 0x20u)
        {
          out += "\\u00";
          out += HEX_DIGITS[(c >> 4u) & 0x0fu];
          out += HEX_DIGITS[c & 0x0fu];
        }
        else
        {
          out += static_cast<char>(c);
        }
        break;
    }
  }
  return out;
}

Dali::Bounds IntersectBounds(const Dali::Bounds& lhs, const Dali::Bounds& rhs)
{
  if(!AreBoundsFinite(lhs) || !AreBoundsFinite(rhs))
  {
    return Dali::Bounds{};
  }

  const float left   = std::max(lhs.x, rhs.x);
  const float top    = std::max(lhs.y, rhs.y);
  const float right  = std::min(lhs.x + lhs.width, rhs.x + rhs.width);
  const float bottom = std::min(lhs.y + lhs.height, rhs.y + rhs.height);
  if(right <= left || bottom <= top)
  {
    return Dali::Bounds{};
  }
  return Dali::Bounds{left, top, right - left, bottom - top};
}

struct EntityDataCandidate
{
  Dali::Actor            actor;
  EntityData::Annotation annotation;
};

void CollectEntityDataCandidates(Dali::Actor actor, std::vector<EntityDataCandidate>& out)
{
  if(!actor)
  {
    return;
  }

  EntityData::Annotation annotation;
  if(ReadActorAnnotation(actor, annotation))
  {
    out.push_back(EntityDataCandidate{actor, std::move(annotation)});
  }

  const uint32_t childCount = actor.GetChildCount();
  for(uint32_t i = 0; i < childCount; ++i)
  {
    CollectEntityDataCandidates(actor.GetChildAt(i), out);
  }
}

} // unnamed namespace

EntityDataService::EntityDataService(FocusedActorProvider* provider)
: mProvider(provider)
{
}

void EntityDataService::SetFocusedActorProvider(FocusedActorProvider* provider)
{
  mProvider = provider;
}

FocusedActorProvider* EntityDataService::GetEffectiveProvider() const
{
  return mProvider ? mProvider : FocusedActorProvider::GetRegisteredProvider();
}

bool EntityDataService::FindById(const std::string& id, EntityData& outEntityData)
{
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: request id=%s\n", id.c_str());

  if(!Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: adaptor unavailable for id=%s\n", id.c_str());
    return false;
  }

  uint32_t actorId = 0;
  if(!ParseActorId(id, actorId))
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: invalid actor id=%s\n", id.c_str());
    return false;
  }

  Dali::WindowContainer windows = Dali::Adaptor::Get().GetWindows();
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: id=%u windows=%zu\n", actorId, windows.size());
  for(auto& window : windows)
  {
    if(!window)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: window is empty\n");
      continue;
    }

    Dali::Layer root = window.GetRootLayer();
    if(!root)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: window has no root layer\n");
      continue;
    }

    Dali::Actor found = root.FindChildById(actorId);
    if(found)
    {
      outEntityData = MakeEntityData(found, GetEffectiveProvider());
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: found id=%s type=%s entityId=%s\n",
                    outEntityData.id.c_str(), outEntityData.type.c_str(), outEntityData.annotation.entityId.c_str());
      return true;
    }

    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: id=%u not in window\n", actorId);
  }

  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindById: actor id=%u not found\n", actorId);
  return false;
}

bool EntityDataService::GetFocusedEntityData(EntityData& outEntityData)
{
  FocusedActorProvider* provider = GetEffectiveProvider();
  if(!provider)
  {
    return false;
  }

  Dali::Actor focused = provider->GetFocusedActor();
  if(!focused)
  {
    return false;
  }

  outEntityData = MakeEntityData(focused, provider);
  return true;
}

bool EntityDataService::GetEntityData(std::vector<EntityData>& outEntityDataList)
{
  outEntityDataList.clear();

  if(!Dali::Adaptor::IsAvailable())
  {
    // No windows to inspect: an empty result is still a success.
    return true;
  }

  std::unordered_set<std::string> emittedIds;

  Dali::WindowContainer windows = Dali::Adaptor::Get().GetWindows();
  for(auto& window : windows)
  {
    if(!window)
    {
      continue;
    }

    Dali::Layer root = window.GetRootLayer();
    if(!root)
    {
      continue;
    }

    // Clipping is evaluated in the window coordinate system, so the window's own
    // clip region is its size at the origin.
    const Dali::PositionSize positionSize = window.GetPositionSize();
    Dali::Bounds windowClipBounds;
    windowClipBounds.x      = 0.0f;
    windowClipBounds.y      = 0.0f;
    windowClipBounds.width  = static_cast<float>(positionSize.width);
    windowClipBounds.height = static_cast<float>(positionSize.height);

    std::vector<EntityDataCandidate> candidates;
    CollectEntityDataCandidates(root, candidates);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetEntityData: window candidates=%zu\n", candidates.size());

    FocusedActorProvider* provider = GetEffectiveProvider();
    for(auto& candidate : candidates)
    {
      Dali::Actor& actor     = candidate.actor;
      const bool   isVisible = IsActorHierarchyVisible(actor);
      if(!isVisible)
      {
        DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetEntityData: skip hidden actor id=%d\n",
                      actor.GetProperty<int32_t>(Dali::Actor::Property::ID));
        continue;
      }

      EntityData entityData = MakeEntityData(actor, provider, std::move(candidate.annotation));

      // Only genuinely annotated entities with a non-empty identity qualify.
      if(entityData.annotation.entityId.empty() || entityData.annotation.entityType.empty())
      {
        DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetEntityData: skip id=%s entityId=%s\n",
                      entityData.id.c_str(), entityData.annotation.entityId.c_str());
        continue;
      }

      Dali::Bounds clipBounds = windowClipBounds;
      for(Dali::Actor parent = actor.GetParent(); parent; parent = parent.GetParent())
      {
        // The window already defines the screen clipping region. A root Layer
        // reports its anchor/center as SCREEN_POSITION, which is not a
        // top-left bounds origin, so do not use it as an ancestor clip.
        if(!parent.GetParent() && parent.GetTypeName() == "Layer")
        {
          continue;
        }
        if(parent.GetProperty<int>(Dali::Actor::Property::CLIPPING_MODE) == static_cast<int>(Dali::ClippingMode::DISABLED))
        {
          continue;
        }
        clipBounds = IntersectBounds(clipBounds, GetActorWindowBounds(parent));
      }

      if(!AreBoundsFinite(entityData.screenBounds) || !AreBoundsFinite(entityData.windowBounds) ||
         clipBounds.width <= 0.0f || clipBounds.height <= 0.0f)
      {
        continue;
      }

      // Emit each Actor ID at most once.
      if(!emittedIds.insert(entityData.id).second)
      {
        continue;
      }

      const float ratio = ComputeVisibilityRatio(entityData.windowBounds, clipBounds);
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetEntityData: id=%s ratio=%f threshold=%f\n",
                    entityData.id.c_str(), ratio, VISIBILITY_THRESHOLD);
      if(ratio >= VISIBILITY_THRESHOLD)
      {
        outEntityDataList.push_back(std::move(entityData));
      }
    }
  }

  // Deterministic order: ScreenBounds.Y, then ScreenBounds.X, then Actor ID.
  // Screen coordinates are used so the order stays meaningful across windows.
  std::sort(outEntityDataList.begin(), outEntityDataList.end(), [](const EntityData& a, const EntityData& b) {
    const int yOrder = CompareCoordinate(a.screenBounds.y, b.screenBounds.y);
    if(yOrder != 0)
    {
      return yOrder < 0;
    }
    const int xOrder = CompareCoordinate(a.screenBounds.x, b.screenBounds.x);
    if(xOrder != 0)
    {
      return xOrder < 0;
    }
    return IdLess(a.id, b.id);
  });

  return true;
}

bool EntityDataService::ToPresentation(const EntityData& entityData, std::string& outPresentation)
{
  outPresentation.clear();
  if(!AreBoundsFinite(entityData.screenBounds) || !AreBoundsFinite(entityData.windowBounds))
  {
    return false;
  }

  std::ostringstream oss;
  oss << "{"
      << "\"id\":\"" << JsonEscape(entityData.id) << "\","
      << "\"type\":\"" << JsonEscape(entityData.type) << "\","
      << "\"description\":\"" << JsonEscape(entityData.description) << "\","
      << "\"screenBounds\":{"
      << "\"x\":" << entityData.screenBounds.x << ","
      << "\"y\":" << entityData.screenBounds.y << ","
      << "\"width\":" << entityData.screenBounds.width << ","
      << "\"height\":" << entityData.screenBounds.height << "},"
      << "\"windowBounds\":{"
      << "\"x\":" << entityData.windowBounds.x << ","
      << "\"y\":" << entityData.windowBounds.y << ","
      << "\"width\":" << entityData.windowBounds.width << ","
      << "\"height\":" << entityData.windowBounds.height << "},"
      << "\"isFocused\":" << (entityData.isFocused ? "true" : "false") << ","
      << "\"isEnabled\":" << (entityData.isEnabled ? "true" : "false") << ","
      << "\"annotation\":{"
      << "\"entityId\":\"" << JsonEscape(entityData.annotation.entityId) << "\","
      << "\"entityType\":\"" << JsonEscape(entityData.annotation.entityType) << "\","
      << "\"entityInfo\":\"" << JsonEscape(entityData.annotation.entityInfo) << "\"}"
      << "}";
  outPresentation = oss.str();
  return true;
}

float EntityDataService::ComputeVisibilityRatio(const Dali::Bounds& entityBounds, const Dali::Bounds& clipBounds) const
{
  if(!AreBoundsFinite(entityBounds) || !AreBoundsFinite(clipBounds))
  {
    return 0.0f;
  }

  const float entityArea = entityBounds.width * entityBounds.height;
  if(entityArea <= 0.0f)
  {
    return 0.0f;
  }

  const float left   = std::max(entityBounds.x, clipBounds.x);
  const float top    = std::max(entityBounds.y, clipBounds.y);
  const float right  = std::min(entityBounds.x + entityBounds.width, clipBounds.x + clipBounds.width);
  const float bottom = std::min(entityBounds.y + entityBounds.height, clipBounds.y + clipBounds.height);

  const float intersectWidth  = right - left;
  const float intersectHeight = bottom - top;
  if(intersectWidth <= 0.0f || intersectHeight <= 0.0f)
  {
    return 0.0f;
  }

  return (intersectWidth * intersectHeight) / entityArea;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
