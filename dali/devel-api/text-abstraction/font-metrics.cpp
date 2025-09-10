/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/font-metrics.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <cmath>

namespace Dali
{
namespace TextAbstraction
{
FontMetrics::FontMetrics()
: ascender{0.f},
  descender{0.f},
  height{0.f},
  underlinePosition{0.f},
  underlineThickness{0.f}
{
}

FontMetrics::FontMetrics(float ascenderPixels,
                         float descenderPixels,
                         float heightPixels,
                         float underlinePositionPixels,
                         float underlineThicknessPixels)
: ascender{ascenderPixels},
  descender{descenderPixels},
  height{heightPixels},
  underlinePosition{underlinePositionPixels},
  underlineThickness{underlineThicknessPixels}
{
  if(TextAbstraction::DesignCompatibilityEnabled())
  {
    ApplyDesignLineHeightCompat();
  }
}

void FontMetrics::ApplyDesignLineHeightCompat()
{
  // values are usually integer-aligned already, but ensure consistency by explicit rounding.
  int ascenderPixels  = static_cast<int>(std::lround(ascender));
  int descenderPixels = static_cast<int>(std::lround(descender));
  int heightPixels    = static_cast<int>(std::lround(height));

  // negative height should not expand line; clamp to 0
  if(heightPixels < 0)
  {
    heightPixels = 0;
  }

  int lineHeight = ascenderPixels - descenderPixels;

  // If lineHeight < height: keep as-is (no even enforcement) or
  // Gap too large -> skip adjustment
  if(lineHeight < heightPixels || lineHeight - heightPixels > 3)
  {
    return;
  }

  // Decide desired lineHeight:
  // - If lineHeight > height: clamp to even <= height
  // - If lineHeight == height: enforce even; if odd, reduce by 1 via descender
  int desiredLineHeight = lineHeight;
  if(lineHeight > heightPixels)
  {
    int targetEvenHeight = std::clamp(heightPixels & ~1, 0, lineHeight);
    desiredLineHeight    = targetEvenHeight;
  }
  else // lineHeight == heightPixels
  {
    if(lineHeight & 1)
    {
      desiredLineHeight = lineHeight - 1;
    }
    else
    {
      // Already equal and even -> done
      return;
    }
  }

  int shrinkAmount = std::max(0, lineHeight - desiredLineHeight);

  // Apply shrink without loop/branch:
  // pairs: (ascender - pairCount, descender + pairCount)
  // residual 1px (if any): descender + residual
  int pairCount = shrinkAmount / 2;
  int residual  = shrinkAmount % 2;

  ascenderPixels  -= pairCount;
  descenderPixels += pairCount + residual;

  ascender  = static_cast<float>(ascenderPixels);
  descender = static_cast<float>(descenderPixels);
  height    = static_cast<float>(heightPixels);
}

} // namespace TextAbstraction

} // namespace Dali
