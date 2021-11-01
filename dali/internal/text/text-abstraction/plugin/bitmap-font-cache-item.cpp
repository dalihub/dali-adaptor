
/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/internal/text/text-abstraction/plugin/bitmap-font-cache-item.h>

#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
BitmapFontCacheItem::BitmapFontCacheItem(const BitmapFont& bitmapFont, FontId fontId)
: font(bitmapFont),
  id(fontId)
{
  // Resize the vector with the pixel buffers.
  pixelBuffers.resize(bitmapFont.glyphs.size());

  // Traverse all the glyphs and load the pixel buffer of those with ascender and descender equal to zero.
  unsigned int index = 0u;
  for(auto& glyph : font.glyphs)
  {
    Devel::PixelBuffer& pixelBuffer = pixelBuffers[index];

    if(EqualsZero(glyph.ascender) && EqualsZero(glyph.descender))
    {
      // Load the glyph.
      pixelBuffer = LoadImageFromFile(glyph.url);

      if(pixelBuffer)
      {
        glyph.ascender = static_cast<float>(pixelBuffer.GetHeight());
      }
    }

    font.ascender  = std::max(glyph.ascender, font.ascender);
    font.descender = std::min(glyph.descender, font.descender);

    ++index;
  }
}

void BitmapFontCacheItem::GetFontMetrics(FontMetrics& metrics, unsigned int dpiVertical) const
{
  metrics.ascender           = font.ascender;
  metrics.descender          = font.descender;
  metrics.height             = metrics.ascender - metrics.descender;
  metrics.underlinePosition  = font.underlinePosition;
  metrics.underlineThickness = font.underlineThickness;
}

bool BitmapFontCacheItem::GetGlyphMetrics(GlyphInfo& glyph, unsigned int dpiVertical, bool horizontal) const
{
  bool success(false);

  unsigned int index = 0u;
  for(auto& item : font.glyphs)
  {
    if(item.utf32 == glyph.index)
    {
      Devel::PixelBuffer& pixelBuffer = const_cast<Devel::PixelBuffer&>(pixelBuffers[index]);
      if(!pixelBuffer)
      {
        pixelBuffer = LoadImageFromFile(item.url);
      }

      glyph.width       = static_cast<float>(pixelBuffer.GetWidth());
      glyph.height      = static_cast<float>(pixelBuffer.GetHeight());
      glyph.xBearing    = 0.f;
      glyph.yBearing    = glyph.height + item.descender;
      glyph.advance     = glyph.width;
      glyph.scaleFactor = 1.f;
      success           = true;
      break;
    }
    ++index;
  }
  return success;
}

void BitmapFontCacheItem::CreateBitmap(
  GlyphIndex glyphIndex, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const
{
  unsigned int index = 0u;
  for(auto& item : font.glyphs)
  {
    if(item.utf32 == glyphIndex)
    {
      Devel::PixelBuffer& pixelBuffer = const_cast<Devel::PixelBuffer&>(pixelBuffers[index]);
      if(!pixelBuffer)
      {
        pixelBuffer = LoadImageFromFile(item.url);
      }

      data.width  = pixelBuffer.GetWidth();
      data.height = pixelBuffer.GetHeight();

      data.isColorBitmap = font.isColorFont;

      ConvertBitmap(data, data.width, data.height, pixelBuffer.GetBuffer());

      // Sets the pixel format.
      data.format = pixelBuffer.GetPixelFormat();
      break;
    }
    ++index;
  }
}

bool BitmapFontCacheItem::IsCharacterSupported(Character character)
{
  for(const auto& glyph : font.glyphs)
  {
    if(glyph.utf32 == character)
    {
      return true;
    }
  }
  return false;
}

} // namespace Dali::TextAbstraction::Internal
