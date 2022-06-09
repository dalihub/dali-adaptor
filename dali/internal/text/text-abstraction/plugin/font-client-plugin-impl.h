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
private:
  /// Redefine FontId name to specifiy the value's usage
  using FontCacheIndex = FontId;

  /**
   * @brief Index of FontCache container.
   */
  struct FontIdCacheItem
  {
    FontDescription::Type type;  ///< The type of font.
    FontCacheIndex        index; ///< Index to the cache of fonts for the specified type. Face or Bitmap
  };

  /**
   * @brief Caches an list of fallback fonts for a given font-description
   */
  struct FallbackCacheItem
  {
    FallbackCacheItem(FontDescription&& fontDescription, FontList* fallbackFonts, CharacterSetList* characterSets);

    FontDescription   fontDescription; ///< The font description.
    FontList*         fallbackFonts;   ///< The list of fallback fonts for the given font-description.
    CharacterSetList* characterSets;   ///< The list of character sets for the given font-description.
  };

  /**
   * @brief Caches an index to the vector of font descriptions for a given font.
   */
  struct FontDescriptionCacheItem
  {
    FontDescriptionCacheItem(const FontDescription& fontDescription,
                             FontDescriptionId      index);
    FontDescriptionCacheItem(FontDescription&& fontDescription,
                             FontDescriptionId index);

    FontDescription   fontDescription; ///< The font description.
    FontDescriptionId index;           ///< Index to the vector of font descriptions.
  };

  /**
   * @brief Pair of FontDescriptionId and PointSize. It will be used to find cached validate font.
   */
  struct FontDescriptionSizeCacheKey
  {
    FontDescriptionSizeCacheKey(FontDescriptionId fontDescriptionId,
                                PointSize26Dot6   requestedPointSize);

    FontDescriptionId fontDescriptionId;  ///< Index to the vector with font descriptions.
    PointSize26Dot6   requestedPointSize; ///< The font point size.

    bool operator==(FontDescriptionSizeCacheKey const& rhs) const noexcept
    {
      return fontDescriptionId == rhs.fontDescriptionId && requestedPointSize == rhs.requestedPointSize;
    }
  };

  /**
   * @brief Custom hash functions for FontDescriptionSizeCacheKey.
   */
  struct FontDescriptionSizeCacheKeyHash
  {
    std::size_t operator()(FontDescriptionSizeCacheKey const& key) const noexcept
    {
      return key.fontDescriptionId ^ key.requestedPointSize;
    }
  };

  /**
   * @brief Caches the font id of the pair font point size and the index to the vector of font descriptions of validated fonts.
   */
  using FontDescriptionSizeCacheContainer = std::unordered_map<FontDescriptionSizeCacheKey, FontCacheIndex, FontDescriptionSizeCacheKeyHash>;

  struct EllipsisItem
  {
    PointSize26Dot6 requestedPointSize;
    GlyphInfo       glyph;
  };

