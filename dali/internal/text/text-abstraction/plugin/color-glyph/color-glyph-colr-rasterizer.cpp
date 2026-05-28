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

// INTERNAL HEADERS
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-common.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-composite.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-gradient.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-bounds.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-outline.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-paint-graph.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-cpal.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-paint-context.h>
#include <dali/integration-api/debug.h>

// =====================================================================
// When DALI_ENABLE_COLR_V1_RENDERER == 0: stubs only
// No ThorVG or FreeType COLRv1 headers are included.
// =====================================================================
#if !DALI_ENABLE_COLR_V1_RENDERER

namespace Dali::TextAbstraction::Internal
{

bool ColorGlyphColrRasterizer::TryGetClipBoxPaintBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  std::size_t variationsHash,
  PaintBounds& outBounds)
{
  outBounds = PaintBounds();
  return false;
}

bool ColorGlyphColrRasterizer::ComputePaintGraphBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  PaintBounds& outBounds)
{
  outBounds = PaintBounds();
  return false;
}

ColorGlyphColrRasterizer::RawClipBox ColorGlyphColrRasterizer::ComputeClipBox(
  FT_Face ftFace,
  GlyphIndex glyphIndex) const
{
  RawClipBox result;
  result.status = ClipBoxCacheStatus::INVALID_INPUT;
  return result;
}

ColorGlyphColrRasterizer::RenderResult ColorGlyphColrRasterizer::RasterizeInternal(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  uint32_t targetWidth,
  uint32_t targetHeight,
  uint16_t paletteIndex,
  std::size_t variationsHash,
  const PaintBounds* paintBoundsHint)
{
  RenderResult result;
  // COLRv1 renderer not available - return failure, caller falls back
  return result;
}

} // namespace Dali::TextAbstraction::Internal

// =====================================================================
// When DALI_ENABLE_COLR_V1_RENDERER == 1: Full implementation
// ThorVG 1.0 API + FreeType COLRv1 API available
// =====================================================================
#else // DALI_ENABLE_COLR_V1_RENDERER

// COLRv1 common types and debug helpers are defined in color-glyph-colr-common.h/cpp.

// =====================================================================
// COLRv1 Implementation TODO (priority order)
// =====================================================================
// 1. PaintSweepGradient - check if current ThorVG API supports sweep/conic gradient
// 2. Additional Composite modes: Multiply, Screen, Overlay, Darken, Lighten, Plus,
//    then SrcOut/DstOut/SrcAtop/DstAtop/Xor. Never fallback to SrcOver.
// 3. Foreground color (palette_index 0xFFFF) - pipe actual text color
// 4. PaintGlyph child Composite clip boundary
// 5. Atlas/render alpha convention investigation (edge artifacts)
//
// Debug: DALI_COLR_DEBUG_GLYPH=<glyphIndex>
// Other env vars:
//   DALI_COLR_TRACE_SHAPED_GLYPHS=1 (font-client-plugin-impl.cpp)
//   DALI_COLR_TRACE_CODEPOINT=<codepoint> (font-face-cache-item.cpp)
//
// =====================================================================
// Coordinate System Notes (canonical reference)
// =====================================================================
//
// FreeType COLRv1 coordinates: font-unit, y-up
// FtOutlineToTvgShape() negates y: font y-up → canvas y-down (y-flipped)
// All gradient coordinates are also y-negated for consistency
// Pixel mapping: pixelX = fontX * scale + offsetX
//                pixelY = -fontY * scale + offsetY
//
// BGRA8888 buffer layout: B=[0], G=[1], R=[2], A=[3]
//
// ThorVG 1.0 ownership: gen() returns raw pointer
//   canvas->add(paint) transfers ownership to canvas
//   Paints not added to canvas must be released via tvg::Paint::rel()
//
// FT_Fixed 16.16: value / 65536.0f  (stop_offset, affine coefficients, coordinates)
// FT_F2Dot14 2.14: value / 16384.0f (alpha, angle)
//
// =====================================================================

// FreeType headers
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_COLOR_H
#include FT_BBOX_H

// ThorVG headers (ThorVG 1.0 API - guaranteed by DALI_HAS_THORVG_COLR_RENDER_API)
#include <thorvg.h>

#include <cstdlib>
#include <cmath>
#include <vector>

namespace Dali::TextAbstraction::Internal
{

namespace
{

// ---- Utility: count non-zero pixels in a BGRA buffer ----
uint32_t CountNonZeroPixels(const uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride)
{
  uint32_t count = 0;
  for(uint32_t y = 0; y < height; ++y)
  {
    const uint8_t* row = buffer + y * stride;
    for(uint32_t x = 0; x < width; ++x)
    {
      // Check alpha channel (byte 3 in BGRA)
      if(row[x * 4 + 3] != 0)
      {
        ++count;
      }
    }
  }
  return count;
}

// ---- Compute bounding box of a glyph outline ----
struct OutlineBBox
{
  float minX{0}, minY{0}, maxX{0}, maxY{0};
  bool valid{false};
};

OutlineBBox ComputeOutlineBBox(FT_Face ftFace, uint32_t glyphIndex)
{
  OutlineBBox bbox;
  FT_Error error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE);
  if(error != FT_Err_Ok) return bbox;

