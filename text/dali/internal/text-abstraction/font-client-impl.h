#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/text-abstraction/font-client.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

/**
 * Implementation of the FontClient
 */
class FontClient : public Dali::BaseObject
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
   * @copydoc Dali::FontClient::Get()
   */
  static Dali::TextAbstraction::FontClient Get();

  /**
   * @copydoc Dali::FontClient::SetDpi()
   */
  void SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi );

  /**
   * @copydoc Dali::FontClient::GetSystemFonts()
   */
  void GetSystemFonts( FontList& systemFonts );

  /**
   * @copydoc Dali::FontClient::FindSystemFont()
   */
  bool FindSystemFont( Character charcode, FontDescription& systemFont );

  /**
   * @copydoc Dali::FontClient::GetFontId()
   */
  FontId GetFontId( const FontPath& path, PointSize26Dot6 pointSize, FaceIndex faceIndex );

  /**
   * @copydoc Dali::FontClient::FindDefaultFont()
   */
  FontId FindDefaultFont( Character charcode );

  /**
   * @copydoc Dali::FontClient::GetFontMetrics()
   */
  void GetFontMetrics( FontId fontId, FontMetrics& metrics );

  /**
   * @copydoc Dali::FontClient::GetGlyphIndex()
   */
  GlyphIndex GetGlyphIndex( FontId fontId, Character charcode );

  /**
   * @copydoc Dali::FontClient::CreateMetrics()
   */
  bool GetGlyphMetrics( GlyphInfo* array, uint32_t size, bool horizontal );

  /**
   * @copydoc Dali::FontClient::CreateBitmap()
   */
  BitmapImage CreateBitmap( FontId fontId, GlyphIndex glyphIndex );

private:

  /**
   * Helper for lazy initialization.
   */
  void CreatePlugin();

  // Undefined copy constructor.
  FontClient( const FontClient& );

  // Undefined assignment constructor.
  FontClient& operator=( FontClient& );

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

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H__
