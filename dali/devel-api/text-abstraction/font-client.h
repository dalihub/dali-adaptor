#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H

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
#include <dali/devel-api/text-abstraction/font-list.h>
#include <dali/devel-api/text-abstraction/glyph-buffer-data.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/base-handle.h>

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
  static const PointSize26Dot6 DEFAULT_POINT_SIZE;   ///< The default point size.
  static const float           DEFAULT_ITALIC_ANGLE; ///< The default software italic angle in radians.

  static const bool     DEFAULT_ATLAS_LIMITATION_ENABLED; ///< The default behavior of whether atlas limitation is enabled in dali.
  static const uint32_t DEFAULT_TEXT_ATLAS_WIDTH;         ///< The default width of text-atlas-block.
  static const uint32_t DEFAULT_TEXT_ATLAS_HEIGHT;        ///< The default height of text-atlas-block.
  static const Size     DEFAULT_TEXT_ATLAS_SIZE;          ///< The default size(width, height) of text-atlas-block.

  static const uint32_t MAX_TEXT_ATLAS_WIDTH;  ///< The maximum width of text-atlas-block.
  static const uint32_t MAX_TEXT_ATLAS_HEIGHT; ///< The maximum height of text-atlas-block.
  static const Size     MAX_TEXT_ATLAS_SIZE;   ///< The maximum height of text-atlas-block.

  static const uint16_t PADDING_TEXT_ATLAS_BLOCK; ///< Padding per edge. How much the block size (width, height) less than the text-atlas-block size (width, height).
  static const Size     MAX_SIZE_FIT_IN_ATLAS;    ///< The maximum block's size fit into text-atlas-block.

  static const uint32_t NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE; ///< Factor multiply point-size in toolkit.

  // For Debug
  static uint32_t GetPerformanceLogThresholdTime(); ///< Return performance log threshold time in miliseconds for debug.
  static bool     IsPerformanceLogEnabled();        ///< Whether performance log is enabled.

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
   * @remarks A reference to the singleton instance of FontClient.
   */
  static FontClient Get();

  /**
   * @brief Create a handle to the new FontClient instance.
   *
   * @return A handle to the FontClient
   * @remarks All functions of this are not thread-safe,
   * so create new handles for each worker thread to utilize them.
   */
  static FontClient New();

  /**
   * @brief Create a handle to the new FontClient instance.
   *
   * @param[in] horizontalDpi The horizontal resolution in DPI.
   * @param[in] verticalDpi The vertical resolution in DPI.
   * @return A handle to the FontClient
   * @remarks All functions of this are not thread-safe,
   * so create new handles for each worker thread to utilize them.
   */
  static FontClient New(uint32_t horizontalDpi, uint32_t verticalDpi);

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
  FontClient(const FontClient& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] handle  A reference to the copied handle.
   * @return A reference to this.
   */
  FontClient& operator=(const FontClient& handle);

  /**
   * @brief This move constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the moved handle.
   */
  FontClient(FontClient&& handle) noexcept;

  /**
   * @brief This move assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] handle  A reference to the moved handle.
   * @return A reference to this.
   */
  FontClient& operator=(FontClient&& handle) noexcept;

  ////////////////////////////////////////
  // Font management and validation.
  ////////////////////////////////////////

  /**
   * @brief Clear all caches in FontClient
   *
   */
  void ClearCache();

  /**
   * @brief Clear caches in FontClient when Locale Changed
   *
   * @note This function only clears caches that needs to be cleared.
   */
  void ClearCacheOnLocaleChanged();

  /**
   * @brief Set the DPI of the target window.
   *
   * @note Multiple windows are not currently supported.
   * @param[in] horizontalDpi The horizontal resolution in DPI.
   * @param[in] verticalDpi The vertical resolution in DPI.
   */
  void SetDpi(unsigned int horizontalDpi, unsigned int verticalDpi);

  /**
   * @brief Sets the DPI of the FontClient based on the current window's DPI.
   *
   * @note Multiple windows are not currently supported.
   */
  void SetDpiFromWindowSystem();

  /**
   * @brief Retrieves the DPI previously set to the target window.
   *
   * @note Multiple windows are not currently supported.
   * @param[out] horizontalDpi The horizontal resolution in DPI.
   * @param[out] verticalDpi The vertical resolution in DPI.
   */
  void GetDpi(unsigned int& horizontalDpi, unsigned int& verticalDpi);

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
  void GetDefaultFonts(FontList& defaultFonts);

  /**
   * @brief Initializes and caches default font from the system.
   */
  void InitDefaultFontDescription();

  /**
   * @brief Retrieve the active default font from the system.
   *
   * @param[out] fontDescription font structure describing the default font.
   */
  void GetDefaultPlatformFontDescription(FontDescription& fontDescription);

  /**
   * @brief Retrieve the list of fonts supported by the system.
   *
   * @param[out] systemFonts A list of font paths, family, width, weight and slant.
   */
  void GetSystemFonts(FontList& systemFonts);

  /**
   * @brief Retrieves the font description of a given font @p fontId.
   *
   * @param[in] fontId The font identifier.
   * @param[out] fontDescription The path, family & style (width, weight and slant) describing the font.
   */
  void GetDescription(FontId fontId, FontDescription& fontDescription);

  /**
   * @brief Retrieves the font point size of a given font @p fontId.
   *
   * @param[in] fontId The font identifier.
   *
   * @return The point size in 26.6 fractional points.
   */
  PointSize26Dot6 GetPointSize(FontId fontId);

  /**
   * @brief Whether the given @p character is supported by the font.
   *
   * @param[in] fontId The id of the font.
   * @param[in] character The character.
   *
   * @return @e true if the character is supported by the font.
   */
  bool IsCharacterSupportedByFont(FontId fontId, Character character);

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
  FontId FindDefaultFont(Character       charcode,
                         PointSize26Dot6 requestedPointSize = DEFAULT_POINT_SIZE,
                         bool            preferColor        = false);

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
  FontId FindFallbackFont(Character              charcode,
                          const FontDescription& preferredFontDescription,
                          PointSize26Dot6        requestedPointSize = DEFAULT_POINT_SIZE,
                          bool                   preferColor        = false);

  /**
   * @brief Retrieve the unique identifier for a font.
   *
   * @param[in] path The path to a font file.
   * @param[in] requestedPointSize The point size in 26.6 fractional points; the default point size is 12*64.
   * @param[in] faceIndex The index of the font face (optional).
   *
   * @return A valid font identifier, or zero if the font does not exist.
   */
  FontId GetFontId(const FontPath& path,
                   PointSize26Dot6 requestedPointSize = DEFAULT_POINT_SIZE,
                   FaceIndex       faceIndex          = 0);

  /**
   * @brief Retrieves a unique font identifier for a given description.
   *
   * @param[in] preferredFontDescription Description of the preferred font.
   *                                     The font will be the closest match to @p preferredFontDescription.
   * @param[in] requestedPointSize The point size in 26.6 fractional points; the default point size is 12*64.
   * @param[in] faceIndex The index of the font face (optional).
   * @param[in] variationsMapPtr The variations used in variable fonts (optional).
   *
   * @return A valid font identifier, or zero if no font is found.
   */
  FontId GetFontId(const FontDescription& preferredFontDescription,
                   PointSize26Dot6        requestedPointSize = DEFAULT_POINT_SIZE,
                   FaceIndex              faceIndex          = 0,
                   Property::Map*         variationsMapPtr   = nullptr);

  /**
   * @brief Retrieves a unique font identifier for a given bitmap font.
   * If the font is not present, it will cache the given font, and give it a new font id.
   *
   * @param[in] bitmapFont A bitmap font.
   *
   * @return A valid font identifier.
   */
  FontId GetFontId(const BitmapFont& bitmapFont);

  /**
   * @brief Check to see if a font is scalable.
   *
   * @param[in] path The path to a font file.
   * @return true if scalable.
   */
  bool IsScalable(const FontPath& path);

  /**
   * @brief Check to see if a font is scalable.
   *
   * @note It the font style is not empty, it will be used instead the font weight and font slant slant.
   *
   * @param[in] fontDescription A font description.
   *
   * @return true if scalable
   */
  bool IsScalable(const FontDescription& fontDescription);

  /**
   * @brief Get a list of sizes available for a fixed size font.
   *
   * @param[in] path The path to a font file.
   * @param[out] sizes A list of the available sizes, if no sizes available will return empty.
   */
  void GetFixedSizes(const FontPath& path, Dali::Vector<PointSize26Dot6>& sizes);

  /**
   * @brief Get a list of sizes available for a fixed size font.
   *
   * @note It the font style is not empty, it will be used instead the font weight and font slant slant.
   *
   * @param[in] fontDescription A font description.
   * @param[out] sizes A list of the available sizes, if no sizes available will return empty.
   */
  void GetFixedSizes(const FontDescription&         fontDescription,
                     Dali::Vector<PointSize26Dot6>& sizes);

  /**
   * @brief Whether the font has Italic style.
   *
   * @param[in] fontId The font identifier.
   *
   * @return true if the font has italic style.
   */
  bool HasItalicStyle(FontId fontId) const;

  ////////////////////////////////////////
  // Font metrics, glyphs and bitmaps.
  ////////////////////////////////////////

  /**
   * @brief Query the metrics for a font.
   *
   * @param[in] fontId The identifier of the font for the required glyph.
   * @param[out] metrics The font metrics.
   */
  void GetFontMetrics(FontId fontId, FontMetrics& metrics);

  /**
   * @brief Retrieve the glyph index for a UTF-32 character code.
   *
   * @param[in] fontId The identifier of the font for the required glyph.
   * @param[in] charcode The UTF-32 character code.
   *
   * @return The glyph index, or zero if the character code is undefined.
   */
  GlyphIndex GetGlyphIndex(FontId fontId, Character charcode);

  /**
   * @brief Return the glyph index of a given character code as modified by the variation selector.
   *
   * @param[in] fontId The identifier of the font for the required glyph.
   * @param[in] charcode The UTF-32 character code.
   * @param[in] variantSelector The UTF-32 character code point of the variation selector.
   *
   * @return The glyph index, or zero if the character code is undefined.
   */
  GlyphIndex GetGlyphIndex(FontId fontId, Character charcode, Character variantSelector);

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
  bool GetGlyphMetrics(GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal = true);

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
  void CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, GlyphBufferData& data, int outlineWidth);

  /**
   * @brief Create a bitmap representation of a glyph.
   *
   * @param[in] fontId The identifier of the font.
   * @param[in] glyphIndex The index of a glyph within the specified font.
   * @param[in] outlineWidth The width of the glyph outline in pixels.
   *
   * @return A valid PixelData, or an empty handle if the glyph could not be rendered.
   */
  PixelData CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth);

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
  void CreateVectorBlob(FontId        fontId,
                        GlyphIndex    glyphIndex,
                        VectorBlob*&  blob,
                        unsigned int& blobLength,
                        unsigned int& nominalWidth,
                        unsigned int& nominalHeight);

  /**
   * @brief Retrieves the ellipsis glyph for a requested point size.
   *
   * @param[in] requestedPointSize The requested point size.
   *
   * @return The ellipsis glyph.
   */
  const GlyphInfo& GetEllipsisGlyph(PointSize26Dot6 requestedPointSize);

  /**
   * @brief Whether the given glyph @p glyphIndex is a color glyph.
   *
   * @param[in] fontId The font id.
   * @param[in] glyphIndex The glyph index.
   *
   * @return @e true if the glyph is a color one.
   */
  bool IsColorGlyph(FontId fontId, GlyphIndex glyphIndex);

  /**
   * @brief Whether the given glyph font is a color.
   *
   * @param[in] fontId The font id.
   *
   * @return @e true if the font is a color one.
   */
  bool IsColorFont(FontId fontId);

  /**
   * @brief  Add custom fonts directory
   *
   * @param[in] path to the fonts directory
   *
   * @return true if the fonts can be added.
   */
  bool AddCustomFontDirectory(const FontPath& path);

  /**
   * @brief Gets the custom fonts directories.
   *
   * @return list of the custom font paths.
   */
  const FontPathList& GetCustomFontDirectories();

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
  GlyphIndex CreateEmbeddedItem(const EmbeddedItemDescription& description, Pixel::Format& pixelFormat);

  /**
   * @brief true to enable Atlas-Limitation.
   *
   * @note Used default configuration.
   * @param[in] enabled The on/off value to enable/disable Atlas-Limitation.
   */
  void EnableAtlasLimitation(bool enabled);

  /**
   * @brief Check Atlas-Limitation is enabled or disabled.
   *
   * @note Used default configuration.
   * return true if Atlas-Limitation is enabled, otherwise false.
   */
  bool IsAtlasLimitationEnabled() const;

  /**
   * @brief retrieve the maximum allowed width and height for text-atlas-block.
   *
   * @note Used default configuration.
   * return the maximum width and height of text-atlas-block.
   */
  Size GetMaximumTextAtlasSize() const;

  /**
   * @brief retrieve the default width and height for text-atlas-block.
   *
   * @note Used default configuration.
   * return the default width and height of text-atlas-block.
   */
  Size GetDefaultTextAtlasSize() const;

  /**
   * @brief retrieve the current maximum width and height for text-atlas-block.
   *
   * @note Used default configuration.
   * return the current maximum width and height of text-atlas-block.
   */
  Size GetCurrentMaximumBlockSizeFitInAtlas() const;

  /**
   * @brief set the achieved size (width and height) for text-atlas-block.
   * If @p currentMaximumBlockSizeFitInAtlas larger than the current maximum text atlas then store, otherwise ignore.
   *
   * @note Used default configuration.
   * return true if the current maximum text atlas size is changed, otherwise false.
   */
  bool SetCurrentMaximumBlockSizeFitInAtlas(const Size& currentMaximumBlockSizeFitInAtlas);

  /**
   * @brief retrieve the number of points to scale-up one unit of point-size.
   *
   * @note Used default configuration.
   * return the number of points per one unit of point-size
   */
  uint32_t GetNumberOfPointsPerOneUnitOfPointSize() const;

