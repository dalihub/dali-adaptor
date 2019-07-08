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

// FILE HEADER
#include <dali/internal/text/text-abstraction/cairo-renderer.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/constants.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <cstring>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/text-abstraction/text-renderer-layout-helper.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/text/text-abstraction/font-client-impl.h>

namespace
{

const float TO_FLOAT = 1.f / 255.f;
const float TO_UCHAR = 255.f;
const float TWO_PI = 2.f * Dali::Math::PI; ///< 360 degrees in radians

/**
 * @brief Run of glyphs that have the same style.
 */
struct GlyphRun
{
  GlyphRun()
  : fontFace{ nullptr },
    fontSize{ 0.0 },
    glyphIndex{ 0u },
    numberOfGlyphs{ 0u },
    fontId{ 0u },
    colorIndex{ 0u },
    isItalicRequired{ false },
    isBoldRequired{ false }
  {}

  FT_Face      fontFace;           ///< The font face used by the glyphs in the run.
  double       fontSize;           ///< The font size used by the glyphs in the run. According the Cairo's documentation this is in user space units. It works if I set the size in pixels.
  unsigned int glyphIndex;         ///< Index to the first glyph of the run.
  unsigned int numberOfGlyphs;     ///< Number of glyphs in the run.
  unsigned int fontId;             ///< The id of the font.
  unsigned int colorIndex;         ///< The index to the color of the glyphs.
  bool         isItalicRequired:1; ///< Whether the italic style is required.
  bool         isBoldRequired:1;   ///< Whether the bold style is required.
};

/**
 * @brief Helper struct used to destroy a bitmap buffer.
 *
 * The font client allocates a bitmap's buffer with the new operator.
 * However, the PixelBuffer class allocates the buffer with the
 * malloc() function and the Rotate() function which is intended
 * for the PixelBuffer as well allocates memory with malloc().
 *
 * This struct keeps the type of allocation and uses the delete[]
 * operator or the free() function to deallocate resources.
 */
struct GlyphBuffer
{
  enum DestructorType
  {
    FREE,
    DELETE
  };

  GlyphBuffer( Dali::TextAbstraction::FontClient::GlyphBufferData& data, DestructorType type )
  : data( data ),
    type( type )
  {
  }

  ~GlyphBuffer()
  {
    switch( type )
    {
      case FREE:
      {
        free( data.buffer );
        break;
      }
      case DELETE:
      {
        delete[] data.buffer;
      }
    }
  }

