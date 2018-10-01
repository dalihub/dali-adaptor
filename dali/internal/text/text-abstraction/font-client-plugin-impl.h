#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/font-metrics.h>
#include <dali/devel-api/text-abstraction/glyph-info.h>
#include <dali/internal/text/text-abstraction/font-client-impl.h>

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
#include <dali/internal/text/glyphy/vector-font-cache.h>
#else
class VectorFontCache;
#endif

// EXTERNAL INCLUDES
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
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
 *@brief Type used for indices addressing the vector with front descriptions of validated fonts.
 */
typedef uint32_t FontDescriptionId;
typedef Vector<_FcCharSet*> CharacterSetList;

/**
 * @brief FontClient implementation.
 */
struct FontClient::Plugin
{
  /**
   * @brief Caches an list of fallback fonts for a given font-description
   */
  struct FallbackCacheItem
  {
    FallbackCacheItem( FontDescription&& fontDescription, FontList* fallbackFonts, CharacterSetList* characterSets );

    FontDescription fontDescription; ///< The font description.
    FontList* fallbackFonts;         ///< The list of fallback fonts for the given font-description.
    CharacterSetList* characterSets; ///< The list of character sets for the given font-description.
  };

  /**
   * @brief Caches an index to the vector of font descriptions for a given font.
   */
  struct FontDescriptionCacheItem
  {
    FontDescriptionCacheItem( const FontDescription& fontDescription,
                              FontDescriptionId index );
    FontDescriptionCacheItem( FontDescription&& fontDescription,
                              FontDescriptionId index );

    FontDescription fontDescription; ///< The font description.
    FontDescriptionId index;         ///< Index to the vector of font descriptions.
  };

  /**
   * @brief Caches the font id of the pair font point size and the index to the vector of font descriptions of validated fonts.
   */
  struct FontDescriptionSizeCacheItem
  {
    FontDescriptionSizeCacheItem( FontDescriptionId validatedFontId,
                                  PointSize26Dot6 requestedPointSize,
                                  FontId fontId );

    FontDescriptionId validatedFontId;    ///< Index to the vector with font descriptions.
    PointSize26Dot6   requestedPointSize; ///< The font point size.
    FontId            fontId;             ///< The font identifier.
  };

  /**
   * @brief Caches the FreeType face and font metrics of the triplet 'path to the font file name, font point size and face index'.
   */
  struct FontFaceCacheItem
  {
    FontFaceCacheItem( FT_Face ftFace,
                       const FontPath& path,
                       PointSize26Dot6 requestedPointSize,
                       FaceIndex face,
                       const FontMetrics& metrics );

    FontFaceCacheItem( FT_Face ftFace,
                       const FontPath& path,
                       PointSize26Dot6 requestedPointSize,
                       FaceIndex face,
                       const FontMetrics& metrics,
                       float fixedWidth,
                       float fixedHeight,
                       bool hasColorTables );

    FT_Face mFreeTypeFace;               ///< The FreeType face.
    FontPath mPath;                      ///< The path to the font file name.
    PointSize26Dot6 mRequestedPointSize; ///< The font point size.
    FaceIndex mFaceIndex;                ///< The face index.
    FontMetrics mMetrics;                ///< The font metrics.
    _FcCharSet* mCharacterSet;           ///< Pointer with the range of characters.
    FT_Short mFixedWidthPixels;          ///< The height in pixels (fixed size bitmaps only)
    FT_Short mFixedHeightPixels;         ///< The height in pixels (fixed size bitmaps only)
    unsigned int mVectorFontId;          ///< The ID of the equivalent vector-based font
    bool mIsFixedSizeBitmap : 1;         ///< Whether the font has fixed size bitmaps.
    bool mHasColorTables    : 1;         ///< Whether the font has color tables.
  };

  struct EllipsisItem
  {
    PointSize26Dot6 requestedPointSize;
    GlyphInfo glyph;
  };

  /**
   * Constructor.
   *
   * Initializes the FreeType library.
   * Initializes the dpi values.
   *
   * @param[in] horizontalDpi The horizontal dpi.
   * @param[in] verticalDpi The vertical dpi.
   */
  Plugin( unsigned int horizontalDpi, unsigned int verticalDpi );

