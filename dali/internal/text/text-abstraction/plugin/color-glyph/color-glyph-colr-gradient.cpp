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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-gradient.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-cpal.h>
#include <dali/integration-api/debug.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <algorithm>
#include <cmath>

namespace Dali::TextAbstraction::Internal
{

// ---- Convert COLRv1 ColorLine extend mode to ThorVG FillSpread ----
tvg::FillSpread ConvertColorLineExtend(FT_PaintExtend extend)
{
  if(extend == FT_COLR_PAINT_EXTEND_REPEAT)
  {
    return tvg::FillSpread::Repeat;
  }
  else if(extend == FT_COLR_PAINT_EXTEND_REFLECT)
  {
    return tvg::FillSpread::Reflect;
  }
  // Default: Pad
  return tvg::FillSpread::Pad;
}

// ---- Build ThorVG-safe color stops for unit range [0,1] ----
//
// ThorVG's colorStops() requires offsets in [0,1]. Raw COLRv1 offsets can be
// outside this range (spec allows it, with extend modes handling the meaning).
// This helper sanitizes raw stops for safe ThorVG consumption.
//
// Strategy:
// - Clamp offsets to [0,1]
// - Ensure first stop is at offset 0 (insert if needed, using first color)
// - Ensure last stop is at offset 1 (insert if needed, using last color)
// - Remove NaN/inf offsets
// - Result: all offsets finite, in [0,1], non-decreasing, at least 2 stops
//
// Returns false if safe stops cannot be produced (e.g., all offsets invalid).
bool BuildThorvgSafeStopsForUnitRange(
  const std::vector<tvg::Fill::ColorStop>& rawStops,
  std::vector<tvg::Fill::ColorStop>& safeStops)
{
  safeStops.clear();

  if(rawStops.empty())
  {
    return false;
  }

  // Filter out NaN/inf and clamp to [0,1]
  for(const auto& s : rawStops)
  {
    if(!std::isfinite(s.offset))
    {
      continue;
    }
    tvg::Fill::ColorStop safe = s;
    safe.offset = std::max(0.0f, std::min(1.0f, s.offset));
    safeStops.push_back(safe);
  }

  if(safeStops.empty())
  {
    return false;
  }

  // Sort by offset (non-decreasing)
  std::sort(safeStops.begin(), safeStops.end(),
    [](const tvg::Fill::ColorStop& a, const tvg::Fill::ColorStop& b) {
      return a.offset < b.offset;
    });

  // Ensure first stop at offset 0
  if(safeStops.front().offset > 0.0f)
  {
    tvg::Fill::ColorStop first = safeStops.front();
    first.offset = 0.0f;
    safeStops.insert(safeStops.begin(), first);
  }

  // Ensure last stop at offset 1
  if(safeStops.back().offset < 1.0f)
  {
    tvg::Fill::ColorStop last = safeStops.back();
    last.offset = 1.0f;
    safeStops.push_back(last);
  }

  return safeStops.size() >= 2;
}

// ---- Validate stops are safe for ThorVG colorStops() ----
// Returns true if all stops have finite offsets in [0,1] and are non-decreasing.
bool ValidateStopsForThorvg(const std::vector<tvg::Fill::ColorStop>& stops)
{
  if(stops.size() < 2)
  {
    return false;
  }
  float prevOffset = -1.0f;
  for(const auto& s : stops)
  {
    if(!std::isfinite(s.offset) || s.offset < 0.0f || s.offset > 1.0f)
    {
      return false;
    }
    if(s.offset < prevOffset)
    {
      return false; // not non-decreasing
    }
    prevOffset = s.offset;
  }
  return true;
}

namespace
{
// FreeType fixed-point conversion constants
// FT_Fixed 16.16: value / 65536.0f  (stop_offset, affine coefficients, coordinates)
// FT_F2Dot14 2.14: value / 16384.0f (alpha, angle)
constexpr float FROM_FT_FIXED  = 1.0f / 65536.0f;  // FT_Fixed 16.16
constexpr float FROM_F2DOT14   = 1.0f / 16384.0f;  // FT_F2Dot14 2.14
} // anonymous namespace

// ---- Build color stops from COLRv1 ColorLine ----
// Shared by HandlePaintRadialGradient and HandlePaintLinearGradient.
// Returns true if at least one stop was added, false if stops are empty.
//
// Per Google COLR gradients spec:
// - Color stop offsets are NOT clamped to [0,1]. Offsets outside [0,1] are
//   valid and their visual meaning is determined by the extend mode (PAD/REPEAT/REFLECT).
// - Stop alpha IS clamped to [0,1] for safety.
// - Stops are sorted by offset after collection.
// - outMinOffset/outMaxOffset report the actual offset range for callers
//   that need to normalize (e.g., LinearGradient geometry adjustment).
// - A single stop (stopCount==1) means the gradient is effectively a solid color;
//   the caller should handle this case (use solid fill instead of gradient).
bool BuildColorStops(
  FT_Face ftFace,
  uint16_t paletteIndex,
  FT_ColorLine& colorline,
  std::vector<tvg::Fill::ColorStop>& stops,
  uint32_t& stopCount,
  float& outMinOffset,
  float& outMaxOffset,
  uint32_t debugGlyph)
{
  stops.clear();
  stopCount = 0;
  outMinOffset = 0.0f;
  outMaxOffset = 1.0f;

  FT_ColorStop stop;
  FT_ColorStopIterator stopIter = colorline.color_stop_iterator;

  while(stopIter.current_color_stop < stopIter.num_color_stops)
  {
    FT_Bool ok = FT_Get_Colorline_Stops(ftFace, &stop, &stopIter);
    if(!ok) break;

    // Per Google COLR gradients spec: stop offset is a real number, NOT clamped to [0,1].
    // Out-of-range offsets are meaningful with REPEAT/REFLECT extend modes.
    //
    // FreeType API type for stop_offset is FT_Fixed (16.16 fixed-point).
    // Conversion: value * FROM_FT_FIXED = value / 65536.0f
    //
    // IMPORTANT: stop_offset uses FT_Fixed (16.16), NOT FT_F2Dot14 (2.14).
    // Using the wrong divisor (16384 instead of 65536) produces offset values
    // that are 4x too large, causing incorrect gradient rendering and
    // unnecessary geometry normalization.
    float offset = static_cast<float>(stop.stop_offset) * FROM_FT_FIXED;

    uint8_t r, g, b, a;
    if(!ResolveCpalColor(ftFace, paletteIndex, stop.color.palette_index, r, g, b, a))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER ColorLine unsupported paletteIndex:%u\n", stop.color.palette_index);
      stops.clear();
      stopCount = 0;
      return false;
    }

    // Apply stop alpha (FT_F2Dot14, clamped to [0,1] for safety)
    float stopAlpha = std::max(0.0f, std::min(1.0f, static_cast<float>(stop.color.alpha) * FROM_F2DOT14));
    a = static_cast<uint8_t>(a * stopAlpha);

    stops.push_back({offset, r, g, b, a});
    ++stopCount;
  }

