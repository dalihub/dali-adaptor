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
#include <dali/internal/imaging/common/loader-astc.h>

// EXTERNAL INCLUDES
#include <cstring>
#include <dali/public-api/common/compile-time-assert.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{

// Max width or height of an image.
const unsigned MAX_TEXTURE_DIMENSION = 4096;
// Max bytes of image data allowed. Not a precise number, just a sanity check.
const unsigned MAX_IMAGE_DATA_SIZE = MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION;

// Minimum and maximum possible sizes for ASTC blocks.
const unsigned int MINIMUM_ASTC_BLOCK_SIZE = 4;
const unsigned int MAXIMUM_ASTC_BLOCK_SIZE = 12;

typedef uint8_t Byte;

// This bytes identify an ASTC native file.
const Byte FileIdentifier[] = {
   0x13, 0xAB, 0xA1, 0x5C
};


/**
 * @brief This struct defines the ASTC file header values. From ASTC specifications.
 * Packed attribute stops the structure from being aligned to compiler defaults
 * so we can be sure of reading the whole header from file in one call to fread().
 * Note: members to not conform to coding standards in order to be consistent with ASTC spec.
 */
struct AstcFileHeader
{
  unsigned char magic[ 4 ];
  unsigned char blockdim_x;
  unsigned char blockdim_y;
  unsigned char blockdim_z;
  unsigned char xsize[ 3 ];
  unsigned char ysize[ 3 ];
  unsigned char zsize[ 3 ];
} __attribute__ ( (__packed__));

using namespace Pixel;

/**
 * @brief This table allows fast conversion from an ASTC block size ([height][width]) to a pixel format.
 * This could be done within a switch, but this way we have a constant time function.
 * Note: As 4 is the minimum block size, 4 is subtracted from both the width and height to optimise size.
 * IE. Table format is: Increasing order of block width from left-to-right:  4 -> 12
 *                      Increasing order of block height from top-to-bottom: 4 -> 12
 */
Pixel::Format AstcLinearBlockSizeToPixelFormatTable[][( MAXIMUM_ASTC_BLOCK_SIZE - MINIMUM_ASTC_BLOCK_SIZE ) + 1] = {
    { COMPRESSED_RGBA_ASTC_4x4_KHR, COMPRESSED_RGBA_ASTC_5x4_KHR, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID },
    { INVALID, COMPRESSED_RGBA_ASTC_5x5_KHR, COMPRESSED_RGBA_ASTC_6x5_KHR, INVALID, COMPRESSED_RGBA_ASTC_8x5_KHR, INVALID, COMPRESSED_RGBA_ASTC_10x5_KHR, INVALID, INVALID },
    { INVALID, INVALID, COMPRESSED_RGBA_ASTC_6x6_KHR, INVALID, COMPRESSED_RGBA_ASTC_8x6_KHR, INVALID, COMPRESSED_RGBA_ASTC_10x6_KHR, INVALID, INVALID },
    { INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID },
    { INVALID, INVALID, INVALID, INVALID, COMPRESSED_RGBA_ASTC_8x8_KHR, INVALID, COMPRESSED_RGBA_ASTC_10x8_KHR, INVALID, INVALID },
    { INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID },
    { INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, COMPRESSED_RGBA_ASTC_10x10_KHR, INVALID, COMPRESSED_RGBA_ASTC_12x10_KHR },
    { INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID },
    { INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, COMPRESSED_RGBA_ASTC_12x12_KHR }
};

/**
 * @brief Uses header information to return the respective ASTC pixel format.
 *
 * @param[in] header A populated AstcFileHeader struct
 * @return    The pixel format, or INVALID if the block size was invalid
 */
Pixel::Format GetAstcPixelFormat( AstcFileHeader& header )
{
  // Check the block size is valid. This will also prevent an invalid read from the conversion table.
  if( ( header.blockdim_x < MINIMUM_ASTC_BLOCK_SIZE ) || ( header.blockdim_x > MAXIMUM_ASTC_BLOCK_SIZE ) ||
      ( header.blockdim_y < MINIMUM_ASTC_BLOCK_SIZE ) || ( header.blockdim_y > MAXIMUM_ASTC_BLOCK_SIZE ) )
  {
    return Pixel::INVALID;
  }

  // Read the equivalent pixel format from the conversion table.
  return AstcLinearBlockSizeToPixelFormatTable[ header.blockdim_y - MINIMUM_ASTC_BLOCK_SIZE ][ header.blockdim_x - MINIMUM_ASTC_BLOCK_SIZE ];
}

/**
 * @brief Internal method to load ASTC header info from a file.
 *
 * @param[in]  filePointer The file pointer to the ASTC file to read
 * @param[out] width       The width is output to this value
 * @param[out] height      The height is output to this value
 * @param[out] fileHeader  This will be populated with the header data
 * @return                 True if the file is valid, false otherwise
 */
