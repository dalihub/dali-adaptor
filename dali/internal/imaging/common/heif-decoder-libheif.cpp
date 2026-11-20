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

// CLASS HEADER
#include <dali/internal/imaging/common/heif-decoder.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/pixel-buffer-integ.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/images/pixel-buffer.h>
#include <dali/public-api/math/degree.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>

#include <libheif/heif.h>
#include <libheif/heif_properties.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-operations.h>
#include <dali/internal/system/common/system-error-print.h>

namespace DALI_NAMESPACE
{
namespace TizenPlatform
{
namespace HeifDecoder
{
namespace
{
using HeifContext = std::unique_ptr<heif_context, decltype(&heif_context_free)>;
using HeifHandle  = std::unique_ptr<heif_image_handle, decltype(&heif_image_handle_release)>;
using HeifImage   = std::unique_ptr<heif_image, decltype(&heif_image_release)>;

bool GetTransformations(const heif_context* context, const heif_image_handle* handle, Dali::Vector<heif_property_id>& properties)
{
  const heif_item_id itemId = heif_image_handle_get_item_id(handle);
  const int          count  = heif_item_get_transformation_properties(context, itemId, nullptr, 0);
  if(DALI_UNLIKELY(count < 0))
  {
    DALI_LOG_ERROR("[HEIF] Failed to query image transformations.\n");
    return false;
  }
  properties.ResizeUninitialized(static_cast<size_t>(count));
  if(count > 0 && heif_item_get_transformation_properties(context, itemId, properties.Begin(), count) != count)
  {
    DALI_LOG_ERROR("[HEIF] Failed to read image transformations.\n");
    return false;
  }
  return true;
}

bool UndoOrientation(const heif_context* context, const heif_image_handle* handle, const Dali::Vector<heif_property_id>& properties, Dali::PixelBuffer& bitmap)
{
  const heif_item_id itemId = heif_image_handle_get_item_id(handle);
  for(size_t index = properties.Count(); index > 0u; --index)
  {
    const heif_property_id property = properties[index - 1u];
    switch(heif_item_get_property_type(context, itemId, property))
    {
      case heif_item_property_type_transform_rotation:
      {
        const int angle = heif_item_get_property_transform_rotation_ccw(context, itemId, property);
        if(DALI_UNLIKELY(angle < 0 || (angle != 0 && !Dali::Internal::Adaptor::PixelBufferOperations::Rotate(bitmap, Degree(static_cast<float>(360 - angle))))))
        {
          DALI_LOG_ERROR("[HEIF] Failed to undo image rotation: %d.\n", angle);
          return false;
        }
        break;
      }
      case heif_item_property_type_transform_mirror:
      {
        const auto direction = heif_item_get_property_transform_mirror(context, itemId, property);
        if(DALI_UNLIKELY((direction == heif_transform_mirror_direction_vertical && !Dali::Internal::Adaptor::PixelBufferOperations::FlipVertical(bitmap)) ||
                         (direction == heif_transform_mirror_direction_horizontal && !Dali::Internal::Adaptor::PixelBufferOperations::FlipHorizontal(bitmap)) ||
                         direction == heif_transform_mirror_direction_invalid))
        {
          DALI_LOG_ERROR("[HEIF] Failed to undo image mirroring: %d.\n", direction);
          return false;
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  return true;
}

/**
 * @brief Read the encoded file and obtain its primary HEIF image handle.
 *
 * The buffer must outlive the context and handle because libheif reads it without copying.
 */
bool ReadHeifImageHandle(FILE* file, Dali::Vector<uint8_t>& buffer, heif_context* context, HeifHandle& handle)
{
  if(DALI_UNLIKELY(file == nullptr))
  {
    DALI_LOG_ERROR("[HEIF] Null file handle passed to HEIF image loader.\n");
    return false;
  }

  if(DALI_UNLIKELY(context == nullptr))
  {
    DALI_LOG_ERROR("[HEIF] Failed to allocate HEIF context.\n");
    return false;
  }

  // Determine the encoded file size before allocating the input buffer.
  if(DALI_UNLIKELY(fseek(file, 0, SEEK_END) != 0))
  {
    DALI_LOG_ERROR("[HEIF] Failed to seek to the end of the image file.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  const long fileSize = ftell(file);
  if(DALI_UNLIKELY(fileSize < 0))
  {
    DALI_LOG_ERROR("[HEIF] Failed to determine the image file size.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fileSize == 0))
  {
    DALI_LOG_ERROR("[HEIF] Image file is empty.\n");
    return false;
  }

  if(DALI_UNLIKELY(fseek(file, 0, SEEK_SET) != 0))
  {
    DALI_LOG_ERROR("[HEIF] Failed to seek to the beginning of the image file.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  buffer.ResizeUninitialized(static_cast<size_t>(fileSize));
  const size_t bytesRead = fread(buffer.Begin(), 1, buffer.Count(), file);
  if(DALI_UNLIKELY(bytesRead != buffer.Count()))
  {
    DALI_LOG_ERROR("[HEIF] Failed to read the image file: expected %zu bytes, read %zu.\n", static_cast<size_t>(buffer.Count()), bytesRead);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  // Parse the container without copying the encoded data, then select its primary image.
  const auto readError = heif_context_read_from_memory_without_copy(context, buffer.Begin(), buffer.Count(), nullptr);
  if(DALI_UNLIKELY(readError.code != heif_error_Ok))
  {
    DALI_LOG_ERROR("[HEIF] Failed to parse image data: %d/%d (%s).\n", readError.code, readError.subcode, readError.message ? readError.message : "unknown error");
    return false;
  }

  heif_image_handle* primary     = nullptr;
  const auto         handleError = heif_context_get_primary_image_handle(context, &primary);
  handle.reset(primary);
  if(DALI_UNLIKELY(handleError.code != heif_error_Ok || !handle))
  {
    DALI_LOG_ERROR("[HEIF] Failed to obtain the primary image handle: %d/%d (%s), handle: %p.\n", handleError.code, handleError.subcode, handleError.message ? handleError.message : "unknown error", static_cast<void*>(primary));
    return false;
  }

  return true;
}
} // namespace

bool ReadHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  Dali::Vector<uint8_t> buffer;
  HeifContext           context(heif_context_alloc(), heif_context_free);
  HeifHandle            handle(nullptr, heif_image_handle_release);
  if(DALI_UNLIKELY(!ReadHeifImageHandle(input.file, buffer, context.get(), handle)))
  {
    DALI_LOG_ERROR("Error loading heic header! Maybe this image is not heic/heif format.\n");
    return false;
  }

  // The display dimensions include clap. Undo only the orientation's dimension swaps.
  int imageWidth  = heif_image_handle_get_width(handle.get());
  int imageHeight = heif_image_handle_get_height(handle.get());
  if(!input.reorientationRequested)
  {
    Dali::Vector<heif_property_id> properties;
    if(!GetTransformations(context.get(), handle.get(), properties))
    {
      return false;
    }
    const heif_item_id itemId = heif_image_handle_get_item_id(handle.get());
    for(size_t index = properties.Count(); index > 0u; --index)
    {
      const heif_property_id property = properties[index - 1u];
      if(heif_item_get_property_type(context.get(), itemId, property) == heif_item_property_type_transform_rotation)
      {
        const int angle = heif_item_get_property_transform_rotation_ccw(context.get(), itemId, property);
        if(DALI_UNLIKELY(angle < 0))
        {
          DALI_LOG_ERROR("[HEIF] Invalid image rotation: %d.\n", angle);
          return false;
        }
        if(angle == 90 || angle == 270)
        {
          std::swap(imageWidth, imageHeight);
        }
      }
    }
  }
  if(DALI_UNLIKELY(imageWidth <= 0 || imageHeight <= 0))
  {
    DALI_LOG_ERROR("[HEIF] Invalid header dimensions: %d x %d.\n", imageWidth, imageHeight);
    return false;
  }

  width  = static_cast<unsigned int>(imageWidth);
  height = static_cast<unsigned int>(imageHeight);
  return true;
}

bool Decode(const Dali::ImageLoader::Input& input, Dali::PixelBuffer& bitmap)
{
  Dali::Vector<uint8_t> buffer;
  HeifContext           context(heif_context_alloc(), heif_context_free);
  HeifHandle            handle(nullptr, heif_image_handle_release);
  if(DALI_UNLIKELY(!ReadHeifImageHandle(input.file, buffer, context.get(), handle)))
  {
    DALI_LOG_ERROR("[HEIF] Failed to prepare the primary image handle for decoding.\n");
    return false;
  }

  // Decode to interleaved RGB or RGBA, preserving the alpha channel when present.
  const bool  alpha   = heif_image_handle_has_alpha_channel(handle.get());
  heif_image* decoded = nullptr;
  const auto  error   = heif_decode_image(handle.get(), &decoded, heif_colorspace_RGB,
                                       alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB, nullptr);

  HeifImage image(decoded, heif_image_release);
  if(DALI_UNLIKELY(error.code != heif_error_Ok || !image))
  {
    DALI_LOG_ERROR("[HEIF] Failed to decode the primary image: %d/%d (%s), image: %p.\n", error.code, error.subcode, error.message ? error.message : "unknown error", static_cast<void*>(decoded));
    return false;
  }

  // Validate dimensions before calculating the destination buffer size.
  const int      width    = heif_image_get_width(image.get(), heif_channel_interleaved);
  const int      height   = heif_image_get_height(image.get(), heif_channel_interleaved);
  const uint32_t channels = alpha ? 4u : 3u;
  if(DALI_UNLIKELY(width <= 0 || height <= 0 || static_cast<size_t>(width) > std::numeric_limits<uint32_t>::max() / channels / static_cast<size_t>(height)))
  {
    DALI_LOG_ERROR("[HEIF] Invalid decoded dimensions or pixel buffer size overflow: %d x %d, %u channels.\n", width, height, channels);
    return false;
  }

  // heif_image_get_plane_readonly already rotates and mirrors the pixels according to the image's orientation and mirror mode.
  // So we can copy them directly to the destination buffer.
  int            strideByte = 0;
  const uint8_t* pixels     = heif_image_get_plane_readonly(image.get(), heif_channel_interleaved, &strideByte);
  const uint32_t rowSize    = static_cast<uint32_t>(width) * channels;
  if(DALI_UNLIKELY(!pixels || strideByte <= 0 || static_cast<uint32_t>(strideByte) < rowSize))
  {
    DALI_LOG_ERROR("[HEIF] Invalid decoded pixel plane: data %p, strideByte %d, row size %u.\n", static_cast<const void*>(pixels), strideByte, rowSize);
    return false;
  }

  // Preserve libheif's row stride in bytes, including padding, in the destination pixel buffer.
  if(DALI_UNLIKELY(static_cast<size_t>(strideByte) > std::numeric_limits<uint32_t>::max() / static_cast<size_t>(height)))
  {
    DALI_LOG_ERROR("[HEIF] Pixel buffer size overflow: strideByte %d, height %d.\n", strideByte, height);
    return false;
  }

  const uint32_t bufferSize  = static_cast<uint32_t>(strideByte) * static_cast<uint32_t>(height);
  auto*          pixelBuffer = static_cast<uint8_t*>(std::malloc(bufferSize));
  if(DALI_UNLIKELY(!pixelBuffer))
  {
    DALI_LOG_ERROR("[HEIF] Failed to allocate the pixel buffer: %u bytes.\n", bufferSize);
    return false;
  }

  std::memcpy(pixelBuffer, pixels, bufferSize);

  // NewPixelBuffer takes ownership of the malloc-allocated pixelBuffer and releases it with free().
  bitmap = Dali::Integration::NewPixelBuffer(pixelBuffer, bufferSize, width, height, strideByte, alpha ? Pixel::RGBA8888 : Pixel::RGB888);
  if(DALI_UNLIKELY(!bitmap || bitmap.GetBuffer() != pixelBuffer))
  {
    DALI_LOG_ERROR("[HEIF] Failed to create the pixel buffer wrapper: %u bytes, %ux%u, stride %d.\n", bufferSize, width, height, strideByte);
    std::free(pixelBuffer);
    return false;
  }
  if(DALI_UNLIKELY(!input.reorientationRequested))
  {
    Dali::Vector<heif_property_id> properties;
    if(DALI_UNLIKELY(!GetTransformations(context.get(), handle.get(), properties) || !UndoOrientation(context.get(), handle.get(), properties, bitmap)))
    {
      return false;
    }
  }
  return true;
}

bool DecodePlanes(const Dali::ImageLoader::Input& input, std::vector<Dali::PixelBuffer>& pixelBuffers)
{
  Dali::Vector<uint8_t> buffer;
  HeifContext           context(heif_context_alloc(), heif_context_free);
  HeifHandle            handle(nullptr, heif_image_handle_release);
  if(DALI_UNLIKELY(!ReadHeifImageHandle(input.file, buffer, context.get(), handle)))
  {
    DALI_LOG_ERROR("[HEIF] Failed to prepare the primary image handle for YUV decoding.\n");
    return false;
  }
  if(heif_image_handle_has_alpha_channel(handle.get()))
  {
    DALI_LOG_ERROR("[HEIF] YUV plane decoding does not support alpha; falling back to RGB.\n");
    return false;
  }

  const auto sourceProfileType = heif_image_handle_get_color_profile_type(handle.get());
  if(sourceProfileType == heif_color_profile_type_rICC || sourceProfileType == heif_color_profile_type_prof)
  {
    DALI_LOG_ERROR("[HEIF] YUV plane decoding cannot apply an ICC color profile; profile type: %d.\n", sourceProfileType);
    return false;
  }
  heif_image* decoded = nullptr;
  const auto  error   = heif_decode_image(handle.get(), &decoded, heif_colorspace_YCbCr, heif_chroma_420, nullptr);
  HeifImage   image(decoded, heif_image_release);
  if(DALI_UNLIKELY(error.code != heif_error_Ok || !image))
  {
    DALI_LOG_ERROR("[HEIF] Failed to decode YUV420 image: %d/%d (%s), image: %p.\n", error.code, error.subcode, error.message ? error.message : "unknown error", static_cast<void*>(decoded));
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  // Use the decoded image profile: it can differ from the container profile.
  const auto profileType = heif_image_get_color_profile_type(image.get());
  if(profileType != heif_color_profile_type_nclx)
  {
    DALI_LOG_ERROR("[HEIF] YUV plane decoding requires an NCLX color profile; profile type: %d.\n", profileType);
    return false;
  }

  heif_color_profile_nclx*                                                          rawProfile   = nullptr;
  const auto                                                                        profileError = heif_image_get_nclx_color_profile(image.get(), &rawProfile);
  std::unique_ptr<heif_color_profile_nclx, decltype(&heif_nclx_color_profile_free)> profile(rawProfile, heif_nclx_color_profile_free);
  if(DALI_UNLIKELY(profileError.code != heif_error_Ok || !profile))
  {
    DALI_LOG_ERROR("[HEIF] Failed to read the decoded NCLX color profile: %d/%d (%s).\n", profileError.code, profileError.subcode, profileError.message ? profileError.message : "unknown error");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  // Check that the decoded YUV planes are compatible with the expected BT.601 or BT.709 color space.
  if(DALI_UNLIKELY(profile->color_primaries != heif_color_primaries_ITU_R_BT_709_5 ||
                   (profile->transfer_characteristics != heif_transfer_characteristic_ITU_R_BT_709_5 &&
                    profile->transfer_characteristics != heif_transfer_characteristic_IEC_61966_2_1) ||
                   (profile->matrix_coefficients != heif_matrix_coefficients_ITU_R_BT_601_6 &&
                    profile->matrix_coefficients != heif_matrix_coefficients_ITU_R_BT_709_5)))
  {
    DALI_LOG_ERROR("[HEIF] Unsupported YUV color profile: primaries %d, transfer %d, matrix %d.\n", profile->color_primaries, profile->transfer_characteristics, profile->matrix_coefficients);
    return false;
  }

  const heif_channel  channels[]  = {heif_channel_Y, heif_channel_Cb, heif_channel_Cr};
  const Pixel::Format formats[]   = {Pixel::L8, Pixel::CHROMINANCE_U, Pixel::CHROMINANCE_V};
  const int           imageWidth  = heif_image_get_width(image.get(), heif_channel_Y);
  const int           imageHeight = heif_image_get_height(image.get(), heif_channel_Y);
  if(DALI_UNLIKELY(imageWidth <= 0 || imageHeight <= 0))
  {
    DALI_LOG_ERROR("[HEIF] Invalid YUV image dimensions: %d x %d.\n", imageWidth, imageHeight);
    return false;
  }

  // heif_image_get_plane_readonly already rotates and mirrors the pixels according to the image's orientation and mirror mode.
  // So we can copy them directly to the destination buffer.
  std::vector<Dali::PixelBuffer> planes;
  planes.reserve(3u);
  for(unsigned int index = 0u; index < 3u; ++index)
  {
    const int   width        = heif_image_get_width(image.get(), channels[index]);
    const int   height       = heif_image_get_height(image.get(), channels[index]);
    const int   bitsPerPixel = heif_image_get_bits_per_pixel(image.get(), channels[index]);
    int         stride       = 0;
    const auto* pixels       = heif_image_get_plane_readonly(image.get(), channels[index], &stride);
    if(DALI_UNLIKELY(width <= 0 || height <= 0 || bitsPerPixel != 8 ||
                     (index != 0u && (width != (imageWidth + 1) / 2 || height != (imageHeight + 1) / 2)) ||
                     !pixels || stride < width))
    {
      DALI_LOG_ERROR("[HEIF] Invalid YUV plane %u: dimensions %d x %d, bits %d, stride %d, data %p.\n", index, width, height, bitsPerPixel, stride, static_cast<const void*>(pixels));
      return false;
    }

    auto plane = Dali::PixelBuffer::New(width, height, formats[index]);
    if(DALI_UNLIKELY(!plane))
    {
      DALI_LOG_ERROR("[HEIF] Failed to allocate YUV plane %u: %d x %d.\n", index, width, height);
      return false;
    }

    auto* destination = plane.GetBuffer();
    if(DALI_UNLIKELY(destination == nullptr))
    {
      DALI_LOG_ERROR("[HEIF] Allocated YUV plane %u has no data.\n", index);
      return false;
    }
    for(int row = 0; row < height; ++row)
    {
      std::memcpy(destination + static_cast<size_t>(row) * width, pixels + static_cast<size_t>(row) * stride, width);
    }
    planes.push_back(plane);
  }

  if(DALI_UNLIKELY(!profile->full_range_flag || profile->matrix_coefficients == heif_matrix_coefficients_ITU_R_BT_709_5))
  {
    auto*        yPlane     = planes[0].GetBuffer();
    auto*        uPlane     = planes[1].GetBuffer();
    auto*        vPlane     = planes[2].GetBuffer();
    const size_t ySize      = static_cast<size_t>(imageWidth) * imageHeight;
    const size_t chromaSize = static_cast<size_t>((imageWidth + 1) / 2) * ((imageHeight + 1) / 2);
    auto         ClampByte  = [](int value)
    {
      return static_cast<uint8_t>(std::clamp(value, 0, 255));
    };
    auto DivideRounded = [](int numerator, int denominator)
    {
      return (numerator + (numerator >= 0 ? (denominator + 1) / 2 : -(denominator + 1) / 2)) / denominator;
    };

    // The image visual expects full-range BT.601. Expand video range first.
    if(!profile->full_range_flag)
    {
      for(size_t index = 0u; index < ySize; ++index)
      {
        yPlane[index] = ClampByte(DivideRounded((static_cast<int>(yPlane[index]) - 16) * 255, 219));
      }
      for(size_t index = 0u; index < chromaSize; ++index)
      {
        uPlane[index] = ClampByte(128 + DivideRounded((static_cast<int>(uPlane[index]) - 128) * 255, 224));
        vPlane[index] = ClampByte(128 + DivideRounded((static_cast<int>(vPlane[index]) - 128) * 255, 224));
      }
    }

    // Convert BT.709 to BT.601 if necessary, using the ITU-R BT.709-5 to ITU-R BT.601-6 conversion matrix.
    if(profile->matrix_coefficients == heif_matrix_coefficients_ITU_R_BT_709_5)
    {
      const int chromaWidth = (imageWidth + 1) / 2;
      for(size_t index = 0u; index < chromaSize; ++index)
      {
        const int u      = static_cast<int>(uPlane[index]) - 128;
        const int v      = static_cast<int>(vPlane[index]) - 128;
        const int yShift = DivideRounded(26 * u + 50 * v, 256);
        uPlane[index]    = ClampByte(128 + DivideRounded(253 * u - 28 * v, 256));
        vPlane[index]    = ClampByte(128 + DivideRounded(-19 * u + 252 * v, 256));
        for(int offsetY = 0; offsetY < 2; ++offsetY)
        {
          const int y = static_cast<int>(index / chromaWidth) * 2 + offsetY;
          if(y >= imageHeight)
          {
            break;
          }
          for(int offsetX = 0; offsetX < 2; ++offsetX)
          {
            const int x = static_cast<int>(index % chromaWidth) * 2 + offsetX;
            if(x < imageWidth)
            {
              const size_t yIndex = static_cast<size_t>(y) * imageWidth + x;
              yPlane[yIndex]      = ClampByte(yPlane[yIndex] + yShift);
            }
          }
        }
      }
    }
  }

  if(DALI_UNLIKELY(!input.reorientationRequested))
  {
    Dali::Vector<heif_property_id> properties;
    if(DALI_UNLIKELY(!GetTransformations(context.get(), handle.get(), properties)))
    {
      return false;
    }
    for(auto& plane : planes)
    {
      if(DALI_UNLIKELY(!UndoOrientation(context.get(), handle.get(), properties, plane)))
      {
        return false;
      }
    }
  }

  pixelBuffers = std::move(planes);
  return true;
}
} // namespace HeifDecoder
} // namespace TizenPlatform
} // namespace DALI_NAMESPACE
