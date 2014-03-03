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

#ifndef __DALI_ADAPTOR_TET_IMAGE_LOADERS_H_
#define __DALI_ADAPTOR_TET_IMAGE_LOADERS_H_

#include <tet_api.h>
#include <dali/dali.h>
#include <dali/integration-api/bitmap.h>

// Simple structure to close the file when finished with it.
struct AutoCloseFile
{
  AutoCloseFile( FILE *fp )
  : filePtr( fp )
  {
  }

  ~AutoCloseFile()
  {
    if ( filePtr )
    {
      fclose( filePtr );
    }
  }

  FILE* filePtr;
};

/// Structure to hold image details and the reference buffer.
struct ImageDetails
{
  /**
   * Normal Constructor.
   *
   * @param[in]  _name    The name of the image to open.  The reference buffer file should have the same name appended with ".buffer".
   * @param[in]  _width   The width of the image.
   * @param[in]  _height  The height of the image.
   */
  ImageDetails( const char * const _name, unsigned int _width, unsigned int _height )
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

  /**
   * Sometimes an image reports an incorrect size in the header than what it actually is. In such a
   * scenario, this constructor should be used.
   *
   * @param[in]  _name            The name of the image to open.  The reference buffer file should have the same name appended with ".buffer".
   * @param[in]  _width           The width of the image.
   * @param[in]  _height          The height of the image.
   * @param[in]  _reportedWidth   The reported width of the image by reading the header.
   * @param[in]  _reportedHeight  The reported height of the image by reading the header.
   */
  ImageDetails( const char * const _name, unsigned int _width, unsigned int _height, unsigned int _reportedWidth, unsigned int _reportedHeight )
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

  /**
   * Destructor
   */
  ~ImageDetails()
  {
    delete [] refBuffer;
  }

  std::string name;
  unsigned int width;
  unsigned int height;
  unsigned int reportedWidth;
  unsigned int reportedHeight;
  unsigned int refBufferSize;
  Dali::PixelBuffer* const refBuffer;

private:

  /**
   * Loads the reference buffer file.
   */
  void LoadBuffer()
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
};

/**
 * A structure storing the methods that should be called when reading an image's header and when
 * reading the bitmap from the image file.
 */
struct LoadFunctions
{
  typedef bool (*LoadBitmapHeaderFunction)(FILE*, unsigned int&, unsigned int&);
  typedef bool (*LoadBitmapFunction)(FILE*, Dali::Integration::Bitmap&, Dali::ImageAttributes&);

  LoadFunctions( LoadBitmapHeaderFunction _header, LoadBitmapFunction _loader )
  : header( _header ),
    loader( _loader )
  {
  }

  LoadBitmapHeaderFunction header;
  LoadBitmapFunction loader;
};

// Helper method to test each image file.
/**
 * Use this method to test the header and and bitmap loading of each image.
 * The loaded bitmap is then checked with the reference bitmap in ImageDetails.
 *
 * @param[in]  image      The image details.
 * @param[in]  functions  The loader functions that need to be called.
 */
void TestImageLoading( const ImageDetails& image, const LoadFunctions& functions )
{
  FILE* fp = fopen( image.name.c_str() , "rb" );
  AutoCloseFile autoClose( fp );
  DALI_TEST_CHECK( fp != NULL );

  // Check the header file.

  unsigned int width(0), height(0);
  DALI_TEST_CHECK( functions.header( fp, width, height ) );

  Dali::DALI_TEST_EQUALS( width,  image.reportedWidth,  TEST_LOCATION );
  Dali::DALI_TEST_EQUALS( height, image.reportedHeight, TEST_LOCATION );

  // Loading the header moves the pointer within the file so reset to start of file.
  fseek( fp, 0, 0 );

  // Create a bitmap object and store a pointer to that object so it is destroyed at the end.
  Dali::Integration::Bitmap * bitmap = Dali::Integration::Bitmap::New( Dali::Integration::Bitmap::BITMAP_2D_PACKED_PIXELS,  false  );
  Dali::Integration::BitmapPtr bitmapPtr( bitmap );

  Dali::ImageAttributes attributes;

  // Load Bitmap and check its return values.
  DALI_TEST_CHECK( functions.loader( fp, *bitmap, attributes ) );
  Dali::DALI_TEST_EQUALS( image.width,  attributes.GetWidth(),  TEST_LOCATION );
  Dali::DALI_TEST_EQUALS( image.height, attributes.GetHeight(), TEST_LOCATION );

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

/**
 * Helper function which should be used when first creating a reference buffer file.
 * Set output file to a file in the /tmp/ directory e.g:
 *   DumpImageBufferToTempFile( "images/pattern.gif" , "/tmp/pattern.gif.buffer" );
 *
 * @param[in]  filename        The path of the image file.
 * @param[in]  targetFilename  The path of where the buffer should be written to.  This should ideally be in the "/tmp" folder.
 * @param[in]  functions       The loader functions to call.
 */
void DumpImageBufferToTempFile( std::string filename, std::string targetFilename, const LoadFunctions& functions )
{
  FILE* fp = fopen( filename.c_str() , "rb" );
  AutoCloseFile autoClose( fp );

  Dali::Integration::Bitmap* bitmap = Dali::Integration::Bitmap::New( Dali::Integration::Bitmap::BITMAP_2D_PACKED_PIXELS,  false );
  Dali::Integration::BitmapPtr bitmapPtr( bitmap );
  Dali::ImageAttributes attributes;

  DALI_TEST_CHECK( functions.loader( fp, *bitmap, attributes ) );

  Dali::PixelBuffer* bufferPtr( bitmapPtr->GetBuffer() );

  FILE* writeFp = fopen( targetFilename.c_str(), "wb" );
  AutoCloseFile autoCloseWrite( writeFp );
  fwrite( bufferPtr, sizeof( Dali::PixelBuffer ), attributes.GetWidth() * attributes.GetHeight(), writeFp );
}

#endif // __DALI_ADAPTOR_TET_IMAGE_LOADERS_H_