  FT_Outline* outline = &ftFace->glyph->outline;
  if(outline->n_points <= 0) return bbox;

  FT_Pos minX = outline->points[0].x;
  FT_Pos minY = outline->points[0].y;
  FT_Pos maxX = outline->points[0].x;
  FT_Pos maxY = outline->points[0].y;

  for(int i = 1; i < outline->n_points; ++i)
  {
    if(outline->points[i].x < minX) minX = outline->points[i].x;
    if(outline->points[i].y < minY) minY = outline->points[i].y;
    if(outline->points[i].x > maxX) maxX = outline->points[i].x;
    if(outline->points[i].y > maxY) maxY = outline->points[i].y;
  }

  bbox.minX = static_cast<float>(minX);
  bbox.minY = static_cast<float>(minY);
  bbox.maxX = static_cast<float>(maxX);
  bbox.maxY = static_cast<float>(maxY);
  bbox.valid = true;
  return bbox;
}

// COLRv1 helpers are implemented in the corresponding color-glyph-colr-* modules:
//   common:    TransformState, BuildGeometryMatrix, IsColrDebugTraceEnabled, PaintFormatToString, CompositeModeToString
//   cpal:      HandlePaintSolid, ResolveCpalColor
//   gradient:  HandlePaintLinearGradient, HandlePaintRadialGradient, BuildPaintMatrix, MultiplyMatrices
//   composite: CompositeBuffer, FreeCompositeBuffer, AllocateCompositeBuffer, IsCompositeModeSupported, CompositeBuffers
//   bounds:    PaintBoundsUnion, TraversePaintBounds, ComputeGlyphOutlineBounds
//   outline:   FtOutlineToTvgShape
//   paint-graph: TryBuildRootPaintForSceneBuilder, BuildCompositeOffscreenRootBuffer

// ---- Paint bounds type alias ----
using PaintBounds = ColorGlyphColrRasterizer::PaintBounds;
using RenderResult = ColorGlyphColrRasterizer::RenderResult;

// Root PaintComposite already owns a composited BGRA buffer. Move that buffer
// into the root RenderResult and preserve the offscreen pixel origin as the
// glyph bitmap offset metadata.
//
// Current root output offset contract:
//   non-composite direct root raster: horizontal=-offsetX, vertical=offsetY
//   root composite offscreen raster: horizontal=-offsetX+pixelX, vertical=offsetY-pixelY
// pixelX/Y is the signed guarded offscreen origin from the same source/backdrop
// union bounds used by non-root Composite placement.
bool MoveRootOffscreenToRenderResult(
  RootCompositeOffscreenResult& offscreen,
  RenderResult& result,
  const PaintBounds& paintBounds,
  float offsetX,
  float offsetY)
{
  if(!offscreen.IsValid())
  {
    return false;
  }

  result.success = true;
  result.buffer = offscreen.surface.buffer;
  result.width = offscreen.surface.width;
  result.height = offscreen.surface.height;
  result.stride = offscreen.surface.stride;
  result.format = Pixel::BGRA8888;

  if(paintBounds.valid)
  {
    result.hasPaintBounds = true;
    result.paintMinX = paintBounds.minX;
    result.paintMinY = paintBounds.minY;
    result.paintMaxX = paintBounds.maxX;
    result.paintMaxY = paintBounds.maxY;
  }
  else
  {
    result.hasPaintBounds = false;
  }

  result.horizontalOffset = static_cast<int32_t>(-offsetX + static_cast<float>(offscreen.pixelX));
  result.verticalOffset = static_cast<int32_t>(offsetY - static_cast<float>(offscreen.pixelY));

  offscreen.surface = CompositeBuffer();
  return true;
}

void ReleaseRootPaint(tvg::Paint*& paint)
{
  if(paint)
  {
    tvg::Paint::rel(paint);
    paint = nullptr;
  }
}

float Clip26Dot6ToFloat(FT_Pos value)
{
  return static_cast<float>(value) / 64.0f;
}

const char* PaintBoundsSourceToString(ColorGlyphColrRasterizer::PaintBounds::Source source)
{
  switch(source)
  {
    case ColorGlyphColrRasterizer::PaintBounds::Source::CLIP_BOX:
    {
      return "CLIP_BOX";
    }
    case ColorGlyphColrRasterizer::PaintBounds::Source::PAINT_GRAPH:
    {
      return "PAINT_GRAPH";
    }
    case ColorGlyphColrRasterizer::PaintBounds::Source::NONE:
    {
      return "NONE";
    }
  }

  return "NONE";
}

void ApplyPaintBoundsToRenderResult(RenderResult& result, const PaintBounds& paintBounds)
{
  if(paintBounds.valid)
  {
    result.hasPaintBounds = true;
    result.paintMinX = paintBounds.minX;
    result.paintMinY = paintBounds.minY;
    result.paintMaxX = paintBounds.maxX;
    result.paintMaxY = paintBounds.maxY;
  }
  else
  {
    result.hasPaintBounds = false;
  }
}

// Root non-Composite paint is a caller-owned ThorVG paint. Transfer it to a
// root SwCanvas target and return a BGRA RenderResult using the existing glyph
// bitmap metadata contract.
bool RenderRootPaintToRenderResult(
  tvg::Paint*& rootPaint,
  RenderResult& result,
  const PaintBounds& paintBounds,
  uint32_t bitmapWidth,
  uint32_t bitmapHeight,
  float offsetX,
  float offsetY,
  uint32_t glyphIndex)
{
  if(!rootPaint || bitmapWidth == 0u || bitmapHeight == 0u)
  {
    ReleaseRootPaint(rootPaint);
    return false;
  }

  if(bitmapWidth > UINT32_MAX / 4u)
  {
    ReleaseRootPaint(rootPaint);
    return false;
  }

  const uint32_t stride = bitmapWidth * 4u;
  const size_t byteCount = static_cast<size_t>(stride) * static_cast<size_t>(bitmapHeight);
  if(bitmapHeight != 0u && byteCount / bitmapHeight != stride)
  {
    ReleaseRootPaint(rootPaint);
    return false;
  }

  uint8_t* buffer = static_cast<uint8_t*>(calloc(byteCount, 1));
  if(!buffer)
  {
    ReleaseRootPaint(rootPaint);
    return false;
  }

  tvg::SwCanvas* canvas = tvg::SwCanvas::gen();
  if(!canvas)
  {
    free(buffer);
    ReleaseRootPaint(rootPaint);
    return false;
  }

  if(canvas->target(reinterpret_cast<uint32_t*>(buffer), bitmapWidth, bitmapWidth, bitmapHeight,
                    tvg::ColorSpace::ARGB8888) != tvg::Result::Success)
  {
    free(buffer);
    delete canvas;
    ReleaseRootPaint(rootPaint);
    return false;
  }

  if(canvas->add(rootPaint) != tvg::Result::Success)
  {
    free(buffer);
    delete canvas;
    ReleaseRootPaint(rootPaint);
    return false;
  }
  rootPaint = nullptr;

  auto drawResult = canvas->draw(true);
  if(drawResult != tvg::Result::Success)
  {
    free(buffer);
    delete canvas;
    return false;
  }

  auto syncResult = canvas->sync();
  if(syncResult != tvg::Result::Success)
  {
    free(buffer);
    delete canvas;
    return false;
  }

  // ThorVG ARGB8888 targets are premultiplied. DALi's color glyph typesetter
  // path premultiplies BGRA glyph pixels while copying them into the text
  // buffer, so expose straight BGRA here to avoid dark double-premultiplied
  // edges.
  UnpremultiplyBgraBuffer(buffer, bitmapWidth, bitmapHeight, stride);

  const uint32_t nonZero = CountNonZeroPixels(buffer, bitmapWidth, bitmapHeight, stride);
  delete canvas;

  result.success = true;
  result.buffer = buffer;
  result.width = bitmapWidth;
  result.height = bitmapHeight;
  result.stride = stride;
  result.format = Pixel::BGRA8888;
  ApplyPaintBoundsToRenderResult(result, paintBounds);
  result.horizontalOffset = static_cast<int32_t>(-offsetX);
  result.verticalOffset = static_cast<int32_t>(offsetY);

  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER DirectRootRaster success glyph:%u bitmap:%ux%u nonZero:%u offset:(%d,%d)\n",
                   glyphIndex, bitmapWidth, bitmapHeight, nonZero,
                   result.horizontalOffset, result.verticalOffset);
  }

  return true;
}

} // anonymous namespace

