#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_BITMAP_FONT_CACHE_ITEM_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_BITMAP_FONT_CACHE_ITEM_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/devel-api/text-abstraction/bitmap-font.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

#include <dali/internal/text/text-abstraction/plugin/font-cache-item-interface.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Stores a bitmap font and its pixel buffers per glyph.
 */
struct BitmapFontCacheItem : public FontCacheItemInterface
{
  /**
   * Constructor
   *
   * @param[in] bitmapFont The font to cache
   * @param[in] fontId The id of the font
   */
  BitmapFontCacheItem(const BitmapFont& bitmapFont);

  /**
   * Destructor
   */
  ~BitmapFontCacheItem() = default;

  /**
   * @copydoc FontCacheItemInterface::GetFontMetrics()
   */
  void GetFontMetrics(FontMetrics& metrics, unsigned int dpiVertical) const override;

  /**
   * @copydoc FontCacheItemInterface::GetGlyphMetrics()
   */
  bool GetGlyphMetrics(GlyphInfo& glyphInfo, unsigned int dpiVertical, bool horizontal) const override;

  /**
   * @copydoc FontCacheItemInterface::CreateBitmap()
   */
  void CreateBitmap(GlyphIndex glyphIndex, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const override;

  /**
   * @copydoc FontCacheItemInterface::IsColorGlyph()
   */
  bool IsColorGlyph(GlyphIndex glyphIndex) const override
  {
    return true;
  }

  /**
   * @copydoc FontCacheItemInterface::IsCharacterSupported()
   */
  bool IsCharacterSupported(Character character) override;

  /**
   * @copydoc FontCacheItemInterface::GetPointSize()
   */
  PointSize26Dot6 GetPointSize() const override
  {
    return TextAbstraction::FontClient::DEFAULT_POINT_SIZE;
  }

  /**
   * @copydoc FontCacheItemInterface::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(Character character) const override
  {
    return 0u;
  }

  /**
   * @copydoc FontCacheItemInterface::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(Character character, Character variantSelector) const override
  {
    return 0u;
  }

  /**
   * @copydoc FontCacheItemInterface::GetTypeface()
   */
  FT_Face GetTypeface() const override
  {
    return nullptr;
  }

  /**
   * @copydoc FontCacheItemInterface::GetHarfBuzzFont()
   */
  HarfBuzzFontHandle GetHarfBuzzFont(const uint32_t& horizontalDpi, const uint32_t& verticalDpi) override
  {
    return nullptr;
  }

  /**
   * @copydoc FontCacheItemInterface::HasItalicStyle()
   */
  bool HasItalicStyle() const override
  {
    return false;
  }

  BitmapFont                      font;         ///< The bitmap font.
  std::vector<Devel::PixelBuffer> pixelBuffers; ///< The pixel buffers of the glyphs.
  FontId                          id;           ///< Index to the vector with the cache of font's ids.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_INTERNAL_TEXT_ABSTRACTION_BITMAP_FONT_CACHE_ITEM_H
