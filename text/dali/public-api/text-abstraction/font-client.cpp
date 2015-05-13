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

// CLASS HEADER
#include <dali/public-api/text-abstraction/font-client.h>

// INTERNAL INCLUDES
#include <dali/internal/text-abstraction/font-client-impl.h>

namespace Dali
{

namespace TextAbstraction
{

const PointSize26Dot6 FontClient::DEFAULT_POINT_SIZE = 768u; // 12*64

FontClient FontClient::Get()
{
  return Internal::FontClient::Get();
}

FontClient::FontClient()
{
}

FontClient::~FontClient()
{
}

FontClient::FontClient( const FontClient& handle )
: BaseHandle( handle )
{
}

FontClient& FontClient::operator=( const FontClient& handle )
{
  BaseHandle::operator=( handle );
  return *this;
}

void FontClient::SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi  )
{
  GetImplementation(*this).SetDpi( horizontalDpi, verticalDpi );
}

void FontClient::GetDpi( unsigned int& horizontalDpi, unsigned int& verticalDpi )
{
  GetImplementation(*this).GetDpi( horizontalDpi, verticalDpi );
}

void FontClient::SetDefaultFontFamily( const std::string& fontFamilyName,
                                       const std::string& fontStyle )
{
  GetImplementation(*this).SetDefaultFontFamily( fontFamilyName, fontStyle );
}

void FontClient::GetDefaultFonts( FontList& defaultFonts )
{
  GetImplementation(*this).GetDefaultFonts( defaultFonts );
}

void FontClient::GetSystemFonts( FontList& systemFonts )
{
  GetImplementation(*this).GetSystemFonts( systemFonts );
}

void FontClient::GetDescription( FontId id, FontDescription& fontDescription )
{
  GetImplementation(*this).GetDescription( id, fontDescription );
}

PointSize26Dot6 FontClient::GetPointSize( FontId id )
{
  return GetImplementation(*this).GetPointSize( id );
}

FontId FontClient::FindDefaultFont( Character charcode, PointSize26Dot6 pointSize, bool preferColor )
{
  return GetImplementation(*this).FindDefaultFont( charcode, pointSize, preferColor );
}

FontId FontClient::GetFontId( const FontPath& path, PointSize26Dot6 pointSize, FaceIndex faceIndex )
{
  return GetImplementation(*this).GetFontId( path, pointSize, faceIndex );
}

FontId FontClient::GetFontId( const FontFamily& fontFamily,
                              const FontStyle& fontStyle,
                              PointSize26Dot6 pointSize,
                              FaceIndex faceIndex )
{
  return GetImplementation(*this).GetFontId( fontFamily,
                                             fontStyle,
                                             pointSize,
                                             faceIndex );
}

bool FontClient::IsScalable( const FontPath& path )
{
  return GetImplementation(*this).IsScalable( path );;
}

bool FontClient::IsScalable( const FontFamily& fontFamily, const FontStyle& style )
{
  return GetImplementation(*this).IsScalable( fontFamily, style );
}

void FontClient::GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes )
{
  GetImplementation(*this).GetFixedSizes( path, sizes );
}

void FontClient::GetFixedSizes( const FontFamily& fontFamily,
                                const FontStyle& style,
                                Dali::Vector< PointSize26Dot6 >& sizes )
{
  GetImplementation(*this).GetFixedSizes( fontFamily, style, sizes );
}

void FontClient::GetFontMetrics( FontId fontId, FontMetrics& metrics )
{
  GetImplementation(*this).GetFontMetrics( fontId, metrics );
}

GlyphIndex FontClient::GetGlyphIndex( FontId fontId, Character charcode )
{
  return GetImplementation(*this).GetGlyphIndex( fontId, charcode );
}

bool FontClient::GetGlyphMetrics( GlyphInfo* array, uint32_t size, bool horizontal )
{
  return GetImplementation(*this).GetGlyphMetrics( array, size, horizontal );
}

BufferImage FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex )
{
  return GetImplementation(*this).CreateBitmap( fontId, glyphIndex );
}

const GlyphInfo& FontClient::GetEllipsisGlyph( PointSize26Dot6 pointSize )
{
  return GetImplementation(*this).GetEllipsisGlyph( pointSize );
}

FontClient::FontClient( Internal::FontClient* internal )
: BaseHandle( internal )
{
}

} // namespace TextAbstraction

} // namespace Dali
