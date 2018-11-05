#ifndef __DALI_TIZEN_PLATFORM_IMAGE_LOADER_INPUT_H__
#define __DALI_TIZEN_PLATFORM_IMAGE_LOADER_INPUT_H__

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
 */

// EXTERNAL INCLUDES
#include <cstdio>
#include <dali/public-api/images/image-operations.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/bitmap.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace ImageLoader
{

/**
 * @brief A simple immutable struct to bundle together parameters for scaling an image.
 */
class ScalingParameters
{
public:
  ScalingParameters( ImageDimensions dimensions = ImageDimensions(), FittingMode::Type fittingMode = FittingMode::DEFAULT, SamplingMode::Type samplingMode = SamplingMode::DEFAULT ) :
    dimensions(dimensions), scalingMode(fittingMode), samplingMode(samplingMode) {}
  const ImageDimensions dimensions;
  const FittingMode::Type scalingMode;
  const SamplingMode::Type samplingMode;
};

  /**
   * @brief Bundle-up the data pushed into an image loader.
   */
struct Input
{
  Input( FILE* file, ScalingParameters scalingParameters = ScalingParameters(), bool reorientationRequested = true ) :
    file(file), scalingParameters(scalingParameters), reorientationRequested(reorientationRequested) {}
  FILE* file;
  ScalingParameters scalingParameters;
  bool reorientationRequested;
};


using LoadBitmapFunction = bool( * )( const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& pixelData );
using LoadBitmapHeaderFunction = bool( * )( const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height );

/**
 * Stores the magic bytes, and the loader and header functions used for each image loader.
 */
struct BitmapLoader
{
  unsigned char magicByte1;        ///< The first byte in the file should be this
  unsigned char magicByte2;        ///< The second byte in the file should be this
  LoadBitmapFunction loader;       ///< The function which decodes the file
  LoadBitmapHeaderFunction header; ///< The function which decodes the header of the file
  Dali::Integration::Bitmap::Profile profile;         ///< The kind of bitmap to be created
                                   ///  (addressable packed pixels or an opaque compressed blob).
};

} // ImageLoader
} // Dali

#endif // __DALI_TIZEN_PLATFORM_IMAGE_LOADER_INPUT_H__
