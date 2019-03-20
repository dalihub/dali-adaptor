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

// CLASS HEADER
#include <dali/internal/text/text-abstraction/font-client-impl.h>

// EXTERNAL INCLUDES
#ifndef DALI_PROFILE_UBUNTU
#ifndef ANDROID
#include <vconf.h>
#endif
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/singleton-service.h>
#ifndef ANDROID
#include <dali/internal/text/text-abstraction/font-client-plugin-impl.h>
#endif

#include <dali/devel-api/text-abstraction/glyph-info.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

FontClient::FontClient()
: mPlugin( nullptr ),
  mDpiHorizontal( 0 ),
  mDpiVertical( 0 )
{
}

FontClient::~FontClient()
{
  delete mPlugin;
}

Dali::TextAbstraction::FontClient FontClient::Get()
{
  Dali::TextAbstraction::FontClient fontClientHandle;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::TextAbstraction::FontClient ) );
    if(handle)
    {
      // If so, downcast the handle
      FontClient* impl = dynamic_cast< Dali::TextAbstraction::Internal::FontClient* >( handle.GetObjectPtr() );
      fontClientHandle = Dali::TextAbstraction::FontClient( impl );
    }
    else // create and register the object
    {
      fontClientHandle = Dali::TextAbstraction::FontClient( new FontClient );
      service.Register( typeid( fontClientHandle ), fontClientHandle );
    }
  }

  return fontClientHandle;
}

void FontClient::ClearCache()
{
#ifndef ANDROID
  if( mPlugin )
  {
    mPlugin->ClearCache();
  }
#endif
}


void FontClient::SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi  )
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical = verticalDpi;
#ifndef ANDROID
  // Allow DPI to be set without loading plugin
  if( mPlugin )
  {
    mPlugin->SetDpi( horizontalDpi, verticalDpi  );
  }
#endif
}

void FontClient::GetDpi( unsigned int& horizontalDpi, unsigned int& verticalDpi )
{
  horizontalDpi = mDpiHorizontal;
  verticalDpi = mDpiVertical;
}

int FontClient::GetDefaultFontSize()
{
  int fontSize( -1 );

#ifndef DALI_PROFILE_UBUNTU
#ifndef ANDROID
  vconf_get_int( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &fontSize );
#endif
#endif // DALI_PROFILE_UBUNTU

  return fontSize;
}

void FontClient::ResetSystemDefaults()
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->ResetSystemDefaults();
#endif
}

void FontClient::GetDefaultFonts( FontList& defaultFonts )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetDefaultFonts( defaultFonts );
#endif
}

void FontClient::GetDefaultPlatformFontDescription( FontDescription& fontDescription )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetDefaultPlatformFontDescription( fontDescription );
#endif
}

void FontClient::GetDescription( FontId id, FontDescription& fontDescription )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetDescription( id, fontDescription );
#endif
}

PointSize26Dot6 FontClient::GetPointSize( FontId id )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetPointSize( id );
#else
  return 0;
#endif
}

bool FontClient::IsCharacterSupportedByFont( FontId fontId, Character character )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->IsCharacterSupportedByFont( fontId, character );
#else
  return false;
#endif
}

void FontClient::GetSystemFonts( FontList& systemFonts )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetSystemFonts( systemFonts );
#endif
}

FontId FontClient::FindDefaultFont( Character charcode,
                                    PointSize26Dot6 requestedPointSize,
                                    bool preferColor )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->FindDefaultFont( charcode,
                                   requestedPointSize,
                                   preferColor );
#else
  return 0;
#endif
}

FontId FontClient::FindFallbackFont( Character charcode,
                                     const FontDescription& preferredFontDescription,
                                     PointSize26Dot6 requestedPointSize,
                                     bool preferColor )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->FindFallbackFont( charcode,
                                    preferredFontDescription,
                                    requestedPointSize,
                                    preferColor );
#else
  return 0;
#endif
}

bool FontClient::IsScalable( const FontPath& path )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->IsScalable( path );
#else
  return false;
#endif
}

bool FontClient::IsScalable( const FontDescription& fontDescription )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->IsScalable( fontDescription );
#else
  return false;
#endif
}

void FontClient::GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetFixedSizes( path, sizes );
#endif
}

void FontClient::GetFixedSizes( const FontDescription& fontDescription,
                                Dali::Vector< PointSize26Dot6 >& sizes )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetFixedSizes( fontDescription, sizes );
#endif
}

bool FontClient::HasItalicStyle( FontId fontId ) const
{
  if( !mPlugin )
  {
    return false;
  }
#ifndef ANDROID
  return mPlugin->HasItalicStyle( fontId );
#else
  return false;
#endif
}

FontId FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetFontId( path,
                             requestedPointSize,
                             faceIndex,
                             true );
#else
  return 0;
#endif
}

FontId FontClient::GetFontId( const FontDescription& fontDescription,
                              PointSize26Dot6 requestedPointSize,
                              FaceIndex faceIndex )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetFontId( fontDescription,
                             requestedPointSize,
                             faceIndex );
#else
  return 0;
#endif
}

FontId FontClient::GetFontId( const BitmapFont& bitmapFont )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetFontId( bitmapFont );
#else
  return 0;
#endif
}

void FontClient::GetFontMetrics( FontId fontId, FontMetrics& metrics )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->GetFontMetrics( fontId, metrics );
#endif
}

GlyphIndex FontClient::GetGlyphIndex( FontId fontId, Character charcode )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetGlyphIndex( fontId, charcode );
#else
  return 0;
#endif
}

bool FontClient::GetGlyphMetrics( GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetGlyphMetrics( array, size, type, horizontal );
#else
  return false;
#endif
}

void FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->CreateBitmap( fontId, glyphIndex, isItalicRequired, isBoldRequired, data, outlineWidth );
#endif
}

PixelData FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->CreateBitmap( fontId, glyphIndex, outlineWidth );
#else
  return PixelData();
#endif
}

void FontClient::CreateVectorBlob( FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight )
{
#ifndef ANDROID
  CreatePlugin();

  mPlugin->CreateVectorBlob( fontId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight );
#endif
}

GlyphInfo glyphInfo;
const GlyphInfo& FontClient::GetEllipsisGlyph( PointSize26Dot6 requestedPointSize )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetEllipsisGlyph( requestedPointSize );
#else
  glyphInfo = GlyphInfo();
  return glyphInfo;
#endif
}

bool FontClient::IsColorGlyph( FontId fontId, GlyphIndex glyphIndex )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->IsColorGlyph( fontId, glyphIndex );
#else
  return false;
#endif
}

GlyphIndex FontClient::CreateEmbeddedItem(const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat)
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->CreateEmbeddedItem( description, pixelFormat );
#else
  return 0;
#endif
}

FT_FaceRec_* FontClient::GetFreetypeFace( FontId fontId )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetFreetypeFace( fontId );
#else
  return nullptr;
#endif
}

FontDescription::Type FontClient::GetFontType( FontId fontId )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->GetFontType( fontId );
#endif
}

bool FontClient::AddCustomFontDirectory( const FontPath& path )
{
#ifndef ANDROID
  CreatePlugin();

  return mPlugin->AddCustomFontDirectory( path );
#endif
}

void FontClient::CreatePlugin()
{
#ifndef ANDROID
  if( !mPlugin )
  {
    mPlugin = new Plugin( mDpiHorizontal, mDpiVertical );
  }
#endif
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