// ============================================================
// ColorGlyphColrRasterizer::TryGetClipBoxPaintBounds
// ============================================================
bool ColorGlyphColrRasterizer::TryGetClipBoxPaintBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  std::size_t variationsHash,
  PaintBounds& outBounds)
{
  outBounds = PaintBounds();

  if(!ftFace || !ftFace->size)
  {
    return false;
  }

  RawClipBox clipBox;
  GetClipBox(ftFace, glyphIndex, variationsHash, clipBox);
  if(!clipBox.IsValid())
  {
    return false;
  }

  const float unitsPerEm = static_cast<float>(ftFace->units_per_EM);
  const float scale = unitsPerEm > 0.0f ? static_cast<float>(ftFace->size->metrics.y_ppem) / unitsPerEm : 0.0f;
  if(scale <= 0.0f)
  {
    return false;
  }

  outBounds.valid = true;
  outBounds.minX = Clip26Dot6ToFloat(clipBox.minX) / scale;
  outBounds.minY = Clip26Dot6ToFloat(clipBox.minY) / scale;
  outBounds.maxX = Clip26Dot6ToFloat(clipBox.maxX) / scale;
  outBounds.maxY = Clip26Dot6ToFloat(clipBox.maxY) / scale;
  outBounds.source = PaintBounds::Source::CLIP_BOX;
  return true;
}

