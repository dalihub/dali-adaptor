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

void FontClient::GetSystemFonts( FontList& systemFonts )
{
  GetImplementation(*this).GetSystemFonts( systemFonts );
}

bool FontClient::FindSystemFont( Character charcode, FontDescription& systemFont )
{
  return GetImplementation(*this).FindSystemFont( charcode, systemFont );
}

FontId FontClient::GetFontId( const FontPath& path, PointSize26Dot6 pointSize, FaceIndex faceIndex )
{
  return GetImplementation(*this).GetFontId( path, pointSize, faceIndex );
}

FontId FontClient::FindDefaultFont( Character charcode )
{
  return GetImplementation(*this).FindDefaultFont( charcode );
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

BitmapImage FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex )
{
  return GetImplementation(*this).CreateBitmap( fontId, glyphIndex );
}

FontClient::FontClient( Internal::FontClient* internal )
: BaseHandle( internal )
{
}

} // namespace TextAbstraction

} // namespace Dali