public: // Not intended for application developers
  /**
   * @brief This constructor is used by FontClient::Get().
   *
   * @param[in] fontClient  A pointer to the internal fontClient object.
   */
  explicit DALI_INTERNAL FontClient(Internal::FontClient* fontClient);
};

/**
 * @brief This is used to improve application launch performance
 *
 * @return A pre-initialized FontClient
 */
DALI_ADAPTOR_API FontClient FontClientPreInitialize();

/**
 * @brief This is used to pre-cache FontConfig in order to improve the runtime performance of the application.
 *
 * @param[in] fallbackFamilyList A list of fallback font families to be pre-cached.
 * @param[in] extraFamilyList A list of additional font families to be pre-cached.
 * @param[in] localeFamily A locale font family to be pre-cached.
 * @param[in] useThread True if the font client should create thread and perform pre-caching, false otherwise.
 * @param[in] syncCreation True if thread creation guarantees syncronization with the main thread, false async creation.
 */
DALI_ADAPTOR_API void FontClientPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool useThread, bool syncCreation);

/**
 * @brief This is used to pre-load FreeType font face in order to improve the runtime performance of the application.
 *
 * @param[in] fontPathList A list of font paths to be pre-loaded.
 * @param[in] memoryFontPathList A list of memory font paths to be pre-loaded.
 * @param[in] useThread True if the font client should create thread and perform font pre-loading, false otherwise.
 * @param[in] syncCreation True if thread creation guarantees syncronization with the main thread, false async creation.
 *
 * @note
 * The fonts in the fontPathList perform FT_New_Face during pre-loading,
 * which can provide some performace benefits.
 *
 * The fonts in the memoryFontPathList read the font file and cache the buffer in memory during pre-load.
 * This enables the use of FT_New_Memory_Face during runtime and provides a performance boost.
 * It requires memory equivalent to the size of each font file.
 */
DALI_ADAPTOR_API void FontClientFontPreLoad(const FontPathList& fontPathList, const FontPathList& memoryFontPathList, bool useThread, bool syncCreation);

/**
 * @brief Joins font threads, waiting for their execution to complete.
 */
DALI_ADAPTOR_API void FontClientJoinFontThreads();

/**
 * @brief Ensure the locale of the font client.
 * @note If there is no locale information, update it using setlocale().
 */
DALI_ADAPTOR_API void EnsureLocale();

/**
 * @brief Gets the current language.
 *
 * @note Returns the language code. (e.g., "en")
 * @return The current language.
 */
DALI_ADAPTOR_API const std::string& GetLocale();

/**
 * @brief Gets the current locale identifier.
 *
 * @note Returns the locale identifier. (e.g., "en_US")
 * @return The current locale identifier.
 */
DALI_ADAPTOR_API const std::string& GetLocaleFull();

/**
 * @brief Sets the current locale.
 *
 * @note Update language and locale identifier.
 * @param[in] locale The current locale.
 */
DALI_ADAPTOR_API void SetLocale(const std::string& locale);

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_FONT_CLIENT_H