// ============================================================
// ColorGlyphColrRasterizer::ComputePaintGraphBounds
// ============================================================
bool ColorGlyphColrRasterizer::ComputePaintGraphBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  PaintBounds& outBounds)
{
  outBounds = PaintBounds();

  if(!ftFace)
  {
    return false;
  }

  FT_OpaquePaint rootPaint;
  rootPaint.p = nullptr;
  rootPaint.insert_root_transform = false;

  const FT_Bool hasPaint = FT_Get_Color_Glyph_Paint(
    ftFace,
    glyphIndex,
    COLR_ROOT_TRANSFORM_OPTION,
    &rootPaint);

  if(!hasPaint || rootPaint.p == nullptr)
  {
    return false;
  }

  TransformState identityTransform;
  PaintBounds bounds;
  TraversePaintBounds(ftFace, rootPaint, bounds, identityTransform, glyphIndex, 0);

  if(!bounds.valid)
  {
    return false;
  }

  outBounds = bounds;
  outBounds.source = PaintBounds::Source::PAINT_GRAPH;
  return true;
}

ColorGlyphColrRasterizer::RawClipBox ColorGlyphColrRasterizer::ComputeClipBox(
  FT_Face ftFace,
  GlyphIndex glyphIndex) const
{
  RawClipBox result;

  if(!ftFace)
  {
    result.status = ClipBoxCacheStatus::INVALID_INPUT;
    return result;
  }

  FT_ClipBox clipBox{};
  if(!FT_Get_Color_Glyph_ClipBox(ftFace, glyphIndex, &clipBox))
  {
    result.status = ClipBoxCacheStatus::MISSING;
    return result;
  }

  const FT_Vector corners[4] = {
    clipBox.bottom_left,
    clipBox.top_left,
    clipBox.top_right,
    clipBox.bottom_right};

  FT_Pos minX = corners[0].x;
  FT_Pos maxX = corners[0].x;
  FT_Pos minY = corners[0].y;
  FT_Pos maxY = corners[0].y;
  for(const FT_Vector& corner : corners)
  {
    if(corner.x < minX) minX = corner.x;
    if(corner.x > maxX) maxX = corner.x;
    if(corner.y < minY) minY = corner.y;
    if(corner.y > maxY) maxY = corner.y;
  }

  const float clipWidth = Clip26Dot6ToFloat(maxX - minX);
  const float clipHeight = Clip26Dot6ToFloat(maxY - minY);
  constexpr float MAX_COLR_CLIPBOX_PIXELS = 1019.0f;
  if(!std::isfinite(clipWidth) || !std::isfinite(clipHeight) ||
     clipWidth <= 0.0f || clipHeight <= 0.0f ||
     clipWidth > MAX_COLR_CLIPBOX_PIXELS || clipHeight > MAX_COLR_CLIPBOX_PIXELS)
  {
    result.status = ClipBoxCacheStatus::INVALID_BOUNDS;
    result.minX = minX;
    result.minY = minY;
    result.maxX = maxX;
    result.maxY = maxY;
    return result;
  }

  result.status = ClipBoxCacheStatus::OK;
  result.minX = minX;
  result.minY = minY;
  result.maxX = maxX;
  result.maxY = maxY;
  return result;
}

