#ifndef DALI_IMAGE_LOADING_H
#define DALI_IMAGE_LOADING_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-string-view.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/image-options.h>
#include <dali/public-api/adaptor-framework/pixel-buffer.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Loads an image synchronously from a local file.
 *
 * The image is decoded at, or downscaled to, a size which fits the requested one, so the
 * returned pixel buffer does not necessarily have the requested dimensions. Query the pixel
 * buffer for the size it ended up with.
 *
 * @SINCE_2_5.35
 * @param[in] url The URL of the image file to load. Only local files are supported
 * @param[in] size The width and height to fit the loaded image to. ImageDimensions(0, 0) loads the image at its own size
 * @param[in] samplingMode The filtering method used when sampling pixels from the input image while fitting it to the requested size
 * @param[in] orientationCorrection Reorient the image to respect any orientation metadata in its header
 * @return A handle to the loaded PixelBuffer, or an empty handle if loading failed
 * @note This method is thread safe, i.e. it can be called from any thread.
 */
DALI_ADAPTOR_API PixelBuffer LoadImageFromFile(
  StringView         url,
  ImageDimensions    size                  = ImageDimensions(0, 0),
  SamplingMode::Type samplingMode          = SamplingMode::BOX_THEN_LINEAR,
  bool               orientationCorrection = true);

/**
 * @brief Gets the size an image is stored at, without loading its pixels.
 *
 * Only the header of the image file is read, which makes this a much cheaper way to find out
 * how large an image is than loading it. This is a synchronous request.
 *
 * @SINCE_2_5.35
 * @param[in] filename The name of the image file. Only local files are supported
 * @param[in] orientationCorrection Reorient the image to respect any orientation metadata in its header,
 * which swaps the width and the height for a quarter turn
 * @return The dimensions of the image, or ImageDimensions(0, 0) if the file could not be read or
 * holds an unsupported image format
 * @note This method is thread safe, i.e. it can be called from any thread.
 * @note The dimensions are the image's own, and are not capped to the largest texture the
 * graphics backend can hold.
 */
DALI_ADAPTOR_API ImageDimensions GetOriginalImageSize(
  StringView filename,
  bool       orientationCorrection = true);

/**
 * @}
 */
} // namespace Dali

#endif // DALI_IMAGE_LOADING_H