  if(stops.empty())
  {
    return false;
  }

  // Sort stops by offset (ascending) per spec requirement
  std::sort(stops.begin(), stops.end(),
    [](const tvg::Fill::ColorStop& a, const tvg::Fill::ColorStop& b) {
      return a.offset < b.offset;
    });

  // Report actual offset range for caller normalization decisions
  outMinOffset = stops.front().offset;
  outMaxOffset = stops.back().offset;

  return true;
}

// ---- Build a ThorVG Matrix for paint transform (gradient coordinates) ----
// This is the paintTransform from ApplyPaintToShape wrapper transforms.
// Since y is already negated in path data, gradient coordinates also need
// y-flip adjustment. The paint transform operates in font-unit space before
// the global scale, so we apply y-negation to keep gradient coords consistent.
tvg::Matrix BuildPaintMatrix(const TransformState& paintTransform)
{
  const float txx = paintTransform.matrix[0];
  const float txy = paintTransform.matrix[1];
  const float tyx = paintTransform.matrix[2];
  const float tyy = paintTransform.matrix[3];
  const float tdx = paintTransform.matrix[4];
  const float tdy = paintTransform.matrix[5];

  // In y-flipped coords: [txx, -txy, -tyx, tyy, tdx, -tdy]
  const float mxx = txx;
  const float mxy = -txy;
  const float myx = -tyx;
  const float myy = tyy;
  const float mdx = tdx;
  const float mdy = -tdy;

  tvg::Matrix matrix;
  matrix.e11 = mxx;  matrix.e12 = mxy;  matrix.e13 = mdx;
  matrix.e21 = myx;  matrix.e22 = myy;  matrix.e23 = mdy;
  matrix.e31 = 0.0f; matrix.e32 = 0.0f; matrix.e33 = 1.0f;
  return matrix;
}

