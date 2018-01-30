#ifndef __DALI_TIZEN_PLATFORM_IMAGE_LOADER_H__
#define __DALI_TIZEN_PLATFORM_IMAGE_LOADER_H__

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/images/image-operations.h>
#include <dali/integration-api/resource-types.h>
#include <dali/integration-api/bitmap.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <string>

namespace Dali
{
namespace Integration
{
typedef IntrusivePtr<Dali::RefObject> ResourcePointer;
} // Integration

namespace TizenPlatform
{
namespace ImageLoader
{
/**
 * Convert a file stream into a bitmap.
 * @param[in] resource The resource to convert.
 * @param[in] path The path to the resource.
 * @param[in] fp File Pointer. Closed on exit.
 * @param[out] bitmap Pointer to write bitmap to
 * @return true on success, false on failure
 */
bool ConvertStreamToBitmap( const Integration::BitmapResourceType& resource, std::string path, FILE * const fp, Dali::Devel::PixelBuffer& pixelBuffer );

/**
 * Convert a bitmap and write to a file stream.
 * @param[in] path The path to the resource.
 * @param[in] fp File Pointer. Closed on exit.
 * @param[out] pixelData Reference to PixelData object.
 * @return true on success, false on failure
 */
bool ConvertBitmapToStream( std::string path, FILE * const fp, Dali::Devel::PixelBuffer& pixelBuffer );

/**
 * Loads an image synchronously
 * @param resource details of the image
 * @param path to the image
 * @return bitmap
 */
Integration::ResourcePointer LoadImageSynchronously( const Integration::BitmapResourceType& resource, const std::string& path );

/**
 * @returns the closest image size
 */
ImageDimensions  GetClosestImageSize( const std::string& filename,
                          ImageDimensions size,
                          FittingMode::Type fittingMode,
                          SamplingMode::Type samplingMode,
                          bool orientationCorrection );

/**
 * @returns the closest image size
 */
ImageDimensions GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                          ImageDimensions size,
                          FittingMode::Type fittingMode,
                          SamplingMode::Type samplingMode,
                          bool orientationCorrection );

} // ImageLoader
} // TizenPlatform
} // Dali

#endif // __DALI_TIZEN_PLATFORM_IMAGE_LOADER_H__
