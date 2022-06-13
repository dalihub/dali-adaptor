#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_CACHE_ITEM_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_CACHE_ITEM_H

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
#include <dali/internal/text/text-abstraction/plugin/font-cache-item-interface.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-glyph-cache-manager.h>
#include <dali/internal/text/text-abstraction/plugin/harfbuzz-proxy-font.h>

// EXTERNAL INCLUDES
#include <fontconfig/fontconfig.h>
#include <memory> // for std::unique_ptr

// EXTERNAL INCLUDES
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Caches the FreeType face and font metrics of the triplet 'path to the font file name, font point size and face index'.
 */
struct FontFaceCacheItem : public FontCacheItemInterface
{
  FontFaceCacheItem(const FT_Library&  freeTypeLibrary,
                    FT_Face            ftFace,
                    const FontPath&    path,
                    PointSize26Dot6    requestedPointSize,
                    FaceIndex          face,
                    const FontMetrics& metrics);

  FontFaceCacheItem(const FT_Library&  freeTypeLibrary,
                    FT_Face            ftFace,
                    const FontPath&    path,
                    PointSize26Dot6    requestedPointSize,
                    FaceIndex          face,
                    const FontMetrics& metrics,
                    int                fixedSizeIndex,
                    float              fixedWidth,
                    float              fixedHeight,
                    bool               hasColorTables);

  FontFaceCacheItem(const FontFaceCacheItem& rhs) = delete; // Do not use copy construct
  FontFaceCacheItem(FontFaceCacheItem&& rhs);

  ~FontFaceCacheItem();

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
  bool IsColorGlyph(GlyphIndex glyphIndex) const override;

  /**
   * @copydoc FontCacheItemInterface::IsCharacterSupported()
   */
  bool IsCharacterSupported(Character character) override;

  /**
   * @copydoc FontCacheItemInterface::GetPointSize()
   */
  PointSize26Dot6 GetPointSize() const override
  {
    return mRequestedPointSize;
  }

  /**
   * @copydoc FontCacheItemInterface::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(Character character) const override;

  /**
   * @copydoc FontCacheItemInterface::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(Character character, Character variantSelector) const override;

  /**
   * @copydoc FontCacheItemInterface::GetTypeface()
   */
  FT_Face GetTypeface() const override
  {
    return mFreeTypeFace;
  }

  /**
   * @copydoc FontCacheItemInterface::GetHarfBuzzFont()
   */
  HarfBuzzFontHandle GetHarfBuzzFont(const uint32_t& horizontalDpi, const uint32_t& verticalDpi) override;

  /**
   * @copydoc FontCacheItemInterface::HasItalicStyle()
   */
  bool HasItalicStyle() const override
  {
    return (0u != (mFreeTypeFace->style_flags & FT_STYLE_FLAG_ITALIC));
  }

public:
  const FT_Library& mFreeTypeLibrary; ///< A handle to a FreeType library instance.
  FT_Face           mFreeTypeFace;    ///< The FreeType face.

  std::unique_ptr<GlyphCacheManager> mGlyphCacheManager; ///< The glyph cache manager. It will cache this face's glyphs.
  std::unique_ptr<HarfBuzzProxyFont> mHarfBuzzProxyFont; ///< The harfbuzz font. It will store harfbuzz relate data.

  FontPath        mPath;                  ///< The path to the font file name.
  PointSize26Dot6 mRequestedPointSize;    ///< The font point size.
  FaceIndex       mFaceIndex;             ///< The face index.
  FontMetrics     mMetrics;               ///< The font metrics.
  _FcCharSet*     mCharacterSet;          ///< Pointer with the range of characters.
  int             mFixedSizeIndex;        ///< Index to the fixed size table for the requested size.
  float           mFixedWidthPixels;      ///< The height in pixels (fixed size bitmaps only)
  float           mFixedHeightPixels;     ///< The height in pixels (fixed size bitmaps only)
  unsigned int    mVectorFontId;          ///< The ID of the equivalent vector-based font
  FontId          mFontId;                ///< Index to the vector with the cache of font's ids.
  bool            mIsFixedSizeBitmap : 1; ///< Whether the font has fixed size bitmaps.
  bool            mHasColorTables : 1;    ///< Whether the font has color tables.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_CACHE_ITEM_H