  /**
   * Default destructor.
   *
   * Frees any allocated resource.
   */
  ~Plugin();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDpi()
   */
  void SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::ResetSystemDefaults()
   */
  void ResetSystemDefaults();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDefaultFont()
   */
  void SetDefaultFont( const FontDescription& preferredFontDescription );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultPlatformFontDescription()
   */
  void GetDefaultPlatformFontDescription( FontDescription& fontDescription );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultFonts()
   */
  void GetDefaultFonts( FontList& defaultFonts );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetSystemFonts()
   */
  void GetSystemFonts( FontList& systemFonts );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDescription()
   */
  void GetDescription( FontId id, FontDescription& fontDescription ) const;

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetPointSize()
   */
  PointSize26Dot6 GetPointSize( FontId id );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsCharacterSupportedByFont()
   */
  bool IsCharacterSupportedByFont( FontId fontId, Character character );

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
  FontId FindFontForCharacter( const FontList& fontList,
                               const CharacterSetList& characterSetList,
                               Character charcode,
                               PointSize26Dot6 requestedPointSize,
                               bool preferColor );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::FindDefaultFont()
   */
  FontId FindDefaultFont( Character charcode,
                          PointSize26Dot6 requestedPointSize,
                          bool preferColor );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::FindFallbackFont()
   */
  FontId FindFallbackFont( Character charcode,
                           const FontDescription& preferredFontDescription,
                           PointSize26Dot6 requestedPointSize,
                           bool preferColor );

  /**
   * @see Dali::TextAbstraction::FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   *
   * @param[in] cacheDescription Whether to cache the font description.
   */
  FontId GetFontId( const FontPath& path,
                    PointSize26Dot6 requestedPointSize,
                    FaceIndex faceIndex,
                    bool cacheDescription );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontDescription& preferredFontDescription, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   */
  FontId GetFontId( const FontDescription& fontDescription,
                    PointSize26Dot6 requestedPointSize,
                    FaceIndex faceIndex );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsScalable( const FontPath& path )
   */
  bool IsScalable( const FontPath& path );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsScalable( const FontDescription& fontDescription )
   */
  bool IsScalable( const FontDescription& fontDescription );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes()
   */
  void GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes()
   */
  void GetFixedSizes( const FontDescription& fontDescription,
                      Dali::Vector< PointSize26Dot6 >& sizes );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontMetrics()
   */
  void GetFontMetrics( FontId fontId, FontMetrics& metrics );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex( FontId fontId, Character charcode );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetGlyphMetrics()
   */
  bool GetGlyphMetrics( GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal );

  /**
   * Helper for GetGlyphMetrics when using bitmaps
   */
  bool GetBitmapMetrics( GlyphInfo* array, uint32_t size, bool horizontal );

