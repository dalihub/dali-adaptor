#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/devel-api/text-abstraction/bitmap-font.h>
#include <dali/devel-api/text-abstraction/font-metrics.h>
#include <dali/devel-api/text-abstraction/glyph-info.h>
#include <dali/internal/text/text-abstraction/font-client-impl.h>
#include <dali/internal/text/text-abstraction/plugin/bitmap-font-cache-item.h>
#include <dali/internal/text/text-abstraction/plugin/embedded-item.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-cache-item.h>
#include <dali/internal/text/text-abstraction/plugin/pixel-buffer-cache-item.h>

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
#include <third-party/glyphy/vector-font-cache.h>
#else
class VectorFontCache;
#endif

// EXTERNAL INCLUDES
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H

// forward declarations of font config types.
struct _FcCharSet;
struct _FcFontSet;
struct _FcPattern;

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
/**
 * @brief Type used for indices addressing the vector with front descriptions of validated fonts.
 */
typedef uint32_t FontDescriptionId;

/**
 * @brief Vector of character sets.
 */
typedef Vector<_FcCharSet*> CharacterSetList;

/**
 * @brief FontClient implementation.
 */
struct FontClient::Plugin
{
public: // Dali::TextAbstraction::FontClient
  /**
   * Constructor.
   *
   * Initializes the FreeType library.
   * Initializes the dpi values.
   *
   * @param[in] horizontalDpi The horizontal dpi.
   * @param[in] verticalDpi The vertical dpi.
   */
  Plugin(unsigned int horizontalDpi, unsigned int verticalDpi);

  /**
   * Default destructor.
   *
   * Frees any allocated resource.
   */
  ~Plugin();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::ClearCache()
   */
  void ClearCache() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDpi()
   */
  void SetDpi(unsigned int horizontalDpi, unsigned int verticalDpi);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::ResetSystemDefaults()
   */
  void ResetSystemDefaults() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultPlatformFontDescription()
   */
  void GetDefaultPlatformFontDescription(FontDescription& fontDescription) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultFonts()
   */
  void GetDefaultFonts(FontList& defaultFonts) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetSystemFonts()
   */
  void GetSystemFonts(FontList& systemFonts) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDescription()
   */
  void GetDescription(FontId fontId, FontDescription& fontDescription) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetPointSize()
   */
  PointSize26Dot6 GetPointSize(FontId fontId) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsCharacterSupportedByFont()
   */
  bool IsCharacterSupportedByFont(FontId fontId, Character character) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::FindDefaultFont()
   */
  FontId FindDefaultFont(Character       charcode,
                         PointSize26Dot6 requestedPointSize,
                         bool            preferColor) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::FindFallbackFont()
   */
  FontId FindFallbackFont(Character              charcode,
                          const FontDescription& preferredFontDescription,
                          PointSize26Dot6        requestedPointSize,
                          bool                   preferColor) const;

