#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_CACHE_HANDLER_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_CACHE_HANDLER_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-manager.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-glyph-cache-manager.h>

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief FontClient Plugin cache item handler.
 */
struct FontClient::Plugin::CacheHandler
{
public:
  /**
   * Constructor.
   */
  CacheHandler();

  /**
   * Default destructor.
   *
   * Frees any allocated resource.
   */
  ~CacheHandler();

public: // Public struct
  /// Redefine FontId name to specifiy the value's usage
  using FontCacheIndex     = FontId;
  using EllipsisCacheIndex = FontId;

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
   * @brief Caches an glyph informations of ellipsis character per each point size.
   */
  struct EllipsisItem
  {
    PointSize26Dot6    requestedPointSize;
    EllipsisCacheIndex index;
    GlyphInfo          glyph;
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
    FontDescriptionSizeCacheKey() = default;

    FontDescriptionSizeCacheKey(FontDescriptionId fontDescriptionId,
                                PointSize26Dot6   requestedPointSize,
                                std::size_t       variationsMapHash);

    FontDescriptionId fontDescriptionId;  ///< Index to the vector with font descriptions.
    PointSize26Dot6   requestedPointSize; ///< The font point size.
    std::size_t       variationsMapHash;

    bool operator==(FontDescriptionSizeCacheKey const& rhs) const noexcept
    {
      return fontDescriptionId == rhs.fontDescriptionId && requestedPointSize == rhs.requestedPointSize && variationsMapHash == rhs.variationsMapHash;
    }
  };

  /**
   * @brief Custom hash functions for FontDescriptionSizeCacheKey.
   */
  struct FontDescriptionSizeCacheKeyHash
  {
    std::size_t operator()(FontDescriptionSizeCacheKey const& key) const noexcept
    {
      return key.fontDescriptionId ^ key.requestedPointSize ^ key.variationsMapHash;
    }
  };

  /**
   * @brief Caches the font id of the pair font point size and the index to the vector of font descriptions of validated fonts.
   */
  using FontDescriptionSizeCacheContainer = std::unordered_map<FontDescriptionSizeCacheKey, FontCacheIndex, FontDescriptionSizeCacheKeyHash>;

public: // Clear cache public
  /**
   * @copydoc Dali::TextAbstraction::FontClient::Plugin::ClearCache()
   */
  void ClearCache();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::Plugin::ClearCacheOnLocaleChanged()
   */
  void ClearCacheOnLocaleChanged();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::Plugin::ResetSystemDefaults()
   */
  void ResetSystemDefaults();

private: // Clear cache private
  /**
   * @brief Free the resources allocated by the FcCharSet objects.
   */
  void ClearCharacterSetFromFontFaceCache();

  /**
   * @brief Free the resources allocated in the fallback cache.
   */
  void ClearFallbackCache();

  /**
   * @brief Free the resources allocated in charset cache.
   */
  void ClearCharacterSet();

private:
  /**
   * @brief Crate the charset resouces by default font and Fallback caches.
   * @pre We should call this API only one times after ClearCharacterSet().
   */
  void CreateCharacterSet();

public: // Find & Cache
  // System / Default

  /**
   * @brief Caches the fonts present in the platform.
   *
   * Calls GetFcFontSet() to retrieve the fonts.
   */
  void InitSystemFonts();

  /**
   * @brief Retrieve the list of default fonts supported by the system.
   */
  void InitDefaultFonts();

  /**
   * @brief Retrieve the active default font from the system.
   */
  void InitDefaultFontDescription();

