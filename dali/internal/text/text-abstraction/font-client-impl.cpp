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
#include <dali/internal/text/text-abstraction/font-client-impl.h>

// EXTERNAL INCLUDES
#ifndef DALI_PROFILE_UBUNTU
#include <vconf.h>
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/singleton-service.h>
#include <dali/internal/text/text-abstraction/font-client-plugin-impl.h>

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

void FontClient::SetDpi( unsigned int horizontalDpi, unsigned int verticalDpi  )
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical = verticalDpi;

  // Allow DPI to be set without loading plugin
  if( mPlugin )
  {
    mPlugin->SetDpi( horizontalDpi, verticalDpi  );
  }
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
  vconf_get_int( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &fontSize );
#endif // DALI_PROFILE_UBUNTU

  return fontSize;
}

void FontClient::ResetSystemDefaults()
{
  CreatePlugin();

  mPlugin->ResetSystemDefaults();
}

void FontClient::GetDefaultFonts( FontList& defaultFonts )
{
  CreatePlugin();

  mPlugin->GetDefaultFonts( defaultFonts );
}

void FontClient::GetDefaultPlatformFontDescription( FontDescription& fontDescription )
{
  CreatePlugin();

  mPlugin->GetDefaultPlatformFontDescription( fontDescription );
}

void FontClient::GetDescription( FontId id, FontDescription& fontDescription )
{
  CreatePlugin();

  mPlugin->GetDescription( id, fontDescription );
}

PointSize26Dot6 FontClient::GetPointSize( FontId id )
{
  CreatePlugin();

  return mPlugin->GetPointSize( id );
}

bool FontClient::IsCharacterSupportedByFont( FontId fontId, Character character )
{
  CreatePlugin();

  return mPlugin->IsCharacterSupportedByFont( fontId, character );
}

void FontClient::GetSystemFonts( FontList& systemFonts )
{
  CreatePlugin();

  mPlugin->GetSystemFonts( systemFonts );
}

FontId FontClient::FindDefaultFont( Character charcode,
                                    PointSize26Dot6 requestedPointSize,
                                    bool preferColor )
{
  CreatePlugin();

  return mPlugin->FindDefaultFont( charcode,
                                   requestedPointSize,
                                   preferColor );
}

FontId FontClient::FindFallbackFont( Character charcode,
                                     const FontDescription& preferredFontDescription,
                                     PointSize26Dot6 requestedPointSize,
                                     bool preferColor )
{
  CreatePlugin();

  return mPlugin->FindFallbackFont( charcode,
                                    preferredFontDescription,
                                    requestedPointSize,
                                    preferColor );
}

bool FontClient::IsScalable( const FontPath& path )
{
  CreatePlugin();

  return mPlugin->IsScalable( path );
}

bool FontClient::IsScalable( const FontDescription& fontDescription )
{
  CreatePlugin();

  return mPlugin->IsScalable( fontDescription );
}

void FontClient::GetFixedSizes( const FontPath& path, Dali::Vector< PointSize26Dot6>& sizes )
{
  CreatePlugin();

  mPlugin->GetFixedSizes( path, sizes );
}

void FontClient::GetFixedSizes( const FontDescription& fontDescription,
                                Dali::Vector< PointSize26Dot6 >& sizes )
{
  CreatePlugin();

  mPlugin->GetFixedSizes( fontDescription, sizes );
}

FontId FontClient::GetFontId( const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex )
{
  CreatePlugin();

  return mPlugin->GetFontId( path,
                             requestedPointSize,
                             faceIndex,
                             true );
}

FontId FontClient::GetFontId( const FontDescription& fontDescription,
                              PointSize26Dot6 requestedPointSize,
                              FaceIndex faceIndex )
{
  CreatePlugin();

  return mPlugin->GetFontId( fontDescription,
                             requestedPointSize,
                             faceIndex );
}

void FontClient::GetFontMetrics( FontId fontId, FontMetrics& metrics )
{
  CreatePlugin();

  return mPlugin->GetFontMetrics( fontId, metrics );
}

GlyphIndex FontClient::GetGlyphIndex( FontId fontId, Character charcode )
{
  CreatePlugin();

  return mPlugin->GetGlyphIndex( fontId, charcode );
}

bool FontClient::GetGlyphMetrics( GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal )
{
  CreatePlugin();

  return mPlugin->GetGlyphMetrics( array, size, type, horizontal );
}

void FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool softwareItalic, bool softwareBold, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth )
{
  CreatePlugin();

  mPlugin->CreateBitmap( fontId, glyphIndex, softwareItalic, softwareBold, data, outlineWidth );
}

PixelData FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
{
  CreatePlugin();

  return mPlugin->CreateBitmap( fontId, glyphIndex, outlineWidth );
}

void FontClient::CreateVectorBlob( FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight )
{
  CreatePlugin();

  return mPlugin->CreateVectorBlob( fontId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight );
}

const GlyphInfo& FontClient::GetEllipsisGlyph( PointSize26Dot6 requestedPointSize )
{
  CreatePlugin();

  return mPlugin->GetEllipsisGlyph( requestedPointSize );
}

bool FontClient::IsColorGlyph( FontId fontId, GlyphIndex glyphIndex )
{
  CreatePlugin();

  return mPlugin->IsColorGlyph( fontId, glyphIndex );
}

FT_FaceRec_* FontClient::GetFreetypeFace( FontId fontId )
{
  CreatePlugin();

  return mPlugin->GetFreetypeFace( fontId );
}

void FontClient::CreatePlugin()
{
  if( !mPlugin )
  {
    mPlugin = new Plugin( mDpiHorizontal, mDpiVertical );
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
