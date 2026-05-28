#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_BOUNDS_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_BOUNDS_H

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

#if DALI_ENABLE_COLR_V1_RENDERER
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_COLOR_H
#include FT_BBOX_H
#endif

namespace Dali::TextAbstraction::Internal
{

#if DALI_ENABLE_COLR_V1_RENDERER

using ColrPaintBounds = ColorGlyphColrRasterizer::PaintBounds;

/**
 * @brief Expands COLRv1 paint bounds with a rectangle.
 *
 * @param[in,out] bounds The accumulated bounds in font units.
 * @param[in] x1 The rectangle minimum x in font units.
 * @param[in] y1 The rectangle minimum y in font units.
 * @param[in] x2 The rectangle maximum x in font units.
 * @param[in] y2 The rectangle maximum y in font units.
 */
void PaintBoundsUnion(ColrPaintBounds& bounds, float x1, float y1, float x2, float y2);

/**
 * @brief Expands COLRv1 paint bounds with a transformed rectangle.
 *
 * @param[in,out] bounds The accumulated bounds in font units.
 * @param[in] bMinX The rectangle minimum x before transform.
 * @param[in] bMinY The rectangle minimum y before transform.
 * @param[in] bMaxX The rectangle maximum x before transform.
 * @param[in] bMaxY The rectangle maximum y before transform.
 * @param[in] transform The accumulated COLRv1 geometry transform.
 */
void PaintBoundsUnionBBox(
  ColrPaintBounds& bounds,
  float bMinX,
  float bMinY,
  float bMaxX,
  float bMaxY,
  const TransformState& transform);

/**
 * @brief Computes the outline bounds of a glyph in font units.
 *
 * @param[in] ftFace The FreeType face handle.
 * @param[in] glyphIndex The glyph index.
 * @return The glyph outline bounds, or invalid bounds on failure.
 */
ColrPaintBounds ComputeGlyphOutlineBounds(FT_Face ftFace, uint32_t glyphIndex);

/**
 * @brief Traverses a COLRv1 paint graph and accumulates conservative bounds.
 *
 * The bounds are in face font units. COLRv1 geometry transforms are applied,
 * but pixel scaling and canvas placement are not.
 *
 * @param[in] ftFace The FreeType face handle.
 * @param[in] opaquePaint The FreeType opaque paint node to traverse.
 * @param[in,out] bounds The accumulated paint bounds.
 * @param[in,out] geometryTransform The accumulated COLRv1 geometry transform.
 * @param[in] debugGlyph The root glyph index used for optional diagnostics.
 * @param[in] depth The current paint graph recursion depth.
 */
void TraversePaintBounds(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  ColrPaintBounds& bounds,
  TransformState& geometryTransform,
  uint32_t debugGlyph,
  uint32_t depth);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_BOUNDS_H
