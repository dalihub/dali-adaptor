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
#include <dali/internal/text-abstraction/font-client-impl.h>

// INTERNAL INCLUDES
#include <singleton-service.h>
#include <dali/internal/text-abstraction/font-client-plugin-impl.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

FontClient::FontClient()
: mPlugin( NULL ),
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
                             requestedPointSize,
                             faceIndex );
}

FontId FontClient::GetFontId( const FontDescription& fontDescription,
                              PointSize26Dot6 requestedPointSize,
                              FaceIndex faceIndex )
{
  CreatePlugin();

  return mPlugin->GetFontId( fontDescription,
                             requestedPointSize,
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

void FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, Dali::TextAbstraction::FontClient::GlyphBufferData& data )
{
  CreatePlugin();

  mPlugin->CreateBitmap( fontId, glyphIndex, data );
}

PixelData FontClient::CreateBitmap( FontId fontId, GlyphIndex glyphIndex )
{
  CreatePlugin();

  return mPlugin->CreateBitmap( fontId, glyphIndex );
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