  // Validate

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
   * @brief Validate a font description.
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] fontDescriptionId Result of validation
   */
  void ValidateFont(const FontDescription& fontDescription,
                    FontDescriptionId&     fontDescriptionId);

  /**
   * @brief Cache in the descrption and validate id information
   * @note We use std::move operation to fontDescription.
   *
   * @param[in] fontDescription The font to validate.
   * @param[in] validatedFontId The index to the vector with font descriptions.
   */
  void CacheValidateFont(FontDescription&& fontDescription,
                         FontDescriptionId validatedFontId);

  // Fallback

  /**
   * @brief Finds a fallback font list from the cache for a given font-description
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] fontList A valid pointer to a font list, or @e nullptr if not found.
   * @param[out] characterSetList A valid pointer to a character set list, or @e nullptr if not found.
   *
   * @return Whether the fallback font list has been found.
   */
  bool FindFallbackFontList(const FontDescription& fontDescription,
                            FontList*&             fontList,
                            CharacterSetList*&     characterSetList) const;

  /**
   * @brief Cache a fallback font list for a given font-description
   * @note We use std::move operation to fontDescription.
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] fontListA valid pointer to a font list.
   * @param[out] characterSetList A valid pointer to a character set list.
   */
  void CacheFallbackFontList(FontDescription&&  fontDescription,
                             FontList*&         fontList,
                             CharacterSetList*& characterSetList);

  // Font / FontFace

  /**
   * @brief Finds in the cache if there is a triplet with the path to the font file name, the font point size and the face index.
   * If there is one , if writes the font identifier in the param @p fontId.
   *
   * @param[in] path Path to the font file name.
   * @param[in] requestedPointSize The font point size.
   * @param[in] faceIndex The face index.
   * @param[in] variationsMapPtr The variations used in variable fonts.
   * @param[out] fontId The font identifier.
   *
   * @return @e true if there triplet is found.
   */
  bool FindFontByPath(const FontPath& path,
                      PointSize26Dot6 requestedPointSize,
                      FaceIndex       faceIndex,
                      Property::Map*  variationsMapPtr,
                      FontId&         fontId) const;

  /**
   * @brief Finds in the cache a pair 'validated font identifier and font point size'.
   * If there is one it writes the font identifier in the param @p fontCacheIndex.
   *
   * @param[in] validatedFontId Index to the vector with font descriptions.
   * @param[in] requestedPointSize The font point size.
   * @param[out] fontCacheIndex The index of font cache identifier.
   * @param[in] variationsMapPtr The variations used in variable fonts.
   *
   * @return @e true if the pair is found.
   */
  bool FindFont(FontDescriptionId validatedFontId,
                PointSize26Dot6   requestedPointSize,
                FontCacheIndex&   fontCacheIndex,
                Property::Map*    variationsMapPtr);

  /**
   * @brief Cache the font descpription size item.
   *
   * @param[in] fontDescriptionId FontDescriptionId of current font.
   * @param[in] requestedPointSize Size of current font.
   * @param[in] variationsMapPtr The variations used in variable fonts.
   * @param[in] fontCacheIndex Index of this font's cache.
   */
  void CacheFontDescriptionSize(FontDescriptionId fontDescriptionId, PointSize26Dot6 requestedPointSize, Property::Map* variationsMapPtr, FontCacheIndex fontCacheIndex);

  /**
   * @brief Cache the font face cache item.
   * @note We use std::move operation to cache item.
   *
   * @param[in] fontFaceCacheItem Font face cache item.
   * @return FontId of newly inserted font cache item.
   */
  FontId CacheFontFaceCacheItem(FontFaceCacheItem&& fontFaceCacheItem);

  /**
   * @brief Caches a font path.
   *
   * @param[in] ftFace The FreeType face.
   * @param[in] fontId The font identifier.
   * @param[in] requestedPointSize The font point size.
   * @param[in] variationsMapPtr The variations used in variable fonts.
   * @param[in] path Path to the font file name.
   */
  void CacheFontPath(FT_Face ftFace, FontId fontId, PointSize26Dot6 requestedPointSize, Property::Map* variationsMapPtr, const FontPath& path);

  // Ellipsis

  /**
   * @brief Finds an ellipsis cache for a given point size
   *
   * @param[in] requestedPointSize Requested point size.
   * @param[out] ellipsisCacheIndex The index of cached ellipsis.
   *
   * @return Whether the ellipsis has been found.
   */
  bool FindEllipsis(PointSize26Dot6 requestedPointSize, EllipsisCacheIndex& ellipsisCacheIndex) const;

  /**
   * @brief Cache an ellipsis item
   * @note We use std::move operation to cache item.
   *
   * @param[in] ellipsisItem Ellipsis item.
   * @return The index of cached ellipsis.
   */
  EllipsisCacheIndex CacheEllipsis(EllipsisItem&& ellipsisItem);

  // Bitmap font

  /**
   * @brief Finds in the cache a bitmap font with the @p bitmapFont family name.
   *
   * @param[in] bitmapFontFamily The font's family name.
   * @param[out] fontId The id of the font.
   *
   * @return Whether the font has been found.
   */
  bool FindBitmapFont(const FontFamily& bitmapFontFamily, FontId& fontId) const;

  /**
   * @brief Cache the bitmap font cache item.
   * @note We use std::move operation to cache item.
   *
   * @param[in] bitmapFontCacheItem Bitmap font cache item.
   * @return FontId of newly inserted font cache item.
   */
  FontId CacheBitmapFontCacheItem(BitmapFontCacheItem&& bitmapFontCacheItem);

  // Embedded

  /**
   * @brief Finds in the cache a pixel buffer for embedded font.
   *
   * @param[in] url The embedded image's url.
   * @param[out] pixelBufferId The id of the loaded pixel buffer.
   *
   * @return Whether the embedded pixel buffer has been found.
   */
  bool FindEmbeddedPixelBufferId(const std::string& url, PixelBufferId& pixelBufferId) const;

  /**
   * @brief Cache the pixel buffer
   * @note We load image syncronously.
   *
   * @param[in] url The url of embedded pixel buffer.
   * @return PixelBufferId of newly inserted pixel buffer. Or 0 if we fail to be load.
   */
  PixelBufferId CacheEmbeddedPixelBuffer(const std::string& url);

  /**
   * @brief Finds in the cache a embedded item.
   *
   * @param pixelBufferId The id of embedded item's pixel buffer.
   * @param width The width of embedded item.
   * @param height The height of embedded item.
   * @param[out] index GlyphIndex of embedded item.
   * @return Whether the embedded item has been found.
   */
  bool FindEmbeddedItem(PixelBufferId pixelBufferId, uint32_t width, uint32_t height, GlyphIndex& index) const;

  /**
   * @brief Cache the embedded item.
   * @note We use std::move operation to cache item.
   *
   * @param[in] embeddedItem The url of embedded pixel buffer.
   * @return GlyphIndex of newly inserted embedded item.
   */
  GlyphIndex CacheEmbeddedItem(EmbeddedItem&& embeddedItem);

  /**
   * @brief Checks FontIdCacheItem cached with the fontId.
   *
   * @param[in] fontId The fontId.
   * @return true if FontIdCacheItem exists.
   */
  bool IsFontIdCacheItemExist(FontId fontId);

  bool IsFontFaceCacheItemExist(FontCacheIndex fontCacheIndex);

  /**
   * @brief Find the FontIdCacheItem from fontId.
   * @note Assert if the value is not found.
   *
   * @param[in] fontId The fontId.
   * @return FontIdCacheItem found.
   */
  FontClient::Plugin::CacheHandler::FontIdCacheItem& FindFontIdCacheItem(FontId fontId);

  /**
   * @brief Find the FontFaceCacheItem from fontCacheIndex.
   * @note Assert if the value is not found.
   *
   * @param[in] fontCacheIndex The fontCacheIndex.
   * @return FontFaceCacheItem found.
   */
  FontFaceCacheItem& FindFontFaceCacheItem(FontCacheIndex fontCacheIndex);

  /**
   * @brief Find the BitmapFontCacheItem from fontCacheIndex.
   * @note Assert if the value is not found.
   *
   * @param[in] fontCacheIndex The fontCacheIndex.
   * @return BitmapFontCacheItem found.
   */
  BitmapFontCacheItem& FindBitmapFontCacheItem(FontCacheIndex fontCacheIndex);