  /**
   * Helper for GetGlyphMetrics when using vectors
   */
  bool GetVectorMetrics( GlyphInfo* array, uint32_t size, bool horizontal );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool softwareItalic, bool softwareBold, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth )
   */
  void CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool softwareItalic, bool softwareBold, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
   */
  PixelData CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::CreateVectorBlob()
   */
  void CreateVectorBlob( FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetEllipsisGlyph()
   */
  const GlyphInfo& GetEllipsisGlyph( PointSize26Dot6 requestedPointSize );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsColorGlyph()
   */
  bool IsColorGlyph( FontId fontId, GlyphIndex glyphIndex );

  /**
   * @copydoc Dali::TextAbstraction::Internal::FontClient::GetFreetypeFace()
   */
  FT_FaceRec_* GetFreetypeFace( FontId fontId );

private:

  /**
   * @brief Caches the fonts present in the platform.
   *
   * Calls GetFcFontSet() to retrieve the fonts.
   */
  void InitSystemFonts();

  /**
   * @brief Gets the FontDescription which matches the given pattern.
   * @param[in] pattern pattern to match against.
   * @param[out] fontDescription the resultant fontDescription that matched.
   * @param[out] characterSet The character set for that pattern.
   * @return true if match found.
   */
  bool MatchFontDescriptionToPattern( _FcPattern* pattern, Dali::TextAbstraction::FontDescription& fontDescription, _FcCharSet** characterSet );

  /**
   * @brief Creates a font family pattern used to match fonts.
   *
   * @param[in] fontDescription The font to cache.
   *
   * @return The pattern.
   */
  _FcPattern* CreateFontFamilyPattern( const FontDescription& fontDescription ) const;

  /**
   * Retrieves the fonts present in the platform.
   *
   * @return A font fonfig data structure with the platform's fonts.
   */
  _FcFontSet* GetFcFontSet() const;

  /**
   * Retrieves a font config object's value from a pattern.
   *
   * @param[in] pattern The font config pattern.
   * @param[in] n The object.
   * @param[out] string The object's value.
   *
   * @return @e true if the operation is successful.
   */
  bool GetFcString( const _FcPattern* const pattern, const char* const n, std::string& string );

  /**
   * Retrieves a font config object's value from a pattern.
   *
   * @param[in] pattern The font config pattern.
   * @param[in] n The object.
   * @param[out] intVal The object's value.
   *
   * @return @e true if the operation is successful.
   */
  bool GetFcInt( const _FcPattern* const pattern, const char* const n, int& intVal );

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
  FontId CreateFont( const FontPath& path,
                     PointSize26Dot6 requestedPointSize,
                     FaceIndex faceIndex,
                     bool cacheDescription );

  /**
   * @brief Copy the FreeType bitmap to the given buffer.
   *
   * @param[out] data The bitmap data.
   * @param[in] srcBitmap The FreeType bitmap.
   */
  void ConvertBitmap( TextAbstraction::FontClient::GlyphBufferData& data, FT_Bitmap srcBitmap );

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
  bool FindFont( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex, FontId& fontId ) const;

  /**
   * @brief Finds in the cache a cluster 'font family, font width, font weight, font slant'
   * If there is one, it writes the index to the vector with font descriptions in the param @p validatedFontId.
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] validatedFontId The index to the vector with font descriptions.
   *
   * @return @e true if the pair is found.
   */
  bool FindValidatedFont( const FontDescription& fontDescription,
                          FontDescriptionId& validatedFontId );

  /**
   * @brief Finds a fallback font list from the cache for a given font-description
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] A valid pointer to a font list, or @e nullptr if not found.
   * @param[out] characterSetList A valid pointer to a character set list, or @e nullptr if not found.
   */
  bool FindFallbackFontList( const FontDescription& fontDescription,
                             FontList*& fontList,
                             CharacterSetList*& characterSetList );

  /**
   * @brief Finds in the cache a pair 'validated font identifier and font point size'.
   * If there is one it writes the font identifier in the param @p fontId.
   *
   * @param[in] validatedFontId Index to the vector with font descriptions.
   * @param[in] requestedPointSize The font point size.
   * @param[out] fontId The font identifier.
   *
   * @return @e true if the pair is found.
   */
  bool FindFont( FontDescriptionId validatedFontId,
                 PointSize26Dot6 requestedPointSize,
                 FontId& fontId );

  /**
   * @brief Validate a font description.
   *
   * @param[in] fontDescription The font to validate.
   * @param[out] validatedFontId Result of validation
   */
  void ValidateFont( const FontDescription& fontDescription,
                     FontDescriptionId& validatedFontId );

  /**
   * Helper for GetDefaultFonts etc.
   *
   * @param[in] fontDescription A font description.
   * @param[out] fontList A list of the fonts which are a close match for fontDescription.
   * @param[out] characterSetList A list of the character sets which are a close match for fontDescription.
   */
  void SetFontList( const FontDescription& fontDescription, FontList& fontList, CharacterSetList& characterSetList );

  /**
   * Caches a font path.
   *
   * @param[in] ftFace The FreeType face.
   * @param[in] id The font identifier.
   * @param[in] requestedPointSize The font point size.
   * @param[in] path Path to the font file name.
   */
  void CacheFontPath( FT_Face ftFace, FontId id, PointSize26Dot6 requestedPointSize,  const FontPath& path );

  /**
   * @brief Creates a character set from a given font's @p description.
   *
   * @param[in] description The font's description.
   *
   * @return A character set.
   */
  _FcCharSet* CreateCharacterSetFromDescription( const FontDescription& description );

  /**
   * @brief Destroy all matched Patterns.
   *
   */
  void DestroyMatchedPatterns();

private:

  // Declared private and left undefined to avoid copies.
  Plugin( const Plugin& );
  // Declared private and left undefined to avoid copies.
  Plugin& operator=( const Plugin& );

private:

  FT_Library mFreeTypeLibrary; ///< A handle to a FreeType library instance.

  unsigned int mDpiHorizontal; ///< Horizontal dpi.
  unsigned int mDpiVertical;   ///< Vertical dpi.

  FontDescription mDefaultFontDescription; ///< The cached default font from the system

  FontList mSystemFonts;       ///< Cached system fonts.
  FontList mDefaultFonts;      ///< Cached default fonts.
  CharacterSetList mDefaultFontCharacterSets;

  std::vector<FallbackCacheItem> mFallbackCache; ///< Cached fallback font lists.

  std::vector<FontFaceCacheItem>            mFontFaceCache;            ///< Caches the FreeType face and font metrics of the triplet 'path to the font file name, font point size and face index'.
  std::vector<FontDescriptionCacheItem>     mValidatedFontCache;       ///< Caches indices to the vector of font descriptions for a given font.
  FontList                                  mFontDescriptionCache;     ///< Caches font descriptions for the validated font.
  CharacterSetList                          mCharacterSetCache;        ///< Caches character set lists for the validated font.
  std::vector<FontDescriptionSizeCacheItem> mFontDescriptionSizeCache; ///< Caches font identifiers for the pairs of font point size and the index to the vector with font descriptions of the validated fonts.

  VectorFontCache* mVectorFontCache;            ///< Separate cache for vector data blobs etc.
  Vector<EllipsisItem> mEllipsisCache;          ///< Caches ellipsis glyphs for a particular point size.
  Vector<_FcPattern*>  mMatchedFcPatternCache;  ///< Contain matched FcPattern pointer.

  bool mDefaultFontDescriptionCached : 1; ///< Whether the default font is cached or not
};

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H
