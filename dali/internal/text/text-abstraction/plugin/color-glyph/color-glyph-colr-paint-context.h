#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_PAINT_CONTEXT_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_PAINT_CONTEXT_H

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
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-common.h>

#include <vector>

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
 * @brief Holds per-glyph state for COLRv1 Scene/offscreen evaluation.
 *
 * The context is passed through paint graph builders and raster helpers. It
 * carries canvas dimensions, palette selection, transform state, diagnostics,
 * and recursion guard state; it does not own ThorVG paint nodes.
 */
struct PaintContext
{
  FT_Face ftFace;          ///< FreeType face for the COLRv1 paint graph.
  uint16_t paletteIndex;   ///< CPAL palette index.
  uint32_t targetWidth;    ///< Target canvas width in pixels.
  uint32_t targetHeight;   ///< Target canvas height in pixels.
  float unitsPerEm;        ///< Font units per em from the face.
  float scale;             ///< Font-unit to pixel scale.
  float offsetX;           ///< X offset from font units to canvas pixels.
  float offsetY;           ///< Y offset from font units to canvas pixels.

  TransformState geometryTransform; ///< Accumulated transform applied to shape outline geometry.

  TransformState paintTransform; ///< Accumulated transform applied to fill/gradient coordinates.

  uint32_t unsupportedCount{0};              ///< Number of unsupported paint nodes encountered.
  uint32_t debugGlyph{0};                    ///< Root glyph index used for optional diagnostics.
  bool allowOffscreenCompositeResult{false}; ///< Whether non-root Composite may return a Picture result.
  ColorGlyphColrRasterizer* rasterizer{nullptr}; ///< Non-owning rasterizer used for ClipBox cache lookup.
  std::size_t variationsHash{0u};                ///< Variation state hash used by the ClipBox cache.
  bool useClipBoxBounds{false};                  ///< Whether this subtree may use ClipBox as its bounds source.
  GlyphIndex clipBoxBoundsGlyph{0u};             ///< Glyph whose ClipBox bounds the current root/subtree.

  std::vector<GlyphIndex> colrGlyphStack; ///< PaintColrGlyph recursion guard stack.
};

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_PAINT_CONTEXT_H
