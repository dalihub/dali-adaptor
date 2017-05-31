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

#include "image-loaders.h"
#include <dali-test-suite-utils.h>


AutoCloseFile::AutoCloseFile( FILE *fp )
: filePtr( fp )
{
}

AutoCloseFile::~AutoCloseFile()
{
  if ( filePtr )
  {
    fclose( filePtr );
  }
}


ImageDetails::ImageDetails( const char * const _name, unsigned int _width, unsigned int _height )
: name( _name ),
  width( _width ),
  height( _height ),
  reportedWidth( _width ),
  reportedHeight( _height ),
  refBufferSize( _width * _height ),
  refBuffer( new Dali::PixelBuffer[ refBufferSize ] )
{
  LoadBuffer();
}

ImageDetails::ImageDetails( const char * const _name, unsigned int _width, unsigned int _height, unsigned int _reportedWidth, unsigned int _reportedHeight )
: name( _name ),
  width( _width ),
  height( _height ),
  reportedWidth( _reportedWidth ),
  reportedHeight( _reportedHeight ),
  refBufferSize( _width * _height ),
  refBuffer( new Dali::PixelBuffer[ refBufferSize ] )
{
  LoadBuffer();
}

ImageDetails::~ImageDetails()
{
  delete [] refBuffer;
}

void ImageDetails::LoadBuffer()
{
  // Load the reference buffer from the buffer file

  std::string refBufferFilename( name + ".buffer" );
  FILE *fp = fopen ( refBufferFilename.c_str(), "rb" );
  AutoCloseFile autoCloseBufferFile( fp );

  if ( fp )
  {
    fread( refBuffer, sizeof( Dali::PixelBuffer ), refBufferSize, fp );
  }
}


LoadFunctions::LoadFunctions( LoadBitmapHeaderFunction _header, LoadBitmapFunction _loader )
: header( _header ),
  loader( _loader )
{
}

void TestImageLoading( const ImageDetails& image, const LoadFunctions& functions, Dali::Integration::Bitmap::Profile bitmapProfile )
{
  FILE* fp = fopen( image.name.c_str() , "rb" );
  AutoCloseFile autoClose( fp );
  DALI_TEST_CHECK( fp != NULL );

  // Check the header file.

  unsigned int width(0), height(0);
  const Dali::TizenPlatform::ImageLoader::Input input( fp );
  DALI_TEST_CHECK( functions.header( input, width, height ) );

  DALI_TEST_EQUALS( width,  image.reportedWidth,  TEST_LOCATION );
  DALI_TEST_EQUALS( height, image.reportedHeight, TEST_LOCATION );

  // Loading the header moves the pointer within the file so reset to start of file.
  fseek( fp, 0, 0 );

  // Create a bitmap object and store a pointer to that object so it is destroyed at the end.
  Dali::Integration::Bitmap * bitmap = Dali::Integration::Bitmap::New( bitmapProfile, ResourcePolicy::OWNED_RETAIN  );
  Dali::Integration::BitmapPtr bitmapPtr( bitmap );

  // Load Bitmap and check its return values.
  DALI_TEST_CHECK( functions.loader( input, *bitmap ) );
  DALI_TEST_EQUALS( image.width,  bitmap->GetImageWidth(),  TEST_LOCATION );
  DALI_TEST_EQUALS( image.height, bitmap->GetImageHeight(), TEST_LOCATION );

  // Compare buffer generated with reference buffer.
  Dali::PixelBuffer* bufferPtr( bitmapPtr->GetBuffer() );
  Dali::PixelBuffer* refBufferPtr( image.refBuffer );
  for ( unsigned int i = 0; i < image.refBufferSize; ++i, ++bufferPtr, ++refBufferPtr )
  {
    if( *bufferPtr != *refBufferPtr )
    {
      tet_result( TET_FAIL );
      tet_printf("%s Failed in %s at line %d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__);
      break;
    }
  }
}

void CompareLoadedImageData( const ImageDetails& image, const LoadFunctions& functions, const uint32_t* master )
{
  FILE* filePointer = fopen( image.name.c_str() , "rb" );
  AutoCloseFile autoClose( filePointer );
  DALI_TEST_CHECK( filePointer != NULL );

  // Check the header file.
  unsigned int width = 0, height = 0;
  const Dali::TizenPlatform::ImageLoader::Input input( filePointer );
  DALI_TEST_CHECK( functions.header( input, width, height ) );

  DALI_TEST_EQUALS( width,  image.reportedWidth,  TEST_LOCATION );
  DALI_TEST_EQUALS( height, image.reportedHeight, TEST_LOCATION );

  // Loading the header moves the pointer within the file so reset to start of file.
  fseek( filePointer, 0, SEEK_SET );

  // Create a bitmap object and store a pointer to that object so it is destroyed at the end.
  Dali::Integration::Bitmap * bitmap = Dali::Integration::Bitmap::New( Dali::Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_RETAIN  );
  Dali::Integration::BitmapPtr bitmapPointer( bitmap );

  // Load Bitmap and check its return values.
  DALI_TEST_CHECK( functions.loader( input, *bitmap ) );
  DALI_TEST_EQUALS( image.width,  bitmap->GetImageWidth(),  TEST_LOCATION );
  DALI_TEST_EQUALS( image.height, bitmap->GetImageHeight(), TEST_LOCATION );

  // Check the bytes per pixel.
  const Pixel::Format pixelFormat = bitmap->GetPixelFormat();
  const unsigned int bytesPerPixel = Pixel::GetBytesPerPixel( pixelFormat );

  // Compare buffer generated with reference buffer.
  Dali::PixelBuffer* pBitmapData( bitmapPointer->GetBuffer() );
  const uint32_t* pMaster( master );

  // Loop through each pixel in the bitmap.
  for ( unsigned int i = 0; i < image.refBufferSize; ++i, ++pMaster )
  {
    unsigned int color = 0;
    // Loop through each byte per pixel, to build up a color value for that pixel.
    for( unsigned int j = 0; j < bytesPerPixel; ++j, ++pBitmapData )
    {
      color = ( color << 8 ) | *pBitmapData;
    }

    // Check the color value is what we expect.
    DALI_TEST_EQUALS( color, *pMaster, TEST_LOCATION );
  }
}

void DumpImageBufferToTempFile( std::string filename, std::string targetFilename, const LoadFunctions& functions )
{
  FILE* fp = fopen( filename.c_str() , "rb" );
  AutoCloseFile autoClose( fp );

  Dali::Integration::Bitmap* bitmap = Dali::Integration::Bitmap::New( Dali::Integration::Bitmap::BITMAP_2D_PACKED_PIXELS,  ResourcePolicy::OWNED_RETAIN );
  Dali::Integration::BitmapPtr bitmapPtr( bitmap );
  const Dali::TizenPlatform::ImageLoader::Input input( fp );

  DALI_TEST_CHECK( functions.loader( input, *bitmap ) );

  Dali::PixelBuffer* bufferPtr( bitmapPtr->GetBuffer() );

  FILE* writeFp = fopen( targetFilename.c_str(), "wb" );
  AutoCloseFile autoCloseWrite( writeFp );
  fwrite( bufferPtr, 1, bitmap->GetBufferSize(), writeFp );
}
