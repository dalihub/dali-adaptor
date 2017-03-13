/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include "emscripten-platform-abstraction.h"

// EXTERNAL INCLUDES
#include <set>
#include <stdint.h>
#include <cstring>
#include <SDL_surface.h>
#include <SDL_image.h>
#include "emscripten/emscripten.h"
#include "emscripten-callbacks.h"

#define EM_LOG(x); // EM_ASM( console.log( x ) );


// INTERNAL INCLUDES
#include <dali/integration-api/platform-abstraction.h>
#include <dali/devel-api/common/ref-counted-dali-vector.h>

namespace
{

Dali::Integration::BitmapPtr LoadResourceEncodedImage( Dali::RefCountedVector<uint8_t>* encodedBlob )
{
  Dali::Integration::BitmapPtr bitmapPtr = NULL;

  if( encodedBlob != 0 )
  {
    const size_t blobSize     = encodedBlob->GetVector().Size();
    uint8_t * const blobBytes = &(encodedBlob->GetVector()[0]);
    DALI_ASSERT_DEBUG( blobSize > 0U );
    DALI_ASSERT_DEBUG( blobBytes != 0U );

    if( blobBytes != 0 && blobSize > 0U )
    {

      SDL_RWops *memory = SDL_RWFromMem(blobBytes, blobSize);

      if(!memory)
      {
        printf("  Error Null pointer from SDL RW memory?\n");
      }

      SDL_Surface *surface = IMG_Load_RW(memory, 0);

      if(surface)
      {
        bitmapPtr = Dali::Integration::Bitmap::New( Dali::Integration::Bitmap::BITMAP_2D_PACKED_PIXELS,
                                                    Dali::ResourcePolicy::OWNED_DISCARD ); // DISCARD; Dali manages

        Dali::Integration::Bitmap::PackedPixelsProfile* packedProfile = bitmapPtr->GetPackedPixelsProfile();
        DALI_ASSERT_ALWAYS(packedProfile);

        unsigned char bytesPerPixel = surface->format->BytesPerPixel;

        Dali::Integration::PixelBuffer* pixels = NULL;

        unsigned char targetBytesPerPixel = 3; // bytesPerPixel;

        // SDL in emscripten returns us a 4byteperpixel image regardless of rgb/png etc
        // Theres no apparent way to differentiate an image with an alpha channel
        // In Dali if an image has an alpha channel it gets sorted. This introduces odd artifacts on rotation
        // as the sorting algorithm presumes front on view.
        // So here well just support pngs with an alpha channel.
        // We're poking around in the format as emscripten currently lacks a file memory api where we could use
        // Dali's machinery to read the format.
        unsigned char *pBytes = blobBytes;

        if( 0x89 == *(pBytes+0) && 0x50 == *(pBytes+1) ) // magic bytes for png_all_filters
        {
          pBytes+=8; // 8 bytes for header
          pBytes+=4; // 4 bytes for chunk length
          pBytes+=4; // 4 bytes for chunk type
          // ihdr data (must be first chunk)
          pBytes+=4; // 4 for width,height
          pBytes+=4;
          pBytes+=1; // 1 for bit depth
          unsigned char ihdr_colorType= *pBytes;    // 1 for bit colorType
          if( (4 == ihdr_colorType ||               // 4 is 8,16 bit depth with alpha LA
               6 == ihdr_colorType) )               // 6 is 8,16 bit depth with alpha RGBA
          {
            targetBytesPerPixel = 4;
          }
        }

        if(3 == targetBytesPerPixel)
        {
          pixels = packedProfile->ReserveBuffer(Dali::Pixel::RGB888,
                                                surface->w, surface->h,
                                                surface->w, surface->h);
        }
        else if(4 == targetBytesPerPixel)
        {
          pixels = packedProfile->ReserveBuffer(Dali::Pixel::RGBA8888,
                                                surface->w, surface->h,
                                                surface->w, surface->h);
        }
        else
        {
          DALI_ASSERT_ALWAYS(0 && "bad bytes per pixel");
        }

        unsigned char* fromPtr = static_cast<unsigned char*>(surface->pixels);

        int stride = surface->pitch;
        int index = 0;
        for(int h = 0; h < surface->h; ++h)
        {
          for(int w = 0; w < (surface->w*bytesPerPixel); w+=bytesPerPixel)
          {
            for(int j = 0; j < targetBytesPerPixel; ++j)
            {
              pixels[ index++ ] = *( (fromPtr + (h * stride) ) + w + j );
            }
          }
        }
      } // if surface
      else
      {
        printf("  Error empty surface when decoding image? (SDL RW Memory ptr=%llx) %s. %d\n", (long long)(memory), SDL_GetError(), blobSize);
      }

    } // if blobSize
    else
    {
      printf(" Error No bytes in image?\n");
    }

  } // if encodedBlob
  else
  {
    printf("  Error Null pointer given for decoding image?\n");
  }

  if(bitmapPtr)
  {
    int x = 0;
    EM_ASM( console.log( "LoadResourceEncodedImage: Image:-" ) );
    x = EM_ASM_INT({
        console.log( $0 ) }, bitmapPtr->GetImageWidth() );
    x = EM_ASM_INT({
        console.log( $0 ) }, bitmapPtr->GetImageHeight() );
    x = EM_ASM_INT({
        console.log( $0 ) }, bitmapPtr->GetBufferSize() );

  }
  else
  {
    EM_ASM( console.log( "LoadResourceEncodedImage: no bitmap data?" ) );
  }

  return bitmapPtr;
}

} // anon namespace

