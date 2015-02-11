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
#include <dali/internal/text-abstraction/font-client-plugin-impl.h>

// INTERNAL INCLUDES
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/text-abstraction/glyph-info.h>
#include <dali/integration-api/debug.h>

/**
 * Conversion from Fractional26.6 to float
 */
namespace
{
const float FROM_266 = 1.0f / 64.0f;

const std::string FONT_FORMAT( "TrueType" );
const std::string DEFAULT_FONT_FAMILY_NAME( "Tizen" );
const std::string DEFAULT_FONT_STYLE( "Regular" );
}

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

FontClient::Plugin::CacheItem::CacheItem( FontId id,
                                          FT_Face ftFace,
                                          const std::string& path,
                                          PointSize26Dot6 pointSize,
                                          FaceIndex face,
                                          const FontMetrics& metrics )
: mFreeTypeFace( ftFace ),
  mPath( path ),
  mPointSize( pointSize ),
  mFaceIndex( face ),
  mMetrics( metrics )
{}

FontClient::Plugin::Plugin( unsigned int horizontalDpi,
                            unsigned int verticalDpi )
: mFreeTypeLibrary( NULL ),
  mDpiHorizontal( horizontalDpi ),
  mDpiVertical( verticalDpi )
{}

FontClient::Plugin::~Plugin()
{
  FT_Done_FreeType( mFreeTypeLibrary );
}

void FontClient::Plugin::Initialize()
{
  int error = FT_Init_FreeType( &mFreeTypeLibrary );
  if( FT_Err_Ok != error )
  {
    DALI_LOG_ERROR( "FreeType Init error: %d\n", error );
  }
}

void FontClient::Plugin::SetDpi( unsigned int horizontalDpi,
                                 unsigned int verticalDpi )
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical = verticalDpi;
}

void FontClient::Plugin::GetSystemFonts( FontList& systemFonts )
{
  if( mSystemFonts.empty() )
  {
    InitSystemFonts();
  }

  systemFonts = mSystemFonts;
}

void FontClient::Plugin::InitSystemFonts()
{
  FcFontSet* fontSet = GetFcFontSet();

  if( fontSet )
  {
    mSystemFonts.reserve( fontSet->nfont );

    for( int i = 0u; i < fontSet->nfont; ++i )
    {
      FcPattern* fontPattern = fontSet->fonts[i];

      std::string path;

      // Skip fonts with no path
      if( GetFcString( fontPattern, FC_FILE, path ) )
      {
        mSystemFonts.push_back( FontDescription() );
        FontDescription& fontDescription = mSystemFonts.back();

        fontDescription.path = path;

        GetFcString( fontPattern, FC_FAMILY, fontDescription.family );
        GetFcString( fontPattern, FC_STYLE, fontDescription.style );
      }
    }

    FcFontSetDestroy( fontSet );
  }
}

_FcFontSet* FontClient::Plugin::GetFcFontSet() const
{
  // create a new pattern.
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern* pattern = FcPatternCreate();

  // create an object set used to define which properties are to be returned in the patterns from FcFontList.
  FcObjectSet* objectSet = FcObjectSetCreate();

  // build an object set from a list of property names
  FcObjectSetAdd( objectSet, FC_FILE );
  FcObjectSetAdd( objectSet, FC_FAMILY );
  FcObjectSetAdd( objectSet, FC_STYLE );

  // get a list of fonts
  // creates patterns from those fonts containing only the objects in objectSet and returns the set of unique such patterns
  FcFontSet* fontset = FcFontList( NULL /* the default configuration is checked to be up to date, and used */, pattern, objectSet );

  // clear up the object set
  if( objectSet )
  {
    FcObjectSetDestroy( objectSet );
  }
  // clear up the pattern
  if( pattern )
  {
    FcPatternDestroy( pattern );
  }

  return fontset;
}