  /**
   * @see Dali::TextAbstraction::FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   *
   * @param[in] cacheDescription Whether to cache the font description.
   */
  FontId GetFontIdByPath(const FontPath& path,
                         PointSize26Dot6 requestedPointSize,
                         FaceIndex       faceIndex,
                         bool            cacheDescription) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontDescription& preferredFontDescription, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   */
  FontId GetFontId(const FontDescription& fontDescription,
                   PointSize26Dot6        requestedPointSize,
                   FaceIndex              faceIndex) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const BitmapFont& bitmapFont )
   */
  FontId GetFontId(const BitmapFont& bitmapFont) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsScalable( const FontPath& path )
   */
  bool IsScalable(const FontPath& path) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsScalable( const FontDescription& fontDescription )
   */
  bool IsScalable(const FontDescription& fontDescription) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes()
   */
  void GetFixedSizes(const FontPath& path, Dali::Vector<PointSize26Dot6>& sizes) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes()
   */
  void GetFixedSizes(const FontDescription&         fontDescription,
                     Dali::Vector<PointSize26Dot6>& sizes) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::HasItalicStyle()
   */
  bool HasItalicStyle(FontId fontId) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontMetrics()
   */
  void GetFontMetrics(FontId fontId, FontMetrics& metrics) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(FontId fontId, Character charcode) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(FontId fontId, Character charcode, Character variantSelector) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphMetrics()
   */
  bool GetGlyphMetrics(GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal) const;

  /**
   * Helper for GetGlyphMetrics when using bitmaps
   */
  bool GetBitmapMetrics(GlyphInfo* array, uint32_t size, bool horizontal) const;

  /**
   * Helper for GetGlyphMetrics when using vectors
   */
  bool GetVectorMetrics(GlyphInfo* array, uint32_t size, bool horizontal) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth )
   */
  void CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
   */
  PixelData CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateVectorBlob()
   */
  void CreateVectorBlob(FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetEllipsisGlyph()
   */
  const GlyphInfo& GetEllipsisGlyph(PointSize26Dot6 requestedPointSize) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsColorGlyph()
   */
  bool IsColorGlyph(FontId fontId, GlyphIndex glyphIndex) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateEmbeddedItem()
   */
  GlyphIndex CreateEmbeddedItem(const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::EnableAtlasLimitation(bool enabled)
   */
  void EnableAtlasLimitation(bool enabled);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsAtlasLimitationEnabled()
   */
  bool IsAtlasLimitationEnabled() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetMaximumTextAtlasSize()
   */
  Size GetMaximumTextAtlasSize() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultTextAtlasSize()
   */
  Size GetDefaultTextAtlasSize() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetCurrentMaximumBlockSizeFitInAtlas()
   */
  Size GetCurrentMaximumBlockSizeFitInAtlas() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetCurrentMaximumBlockSizeFitInAtlas(const Size& currentMaximumBlockSizeFitInAtlas)
   */
  bool SetCurrentMaximumBlockSizeFitInAtlas(const Size& currentMaximumBlockSizeFitInAtlas);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetNumberOfPointsPerOneUnitOfPointSize()
   */
  uint32_t GetNumberOfPointsPerOneUnitOfPointSize() const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::AddCustomFontDirectory()
   */
  bool AddCustomFontDirectory(const FontPath& path);

public: // Dali::TextAbstraction::Internal::FontClient
  /**
   * @copydoc Dali::TextAbstraction::Internal::FontClient::GetFreetypeFace()
   */
  FT_FaceRec_* GetFreetypeFace(FontId fontId) const;

  /**
   * @copydoc Dali::TextAbstraction::Internal::FontClient::GetFontType()
   */
  FontDescription::Type GetFontType(FontId fontId) const;

  /**
   * @copydoc Dali::TextAbstraction::Internal::FontClient::GetHarfBuzzFont()
   */
  HarfBuzzFontHandle GetHarfBuzzFont(FontId fontId) const;

  /**
   * @copydoc Dali::TextAbstraction::Internal::FontClient::FontPreCache()
   */
  void FontPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily) const;

private:
  /**
   * Get the cached font item for the given font
   * @param[in] fontId The font id to search for
   * @return the matching cached font item
   */
  const FontCacheItemInterface* GetCachedFontItem(FontId fontId) const;

  /**
   * @brief Finds within the @p fontList a font which support the @p carcode.
   *
   * @param[in] fontList A list of font paths, family, width, weight and slant.
   * @param[in] characterSetList A list that contains a character set for each description of the font list.
   * @param[in] charcode The character for which a font is needed.
   * @param[in] requestedPointSize The point size in 26.6 fractional points.
   * @param[in] preferColor @e true if a color font is preferred.
   *
   * @return A valid font identifier, or zero if no font is found.
   */
  FontId FindFontForCharacter(const FontList&         fontList,
                              const CharacterSetList& characterSetList,
                              Character               charcode,
                              PointSize26Dot6         requestedPointSize,
                              bool                    preferColor) const;

  /**
   * @brief Creates a font.
   *
   * @param[in] path The path to the font file name.
   * @param[in] requestedPointSize The requested point size.
   * @param[in] faceIndex A face index.
   * @param[in] cacheDescription Whether to cache the font description.
   *
   * @return The font identifier.
   */
  FontId CreateFont(const FontPath& path,
                    PointSize26Dot6 requestedPointSize,
                    FaceIndex       faceIndex,
                    bool            cacheDescription) const;

private:
  Plugin(const Plugin&) = delete;
  Plugin& operator=(const Plugin&) = delete;

private:
  FT_Library mFreeTypeLibrary; ///< A handle to a FreeType library instance.

  unsigned int mDpiHorizontal; ///< Horizontal dpi.
  unsigned int mDpiVertical;   ///< Vertical dpi.

  bool    mIsAtlasLimitationEnabled : 1;      ///< Whether the validation on maximum atlas block size, then reduce block size to fit into it is enabled or not.
  Vector2 mCurrentMaximumBlockSizeFitInAtlas; ///< The current maximum size (width, height) of text-atlas-block.

private:
  VectorFontCache* mVectorFontCache; ///< Separate cache for vector data blobs etc.

  struct CacheHandler;
  CacheHandler* mCacheHandler; ///< Seperate cache for font data.
};

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H
