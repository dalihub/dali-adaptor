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

// CLASS HEADER
#include <dali/devel-api/text-abstraction/font-client.h>

// INTERNAL INCLUDES
#include <dali/internal/text/text-abstraction/font-client-impl.h>

namespace Dali
{

namespace TextAbstraction
{

const PointSize26Dot6 FontClient::DEFAULT_POINT_SIZE = 768u; // 12*64

FontClient::GlyphBufferData::GlyphBufferData()
: buffer( nullptr ),
  width( 0u ),
  height( 0u ),
  format( Pixel::A8 )
{
}

FontClient::GlyphBufferData::~GlyphBufferData()
{
}

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

int FontClient::GetDefaultFontSize()
{
  return GetImplementation(*this).GetDefaultFontSize();
}

void FontClient::ResetSystemDefaults()
{
  GetImplementation(*this).ResetSystemDefaults();
}

void FontClient::GetDefaultFonts( FontList& defaultFonts )
{
  GetImplementation(*this).GetDefaultFonts( defaultFonts );
}

void FontClient::GetDefaultPlatformFontDescription( FontDescription& fontDescription )
{
  GetImplementation(*this).GetDefaultPlatformFontDescription( fontDescription );
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

bool FontClient::IsCharacterSupportedByFont( FontId fontId, Character character )
{
  return GetImplementation(*this).IsCharacterSupportedByFont( fontId, character );
}

FontId FontClient::FindDefaultFont( Character charcode,
                                    PointSize26Dot6 requestedPointSize,
                                    bool preferColor )
{
  return GetImplementation(*this).FindDefaultFont( charcode,
                                                   requestedPointSize,
                                                   preferColor );
}

FontId FontClient::FindFallbackFont( Character charcode,
                                     const FontDescription& preferredFontDescription,
                                     PointSize26Dot6 requestedPointSize,
                                     bool preferColor )
{
  return GetImplementation(*this).FindFallbackFont( charcode, preferredFontDescription, requestedPointSize, preferColor );
}

FontId FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
{
  return GetImplementation(*this).GetFontId( path, requestedPointSize, faceIndex );
}

FontId FontClient::GetFontId( const FontDescription& fontDescription,
                              PointSize26Dot6 requestedPointSize,
                              FaceIndex faceIndex )
{
  return GetImplementation(*this).GetFontId( fontDescription,
                                             requestedPointSize,
                                             faceIndex );
}

bool FontClient::IsScalable( const FontPath& path )
{
  return GetImplementation(*this).IsScalable( path );
}

bool FontClient::IsScalable( const FontDescription& fontDescription )
{
  return GetImplementation(*this).IsScalable( fontDescription );
}

void FontClient::GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes )
{
  GetImplementation(*this).GetFixedSizes( path, sizes );
}

void FontClient::GetFixedSizes( const FontDescription& fontDescription,
                                Dali::Vector< PointSize26Dot6 >& sizes )
{
  GetImplementation(*this).GetFixedSizes( fontDescription, sizes );
}

void FontClient::GetFontMetrics( FontId fontId, FontMetrics& metrics )
{
  GetImplementation(*this).GetFontMetrics( fontId, metrics );
}

GlyphIndex FontClient::GetGlyphIndex( FontId fontId, Character charcode )
{
  return GetImplementation(*this).GetGlyphIndex( fontId, charcode );
}

bool FontClient::GetGlyphMetrics( GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal )
{
  return GetImplementation(*this).GetGlyphMetrics( array, size, type, horizontal );
}

void FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool softwareItalic, bool softwareBold, GlyphBufferData& data, int outlineWidth )
{
  GetImplementation(*this).CreateBitmap( fontId, glyphIndex, softwareItalic, softwareBold, data, outlineWidth );
}

PixelData FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
{
  return GetImplementation(*this).CreateBitmap( fontId, glyphIndex, outlineWidth );
}

void FontClient::CreateVectorBlob( FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight )
{
  GetImplementation(*this).CreateVectorBlob( fontId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight );
}

const GlyphInfo& FontClient::GetEllipsisGlyph( PointSize26Dot6 requestedPointSize )
{
  return GetImplementation(*this).GetEllipsisGlyph( requestedPointSize );
}

bool FontClient::IsColorGlyph( FontId fontId, GlyphIndex glyphIndex )
{
  return GetImplementation(*this).IsColorGlyph( fontId, glyphIndex );
}

FontClient::FontClient( Internal::FontClient* internal )
: BaseHandle( internal )
{
}

} // namespace TextAbstraction

} // namespace Dali
