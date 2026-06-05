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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-composite.h>
#include <dali/integration-api/debug.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace Dali::TextAbstraction::Internal
{

namespace
{

// ---- SoftLight per-pixel blend (W3C Compositing and Blending Level 1) ----
//
// Per spec: https://www.w3.org/TR/compositing-1/#blendingsoftlight
//
// Color formula (normalized Cb, Cs in [0,1]):
//   if Cs <= 0.5: B(Cb,Cs) = Cb - (1 - 2*Cs) * Cb * (1 - Cb)
//   else:         B(Cb,Cs) = Cb + (2*Cs - 1) * (D(Cb) - Cb)
//   where D(Cb) = ((16*Cb - 12)*Cb + 4)*Cb  if Cb <= 0.25
//                 sqrt(Cb)                    otherwise
//
// Alpha compositing (W3C general formula for separable blend modes):
//   Ao = As + Ab * (1 - As)
//   Co_premul = (1 - Ab) * As * Cs + (1 - As) * Ab * Cb + As * Ab * B(Cb, Cs)
//   if Ao > 0: Co = Co_premul / Ao
//   else: transparent black
//
// This follows the W3C compositing model more closely than the previous
// simplified straight-alpha path. The premultiplied intermediate calculation
// correctly handles edge cases where source and backdrop have partial alpha,
// reducing halo/edge artifacts at semi-transparent boundaries.
//

// BGRA byte order: B=[0], G=[1], R=[2], A=[3]
void BlendSoftLightPixelBgra(
  uint8_t* outPixel,
  const uint8_t* backdropPixel,
  const uint8_t* sourcePixel)
{
  const float Ab = backdropPixel[3] / 255.0f;
  const float As = sourcePixel[3] / 255.0f;

  // Output alpha: standard source-over
  const float Ao = As + Ab * (1.0f - As);

  if(Ao <= 0.0f)
  {
    outPixel[0] = 0; outPixel[1] = 0; outPixel[2] = 0; outPixel[3] = 0;
    return;
  }

  // Process B, G, R channels using W3C general compositing formula
  for(int i = 0; i < 3; ++i)
  {
    const float Cb = backdropPixel[i] / 255.0f;
    const float Cs = sourcePixel[i] / 255.0f;

    // SoftLight blend function B(Cb, Cs)
    float B;
    if(Cs <= 0.5f)
    {
      B = Cb - (1.0f - 2.0f * Cs) * Cb * (1.0f - Cb);
    }
    else
    {
      float D;
      if(Cb <= 0.25f)
      {
        D = ((16.0f * Cb - 12.0f) * Cb + 4.0f) * Cb;
      }
      else
      {
        D = std::sqrt(Cb);
      }
      B = Cb + (2.0f * Cs - 1.0f) * (D - Cb);
    }

    // W3C general compositing formula for separable blend modes:
    //   Co_premul = (1 - Ab) * As * Cs + (1 - As) * Ab * Cb + As * Ab * B
    //   Co = Co_premul / Ao
    const float Co_premul = (1.0f - Ab) * As * Cs + (1.0f - As) * Ab * Cb + As * Ab * B;
    const float Co = Co_premul / Ao;
    outPixel[i] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, Co * 255.0f)));
  }

  outPixel[3] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, Ao * 255.0f)));
}

// ---- Porter-Duff composite helpers ----

uint8_t ClampToByte(float value)
{
  return static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, value)));
}

void WriteTransparentBgra(uint8_t* outPixel)
{
  outPixel[0] = 0;
  outPixel[1] = 0;
  outPixel[2] = 0;
  outPixel[3] = 0;
}

void WritePremulBgra(uint8_t* outPixel, float coPremulB, float coPremulG, float coPremulR, float ao)
{
  if(ao <= 0.0f)
  {
    WriteTransparentBgra(outPixel);
    return;
  }

  outPixel[0] = ClampToByte((coPremulB / ao) * 255.0f);
  outPixel[1] = ClampToByte((coPremulG / ao) * 255.0f);
  outPixel[2] = ClampToByte((coPremulR / ao) * 255.0f);
  outPixel[3] = ClampToByte(ao * 255.0f);
}

