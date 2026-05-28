#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_COMMON_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_COMMON_H

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
 * @brief FreeType root transform policy used for COLRv1 paint retrieval.
 *
 * The renderer applies its own canvas mapping and y-flip, so FreeType root
 * transform expansion is disabled when reading the COLRv1 paint graph.
 */
#define COLR_ROOT_TRANSFORM_OPTION FT_COLOR_NO_ROOT_TRANSFORM

/**
 * @brief Accumulates a COLRv1 2x3 affine transform.
 *
 * Matrix values use COLRv1 font coordinate space before the renderer applies
 * global scale and canvas placement.
 */
struct TransformState
{
  float matrix[6] = {1, 0, 0, 1, 0, 0}; ///< 2x3 affine matrix [xx, xy, yx, yy, dx, dy].
  bool hasTransform{false};             ///< Whether any non-identity transform was applied.

  /**
   * @brief Resets the transform to identity.
   */
  void Reset()
  {
    matrix[0] = 1.0f; matrix[1] = 0.0f;
    matrix[2] = 0.0f; matrix[3] = 1.0f;
    matrix[4] = 0.0f; matrix[5] = 0.0f;
    hasTransform = false;
  }

  /**
   * @brief Applies a 2x3 affine transform on top of the current state.
   *
   * @param[in] xx The xx matrix component.
   * @param[in] xy The xy matrix component.
   * @param[in] yx The yx matrix component.
   * @param[in] yy The yy matrix component.
   * @param[in] dx The x translation component.
   * @param[in] dy The y translation component.
   */
  void Apply(float xx, float xy, float yx, float yy, float dx, float dy)
  {
    float nxx = xx * matrix[0] + xy * matrix[2];
    float nxy = xx * matrix[1] + xy * matrix[3];
    float nyx = yx * matrix[0] + yy * matrix[2];
    float nyy = yx * matrix[1] + yy * matrix[3];
    float ndx = dx * matrix[0] + dy * matrix[2] + matrix[4];
    float ndy = dx * matrix[1] + dy * matrix[3] + matrix[5];
    matrix[0] = nxx; matrix[1] = nxy;
    matrix[2] = nyx; matrix[3] = nyy;
    matrix[4] = ndx; matrix[5] = ndy;
    hasTransform = true;
  }
};

/**
 * @brief Checks whether detailed COLRv1 trace logging is enabled for a glyph.
 *
 * The DALI_COLR_DEBUG_GLYPH environment variable is parsed once and compared
 * with the supplied glyph index.
 *
 * @param[in] glyphIndex The glyph index.
 * @return @e true if trace logging is enabled for the glyph.
 */
bool IsColrDebugTraceEnabled(uint32_t glyphIndex);

/**
 * @brief Returns a readable name for a FreeType COLRv1 paint format.
 *
 * @param[in] format The FreeType paint format.
 * @return A static string for diagnostics.
 */
const char* PaintFormatToString(FT_PaintFormat format);

/**
 * @brief Returns a readable name for a FreeType COLRv1 composite mode.
 *
 * @param[in] mode The FreeType composite mode.
 * @return A static string for diagnostics.
 */
const char* CompositeModeToString(FT_Composite_Mode mode);

/**
 * @brief Builds a ThorVG matrix from COLRv1 geometry transform and canvas mapping.
 *
 * @param[in] geometryTransform The accumulated COLRv1 geometry transform.
 * @param[in] globalScale The font-unit to pixel scale.
 * @param[in] offsetX The x canvas offset in pixels.
 * @param[in] offsetY The y canvas offset in pixels.
 * @return The ThorVG transform matrix.
 */
tvg::Matrix BuildGeometryMatrix(
  const TransformState& geometryTransform,
  float globalScale,
  float offsetX,
  float offsetY);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_COMMON_H
