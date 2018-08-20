#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

  /**
   * @copydoc Dali::TextAbstraction::FontClient::Get()
   */
  static Dali::TextAbstraction::FontClient Get();

  /**
   * @copydoc Dali::TextAbstraction::FontClient::SetDpi()
   */
  void SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDpi()
   */
  void GetDpi( unsigned int& horizontalDpi, unsigned int& verticalDpi );

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
  void GetDefaultFonts( FontList& defaultFonts );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDefaultPlatformFontDescription()
   */
  void GetDefaultPlatformFontDescription( FontDescription& fontDescription );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetSystemFonts()
   */
  void GetSystemFonts( FontList& systemFonts );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetDescription()
   */
  void GetDescription( FontId id, FontDescription& fontDescription );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetPointSize()
   */
  PointSize26Dot6 GetPointSize( FontId id );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::IsCharacterSupportedByFont()
   */
  bool IsCharacterSupportedByFont( FontId fontId, Character character );

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
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
   */
  FontId GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::GetFontId( const FontDescription& fontDescription, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
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
   * @copydoc Dali::TextAbstraction::FontClient::GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes )
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
   * @brief Retrieves the pointer to the FreeType Font Face for the given @p fontId.
   *
   * @param[in] fontId The font id.
   *
   * @return The pointer to the FreeType Font Face.
   */
  FT_FaceRec_* GetFreetypeFace( FontId fontId );

  /**
   * @copydoc Dali::TextAbstraction::FontClient::AddCustomFontDirectory()
   */
  bool AddCustomFontDirectory( const FontPath& path );

private:

  /**
   * Helper for lazy initialization.
   */
  void CreatePlugin();

  // Undefined copy constructor.
  FontClient( const FontClient& );

  // Undefined assignment constructor.
  FontClient& operator=( const FontClient& );

private:

  struct Plugin;
  Plugin* mPlugin;

  // Allows DPI to be set without loading plugin
  unsigned int mDpiHorizontal;
  unsigned int mDpiVertical;

}; // class FontClient

} // namespace Internal

inline static Internal::FontClient& GetImplementation(FontClient& fontClient)
{
  DALI_ASSERT_ALWAYS( fontClient && "fontClient handle is empty" );
  BaseObject& handle = fontClient.GetBaseObject();
  return static_cast<Internal::FontClient&>(handle);
}

inline static const Internal::FontClient& GetImplementation(const FontClient& fontClient)
{
  DALI_ASSERT_ALWAYS( fontClient && "fontClient handle is empty" );
  const BaseObject& handle = fontClient.GetBaseObject();
  return static_cast<const Internal::FontClient&>(handle);
}

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H
