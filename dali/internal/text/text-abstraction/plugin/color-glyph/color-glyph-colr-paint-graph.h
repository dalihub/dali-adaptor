#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_PAINT_GRAPH_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_PAINT_GRAPH_H

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
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-composite.h>
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
 * @brief Builds a root COLRv1 paint node for direct root rasterization.
 *
 * On success, @p outPaint receives a caller-owned ThorVG paint node. The caller
 * must either add it to a canvas/scene or release it through the existing ThorVG
 * ownership path. Unsupported paint graph nodes fail closed.
 *
 * @param[in] ftFace The FreeType face handle.
 * @param[in] opaquePaint The FreeType root opaque paint.
 * @param[in] ctx The current COLRv1 paint context.
 * @param[out] outPaint The built ThorVG paint on success.
 * @return @e true if a root paint was built.
 */
bool TryBuildRootPaintForSceneBuilder(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  PaintContext& ctx,
  tvg::Paint*& outPaint);

/**
 * @brief Root PaintComposite output produced by the shared offscreen composite core.
 *
 * The pixel buffer is caller-owned and must be moved into the root RenderResult
 * or released with FreeCompositeBuffer(). Non-root Composite uses the internal
 * OffscreenBuffer -> Picture path instead of this root-only contract.
 */
struct RootCompositeOffscreenResult
{
  CompositeBuffer surface{}; ///< Composited BGRA8888 surface.
  int32_t pixelX{0};         ///< Left origin of the surface in root pixel coordinates.
  int32_t pixelY{0};         ///< Top origin of the surface in root pixel coordinates.

  /**
   * @brief Checks whether the root Composite result owns a valid surface.
   *
   * @return @e true if the surface can be moved to RenderResult.
   */
  bool IsValid() const
  {
    return surface.valid && surface.buffer != nullptr;
  }
};

/**
 * @brief Builds a root PaintComposite result with the shared offscreen core.
 *
 * The source and backdrop paint subtrees are rasterized into bounded offscreen
 * BGRA8888 buffers, composited, and returned as a root-only buffer result.
 *
 * @param[in] ftFace The FreeType face handle.
 * @param[in] composite The FreeType COLRv1 PaintComposite.
 * @param[in] ctx The current COLRv1 paint context.
 * @param[in] depth The current paint graph recursion depth.
 * @return A valid root Composite result on success.
 */
RootCompositeOffscreenResult BuildCompositeOffscreenRootBuffer(
  FT_Face ftFace,
  const FT_PaintComposite& composite,
  const PaintContext& ctx,
  uint32_t depth);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_PAINT_GRAPH_H