bool FontClient::Plugin::GetFcString( const FcPattern* pattern,
                                      const char* n,
                                      std::string& string )
{
  FcChar8* file = NULL;
  const FcResult retVal = FcPatternGetString( pattern, n, 0u, &file );

  if( FcResultMatch == retVal )
  {
    // Have to use reinterpret_cast because FcChar8 is unsigned char*, not a const char*.
    string.assign( reinterpret_cast<const char*>( file ) );

    return true;
  }

  return false;
}

void FontClient::Plugin::GetDescription( FontId id,
                                         FontDescription& fontDescription ) const
{
  // TODO
}

PointSize26Dot6 FontClient::Plugin::GetPointSize( FontId id )
{
  return 12*64;
}

FontId FontClient::Plugin::FindDefaultFont( Character charcode,
                                            PointSize26Dot6 pointSize )
{
  // TODO - Use FcCharSetHasChar()
  return FontId(0);
}

FontId FontClient::Plugin::GetFontId( const std::string& path,
                                      PointSize26Dot6 pointSize,
                                      FaceIndex faceIndex )
{
  FontId id( 0 );

  if( NULL != mFreeTypeLibrary )
  {
    FontId foundId(0);
    if( FindFont( path, pointSize, faceIndex, foundId ) )
    {
      id = foundId;
    }
    else
    {
      id = CreateFont( path, pointSize, faceIndex );
    }
  }

  return id;
}

FontId FontClient::Plugin::GetFontId( const FontFamily& fontFamily,
                                      const FontStyle& fontStyle,
                                      PointSize26Dot6 pointSize,
                                      FaceIndex faceIndex )
{
  return 0u;
}

GlyphIndex FontClient::Plugin::GetGlyphIndex( FontId fontId,
                                              Character charcode )
{
  GlyphIndex index( 0 );

  if( fontId > 0 &&
      fontId-1 < mFontCache.size() )
  {
    FT_Face ftFace = mFontCache[fontId-1].mFreeTypeFace;

    index = FT_Get_Char_Index( ftFace, charcode );
  }

  return index;
}

FontId FontClient::Plugin::CreateFont( const std::string& path,
                                       PointSize26Dot6 pointSize,
                                       FaceIndex faceIndex )
{
  FontId id( 0 );

  // Create & cache new font face
  FT_Face ftFace;
  int error = FT_New_Face( mFreeTypeLibrary,
                           path.c_str(),
                           0,
                           &ftFace );

  if( FT_Err_Ok == error )
  {
    error = FT_Set_Char_Size( ftFace,
                              0,
                              pointSize,
                              mDpiHorizontal,
                              mDpiVertical );

    if( FT_Err_Ok == error )
    {
      id = mFontCache.size() + 1;

      FT_Size_Metrics& ftMetrics = ftFace->size->metrics;

      FontMetrics metrics( static_cast< float >( ftMetrics.ascender  ) * FROM_266,
                           static_cast< float >( ftMetrics.descender ) * FROM_266,
                           static_cast< float >( ftMetrics.height    ) * FROM_266 );

      mFontCache.push_back( CacheItem( id, ftFace, path, pointSize, faceIndex, metrics ) );
    }
    else
    {
      DALI_LOG_ERROR( "FreeType Set_Char_Size error: %d for pointSize %d\n", pointSize );
    }
  }
  else
  {
    DALI_LOG_ERROR( "FreeType New_Face error: %d for %s\n", error, path.c_str() );
  }

  return id;
}

void FontClient::Plugin::GetFontMetrics( FontId fontId,
                                         FontMetrics& metrics )
{
  if( fontId > 0 &&
      fontId-1 < mFontCache.size() )
  {
    metrics = mFontCache[fontId-1].mMetrics;
  }
  else
  {
    DALI_LOG_ERROR( "Invalid font ID %d\n", fontId );
  }
}