public:
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
  void ClearCache();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDpi()
   */
  void SetDpi(unsigned int horizontalDpi, unsigned int verticalDpi);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::ResetSystemDefaults()
   */
  void ResetSystemDefaults();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDefaultFont()
   */
  void SetDefaultFont(const FontDescription& preferredFontDescription);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultPlatformFontDescription()
   */
  void GetDefaultPlatformFontDescription(FontDescription& fontDescription);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultFonts()
   */
  void GetDefaultFonts(FontList& defaultFonts);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetSystemFonts()
   */
  void GetSystemFonts(FontList& systemFonts);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDescription()
   */
  void GetDescription(FontId id, FontDescription& fontDescription) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetPointSize()
   */
  PointSize26Dot6 GetPointSize(FontId id);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsCharacterSupportedByFont()
   */
  bool IsCharacterSupportedByFont(FontId fontId, Character character);

  /**
   * Get the cached font item for the given font
   * @param[in] id The font id to search for
   * @return the matching cached font item
   */
  const FontCacheItemInterface* GetCachedFontItem(FontId id) const;

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
                              bool                    preferColor);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::FindDefaultFont()
   */
  FontId FindDefaultFont(Character       charcode,
                         PointSize26Dot6 requestedPointSize,
                         bool            preferColor);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::FindFallbackFont()
   */
  FontId FindFallbackFont(Character              charcode,
                          const FontDescription& preferredFontDescription,
                          PointSize26Dot6        requestedPointSize,
                          bool                   preferColor);

  /**
   * @see Dali::TextAbstraction::FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   *
   * @param[in] cacheDescription Whether to cache the font description.
   */
  FontId GetFontId(const FontPath& path,
                   PointSize26Dot6 requestedPointSize,
                   FaceIndex       faceIndex,
                   bool            cacheDescription);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontDescription& preferredFontDescription, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   */
  FontId GetFontId(const FontDescription& fontDescription,
                   PointSize26Dot6        requestedPointSize,
                   FaceIndex              faceIndex);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const BitmapFont& bitmapFont )
   */
  FontId GetFontId(const BitmapFont& bitmapFont);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsScalable( const FontPath& path )
   */
  bool IsScalable(const FontPath& path);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsScalable( const FontDescription& fontDescription )
   */
  bool IsScalable(const FontDescription& fontDescription);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes()
   */
  void GetFixedSizes(const FontPath& path, Dali::Vector<PointSize26Dot6>& sizes);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes()
   */
  void GetFixedSizes(const FontDescription&         fontDescription,
                     Dali::Vector<PointSize26Dot6>& sizes);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::HasItalicStyle()
   */
  bool HasItalicStyle(FontId fontId) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontMetrics()
   */
  void GetFontMetrics(FontId fontId, FontMetrics& metrics);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(FontId fontId, Character charcode);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex(FontId fontId, Character charcode, Character variantSelector);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphMetrics()
   */
  bool GetGlyphMetrics(GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal);

  /**
   * Helper for GetGlyphMetrics when using bitmaps
   */
  bool GetBitmapMetrics(GlyphInfo* array, uint32_t size, bool horizontal);

  /**
   * Helper for GetGlyphMetrics when using vectors
   */
  bool GetVectorMetrics(GlyphInfo* array, uint32_t size, bool horizontal);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth )
   */
  void CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
   */
  PixelData CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateVectorBlob()
   */
  void CreateVectorBlob(FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetEllipsisGlyph()
   */
  const GlyphInfo& GetEllipsisGlyph(PointSize26Dot6 requestedPointSize);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsColorGlyph()
   */
  bool IsColorGlyph(FontId fontId, GlyphIndex glyphIndex);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateEmbeddedItem()
   */
  GlyphIndex CreateEmbeddedItem(const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat);

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
   * @copydoc Dali::TextAbstraction::Internal::FontClient::GetFreetypeFace()
   */
  FT_FaceRec_* GetFreetypeFace(FontId fontId);

  /**
   * @copydoc Dali::TextAbstraction::Internal::FontClient::GetFontType()
   */
  FontDescription::Type GetFontType(FontId fontId);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::AddCustomFontDirectory()
   */
  bool AddCustomFontDirectory(const FontPath& path);

private:
  /**
   * @brief Caches the fonts present in the platform.
   *
   * Calls GetFcFontSet() to retrieve the fonts.
   */
  void InitSystemFonts();

  /**
   * @brief Gets the FontDescription which matches the given pattern.
   *
   * @note The reference counter of the @p characterSet has been increased. Call FcCharSetDestroy to decrease it.
   *
   * @param[in] pattern pattern to match against.
   * @param[out] fontDescription the resultant fontDescription that matched.
   * @param[out] characterSet The character set for that pattern.
   * @return true if match found.
   */
  bool MatchFontDescriptionToPattern(_FcPattern* pattern, Dali::TextAbstraction::FontDescription& fontDescription, _FcCharSet** characterSet);

  /**
   * @brief Retrieves the fonts present in the platform.
   *
   * @note Need to call FcFontSetDestroy to free the allocated resources.
   *
   * @return A font fonfig data structure with the platform's fonts.
   */
  _FcFontSet* GetFcFontSet() const;

  /**
   * @brief Retrieves a font config object's value from a pattern.
   *
   * @param[in] pattern The font config pattern.
   * @param[in] n The object.
   * @param[out] string The object's value.
   *
   * @return @e true if the operation is successful.
   */
  bool GetFcString(const _FcPattern* const pattern, const char* const n, std::string& string);

  /**
   * @brief Retrieves a font config object's value from a pattern.
   *
   * @param[in] pattern The font config pattern.
   * @param[in] n The object.
   * @param[out] intVal The object's value.
   *
   * @return @e true if the operation is successful.
   */
  bool GetFcInt(const _FcPattern* const pattern, const char* const n, int& intVal);

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
                    bool            cacheDescription);

  /**
   * @brief Finds in the cache if there is a triplet with the path to the font file name, the font point size and the face index.
   * If there is one , if writes the font identifier in the param @p fontId.
   *
   * @param[in] path Path to the font file name.
   * @param[in] requestedPointSize The font point size.
   * @param[in] faceIndex The face index.
   * @param[out] fontId The font identifier.
   *
   * @return @e true if there triplet is found.
   */
  bool FindFont(const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex, FontId& fontId) const;

  /**
   * @brief Finds in the cache a cluster 'font family, font width, font weight, font slant'
   * If there is one, it writes the index to the vector with font descriptions in the param @p validatedFontId.
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] validatedFontId The index to the vector with font descriptions.
   *
   * @return @e true if the pair is found.
   */
  bool FindValidatedFont(const FontDescription& fontDescription,
                         FontDescriptionId&     validatedFontId);

  /**
   * @brief Finds a fallback font list from the cache for a given font-description
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] A valid pointer to a font list, or @e nullptr if not found.
   * @param[out] characterSetList A valid pointer to a character set list, or @e nullptr if not found.
   */
  bool FindFallbackFontList(const FontDescription& fontDescription,
                            FontList*&             fontList,
                            CharacterSetList*&     characterSetList);

  /**
   * @brief Finds in the cache a pair 'validated font identifier and font point size'.
   * If there is one it writes the font identifier in the param @p fontCacheIndex.
   *
   * @param[in] validatedFontId Index to the vector with font descriptions.
   * @param[in] requestedPointSize The font point size.
   * @param[out] fontCacheIndex The index of font cache identifier.
   *
   * @return @e true if the pair is found.
   */
  bool FindFont(FontDescriptionId validatedFontId,
                PointSize26Dot6   requestedPointSize,
                FontCacheIndex&   fontCacheIndex);

  /**
   * @brief Finds in the cache a bitmap font with the @p bitmapFont family name.
   *
   * @param[in] bitmapFont The font's family name.
   * @param[out] fontId The id of the font.
   *
   * @return Whether the font has been found.
   */
  bool FindBitmapFont(const FontFamily& bitmapFont, FontId& fontId) const;

  /**
   * @brief Validate a font description.
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] validatedFontId Result of validation
   */
  void ValidateFont(const FontDescription& fontDescription,
                    FontDescriptionId&     validatedFontId);

  /**
   * @brief Helper for GetDefaultFonts etc.
   *
   * @note CharacterSetList is a vector of FcCharSet that are reference counted. It's needed to call FcCharSetDestroy to decrease the reference counter.
   *
   * @param[in] fontDescription A font description.
   * @param[out] fontList A list of the fonts which are a close match for fontDescription.
   * @param[out] characterSetList A list of character sets which are a close match for fontDescription.
   */
  void SetFontList(const FontDescription& fontDescription, FontList& fontList, CharacterSetList& characterSetList);

  /**
   * Caches a font path.
   *
   * @param[in] ftFace The FreeType face.
   * @param[in] id The font identifier.
   * @param[in] requestedPointSize The font point size.
   * @param[in] path Path to the font file name.
   */
  void CacheFontPath(FT_Face ftFace, FontId id, PointSize26Dot6 requestedPointSize, const FontPath& path);

  /**
   * @brief Free the resources allocated in the fallback cache.
   *
   * @param[in] fallbackCache The fallback cache.
   */
  void ClearFallbackCache(std::vector<FallbackCacheItem>& fallbackCache);

  /**
   * @brief Free the resources allocated by the FcCharSet objects.
   */
  void ClearCharacterSetFromFontFaceCache();

