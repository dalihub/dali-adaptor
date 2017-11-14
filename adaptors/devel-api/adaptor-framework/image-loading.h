#ifndef DALI_IMAGE_LOADING_H
#define DALI_IMAGE_LOADING_H

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
#include <string>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/images/image-operations.h>

#ifdef DALI_ADAPTOR_COMPILATION  // full path doesn't exist until adaptor is installed so we have to use relative
// @todo Make dali-adaptor code folder structure mirror the folder structure installed to dali-env
#include <pixel-buffer.h>
#else
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#endif

namespace Dali
{

/**
 * @brief Load an image synchronously from local file.
 *
 * @note This method is thread safe, i.e. can be called from any thread.
 *
 * @param [in] url The URL of the image file to load.
 * @param [in] size The width and height to fit the loaded image to, 0.0 means whole image
 * @param [in] fittingMode The method used to fit the shape of the image before loading to the shape defined by the size parameter.
 * @param [in] samplingMode The filtering method used when sampling pixels from the input image while fitting it to desired size.
 * @param [in] orientationCorrection Reorient the image to respect any orientation metadata in its header.
 * @return handle to the loaded PixelBuffer object or an empty handle in case loading failed.
 */
DALI_IMPORT_API Devel::PixelBuffer LoadImageFromFile(
  const std::string& url,
  ImageDimensions size = ImageDimensions( 0, 0 ),
  FittingMode::Type fittingMode = FittingMode::DEFAULT,
  SamplingMode::Type samplingMode = SamplingMode::BOX_THEN_LINEAR,
  bool orientationCorrection = true );

/**
 * @brief Determine the size of an image that LoadImageFromFile will provide when
 * given the same image loading parameters.
 *
 * This is a synchronous request.
 * This function is used to determine the size of an image before it has loaded.
 * @param[in] filename name of the image.
 * @param[in] size The requested size for the image.
 * @param[in] fittingMode The method to use to map the source image to the desired
 * dimensions.
 * @param[in] samplingMode The image filter to use if the image needs to be
 * downsampled to the requested size.
 * @param[in] orientationCorrection Whether to use image metadata to rotate or
 * flip the image, e.g., from portrait to landscape.
 * @return dimensions that image will have if it is loaded with given parameters.
 */
DALI_IMPORT_API ImageDimensions GetClosestImageSize(
  const std::string& filename,
  ImageDimensions size = ImageDimensions(0, 0),
  FittingMode::Type fittingMode = FittingMode::DEFAULT,
  SamplingMode::Type samplingMode = SamplingMode::BOX_THEN_LINEAR ,
  bool orientationCorrection = true );

/**
 * @brief Load an image synchronously from a remote resource.
 *
 * @param [in] url The URL of the image file to load.
 * @param [in] size The width and height to fit the loaded image to, 0.0 means whole image
 * @param [in] fittingMode The method used to fit the shape of the image before loading to the shape defined by the size parameter.
 * @param [in] samplingMode The filtering method used when sampling pixels from the input image while fitting it to desired size.
 * @param [in] orientationCorrection Reorient the image to respect any orientation metadata in its header.
 *
 * @return handle to the loaded PixelBuffer object or an empty handle in case downloading or decoding failed.
 */
DALI_IMPORT_API Devel::PixelBuffer DownloadImageSynchronously(
  const std::string& url,
  ImageDimensions size = ImageDimensions( 0, 0 ),
  FittingMode::Type fittingMode = FittingMode::DEFAULT,
  SamplingMode::Type samplingMode = SamplingMode::BOX_THEN_LINEAR,
  bool orientationCorrection = true );

/**
 * @brief Set the maximum texture size. Then size can be kwown by GL_MAX_TEXTURE_SIZE.
 *
 * @param [in] size The maximum texture size to set
 */
void SetMaxTextureSize( unsigned int size );

/**
 * @brief get the maximum texture size.
 *
 * @return The maximum texture size
 */
DALI_IMPORT_API unsigned int GetMaxTextureSize();

} // Dali

#endif // DALI_IMAGE_LOADING_H