// ---- Separable blend channel function ----
// Returns the blend result B(Cb, Cs) for a single color channel.
// Used by Multiply, Screen, Darken, Lighten, Difference.
float BlendSeparableChannel(FT_Composite_Mode mode, float cb, float cs)
{
  switch(mode)
  {
    case FT_COLR_COMPOSITE_MULTIPLY:
      return cb * cs;

    case FT_COLR_COMPOSITE_SCREEN:
      return cb + cs - cb * cs;

    case FT_COLR_COMPOSITE_DARKEN:
      return std::min(cb, cs);

    case FT_COLR_COMPOSITE_LIGHTEN:
      return std::max(cb, cs);

    case FT_COLR_COMPOSITE_DIFFERENCE:
      return std::abs(cb - cs);

    case FT_COLR_COMPOSITE_OVERLAY:
      if(cb <= 0.5f)
      {
        return 2.0f * cb * cs;
      }
      else
      {
        return 1.0f - 2.0f * (1.0f - cb) * (1.0f - cs);
      }

    case FT_COLR_COMPOSITE_COLOR_DODGE:
      if(cb <= 0.0f)
      {
        return 0.0f;
      }
      else if(cs >= 1.0f)
      {
        return 1.0f;
      }
      else
      {
        return std::min(1.0f, cb / (1.0f - cs));
      }

    case FT_COLR_COMPOSITE_COLOR_BURN:
      if(cb >= 1.0f)
      {
        return 1.0f;
      }
      else if(cs <= 0.0f)
      {
        return 0.0f;
      }
      else
      {
        return 1.0f - std::min(1.0f, (1.0f - cb) / cs);
      }

    case FT_COLR_COMPOSITE_HARD_LIGHT:
      if(cs <= 0.5f)
      {
        return 2.0f * cb * cs;
      }
      else
      {
        return 1.0f - 2.0f * (1.0f - cb) * (1.0f - cs);
      }

    case FT_COLR_COMPOSITE_EXCLUSION:
      return cb + cs - 2.0f * cb * cs;

    default:
      return cs;
  }
}

// ---- Separable blend per-pixel ----
// Applies a separable blend mode using the W3C general compositing formula:
//   Ao = As + Ab * (1 - As)
//   Co_premul = (1 - Ab) * As * Cs + (1 - As) * Ab * Cb + As * Ab * B(Cb, Cs)
//   Co = Co_premul / Ao
void BlendSeparablePixelBgra(
  uint8_t* outPixel,
  const uint8_t* backdropPixel,
  const uint8_t* sourcePixel,
  FT_Composite_Mode mode)
{
  const float Ab = backdropPixel[3] / 255.0f;
  const float As = sourcePixel[3] / 255.0f;
  const float Ao = As + Ab * (1.0f - As);

  if(Ao <= 0.0f)
  {
    WriteTransparentBgra(outPixel);
    return;
  }

  float coPremul[3];
  for(int c = 0; c < 3; ++c)
  {
    const float Cb = backdropPixel[c] / 255.0f;
    const float Cs = sourcePixel[c] / 255.0f;
    const float B = BlendSeparableChannel(mode, Cb, Cs);

    coPremul[c] =
      (1.0f - Ab) * As * Cs +
      (1.0f - As) * Ab * Cb +
      As * Ab * B;
  }

  WritePremulBgra(outPixel, coPremul[0], coPremul[1], coPremul[2], Ao);
}

// ---- Non-separable (HSL) blend helpers ----

struct FloatRgb
{
  float r{0.0f};
  float g{0.0f};
  float b{0.0f};
};

float Lum(const FloatRgb& color)
{
  return 0.3f * color.r + 0.59f * color.g + 0.11f * color.b;
}

float Sat(const FloatRgb& color)
{
  return std::max({color.r, color.g, color.b}) - std::min({color.r, color.g, color.b});
}

FloatRgb ClipColor(FloatRgb color)
{
  const float L = Lum(color);
  const float n = std::min({color.r, color.g, color.b});
  const float x = std::max({color.r, color.g, color.b});

  if(n < 0.0f)
  {
    const float denom = L - n;
    if(denom > 0.0f)
    {
      color.r = L + (color.r - L) * L / denom;
      color.g = L + (color.g - L) * L / denom;
      color.b = L + (color.b - L) * L / denom;
    }
  }

  if(x > 1.0f)
  {
    const float denom = x - L;
    if(denom > 0.0f)
    {
      color.r = L + (color.r - L) * (1.0f - L) / denom;
      color.g = L + (color.g - L) * (1.0f - L) / denom;
      color.b = L + (color.b - L) * (1.0f - L) / denom;
    }
  }

  return color;
}

