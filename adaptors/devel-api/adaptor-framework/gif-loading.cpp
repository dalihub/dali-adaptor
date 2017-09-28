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
 *
 * When the available GIFLIB is with version up to 4.1.6, for using the New functions DGifSavedExtensionToGCB()
 * which makes it easy to read GIF89 graphics control blocks in saved images,
 * we copied the DGifExtensionToGCB and DGifSavedExtensionToGCB functions
 * along with the GraphicsControlBlock structure from the GIFLIB 5.1.4 to this source file.
 *
 *     The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond
 *     Permission is hereby granted, free of charge, to any person obtaining a copy
 *     of this software and associated documentation files (the "Software"), to deal
 *     in the Software without restriction, including without limitation the rights
 *     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *     copies of the Software, and to permit persons to whom the Software is
 *     furnished to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included in
 *     all copies or substantial portions of the Software.

 *     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *     THE SOFTWARE.
 */

// CLASS HEADER
#include "gif-loading.h"

// EXTERNAL INCLUDES
#include <gif_lib.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/images/pixel-data.h>

namespace
{
// forward declaration of function
void GifCopyLine(unsigned char* destination, unsigned char* source, const ColorMapObject* colorMap, int transparent, int copyWidth, bool replace );

#ifdef GIF_LIB_VERSION

/*********************************************
 *  GIFLIB version up to 4.1.6
 ******************************************/
/**
 * With GIFLIB version up to 4.1.6, for using the New functions DGifSavedExtensionToGCB()
 * which makes it easy to read GIF89 graphics control blocks in saved images,
 * we copied the DGifExtensionToGCB and DGifSavedExtensionToGCB functions
 * along with the GraphicsControlBlock structure from the GIFLIB 5.1.4 to this source file.
 */

/* compose unsigned little endian value */
#define UNSIGNED_LITTLE_ENDIAN(lo, hi) ((lo) | ((hi) << 8))

typedef struct GraphicsControlBlock {
    int DisposalMode;
#define DISPOSAL_UNSPECIFIED      0       /* No disposal specified. */
#define DISPOSE_DO_NOT            1       /* Leave image in place */
#define DISPOSE_BACKGROUND        2       /* Set area too background color */
#define DISPOSE_PREVIOUS          3       /* Restore to previous content */
    bool UserInputFlag;      /* User confirmation required before disposal */
    int DelayTime;           /* pre-display delay in 0.01sec units */
    int TransparentColor;    /* Palette index for transparency, -1 if none */
#define NO_TRANSPARENT_COLOR    -1
} GraphicsControlBlock;

/******************************************************************************
 Extract a Graphics Control Block from raw extension data
******************************************************************************/
int DGifExtensionToGCB(const size_t GifExtensionLength,
                       char *GifExtension,
                       GraphicsControlBlock *GCB)
{
  if (GifExtensionLength != 4)
  {
    return GIF_ERROR;
  }

  GCB->DisposalMode = (GifExtension[0] >> 2) & 0x07;
  GCB->UserInputFlag = (GifExtension[0] & 0x02) != 0;
  GCB->DelayTime = UNSIGNED_LITTLE_ENDIAN(GifExtension[1], GifExtension[2]);
  if (GifExtension[0] & 0x01)
  {
    GCB->TransparentColor = reinterpret_cast< int >( GifExtension[3]+256 ) % 256;
  }
  else
  {
    GCB->TransparentColor = NO_TRANSPARENT_COLOR;
  }

  return GIF_OK;
}

/******************************************************************************
 Extract the Graphics Control Block for a saved image, if it exists.
******************************************************************************/
int DGifSavedExtensionToGCB(GifFileType *GifFile, int ImageIndex, GraphicsControlBlock *GCB)
{
  int i;

  if (ImageIndex < 0 || ImageIndex > GifFile->ImageCount - 1)
    return GIF_ERROR;

  GCB->DisposalMode = DISPOSAL_UNSPECIFIED;
  GCB->UserInputFlag = false;
  GCB->DelayTime = 0;
  GCB->TransparentColor = NO_TRANSPARENT_COLOR;

  for (i = 0; i < GifFile->SavedImages[ImageIndex].ExtensionBlockCount; i++) {
    ExtensionBlock *ep = &GifFile->SavedImages[ImageIndex].ExtensionBlocks[i];
    if (ep->Function == GRAPHICS_EXT_FUNC_CODE)
      return DGifExtensionToGCB(ep->ByteCount, ep->Bytes, GCB);
  }

  return GIF_ERROR;
}

/******************************************************************************
 End of code copy from GIFLIB version 5.
******************************************************************************/


// simple class to enforce clean-up of GIF structures
struct GifAutoCleanup
{
  GifAutoCleanup(GifFileType*& _gifInfo)
  : gifInfo(_gifInfo)
  {
  }

