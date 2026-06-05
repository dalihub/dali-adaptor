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
 */

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-cpal.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-cpal-parser.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-common.h>
#include <dali/integration-api/debug.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <algorithm>
#include <cmath>

namespace Dali::TextAbstraction::Internal
{

namespace
{
// FreeType fixed-point conversion constant
// FT_F2Dot14 2.14: value / 16384.0f (alpha, angle)
constexpr float FROM_F2DOT14 = 1.0f / 16384.0f;  // FT_F2Dot14 2.14
} // anonymous namespace

// ---- Resolve CPAL color for a palette index ----
bool ResolveCpalColor(FT_Face ftFace, uint16_t paletteIndex, uint16_t colorIndex, uint8_t& outR, uint8_t& outG, uint8_t& outB, uint8_t& outA)
{
  // palette_index 0xFFFF indicates the current text foreground color.
  // It must not be treated as an actual CPAL palette entry.
  if(colorIndex == 0xFFFF)
  {
    outR = 0; outG = 0; outB = 0; outA = 0;
    return false;
  }

  CpalColor color;
  ColorGlyphCpalParser::ResolveColor(ftFace, paletteIndex, colorIndex, color);
  if(color.valid)
  {
    outR = color.r;
    outG = color.g;
    outB = color.b;
    outA = color.a;
    return true;
  }

  // Keep deterministic output values for callers that inspect them, but signal
  // failure so the paint path does not synthesize a color.
  outR = 0; outG = 0; outB = 0; outA = 255;
  return false;
}

// ---- Handle PaintSolid ----
bool HandlePaintSolid(FT_PaintSolid& solid, tvg::Shape* shape, PaintContext& ctx)
{
  uint8_t r, g, b, a;
  if(!ResolveCpalColor(ctx.ftFace, ctx.paletteIndex, solid.color.palette_index, r, g, b, a))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER PaintSolid unsupported paletteIndex:%u alpha:%.1f\n",
                          solid.color.palette_index, solid.color.alpha);
    return false;
  }

  // Apply alpha from FT_PaintColor (FT_F2Dot14, clamped to [0,1] for safety)
  float alphaF = std::max(0.0f, std::min(1.0f, static_cast<float>(solid.color.alpha) * FROM_F2DOT14));
  a = static_cast<uint8_t>(a * alphaF);

  // ThorVG 1.0: fill(r, g, b, a) - same API as 0.8 for solid color
  shape->fill(r, g, b, a);

  if(IsColrDebugTraceEnabled(ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER PaintSolid paletteIndex:%u alpha:%.3f rgba:(%u,%u,%u,%u)\n",
                   solid.color.palette_index, alphaF, r, g, b, a);
  }

  return true;
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
