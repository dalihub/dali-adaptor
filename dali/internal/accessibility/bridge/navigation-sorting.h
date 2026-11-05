#ifndef DALI_INTERNAL_ACCESSIBILITY_NAVIGATION_SORTING_H
#define DALI_INTERNAL_ACCESSIBILITY_NAVIGATION_SORTING_H

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

#include <dali/public-api/common/dali-namespace.h>
#include <dali/public-api/math/math-utils.h>
#include <dali/public-api/math/rect.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace DALI_NAMESPACE::Accessibility::Internal
{
/**
 * @brief Orders children using one navigation-bounds snapshot per entry.
 *
 * The getter resolves navigation bounds, including any parent fallback, before
 * sorting. Comparisons and line grouping must never query live scene geometry:
 * changing comparison results can make std::sort walk outside its input range.
 * The snapshot is local to this invocation; later navigation queries refresh it.
 *
 * Zero-area entries are omitted, matching the bridge's existing line splitting.
 * On non-finite coordinates or negative dimensions, returns false and leaves
 * children unchanged so that an invalid rectangle cannot discard a subtree.
 * Finite off-screen coordinates are allowed; this does not validate their source
 * or protect against an invalid object dereferenced by the bounds getter.
 */
template<typename Node, typename BoundsGetter>
bool SortChildrenFromTopLeft(std::vector<Node*>& children, BoundsGetter&& getBounds)
{
  struct ChildBounds
  {
    Node*        child;
    Dali::Bounds bounds;
  };

  std::vector<ChildBounds> snapshot;
  snapshot.reserve(children.size());
  for(auto* child : children)
  {
    if(!child)
    {
      continue;
    }

    const auto bounds = getBounds(child);
    if(!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
       !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
       bounds.width < 0.f || bounds.height < 0.f)
    {
      return false;
    }
    if(Dali::EqualsZero(bounds.width) || Dali::EqualsZero(bounds.height))
    {
      continue;
    }
    snapshot.push_back({child, bounds});
  }

  std::stable_sort(snapshot.begin(), snapshot.end(), [](const ChildBounds& lhs, const ChildBounds& rhs)
  {
    return lhs.bounds.y < rhs.bounds.y;
  });

  for(auto lineBegin = snapshot.begin(); lineBegin != snapshot.end();)
  {
    // Preserve the existing first-item center rule. Double arithmetic avoids
    // overflow in center calculations even for very large finite float bounds.
    const double lineCenter = lineBegin->bounds.y + 0.5 * lineBegin->bounds.height;
    auto         lineEnd    = lineBegin + 1;
    while(lineEnd != snapshot.end() && lineCenter >= lineEnd->bounds.y + 0.5 * lineEnd->bounds.height)
    {
      ++lineEnd;
    }
    std::stable_sort(lineBegin, lineEnd, [](const ChildBounds& lhs, const ChildBounds& rhs)
    {
      return lhs.bounds.x < rhs.bounds.x;
    });
    lineBegin = lineEnd;
  }

  // Publish only after every bounds query and sort has completed successfully.
  children.clear();
  for(const auto& entry : snapshot)
  {
    children.push_back(entry.child);
  }
  return true;
}
} // namespace DALI_NAMESPACE::Accessibility::Internal

#endif // DALI_INTERNAL_ACCESSIBILITY_NAVIGATION_SORTING_H
