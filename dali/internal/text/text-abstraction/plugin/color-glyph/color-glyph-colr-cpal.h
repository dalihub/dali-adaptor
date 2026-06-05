#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_CPAL_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_CPAL_H

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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-paint-context.h>

#include <cstdint>

#if DALI_ENABLE_COLR_V1_RENDERER
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_COLOR_H
#include <thorvg.h>
#endif

namespace Dali::TextAbstraction::Internal
{

#if DALI_ENABLE_COLR_V1_RENDERER

/**
 * @brief Resolves a CPAL palette entry to RGBA components.
 *
 * The foreground palette index 0xFFFF is not replaced with a fake color; it
 * fails closed until the text foreground color is plumbed into COLRv1 rendering.
 *
 * @param[in] ftFace The FreeType face handle.
 * @param[in] paletteIndex The CPAL palette index.
 * @param[in] colorIndex The CPAL color record index.
 * @param[out] outR The resolved red component.
 * @param[out] outG The resolved green component.
 * @param[out] outB The resolved blue component.
 * @param[out] outA The resolved alpha component.
 * @return @e true if the color is resolved.
 */
bool ResolveCpalColor(
  FT_Face ftFace,
  uint16_t paletteIndex,
  uint16_t colorIndex,
  uint8_t& outR,
  uint8_t& outG,
  uint8_t& outB,
  uint8_t& outA);

/**
 * @brief Applies a COLRv1 PaintSolid fill to a ThorVG shape.
 *
 * @param[in] solid The FreeType COLRv1 solid paint.
 * @param[in,out] shape The shape receiving the fill color.
 * @param[in] ctx The current COLRv1 paint context.
 * @return @e true if the fill is applied.
 */
bool HandlePaintSolid(
  FT_PaintSolid& solid,
  tvg::Shape* shape,
  PaintContext& ctx);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_CPAL_H