private:
  Plugin(const Plugin&) = delete;
  Plugin& operator=(const Plugin&) = delete;

private:
  FT_Library mFreeTypeLibrary; ///< A handle to a FreeType library instance.

  unsigned int mDpiHorizontal; ///< Horizontal dpi.
  unsigned int mDpiVertical;   ///< Vertical dpi.

  FontDescription mDefaultFontDescription; ///< The cached default font from the system

  FontList         mSystemFonts;  ///< Cached system fonts.
  FontList         mDefaultFonts; ///< Cached default fonts.
  CharacterSetList mDefaultFontCharacterSets;

  std::vector<FallbackCacheItem> mFallbackCache; ///< Cached fallback font lists.

  Vector<FontIdCacheItem>               mFontIdCache;          ///< Caches from FontId to FontCacheIndex.
  std::vector<FontFaceCacheItem>        mFontFaceCache;        ///< Caches the FreeType face and font metrics of the triplet 'path to the font file name, font point size and face index'.
  std::vector<FontDescriptionCacheItem> mValidatedFontCache;   ///< Caches indices to the vector of font descriptions for a given font.
  FontList                              mFontDescriptionCache; ///< Caches font descriptions for the validated font.
  CharacterSetList                      mCharacterSetCache;    ///< Caches character set lists for the validated font.

  FontDescriptionSizeCacheContainer mFontDescriptionSizeCache; ///< Caches font identifiers for the pairs of font point size and the index to the vector with font descriptions of the validated fonts.

  VectorFontCache* mVectorFontCache; ///< Separate cache for vector data blobs etc.

  Vector<EllipsisItem>              mEllipsisCache;     ///< Caches ellipsis glyphs for a particular point size.
  std::vector<PixelBufferCacheItem> mPixelBufferCache;  ///< Caches the pixel buffer of a url.
  Vector<EmbeddedItem>              mEmbeddedItemCache; ///< Cache embedded items.
  std::vector<BitmapFontCacheItem>  mBitmapFontCache;   ///< Stores bitmap fonts.

  bool mDefaultFontDescriptionCached : 1; ///< Whether the default font is cached or not

  bool    mIsAtlasLimitationEnabled : 1;      ///< Whether the validation on maximum atlas block size, then reduce block size to fit into it is enabled or not.
  Vector2 mCurrentMaximumBlockSizeFitInAtlas; ///< The current maximum size (width, height) of text-atlas-block.
};

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H
