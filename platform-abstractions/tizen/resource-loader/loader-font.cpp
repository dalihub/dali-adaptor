/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <cmath>

#include "loader-font.h"
#include <dali/public-api/images/distance-field.h>

namespace Dali
{
namespace TizenPlatform
{
using namespace Dali::Integration;

#if defined(DEBUG_ENABLED)
namespace
{
static Integration::Log::Filter* gLogFilter = Log::Filter::New(Log::General, false, "LOG_LOADER_FONT");
} // unnamed namespace
#endif

static TizenFace* LoadFontFile(const std::string &fileName, FT_Library freeType)
{
  TizenFace* tizenFace = NULL;

  // Open freetype font file
  FILE* fontFile = fopen(fileName.c_str(), "rb");
  if(fontFile != NULL)
  {
    // determine file size
    unsigned int fileSize( 0u );
    if( fseek(fontFile, 0, SEEK_END) )
    {
      DALI_LOG_ERROR("Error seeking to end of file\n");
    }
    else
    {
      long positionIndicator = ftell(fontFile);
      if( positionIndicator > -1L )
      {
        fileSize = static_cast<unsigned int>(positionIndicator);
      }
    }

    if( fileSize > 0u )
    {
      if( fseek(fontFile, 0, SEEK_SET) )
      {
        DALI_LOG_ERROR("Error seeking to start of file\n");
      }

      // allocate a buffer for the file (throws a memory exception on failure)
      tizenFace = new TizenFace;
      tizenFace->buffer = new FT_Byte[fileSize];

      // read the file
      unsigned int bytes = fread(tizenFace->buffer, 1, fileSize, fontFile);
      if (bytes != fileSize)
      {
        DALI_LOG_WARNING("Number of bytes read (%d) does not match the file's size (%d)\n", bytes, fileSize);
      }

      // create a freetype face from the memory buffer
      FT_Error retVal = FT_New_Memory_Face(freeType, tizenFace->buffer, fileSize, 0, &tizenFace->face);
      if (retVal != FT_Err_Ok)
      {
        DALI_LOG_WARNING("FT_New_Face failed on %s\n", fileName.c_str());
        delete tizenFace;
        tizenFace = NULL;
      }
    }

    // finished with file, so close the handle
    fclose(fontFile);
  }

  return tizenFace;
}

TizenFace* LoadFontFace(const std::string &fileName,
                      const PointSize pointSize,
                      const unsigned int dpiHor, const unsigned int dpiVer,
                      FT_Library freeType)
{
  TizenFace* tizenFace = LoadFontFile(fileName, freeType);

  if (NULL != tizenFace)
  {
    FT_Face face = tizenFace->face;

    // set the char size for glyph scaling
    FT_Error retVal = FT_Set_Char_Size(face, 0, static_cast<unsigned int>(roundf(pointSize.value))<<6, dpiHor, dpiVer);
    if (retVal != FT_Err_Ok)
    {
      DALI_LOG_WARNING("FT_Set_Char_Size failed on %s\n", fileName.c_str());
      delete tizenFace;
      tizenFace = NULL;
    }
    else
    {
      DALI_LOG_INFO (gLogFilter, Log::Verbose, "Loading font %s Glyphs(%d) Points(%f) DPI(%d %d) Units/EM(%d) PPEM(%d %d) Scale(%f %f)\n",
                     fileName.c_str(), face->num_glyphs,
                     roundf(pointSize.value), dpiHor, dpiVer, face->units_per_EM,
                     face->size->metrics.x_ppem, face->size->metrics.y_ppem,
                     face->size->metrics.x_scale / 65536.0f,
                     face->size->metrics.y_scale / 65536.0f);
    }
  }

  return tizenFace;
}


TizenFace* LoadFontFace(const std::string &fileName,
                      const PixelSize pixelSize,
                      FT_Library freeType)
{
  TizenFace* tizenFace = LoadFontFile(fileName, freeType);

  if (NULL != tizenFace)
  {
    FT_Face face = tizenFace->face;

    // set the pixel size for glyph scaling
    FT_Error retVal = FT_Set_Pixel_Sizes(face, pixelSize, pixelSize);
    if (retVal != FT_Err_Ok)
    {
      DALI_LOG_WARNING("FT_Set_Pixel_Sizes failed on %s\n", fileName.c_str());
      delete tizenFace;
      tizenFace = NULL;
    }
    else
    {
      DALI_LOG_INFO (gLogFilter, Log::Verbose, "Loading font %s Glyphs(%d) Pixels(%d) Units/EM(%d) PPEM(%d %d) Scale(%f %f)\n",
                     fileName.c_str(), face->num_glyphs,
                     pixelSize.value, face->units_per_EM,
                     face->size->metrics.x_ppem, face->size->metrics.y_ppem,
                     face->size->metrics.x_scale / 65536.0f,
                     face->size->metrics.y_scale / 65536.0f);
    }
  }

  return tizenFace;
}

// charcode must be UTF-32 representation
FT_Glyph GetGlyph(FT_Face face, FT_ULong charcode, FT_Int32 load_flags)
{
  if (face == NULL)
  {
    DALI_LOG_WARNING("face == NULL\n");
    return NULL;
  }

  // FT_Load_Char is always succeeds providing the face is valid. So if the character code is not
  // found it will just return a default glyph
  FT_Error retVal = FT_Load_Char(face, charcode , load_flags);
  if(retVal != FT_Err_Ok)
  {
    DALI_LOG_WARNING("FT_Load_Char failed %d\n", retVal);
    return NULL;
  }

  // get the glyph
  FT_Glyph glyph;
  retVal = FT_Get_Glyph(face->glyph, &glyph);
  if(retVal != FT_Err_Ok)
  {
    DALI_LOG_WARNING("FT_Get_Glyph failed %d\n", retVal);
    return NULL;
  }

  return glyph;
}

// use getBitmap = false to find out glyph x-advance without copying/rendering bitmap
GlyphSet::Character* GetCharacter(FT_Face face, FT_ULong charcode,
                                  const unsigned int fieldSize, const unsigned int fieldPadding,
                                  const Vector2& maxGlyphCell,
                                  const bool renderBitmap, const bool highQuality)
{
  FT_Glyph glyph = GetGlyph(face, charcode, FT_LOAD_DEFAULT|FT_LOAD_NO_AUTOHINT);
  if(glyph == NULL)
  {
    // early out, if glyph is NULL
    return NULL;
  }

  // scale factor for unit scaled glyphs
  const float xScale = 1.0f / (face->size->metrics.x_scale / 65536.0f);
  const float yScale = 1.0f / (face->size->metrics.y_scale / 65536.0f);

  // create a new glyph-metric for the letter
  GlyphMetrics glyphMetrics;
  glyphMetrics.code     = charcode;
  glyphMetrics.quality  = (highQuality ? 1 : 0);
  glyphMetrics.xPosition = 0;
  glyphMetrics.yPosition = 0;

  // Set correct glyph size for underline
  const uint32_t UNDERLINE_CHARACTER( 0x0332 );
  if(charcode == UNDERLINE_CHARACTER)
  {
    float underlineBitmapWidth(64.f);
    float underlineBitmapHeight( 8.f );
    glyphMetrics.left     = 0.f;
    glyphMetrics.top      = 0.f;
    glyphMetrics.width    = xScale * underlineBitmapWidth;
    glyphMetrics.height   = yScale * underlineBitmapHeight;
    glyphMetrics.xAdvance = glyphMetrics.width;
  }
  else
  {
    // Regular glyphs
    glyphMetrics.left     = xScale * face->glyph->metrics.horiBearingX / 64.0f;
    glyphMetrics.top      = yScale * face->glyph->metrics.horiBearingY / 64.0f;
    glyphMetrics.width    = xScale * face->glyph->metrics.width / 64.0f;
    glyphMetrics.height   = yScale * face->glyph->metrics.height / 64.0f;
    glyphMetrics.xAdvance = xScale * face->glyph->metrics.horiAdvance / 64.0f;
  }

  // TODO: Look at generating the distance field directly from the glyph vector outline
  //       instead of FreeType's scaled bitmap
  BitmapPtr bitmapData = NULL;
  // bitmap required?
  if (renderBitmap)
  {
    // convert glyph to bitmap
    if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
    {
      FT_Error retVal = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
      if(retVal != FT_Err_Ok)
      {
        DALI_LOG_WARNING("FT_Glyph_To_Bitmap failed %d\n", retVal);
        FT_Done_Glyph(glyph);
        return NULL;
      }
    }
    // cast the FT_Glyph to a FT_BitmapGlyph
    FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph) glyph;

    // access the underlying bitmap data
    FT_Bitmap bitmap = bitmapGlyph->bitmap;

    DALI_LOG_INFO(gLogFilter, Log::Verbose, "%s(%c %f %f %f %f %f %f) [%.2f %.2f %.2f %.2f %f %f]\n",
                  __FUNCTION__, (int)charcode,
                  glyphMetrics.left, glyphMetrics.top, glyphMetrics.width, glyphMetrics.height,
                  glyphMetrics.xAdvance, face->height/64.0f,
                  xScale * bitmapGlyph->left, yScale * bitmapGlyph->top, xScale * bitmap.width, yScale * bitmap.rows,
                  glyph->advance.x / 65536.0f * xScale, face->ascender / 64.0f);

    // create a new bitmap for the glyph
    if(charcode == UNDERLINE_CHARACTER)
    {
      float underlineBitmapWidth( glyphMetrics.width / xScale );
      float underlineBitmapHeight( glyphMetrics.height / yScale );

      const unsigned int underlineBitmapSize(underlineBitmapWidth*underlineBitmapHeight);
      std::vector<unsigned char> underlineAlphaMap(underlineBitmapSize);
      std::fill(underlineAlphaMap.begin(), underlineAlphaMap.end(), 0xff);

      bitmapData = Integration::Bitmap::New(Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD);
      bitmapData->GetPackedPixelsProfile()->ReserveBuffer(Pixel::A8, fieldSize, fieldSize);
      GenerateDistanceFieldMap( &(*underlineAlphaMap.begin()), Vector2(underlineBitmapWidth, underlineBitmapHeight),
                               bitmapData->GetBuffer(), Vector2(fieldSize, fieldSize),
                               fieldPadding, Vector2( maxGlyphCell.width / xScale, maxGlyphCell.height / yScale ),
                               highQuality );
    }
    else
    {
      if (0 != (bitmap.width * bitmap.rows))
      {
        bitmapData = Integration::Bitmap::New(Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD);
        bitmapData->GetPackedPixelsProfile()->ReserveBuffer(Pixel::A8, fieldSize, fieldSize);

        GenerateDistanceFieldMap(bitmap.buffer, Vector2(bitmap.width, bitmap.rows),
            bitmapData->GetBuffer(), Vector2(fieldSize, fieldSize),
            fieldPadding, Vector2( maxGlyphCell.width / xScale, maxGlyphCell.height / yScale ),
            highQuality );
      }
      else
      {
        // FreeType has rendered a zero width / height bitmap.
        // This will only happen with a few characters like no break space (nbsp)
        // @todo filter out known, non-visible characters in Dali-core.
        // However we will still need this code for characters like OGHAM SPACE MARK
        // which will be blank with some fonts, and visible with others.
        // Create a dummy bitmap of size 64,64
        bitmapData = Integration::Bitmap::New(Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD);
        bitmapData->GetPackedPixelsProfile()->ReserveBuffer(Pixel::A8, fieldSize, fieldSize);
        PixelBuffer* pixelBuffer = bitmapData->GetBuffer();
        memset( pixelBuffer, 0x0, fieldSize * fieldSize );
      }
    }
  }