public: // Other public API
  FontFaceManager* GetFontFaceManager() const
  {
    return mFontFaceManager.get();
  }

  GlyphCacheManager* GetGlyphCacheManager() const
  {
    return mGlyphCacheManager.get();
  }

private:
  CacheHandler(const CacheHandler&) = delete;
  CacheHandler& operator=(const CacheHandler&) = delete;

  using DescriptionCacheContainer = LRUCacheContainer<FontDescriptionSizeCacheKey, FontCacheIndex, FontDescriptionSizeCacheKeyHash>;

public:                                    // Cache container list
  FcConfig*       mFontConfig;             ///< A handle to a FontConfig library instance.
  FontDescription mDefaultFontDescription; ///< Cached default font from the system

  FontList         mSystemFonts;              ///< Cached system fonts.
  FontList         mDefaultFonts;             ///< Cached default fonts.
  CharacterSetList mDefaultFontCharacterSets; ///< Cached default fonts character set.

  std::vector<FallbackCacheItem> mFallbackCache; ///< Cached fallback font lists.

  std::unordered_map<FontId, FontIdCacheItem>             mFontIdCache;     ///< Caches from FontId to FontCacheIndex.
  std::unordered_map<FontCacheIndex, FontFaceCacheItem>   mFontFaceCache;   ///< Caches the FreeType face and font metrics of the triplet 'path to the font file name, font point size and face index'.
  std::unordered_map<FontCacheIndex, BitmapFontCacheItem> mBitmapFontCache; ///< Stores bitmap fonts.

  std::vector<FontDescriptionCacheItem> mValidatedFontCache;   ///< Caches indices to the vector of font descriptions for a given font.
  FontList                              mFontDescriptionCache; ///< Caches font descriptions for the validated font.
  CharacterSetList                      mCharacterSetCache;    ///< Caches character set lists for the validated font.

  DescriptionCacheContainer mFontDescriptionSizeCache; ///< LRU Cache container of glyph

  std::vector<EllipsisItem>         mEllipsisCache;     ///< Caches ellipsis glyphs for a particular point size.
  std::vector<PixelBufferCacheItem> mPixelBufferCache;  ///< Caches the pixel buffer of a url.
  std::vector<EmbeddedItem>         mEmbeddedItemCache; ///< Cache embedded items.

  FontPathList mCustomFontDirectories;  ///< Cache custom font directories to recovery upon reinitialization.
  std::unique_ptr<FontFaceManager>   mFontFaceManager;   ///< The freetype font face manager. It will cache font face.
  std::unique_ptr<GlyphCacheManager> mGlyphCacheManager; ///< The glyph cache manager. It will cache this face's glyphs.

private:                                                 // Member value

  FontDescription   mLatestFoundFontDescription; ///< Latest found font description and id in FindValidatedFont()
  FontDescriptionId mLatestFoundFontDescriptionId;

  FontDescriptionSizeCacheKey mLatestFoundCacheKey; ///< Latest found font description and id in FindFont()
  FontCacheIndex              mLatestFoundCacheIndex;

  uint32_t mFontCacheCount;
  bool mDefaultFontDescriptionCached : 1; ///< Whether the default font is cached or not
};

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_CACHE_HANDLER_H