bool LoadAstcHeader( FILE * const filePointer, unsigned int& width, unsigned int& height, AstcFileHeader& fileHeader )
{
  // Pull the bytes of the file header in as a block:
  const unsigned int readLength = sizeof( AstcFileHeader );
  if( fread( &fileHeader, 1, readLength, filePointer ) != readLength )
  {
    return false;
  }

  // Check the header contains the ASTC native file identifier.
  bool headerIsValid = memcmp( fileHeader.magic, FileIdentifier, sizeof( fileHeader.magic ) ) == 0;
  if( !headerIsValid )
  {
    DALI_LOG_ERROR( "File is not a valid ASTC native file\n" );
    // Return here as otherwise, if not a valid ASTC file, we are likely to pick up other header errors spuriously.
    return false;
  }

  // Convert the 3-byte values for width and height to a single resultant value.
  width = fileHeader.xsize[0] | ( fileHeader.xsize[1] << 8 ) | ( fileHeader.xsize[2] << 16 );
  height = fileHeader.ysize[0] | ( fileHeader.ysize[1] << 8 ) | ( fileHeader.ysize[2] << 16 );

  const unsigned int zDepth = fileHeader.zsize[0] + ( fileHeader.zsize[1] << 8 ) + ( fileHeader.zsize[2] << 16 );

  // Check image dimensions are within limits.
  if( ( width > MAX_TEXTURE_DIMENSION ) || ( height > MAX_TEXTURE_DIMENSION ) )
  {
    DALI_LOG_ERROR( "ASTC file has larger than supported dimensions: %d,%d\n", width, height );
    headerIsValid = false;
  }

  // Confirm the ASTC block does not have any Z depth.
  if( zDepth != 1 )
  {
    DALI_LOG_ERROR( "ASTC files with z size other than 1 are not supported. Z size is: %d\n", zDepth );
    headerIsValid = false;
  }

  return headerIsValid;
}

} // Unnamed namespace.


// File loading API entry-point:
bool LoadAstcHeader( const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height )
{
  AstcFileHeader fileHeader;
  return LoadAstcHeader( input.file, width, height, fileHeader );
}

// File loading API entry-point:
bool LoadBitmapFromAstc( const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap )
{
  FILE* const filePointer = input.file;
  if( !filePointer )
  {
    DALI_LOG_ERROR( "Null file handle passed to ASTC compressed bitmap file loader.\n" );
    return false;
  }

  // Load the header info.
  AstcFileHeader fileHeader;
  unsigned int width, height;

  if( !LoadAstcHeader( filePointer, width, height, fileHeader ) )
  {
    DALI_LOG_ERROR( "Could not load ASTC Header from file.\n" );
    return false;
  }

  // Retrieve the pixel format from the ASTC block size.
  Pixel::Format pixelFormat = GetAstcPixelFormat( fileHeader );
  if( pixelFormat == Pixel::INVALID )
  {
    DALI_LOG_ERROR( "No internal pixel format supported for ASTC file pixel format.\n" );
    return false;
  }

  // Retrieve the file size.
  if( fseek( filePointer, 0L, SEEK_END ) )
  {
    DALI_LOG_ERROR( "Could not seek through file.\n" );
    return false;
  }

  off_t fileSize = ftell( filePointer );
  if( fileSize == -1L )
  {
    DALI_LOG_ERROR( "Could not determine ASTC file size.\n" );
    return false;
  }

  if( fseek( filePointer, sizeof( AstcFileHeader ), SEEK_SET ) )
  {
    DALI_LOG_ERROR( "Could not seek through file.\n" );
    return false;
  }

  // Data size is file size - header size.
  size_t imageByteCount = fileSize - sizeof( AstcFileHeader );

  // Sanity-check the image data is not too large and that it is at less than 2 bytes per texel:
  if( ( imageByteCount > MAX_IMAGE_DATA_SIZE ) || ( imageByteCount > ( ( static_cast< size_t >( width ) * height ) << 1 ) ) )
  {
    DALI_LOG_ERROR( "ASTC file has too large image-data field.\n" );
    return false;
  }

  // allocate pixel data
  bitmap = Dali::Devel::PixelBuffer::New(width, height, pixelFormat);

  // Compressed format won't allocate the buffer
  auto pixels = bitmap.GetBuffer();
  if( !pixels )
  {
    // allocate buffer manually
    auto& impl = GetImplementation( bitmap );
    impl.AllocateFixedSize( imageByteCount );
    pixels = bitmap.GetBuffer();
  }

  // Load the image data.
  const size_t bytesRead = fread( pixels, 1, imageByteCount, filePointer );

  // Check the size of loaded data is what we expected.
  if( bytesRead != imageByteCount )
  {
    DALI_LOG_ERROR( "Read of image pixel data failed.\n" );
    return false;
  }

  return true;
}

} // namespace TizenPlatform

} // namespace Dali
