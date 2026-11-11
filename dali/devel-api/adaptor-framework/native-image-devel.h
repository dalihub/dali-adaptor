#ifndef DALI_NATIVE_IMAGE_DEVEL_H
#define DALI_NATIVE_IMAGE_DEVEL_H
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
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/math/rect.h>
#include <functional>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/public-api/adaptor-framework/native-image.h>

namespace DALI_NAMESPACE
{
namespace DevelNativeImage
{
/**
 * @brief Converts the current pixel contents to either a JPEG or PNG format
 * and write that to the filesystem.
 *
 * @param[in] image The instance of NativeImage.
 * @param[in] filename Identify the filesystem location at which to write the encoded image.
 *                     The extension determines the encoding used.
 *                     The two valid encoding are (".jpeg"|".jpg") and ".png".
 * @param[in] quality The value to control image quality for jpeg file format in the range [1, 100]
 * @return    @c true if the pixels were written, and @c false otherwise
 */
DALI_ADAPTOR_API bool EncodeToFile(NativeImage& image, const std::string& filename, uint32_t quality);

/**
 * @brief Acquire buffer and information of an internal native image.
 *
 * AcquireBuffer() and ReleaseBuffer() are a pair.
 * ReleaseBuffer() MUST be called after AcquireBuffer().
 * @param[in] image The instance of NativeImage.
 * @param[out] width The width of image
 * @param[out] height The height of image
 * @param[out] stride The stride of image
 * @return     The buffer of an internal native image
 * @note This locks a mutex until ReleaseBuffer is called.
 */
DALI_ADAPTOR_API uint8_t* AcquireBuffer(NativeImage& image, uint32_t& width, uint32_t& height, uint32_t& stride);

/**
 * @brief Release information of an internal native image.
 *
 * AcquireBuffer() and ReleaseBuffer() are a pair.
 * ReleaseBuffer() MUST be called after AcquireBuffer().
 * @param[in] image The instance of NativeImage.
 * @param[in] updatedArea The updated area of the buffer.
 * @return @c true If the buffer is released successfully, and @c false otherwise
 * @note The empty updatedArea means that the entire area has been changed.
 * @note This unlocks the mutex locked by AcquireBuffer.
 */
DALI_ADAPTOR_API bool ReleaseBuffer(NativeImage& image, const Rect<uint32_t>& updatedArea);

/**
 * @brief Sets PixelBuffers to NativeImage
 *
 * @param[in] image The instance of NativeImage.
 * @param[in] pixbuf Pixel buffer to copy.
 * @param[in] pixelFormat Pixel format of the pixel buffer
 * @return @c true If the buffer is successfully set.
 * @note The width and height of the input pixel buffer should be same with those of NativeImage.
 * @note Only Pixel::Format::RGB888 and Pixel::Format::RGBA8888 are available as a input pixel format.
 */
DALI_ADAPTOR_API bool SetPixels(NativeImage& image, uint8_t* pixbuf, const Pixel::Format& pixelFormat);

/**
 * @brief Set the Resource Destruction Callback object
 *
 * This function does not opt into the Tizen GLES deferred GPU release path.
 * Use SetDeferredResourceDestruction() to request GPU completion tracking.
 *
 * @note This registration and SetDeferredResourceDestruction() are mutually
 * exclusive. The latest registration replaces the previous one. On Tizen GLES,
 * this function clears the deferred configuration and cancels its pending result
 * notifications. Resources already transferred to the deferred release queue
 * continue to be processed without notifying the replacement callback.
 * Select the destruction mode before graphics use.
 *
 * @param[in] image The instance of NativeImage.
 * @param[in] callback The Resource Destruction callback
 * @param[in] ownedCallback True if ownership of the callback is passed onto this class. False if ownership of the callback is exclude.
 */
DALI_ADAPTOR_API void SetResourceDestructionCallback(NativeImage& image, EventThreadCallback* callback, bool ownedCallback);

/**
 * @brief The result of a resource destruction request.
 */
enum class ResourceDestructionResult
{
  SUCCESS, ///< The backend has destroyed the graphics resource.
  ERROR    ///< GPU completion could not be confirmed; the resource is retained.
};

using ResourceDestructionCallback = std::function<void(ResourceDestructionResult)>;

/**
 * @brief Configures deferred graphics-resource destruction with a one-shot result callback.
 *
 * On Tizen GLES, a non-empty resultCallback enables GPU completion tracking.
 * A subsequent resource destruction request defers release when necessary.
 * This function does not itself request resource destruction.
 *
 * The callback runs on the event thread after resource destruction succeeds
 * or GPU completion checking fails. Pending fences do not notify the callback.
 * ERROR does not permit source reuse.
 * Backends supporting deferred destruction report wait failures as ERROR;
 * other backends report SUCCESS through their existing destruction path.
 *
 * Register or clear on the event thread, before graphics use or destruction of
 * objects captured by the callback. An executing callback may clear itself or
 * destroy its owner and remains valid until return.
 * Register again to receive a result for another resource destruction request.
 *
 * @note This configuration and SetResourceDestructionCallback() are mutually
 * exclusive. The latest registration replaces the previous one. An empty
 * resultCallback clears the configuration and cancels future and queued result
 * notifications. Resources already transferred to the deferred release queue
 * continue to be processed without notifying a replacement callback.
 *
 * @param[in] image The instance of NativeImage.
 * @param[in] resultCallback The result callback, or an empty function to clear the configuration.
 */
DALI_ADAPTOR_API void SetDeferredResourceDestruction(NativeImage& image, ResourceDestructionCallback resultCallback);

/**
 * @brief Whether the sole owner may return the source without waiting for graphics.
 *
 * Call on the event thread after delivering a frame. Returns false if another
 * owner can still submit the image, GPU release is pending, or the backend
 * cannot establish that release is safe. Does not release any resource.
 *
 * @param[in] image The instance of NativeImage.
 * @return True if the image has sole ownership and no live or pending graphics resources.
 */
DALI_ADAPTOR_API bool CanReleaseSource(NativeImage& image);

/**
 * @brief Enable a back buffer.
 * @param[in] image The instance of NativeImage.
 * @param[in] enable Whether a back buffer is enabled.
 * @note The front buffer will be copied to the back buffer before rendering if the front buffer is updated by AcquireBuffer and ReleaseBuffer.
 */
DALI_ADAPTOR_API void EnableBackBuffer(NativeImage& image, bool enable);

} // namespace DevelNativeImage

} //namespace DALI_NAMESPACE

#endif // DALI_NATIVE_IMAGE_DEVEL_H
