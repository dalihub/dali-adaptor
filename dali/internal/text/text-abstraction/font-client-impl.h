#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>

struct FT_FaceRec_;

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
using HarfBuzzFontHandle = void*; ///< @note We don't want to make other class include harfbuzz header. So we will keep harfbuzz font data as HarfBuzzFontHandle.

/**
 * Implementation of the FontClient
 */
class FontClient : public BaseObject
{
public:
  /**
   * Constructor
   */
  FontClient();

  /**
   * Destructor
   */
  ~FontClient();

public: // API for Dali::TextAbstraction::FontClient used.
  /**
   * @copydoc Dali::TextAbstraction::FontClient::Get()
   */
  static Dali::TextAbstraction::FontClient Get();

  /**
   * @brief This is used to improve application launch performance
   *
   * @return A pre-initialized FontClient
   */
  static Dali::TextAbstraction::FontClient PreInitialize();

  /**
   * @brief This is used to pre-cache fonts in order to improve the runtime performance of the application.
   *
   * @see Dali::TextAbstraction::FontClientPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool useThread);
   */
  static void PreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool useUiThread);

  /**
   * @brief This is used to creates a global font client and pre-caches the fonts.
   */
  static void PreCacheRun(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::ClearCache()
   */
  void ClearCache();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDpi()
   */
  void SetDpi(unsigned int horizontalDpi, unsigned int verticalDpi);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDpi()
   */
  void GetDpi(unsigned int& horizontalDpi, unsigned int& verticalDpi);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultFontSize()
   */
  int GetDefaultFontSize();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::ResetSystemDefaults()
   */
  void ResetSystemDefaults();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultFonts()
   */
  void GetDefaultFonts(FontList& defaultFonts);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultPlatformFontDescription()
   */
  void GetDefaultPlatformFontDescription(FontDescription& fontDescription);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetSystemFonts()
   */
  void GetSystemFonts(FontList& systemFonts);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDescription()
   */
  void GetDescription(FontId fontId, FontDescription& fontDescription);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetPointSize()
   */
  PointSize26Dot6 GetPointSize(FontId fontId);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsCharacterSupportedByFont()
   */
  bool IsCharacterSupportedByFont(FontId fontId, Character character);

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
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   */
  FontId GetFontId(const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex);

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontDescription& fontDescription, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
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
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes )
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
   * @copydoc Dali::TextAbstraction::FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::GlyphBufferData& data, int outlineWidth )
   */
  void CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::GlyphBufferData& data, int outlineWidth);

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
   * @copydoc Dali::TextAbstraction::FontClient::AddCustomFontDirectory()
   */
  bool AddCustomFontDirectory(const FontPath& path);

public: // API for Dali::TextAbstraction::Internal::FontClient used.
  /**
   * @brief Retrieves the pointer to the FreeType Font Face for the given @p fontId.
   *
   * @param[in] fontId The font id.
   *
   * @return The pointer to the FreeType Font Face.
   */
  FT_FaceRec_* GetFreetypeFace(FontId fontId);

  /**
   * @brief Retrieves the type of font.
   *
   * @param[in] fontId The font id.
   *
   * @return FACE_FONT if the font has been loaded by FreeType, BITMAP_FONT if it's a font that has been loaded from images or INVALID if it's a non valid font.
   */
  FontDescription::Type GetFontType(FontId fontId);

  /**
   * @brief Get the harfbuzz font data of font.
   *
   * @param fontId The font id.
   * @return The harfbuzz font data, or nullptr if failed.
   */
  HarfBuzzFontHandle GetHarfBuzzFont(FontId fontId);

  /**
   * @brief This is used to pre-cache fonts in order to improve the runtime performance of the application.
   *
   * @param[in] fallbackFamilyList A list of fallback font families to be pre-cached.
   * @param[in] extraFamilyList A list of additional font families to be pre-cached.
   * @param[in] localeFamily A locale font family to be pre-cached.
   */
  void FontPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily);

private:
  /**
   * Helper for lazy initialization.
   */
  void CreatePlugin();

  // Undefined copy constructor.
  FontClient(const FontClient&);

  // Undefined assignment constructor.
  FontClient& operator=(const FontClient&);

private:
  struct Plugin;
  Plugin* mPlugin;

  // Allows DPI to be set without loading plugin
  unsigned int mDpiHorizontal;
  unsigned int mDpiVertical;

  static Dali::TextAbstraction::FontClient gPreInitializedFontClient;
  static Dali::TextAbstraction::FontClient gPreCachedFontClient;

}; // class FontClient

} // namespace Internal

inline static Internal::FontClient& GetImplementation(FontClient& fontClient)
{
  DALI_ASSERT_ALWAYS(fontClient && "fontClient handle is empty");
  BaseObject& handle = fontClient.GetBaseObject();
  return static_cast<Internal::FontClient&>(handle);
}

inline static const Internal::FontClient& GetImplementation(const FontClient& fontClient)
{
  DALI_ASSERT_ALWAYS(fontClient && "fontClient handle is empty");
  const BaseObject& handle = fontClient.GetBaseObject();
  return static_cast<const Internal::FontClient&>(handle);
}

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H
