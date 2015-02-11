#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H__

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

// INTERNAL INCLUDES
#include <dali/public-api/text-abstraction/font-metrics.h>
#include <dali/internal/text-abstraction/font-client-impl.h>

// EXTERNAL INCLUDES
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <fontconfig/fontconfig.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

struct FontClient::Plugin
{
  struct CacheItem
  {
    CacheItem( FontId id, FT_Face ftFace, const std::string& path, PointSize26Dot6 pointSize, FaceIndex face, const FontMetrics& metrics );

    FT_Face mFreeTypeFace;
    std::string mPath;
    PointSize26Dot6 mPointSize;
    FaceIndex mFaceIndex;
    FontMetrics mMetrics;
  };

  Plugin( unsigned int horizontalDpi, unsigned int verticalDpi );

  ~Plugin();

  void Initialize();

  void SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi );

  void GetSystemFonts( FontList& systemFonts );

  void InitSystemFonts();

  _FcFontSet* GetFcFontSet() const;

  bool GetFcString( const FcPattern* pattern, const char* n, std::string& string );

  void GetDescription( FontId id, FontDescription& fontDescription ) const;

  PointSize26Dot6 GetPointSize( FontId id );

  FontId FindDefaultFont( Character charcode, PointSize26Dot6 pointSize );

  FontId GetFontId( const std::string& path, PointSize26Dot6 pointSize, FaceIndex faceIndex );

  FontId GetFontId( const FontFamily& fontFamily,
                    const FontStyle& fontStyle,
                    PointSize26Dot6 pointSize,
                    FaceIndex faceIndex );

  GlyphIndex GetGlyphIndex( FontId fontId, Character charcode );

  FontId CreateFont( const std::string& path, PointSize26Dot6 pointSize, FaceIndex faceIndex );

  void GetFontMetrics( FontId fontId, FontMetrics& metrics );

  bool GetGlyphMetrics( GlyphInfo* array, uint32_t size, bool horizontal );

  BitmapImage CreateBitmap( FontId fontId, GlyphIndex glyphIndex );

  void ConvertBitmap( BitmapImage& destBitmap, FT_Bitmap srcBitmap );

private:

  bool FindFont( const std::string& path, PointSize26Dot6 pointSize, FaceIndex faceIndex, FontId& found ) const;

  FT_Library mFreeTypeLibrary;

  FontList mSystemFonts;

  std::vector<CacheItem> mFontCache;

  unsigned int mDpiHorizontal;
  unsigned int mDpiVertical;
};

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_PLUGIN_IMPL_H__