FloatRgb SetLum(FloatRgb color, float targetLum)
{
  const float d = targetLum - Lum(color);
  color.r += d;
  color.g += d;
  color.b += d;
  return ClipColor(color);
}

FloatRgb SetSat(FloatRgb color, float targetSat)
{
  // Find min, mid, max channels by index to preserve R/G/B positions
  float* channels[3] = {&color.r, &color.g, &color.b};

  // Sort indices by value
  int idx[3] = {0, 1, 2};
  if(*channels[idx[0]] > *channels[idx[1]]) std::swap(idx[0], idx[1]);
  if(*channels[idx[1]] > *channels[idx[2]]) std::swap(idx[1], idx[2]);
  if(*channels[idx[0]] > *channels[idx[1]]) std::swap(idx[0], idx[1]);
  // Now idx[0]=min, idx[1]=mid, idx[2]=max

  const float cMin = *channels[idx[0]];
  const float cMid = *channels[idx[1]];
  const float cMax = *channels[idx[2]];

  if(cMax > cMin)
  {
    *channels[idx[1]] = ((cMid - cMin) * targetSat) / (cMax - cMin);
    *channels[idx[2]] = targetSat;
  }
  else
  {
    *channels[idx[1]] = 0.0f;
    *channels[idx[2]] = 0.0f;
  }
  *channels[idx[0]] = 0.0f;

  return color;
}

FloatRgb BlendNonSeparableRgb(FT_Composite_Mode mode, const FloatRgb& backdrop, const FloatRgb& source)
{
  switch(mode)
  {
    case FT_COLR_COMPOSITE_HSL_HUE:
      return SetLum(SetSat(source, Sat(backdrop)), Lum(backdrop));

    case FT_COLR_COMPOSITE_HSL_SATURATION:
      return SetLum(SetSat(backdrop, Sat(source)), Lum(backdrop));

    case FT_COLR_COMPOSITE_HSL_COLOR:
      return SetLum(source, Lum(backdrop));

    case FT_COLR_COMPOSITE_HSL_LUMINOSITY:
      return SetLum(backdrop, Lum(source));

    default:
      return source;
  }
}

void BlendNonSeparablePixelBgra(
  uint8_t* outPixel,
  const uint8_t* backdropPixel,
  const uint8_t* sourcePixel,
  FT_Composite_Mode mode)
{
  const float Ab = backdropPixel[3] / 255.0f;
  const float As = sourcePixel[3] / 255.0f;
  const float Ao = As + Ab * (1.0f - As);

  if(Ao <= 0.0f)
  {
    WriteTransparentBgra(outPixel);
    return;
  }

  // BGRA input → RGB struct
  const FloatRgb Cb{backdropPixel[2] / 255.0f,
                    backdropPixel[1] / 255.0f,
                    backdropPixel[0] / 255.0f};

  const FloatRgb Cs{sourcePixel[2] / 255.0f,
                    sourcePixel[1] / 255.0f,
                    sourcePixel[0] / 255.0f};

  const FloatRgb B = BlendNonSeparableRgb(mode, Cb, Cs);

  const float coPremulR =
    (1.0f - Ab) * As * Cs.r +
    (1.0f - As) * Ab * Cb.r +
    As * Ab * B.r;

  const float coPremulG =
    (1.0f - Ab) * As * Cs.g +
    (1.0f - As) * Ab * Cb.g +
    As * Ab * B.g;

  const float coPremulB =
    (1.0f - Ab) * As * Cs.b +
    (1.0f - As) * Ab * Cb.b +
    As * Ab * B.b;

  WritePremulBgra(outPixel, coPremulB, coPremulG, coPremulR, Ao);
}

} // anonymous namespace

void FreeCompositeBuffer(CompositeBuffer& buf)
{
  if(buf.buffer)
  {
    free(buf.buffer);
    buf.buffer = nullptr;
  }
  buf.width = 0;
  buf.height = 0;
  buf.stride = 0;
  buf.valid = false;
}

// Allocate a CompositeBuffer with calloc. Returns valid=false on failure.
CompositeBuffer AllocateCompositeBuffer(uint32_t width, uint32_t height)
{
  CompositeBuffer buf;
  buf.width = width;
  buf.height = height;
  buf.stride = width * 4;
  uint32_t size = buf.stride * height;
  if(size == 0) return buf;
  buf.buffer = static_cast<uint8_t*>(calloc(size, 1));
  if(buf.buffer)
  {
    buf.valid = true;
  }
  return buf;
}