bool FontClient::Plugin::GetGlyphMetrics( GlyphInfo* array,
                                          uint32_t size,
                                          bool horizontal )
{
  bool success( true );

  for( unsigned int i=0; i<size; ++i )
  {
    FontId fontId = array[i].fontId;

    if( fontId > 0 &&
        fontId-1 < mFontCache.size() )
    {
      FT_Face ftFace = mFontCache[fontId-1].mFreeTypeFace;

      int error = FT_Load_Glyph( ftFace, array[i].index, FT_LOAD_DEFAULT );

      if( FT_Err_Ok == error )
      {
        array[i].width  = static_cast< float >( ftFace->glyph->metrics.width ) * FROM_266;
        array[i].height = static_cast< float >( ftFace->glyph->metrics.height ) * FROM_266 ;
        if( horizontal )
        {
          array[i].xBearing = static_cast< float >( ftFace->glyph->metrics.horiBearingX ) * FROM_266;
          array[i].yBearing = static_cast< float >( ftFace->glyph->metrics.horiBearingY ) * FROM_266;
          array[i].advance  = static_cast< float >( ftFace->glyph->metrics.horiAdvance ) * FROM_266;
        }
        else
        {
          array[i].xBearing = static_cast< float >( ftFace->glyph->metrics.vertBearingX ) * FROM_266;
          array[i].yBearing = static_cast< float >( ftFace->glyph->metrics.vertBearingY ) * FROM_266;
          array[i].advance  = static_cast< float >( ftFace->glyph->metrics.vertAdvance ) * FROM_266;
        }
      }
      else
      {
        success = false;
      }
    }
    else
    {
      success = false;
    }
  }

  return success;
}

BitmapImage FontClient::Plugin::CreateBitmap( FontId fontId,
                                              GlyphIndex glyphIndex )
{
  BitmapImage bitmap;

  if( fontId > 0 &&
      fontId-1 < mFontCache.size() )
  {
    FT_Face ftFace = mFontCache[fontId-1].mFreeTypeFace;

    FT_Error error = FT_Load_Glyph( ftFace, glyphIndex, FT_LOAD_DEFAULT );
    if( FT_Err_Ok == error )
    {
      FT_Glyph glyph;
      error = FT_Get_Glyph( ftFace->glyph, &glyph );

      // Convert to bitmap if necessary
      if ( FT_Err_Ok == error )
      {
        if( glyph->format != FT_GLYPH_FORMAT_BITMAP )
        {
          error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
        }
        else
        {
          DALI_LOG_ERROR( "FT_Glyph_To_Bitmap Failed with error: %d\n", error );
        }
      }
      else
      {
        DALI_LOG_ERROR( "FT_Get_Glyph Failed with error: %d\n", error );
      }

      if( FT_Err_Ok == error )
      {
        // Access the underlying bitmap data
        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
        ConvertBitmap( bitmap, bitmapGlyph->bitmap );
      }

      // Created FT_Glyph object must be released with FT_Done_Glyph
      FT_Done_Glyph( glyph );
    }
    else
    {
      DALI_LOG_ERROR( "FT_Load_Glyph Failed with error: %d\n", error );
    }
  }

  return bitmap;
}

void FontClient::Plugin::ConvertBitmap( BitmapImage& destBitmap,
                                        FT_Bitmap srcBitmap )
{
  if( srcBitmap.width*srcBitmap.rows > 0 )
  {
    // TODO - Support all pixel modes
    if( FT_PIXEL_MODE_GRAY == srcBitmap.pixel_mode )
    {
      if( srcBitmap.pitch == srcBitmap.width )
      {
        destBitmap = BitmapImage::New( srcBitmap.width, srcBitmap.rows, Pixel::L8 );

        PixelBuffer* destBuffer = destBitmap.GetBuffer();
        memcpy( destBuffer, srcBitmap.buffer, srcBitmap.width*srcBitmap.rows );
      }
    }
  }
}

bool FontClient::Plugin::FindFont( const std::string& path,
                                   PointSize26Dot6 pointSize,
                                   FaceIndex faceIndex,
                                   FontId& found ) const
{
  for( unsigned int i=0; i<mFontCache.size(); ++i )
  {
    if( mFontCache[i].mPointSize == pointSize &&
        mFontCache[i].mPath == path &&
        mFontCache[i].mFaceIndex == faceIndex )
    {
      found = i + 1;
      return true;
    }
  }

  return false;
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
