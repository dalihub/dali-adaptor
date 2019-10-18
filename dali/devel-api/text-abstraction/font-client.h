#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/images/buffer-image.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/devel-api/text-abstraction/font-list.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace TextAbstraction
{

struct FontMetrics;
struct GlyphInfo;
struct BitmapFont;

namespace Internal DALI_INTERNAL
{
class FontClient;
}

/**
 * @brief FontClient provides access to font information and resources.
 *
 * <h3>Querying the System Fonts</h3>
 *
 * A "system font" is described by a "path" to a font file on the native filesystem, along with a "family" and "style".
 * For example on the Ubuntu system a "Regular" style font from the "Ubuntu Mono" family can be accessed from "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf".
 *
 * <h3>Accessing Fonts</h3>
 *
 * A "font" is created from the system for a specific point size in 26.6 fractional points. A "FontId" is used to identify each font.
 * For example two different fonts with point sizes 10 & 12 can be created from the "Ubuntu Mono" family:
 * @code
 * FontClient fontClient   = FontClient::Get();
 * FontId ubuntuMonoTen    = fontClient.GetFontId( "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf", 10*64 );
 * FontId ubuntuMonoTwelve = fontClient.GetFontId( "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf", 12*64 );
 * @endcode
 * Glyph metrics and bitmap resources can then be retrieved using the FontId.
 */
class DALI_ADAPTOR_API FontClient : public BaseHandle
{
public:
  static const PointSize26Dot6 DEFAULT_POINT_SIZE; ///< The default point size.
  static const float DEFAULT_ITALIC_ANGLE;         ///< The default software italic angle in radians.

  /**
   * @brief Struct used to retrieve the glyph's bitmap.
   */
  struct DALI_ADAPTOR_API GlyphBufferData
  {
    /**
     * @brief Constructor.
     *
     * Initializes struct members to their defaults.
     */
    GlyphBufferData();

    /**
     * @brief Destructor.
     */
    ~GlyphBufferData();

    unsigned char* buffer;          ///< The glyph's bitmap buffer data.
    unsigned int   width;           ///< The width of the bitmap.
    unsigned int   height;          ///< The height of the bitmap.
    int            outlineOffsetX;  ///< The additional horizontal offset to be added for the glyph's position for outline.
    int            outlineOffsetY;  ///< The additional vertical offset to be added for the glyph's position for outline.
    Pixel::Format  format;          ///< The pixel's format of the bitmap.
    bool           isColorEmoji:1;  ///< Whether the glyph is an emoji.
    bool           isColorBitmap:1; ///< Whether the glyph is a color bitmap.
  };

  /**
   * @brief Used to load an embedded item into the font client.
   */
  struct EmbeddedItemDescription
  {
    std::string       url;               ///< The url path of the image.
    unsigned int      width;             ///< The width of the item.
    unsigned int      height;            ///< The height of the item.
    ColorBlendingMode colorblendingMode; ///< Whether the color of the image is multiplied by the color of the text.
  };

public:

  /**
   * @brief Retrieve a handle to the FontClient instance.
   *
   * @return A handle to the FontClient
   */
  static FontClient Get();

  /**
   * @brief Create an uninitialized TextAbstraction handle.
   */
  FontClient();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~FontClient();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle.
   */
  FontClient( const FontClient& handle );

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] handle  A reference to the copied handle.
   * @return A reference to this.
   */
  FontClient& operator=( const FontClient& handle );

  ////////////////////////////////////////
  // Font management and validation.
  ////////////////////////////////////////

  /**
   * @brief Clear all caches in FontClient
   *
   */
  void ClearCache();

  /**
   * @brief Set the DPI of the target window.
   *
   * @note Multiple windows are not currently supported.
   * @param[in] horizontalDpi The horizontal resolution in DPI.
   * @param[in] verticalDpi The vertical resolution in DPI.
   */
  void SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi );

  /**
   * @brief Retrieves the DPI previously set to the target window.
   *
   * @note Multiple windows are not currently supported.
   * @param[out] horizontalDpi The horizontal resolution in DPI.
   * @param[out] verticalDpi The vertical resolution in DPI.
   */
  void GetDpi( unsigned int& horizontalDpi, unsigned int& verticalDpi );

  /**
   * @brief Called by Dali to retrieve the default font size for the platform.
   *
   * This is an accessibility size, which is mapped to a UI Control specific point-size in stylesheets.
   * For example if zero the smallest size, this could potentially map to TextLabel point-size 8.
   * @return The default font size.
   */
  int GetDefaultFontSize();

  /**
   * @brief Called when the user changes the system defaults.
   *
   * @post Previously cached system defaults are removed.
   */
  void ResetSystemDefaults();

  /**
   * @brief Retrieve the list of default fonts supported by the system.
   *
   * @param[out] defaultFonts A list of default font paths, family, width, weight and slant.
   */
  void GetDefaultFonts( FontList& defaultFonts );

  /**
   * @brief Retrieve the active default font from the system.
   *
   * @param[out] fontDescription font structure describing the default font.
   */
  void GetDefaultPlatformFontDescription( FontDescription& fontDescription );

  /**
   * @brief Retrieve the list of fonts supported by the system.
   *
   * @param[out] systemFonts A list of font paths, family, width, weight and slant.
   */
  void GetSystemFonts( FontList& systemFonts );

  /**
   * @brief Retrieves the font description of a given font @p id.
   *
   * @param[in] id The font identifier.
   * @param[out] fontDescription The path, family & style (width, weight and slant) describing the font.
   */
  void GetDescription( FontId id, FontDescription& fontDescription );

  /**
   * @brief Retrieves the font point size of a given font @p id.
   *
   * @param[in] id The font identifier.
   *
   * @return The point size in 26.6 fractional points.
   */
  PointSize26Dot6 GetPointSize( FontId id );

  /**
   * @brief Whether the given @p character is supported by the font.
   *
   * @param[in] fontId The id of the font.
   * @param[in] character The character.
   *
   * @return @e true if the character is supported by the font.
   */
  bool IsCharacterSupportedByFont( FontId fontId, Character character );

  /**
   * @brief Find the default font for displaying a UTF-32 character.
   *
   * This is useful when localised strings are provided for multiple languages
   * i.e. when a single default font does not work for all languages.
   *
   * @param[in] charcode The character for which a font is needed.
   * @param[in] requestedPointSize The point size in 26.6 fractional points; the default point size is 12*64.
   * @param[in] preferColor @e true if a color font is preferred.
   *
   * @return A valid font identifier, or zero if the font does not exist.
   */
  FontId FindDefaultFont( Character charcode,
                          PointSize26Dot6 requestedPointSize = DEFAULT_POINT_SIZE,
                          bool preferColor = false );

  /**
   * @brief Find a fallback-font for displaying a UTF-32 character.
   *
   * This is useful when localised strings are provided for multiple languages
   * i.e. when a single default font does not work for all languages.
   *
   * @param[in] charcode The character for which a font is needed.
   * @param[in] preferredFontDescription Description of the preferred font which may not provide a glyph for @p charcode.
   *                                     The fallback-font will be the closest match to @p preferredFontDescription, which does support the required glyph.
   * @param[in] requestedPointSize The point size in 26.6 fractional points; the default point size is 12*64.
   * @param[in] preferColor @e true if a color font is preferred.
   *
   * @return A valid font identifier, or zero if the font does not exist.
   */
  FontId FindFallbackFont( Character charcode,
                           const FontDescription& preferredFontDescription,
                           PointSize26Dot6 requestedPointSize = DEFAULT_POINT_SIZE,
                           bool preferColor = false );

  /**
   * @brief Retrieve the unique identifier for a font.
   *
   * @param[in] path The path to a font file.
   * @param[in] requestedPointSize The point size in 26.6 fractional points; the default point size is 12*64.
   * @param[in] faceIndex The index of the font face (optional).
   *
   * @return A valid font identifier, or zero if the font does not exist.
   */
  FontId GetFontId( const FontPath& path,
                    PointSize26Dot6 requestedPointSize = DEFAULT_POINT_SIZE,
                    FaceIndex faceIndex = 0 );

  /**
   * @brief Retrieves a unique font identifier for a given description.
   *
   * @param[in] preferredFontDescription Description of the preferred font.
   *                                     The font will be the closest match to @p preferredFontDescription.
   * @param[in] requestedPointSize The point size in 26.6 fractional points; the default point size is 12*64.
   * @param[in] faceIndex The index of the font face (optional).
   *
   * @return A valid font identifier, or zero if no font is found.
   */
  FontId GetFontId( const FontDescription& preferredFontDescription,
                    PointSize26Dot6 requestedPointSize = DEFAULT_POINT_SIZE,
                    FaceIndex faceIndex = 0 );

  /**
   * @brief Retrieves a unique font identifier for a given bitmap font.
   *
   * @param[in] bitmapFont A bitmap font.
   *
   * @return A valid font identifier, or zero if no bitmap font is created.
   */
  FontId GetFontId( const BitmapFont& bitmapFont );

  /**
   * @brief Check to see if a font is scalable.
   *
   * @param[in] path The path to a font file.
   * @return true if scalable.
   */
  bool IsScalable( const FontPath& path );

  /**
   * @brief Check to see if a font is scalable.
   *
   * @note It the font style is not empty, it will be used instead the font weight and font slant slant.
   *
   * @param[in] fontDescription A font description.
   *
   * @return true if scalable
   */
  bool IsScalable( const FontDescription& fontDescription );

  /**
   * @brief Get a list of sizes available for a fixed size font.
   *
   * @param[in] path The path to a font file.
   * @param[out] sizes A list of the available sizes, if no sizes available will return empty.
   */
  void GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes );

  /**
   * @brief Get a list of sizes available for a fixed size font.
   *
   * @note It the font style is not empty, it will be used instead the font weight and font slant slant.
   *
   * @param[in] fontDescription A font description.
   * @param[out] sizes A list of the available sizes, if no sizes available will return empty.
   */
  void GetFixedSizes( const FontDescription& fontDescription,
                      Dali::Vector< PointSize26Dot6 >& sizes );

  /**
   * @brief Whether the font has Italic style.
   *
   * @param[in] fontId The font identifier.
   *
   * @return true if the font has italic style.
   */
  bool HasItalicStyle( FontId fontId ) const;

  ////////////////////////////////////////
  // Font metrics, glyphs and bitmaps.
  ////////////////////////////////////////

  /**
   * @brief Query the metrics for a font.
   *
   * @param[in] fontId The identifier of the font for the required glyph.
   * @param[out] metrics The font metrics.
   */
  void GetFontMetrics( FontId fontId, FontMetrics& metrics );

  /**
   * @brief Retrieve the glyph index for a UTF-32 character code.
   *
   * @param[in] fontId The identifier of the font for the required glyph.
   * @param[in] charcode The UTF-32 character code.
   *
   * @return The glyph index, or zero if the character code is undefined.
   */
  GlyphIndex GetGlyphIndex( FontId fontId, Character charcode );

  /**
   * @brief Retrieve the metrics for a series of glyphs.
   *
   * @param[in,out] array An array of glyph-info structures with initialized FontId & GlyphIndex values.
   *                      It may contain the advance and an offset set into the bearing from the shaping tool.
   *                      On return, the glyph's size value will be initialized. The bearing value will be updated by adding the font's glyph bearing to the one set by the shaping tool.
   * @param[in] size The size of the array.
   * @param[in] type The type of glyphs used for rendering; either bitmaps or vectors.
   * @param[in] horizontal True for horizontal layouts (set to false for vertical layouting).
   *
   * @return @e true if all of the requested metrics were found.
   */
  bool GetGlyphMetrics( GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal = true );

  /**
   * @brief Create a bitmap representation of a glyph.
   *
   * @note The caller is responsible for deallocating the bitmap data @p data.buffer using delete[].
   *
   * @param[in]  fontId           The identifier of the font.
   * @param[in]  glyphIndex       The index of a glyph within the specified font.
   * @param[in]  isItalicRequired Whether the glyph requires italic style.
   * @param[in]  isBoldRequired   Whether the glyph requires bold style.
   * @param[out] data             The bitmap data.
   * @param[in]  outlineWidth     The width of the glyph outline in pixels.
   */
  void CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, GlyphBufferData& data, int outlineWidth );

  /**
   * @brief Create a bitmap representation of a glyph.
   *
   * @param[in] fontId The identifier of the font.
   * @param[in] glyphIndex The index of a glyph within the specified font.
   * @param[in] outlineWidth The width of the glyph outline in pixels.
   *
   * @return A valid BufferImage, or an empty handle if the glyph could not be rendered.
   */
  PixelData CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth );

  /**
   * @brief Create a vector representation of a glyph.
   *
   * @note This feature requires highp shader support and is not available on all platforms
   * @param[in] fontId The identifier of the font.
   * @param[in] glyphIndex The index of a glyph within the specified font.
   * @param[out] blob A blob of data; this is owned by FontClient and should be copied by the caller of CreateVectorData().
   * @param[out] blobLength The length of the blob data, or zero if the blob creation failed.
   * @param[out] nominalWidth The width of the blob.
   * @param[out] nominalHeight The height of the blob.
   */
  void CreateVectorBlob( FontId fontId,
                         GlyphIndex glyphIndex,
                         VectorBlob*& blob,
                         unsigned int& blobLength,
                         unsigned int& nominalWidth,
                         unsigned int& nominalHeight );

  /**
   * @brief Retrieves the ellipsis glyph for a requested point size.
   *
   * @param[in] requestedPointSize The requested point size.
   *
   * @return The ellipsis glyph.
   */
  const GlyphInfo& GetEllipsisGlyph( PointSize26Dot6 requestedPointSize );

  /**
   * @brief Whether the given glyph @p glyphIndex is a color glyph.
   *
   * @param[in] fontId The font id.
   * @param[in] glyphIndex The glyph index.
   *
   * @return @e true if the glyph is a color one.
   */
  bool IsColorGlyph( FontId fontId, GlyphIndex glyphIndex );

  /**
   * @brief  Add custom fonts directory
   *
   * @param[in] path to the fonts directory
   *
   * @return true if the fonts can be added.
   */
  bool AddCustomFontDirectory( const FontPath& path );

  /**
   * @brief Creates and stores an embedded item and it's metrics.
   *
   * If in the @p description there is a non empty url, it calls Dali::LoadImageFromFile() internally.
   * If in the @p description there is a url and @e width or @e height are zero it stores the default size. Otherwise the image is resized.
   * If the url in the @p description is empty it stores the size.
   *
   * @param[in] description The description of the embedded item.
   * @param[out] pixelFormat The pixel format of the image.
   *
   * return The index within the vector of embedded items.
   */
  GlyphIndex CreateEmbeddedItem( const EmbeddedItemDescription& description, Pixel::Format& pixelFormat);


public: // Not intended for application developers
  /**
   * @brief This constructor is used by FontClient::Get().
   *
   * @param[in] fontClient  A pointer to the internal fontClient object.
   */
  explicit DALI_INTERNAL FontClient( Internal::FontClient* fontClient );
};

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H