void UnpremultiplyBgraBuffer(uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride)
{
  if(!buffer || width == 0u || height == 0u || stride < width * 4u)
  {
    return;
  }

  for(uint32_t y = 0u; y < height; ++y)
  {
    uint8_t* row = buffer + static_cast<size_t>(y) * stride;
    for(uint32_t x = 0u; x < width; ++x)
    {
      uint8_t* pixel = row + x * 4u;
      const uint8_t alpha = pixel[3];
      if(alpha == 0u)
      {
        pixel[0] = 0u;
        pixel[1] = 0u;
        pixel[2] = 0u;
        continue;
      }
      if(alpha == 255u)
      {
        continue;
      }

      for(uint32_t c = 0u; c < 3u; ++c)
      {
        const uint32_t straight = (static_cast<uint32_t>(pixel[c]) * 255u + alpha / 2u) / alpha;
        pixel[c] = static_cast<uint8_t>(std::min(straight, 255u));
      }
    }
  }
}

bool IsCompositeModeSupported(FT_Composite_Mode mode)
{
  switch(mode)
  {
    case FT_COLR_COMPOSITE_CLEAR:
    case FT_COLR_COMPOSITE_SRC:
    case FT_COLR_COMPOSITE_DEST:
    case FT_COLR_COMPOSITE_SRC_OVER:
    case FT_COLR_COMPOSITE_DEST_OVER:
    case FT_COLR_COMPOSITE_SRC_IN:
    case FT_COLR_COMPOSITE_DEST_IN:
    case FT_COLR_COMPOSITE_SRC_OUT:
    case FT_COLR_COMPOSITE_DEST_OUT:
    case FT_COLR_COMPOSITE_SRC_ATOP:
    case FT_COLR_COMPOSITE_DEST_ATOP:
    case FT_COLR_COMPOSITE_XOR:
    case FT_COLR_COMPOSITE_PLUS:
    case FT_COLR_COMPOSITE_MULTIPLY:
    case FT_COLR_COMPOSITE_SCREEN:
    case FT_COLR_COMPOSITE_OVERLAY:
    case FT_COLR_COMPOSITE_DARKEN:
    case FT_COLR_COMPOSITE_LIGHTEN:
    case FT_COLR_COMPOSITE_COLOR_DODGE:
    case FT_COLR_COMPOSITE_COLOR_BURN:
    case FT_COLR_COMPOSITE_HARD_LIGHT:
    case FT_COLR_COMPOSITE_SOFT_LIGHT:
    case FT_COLR_COMPOSITE_DIFFERENCE:
    case FT_COLR_COMPOSITE_EXCLUSION:
    case FT_COLR_COMPOSITE_HSL_HUE:
    case FT_COLR_COMPOSITE_HSL_SATURATION:
    case FT_COLR_COMPOSITE_HSL_COLOR:
    case FT_COLR_COMPOSITE_HSL_LUMINOSITY:
    {
      return true;
    }

    default:
    {
      return false;
    }
  }
}