  // finished with glyph, so, release it
  FT_Done_Glyph(glyph);

  return new GlyphSet::Character(bitmapData, glyphMetrics);
}

Integration::BitmapPtr GetGlyphBitmap( FT_Face face, FT_ULong charCode )
{
  Integration::BitmapPtr image;

  FT_Glyph ftGlyph = GetGlyph( face, charCode, FT_LOAD_RENDER );
  if( NULL != ftGlyph )
  {
    FT_Error ftError = FT_Err_Ok;

    // convert glyph to bitmap
    if( ftGlyph->format != FT_GLYPH_FORMAT_BITMAP )
    {
      ftError = FT_Glyph_To_Bitmap( &ftGlyph, FT_RENDER_MODE_NORMAL, 0, 1 );

      if( ftError != FT_Err_Ok)
      {
        DALI_LOG_WARNING( "FT_Glyph_To_Bitmap failed %d\n", ftError );
        FT_Done_Glyph( ftGlyph );
        return image;
      }
    }

    // cast the FT_Glyph to a FT_BitmapGlyph
    FT_BitmapGlyph ftBitmapGlyph = (FT_BitmapGlyph)ftGlyph;

    // access the underlying bitmap data
    FT_Bitmap ftBitmap = ftBitmapGlyph->bitmap;

    const std::size_t size = ftBitmap.width * ftBitmap.rows;
    if( 0 < size )
    {
      image = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD );
      image->GetPackedPixelsProfile()->ReserveBuffer( Pixel::A8, ftBitmap.width, ftBitmap.rows );

      memcpy( static_cast<unsigned char*>( image->GetBuffer() ), ftBitmap.buffer, size );
    }

    // finished with glyph, so, release it
    FT_Done_Glyph( ftGlyph );
  }

  return image;
}

} // namespace TizenPlatform

} // namespace Dali