  ~GifAutoCleanup()
  {
    if(NULL != gifInfo)
    {
      // clean up GIF resources
      DGifCloseFile( gifInfo );
    }
  }

  GifFileType*& gifInfo;
};

/**
 * Open a new gif file for read.
 * @param[in] url The url of the gif to load.
 * @param[out] gifInfo The dynamically allocated GifFileType pointer which serves as the GIF info record.
 * @return True if the file is opened successfully, false otherwise.
 */
bool GifOpen( const char* url, GifFileType*& gifInfo )
{
  gifInfo = DGifOpenFileName( url );
  if( gifInfo == NULL )
  {
    DALI_LOG_ERROR( "GIF Loader: DGifOpen failed. \n" );
    return false;
  }
  return true;
}

/**
 * With GIFLIB version 4.1.6, the interlacing needs to be handled manually
 */
// Used in the GIF interlace algorithm to determine the starting byte and the increment required
// for each pass.
struct InterlacePair
{
  unsigned int startingByte;
  unsigned int incrementalByte;
};

// Used in the GIF interlace algorithm to determine the order and which location to read data from
// the file.
const InterlacePair INTERLACE_PAIR_TABLE [] = {
  { 0, 8 }, // Starting at 0, read every 8 bytes.
  { 4, 8 }, // Starting at 4, read every 8 bytes.
  { 2, 4 }, // Starting at 2, read every 4 bytes.
  { 1, 2 }, // Starting at 1, read every 2 bytes.
};
const int INTERLACE_PAIR_TABLE_SIZE( sizeof( INTERLACE_PAIR_TABLE ) / sizeof( InterlacePair ) );

/**
 * copy a image from color-index formated source to the the RGBA formated destination
 * @param[out] destination The RGBA formated destination.
 * @param[in] source The color-index formated source.
 * @param[in] width The width of the destination image.
 * @param[in] height The height of the destination image.
 * @param[in] imageDesc The description of the source image.
 * @param[in] colorMap The color map for mapping the color index to RGB values.
 * @param[in] transparent The color index which is interpreted as transparent.
 * @param[in] replace If true, the pixel with transparent color should be set as transparent.
 *                    If false, skip the pixel with transparent color, so that the previously initialized color is used.
 */
void GifCopyFrame( unsigned char* destination, unsigned char* source, int width, int height,
                   const GifImageDesc& imageDesc, const ColorMapObject* colorMap,
                   int transparent, bool replace )
{
  // Calculate the copy size as the image might only cover sub area of the frame
  int copyWidth = imageDesc.Width <= ( width-imageDesc.Left) ? imageDesc.Width : width - imageDesc.Left;
  int copyHeight = imageDesc.Height <= ( height-imageDesc.Top) ? imageDesc.Height : height - imageDesc.Top;

  unsigned char* row;
  // copy line by line from the color-index formated source to the RGBA formated destination.
  if( imageDesc.Interlace ) // With GIFLIB version 4.1, the interlacing needs to be handled manually
  {
    const InterlacePair* interlacePairPtr( INTERLACE_PAIR_TABLE );
    for ( int interlacePair = 0; interlacePair < INTERLACE_PAIR_TABLE_SIZE; ++interlacePair, ++interlacePairPtr )
    {
      for( int currentRow = interlacePairPtr->startingByte; currentRow < copyHeight; currentRow +=interlacePairPtr->incrementalByte )
      {
        row = destination + (imageDesc.Top + currentRow) * width * 4 + imageDesc.Left * 4 ;
        GifCopyLine( row, source, colorMap, transparent, copyWidth, replace);
        source += imageDesc.Width;
      }
    }
  }
  else
  {
    for( int currentRow = 0; currentRow < copyHeight; currentRow++ )
    {
      row = destination + (imageDesc.Top + currentRow) * width * 4 + imageDesc.Left * 4 ;
      GifCopyLine( row, source, colorMap, transparent, copyWidth, replace);
      source += imageDesc.Width;
    }
  }
}

#else

/*************************************************
 * GIFLIB major version 5
 *************************************************/

// simple class to enforce clean-up of GIF structures
struct GifAutoCleanup
{
  GifAutoCleanup(GifFileType*& _gifInfo)
  : gifInfo(_gifInfo)
  {
  }