// ---- Composite two straight BGRA8888 buffers using the specified composite mode ----
//
// Input: backdrop and source CompositeBuffers (must be valid, same dimensions)
// Output: straight BGRA8888 CompositeBuffer (allocated internally, caller owns)
// Returns true on success, false if mode is unsupported or validation fails.
bool CompositeBuffers(
  FT_Composite_Mode mode,
  const CompositeBuffer& backdrop,
  const CompositeBuffer& source,
  CompositeBuffer& output,
  uint32_t debugGlyph)
{
  // Validation
  if(!backdrop.valid || !source.valid) return false;
  if(backdrop.width != source.width || backdrop.height != source.height) return false;
  if(backdrop.stride < backdrop.width * 4 || source.stride < source.width * 4) return false;

  // Allocate output
  output = AllocateCompositeBuffer(backdrop.width, backdrop.height);
  if(!output.valid) return false;

  const uint32_t w = backdrop.width;
  const uint32_t h = backdrop.height;

  switch(mode)
  {
    case FT_COLR_COMPOSITE_SOFT_LIGHT:
    {
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          BlendSoftLightPixelBgra(outRow + x * 4, bdRow + x * 4, srRow + x * 4);
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_SRC:
    {
      // Copy source to output
      for(uint32_t y = 0; y < h; ++y)
      {
        memcpy(output.buffer + y * output.stride,
               source.buffer + y * source.stride,
               w * 4);
      }
      return true;
    }

    case FT_COLR_COMPOSITE_DEST:
    {
      // Copy backdrop to output
      for(uint32_t y = 0; y < h; ++y)
      {
        memcpy(output.buffer + y * output.stride,
               backdrop.buffer + y * backdrop.stride,
               w * 4);
      }
      return true;
    }

    case FT_COLR_COMPOSITE_SRC_OVER:
    {
      // Source-over compositing using W3C premultiplied alpha formula:
      //   Ao = As + Ab * (1 - As)
      //   Co_premul = As * Cs + Ab * Cb * (1 - As)
      //   if Ao > 0: Co = Co_premul / Ao
      //   else: transparent black
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = As + Ab * (1.0f - As);
          if(Ao <= 0.0f)
          {
            outRow[x*4+0] = 0; outRow[x*4+1] = 0;
            outRow[x*4+2] = 0; outRow[x*4+3] = 0;
          }
          else
          {
            for(int c = 0; c < 3; ++c)
            {
              const float Cs = srRow[x*4+c] / 255.0f;
              const float Cb = bdRow[x*4+c] / 255.0f;
              const float Co_premul = As * Cs + Ab * Cb * (1.0f - As);
              const float Co = Co_premul / Ao;
              outRow[x*4+c] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, Co * 255.0f)));
            }
            outRow[x*4+3] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, Ao * 255.0f)));
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_SRC_IN:
    {
      // Porter-Duff SrcIn: source inside backdrop
      // Per W3C Compositing and Blending Level 1 spec:
      //   Ao = As * Ab
      //   Co_premul = Cs * As * Ab
      //   if Ao > 0: Co = Co_premul / Ao = Cs (source color preserved)
      //   else: transparent black
      //
      // Simplification: when Ao > 0, Co = Cs (source color unchanged),
      // only alpha is modified by backdrop alpha.
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = As * Ab;

          if(Ao <= 0.0f)
          {
            outRow[x*4+0] = 0; outRow[x*4+1] = 0;
            outRow[x*4+2] = 0; outRow[x*4+3] = 0;
          }
          else
          {
            // Source color is preserved, only alpha is masked by backdrop
            outRow[x*4+0] = srRow[x*4+0];
            outRow[x*4+1] = srRow[x*4+1];
            outRow[x*4+2] = srRow[x*4+2];
            outRow[x*4+3] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, Ao * 255.0f)));
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_CLEAR:
    {
      // Porter-Duff Clear: both source and backdrop removed
      // Ao = 0, Co = transparent black
      for(uint32_t y = 0; y < h; ++y)
      {
        uint8_t* outRow = output.buffer + y * output.stride;
        memset(outRow, 0, w * 4);
      }
      return true;
    }

    case FT_COLR_COMPOSITE_DEST_OVER:
    {
      // Porter-Duff DestOver: backdrop over source
      // Ao = Ab + As * (1 - Ab)
      // Co_premul = Ab * Cb + As * Cs * (1 - Ab)
      // Co = Co_premul / Ao
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = Ab + As * (1.0f - Ab);

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            const float CbB = bdRow[x*4+0] / 255.0f;
            const float CbG = bdRow[x*4+1] / 255.0f;
            const float CbR = bdRow[x*4+2] / 255.0f;
            const float CsB = srRow[x*4+0] / 255.0f;
            const float CsG = srRow[x*4+1] / 255.0f;
            const float CsR = srRow[x*4+2] / 255.0f;

            const float coPremulB = Ab * CbB + As * CsB * (1.0f - Ab);
            const float coPremulG = Ab * CbG + As * CsG * (1.0f - Ab);
            const float coPremulR = Ab * CbR + As * CsR * (1.0f - Ab);
            WritePremulBgra(outRow + x * 4, coPremulB, coPremulG, coPremulR, Ao);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_DEST_IN:
    {
      // Porter-Duff DestIn: backdrop inside source
      // Ao = Ab * As
      // Co = Cb (backdrop color preserved, alpha masked by source)
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = Ab * As;

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            // Backdrop color preserved, alpha masked by source
            outRow[x*4+0] = bdRow[x*4+0];
            outRow[x*4+1] = bdRow[x*4+1];
            outRow[x*4+2] = bdRow[x*4+2];
            outRow[x*4+3] = ClampToByte(Ao * 255.0f);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_SRC_OUT:
    {
      // Porter-Duff SrcOut: source outside backdrop
      // Ao = As * (1 - Ab)
      // Co = Cs (source color preserved, alpha masked by 1-Ab)
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = As * (1.0f - Ab);

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            // Source color preserved, alpha masked by (1 - Ab)
            outRow[x*4+0] = srRow[x*4+0];
            outRow[x*4+1] = srRow[x*4+1];
            outRow[x*4+2] = srRow[x*4+2];
            outRow[x*4+3] = ClampToByte(Ao * 255.0f);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_DEST_OUT:
    {
      // Porter-Duff DestOut: backdrop outside source
      // Ao = Ab * (1 - As)
      // Co = Cb (backdrop color preserved, alpha masked by 1-As)
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = Ab * (1.0f - As);

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            // Backdrop color preserved, alpha masked by (1 - As)
            outRow[x*4+0] = bdRow[x*4+0];
            outRow[x*4+1] = bdRow[x*4+1];
            outRow[x*4+2] = bdRow[x*4+2];
            outRow[x*4+3] = ClampToByte(Ao * 255.0f);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_SRC_ATOP:
    {
      // Porter-Duff SrcAtop: source atop backdrop
      // Ao = Ab
      // Co_premul = As * Cs * Ab + Ab * Cb * (1 - As)
      // Co = Co_premul / Ao
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = Ab;

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            const float CbB = bdRow[x*4+0] / 255.0f;
            const float CbG = bdRow[x*4+1] / 255.0f;
            const float CbR = bdRow[x*4+2] / 255.0f;
            const float CsB = srRow[x*4+0] / 255.0f;
            const float CsG = srRow[x*4+1] / 255.0f;
            const float CsR = srRow[x*4+2] / 255.0f;

            const float coPremulB = As * CsB * Ab + Ab * CbB * (1.0f - As);
            const float coPremulG = As * CsG * Ab + Ab * CbG * (1.0f - As);
            const float coPremulR = As * CsR * Ab + Ab * CbR * (1.0f - As);
            WritePremulBgra(outRow + x * 4, coPremulB, coPremulG, coPremulR, Ao);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_DEST_ATOP:
    {
      // Porter-Duff DestAtop: backdrop atop source
      // Ao = As
      // Co_premul = Ab * Cb * As + As * Cs * (1 - Ab)
      // Co = Co_premul / Ao
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = As;

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            const float CbB = bdRow[x*4+0] / 255.0f;
            const float CbG = bdRow[x*4+1] / 255.0f;
            const float CbR = bdRow[x*4+2] / 255.0f;
            const float CsB = srRow[x*4+0] / 255.0f;
            const float CsG = srRow[x*4+1] / 255.0f;
            const float CsR = srRow[x*4+2] / 255.0f;

            const float coPremulB = Ab * CbB * As + As * CsB * (1.0f - Ab);
            const float coPremulG = Ab * CbG * As + As * CsG * (1.0f - Ab);
            const float coPremulR = Ab * CbR * As + As * CsR * (1.0f - Ab);
            WritePremulBgra(outRow + x * 4, coPremulB, coPremulG, coPremulR, Ao);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_XOR:
    {
      // Porter-Duff Xor: source outside backdrop + backdrop outside source
      // Ao = As * (1 - Ab) + Ab * (1 - As)
      // Co_premul = As * Cs * (1 - Ab) + Ab * Cb * (1 - As)
      // Co = Co_premul / Ao
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x*4+3] / 255.0f;
          const float Ab = bdRow[x*4+3] / 255.0f;
          const float Ao = As * (1.0f - Ab) + Ab * (1.0f - As);

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            const float CbB = bdRow[x*4+0] / 255.0f;
            const float CbG = bdRow[x*4+1] / 255.0f;
            const float CbR = bdRow[x*4+2] / 255.0f;
            const float CsB = srRow[x*4+0] / 255.0f;
            const float CsG = srRow[x*4+1] / 255.0f;
            const float CsR = srRow[x*4+2] / 255.0f;

            const float coPremulB = As * CsB * (1.0f - Ab) + Ab * CbB * (1.0f - As);
            const float coPremulG = As * CsG * (1.0f - Ab) + Ab * CbG * (1.0f - As);
            const float coPremulR = As * CsR * (1.0f - Ab) + Ab * CbR * (1.0f - As);
            WritePremulBgra(outRow + x * 4, coPremulB, coPremulG, coPremulR, Ao);
          }
        }
      }
      return true;
    }

    case FT_COLR_COMPOSITE_PLUS:
    {
      // Porter-Duff Plus (Lighter): additive compositing
      // Ao = min(1, As + Ab)
      // Co_premul = min(1, As * Cs + Ab * Cb)
      // Co = Co_premul / Ao
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;
        for(uint32_t x = 0; x < w; ++x)
        {
          const float As = srRow[x * 4 + 3] / 255.0f;
          const float Ab = bdRow[x * 4 + 3] / 255.0f;
          const float Ao = std::min(1.0f, As + Ab);

          if(Ao <= 0.0f)
          {
            WriteTransparentBgra(outRow + x * 4);
          }
          else
          {
            const float CbB = bdRow[x * 4 + 0] / 255.0f;
            const float CbG = bdRow[x * 4 + 1] / 255.0f;
            const float CbR = bdRow[x * 4 + 2] / 255.0f;
            const float CsB = srRow[x * 4 + 0] / 255.0f;
            const float CsG = srRow[x * 4 + 1] / 255.0f;
            const float CsR = srRow[x * 4 + 2] / 255.0f;

            const float coPremulB = std::min(1.0f, Ab * CbB + As * CsB);
            const float coPremulG = std::min(1.0f, Ab * CbG + As * CsG);
            const float coPremulR = std::min(1.0f, Ab * CbR + As * CsR);
            WritePremulBgra(outRow + x * 4, coPremulB, coPremulG, coPremulR, Ao);
          }
        }
      }
      return true;
    }

    // ---- Separable blend modes ----
    // Using W3C general compositing formula:
    //   Ao = As + Ab * (1 - As)
    //   Co_premul = (1 - Ab) * As * Cs + (1 - As) * Ab * Cb + As * Ab * B(Cb, Cs)
    //   Co = Co_premul / Ao
    case FT_COLR_COMPOSITE_MULTIPLY:
    case FT_COLR_COMPOSITE_SCREEN:
    case FT_COLR_COMPOSITE_OVERLAY:
    case FT_COLR_COMPOSITE_DARKEN:
    case FT_COLR_COMPOSITE_LIGHTEN:
    case FT_COLR_COMPOSITE_COLOR_DODGE:
    case FT_COLR_COMPOSITE_COLOR_BURN:
    case FT_COLR_COMPOSITE_HARD_LIGHT:
    case FT_COLR_COMPOSITE_DIFFERENCE:
    case FT_COLR_COMPOSITE_EXCLUSION:
    {
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;

        for(uint32_t x = 0; x < w; ++x)
        {
          BlendSeparablePixelBgra(
            outRow + x * 4,
            bdRow + x * 4,
            srRow + x * 4,
            mode);
        }
      }
      return true;
    }

    // ---- Non-separable (HSL) blend modes ----
    // Using W3C general compositing formula with non-separable blend function:
    //   Ao = As + Ab * (1 - As)
    //   Co_premul = (1 - Ab) * As * Cs + (1 - As) * Ab * Cb + As * Ab * B(Cb, Cs)
    //   where B is an RGB vector from BlendNonSeparableRgb()
    //   Co = Co_premul / Ao
    case FT_COLR_COMPOSITE_HSL_HUE:
    case FT_COLR_COMPOSITE_HSL_SATURATION:
    case FT_COLR_COMPOSITE_HSL_COLOR:
    case FT_COLR_COMPOSITE_HSL_LUMINOSITY:
    {
      for(uint32_t y = 0; y < h; ++y)
      {
        const uint8_t* bdRow = backdrop.buffer + y * backdrop.stride;
        const uint8_t* srRow = source.buffer + y * source.stride;
        uint8_t* outRow = output.buffer + y * output.stride;

        for(uint32_t x = 0; x < w; ++x)
        {
          BlendNonSeparablePixelBgra(
            outRow + x * 4,
            bdRow + x * 4,
            srRow + x * 4,
            mode);
        }
      }
      return true;
    }

    default:
    {
      DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER CompositeBuffers unsupported mode:%d\n",
                     static_cast<int>(mode));
      FreeCompositeBuffer(output);
      return false;
    }
  }
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