// ============================================================
// ColorGlyphColrRasterizer::RasterizeInternal
// ============================================================
ColorGlyphColrRasterizer::RenderResult ColorGlyphColrRasterizer::RasterizeInternal(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  uint32_t targetWidth,
  uint32_t targetHeight,
  uint16_t paletteIndex,
  std::size_t variationsHash,
  const PaintBounds* paintBoundsHint)
{
  RenderResult result;

  if(!ftFace || targetWidth == 0 || targetHeight == 0)
  {
    DALI_LOG_ERROR("COLOR_GLYPH_COLR_RENDER Fail glyph:%u reason:invalid-args\n", glyphIndex);
    return result;
  }

  // ThorVG is initialized by ColorGlyphColrRasterizer constructor.
  // Rasterization can proceed only when that initialization succeeded.
  if(!mThorvgInitialized)
  {
    DALI_LOG_ERROR("COLOR_GLYPH_COLR_RENDER Fail glyph:%u reason:thorvg-init-failed\n", glyphIndex);
    return result;
  }

  // Get root paint for this glyph - MUST initialize before calling FT_Get_Color_Glyph_Paint
  FT_OpaquePaint rootPaint;
  rootPaint.p = nullptr;
  rootPaint.insert_root_transform = false;

  // Use COLR_ROOT_TRANSFORM_OPTION for root transform control
  // See the define at the top of this file for explanation
  FT_Bool hasPaint = FT_Get_Color_Glyph_Paint(ftFace, glyphIndex,
                                                COLR_ROOT_TRANSFORM_OPTION,
                                                &rootPaint);
  if(!hasPaint || rootPaint.p == nullptr)
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER Fail glyph:%u reason:no-root-paint hasPaint:%d rootPaint.p:%p\n",
                   glyphIndex, static_cast<int>(hasPaint), static_cast<void*>(rootPaint.p));
    return result;
  }

  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RootPaint OK glyph:%u hasPaint:%d rootPaint.p:%p insertRootTransform:%d\n",
                   glyphIndex, static_cast<int>(hasPaint), static_cast<void*>(rootPaint.p),
                   static_cast<int>(rootPaint.insert_root_transform));
  }

  // Compute bounding box of the base glyph outline for scaling
  OutlineBBox bbox = ComputeOutlineBBox(ftFace, glyphIndex);

  // If bbox is invalid, try to use font metrics
  float unitsPerEm = static_cast<float>(ftFace->units_per_EM);
  if(unitsPerEm <= 0) unitsPerEm = 1000.0f;

  // Compute paint bounds using the shared helper
  // This gives the union of all PaintGlyph outline bounds with geometry transforms applied
  // If a precomputed hint is available (e.g., from the rasterizer cache), use it to
  // avoid redundant paint graph traversal. Fall back to direct computation if no hint.
  PaintBounds paintBounds;
  if(paintBoundsHint && paintBoundsHint->valid)
  {
    paintBounds = *paintBoundsHint;
  }
  else
  {
    GetPaintBounds(ftFace, glyphIndex, variationsHash, paintBounds);
  }

  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER PaintBounds glyph:%u valid:%d min:(%.1f,%.1f) max:(%.1f,%.1f) size:(%.1f,%.1f)\n",
                   glyphIndex, paintBounds.valid,
                   paintBounds.minX, paintBounds.minY, paintBounds.maxX, paintBounds.maxY,
                   paintBounds.valid ? paintBounds.maxX - paintBounds.minX : 0.0f,
                   paintBounds.valid ? paintBounds.maxY - paintBounds.minY : 0.0f);
  }
  // Log transform configuration (after unitsPerEm is available)
  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    const char* rootTransformName =
      (COLR_ROOT_TRANSFORM_OPTION == FT_COLOR_INCLUDE_ROOT_TRANSFORM) ? "INCLUDE_ROOT_TRANSFORM" : "NO_ROOT_TRANSFORM";
    const char* boundsSource = paintBounds.valid ? PaintBoundsSourceToString(paintBounds.source) : (bbox.valid ? "BASE_GLYPH" : "UPEM_FALLBACK");
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER TransformConfig glyph:%u root:%s yFlip:PATH_NEGATE bbox:%s upem:%.0f\n",
                   glyphIndex, rootTransformName, boundsSource, unitsPerEm);
  }

  // Calculate bitmap size, scale, and offset
  // Priority: paint bounds + font scale (primary) > base glyph bbox fit > UPEM fallback
  //
  // Primary path (PAINT_GRAPH): Use the same font scale as GetGlyphMetrics()
  //   fontScale = ppem / unitsPerEm
  //   bitmap size = paint bounds * fontScale + padding
  //   This ensures metric and bitmap use the same coordinate system.
  //
  // Fallback paths (BASE_GLYPH, UPEM_FALLBACK): Fit into targetWidth x targetHeight
  //   These are used when paint bounds are not available.
  float scale;
  float offsetX, offsetY;
  uint32_t bitmapWidth = targetWidth;
  uint32_t bitmapHeight = targetHeight;
  const char* boundsSource = "UPEM_FALLBACK";

  if(paintBounds.valid && (paintBounds.maxX - paintBounds.minX) > 0 && (paintBounds.maxY - paintBounds.minY) > 0 && ftFace->size)
  {
    // Primary path: ClipBox-first or paint graph bounds + uniform font scale
    //
    // DALi text font size is vertical-size based. COLRv1 glyph rendering uses a
    // uniform font scale derived from y_ppem / units_per_EM, matching normal text
    // rendering semantics. Supporting non-uniform x/y scale would require applying
    // an affine transform to the paint graph, which is outside this path.
    //
    // For the PAINT_GRAPH path, targetWidth/targetHeight are not used as a fit box.
    // The bitmap size is derived from paint bounds and font scale to keep metrics
    // and rasterization consistent. targetWidth/targetHeight are used only by
    // fallback paths where paint bounds are unavailable.
    scale = static_cast<float>(ftFace->size->metrics.y_ppem) / unitsPerEm;

    if(scale <= 0.0f)
    {
      if(IsColrDebugTraceEnabled(glyphIndex))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER Fail glyph:%u reason:invalid-scale scale:%f\n",
                       glyphIndex, scale);
      }
      return result;
    }

    const float paintW = paintBounds.maxX - paintBounds.minX;
    const float paintH = paintBounds.maxY - paintBounds.minY;

    // Bitmap size from paint bounds * uniform scale + padding
    const float padding = 2.0f;
    bitmapWidth  = static_cast<uint32_t>(std::ceil(paintW * scale + padding * 2.0f));
    bitmapHeight = static_cast<uint32_t>(std::ceil(paintH * scale + padding * 2.0f));

    // Maximum bitmap size limit.
    // TODO: Replace this local limit with FontClient/atlas policy input.
    // 1019 matches MAX_TEXT_ATLAS_SIZE(1024) - PADDING_TEXT_ATLAS_BLOCK(5).
    // Do NOT silently clamp; fail gracefully if the bitmap exceeds this limit.
    constexpr uint32_t MAX_COLR_BITMAP_WIDTH  = 1019u;
    constexpr uint32_t MAX_COLR_BITMAP_HEIGHT = 1019u;

    if(bitmapWidth > MAX_COLR_BITMAP_WIDTH || bitmapHeight > MAX_COLR_BITMAP_HEIGHT)
    {
      if(IsColrDebugTraceEnabled(glyphIndex))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER Fail glyph:%u reason:bitmap-too-large bitmap:%ux%u limit:%ux%u\n",
                       glyphIndex, bitmapWidth, bitmapHeight,
                       MAX_COLR_BITMAP_WIDTH, MAX_COLR_BITMAP_HEIGHT);
      }
      return result;
    }

    if(bitmapWidth == 0u) bitmapWidth = 1u;
    if(bitmapHeight == 0u) bitmapHeight = 1u;

    // Offset: place paint bounds within the bitmap
    // In font units (y-up), after y-negation in FtOutlineToTvgShape:
    //   bitmap left edge = padding → pen position maps to x = padding - paintBounds.minX * scale
    //   bitmap top edge = padding → baseline maps to y = padding + paintBounds.maxY * scale
    offsetX = padding - paintBounds.minX * scale;
    offsetY = padding + paintBounds.maxY * scale;
    boundsSource = PaintBoundsSourceToString(paintBounds.source);

    if(IsColrDebugTraceEnabled(glyphIndex))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER BBox source:%s min:(%.0f,%.0f) max:(%.0f,%.0f) scale:%.4f offset:(%.1f,%.1f) bitmap:%ux%u\n",
                     boundsSource, paintBounds.minX, paintBounds.minY, paintBounds.maxX, paintBounds.maxY, scale, offsetX, offsetY, bitmapWidth, bitmapHeight);
    }
  }
  else if(bbox.valid && (bbox.maxX - bbox.minX) > 0 && (bbox.maxY - bbox.minY) > 0)
  {
    // Fallback: fit base glyph bbox into target buffer
    float glyphW = bbox.maxX - bbox.minX;
    float glyphH = bbox.maxY - bbox.minY;

    float pad = 2.0f;
    float scaleX = (targetWidth - pad * 2) / glyphW;
    float scaleY = (targetHeight - pad * 2) / glyphH;
    scale = (scaleX < scaleY) ? scaleX : scaleY;

    offsetX = (targetWidth - glyphW * scale) / 2.0f - bbox.minX * scale;
    offsetY = (targetHeight - glyphH * scale) / 2.0f + bbox.maxY * scale;
    boundsSource = "BASE_GLYPH";

    if(IsColrDebugTraceEnabled(glyphIndex))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER BBox source:BASE_GLYPH min:(%.0f,%.0f) max:(%.0f,%.0f) scale:%.4f offset:(%.1f,%.1f)\n",
                     bbox.minX, bbox.minY, bbox.maxX, bbox.maxY, scale, offsetX, offsetY);
    }
  }
  else
  {
    scale = static_cast<float>(targetWidth) / unitsPerEm;
    offsetX = 0.0f;
    offsetY = static_cast<float>(targetHeight);
    boundsSource = "UPEM_FALLBACK";

    if(IsColrDebugTraceEnabled(glyphIndex))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER BBoxSource glyph:%u source:UPEM_FALLBACK reason:base-glyph-empty upem:%.0f scale:%.4f offset:(%.1f,%.1f)\n",
                     glyphIndex, unitsPerEm, scale, offsetX, offsetY);
    }
  }

  // Log bitmap config for debug glyph
  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER BitmapConfig glyph:%u ppem:(%u,%u) upem:%.0f bitmap:%ux%u scale:%.4f offset:(%.1f,%.1f) boundsSource:%s\n",
                   glyphIndex,
                   ftFace->size ? ftFace->size->metrics.x_ppem : 0,
                   ftFace->size ? ftFace->size->metrics.y_ppem : 0,
                   unitsPerEm, bitmapWidth, bitmapHeight, scale, offsetX, offsetY, boundsSource);
  }

  // ---- Root Composite path ----
  // Root Composite returns a raw RenderResult, so it reuses the offscreen
  // composite core and moves the composited buffer into the raster result rather
  // than wrapping it as a Picture for parent Scene insertion.
  {
    FT_COLR_Paint rootPaintPeek;
    if(FT_Get_Paint(ftFace, rootPaint, &rootPaintPeek) &&
       rootPaintPeek.format == FT_COLR_PAINTFORMAT_COMPOSITE)
    {
      if(IsColrDebugTraceEnabled(glyphIndex))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RootComposite detected glyph:%u mode:%d(%s) - using offscreen core\n",
                       glyphIndex,
                       static_cast<int>(rootPaintPeek.u.composite.composite_mode),
                       CompositeModeToString(rootPaintPeek.u.composite.composite_mode));
      }

      PaintContext rootCompositeCtx;
      rootCompositeCtx.ftFace = ftFace;
      rootCompositeCtx.paletteIndex = paletteIndex;
      rootCompositeCtx.targetWidth = bitmapWidth;
      rootCompositeCtx.targetHeight = bitmapHeight;
      rootCompositeCtx.unitsPerEm = unitsPerEm;
      rootCompositeCtx.scale = scale;
      rootCompositeCtx.offsetX = offsetX;
      rootCompositeCtx.offsetY = offsetY;
      rootCompositeCtx.debugGlyph = glyphIndex;
      rootCompositeCtx.allowOffscreenCompositeResult = false;
      rootCompositeCtx.rasterizer = this;
      rootCompositeCtx.variationsHash = variationsHash;
      rootCompositeCtx.useClipBoxBounds = true;
      rootCompositeCtx.clipBoxBoundsGlyph = glyphIndex;

      RootCompositeOffscreenResult offscreen = BuildCompositeOffscreenRootBuffer(
        ftFace, rootPaintPeek.u.composite, rootCompositeCtx, 0);

      if(MoveRootOffscreenToRenderResult(offscreen, result, paintBounds, offsetX, offsetY))
      {
        if(IsColrDebugTraceEnabled(glyphIndex))
        {
          uint32_t nonZero = CountNonZeroPixels(result.buffer, result.width, result.height, result.stride);
          DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RootComposite success glyph:%u bitmap:%ux%u origin:(%d,%d) offset:(%d,%d) nonZero:%u\n",
                         glyphIndex,
                         result.width,
                         result.height,
                         offscreen.pixelX,
                         offscreen.pixelY,
                         result.horizontalOffset,
                         result.verticalOffset,
                         nonZero);
        }

        return result;
      }
      else
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RootComposite failed glyph:%u\n",
                       glyphIndex);
        return result; // invalid
      }
    }
  }

  // ---- Normal (non-Composite) paint graph path ----
  // Set up paint context
  PaintContext ctx;
  ctx.ftFace = ftFace;
  ctx.paletteIndex = paletteIndex;
  ctx.targetWidth = targetWidth;
  ctx.targetHeight = targetHeight;
  ctx.unitsPerEm = unitsPerEm;
  ctx.scale = scale;
  ctx.offsetX = offsetX;
  ctx.offsetY = offsetY;
  ctx.debugGlyph = glyphIndex;
  ctx.rasterizer = this;
  ctx.variationsHash = variationsHash;
  ctx.useClipBoxBounds = true;
  ctx.clipBoxBoundsGlyph = glyphIndex;

  // Verify TransformState identity initialization (debug glyph only)
  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER TransformStateInit geometry:[%.2f,%.2f,%.2f,%.2f,%.1f,%.1f] paint:[%.2f,%.2f,%.2f,%.2f,%.1f,%.1f]\n",
                   ctx.geometryTransform.matrix[0], ctx.geometryTransform.matrix[1],
                   ctx.geometryTransform.matrix[2], ctx.geometryTransform.matrix[3],
                   ctx.geometryTransform.matrix[4], ctx.geometryTransform.matrix[5],
                   ctx.paintTransform.matrix[0], ctx.paintTransform.matrix[1],
                   ctx.paintTransform.matrix[2], ctx.paintTransform.matrix[3],
                   ctx.paintTransform.matrix[4], ctx.paintTransform.matrix[5]);
  }

  tvg::Paint* rootScenePaint = nullptr;
  if(TryBuildRootPaintForSceneBuilder(ftFace, rootPaint, ctx, rootScenePaint))
  {
    if(RenderRootPaintToRenderResult(rootScenePaint, result, paintBounds,
                                     bitmapWidth, bitmapHeight, offsetX, offsetY,
                                     glyphIndex))
    {
      return result;
    }

    if(IsColrDebugTraceEnabled(glyphIndex))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER DirectRootRaster failed glyph:%u - fail closed\n",
                     glyphIndex);
    }
  }

  if(IsColrDebugTraceEnabled(glyphIndex))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER Fail glyph:%u reason:root-builder-or-raster-failed\n", glyphIndex);
  }

  return result;
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER

// ============================================================
// Common public API - always compiled regardless of
// DALI_ENABLE_COLR_V1_RENDERER value.
// This ensures symbols exist in both enabled and disabled builds,
// preventing linker errors when other translation units reference them.
// ============================================================

namespace Dali::TextAbstraction::Internal
{

ColorGlyphColrRasterizer::ColorGlyphColrRasterizer(std::size_t maxPaintBoundsCacheSize)
: mClipBoxCache(maxPaintBoundsCacheSize),
  mPaintBoundsCache(maxPaintBoundsCacheSize),
  mThorvgInitialized(false)
{
#if DALI_ENABLE_COLR_V1_RENDERER
  const tvg::Result result = tvg::Initializer::init(0);
  if(result == tvg::Result::Success)
  {
    mThorvgInitialized = true;
  }
  else
  {
    mThorvgInitialized = false;
    DALI_LOG_ERROR("COLOR_GLYPH_COLR_RENDER ThorVG init failed result:%d\n", static_cast<int>(result));
  }
#endif
}

ColorGlyphColrRasterizer::~ColorGlyphColrRasterizer()
{
#if DALI_ENABLE_COLR_V1_RENDERER
  if(mThorvgInitialized)
  {
    tvg::Initializer::term();
    mThorvgInitialized = false;
  }
#endif
}

bool ColorGlyphColrRasterizer::GetPaintBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  std::size_t variationsHash,
  PaintBounds& outBounds)
{
  outBounds = PaintBounds();

  if(!ftFace)
  {
    return false;
  }

  if(TryGetClipBoxPaintBounds(ftFace, glyphIndex, variationsHash, outBounds))
  {
    return true;
  }

  return GetFallbackPaintGraphBounds(ftFace, glyphIndex, variationsHash, outBounds);
}