  Dali::TextAbstraction::FontClient::GlyphBufferData& data;
  DestructorType type;
};

/**
 * @brief Creates a pixel buffer with all pixels set to transparent.
 *
 * @param[in] parameters Contains the width and height of the pixel buffer.
 *
 * @return The pixel buffer.
 */
Dali::Devel::PixelBuffer CreateVoidPixelBuffer( const Dali::TextAbstraction::TextRenderer::Parameters& parameters )
{
  Dali::Pixel::Format pixelFormat = parameters.pixelFormat == Dali::TextAbstraction::TextRenderer::Parameters::A8 ? Dali::Pixel::A8 : Dali::Pixel::RGBA8888;
  Dali::Devel::PixelBuffer pixelBuffer = Dali::Devel::PixelBuffer::New( parameters.width,
                                                                        parameters.height,
                                                                        pixelFormat );

  const unsigned int bufferSize = parameters.width * parameters.height * Dali::Pixel::GetBytesPerPixel( pixelFormat );
  unsigned char* buffer = pixelBuffer.GetBuffer();
  memset( buffer, 0, bufferSize );

  return pixelBuffer;
}

/**
 * @brief Wraps the vertices of glyphs laid out on a horizontal strainght line on a circular path.
 *
 * It copies the vertices from the extra cairo context created to lay out the text
 * on a horizontal straight line to the cairo context used to render it.
 *
 * @param[in,out] cr The cairo context used to render the text.
 * @param[in] circularCr The extra cairo context created to layout horizontal text.
 * @param[in] parameters The parameters of the circular path.
 */
void WrapToCircularPath( cairo_t* cr, cairo_t* circularCr, const Dali::TextAbstraction::CircularTextParameters& parameters )
{
  bool first = true;

  // Copy the path to get a cairo_path_t pointer used to iterate through all its items.
  std::unique_ptr<cairo_path_t, void(*)(cairo_path_t*)> path( cairo_copy_path( circularCr ), cairo_path_destroy );

  // Iterates through all the path items and transform each vertex to follow the circle.
  // Transformed vertices are added to a new path in the 'cr' context (the one used to render the circular text)
  for( int i = 0; i < path->num_data; i += path->data[i].header.length )
  {
    cairo_path_data_t* data = &path->data[i];

    switch( data->header.type )
    {
      case CAIRO_PATH_MOVE_TO:
      {
        if( first )
        {
          cairo_new_path( cr );
        }

        first = false;
        double x = data[1].point.x;
        double y = data[1].point.y;
        Dali::TextAbstraction::TransformToArc( parameters, x, y );
        cairo_move_to( cr, x, y );
        break;
      }
      case CAIRO_PATH_LINE_TO:
      {
        double x = data[1].point.x;
        double y = data[1].point.y;
        Dali::TextAbstraction::TransformToArc( parameters, x, y );
        cairo_line_to( cr, x, y );
        break;
      }
      case CAIRO_PATH_CURVE_TO:
      {
        double x1 = data[1].point.x;
        double y1 = data[1].point.y;
        double x2 = data[2].point.x;
        double y2 = data[2].point.y;
        double x3 = data[3].point.x;
        double y3 = data[3].point.y;
        Dali::TextAbstraction::TransformToArc( parameters, x1, y1 );
        Dali::TextAbstraction::TransformToArc( parameters, x2, y2 );
        Dali::TextAbstraction::TransformToArc( parameters, x3, y3 );
        cairo_curve_to( cr, x1, y1, x2, y2, x3, y3 );
        break;
      }
      case CAIRO_PATH_CLOSE_PATH:
      {
        cairo_close_path( cr );
        break;
      }
      default:
      {
        DALI_LOG_WARNING( "Type of path not handled.\n" );
        // Nothing else to do.
        break;
      }
    }
  }
}

} // namespace

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

