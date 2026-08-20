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
#include <charconv>
#include <cmath>
#include <cstdint>
#include <locale>
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

  uint32_t   value  = 0u;
  const auto result = std::from_chars(id.data(), id.data() + id.size(), value, 10);
  if(result.ec != std::errc{} || result.ptr != id.data() + id.size())
  {
    return false;
  }

  outId = value;
  return true;
}

/**
 * @brief Total ordering of Actor-ID strings.
 *
 * Plain decimal IDs sort before malformed IDs, then by numeric value. Exact
 * string order breaks ties and orders malformed IDs.
 */
bool IdLess(const std::string& lhs, const std::string& rhs)
{
  uint32_t   leftActorId    = 0;
  uint32_t   rightActorId   = 0;
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

float ComputeVisibilityRatio(const Dali::Bounds& entityBounds, const Dali::Bounds& clipBounds)
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

void AppendAnnotatedEntities(Dali::Actor                      root,
                             const Dali::Bounds&              windowClipBounds,
                             FocusedActorProvider*            provider,
                             std::unordered_set<std::string>& emittedActorIds,
                             std::vector<EntityData>&         outEntityDataList)
{
  std::vector<EntityDataCandidate> candidates;
  CollectEntityDataCandidates(root, candidates);
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetAnnotatedEntities: candidates=%zu\n", candidates.size());

  for(auto& candidate : candidates)
  {
    Dali::Actor& actor     = candidate.actor;
    const bool   isVisible = IsActorHierarchyVisible(actor);
    if(!isVisible)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetAnnotatedEntities: skip hidden actor id=%u\n", actor.GetId());
      continue;
    }

    EntityData entityData = MakeEntityData(actor, provider, std::move(candidate.annotation));

    if(entityData.annotation.entityId.empty() || entityData.annotation.entityType.empty())
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetAnnotatedEntities: skip actorId=%s entityId=%s\n",
                    entityData.actorId.c_str(), entityData.annotation.entityId.c_str());
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

    if(!emittedActorIds.insert(entityData.actorId).second)
    {
      continue;
    }

    const float ratio = ComputeVisibilityRatio(entityData.windowBounds, clipBounds);
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData GetAnnotatedEntities: actorId=%s ratio=%f threshold=%f\n",
                  entityData.actorId.c_str(), ratio, EntityDataService::VISIBILITY_THRESHOLD);
    if(ratio >= EntityDataService::VISIBILITY_THRESHOLD)
    {
      outEntityDataList.push_back(std::move(entityData));
    }
  }
}

void SortEntityData(std::vector<EntityData>& entityDataList)
{
  // Deterministic order: ScreenBounds.Y, then ScreenBounds.X, then Actor ID.
  // Screen coordinates are used so the order stays meaningful across windows.
  std::sort(entityDataList.begin(), entityDataList.end(), [](const EntityData& a, const EntityData& b)
  {
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
    return IdLess(a.actorId, b.actorId);
  });
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
  return mProvider ? mProvider : Dali::Integration::GetFocusedActorProvider();
}

bool EntityDataService::FindByActorId(const std::string& actorId, EntityData& outEntityData)
{
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: request actorId=%s\n", actorId.c_str());

  if(!Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: adaptor unavailable for actorId=%s\n", actorId.c_str());
    return false;
  }

  Dali::WindowContainer windows = Dali::Adaptor::Get().GetWindows();
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: windows=%zu\n", windows.size());
  for(auto& window : windows)
  {
    if(!window)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: window is empty\n");
      continue;
    }

    Dali::Layer root = window.GetRootLayer();
    if(!root)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: window has no root layer\n");
      continue;
    }

    if(FindByActorId(actorId, root, outEntityData))
    {
      return true;
    }
  }

  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: actorId=%s not found\n", actorId.c_str());
  return false;
}

bool EntityDataService::FindByActorId(const std::string& actorId, Dali::Actor root, EntityData& outEntityData)
{
  uint32_t parsedActorId = 0u;
  if(!root || !ParseActorId(actorId, parsedActorId))
  {
    DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: invalid actorId=%s\n", actorId.c_str());
    return false;
  }

  Dali::Actor found = root.FindChildById(parsedActorId);
  if(!found && root.GetId() == parsedActorId)
  {
    found = root;
  }
  if(!found)
  {
    return false;
  }

  outEntityData = MakeEntityData(found, GetEffectiveProvider());
  DALI_LOG_INFO(gLogFilter, Debug::Verbose, "EntityData FindByActorId: found actorId=%s actorTypeName=%s entityId=%s\n",
                outEntityData.actorId.c_str(), outEntityData.actorTypeName.c_str(), outEntityData.annotation.entityId.c_str());
  return true;
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

bool EntityDataService::GetAnnotatedEntities(std::vector<EntityData>& outEntityDataList)
{
  outEntityDataList.clear();

  if(!Dali::Adaptor::IsAvailable())
  {
    // No windows to inspect: an empty result is still a success.
    return true;
  }

  std::unordered_set<std::string> emittedActorIds;

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
    Dali::Bounds             windowClipBounds;
    windowClipBounds.x      = 0.0f;
    windowClipBounds.y      = 0.0f;
    windowClipBounds.width  = static_cast<float>(positionSize.width);
    windowClipBounds.height = static_cast<float>(positionSize.height);

    AppendAnnotatedEntities(root, windowClipBounds, GetEffectiveProvider(), emittedActorIds, outEntityDataList);
  }

  SortEntityData(outEntityDataList);

  return true;
}

bool EntityDataService::GetAnnotatedEntities(Dali::Actor root, const Dali::Bounds& clipBounds, std::vector<EntityData>& outEntityDataList)
{
  outEntityDataList.clear();
  std::unordered_set<std::string> emittedActorIds;
  AppendAnnotatedEntities(root, clipBounds, GetEffectiveProvider(), emittedActorIds, outEntityDataList);
  SortEntityData(outEntityDataList);

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
  oss.imbue(std::locale::classic());
  oss << "{"
      << "\"id\":\"" << JsonEscape(entityData.actorId) << "\","
      << "\"type\":\"" << JsonEscape(entityData.actorTypeName) << "\","
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

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