  ~GifAutoCleanup()
  {
    if(NULL != gifInfo)
    {
      // clean up GIF resources
      int errorCode = 0; //D_GIF_SUCCEEDED is 0
      DGifCloseFile( gifInfo, &errorCode );

      if( errorCode )
      {
        DALI_LOG_ERROR( "GIF Loader: DGifCloseFile Error. Code: %d\n", errorCode );
      }
    }
  }

  GifFileType*& gifInfo;
};

/**
 * Open a new gif file for read.
 * @param[in] url The url of the gif to load.
 * @param[out] gifInfo The dynamically allocated GifFileType pointer which serves as the GIF info record.
 * @return True if the file is opened successfully, false otherwise.
 */
bool GifOpen( const char* url, GifFileType*& gifInfo )
{
  int errorCode;
  gifInfo = DGifOpenFileName(url, &errorCode);
  if (gifInfo == NULL)
  {
    DALI_LOG_ERROR( "GIF Loader: DGifOpenFileName Error. Code: %d\n", errorCode );
    return false;
  }
  return true;
}

/**
 * copy a image from color-index formated source to the the RGBA formated destination
 * @param[out] destination The RGBA formated destination.
 * @param[in] source The color-index formated source.
 * @param[in] width The width of the destination image.
 * @param[in] height The height of the destination image.
 * @param[in] imageDesc The description of the source image.
 * @param[in] colorMap The color map for mapping the color index to RGB values.
 * @param[in] transparent The color index which is interpreted as transparent.
 * @param[in] replace If true, the pixel with transparent color should be set as transparent.
 *                    If false, skip the pixel with transparent color, so that the previously initialized color is used.
 */
void GifCopyFrame( unsigned char* destination, unsigned char* source, int width, int height,
                   const GifImageDesc& imageDesc, const ColorMapObject* colorMap,
                    int transparent, bool replace )
{
  // Calculate the copy size as the image might only cover sub area of the frame
  int copyWidth = imageDesc.Width <= ( width-imageDesc.Left) ? imageDesc.Width : width - imageDesc.Left;
  int copyHeight = imageDesc.Height <= ( height-imageDesc.Top) ? imageDesc.Height : height - imageDesc.Top;

  unsigned char* row;
  // copy line by line from the color-index formated source to the RGBA formated destination.
  for( int currentRow = 0; currentRow < copyHeight; currentRow++ )
  {
    row = destination + (imageDesc.Top + currentRow) * width * 4 + imageDesc.Left * 4 ;
    GifCopyLine( row, source, colorMap, transparent, copyWidth, replace);
    source += imageDesc.Width;
  }
}

#endif // End of code for different GIF_LIB_VERSION

/**
 * copy one line from the color-index formated source to the RGBA formated destination.
 * @param[out] destination The RGBA formated destination.
 * @param[in] source The color-index formated source.
 * @param[in] transparent The color index which is interpreted as transparent.
 * @param[in] color The color map for mapping the color index to RGB values.
 * @param[in] copyWidth The copy width.
 * @param[in] replace If true, the pixel with transparent color should be set as transparent.
 *                    If false, skip the pixel with transparent color, so that the previously initialized color is used.
 */
void GifCopyLine(unsigned char* destination, unsigned char* source, const ColorMapObject* colorMap, int transparent, int copyWidth, bool replace )
{
  for ( ; copyWidth > 0; copyWidth--, source++ )
  {
    if( replace || *source != transparent )
    {
      *(destination++) = colorMap->Colors[*source].Red;
      *(destination++) = colorMap->Colors[*source].Green;
      *(destination++) = colorMap->Colors[*source].Blue;
      *(destination++) = *source == transparent ? 0x00 : 0xff;
    }
    else
    {
      destination += 4;
    }
  }
}


/**
 * Decode one frame of the animated gif.
 * @param[out] delay The delay time of this frame.
 * @param[in] gifInfo The GifFileType structure.
 * @param[in] backgroundcolor The global background color.
 * @param[in] frameIndex The index of this frame.
 * @param[in] lastPreservedFrame The pixel buffer of the last preserved frame.
 * @param[in] previousFrame The pixel buffer of the previous frame.
 * @param[in] clearFrameArea The area to be cleared if the disposal mode is DISPOSE_BACKGROUND
 * @return The pixel buffer of the current frame. *
 */
unsigned char* DecodeOneFrame( int& delay, GifFileType* gifInfo, const Dali::Vector<unsigned char>& backgroundColor,
                               unsigned int frameIndex, unsigned char*& lastPreservedFrame,
                               unsigned char*& previousFrame, Dali::Rect<int>& clearFrameArea )
{
  // Fetch the graphics control block
  GraphicsControlBlock graphicsControlBlock;
  if( int errorCode = DGifSavedExtensionToGCB( gifInfo, frameIndex, &graphicsControlBlock ) != GIF_OK
      && gifInfo->ImageCount > 1 ) // for static gif, graphics control block may not been specified
  {
    DALI_LOG_ERROR( "GIF Loader: DGifSavedExtensionToGCB Error. Code: %d\n", errorCode );
  }

  // Read frame delay time, multiply 10 to change time unit to millisecods
  delay = graphicsControlBlock.DelayTime * 10.f;

  const int width = gifInfo->SWidth;
  const int height = gifInfo->SHeight;

  const SavedImage& frame = gifInfo->SavedImages[frameIndex];

  // get the color map. If there is a local one, use the local color map, otherwise use the global color map
  ColorMapObject* colorMap = frame.ImageDesc.ColorMap ? frame.ImageDesc.ColorMap : gifInfo->SColorMap;
  if (colorMap == NULL || colorMap->ColorCount != (1 << colorMap->BitsPerPixel))
  {
    DALI_LOG_WARNING( "GIF Loader: potentially corrupt color map\n" );
    return NULL;
  }

  // Allocate the buffer
  int bufferSize = width*height*4;
  unsigned char* buffer = new unsigned char[ bufferSize ];

  // check whether buffer initializetion is needed
  bool completelyCovered = graphicsControlBlock.TransparentColor == NO_TRANSPARENT_COLOR
                           && frame.ImageDesc.Left == 0 && frame.ImageDesc.Top == 0
                           && frame.ImageDesc.Width == width && frame.ImageDesc.Height == height;

   // if not completely covered, needs to initialise the pixels
  // depends on the disposal method, it would be initialised to background color, previous frame or the last preserved frame
  if( !completelyCovered )
  {
    if( previousFrame && ( graphicsControlBlock.DisposalMode == DISPOSAL_UNSPECIFIED
                        || graphicsControlBlock.DisposalMode == DISPOSE_DO_NOT
                        || graphicsControlBlock.DisposalMode == DISPOSE_BACKGROUND) )
    {
      // disposal none: overlaid on the previous frame
      if( clearFrameArea.height < height || clearFrameArea.width < width )
      {
        for( int i = 0; i < bufferSize; i++  )
        {
          buffer[i] = previousFrame[i];
        }
      }
      // background disposal: When the time delay is finished for a particular frame, the area that was overlaid by that frame is cleared.
      // Not the whole canvas, just the area that was overlaid. Once that is done then the resulting canvas is what is passed to the next frame of the animation,
      // to be overlaid by that frames image.
      for( int row = 0; row < clearFrameArea.height; row++  )
      {
        int idx = ( clearFrameArea.y + row)* width *4 + clearFrameArea.x * 4 + 3;
        for( int col = 0; col < clearFrameArea.width; col++, idx+=4  )
        {
          buffer[idx] = 0x00;
         }
      }
    }
    else if( lastPreservedFrame && graphicsControlBlock.DisposalMode == DISPOSE_PREVIOUS )
    {
      // previous disposal: When the current image is finished, return the canvas to what it looked like before the image was overlaid.
      for( int i = 0; i < bufferSize; i++  )
      {
        buffer[i] = lastPreservedFrame[i];
      }
    }
    else if( !previousFrame && graphicsControlBlock.DisposalMode == DISPOSE_BACKGROUND)
    {
      // background disposal for first frame: clear to transparency
      for( int i = 3; i < bufferSize; i+=4  )
      {
        buffer[i] = 0x00;
      }
    }
    else
    {
      for( int i = 0; i < bufferSize; i+=4  )
      {
        buffer[i] = backgroundColor[0];
        buffer[i+1] = backgroundColor[1];
        buffer[i+2] = backgroundColor[2];
        buffer[i+3] = backgroundColor[3];
      }
    }
  }

  unsigned char* source = frame.RasterBits;
  bool replace = completelyCovered || (frameIndex == 0 && graphicsControlBlock.DisposalMode != DISPOSE_BACKGROUND);
  GifCopyFrame( buffer, source, width, height, frame.ImageDesc, colorMap, graphicsControlBlock.TransparentColor, replace );

  // update the pixel buffer of the previous frame and the last preserved frame
  if( graphicsControlBlock.DisposalMode != DISPOSE_BACKGROUND && graphicsControlBlock.DisposalMode != DISPOSE_PREVIOUS )
  {
    lastPreservedFrame = buffer;
  }
  previousFrame = buffer;
  if( graphicsControlBlock.DisposalMode == DISPOSE_BACKGROUND )
  {
    clearFrameArea.x = frame.ImageDesc.Left;
    clearFrameArea.y = frame.ImageDesc.Top;
    clearFrameArea.width = frame.ImageDesc.Width;
    clearFrameArea.height = frame.ImageDesc.Height;
  }
  else
  {
    clearFrameArea.width = 0;
    clearFrameArea.height = 0;
  }

  return buffer;
}

} // Anonymous namespace