Devel::PixelBuffer RenderTextCairo( const TextAbstraction::TextRenderer::Parameters& parameters )
{
  const unsigned int numberOfGlyphs = parameters.glyphs.Count();

  if( 0u == numberOfGlyphs )
  {
    // return a pixel buffer with all pixels set to transparent.
    return CreateVoidPixelBuffer( parameters );
  }

  // Choose the pixel format to be used.
  //
  // @note Behdad wrote "Upper 8 bits maps to the fourth byte in a little-endian machine like the intels."
  //       https://lists.cairographics.org/archives/cairo/2006-March/006563.html
  //
  //       Here in practice Cairo's ARGB32 is like DALi's RGBA8888.
  //
  const bool isDstRgba = TextAbstraction::TextRenderer::Parameters::RGBA8888 == parameters.pixelFormat;
  const Pixel::Format pixelFormat = isDstRgba ? Pixel::Format::RGBA8888 : Pixel::Format::A8;
  const cairo_format_t cairoFormat = isDstRgba ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_A8;

  const int bpp = Pixel::GetBytesPerPixel( pixelFormat );
  if( 0u == bpp )
  {
    // return a pixel buffer with all pixels set to transparent.
    return CreateVoidPixelBuffer( parameters );
  }

  // This function provides a stride value that will respect all alignment requirements of the
  // accelerated image-rendering code within cairo.
  const int stride = cairo_format_stride_for_width( cairoFormat,
                                                    static_cast<int>( parameters.width ) );
  const int strideWidth = stride / bpp;

  // Convert from DALi glyphs to Cairo glyphs.
  std::vector<cairo_glyph_t> cairoGlyphs;
  cairoGlyphs.resize( numberOfGlyphs );
  cairo_glyph_t* cairoGlyphsBuffer = &cairoGlyphs[0u];

  const GlyphInfo* const daliGlyphsBuffer = parameters.glyphs.Begin();
  const Vector2* const positionsBuffer = parameters.positions.Begin();
  const ColorIndex* const colorIndicesBuffer = ( 0u == parameters.colorIndices.Count() ) ? nullptr : parameters.colorIndices.Begin();

  for( unsigned index = 0u; index < numberOfGlyphs; ++index )
  {
    const GlyphInfo& daliGlyph = *( daliGlyphsBuffer + index );
    const Vector2& position = *( positionsBuffer + index );
    cairo_glyph_t& cairoGlyph = *( cairoGlyphsBuffer + index );

    cairoGlyph.index = daliGlyph.index;
    cairoGlyph.x = std::round( position.x );
    cairoGlyph.y = std::round( position.y );
  }

  // Retrieve the FreeType fonts needed by Cairo from the font-client.
  Dali::TextAbstraction::FontClient fontClient = Dali::TextAbstraction::FontClient::Get();

  FT_Library ftLibrary;
  auto error = FT_Init_FreeType( &ftLibrary );
  if( error )
  {
    DALI_LOG_ERROR( "Error initializing FT library\n" );

    // return a pixel buffer with all pixels set to transparent.
    return CreateVoidPixelBuffer( parameters );
  }

  // Vector used to store the FreeType font faces, its size and the run of glyphs that use the font.
  std::vector<GlyphRun> glyphRuns;
  glyphRuns.reserve( 8u );

  // The size set in Cairo and FreeType has different units.
  // Before the size is set in Cairo it needs to be converted according the formula
  // 'pixel_size = point_size * resolution / 72' got from the FreeType doc.
  // https://www.freetype.org/freetype2/docs/glyphs/glyphs-2.html

  unsigned int horizontalDpi = 0u;
  unsigned int verticalDpi = 0u;
  fontClient.GetDpi( horizontalDpi, verticalDpi );
  const double dVerticalDpi = static_cast<double>( verticalDpi );

  const double FROM_26_DOT_6_TO_PIXELS = dVerticalDpi / ( 64.0 * 72.0 );

  GlyphRun currentGlyphRun;
  currentGlyphRun.fontId = 0u;
  currentGlyphRun.colorIndex = 0u;
  currentGlyphRun.isItalicRequired = false;
  currentGlyphRun.isBoldRequired = false;
  for( unsigned index = 0u; index < numberOfGlyphs; ++index )
  {
    const GlyphInfo& daliGlyph = *( daliGlyphsBuffer + index );
    const FontId fontId = daliGlyph.fontId;
    const ColorIndex colorIndex = ( nullptr == colorIndicesBuffer ) ? 0u : *( colorIndicesBuffer + index );
    const bool isItalicRequired = daliGlyph.isItalicRequired;
    const bool isBoldRequired = daliGlyph.isBoldRequired;

    if( ( fontId != currentGlyphRun.fontId ) ||
        ( ( 0u == fontId ) && ( 0u != daliGlyph.index ) ) ||
        ( colorIndex != currentGlyphRun.colorIndex ) ||
        ( isItalicRequired != currentGlyphRun.isItalicRequired ) ||
        ( isBoldRequired != currentGlyphRun.isBoldRequired ) )
    {
      // There is a new run. First set the number of glyphs of the previous run and store it.
      currentGlyphRun.numberOfGlyphs = index - currentGlyphRun.glyphIndex;
      if( 0u != currentGlyphRun.numberOfGlyphs )
      {
        glyphRuns.push_back( currentGlyphRun );
      }

      currentGlyphRun.fontFace = nullptr;
      currentGlyphRun.fontSize = 0.0;
      currentGlyphRun.glyphIndex = index;
      currentGlyphRun.numberOfGlyphs = 0u;
      currentGlyphRun.fontId = 0u;
      currentGlyphRun.colorIndex = 0u;
      currentGlyphRun.isItalicRequired = false;
      currentGlyphRun.isBoldRequired = false;

      if( 0u != fontId )
      {
        // Get the font's path file name from the font Id.
        FontDescription fontDescription;
        fontClient.GetDescription( fontId, fontDescription );

        switch( fontDescription.type )
        {
          case FontDescription::FACE_FONT:
          {
            // Create a FreeType font's face.
            auto error = FT_New_Face( ftLibrary, fontDescription.path.c_str(), 0u, &currentGlyphRun.fontFace );
            if( error )
            {
              DALI_LOG_ERROR( "Error in FT while creating a new face\n" );

              // return a pixel buffer with all pixels set to transparent.
              return CreateVoidPixelBuffer( parameters );
            }

            // Set the font's size. It needs to be set in the Freetype font and in the Cairo's context.
            unsigned int fontSize = fontClient.GetPointSize( fontId );

            // Font's size to be set in the Cairo's context.
            currentGlyphRun.fontSize = FROM_26_DOT_6_TO_PIXELS * static_cast<double>( fontSize );
            break;
          }
          case FontDescription::BITMAP_FONT:
          {
            //Nothing to do.
            break;
          }
          default:
          {
            //Nothing to do.
            break;
          }
        }
      }
      currentGlyphRun.fontId = fontId;
      currentGlyphRun.colorIndex = colorIndex;
      currentGlyphRun.isItalicRequired = isItalicRequired;
      currentGlyphRun.isBoldRequired = isBoldRequired;
    }
  }

  // Calculate the number of glyphs of the last run and store it.
  currentGlyphRun.numberOfGlyphs = numberOfGlyphs - currentGlyphRun.glyphIndex;
  if( 0u != currentGlyphRun.numberOfGlyphs )
  {
    glyphRuns.push_back( currentGlyphRun );
  }

  // Creates the pixel buffer and retrieves the buffer pointer used to create the Cairo's surface.
  Devel::PixelBuffer pixelBuffer = Devel::PixelBuffer::New( strideWidth, parameters.height, pixelFormat );

  unsigned char* buffer = pixelBuffer.GetBuffer();
  const unsigned int bufferSize = stride * parameters.height;
  memset( buffer, 0, bufferSize );

  std::unique_ptr<cairo_surface_t, void(*)(cairo_surface_t*)> surfacePtr( cairo_image_surface_create_for_data( buffer,
                                                                                                               cairoFormat,
                                                                                                               parameters.width,
                                                                                                               parameters.height,
                                                                                                               stride ),
                                                                          cairo_surface_destroy );
  cairo_surface_t* surface = surfacePtr.get();

  if( ( nullptr == surface ) || ( CAIRO_STATUS_SUCCESS != cairo_surface_status( surface ) ) )
  {
    DALI_LOG_ERROR( "Failed to create a cairo's surface\n" );

    return CreateVoidPixelBuffer( parameters );
  }

  // Whether the text is circular.
  const bool isCircularText = 0u != parameters.radius;

  // Creates a surface for circular text.
  //
  // The reason to create a surface for circular text is that the strategy
  // followed is to layout the text in a straight horizontal line and apply a
  // transform to each vertex that forms the geometry of the glyphs to place
  // and bend the glyphs accordingly to the circular path.
  //
  // As the glyphs are laid out first in a straight line they may exceed the
  // boundaries of the surface in that case cairo ignores them.
  std::unique_ptr<cairo_surface_t, void(*)(cairo_surface_t*)> circularSurfacePtr( nullptr, cairo_surface_destroy );
  cairo_surface_t* circularSurface = nullptr;
  if( isCircularText )
  {
    circularSurfacePtr.reset( cairo_surface_create_similar( surface,
                                                            CAIRO_CONTENT_ALPHA,
                                                            parameters.circularWidth,
                                                            parameters.circularHeight ) );
    circularSurface = circularSurfacePtr.get();

    if( ( nullptr == circularSurface ) || ( CAIRO_STATUS_SUCCESS != cairo_surface_status( circularSurface ) ) )
    {
      DALI_LOG_ERROR( "Failed to create a cairo's circular surface\n" );

      return CreateVoidPixelBuffer( parameters );
    }
  }

  std::unique_ptr<cairo_t, void(*)(cairo_t*)> crPtr( cairo_create( surface ), cairo_destroy );
  cairo_t* cr = crPtr.get();

  if( CAIRO_STATUS_SUCCESS != cairo_status( cr ) )
  {
    DALI_LOG_ERROR( "Failed to create a cairo context\n" );

    return CreateVoidPixelBuffer( parameters );
  }

  std::unique_ptr<cairo_t, void(*)(cairo_t*)> circularCrPtr( nullptr, cairo_destroy );
  cairo_t* circularCr = nullptr;

  if( isCircularText )
  {
    circularCrPtr.reset( cairo_create( circularSurface ) );
    circularCr = circularCrPtr.get();

    if( CAIRO_STATUS_SUCCESS != cairo_status( circularCr ) )
    {
      DALI_LOG_ERROR( "Failed to create a cairo context\n" );

      return CreateVoidPixelBuffer( parameters );
    }
  }

  CircularTextParameters circularTextParameters;

  // Render the glyphs.
  if( isCircularText )
  {
    // Set the parameters.
    circularTextParameters.isClockwise = ( TextAbstraction::TextRenderer::Parameters::CLOCKWISE == parameters.circularLayout );

    circularTextParameters.centerX = static_cast<double>( parameters.centerX );
    circularTextParameters.centerY = static_cast<double>( parameters.centerY );
    circularTextParameters.radius = static_cast<double>( parameters.radius );
    circularTextParameters.invRadius = 1.0 / circularTextParameters.radius;
    circularTextParameters.beginAngle = -parameters.beginAngle + Dali::Math::PI_2;
  }

  cairo_move_to( cr, 0.0, 0.0 );

  for( const auto& run: glyphRuns )
  {
    const bool isEmoji = parameters.isEmoji[run.glyphIndex];
    if( isEmoji || ( nullptr == run.fontFace ) )
    {
      // Retrieve the color for the glyph.
      const Vector4& color = parameters.colors[run.colorIndex];

      const unsigned int lastGlyphIndex = run.glyphIndex + run.numberOfGlyphs;
      for( unsigned int index = run.glyphIndex; index < lastGlyphIndex; ++index )
      {
        // Whether it's a bitmap font.
        const bool doBlendWithTextColor = !isEmoji && ( ColorBlendingMode::MULTIPLY == parameters.blendingMode[index] );

        // Check if there is an embedded image or a bitmap font image.
        const GlyphIndex glyphFontIndex = daliGlyphsBuffer[index].index;
        if( 0u != glyphFontIndex )
        {
          // The embedded image could be A8, RGBA8888 or BGRA8888.
          //
          // If the embedded image is RGBA8888 or BGRA8888 then the cairo's buffer is ARGB32. It's needed to convert from RGBA or BGRA to ARGB.
          // If the embedded image is A8 it's needed to check if the cairo's buffer is A8 or ARGB32 and do the conversion if needed.

          const cairo_glyph_t& glyph = *( cairoGlyphsBuffer + index );

          // Retrieve the image
          TextAbstraction::FontClient::GlyphBufferData data;
          std::unique_ptr<GlyphBuffer> glyphBufferPtr( new GlyphBuffer( data, GlyphBuffer::DELETE ) );
          if( isEmoji )
          {
            data.width = parameters.glyphs[run.glyphIndex].width;
            data.height = parameters.glyphs[run.glyphIndex].height;
          }

          fontClient.CreateBitmap( run.fontId, glyphFontIndex, false, false, data, 0u );

          if( nullptr == data.buffer )
          {
            // nothing else to do if there is no image.
            continue;
          }

          // Calculate the position for the circular text.
          double glyphX = glyph.x;
          double glyphY = glyph.y;

          if( isCircularText )
          {
            // Center of the bitmap.
            const double halfWidth = 0.5 * static_cast<double>( data.width );
            const double halfHeight = 0.5 * static_cast<double>( data.height );

            double centerX = glyph.x + halfWidth;
            double centerY = glyph.y - halfHeight;

            float radians = circularTextParameters.beginAngle + ( circularTextParameters.isClockwise ? -1.f : 1.f ) * ( Dali::Math::PI_2 + circularTextParameters.invRadius * centerX );
            radians = fmod( radians, TWO_PI );
            radians += ( radians < 0.f ) ? TWO_PI : 0.f;

            TransformToArc( circularTextParameters, centerX, centerY );

            uint8_t* pixelsOut = nullptr;
            unsigned int widthOut = data.width;
            unsigned int heightOut = data.height;
            const unsigned int pixelSize = Pixel::GetBytesPerPixel( data.format );

            Dali::Internal::Platform::RotateByShear( data.buffer,
                                                     data.width,
                                                     data.height,
                                                     pixelSize,
                                                     radians,
                                                     pixelsOut,
                                                     widthOut,
                                                     heightOut );
            if( nullptr != pixelsOut )
            {
              delete[] data.buffer;
              data.buffer = pixelsOut;
              glyphBufferPtr.get()->type = GlyphBuffer::FREE;
              data.width = widthOut;
              data.height = heightOut;
            }

            glyphX = centerX - 0.5 * static_cast<double>( data.width );
            glyphY = centerY + 0.5 * static_cast<double>( data.height );
          }


          if( ( Pixel::A8 != data.format ) &&
              ( Pixel::L8 != data.format ) &&
              ( Pixel::RGBA8888 != data.format ) &&
              ( Pixel::BGRA8888 != data.format ) )
          {
            DALI_LOG_ERROR( " Cairo Renderer: The valid pixel format for embedded items are A8 or RGBA8888\n" );
            continue;
          }

          // Check if the item is out of the buffer.
          if( ( glyphX + static_cast<float>( data.width ) < 0.f ) ||
              ( glyphX > static_cast<float>( strideWidth ) ) ||
              ( glyphY < 0.f ) ||
              ( glyphY - static_cast<float>( data.height ) > static_cast<float>( parameters.height ) ) )
          {
            // The embedded item is completely out of the buffer.
            continue;
          }

          const bool isSrcA = ( Pixel::A8 == data.format ) || ( Pixel::L8 == data.format );
          const bool isSrcRgba = Pixel::RGBA8888 == data.format;
          const bool isSrcBgra = Pixel::BGRA8888 == data.format;

          // 0 -> image and cairo buffer are A8
          // 1 -> image is A8, cairo buffer is ARGB
          // 2 -> image is RGBA and cairo buffer is ARGB
          // 3 -> image is BGRA and cairo buffer is ARGB
          int rgbaCase = 0;
          if( isSrcA && isDstRgba )
          {
            rgbaCase = 1;
          }
          else if( isSrcRgba && isDstRgba )
          {
            rgbaCase = 2;
          }
          else if( isSrcBgra && isDstRgba )
          {
            rgbaCase = 3;
          }
          else if( ( isSrcRgba || isSrcBgra ) && !isDstRgba )
          {
            DALI_LOG_ERROR( "Cairo Renderer: The embedded image is RGBA or BGRA and the Cairo's buffer has been creates with A8 format!\n" );
            continue;
          }

          // Select the cropped source image area to copy into the surface buffer
          unsigned int glyphUintX = 0u;
          unsigned int glyphUintY = 0u;
          unsigned int srcWidth = data.width;
          unsigned int srcHeight = data.height;
          unsigned int xSrcIndex = 0u;
          unsigned int ySrcIndex = 0u;
          if( glyphX < 0.f )
          {
            xSrcIndex = static_cast<unsigned int>( std::abs( glyphX ) );
            srcWidth -= xSrcIndex;
          }
          else
          {
            glyphUintX = static_cast<unsigned int>( glyphX );
          }

          if( glyphUintX + srcWidth > static_cast<unsigned int>( strideWidth ) )
          {
            srcWidth -= ( ( glyphUintX + srcWidth ) - strideWidth );
          }

          if( glyphY - static_cast<float>( srcHeight ) < 0.f )
          {
            ySrcIndex = static_cast<unsigned int>( std::abs( glyphY - static_cast<float>( srcHeight ) ) );
            srcHeight -= ySrcIndex;
          }
          else
          {
            glyphUintY = static_cast<unsigned int>( glyphY - static_cast<float>( srcHeight ) );
          }

          if( glyphUintY + srcHeight > parameters.height )
          {
            srcHeight -= ( ( glyphUintY + srcHeight ) - parameters.height );
          }

          // Calculate the source and destination indices.
          const unsigned int srcPixelSize = Pixel::GetBytesPerPixel( data.format );
          const unsigned int dstPixelSize = Pixel::GetBytesPerPixel( pixelFormat );

          unsigned int srcIndex = srcPixelSize * ( ySrcIndex * srcWidth + xSrcIndex );
          unsigned int dstIndex = dstPixelSize * ( glyphUintY * strideWidth + glyphUintX );

          const unsigned int srcWidthOffset = srcPixelSize * ( data.width - srcWidth );
          const unsigned int dstWidthOffset = dstPixelSize * ( strideWidth - srcWidth );

          // Copy the image to the surface
          for( unsigned int j = 0; j < srcHeight; ++j )
          {
            for( unsigned int i = 0; i < srcWidth; ++i )
            {
              switch( rgbaCase )
              {
                case 0: // Both the image's buffer and cairo's buffer are A8
                {
                  const unsigned char alpha = *( data.buffer + srcIndex );
                  if( alpha != 0u )
                  {
                    // @todo needs a proper blending!
                    *( buffer + dstIndex ) = alpha;
                  }
                  break;
                }
                case 1: // The image's buffer is A8 and the cairo's buffer is ARGB
                {
                  const unsigned char alpha = *( data.buffer + srcIndex );
                  if( alpha != 0u )
                  {
                    // @todo needs a proper blending!
                    const float srcAlpha = TO_FLOAT * static_cast<float>( alpha );

                    // Write the RGBA modulated with the given default color.
                    const float* const colorPtr = color.AsFloat();
                    *( buffer + dstIndex + 0u ) = static_cast<unsigned char>( TO_UCHAR * colorPtr[0u] * srcAlpha );
                    *( buffer + dstIndex + 1u ) = static_cast<unsigned char>( TO_UCHAR * colorPtr[1u] * srcAlpha );
                    *( buffer + dstIndex + 2u ) = static_cast<unsigned char>( TO_UCHAR * colorPtr[2u] * srcAlpha );
                    *( buffer + dstIndex + 3u ) = static_cast<unsigned char>( TO_UCHAR * colorPtr[3u] * srcAlpha );
                  }
                  break;
                }
                case 2: // The image's buffer is RGBA and the cairo's buffer is ARGB
                {
                  const unsigned char alpha = *(data.buffer + srcIndex + 3u);
                  if( alpha != 0u )
                  {
                    if( doBlendWithTextColor )
                    {
                      const float* const colorPtr = color.AsFloat();

                      const float srcAlpha = TO_FLOAT * static_cast<float>(alpha) * colorPtr[3u];

                      *(buffer + dstIndex + 0u) = static_cast<unsigned char>( static_cast<float>( *(data.buffer + srcIndex + 0u) ) * colorPtr[0u] );
                      *(buffer + dstIndex + 1u) = static_cast<unsigned char>( static_cast<float>( *(data.buffer + srcIndex + 1u) ) * colorPtr[1u] );
                      *(buffer + dstIndex + 2u) = static_cast<unsigned char>( static_cast<float>( *(data.buffer + srcIndex + 2u) ) * colorPtr[2u] );

                      // Write the alpha.
                      *(buffer + dstIndex + 3u) = static_cast<unsigned char>( TO_UCHAR * srcAlpha );
                    }
                    else
                    {
                      // @todo needs a proper blending!
                      // Write the RGB
                      *(buffer + dstIndex + 0u) = *(data.buffer + srcIndex + 0u);
                      *(buffer + dstIndex + 1u) = *(data.buffer + srcIndex + 1u);
                      *(buffer + dstIndex + 2u) = *(data.buffer + srcIndex + 2u);

                      // Write the alpha.
                      *(buffer + dstIndex + 3u) = *(data.buffer + srcIndex + 3u);
                    }
                  }
                  break;
                }
                case 3: // The image's buffer is BGRA and the cairo's buffer is ARGB
                {
                  const unsigned char alpha = *(data.buffer + srcIndex + 3u);
                  if( alpha != 0u )
                  {
                    if( doBlendWithTextColor )
                    {
                      const float* const colorPtr = color.AsFloat();

                      const float srcAlpha = TO_FLOAT * static_cast<float>(alpha) * colorPtr[3u];

                      *(buffer + dstIndex + 0u) = static_cast<unsigned char>( static_cast<float>( *(data.buffer + srcIndex + 2u) ) * colorPtr[0u] );
                      *(buffer + dstIndex + 1u) = static_cast<unsigned char>( static_cast<float>( *(data.buffer + srcIndex + 1u) ) * colorPtr[1u] );
                      *(buffer + dstIndex + 2u) = static_cast<unsigned char>( static_cast<float>( *(data.buffer + srcIndex + 0u) ) * colorPtr[2u] );

                      // Write the alpha.
                      *(buffer + dstIndex + 3u) = static_cast<unsigned char>( TO_UCHAR * srcAlpha );
                    }
                    else
                    {
                      // @todo needs a proper blending!
                      // Write the RGBA
                      *(buffer + dstIndex + 0u) = *(data.buffer + srcIndex + 2u);
                      *(buffer + dstIndex + 1u) = *(data.buffer + srcIndex + 1u);
                      *(buffer + dstIndex + 2u) = *(data.buffer + srcIndex + 0u);
                      *(buffer + dstIndex + 3u) = *(data.buffer + srcIndex + 3u);
                    }
                  }
                  break;
                }
                default:
                {
                  DALI_ASSERT_ALWAYS( !"Cairo Renderer: The accepted values for this switch case are: 0, 1, 2!" );
                }
              } // switch
              srcIndex += srcPixelSize;
              dstIndex += dstPixelSize;
            } // for width
            srcIndex += srcWidthOffset;
            dstIndex += dstWidthOffset;
          } // for height
        }
      }
    }
    else
    {
      // Sets the color. The color is actually BGRA
      const Vector4& color = parameters.colors[run.colorIndex];

      cairo_set_source_rgba( cr,
                             static_cast<double>( color.b ),
                             static_cast<double>( color.g ),
                             static_cast<double>( color.r ),
                             static_cast<double>( color.a ) );

      // Create the Cairo's font from the FreeType font.
      int options = 0;
      options = CAIRO_HINT_STYLE_SLIGHT;
      std::unique_ptr<cairo_font_face_t, void(*)(cairo_font_face_t*)> fontFacePtr( cairo_ft_font_face_create_for_ft_face( run.fontFace, options ), cairo_font_face_destroy );
      cairo_font_face_t* fontFace = fontFacePtr.get();

      static const cairo_user_data_key_t key = { 0 };
      cairo_status_t status = cairo_font_face_set_user_data( fontFace, &key, run.fontFace, reinterpret_cast<cairo_destroy_func_t>( FT_Done_Face ) );
      if( status )
      {
        cairo_font_face_destroy( fontFace );
      }

      unsigned int ftSynthesizeFlag = 0u;
      if( run.isBoldRequired && !( run.fontFace->style_flags & FT_STYLE_FLAG_BOLD ) )
      {
        ftSynthesizeFlag |= CAIRO_FT_SYNTHESIZE_BOLD;
      }

      cairo_ft_font_face_set_synthesize( fontFace, ftSynthesizeFlag );

      cairo_font_face_reference( fontFace );

      const bool synthesizeItalic = ( run.isItalicRequired && !( run.fontFace->style_flags & FT_STYLE_FLAG_ITALIC ) );

      if( CAIRO_STATUS_SUCCESS != cairo_font_face_status( fontFace ) )
      {
        DALI_LOG_ERROR( "Failed to load the Freetype Font\n" );
      }

      // Sets the font.
      cairo_set_font_face( isCircularText ? circularCr : cr, fontFace );

      // Sets the size
      cairo_set_font_size( isCircularText ? circularCr : cr, run.fontSize );

      // Render the glyphs.
      if( isCircularText )
      {
        circularTextParameters.synthesizeItalic = synthesizeItalic;

        const unsigned int glyphJump = circularTextParameters.synthesizeItalic ? 1u : run.numberOfGlyphs;

        for( unsigned int index = 0u; index < run.numberOfGlyphs; index += glyphJump )
        {
          // Clears the current path where the text is laid out on a horizontal straight line.
          cairo_new_path( circularCr );
          cairo_move_to( circularCr, 0.0, 0.0 );

          cairo_glyph_path( circularCr, ( cairoGlyphsBuffer + run.glyphIndex + index ), glyphJump );

          WrapToCircularPath( cr, circularCr, circularTextParameters );
          cairo_fill( cr );
        }
      }
      else
      {
        if( synthesizeItalic )
        {
          // Apply a shear transform to synthesize the italics.
          // For a reason Cairo may trim some glyphs if the CAIRO_FT_SYNTHESIZE_OBLIQUE flag is used.

          // This is to calculate an offset used to compensate the 'translation' done by the shear transform
          // as it's done for the whole render buffer.
          double maxY = 0.0;
          for( unsigned int index = run.glyphIndex, endIndex = run.glyphIndex + run.numberOfGlyphs; index < endIndex; ++index )
          {
            maxY = std::max( maxY, (*( cairoGlyphsBuffer + index )).y );
          }

          cairo_matrix_t matrix;
          cairo_matrix_init( &matrix,
                                                                                  1.0, 0.0,
                                   -TextAbstraction::FontClient::DEFAULT_ITALIC_ANGLE, 1.0,
                             maxY * TextAbstraction::FontClient::DEFAULT_ITALIC_ANGLE, 0.0 );

          cairo_transform( cr, &matrix );
        }

        cairo_show_glyphs( cr, ( cairoGlyphsBuffer + run.glyphIndex ), run.numberOfGlyphs );

        if( synthesizeItalic )
        {
          // Restore the transform matrix to the identity.
          cairo_matrix_t matrix;
          cairo_matrix_init_identity( &matrix );
          cairo_set_matrix( cr, &matrix );
        }

        cairo_fill( cr );
      }
    }
  }

  return pixelBuffer;
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
