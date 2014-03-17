//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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

void TestImageLoading( const ImageDetails& image, const LoadFunctions& functions )
{
  FILE* fp = fopen( image.name.c_str() , "rb" );
  AutoCloseFile autoClose( fp );
  DALI_TEST_CHECK( fp != NULL );

  // Check the header file.

  unsigned int width(0), height(0);
  Dali::ImageAttributes attributes;
  DALI_TEST_CHECK( functions.header( fp, attributes, width, height ) );

  DALI_TEST_EQUALS( width,  image.reportedWidth,  TEST_LOCATION );
  DALI_TEST_EQUALS( height, image.reportedHeight, TEST_LOCATION );

  // Loading the header moves the pointer within the file so reset to start of file.
  fseek( fp, 0, 0 );

  // Create a bitmap object and store a pointer to that object so it is destroyed at the end.
  Dali::Integration::ImageDataPtr bitmap = Integration::NewBitmapImageData( 64, 64, Pixel::A8 );

  // Load Bitmap and check its return values.
  DALI_TEST_CHECK( functions.loader( fp, attributes, bitmap ) );
  DALI_TEST_EQUALS( image.width,  attributes.GetWidth(),  TEST_LOCATION );
  DALI_TEST_EQUALS( image.height, attributes.GetHeight(), TEST_LOCATION );

  // Compare buffer generated with reference buffer.
  Dali::PixelBuffer* bufferPtr( bitmap->GetBuffer() );
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

void DumpImageBufferToTempFile( std::string filename, std::string targetFilename, const LoadFunctions& functions )
{
  FILE* fp = fopen( filename.c_str() , "rb" );
  AutoCloseFile autoClose( fp );

  Dali::Integration::ImageDataPtr bitmap = Integration::NewBitmapImageData( 64, 64, Pixel::A8 );
  Dali::ImageAttributes attributes;

  DALI_TEST_CHECK( functions.loader( fp, attributes, bitmap ) );

  Dali::PixelBuffer* bufferPtr( bitmap->GetBuffer() );

  FILE* writeFp = fopen( targetFilename.c_str(), "wb" );
  AutoCloseFile autoCloseWrite( writeFp );
  fwrite( bufferPtr, sizeof( Dali::PixelBuffer ), attributes.GetWidth() * attributes.GetHeight(), writeFp );
}