namespace Dali
{

bool LoadAnimatedGifFromFile( const std::string& url, std::vector<Dali::PixelData>& pixelData, Dali::Vector<uint32_t>& frameDelays )
{
  // open GIF file
  GifFileType* gifInfo = NULL;
  GifAutoCleanup autoGif( gifInfo );
  // enforce clean-up of the GIF structure when finishing this method.
  if( !GifOpen( url.c_str(), gifInfo ) )
  {
    return false;
  }

  // read GIF file
  if( DGifSlurp( gifInfo ) != GIF_OK )
  {
    DALI_LOG_ERROR( "GIF Loader: DGifSlurp failed. \n" );
    return false;
  }

  // validate attributes
  if( gifInfo->ImageCount < 1 )
  {
    DALI_LOG_ERROR( "GIF Loader: frame count < 1. \n" );
    return false;
  }

  // read the image size and frame count
  ImageDimensions size( gifInfo->SWidth, gifInfo->SHeight );

  // process frames
  unsigned char* previousFrame = NULL;
  unsigned char* lastPreservedFrame = NULL;

  // previous frame area
  Rect<int> clearFrameArea;

  // get background color
  Dali::Vector<unsigned char> backgroundColor;
  backgroundColor.Resize( 4 );
  ColorMapObject* globalColorMap = gifInfo->SColorMap;
  if( gifInfo->SColorMap )
  {
    backgroundColor[0] = globalColorMap->Colors[ gifInfo->SBackGroundColor ].Red;
    backgroundColor[1] = globalColorMap->Colors[ gifInfo->SBackGroundColor ].Green;
    backgroundColor[2] = globalColorMap->Colors[ gifInfo->SBackGroundColor ].Blue;
    backgroundColor[3] = 0xff;
  }

  int delay;
  unsigned char* buffer = NULL;
  // decode the gif frame by frame
  pixelData.clear();
  frameDelays.Clear();
  for( int i = 0; i < gifInfo->ImageCount; i++ )
  {
    buffer = DecodeOneFrame( delay, gifInfo, backgroundColor, i, lastPreservedFrame, previousFrame, clearFrameArea );
    if( buffer )
    {
      pixelData.push_back( Dali::PixelData::New( buffer, size.GetWidth()*size.GetHeight()*4,
                                                     size.GetWidth(), size.GetHeight(),
                                                     Dali::Pixel::RGBA8888, Dali::PixelData::DELETE_ARRAY ) );
      frameDelays.PushBack( delay );
    }
    else
    {
      DALI_LOG_ERROR( "GIF Loader: Loade frame data fail. FrameIndex: %d\n", i );
    }
  }

  return true;
}

ImageDimensions GetGifImageSize( const std::string& url )
{
  GifFileType* gifInfo = NULL;
 // enforce clean-up of the GIF structure when finishing this method.
  GifAutoCleanup autoGif( gifInfo );
  if( !GifOpen( url.c_str(), gifInfo ) )
  {
    return ImageDimensions();
  }
  return ImageDimensions( gifInfo->SWidth, gifInfo->SHeight );
}

} // namespace Dali