namespace
{

// ---- 3x3 affine matrix multiply helper ----
// Computes result = A * B (column-major ThorVG layout):
//   [e11 e12 e13]   [e11 e12 e13]
//   [e21 e22 e23] * [e21 e22 e23]
//   [e31 e32 e33]   [e31 e32 e33]
// Assumes e31=e32=0, e33=1 for both inputs.
inline tvg::Matrix MultiplyMatrices(const tvg::Matrix& a, const tvg::Matrix& b)
{
  tvg::Matrix result;
  result.e11 = a.e11 * b.e11 + a.e12 * b.e21;
  result.e12 = a.e11 * b.e12 + a.e12 * b.e22;
  result.e13 = a.e11 * b.e13 + a.e12 * b.e23 + a.e13;
  result.e21 = a.e21 * b.e11 + a.e22 * b.e21;
  result.e22 = a.e21 * b.e12 + a.e22 * b.e22;
  result.e23 = a.e21 * b.e13 + a.e22 * b.e23 + a.e23;
  result.e31 = 0.0f;
  result.e32 = 0.0f;
  result.e33 = 1.0f;
  return result;
}

} // anonymous namespace

// ---- Handle PaintLinearGradient ----
//
// Google COLR gradients spec interpretation of p0/p1/p2:
//
//   p0: gradient start point (color stop offset 0.0)
//   p1: gradient end point (color stop offset 1.0)
//   p2: gradient normal/orientation point
//
// The gradient line runs from p0 to p1. The gradient parameter s maps
// offset 0.0 → p0, offset 1.0 → p1.
//
// p2 defines the orientation of the gradient bands (iso-lines).
// When p2 == p1 (common simple case), the gradient bands are perpendicular
// to the p0→p1 line (standard linear gradient behavior).
//
// When p2 != p1, p2 defines a different orientation for the gradient bands.
// The gradient bands should be perpendicular to the line from p0 to p2.
// This requires a shear transform to skew ThorVG's native perpendicular
// bands to match the p2-defined orientation.
//
// Current implementation status:
// - p0→p1 direct gradient: fully implemented and stable.
// - p2 orientation shear: implemented but deferred. The shear math is
//   correct in principle, but real-world testing shows it may interact
//   poorly with other unimplemented features (PaintComposite, PaintColrGlyph).
//   The shear is applied when p2 != p1 and p2 != p0, but if it causes
//   visual regressions, it can be disabled by setting needShear = false.
//
// TODO: Advanced p2 orientation handling is limited. Full verification
// requires PaintComposite implementation, as many complex emoji use
// composite operations that currently fall back to solid gray.
//
// y-flip: FtOutlineToTvgShape() negates y coordinates (font y-up → canvas y-down).
// All gradient coordinates are converted to y-flipped space to match the path
// coordinate system. No double y-flip is applied.
//
bool HandlePaintLinearGradient(FT_PaintLinearGradient& linear, tvg::Shape* shape, PaintContext& ctx)
{
  if(IsColrDebugTraceEnabled(ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER PaintLinearGradient p0:(%.1f,%.1f) p1:(%.1f,%.1f) p2:(%.1f,%.1f)\n",
                   static_cast<float>(linear.p0.x) / 65536.0f,
                   static_cast<float>(linear.p0.y) / 65536.0f,
                   static_cast<float>(linear.p1.x) / 65536.0f,
                   static_cast<float>(linear.p1.y) / 65536.0f,
                   static_cast<float>(linear.p2.x) / 65536.0f,
                   static_cast<float>(linear.p2.y) / 65536.0f);
  }

  // Convert COLRv1 p0/p1/p2 from F16.16 font-unit space to y-flipped space.
  // FtOutlineToTvgShape negates y coordinates (font y-up → canvas y-down),
  // so gradient coordinates must also be in y-flipped space for consistency.
  // No double y-flip: we negate once here, and the path data is already negated.
  const float p0x =  static_cast<float>(linear.p0.x) / 65536.0f;
  const float p0y = -static_cast<float>(linear.p0.y) / 65536.0f;
  const float p1x =  static_cast<float>(linear.p1.x) / 65536.0f;
  const float p1y = -static_cast<float>(linear.p1.y) / 65536.0f;
  const float p2x =  static_cast<float>(linear.p2.x) / 65536.0f;
  const float p2y = -static_cast<float>(linear.p2.y) / 65536.0f;

  // Gradient direction vector (p0 → p1): axis along which gradient parameter varies.
  const float v1x = p1x - p0x;
  const float v1y = p1y - p0y;

  // Degenerate check: v1 must have non-zero length for a valid gradient direction.
  const float v1LenSq = v1x * v1x + v1y * v1y;
  if(v1LenSq < 1e-10f)
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient unsupported: degenerate v1 (p0==p1)\n");
    return false;
  }

  // ---- Build color stops ----
  FT_ColorLine& colorline = linear.colorline;
  tvg::FillSpread spread = ConvertColorLineExtend(colorline.extend);

  std::vector<tvg::Fill::ColorStop> stops;
  uint32_t stopCount = 0;
  float minOffset = 0.0f, maxOffset = 1.0f;
  if(!BuildColorStops(ctx.ftFace, ctx.paletteIndex, colorline, stops, stopCount, minOffset, maxOffset, ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient unsupported: no usable stops\n");
    return false;
  }

  // Single stop: effectively solid color
  if(stopCount == 1)
  {
    const auto& s = stops[0];
    shape->fill(s.r, s.g, s.b, s.a);
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient single stop → solid rgba:(%u,%u,%u,%u)\n", s.r, s.g, s.b, s.a);
    }
    return true;
  }

  // Allocate gradient (after early returns for no stops / single stop)
  tvg::LinearGradient* grad = tvg::LinearGradient::gen();
  if(!grad)
  {
    DALI_LOG_ERROR("COLOR_GLYPH_COLR_RENDER LinearGradient failed: grad alloc failed\n");
    return false;
  }

  // ---- Compute gradient start/end points and stop normalization ----
  //
  // The gradient line in font-unit space goes from p0 (offset 0.0) to p1 (offset 1.0).
  // If stop offsets are outside [0,1], extend the gradient line to preserve
  // the ColorLine meaning with PAD/REPEAT/REFLECT extend modes.
  //
  // Normalization approach (LinearGradient-specific):
  //   - Compute effective start/end from minOffset/maxOffset
  //   - Adjust gradient start/end: start = p0 + minOffset * v1, end = p0 + maxOffset * v1
  //   - Normalize stop offsets to [0,1]
  //
  // This preserves the visual meaning: the color at the original offset position
  // remains the same, and the extend mode works correctly beyond the normalized range.
  // Note: This normalization is NOT applied to RadialGradient, which requires
  // different geometry adjustment.
  //
  float gradStartX = p0x + minOffset * v1x;
  float gradStartY = p0y + minOffset * v1y;
  float gradEndX   = p0x + maxOffset * v1x;
  float gradEndY   = p0y + maxOffset * v1y;

  // Normalize stop offsets to [0,1] for ThorVG
  if(minOffset != maxOffset)
  {
    const float range = maxOffset - minOffset;
    for(auto& s : stops)
    {
      s.offset = (s.offset - minOffset) / range;
    }
  }

  // Create ThorVG linear gradient with computed start/end points
  grad->linear(gradStartX, gradStartY, gradEndX, gradEndY);
  grad->spread(spread);

  // Defensive: validate stops are safe for ThorVG before passing to colorStops().
  // After normalization, all offsets should be in [0,1], but check for safety.
  if(!ValidateStopsForThorvg(stops))
  {
    // Fallback: use BuildThorvgSafeStopsForUnitRange to sanitize
    std::vector<tvg::Fill::ColorStop> safeStops;
    if(!BuildThorvgSafeStopsForUnitRange(stops, safeStops))
    {
      delete grad;
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient failed: safe stops failed after normalize\n");
      return false;
    }
    grad->colorStops(safeStops.data(), safeStops.size());
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient stops sanitized after normalize raw:%u safe:%u\n",
                      stopCount, static_cast<uint32_t>(safeStops.size()));
    }
  }
  else
  {
    grad->colorStops(stops.data(), stops.size());
  }

  if(IsColrDebugTraceEnabled(ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient stops:%u spread:%d offsetRange:[%.3f,%.3f] start:(%.1f,%.1f) end:(%.1f,%.1f)\n",
                   stopCount, static_cast<int>(spread), minOffset, maxOffset,
                   gradStartX, gradStartY, gradEndX, gradEndY);
  }

  // ---- Compute the final transform (applied exactly once) ----
  //
  // Two possible transform sources:
  // 1. Shear from p2 orientation (when p2 != p1 and p2 != p0)
  // 2. Paint transform from wrapper paints inside PaintGlyph
  //
  // These are composed into a single final matrix and applied once via
  // grad->transform() to avoid any ambiguity about ThorVG's transform
  // accumulation behavior.
  //
  // Composition order: finalMatrix = paintMatrix * shearMatrix
  // This first applies the shear (adjusting band orientation in font-unit space),
  // then applies the paint transform (in y-flipped font-unit space).

  // --- Step 1: Compute shear matrix from p2 orientation ---
  tvg::Matrix shearMatrix;
  bool hasShear = false;

  // Check if p2 defines a different orientation than p1.
  // p2 == p1 means standard perpendicular bands (no shear needed).
  // p2 == p0 is degenerate (use standard perpendicular as fallback).
  const float wx = p2x - p0x;
  const float wy = p2y - p0y;
  const float wLenSq = wx * wx + wy * wy;

  // Check if p2 is different from p1 (not just collinear)
  // Cross direction: perpendicular to p0→p2, rotated 90° CCW
  const float crossX = (wLenSq > 1e-10f) ? -wy : -v1y;
  const float crossY = (wLenSq > 1e-10f) ?  wx :  v1x;

  // Check if cross is already perpendicular to v1
  const float dotV1Cross = v1x * crossX + v1y * crossY;
  const float crossLenSq = crossX * crossX + crossY * crossY;
  hasShear = (std::abs(dotV1Cross) > 1e-6f * std::sqrt(v1LenSq * crossLenSq));

  if(hasShear)
  {
    // Build shear: M = [v1 | v1_perp] * inv([v1 | cross])
    // This maps v1→v1 and cross→v1_perp, skewing the coordinate system
    // so ThorVG's perpendicular bands align with the p2-defined orientation.
    const float v1px = -v1y; // v1_perp
    const float v1py =  v1x;

    const float det = v1x * crossY - crossX * v1y;
    if(std::abs(det) < 1e-10f)
    {
      // Degenerate: v1 and cross are parallel - skip shear
      hasShear = false;
      if(IsColrDebugTraceEnabled(ctx.debugGlyph))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient shear: degenerate det, skipping\n");
      }
    }
    else
    {
      const float invDet = 1.0f / det;
      const float a11 =  crossY * invDet;
      const float a12 = -crossX * invDet;
      const float a21 = -v1y * invDet;
      const float a22 =  v1x * invDet;

      shearMatrix.e11 = v1x * a11 + v1px * a21;
      shearMatrix.e12 = v1x * a12 + v1px * a22;
      shearMatrix.e13 = 0.0f;
      shearMatrix.e21 = v1y * a11 + v1py * a21;
      shearMatrix.e22 = v1y * a12 + v1py * a22;
      shearMatrix.e23 = 0.0f;
      shearMatrix.e31 = 0.0f;
      shearMatrix.e32 = 0.0f;
      shearMatrix.e33 = 1.0f;

      if(IsColrDebugTraceEnabled(ctx.debugGlyph))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient shear computed [%.3f,%.3f,%.1f,%.3f,%.3f,%.1f] dotV1Cross:%.1f\n",
                       shearMatrix.e11, shearMatrix.e12, 0.0f,
                       shearMatrix.e21, shearMatrix.e22, 0.0f, dotV1Cross);
      }
    }
  }

  // --- Step 2: Compose and apply the final transform exactly once ---
  if(ctx.paintTransform.hasTransform)
  {
    tvg::Matrix paintMatrix = BuildPaintMatrix(ctx.paintTransform);

    if(hasShear)
    {
      // finalMatrix = paintMatrix * shearMatrix
      tvg::Matrix finalMatrix = MultiplyMatrices(paintMatrix, shearMatrix);
      grad->transform(finalMatrix);

      if(IsColrDebugTraceEnabled(ctx.debugGlyph))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient transform: paintMatrix*shearMatrix [%.2f,%.2f,%.1f,%.2f,%.2f,%.1f]\n",
                       finalMatrix.e11, finalMatrix.e12, finalMatrix.e13,
                       finalMatrix.e21, finalMatrix.e22, finalMatrix.e23);
      }
    }
    else
    {
      // No shear, just paint transform
      grad->transform(paintMatrix);

      if(IsColrDebugTraceEnabled(ctx.debugGlyph))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient transform: paintMatrix only [%.2f,%.2f,%.1f,%.2f,%.2f,%.1f]\n",
                       paintMatrix.e11, paintMatrix.e12, paintMatrix.e13,
                       paintMatrix.e21, paintMatrix.e22, paintMatrix.e23);
      }
    }
  }
  else if(hasShear)
  {
    // Shear only, no paint transform
    grad->transform(shearMatrix);

    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER LinearGradient transform: shearMatrix only [%.3f,%.3f,%.1f,%.3f,%.3f,%.1f]\n",
                     shearMatrix.e11, shearMatrix.e12, 0.0f,
                     shearMatrix.e21, shearMatrix.e22, 0.0f);
    }
  }
  // else: no transform needed (no shear, no paintTransform)

  // ThorVG 1.0: shape->fill(Fill*) takes raw pointer, ownership transfers to ThorVG
  shape->fill(grad);
  return true;
}