bool ColorGlyphColrRasterizer::GetFallbackPaintGraphBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  std::size_t variationsHash,
  PaintBounds& outBounds)
{
  outBounds = PaintBounds();

  if(!ftFace)
  {
    return false;
  }

  const PaintBoundsCacheKey key(ftFace, glyphIndex, variationsHash);

  auto iter = mPaintBoundsCache.Find(key);
  if(iter != mPaintBoundsCache.End())
  {
    auto& data = mPaintBoundsCache.Get(key);
    outBounds = data.bounds;
    return data.valid;
  }

  PaintBoundsCacheData data;
  data.valid = ComputePaintGraphBounds(ftFace, glyphIndex, data.bounds);

  mPaintBoundsCache.Push(key, data);

  outBounds = data.bounds;
  return data.valid;
}

ColorGlyphColrRasterizer::ClipBoxCacheStatus ColorGlyphColrRasterizer::GetClipBox(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  std::size_t variationsHash,
  RawClipBox& outClipBox)
{
  outClipBox = RawClipBox();

  if(!ftFace)
  {
    outClipBox.status = ClipBoxCacheStatus::INVALID_INPUT;
    return outClipBox.status;
  }

  const ClipBoxCacheKey key(ftFace, glyphIndex, variationsHash);

  auto iter = mClipBoxCache.Find(key);
  if(iter != mClipBoxCache.End())
  {
    auto& data = mClipBoxCache.Get(key);
    outClipBox = data.clipBox;
    return outClipBox.status;
  }

  ClipBoxCacheData data;
  data.clipBox = ComputeClipBox(ftFace, glyphIndex);
  mClipBoxCache.Push(key, data);

  outClipBox = data.clipBox;
  return outClipBox.status;
}

ColorGlyphColrRasterizer::RenderResult ColorGlyphColrRasterizer::Rasterize(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  std::size_t variationsHash,
  uint32_t targetWidth,
  uint32_t targetHeight,
  uint16_t paletteIndex)
{
  PaintBounds paintBounds;
  const bool hasPaintBounds = GetPaintBounds(ftFace, glyphIndex, variationsHash, paintBounds);

  return RasterizeInternal(
    ftFace,
    glyphIndex,
    targetWidth,
    targetHeight,
    paletteIndex,
    variationsHash,
    hasPaintBounds ? &paintBounds : nullptr);
}

void ColorGlyphColrRasterizer::ClearCache()
{
  mClipBoxCache.Clear();
  mPaintBoundsCache.Clear();
}

} // namespace Dali::TextAbstraction::Internal
