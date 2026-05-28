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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-common.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <cstdlib>
#include <cstdio>
#include <cstdint>

namespace Dali::TextAbstraction::Internal
{

// DALI_COLR_DEBUG_GLYPH=<glyphIndex> enables detailed trace logs for one glyph.
// The value is parsed once and invalid values silently disable tracing.
bool IsColrDebugTraceEnabled(uint32_t glyphIndex)
{
  struct ColrDebugTraceConfig
  {
    uint32_t glyphIndex{0};
    bool enabled{false};

    ColrDebugTraceConfig()
    {
      const char* env = std::getenv("DALI_COLR_DEBUG_GLYPH");
      if(env && env[0] != '\0')
      {
        char* end = nullptr;
        unsigned long value = std::strtoul(env, &end, 10);
        if(end != env && value <= UINT32_MAX)
        {
          glyphIndex = static_cast<uint32_t>(value);
          enabled = true;
        }
      }
    }
  };

  static const ColrDebugTraceConfig config;

  return config.enabled && (glyphIndex == config.glyphIndex);
}

// ---- Paint format name helper for debug logging ----
// Maps FT_PaintFormat enum values to human-readable names.
// Based on FreeType 2.13.2 FT_COLR_PAINTFORMAT_* definitions.
// FreeType 2.13.2 enum values (NOT sequential - uses even numbers with gaps):
//   1=ColrLayers, 2=Solid, 4=LinearGradient, 6=RadialGradient, 8=SweepGradient
//   10=Glyph, 11=ColrGlyph, 12=Transform, 14=Translate, 16=Scale
//   24=Rotate, 28=Skew, 32=Composite, 255=Unsupported
const char* PaintFormatToString(FT_PaintFormat format)
{
  switch(format)
  {
    case FT_COLR_PAINTFORMAT_COLR_LAYERS:     return "ColrLayers";
    case FT_COLR_PAINTFORMAT_SOLID:           return "Solid";
    case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT: return "LinearGradient";
    case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT: return "RadialGradient";
    case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:  return "SweepGradient";
    case FT_COLR_PAINTFORMAT_GLYPH:           return "Glyph";
    case FT_COLR_PAINTFORMAT_COLR_GLYPH:      return "ColrGlyph";
    case FT_COLR_PAINTFORMAT_TRANSFORM:       return "Transform";
    case FT_COLR_PAINTFORMAT_TRANSLATE:       return "Translate";
    case FT_COLR_PAINTFORMAT_SCALE:           return "Scale";
    case FT_COLR_PAINTFORMAT_ROTATE:          return "Rotate";
    case FT_COLR_PAINTFORMAT_SKEW:            return "Skew";
    case FT_COLR_PAINTFORMAT_COMPOSITE:       return "Composite";
    default:
    {
      static thread_local char buf[32];
      snprintf(buf, sizeof(buf), "Unknown(%d)", static_cast<int>(format));
      return buf;
    }
  }
}

// ---- Composite mode string helper for debug logging ----
// Maps FT_Composite_Mode enum values to human-readable names.
// Only used for debug diagnostics; does not affect rendering.
const char* CompositeModeToString(FT_Composite_Mode mode)
{
  switch(mode)
  {
    case FT_COLR_COMPOSITE_CLEAR:          return "Clear";
    case FT_COLR_COMPOSITE_SRC:            return "Src";
    case FT_COLR_COMPOSITE_DEST:           return "Dest";
    case FT_COLR_COMPOSITE_SRC_OVER:       return "SrcOver";
    case FT_COLR_COMPOSITE_DEST_OVER:      return "DstOver";
    case FT_COLR_COMPOSITE_SRC_IN:         return "SrcIn";
    case FT_COLR_COMPOSITE_DEST_IN:        return "DstIn";
    case FT_COLR_COMPOSITE_SRC_OUT:        return "SrcOut";
    case FT_COLR_COMPOSITE_DEST_OUT:       return "DstOut";
    case FT_COLR_COMPOSITE_SRC_ATOP:       return "SrcAtop";
    case FT_COLR_COMPOSITE_DEST_ATOP:      return "DstAtop";
    case FT_COLR_COMPOSITE_XOR:            return "Xor";
    case FT_COLR_COMPOSITE_PLUS:           return "Plus";
    case FT_COLR_COMPOSITE_SCREEN:         return "Screen";
    case FT_COLR_COMPOSITE_OVERLAY:        return "Overlay";
    case FT_COLR_COMPOSITE_DARKEN:         return "Darken";
    case FT_COLR_COMPOSITE_LIGHTEN:        return "Lighten";
    case FT_COLR_COMPOSITE_COLOR_DODGE:    return "ColorDodge";
    case FT_COLR_COMPOSITE_COLOR_BURN:     return "ColorBurn";
    case FT_COLR_COMPOSITE_HARD_LIGHT:     return "HardLight";
    case FT_COLR_COMPOSITE_SOFT_LIGHT:     return "SoftLight";
    case FT_COLR_COMPOSITE_DIFFERENCE:     return "Difference";
    case FT_COLR_COMPOSITE_EXCLUSION:      return "Exclusion";
    case FT_COLR_COMPOSITE_MULTIPLY:       return "Multiply";
    case FT_COLR_COMPOSITE_HSL_HUE:        return "HslHue";
    case FT_COLR_COMPOSITE_HSL_SATURATION: return "HslSaturation";
    case FT_COLR_COMPOSITE_HSL_COLOR:      return "HslColor";
    case FT_COLR_COMPOSITE_HSL_LUMINOSITY: return "HslLuminosity";
    default:
    {
      static thread_local char buf[32];
      snprintf(buf, sizeof(buf), "Unknown(%d)", static_cast<int>(mode));
      return buf;
    }
  }
}

// ---- Build a ThorVG Matrix for geometry transform (shape path) ----
// This composes: GlobalOffset + GlobalScale * GeometryTransform
// Since y is already negated in path data (FtOutlineToTvgShape), the geometry
// transform y components need negation to stay consistent with y-flipped coords.
tvg::Matrix BuildGeometryMatrix(
  const TransformState& geometryTransform,
  float globalScale,
  float offsetX,
  float offsetY)
{
  const float s = globalScale;
  const float txx = geometryTransform.matrix[0];
  const float txy = geometryTransform.matrix[1];
  const float tyx = geometryTransform.matrix[2];
  const float tyy = geometryTransform.matrix[3];
  const float tdx = geometryTransform.matrix[4];
  const float tdy = geometryTransform.matrix[5];

  // In y-flipped coords: [txx, -txy, -tyx, tyy, tdx, -tdy]
  // Then apply global scale and offset.
  const float mxx = s * txx;
  const float mxy = s * (-txy);
  const float myx = s * (-tyx);
  const float myy = s * tyy;
  const float mdx = s * tdx + offsetX;
  const float mdy = s * (-tdy) + offsetY;

  tvg::Matrix matrix;
  matrix.e11 = mxx;  matrix.e12 = mxy;  matrix.e13 = mdx;
  matrix.e21 = myx;  matrix.e22 = myy;  matrix.e23 = mdy;
  matrix.e31 = 0.0f; matrix.e32 = 0.0f; matrix.e33 = 1.0f;
  return matrix;
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