// ---- Handle PaintRadialGradient ----
bool HandlePaintRadialGradient(FT_PaintRadialGradient& radial, tvg::Shape* shape, PaintContext& ctx)
{
  if(IsColrDebugTraceEnabled(ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER PaintRadialGradient c0:(%.1f,%.1f) r0:%.1f c1:(%.1f,%.1f) r1:%.1f\n",
                   static_cast<float>(radial.c0.x) / 65536.0f,
                   static_cast<float>(radial.c0.y) / 65536.0f,
                   static_cast<float>(radial.r0) / 65536.0f,
                   static_cast<float>(radial.c1.x) / 65536.0f,
                   static_cast<float>(radial.c1.y) / 65536.0f,
                   static_cast<float>(radial.r1) / 65536.0f);
  }

  // ThorVG 1.0: gen() returns raw pointer (no .release() needed)
  tvg::RadialGradient* grad = tvg::RadialGradient::gen();
  if(!grad)
  {
    DALI_LOG_ERROR("COLOR_GLYPH_COLR_RENDER RadialGradient failed: grad alloc failed\n");
    return false;
  }

  // Convert COLRv1 radial gradient coordinates from F16.16 font-unit space to
  // y-flipped space, matching the path coordinate system used by FtOutlineToTvgShape().
  //
  // FtOutlineToTvgShape() negates y coordinates (font y-up → canvas y-down),
  // so gradient coordinates must also be in y-flipped space for consistency.
  // This is the same y-flip applied in HandlePaintLinearGradient().
  //
  // ThorVG 1.0: radial(cx, cy, r, fx, fy, fr)
  //   cx/cy/r = outer/end circle (COLRv1 c1/r1)
  //   fx/fy/fr = focal/start circle (COLRv1 c0/r0)
  //
  // COLRv1 spec: c0/r0 = start circle (offset 0.0), c1/r1 = end circle (offset 1.0)
  const float rawC0x = static_cast<float>(radial.c0.x) / 65536.0f;
  const float rawC0y = static_cast<float>(radial.c0.y) / 65536.0f;
  const float rawR0  = static_cast<float>(radial.r0) / 65536.0f;
  const float rawC1x = static_cast<float>(radial.c1.x) / 65536.0f;
  const float rawC1y = static_cast<float>(radial.c1.y) / 65536.0f;
  const float rawR1  = static_cast<float>(radial.r1) / 65536.0f;

  // y-flip: negate y coordinates to match path coordinate system
  float c0x =  rawC0x;
  float c0y = -rawC0y;
  float c1x =  rawC1x;
  float c1y = -rawC1y;

  // Build color stops FIRST to determine offset range before computing geometry.
  // This is different from the previous approach where grad->radial() was called
  // before stops were known. We need the offset range to decide whether to
  // normalize the circle geometry.
  FT_ColorLine& colorline = radial.colorline;
  tvg::FillSpread spread = ConvertColorLineExtend(colorline.extend);
  grad->spread(spread);

  std::vector<tvg::Fill::ColorStop> stops;
  uint32_t stopCount = 0;
  float minOffset = 0.0f, maxOffset = 1.0f;
  if(!BuildColorStops(ctx.ftFace, ctx.paletteIndex, colorline, stops, stopCount, minOffset, maxOffset, ctx.debugGlyph))
  {
    delete grad;
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient unsupported: no usable stops\n");
    return false;
  }

  // Single stop: effectively solid color
  if(stopCount == 1)
  {
    delete grad;
    const auto& s = stops[0];
    shape->fill(s.r, s.g, s.b, s.a);
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient single stop → solid rgba:(%u,%u,%u,%u)\n", s.r, s.g, s.b, s.a);
    }
    return true;
  }

  // ---- RadialGradient circle geometry normalization ----
  //
  // Per Google COLR gradients spec, the radial gradient parameter t interpolates
  // between the start circle (c0/r0, offset 0.0) and end circle (c1/r1, offset 1.0).
  // When color stop offsets are outside [0,1], the gradient extends beyond the
  // original circles. For example:
  //   - offset -0.5 means halfway "before" the start circle
  //   - offset 1.5 means halfway "after" the end circle
  //
  // To preserve this meaning while keeping ThorVG's colorStops() requirement of
  // offsets in [0,1], we can recompute the start/end circles to match the actual
  // stop offset range, then normalize the stop offsets to [0,1].
  //
  // Given the original parameterization:
  //   center(t) = c0 + t * (c1 - c0)
  //   radius(t) = r0 + t * (r1 - r0)
  //
  // For stop offset range [minOffset, maxOffset]:
  //   cStart = c0 + minOffset * (c1 - c0)
  //   rStart = r0 + minOffset * (r1 - r0)
  //   cEnd   = c0 + maxOffset * (c1 - c0)
  //   rEnd   = r0 + maxOffset * (r1 - r0)
  //   normalizedOffset = (offset - minOffset) / (maxOffset - minOffset)
  //
  // This preserves the visual meaning: the color at each original offset position
  // remains the same, and the extend mode (PAD/REPEAT/REFLECT) works correctly
  // on the normalized gradient.
  //
  // Limitations:
  // - This linear interpolation assumes the gradient parameter varies linearly
  //   between circles, which matches the COLRv1 spec for radial gradients.
  // - If rStart or rEnd becomes negative due to out-of-range offsets, we must
  //   clamp or fallback to prevent ThorVG issues.
  // - The extend mode is preserved: PAD extends the edge colors, REPEAT/REFLECT
  //   cycle the gradient within the normalized range.
  //
  // When offsets are already in [0,1], no normalization is needed and the
  // original circle geometry is used directly.

  bool useNormalizedGeometry = false;
  float finalC0x = c0x, finalC0y = c0y, finalR0 = rawR0;
  float finalC1x = c1x, finalC1y = c1y, finalR1 = rawR1;

  // Check if normalization is needed
  const bool offsetsInRange = (minOffset >= 0.0f && maxOffset <= 1.0f);
  const bool rangeIsDegenerate = (maxOffset - minOffset < 1e-6f);

  if(!offsetsInRange && !rangeIsDegenerate)
  {
    // Compute normalized circle geometry
    // center(t) = c0 + t * (c1 - c0), radius(t) = r0 + t * (r1 - r0)
    // Note: use y-flipped centers (c0x/c0y/c1x/c1y) for center interpolation
    const float dcx = c1x - c0x;
    const float dcy = c1y - c0y;
    const float dr  = rawR1 - rawR0;

    float normC0x = c0x + minOffset * dcx;
    float normC0y = c0y + minOffset * dcy;
    float normR0  = rawR0 + minOffset * dr;

    float normC1x = c0x + maxOffset * dcx;
    float normC1y = c0y + maxOffset * dcy;
    float normR1  = rawR0 + maxOffset * dr;

    // Radius defense policy:
    // - ThorVG radial() does not accept negative radius values.
    // - A zero or negative end radius (outer circle) would make the gradient
    //   degenerate. Use a guarded minimal positive radius.
    // - A negative start radius (focal circle) is clamped to 0.
    // - Well-formed COLRv1 fonts should not produce negative radii. This guard
    //   handles malformed or extreme offset ranges without synthesizing a new feature.
    if(normR1 <= 0.0f)
    {
      // End radius must be positive for a valid radial gradient.
      // If normalization produced a non-positive end radius, the offset range
      // extends so far beyond the original circles that the geometry is degenerate.
      normR1 = 1.0f;
      if(IsColrDebugTraceEnabled(ctx.debugGlyph))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient normalized rEnd<=0, clamped to 1.0\n");
      }
    }

    if(normR0 < 0.0f)
    {
      // Start (focal) radius should not be negative.
      // Clamp to 0: the focal point becomes a point rather than a circle.
      normR0 = 0.0f;
      if(IsColrDebugTraceEnabled(ctx.debugGlyph))
      {
        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient normalized rStart<0, clamped to 0.0\n");
      }
    }

    // Normalize stop offsets to [0,1]
    const float range = maxOffset - minOffset;
    for(auto& s : stops)
    {
      s.offset = (s.offset - minOffset) / range;
    }

    finalC0x = normC0x;
    finalC0y = normC0y;
    finalR0  = normR0;
    finalC1x = normC1x;
    finalC1y = normC1y;
    finalR1  = normR1;
    useNormalizedGeometry = true;

    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGeometry normalization applied: offsetRange:[%.3f,%.3f]\n",
                     minOffset, maxOffset);
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGeometry norm c0:(%.1f,%.1f) r0:%.1f c1:(%.1f,%.1f) r1:%.1f\n",
                     normC0x, normC0y, normR0, normC1x, normC1y, normR1);
    }
  }
  else if(rangeIsDegenerate)
  {
    // minOffset ≈ maxOffset: effectively a single color
    // Use the color at that offset as a solid fill
    delete grad;
    // Use the first stop's color.
    const auto& s = stops[0];
    shape->fill(s.r, s.g, s.b, s.a);
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient degenerate offset range [%.3f,%.3f] → solid\n",
                     minOffset, maxOffset);
    }
    return true;
  }

  // Set the radial gradient geometry on ThorVG
  // ThorVG 1.0: radial(cx, cy, r, fx, fy, fr)
  //   cx/cy/r = outer/end circle (COLRv1 c1/r1)
  //   fx/fy/fr = focal/start circle (COLRv1 c0/r0)
  grad->radial(finalC1x, finalC1y, finalR1, finalC0x, finalC0y, finalR0);

  // Detailed debug log for RadialGradient analysis
  if(IsColrDebugTraceEnabled(ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient raw c0:(%.1f,%.1f) r0:%.1f c1:(%.1f,%.1f) r1:%.1f\n",
                   rawC0x, rawC0y, rawR0, rawC1x, rawC1y, rawR1);
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient yflip c0:(%.1f,%.1f) c1:(%.1f,%.1f)\n",
                   c0x, c0y, c1x, c1y);
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGeometry normalized:%s offsetRange:[%.3f,%.3f]\n",
                   useNormalizedGeometry ? "yes" : "no", minOffset, maxOffset);
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient ThorVG radial(cx:%.1f,cy:%.1f,r:%.1f,fx:%.1f,fy:%.1f,fr:%.1f)\n",
                   finalC1x, finalC1y, finalR1, finalC0x, finalC0y, finalR0);
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient paintTransform hasTransform:%s\n",
                   ctx.paintTransform.hasTransform ? "true" : "false");
  }

  // Set color stops on ThorVG.
  // After normalization, stops should be in [0,1]. Validate as a safety measure.
  // If validation fails (e.g., due to floating point edge cases), fall back to
  // BuildThorvgSafeStopsForUnitRange which clamps and ensures boundary stops.
  if(!ValidateStopsForThorvg(stops))
  {
    std::vector<tvg::Fill::ColorStop> safeStops;
    if(!BuildThorvgSafeStopsForUnitRange(stops, safeStops))
    {
      delete grad;
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient failed: safe stops failed\n");
      return false;
    }
    grad->colorStops(safeStops.data(), safeStops.size());
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient stops sanitized raw:%u safe:%u offsetRange:[%.3f,%.3f] normalized:%s\n",
                      stopCount, static_cast<uint32_t>(safeStops.size()), minOffset, maxOffset,
                      useNormalizedGeometry ? "yes" : "no");
    }
  }
  else
  {
    grad->colorStops(stops.data(), stops.size());
  }

  if(IsColrDebugTraceEnabled(ctx.debugGlyph))
  {
    DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient stops:%u spread:%d offsetRange:[%.3f,%.3f] normalized:%s\n",
                   stopCount, static_cast<int>(spread), minOffset, maxOffset,
                   useNormalizedGeometry ? "yes" : "no");
  }

  // Apply paint transform to gradient coordinates if present.
  if(ctx.paintTransform.hasTransform)
  {
    tvg::Matrix paintMatrix = BuildPaintMatrix(ctx.paintTransform);
    grad->transform(paintMatrix);
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient paintTransform applied:1 matrix:[%.2f,%.2f,%.1f,%.2f,%.2f,%.1f]\n",
                     paintMatrix.e11, paintMatrix.e12, paintMatrix.e13,
                     paintMatrix.e21, paintMatrix.e22, paintMatrix.e23);
    }
  }
  else
  {
    if(IsColrDebugTraceEnabled(ctx.debugGlyph))
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER RadialGradient paintTransform applied:0 (identity)\n");
    }
  }

  // ThorVG 1.0: shape->fill(Fill*) takes raw pointer, ownership transfers to ThorVG
  shape->fill(grad);
  return true;
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
