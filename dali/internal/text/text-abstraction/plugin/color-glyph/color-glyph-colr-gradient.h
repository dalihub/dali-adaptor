#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_GRADIENT_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_GRADIENT_H

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
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-paint-context.h>

#include <cstdint>
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
 * @brief Converts a COLRv1 ColorLine extend mode to ThorVG fill spread.
 *
 * @param[in] extend The FreeType COLRv1 extend mode.
 * @return The ThorVG fill spread mode.
 */
tvg::FillSpread ConvertColorLineExtend(FT_PaintExtend extend);

/**
 * @brief Builds ThorVG-safe color stops for a normalized [0, 1] range.
 *
 * COLRv1 ColorLine offsets can fall outside [0, 1]. ThorVG gradients require
 * safe stops in the unit range, so this helper prepares the stops without
 * changing unsupported paint types such as SweepGradient into another gradient.
 *
 * @param[in] rawStops The resolved COLRv1 color stops.
 * @param[out] safeStops The normalized ThorVG color stops.
 * @return @e true if safe stops were produced.
 */
bool BuildThorvgSafeStopsForUnitRange(
  const std::vector<tvg::Fill::ColorStop>& rawStops,
  std::vector<tvg::Fill::ColorStop>& safeStops);

/**
 * @brief Validates color stops before passing them to ThorVG.
 *
 * @param[in] stops The ThorVG color stops.
 * @return @e true if the stop list is non-empty and valid for ThorVG.
 */
bool ValidateStopsForThorvg(const std::vector<tvg::Fill::ColorStop>& stops);

/**
 * @brief Resolves FreeType COLRv1 ColorLine stops to ThorVG color stops.
 *
 * Stop colors are read from CPAL. Foreground palette index 0xFFFF fails closed
 * until foreground color plumbing is available.
 *
 * @param[in] ftFace The FreeType face handle.
 * @param[in] paletteIndex The CPAL palette index.
 * @param[in] colorline The FreeType COLRv1 ColorLine.
 * @param[out] stops The resolved ThorVG color stops.
 * @param[out] stopCount The number of resolved stops.
 * @param[out] outMinOffset The minimum COLRv1 stop offset.
 * @param[out] outMaxOffset The maximum COLRv1 stop offset.
 * @param[in] debugGlyph The root glyph index used for optional diagnostics.
 * @return @e true if the stops are resolved.
 */
bool BuildColorStops(
  FT_Face ftFace,
  uint16_t paletteIndex,
  FT_ColorLine& colorline,
  std::vector<tvg::Fill::ColorStop>& stops,
  uint32_t& stopCount,
  float& outMinOffset,
  float& outMaxOffset,
  uint32_t debugGlyph);

/**
 * @brief Builds a ThorVG matrix for a COLRv1 paint-space transform.
 *
 * @param[in] paintTransform The accumulated paint transform.
 * @return The ThorVG transform matrix.
 */
tvg::Matrix BuildPaintMatrix(const TransformState& paintTransform);

/**
 * @brief Applies a COLRv1 PaintLinearGradient fill to a ThorVG shape.
 *
 * @param[in] linear The FreeType COLRv1 linear gradient paint.
 * @param[in,out] shape The shape receiving the gradient fill.
 * @param[in] ctx The current COLRv1 paint context.
 * @return @e true if the fill is applied.
 */
bool HandlePaintLinearGradient(
  FT_PaintLinearGradient& linear,
  tvg::Shape* shape,
  PaintContext& ctx);

/**
 * @brief Applies a COLRv1 PaintRadialGradient fill to a ThorVG shape.
 *
 * @param[in] radial The FreeType COLRv1 radial gradient paint.
 * @param[in,out] shape The shape receiving the gradient fill.
 * @param[in] ctx The current COLRv1 paint context.
 * @return @e true if the fill is applied.
 */
bool HandlePaintRadialGradient(
  FT_PaintRadialGradient& radial,
  tvg::Shape* shape,
  PaintContext& ctx);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_GRADIENT_H
