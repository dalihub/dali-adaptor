#ifndef DALI_TEST_ABSTRACTION_INTERNAL_FONT_CACHE_ITEM_INTERFACE_H
#define DALI_TEST_ABSTRACTION_INTERNAL_FONT_CACHE_ITEM_INTERFACE_H

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

#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/text-abstraction/font-metrics.h>
#include <dali/devel-api/text-abstraction/glyph-info.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Dali::TextAbstraction::Internal
{
struct FontCacheItemInterface
{
  /**
   * Get the font metrics
   *
   * @param[out] metrics The metrics struct to fill out
   */
  virtual void GetFontMetrics(FontMetrics& metrics, unsigned int dpiVertical) const = 0;

  /**
   * Get glyph metrics
   *
   * @param[in,out] glyph The glyph to fill
   */
  virtual bool GetGlyphMetrics(GlyphInfo& glyphInfo, unsigned int dpiVertical, bool horizontal) const = 0;

  /**
   * Create a bitmap for the given glyph
   *
   * @param[in] glyphIndex The index of the glyph
   * @param[out] data The bitmap data for the glyph
   * @param[in] outlineWidth
   * @param[in] isItalicRequired
   * @param[in] isBoldRequired
   */
  virtual void CreateBitmap(GlyphIndex glyphIndex, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const = 0;

  /**
   * Return true if the glyph is colored
   *
   * @param[in] glyphIndex The index of the glyph
   * @return true if the glyph is colored
   */
  virtual bool IsColorGlyph(GlyphIndex glyphIndex) const = 0;

  /**
   * Check if the character is supported by this font
   * @param[in] character The character to test
   */
  virtual bool IsCharacterSupported(Character character) = 0;

  /**
   * Get the point size of this font
   * @return the point size
   */
  virtual PointSize26Dot6 GetPointSize() const = 0;

  /**
   * Get the index into this font's glyph table of the character
   *
   * @param[in] character to look up
   * @return the glyph index of this character
   */
  virtual GlyphIndex GetGlyphIndex(Character character) const = 0;

  /**
   * Get the index into this font's glyph table of the character
   *
   * @param[in] character to look up
   * @param[in] variantSelector the variation selector modify character code
   * @return the glyph index of this character
   */
  virtual GlyphIndex GetGlyphIndex(Character character, Character variantSelector) const = 0;

  /**
   * Get the freetype typeface for this font.
   */
  virtual FT_Face GetTypeface() const = 0;

  /**
   * @return true if this font has an italic style
   */
  virtual bool HasItalicStyle() const = 0;
};

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEST_ABSTRACTION_INTERNAL_FONT_CACHE_ITEM_INTERFACE_H