namespace Dali
{

EmscriptenPlatformAbstraction::EmscriptenPlatformAbstraction()
  :
    mSize(10,10)
{
}


EmscriptenPlatformAbstraction::~EmscriptenPlatformAbstraction()
{
}

void EmscriptenPlatformAbstraction::GetTimeMicroseconds(unsigned int &seconds, unsigned int &microSeconds)
{
  double current = EM_ASM_DOUBLE_V({ return new Date().getTime(); }); // getTime() in ms

  seconds         = static_cast<unsigned int>(current/1000.0);
  microSeconds    = (static_cast<unsigned int>(current) - seconds*1000.0) * 1000;
}

ImageDimensions EmscriptenPlatformAbstraction::GetClosestImageSize( const std::string& filename,
                                                             ImageDimensions size,
                                                             FittingMode::Type fittingMode,
                                                             SamplingMode::Type samplingMode,
                                                             bool orientationCorrection )
{
  return Dali::Internal::Emscripten::LoadImageMetadata(filename, size, fittingMode, samplingMode, orientationCorrection);
}

ImageDimensions EmscriptenPlatformAbstraction::GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                                             ImageDimensions size,
                                                             FittingMode::Type fittingMode,
                                                             SamplingMode::Type samplingMode,
                                                             bool orientationCorrection )
{
  // @todo
  return Dali::ImageDimensions(); // Dali::Internal::Emscripten::LoadImageMetadata(filename, size, fittingMode, samplingMode, orientationCorrection);
}

Integration::ResourcePointer EmscriptenPlatformAbstraction::LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath )
{
  Integration::ResourcePointer ret;

  switch(resourceType.id)
  {
    case Integration::ResourceBitmap:
    {
      const Integration::BitmapResourceType& bitmapResource( static_cast<const Integration::BitmapResourceType&>(resourceType) );

      Integration::BitmapPtr bitmapPtr = Dali::Internal::Emscripten::GetImage( bitmapResource.size,
                                                                               bitmapResource.scalingMode,
                                                                               bitmapResource.samplingMode,
                                                                               bitmapResource.orientationCorrection,
                                                                               resourcePath );

      ret = bitmapPtr;
    }
    break;
  } // switch(resourceType->id)

  return ret;
}

Integration::BitmapPtr EmscriptenPlatformAbstraction::DecodeBuffer( const Integration::ResourceType& resourceType, uint8_t * buffer, size_t bufferSize )
{
  return Integration::BitmapPtr();
}


bool EmscriptenPlatformAbstraction::LoadShaderBinaryFile( const std::string& filename, Dali::Vector< unsigned char >& buffer ) const
{
  EM_LOG("EmscriptenPlatformAbstraction::LoadShaderBinaryFile");
  return false;
}

void EmscriptenPlatformAbstraction::UpdateDefaultsFromDevice()
{
  DALI_ASSERT_ALWAYS("!Unimplemented");
  mGetDefaultFontFamilyResult+=1.0f;
}

void EmscriptenPlatformAbstraction::IncrementGetTimeResult(size_t milliseconds)
{
}

} // Dali
