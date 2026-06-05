#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_CACHE_ITEM_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_CACHE_ITEM_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>
#include <dali/internal/text/text-abstraction/plugin/font-cache-item-interface.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-glyph-cache-manager.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-manager.h>
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
  /**
   * @brief Creates a cache item for a scalable FreeType face.
   *
   * The color font fields are cached table/renderability metadata and do not
   * rasterize COLRv1 glyphs during construction.
   *
   * @param[in] freeTypeLibrary A FreeType library handle.
   * @param[in] ftFace The FreeType face.
   * @param[in] fontFaceManager The owning font face manager.
   * @param[in] glyphCacheManager The glyph cache manager.
   * @param[in] colorGlyphColrRasterizer Non-owning COLRv1 rasterizer pointer.
   * @param[in] path The font file path.
   * @param[in] requestedPointSize The requested point size in 26.6 format.
   * @param[in] face The face index.
   * @param[in] metrics The font metrics.
   * @param[in] variationsHash The variation coordinate hash.
   * @param[in] freeTypeCoords FreeType variation coordinates.
   * @param[in] harfBuzzVariations HarfBuzz variation coordinates.
   * @param[in] hasColorTables Whether color font tables were detected.
   * @param[in] colorFontInfo Detected SFNT color table flags.
   * @param[in] colorFontRenderability Current-build color renderability classification.
   */
  FontFaceCacheItem(const FT_Library&                  freeTypeLibrary,
                    FT_Face                            ftFace,
                    FontFaceManager*                   fontFaceManager,
                    GlyphCacheManager*                 glyphCacheManager,
                    ColorGlyphColrRasterizer*          colorGlyphColrRasterizer,
                    const FontPath&                    path,
                    PointSize26Dot6                    requestedPointSize,
                    FaceIndex                          face,
                    const FontMetrics&                 metrics,
                    const std::size_t                  variationsHash,
                    const std::vector<FT_Fixed>&       freeTypeCoords,
                    const std::vector<hb_variation_t>& harfBuzzVariations,
                    bool                               hasColorTables = false,
                    const FontFaceManager::ColorFontInfo& colorFontInfo = FontFaceManager::ColorFontInfo{},
                    FontFaceManager::ColorFontRenderability colorFontRenderability = FontFaceManager::ColorFontRenderability::NotColorFont);

  /**
   * @brief Creates a cache item for a fixed-size bitmap FreeType face.
   *
   * Bitmap color fonts may be color-renderable through FT_LOAD_COLOR but are
   * not considered renderable COLRv1 fonts.
   *
   * @param[in] freeTypeLibrary A FreeType library handle.
   * @param[in] ftFace The FreeType face.
   * @param[in] fontFaceManager The owning font face manager.
   * @param[in] glyphCacheManager The glyph cache manager.
   * @param[in] colorGlyphColrRasterizer Non-owning COLRv1 rasterizer pointer.
   * @param[in] path The font file path.
   * @param[in] requestedPointSize The requested point size in 26.6 format.
   * @param[in] face The face index.
   * @param[in] metrics The font metrics.
   * @param[in] fixedSizeIndex The selected fixed-size strike index.
   * @param[in] fixedWidth The fixed strike width in pixels.
   * @param[in] fixedHeight The fixed strike height in pixels.
   * @param[in] hasColorTables Whether color font tables were detected.
   * @param[in] colorFontInfo Detected SFNT color table flags.
   * @param[in] colorFontRenderability Current-build color renderability classification.
   */
  FontFaceCacheItem(const FT_Library&  freeTypeLibrary,
                    FT_Face            ftFace,
                    FontFaceManager*   fontFaceManager,
                    GlyphCacheManager* glyphCacheManager,
                    ColorGlyphColrRasterizer* colorGlyphColrRasterizer,
                    const FontPath&    path,
                    PointSize26Dot6    requestedPointSize,
                    FaceIndex          face,
                    const FontMetrics& metrics,
                    int                fixedSizeIndex,
                    float              fixedWidth,
                    float              fixedHeight,
                    bool               hasColorTables,
                    const FontFaceManager::ColorFontInfo& colorFontInfo = FontFaceManager::ColorFontInfo{},
                    FontFaceManager::ColorFontRenderability colorFontRenderability = FontFaceManager::ColorFontRenderability::NotColorFont);

  FontFaceCacheItem(const FontFaceCacheItem& rhs) = delete; // Do not use copy construct
  FontFaceCacheItem(FontFaceCacheItem&& rhs) noexcept;

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
  void CreateBitmap(GlyphIndex glyphIndex, Dali::TextAbstraction::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const override;

  /**
   * @copydoc FontCacheItemInterface::IsColorGlyph()
   */
  bool IsColorGlyph(GlyphIndex glyphIndex) const override;

  /**
   * @copydoc FontCacheItemInterface::IsColorFont()
   */
  bool IsColorFont() const override;

  /**
   * @brief Checks whether this font is renderable by the COLRv1 renderer.
   *
   * RenderableBitmap fonts return false because they use FT_LOAD_COLOR rather
   * than the COLRv1 renderer.
   */
  bool IsRenderableColrV1Font() const override;

  /**
   * @brief Checks whether a glyph has a renderable COLRv1 root paint.
   *
   * This is a glyph-level capability query and must not rasterize the glyph.
   */
  bool IsRenderableColrV1Glyph(GlyphIndex glyphIndex) const override;

  /**
   * @copydoc FontCacheItemInterface::IsCharacterSupported()
   */
  bool IsCharacterSupported(FcConfig* fontConfig, Character character) override;

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

  FontFaceManager*                   mFontFaceManager;        ///< The reference of font face manager. Owned from font-client-plugin-cache-handler.
  GlyphCacheManager*                 mGlyphCacheManager;      ///< The reference of Glyph cache manager. Owned from font-client-plugin-cache-handler.
  ColorGlyphColrRasterizer*          mColorGlyphColrRasterizer; ///< Non-owning pointer to COLRv1 rasterizer. Owned by CacheHandler.
  std::unique_ptr<HarfBuzzProxyFont> mHarfBuzzProxyFont;      ///< The harfbuzz font. It will store harfbuzz relate data.

  FontPath                    mPath;                  ///< The path to the font file name.
  PointSize26Dot6             mRequestedPointSize;    ///< The font point size.
  FaceIndex                   mFaceIndex;             ///< The face index.
  FontMetrics                 mMetrics;               ///< The font metrics.
  _FcCharSet*                 mCharacterSet;          ///< Pointer with the range of characters.
  int                         mFixedSizeIndex;        ///< Index to the fixed size table for the requested size.
  float                       mFixedWidthPixels;      ///< The height in pixels (fixed size bitmaps only)
  float                       mFixedHeightPixels;     ///< The height in pixels (fixed size bitmaps only)
  unsigned int                mVectorFontId;          ///< The ID of the equivalent vector-based font
  FontId                      mFontId;                ///< Index to the vector with the cache of font's ids.
  bool                        mIsFixedSizeBitmap : 1; ///< Whether the font has fixed size bitmaps.
  bool                        mHasColorTables : 1;    ///< Whether any color font tables were detected.
  FontFaceManager::ColorFontInfo           mColorFontInfo;          ///< Detected SFNT color table flags.
  FontFaceManager::ColorFontRenderability  mColorFontRenderability; ///< Current-build color renderability classification.
  std::size_t                 mVariationsHash;        ///< The hash of the variations to use key.
  std::vector<FT_Fixed>       mFreeTypeCoords;        ///< The FreeType coordinates for the variations.
  std::vector<hb_variation_t> mHarfBuzzVariations;    ///< The HarfBuzz variations data.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_CACHE_ITEM_H
