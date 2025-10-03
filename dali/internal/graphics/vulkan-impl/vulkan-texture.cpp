/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/devel-api/scripting/enum-helper.h>
#include <dali/integration-api/pixel-data-integ.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer-request.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-utils.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

// EXTERNAL INCLUDES
#include <errno.h>
#include <fcntl.h>
#include <tbm_bo.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <tbm_type_common.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

namespace
{
PFN_vkBindImageMemory2KHR             gPfnBindImageMemory2KHR             = nullptr;
PFN_vkGetImageMemoryRequirements2KHR  gPfnGetImageMemoryRequirements2KHR  = nullptr;
PFN_vkGetMemoryFdPropertiesKHR        gPfnGetMemoryFdPropertiesKHR        = nullptr;
PFN_vkCreateSamplerYcbcrConversionKHR gPfnCreateSamplerYcbcrConversionKHR = nullptr;

// clang-format off

// TBM format to Vulkan format mapping
const std::pair<tbm_format, vk::Format> FORMAT_MAPPING[] = {
  {TBM_FORMAT_RGB888, vk::Format::eB8G8R8A8Unorm},
  {TBM_FORMAT_XRGB8888, vk::Format::eB8G8R8A8Unorm},
  {TBM_FORMAT_RGBX8888, vk::Format::eB8G8R8A8Unorm},
  {TBM_FORMAT_ARGB8888, vk::Format::eB8G8R8A8Unorm},
  {TBM_FORMAT_RGBA8888, vk::Format::eB8G8R8A8Unorm},
  {TBM_FORMAT_BGR888, vk::Format::eR8G8B8A8Unorm},
  {TBM_FORMAT_XBGR8888, vk::Format::eR8G8B8A8Unorm},
  {TBM_FORMAT_BGRX8888, vk::Format::eR8G8B8A8Unorm},
  {TBM_FORMAT_ABGR8888, vk::Format::eR8G8B8A8Unorm},
  {TBM_FORMAT_BGRA8888, vk::Format::eR8G8B8A8Unorm},
  {TBM_FORMAT_NV12, vk::Format::eG8B8R82Plane420Unorm},
  {TBM_FORMAT_NV21, vk::Format::eG8B8R82Plane420Unorm}
};

// YCbCr formats that need conversion
const tbm_format YUV_FORMATS[] = {
  TBM_FORMAT_NV12,
  TBM_FORMAT_NV21
};

// Plane aspect flags for disjoint multi-plane binding
const vk::ImageAspectFlagBits PLANE_ASPECT_FLAGS[] = {
  vk::ImageAspectFlagBits::eMemoryPlane0EXT,
  vk::ImageAspectFlagBits::eMemoryPlane1EXT,
  vk::ImageAspectFlagBits::eMemoryPlane2EXT,
  vk::ImageAspectFlagBits::eMemoryPlane3EXT
};

// clang-format on

const int NUM_FORMATS_BLENDING_REQUIRED = 18;

vk::Format GetVulkanFormat(tbm_format tbmFormat)
{
  for(const auto& mapping : FORMAT_MAPPING)
  {
    if(mapping.first == tbmFormat)
    {
      return mapping.second;
    }
  }

  DALI_LOG_ERROR("Unsupported TBM format: %d\n", tbmFormat);
  return vk::Format::eUndefined;
}

bool RequiresYcbcrConversion(tbm_format tbmFormat)
{
  for(const auto& format : YUV_FORMATS)
  {
    if(format == tbmFormat)
    {
      return true;
    }
  }
  return false;
}

} // namespace

namespace Dali::Graphics::Vulkan
{
/**
 * Remaps components
 */
inline vk::ComponentMapping GetVkComponentMappingInlined(Dali::Graphics::Format format)
{
  switch(format)
  {
    case Dali::Graphics::Format::L8:
    {
      return vk::ComponentMapping{
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eOne};
    }
    case Dali::Graphics::Format::L8A8:
    {
      return vk::ComponentMapping{
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eG,
      };
    }
    default:
    {
      return vk::ComponentMapping{
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB,
        vk::ComponentSwizzle::eA};
    }
  }
}

/**
 * Converts API pixel format to Vulkan
 */
constexpr vk::Format ConvertApiToVkConst(Dali::Graphics::Format format)
{
  switch(format)
  {
    case Dali::Graphics::Format::L8:
    {
      return vk::Format::eR8Unorm;
    }
    case Dali::Graphics::Format::L8A8:
    {
      return vk::Format::eR8G8Unorm;
    }
    case Dali::Graphics::Format::UNDEFINED:
    {
      return vk::Format::eUndefined;
    }
    case Dali::Graphics::Format::R4G4_UNORM_PACK8:
    {
      return vk::Format::eR4G4UnormPack8;
    }
    case Dali::Graphics::Format::R4G4B4A4_UNORM_PACK16:
    {
      return vk::Format::eR4G4B4A4UnormPack16;
    }
    case Dali::Graphics::Format::B4G4R4A4_UNORM_PACK16:
    {
      return vk::Format::eB4G4R4A4UnormPack16;
    }
    case Dali::Graphics::Format::R5G6B5_UNORM_PACK16:
    {
      return vk::Format::eR5G6B5UnormPack16;
    }
    case Dali::Graphics::Format::B5G6R5_UNORM_PACK16:
    {
      return vk::Format::eB5G6R5UnormPack16;
    }
    case Dali::Graphics::Format::R5G5B5A1_UNORM_PACK16:
    {
      return vk::Format::eR5G5B5A1UnormPack16;
    }
    case Dali::Graphics::Format::B5G5R5A1_UNORM_PACK16:
    {
      return vk::Format::eB5G5R5A1UnormPack16;
    }
    case Dali::Graphics::Format::A1R5G5B5_UNORM_PACK16:
    {
      return vk::Format::eA1R5G5B5UnormPack16;
    }
    case Dali::Graphics::Format::R8_UNORM:
    {
      return vk::Format::eR8Unorm;
    }
    case Dali::Graphics::Format::R8_SNORM:
    {
      return vk::Format::eR8Snorm;
    }
    case Dali::Graphics::Format::R8_USCALED:
    {
      return vk::Format::eR8Uscaled;
    }
    case Dali::Graphics::Format::R8_SSCALED:
    {
      return vk::Format::eR8Sscaled;
    }
    case Dali::Graphics::Format::R8_UINT:
    {
      return vk::Format::eR8Uint;
    }
    case Dali::Graphics::Format::R8_SINT:
    {
      return vk::Format::eR8Sint;
    }
    case Dali::Graphics::Format::R8_SRGB:
    {
      return vk::Format::eR8Srgb;
    }
    case Dali::Graphics::Format::R8G8_UNORM:
    {
      return vk::Format::eR8G8Unorm;
    }
    case Dali::Graphics::Format::R8G8_SNORM:
    {
      return vk::Format::eR8G8Snorm;
    }
    case Dali::Graphics::Format::R8G8_USCALED:
    {
      return vk::Format::eR8G8Uscaled;
    }
    case Dali::Graphics::Format::R8G8_SSCALED:
    {
      return vk::Format::eR8G8Sscaled;
    }
    case Dali::Graphics::Format::R8G8_UINT:
    {
      return vk::Format::eR8G8Uint;
    }
    case Dali::Graphics::Format::R8G8_SINT:
    {
      return vk::Format::eR8G8Sint;
    }
    case Dali::Graphics::Format::R8G8_SRGB:
    {
      return vk::Format::eR8G8Srgb;
    }
    case Dali::Graphics::Format::R8G8B8_UNORM:
    {
      return vk::Format::eR8G8B8Unorm;
    }
    case Dali::Graphics::Format::R8G8B8_SNORM:
    {
      return vk::Format::eR8G8B8Snorm;
    }
    case Dali::Graphics::Format::R8G8B8_USCALED:
    {
      return vk::Format::eR8G8B8Uscaled;
    }
    case Dali::Graphics::Format::R8G8B8_SSCALED:
    {
      return vk::Format::eR8G8B8Sscaled;
    }
    case Dali::Graphics::Format::R8G8B8_UINT:
    {
      return vk::Format::eR8G8B8Uint;
    }
    case Dali::Graphics::Format::R8G8B8_SINT:
    {
      return vk::Format::eR8G8B8Sint;
    }
    case Dali::Graphics::Format::R8G8B8_SRGB:
    {
      return vk::Format::eR8G8B8Srgb;
    }
    case Dali::Graphics::Format::B8G8R8_UNORM:
    {
      return vk::Format::eB8G8R8Unorm;
    }
    case Dali::Graphics::Format::B8G8R8_SNORM:
    {
      return vk::Format::eB8G8R8Snorm;
    }
    case Dali::Graphics::Format::B8G8R8_USCALED:
    {
      return vk::Format::eB8G8R8Uscaled;
    }
    case Dali::Graphics::Format::B8G8R8_SSCALED:
    {
      return vk::Format::eB8G8R8Sscaled;
    }
    case Dali::Graphics::Format::B8G8R8_UINT:
    {
      return vk::Format::eB8G8R8Uint;
    }
    case Dali::Graphics::Format::B8G8R8_SINT:
    {
      return vk::Format::eB8G8R8Sint;
    }
    case Dali::Graphics::Format::B8G8R8_SRGB:
    {
      return vk::Format::eB8G8R8Srgb;
    }
    case Dali::Graphics::Format::R8G8B8A8_UNORM:
    {
      return vk::Format::eR8G8B8A8Unorm;
    }
    case Dali::Graphics::Format::R8G8B8A8_SNORM:
    {
      return vk::Format::eR8G8B8A8Snorm;
    }
    case Dali::Graphics::Format::R8G8B8A8_USCALED:
    {
      return vk::Format::eR8G8B8A8Uscaled;
    }
    case Dali::Graphics::Format::R8G8B8A8_SSCALED:
    {
      return vk::Format::eR8G8B8A8Sscaled;
    }
    case Dali::Graphics::Format::R8G8B8A8_UINT:
    {
      return vk::Format::eR8G8B8A8Uint;
    }
    case Dali::Graphics::Format::R8G8B8A8_SINT:
    {
      return vk::Format::eR8G8B8A8Sint;
    }
    case Dali::Graphics::Format::R8G8B8A8_SRGB:
    {
      return vk::Format::eR8G8B8A8Srgb;
    }
    case Dali::Graphics::Format::B8G8R8A8_UNORM:
    {
      return vk::Format::eB8G8R8A8Unorm;
    }
    case Dali::Graphics::Format::B8G8R8A8_SNORM:
    {
      return vk::Format::eB8G8R8A8Snorm;
    }
    case Dali::Graphics::Format::B8G8R8A8_USCALED:
    {
      return vk::Format::eB8G8R8A8Uscaled;
    }
    case Dali::Graphics::Format::B8G8R8A8_SSCALED:
    {
      return vk::Format::eB8G8R8A8Sscaled;
    }
    case Dali::Graphics::Format::B8G8R8A8_UINT:
    {
      return vk::Format::eB8G8R8A8Uint;
    }
    case Dali::Graphics::Format::B8G8R8A8_SINT:
    {
      return vk::Format::eB8G8R8A8Sint;
    }
    case Dali::Graphics::Format::B8G8R8A8_SRGB:
    {
      return vk::Format::eB8G8R8A8Srgb;
    }
    case Dali::Graphics::Format::A8B8G8R8_UNORM_PACK32:
    {
      return vk::Format::eA8B8G8R8UnormPack32;
    }
    case Dali::Graphics::Format::A8B8G8R8_SNORM_PACK32:
    {
      return vk::Format::eA8B8G8R8SnormPack32;
    }
    case Dali::Graphics::Format::A8B8G8R8_USCALED_PACK32:
    {
      return vk::Format::eA8B8G8R8UscaledPack32;
    }
    case Dali::Graphics::Format::A8B8G8R8_SSCALED_PACK32:
    {
      return vk::Format::eA8B8G8R8SscaledPack32;
    }
    case Dali::Graphics::Format::A8B8G8R8_UINT_PACK32:
    {
      return vk::Format::eA8B8G8R8UintPack32;
    }
    case Dali::Graphics::Format::A8B8G8R8_SINT_PACK32:
    {
      return vk::Format::eA8B8G8R8SintPack32;
    }
    case Dali::Graphics::Format::A8B8G8R8_SRGB_PACK32:
    {
      return vk::Format::eA8B8G8R8SrgbPack32;
    }
    case Dali::Graphics::Format::A2R10G10B10_UNORM_PACK32:
    {
      return vk::Format::eA2R10G10B10UnormPack32;
    }
    case Dali::Graphics::Format::A2R10G10B10_SNORM_PACK32:
    {
      return vk::Format::eA2R10G10B10SnormPack32;
    }
    case Dali::Graphics::Format::A2R10G10B10_USCALED_PACK32:
    {
      return vk::Format::eA2R10G10B10UscaledPack32;
    }
    case Dali::Graphics::Format::A2R10G10B10_SSCALED_PACK32:
    {
      return vk::Format::eA2R10G10B10SscaledPack32;
    }
    case Dali::Graphics::Format::A2R10G10B10_UINT_PACK32:
    {
      return vk::Format::eA2R10G10B10UintPack32;
    }
    case Dali::Graphics::Format::A2R10G10B10_SINT_PACK32:
    {
      return vk::Format::eA2R10G10B10SintPack32;
    }
    case Dali::Graphics::Format::A2B10G10R10_UNORM_PACK32:
    {
      return vk::Format::eA2B10G10R10UnormPack32;
    }
    case Dali::Graphics::Format::A2B10G10R10_SNORM_PACK32:
    {
      return vk::Format::eA2B10G10R10SnormPack32;
    }
    case Dali::Graphics::Format::A2B10G10R10_USCALED_PACK32:
    {
      return vk::Format::eA2B10G10R10UscaledPack32;
    }
    case Dali::Graphics::Format::A2B10G10R10_SSCALED_PACK32:
    {
      return vk::Format::eA2B10G10R10SscaledPack32;
    }
    case Dali::Graphics::Format::A2B10G10R10_UINT_PACK32:
    {
      return vk::Format::eA2B10G10R10UintPack32;
    }
    case Dali::Graphics::Format::A2B10G10R10_SINT_PACK32:
    {
      return vk::Format::eA2B10G10R10SintPack32;
    }
    case Dali::Graphics::Format::R16_UNORM:
    {
      return vk::Format::eR16Unorm;
    }
    case Dali::Graphics::Format::R16_SNORM:
    {
      return vk::Format::eR16Snorm;
    }
    case Dali::Graphics::Format::R16_USCALED:
    {
      return vk::Format::eR16Uscaled;
    }
    case Dali::Graphics::Format::R16_SSCALED:
    {
      return vk::Format::eR16Sscaled;
    }
    case Dali::Graphics::Format::R16_UINT:
    {
      return vk::Format::eR16Uint;
    }
    case Dali::Graphics::Format::R16_SINT:
    {
      return vk::Format::eR16Sint;
    }
    case Dali::Graphics::Format::R16_SFLOAT:
    {
      return vk::Format::eR16Sfloat;
    }
    case Dali::Graphics::Format::R16G16_UNORM:
    {
      return vk::Format::eR16G16Unorm;
    }
    case Dali::Graphics::Format::R16G16_SNORM:
    {
      return vk::Format::eR16G16Snorm;
    }
    case Dali::Graphics::Format::R16G16_USCALED:
    {
      return vk::Format::eR16G16Uscaled;
    }
    case Dali::Graphics::Format::R16G16_SSCALED:
    {
      return vk::Format::eR16G16Sscaled;
    }
    case Dali::Graphics::Format::R16G16_UINT:
    {
      return vk::Format::eR16G16Uint;
    }
    case Dali::Graphics::Format::R16G16_SINT:
    {
      return vk::Format::eR16G16Sint;
    }
    case Dali::Graphics::Format::R16G16_SFLOAT:
    {
      return vk::Format::eR16G16Sfloat;
    }
    case Dali::Graphics::Format::R16G16B16_UNORM:
    {
      return vk::Format::eR16G16B16Unorm;
    }
    case Dali::Graphics::Format::R16G16B16_SNORM:
    {
      return vk::Format::eR16G16B16Snorm;
    }
    case Dali::Graphics::Format::R16G16B16_USCALED:
    {
      return vk::Format::eR16G16B16Uscaled;
    }
    case Dali::Graphics::Format::R16G16B16_SSCALED:
    {
      return vk::Format::eR16G16B16Sscaled;
    }
    case Dali::Graphics::Format::R16G16B16_UINT:
    {
      return vk::Format::eR16G16B16Uint;
    }
    case Dali::Graphics::Format::R16G16B16_SINT:
    {
      return vk::Format::eR16G16B16Sint;
    }
    case Dali::Graphics::Format::R16G16B16_SFLOAT:
    {
      return vk::Format::eR16G16B16Sfloat;
    }
    case Dali::Graphics::Format::R16G16B16A16_UNORM:
    {
      return vk::Format::eR16G16B16A16Unorm;
    }
    case Dali::Graphics::Format::R16G16B16A16_SNORM:
    {
      return vk::Format::eR16G16B16A16Snorm;
    }
    case Dali::Graphics::Format::R16G16B16A16_USCALED:
    {
      return vk::Format::eR16G16B16A16Uscaled;
    }
    case Dali::Graphics::Format::R16G16B16A16_SSCALED:
    {
      return vk::Format::eR16G16B16A16Sscaled;
    }
    case Dali::Graphics::Format::R16G16B16A16_UINT:
    {
      return vk::Format::eR16G16B16A16Uint;
    }
    case Dali::Graphics::Format::R16G16B16A16_SINT:
    {
      return vk::Format::eR16G16B16A16Sint;
    }
    case Dali::Graphics::Format::R16G16B16A16_SFLOAT:
    {
      return vk::Format::eR16G16B16A16Sfloat;
    }
    case Dali::Graphics::Format::R32_UINT:
    {
      return vk::Format::eR32Uint;
    }
    case Dali::Graphics::Format::R32_SINT:
    {
      return vk::Format::eR32Sint;
    }
    case Dali::Graphics::Format::R32_SFLOAT:
    {
      return vk::Format::eR32Sfloat;
    }
    case Dali::Graphics::Format::R32G32_UINT:
    {
      return vk::Format::eR32G32Uint;
    }
    case Dali::Graphics::Format::R32G32_SINT:
    {
      return vk::Format::eR32G32Sint;
    }
    case Dali::Graphics::Format::R32G32_SFLOAT:
    {
      return vk::Format::eR32G32Sfloat;
    }
    case Dali::Graphics::Format::R32G32B32_UINT:
    {
      return vk::Format::eR32G32B32Uint;
    }
    case Dali::Graphics::Format::R32G32B32_SINT:
    {
      return vk::Format::eR32G32B32Sint;
    }
    case Dali::Graphics::Format::R32G32B32_SFLOAT:
    {
      return vk::Format::eR32G32B32Sfloat;
    }
    case Dali::Graphics::Format::R32G32B32A32_UINT:
    {
      return vk::Format::eR32G32B32A32Uint;
    }
    case Dali::Graphics::Format::R32G32B32A32_SINT:
    {
      return vk::Format::eR32G32B32A32Sint;
    }
    case Dali::Graphics::Format::R32G32B32A32_SFLOAT:
    {
      return vk::Format::eR32G32B32A32Sfloat;
    }
    case Dali::Graphics::Format::R64_UINT:
    {
      return vk::Format::eR64Uint;
    }
    case Dali::Graphics::Format::R64_SINT:
    {
      return vk::Format::eR64Sint;
    }
    case Dali::Graphics::Format::R64_SFLOAT:
    {
      return vk::Format::eR64Sfloat;
    }
    case Dali::Graphics::Format::R64G64_UINT:
    {
      return vk::Format::eR64G64Uint;
    }
    case Dali::Graphics::Format::R64G64_SINT:
    {
      return vk::Format::eR64G64Sint;
    }
    case Dali::Graphics::Format::R64G64_SFLOAT:
    {
      return vk::Format::eR64G64Sfloat;
    }
    case Dali::Graphics::Format::R64G64B64_UINT:
    {
      return vk::Format::eR64G64B64Uint;
    }
    case Dali::Graphics::Format::R64G64B64_SINT:
    {
      return vk::Format::eR64G64B64Sint;
    }
    case Dali::Graphics::Format::R64G64B64_SFLOAT:
    {
      return vk::Format::eR64G64B64Sfloat;
    }
    case Dali::Graphics::Format::R64G64B64A64_UINT:
    {
      return vk::Format::eR64G64B64A64Uint;
    }
    case Dali::Graphics::Format::R64G64B64A64_SINT:
    {
      return vk::Format::eR64G64B64A64Sint;
    }
    case Dali::Graphics::Format::R64G64B64A64_SFLOAT:
    {
      return vk::Format::eR64G64B64A64Sfloat;
    }
    case Dali::Graphics::Format::B10G11R11_UFLOAT_PACK32:
    {
      return vk::Format::eB10G11R11UfloatPack32;
    }
    case Dali::Graphics::Format::R11G11B10_UFLOAT_PACK32:
    {
      return vk::Format::eB10G11R11UfloatPack32;
    }
    case Dali::Graphics::Format::E5B9G9R9_UFLOAT_PACK32:
    {
      return vk::Format::eE5B9G9R9UfloatPack32;
    }
    case Dali::Graphics::Format::D16_UNORM:
    {
      return vk::Format::eD16Unorm;
    }
    case Dali::Graphics::Format::X8_D24_UNORM_PACK32:
    {
      return vk::Format::eX8D24UnormPack32;
    }
    case Dali::Graphics::Format::D32_SFLOAT:
    {
      return vk::Format::eD32Sfloat;
    }
    case Dali::Graphics::Format::S8_UINT:
    {
      return vk::Format::eS8Uint;
    }
    case Dali::Graphics::Format::D16_UNORM_S8_UINT:
    {
      return vk::Format::eD16UnormS8Uint;
    }
    case Dali::Graphics::Format::D24_UNORM_S8_UINT:
    {
      return vk::Format::eD24UnormS8Uint;
    }
    case Dali::Graphics::Format::D32_SFLOAT_S8_UINT:
    {
      return vk::Format::eD32SfloatS8Uint;
    }
    case Dali::Graphics::Format::BC1_RGB_UNORM_BLOCK:
    {
      return vk::Format::eBc1RgbUnormBlock;
    }
    case Dali::Graphics::Format::BC1_RGB_SRGB_BLOCK:
    {
      return vk::Format::eBc1RgbSrgbBlock;
    }
    case Dali::Graphics::Format::BC1_RGBA_UNORM_BLOCK:
    {
      return vk::Format::eBc1RgbaUnormBlock;
    }
    case Dali::Graphics::Format::BC1_RGBA_SRGB_BLOCK:
    {
      return vk::Format::eBc1RgbaSrgbBlock;
    }
    case Dali::Graphics::Format::BC2_UNORM_BLOCK:
    {
      return vk::Format::eBc2UnormBlock;
    }
    case Dali::Graphics::Format::BC2_SRGB_BLOCK:
    {
      return vk::Format::eBc2SrgbBlock;
    }
    case Dali::Graphics::Format::BC3_UNORM_BLOCK:
    {
      return vk::Format::eBc3UnormBlock;
    }
    case Dali::Graphics::Format::BC3_SRGB_BLOCK:
    {
      return vk::Format::eBc3SrgbBlock;
    }
    case Dali::Graphics::Format::BC4_UNORM_BLOCK:
    {
      return vk::Format::eBc4UnormBlock;
    }
    case Dali::Graphics::Format::BC4_SNORM_BLOCK:
    {
      return vk::Format::eBc4SnormBlock;
    }
    case Dali::Graphics::Format::BC5_UNORM_BLOCK:
    {
      return vk::Format::eBc5UnormBlock;
    }
    case Dali::Graphics::Format::BC5_SNORM_BLOCK:
    {
      return vk::Format::eBc5SnormBlock;
    }
    case Dali::Graphics::Format::BC6H_UFLOAT_BLOCK:
    {
      return vk::Format::eBc6HUfloatBlock;
    }
    case Dali::Graphics::Format::BC6H_SFLOAT_BLOCK:
    {
      return vk::Format::eBc6HSfloatBlock;
    }
    case Dali::Graphics::Format::BC7_UNORM_BLOCK:
    {
      return vk::Format::eBc7UnormBlock;
    }
    case Dali::Graphics::Format::BC7_SRGB_BLOCK:
    {
      return vk::Format::eBc7SrgbBlock;
    }
    case Dali::Graphics::Format::ETC2_R8G8B8_UNORM_BLOCK:
    {
      return vk::Format::eEtc2R8G8B8UnormBlock;
    }
    case Dali::Graphics::Format::ETC2_R8G8B8_SRGB_BLOCK:
    {
      return vk::Format::eEtc2R8G8B8SrgbBlock;
    }
    case Dali::Graphics::Format::ETC2_R8G8B8A1_UNORM_BLOCK:
    {
      return vk::Format::eEtc2R8G8B8A1UnormBlock;
    }
    case Dali::Graphics::Format::ETC2_R8G8B8A1_SRGB_BLOCK:
    {
      return vk::Format::eEtc2R8G8B8A1SrgbBlock;
    }
    case Dali::Graphics::Format::ETC2_R8G8B8A8_UNORM_BLOCK:
    {
      return vk::Format::eEtc2R8G8B8A8UnormBlock;
    }
    case Dali::Graphics::Format::ETC2_R8G8B8A8_SRGB_BLOCK:
    {
      return vk::Format::eEtc2R8G8B8A8SrgbBlock;
    }
    case Dali::Graphics::Format::EAC_R11_UNORM_BLOCK:
    {
      return vk::Format::eEacR11UnormBlock;
    }
    case Dali::Graphics::Format::EAC_R11_SNORM_BLOCK:
    {
      return vk::Format::eEacR11SnormBlock;
    }
    case Dali::Graphics::Format::EAC_R11G11_UNORM_BLOCK:
    {
      return vk::Format::eEacR11G11UnormBlock;
    }
    case Dali::Graphics::Format::EAC_R11G11_SNORM_BLOCK:
    {
      return vk::Format::eEacR11G11SnormBlock;
    }
    case Dali::Graphics::Format::ASTC_4x4_UNORM_BLOCK:
    {
      return vk::Format::eAstc4x4UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_4x4_SRGB_BLOCK:
    {
      return vk::Format::eAstc4x4SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_5x4_UNORM_BLOCK:
    {
      return vk::Format::eAstc5x4UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_5x4_SRGB_BLOCK:
    {
      return vk::Format::eAstc5x4SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_5x5_UNORM_BLOCK:
    {
      return vk::Format::eAstc5x5UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_5x5_SRGB_BLOCK:
    {
      return vk::Format::eAstc5x5SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_6x5_UNORM_BLOCK:
    {
      return vk::Format::eAstc6x5UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_6x5_SRGB_BLOCK:
    {
      return vk::Format::eAstc6x5SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_6x6_UNORM_BLOCK:
    {
      return vk::Format::eAstc6x6UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_6x6_SRGB_BLOCK:
    {
      return vk::Format::eAstc6x6SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_8x5_UNORM_BLOCK:
    {
      return vk::Format::eAstc8x5UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_8x5_SRGB_BLOCK:
    {
      return vk::Format::eAstc8x5SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_8x6_UNORM_BLOCK:
    {
      return vk::Format::eAstc8x6UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_8x6_SRGB_BLOCK:
    {
      return vk::Format::eAstc8x6SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_8x8_UNORM_BLOCK:
    {
      return vk::Format::eAstc8x8UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_8x8_SRGB_BLOCK:
    {
      return vk::Format::eAstc8x8SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_10x5_UNORM_BLOCK:
    {
      return vk::Format::eAstc10x5UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_10x5_SRGB_BLOCK:
    {
      return vk::Format::eAstc10x5SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_10x6_UNORM_BLOCK:
    {
      return vk::Format::eAstc10x6UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_10x6_SRGB_BLOCK:
    {
      return vk::Format::eAstc10x6SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_10x8_UNORM_BLOCK:
    {
      return vk::Format::eAstc10x8UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_10x8_SRGB_BLOCK:
    {
      return vk::Format::eAstc10x8SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_10x10_UNORM_BLOCK:
    {
      return vk::Format::eAstc10x10UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_10x10_SRGB_BLOCK:
    {
      return vk::Format::eAstc10x10SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_12x10_UNORM_BLOCK:
    {
      return vk::Format::eAstc12x10UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_12x10_SRGB_BLOCK:
    {
      return vk::Format::eAstc12x10SrgbBlock;
    }
    case Dali::Graphics::Format::ASTC_12x12_UNORM_BLOCK:
    {
      return vk::Format::eAstc12x12UnormBlock;
    }
    case Dali::Graphics::Format::ASTC_12x12_SRGB_BLOCK:
    {
      return vk::Format::eAstc12x12SrgbBlock;
    }
    case Dali::Graphics::Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
    {
      return vk::Format::ePvrtc12BppUnormBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
    {
      return vk::Format::ePvrtc14BppUnormBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
    {
      return vk::Format::ePvrtc22BppUnormBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
    {
      return vk::Format::ePvrtc24BppUnormBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
    {
      return vk::Format::ePvrtc12BppSrgbBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
    {
      return vk::Format::ePvrtc14BppSrgbBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
    {
      return vk::Format::ePvrtc22BppSrgbBlockIMG;
    }
    case Dali::Graphics::Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
    {
      return vk::Format::ePvrtc24BppSrgbBlockIMG;
    }
  }
  return {};
}

constexpr bool IsCompressed(Dali::Graphics::Format format)
{
  switch(format)
  {
    case Graphics::Format::EAC_R11_UNORM_BLOCK:
    case Graphics::Format::EAC_R11_SNORM_BLOCK:
    case Graphics::Format::EAC_R11G11_UNORM_BLOCK:
    case Graphics::Format::EAC_R11G11_SNORM_BLOCK:
    case Graphics::Format::ETC2_R8G8B8_UNORM_BLOCK:
    case Graphics::Format::ETC2_R8G8B8_SRGB_BLOCK:
    case Graphics::Format::ETC2_R8G8B8A1_UNORM_BLOCK:
    case Graphics::Format::ETC2_R8G8B8A1_SRGB_BLOCK:
    case Graphics::Format::ETC2_R8G8B8A8_UNORM_BLOCK:
    case Graphics::Format::ETC2_R8G8B8A8_SRGB_BLOCK:
    case Graphics::Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case Graphics::Format::ASTC_4x4_UNORM_BLOCK:
    case Graphics::Format::ASTC_5x4_UNORM_BLOCK:
    case Graphics::Format::ASTC_5x5_UNORM_BLOCK:
    case Graphics::Format::ASTC_6x5_UNORM_BLOCK:
    case Graphics::Format::ASTC_6x6_UNORM_BLOCK:
    case Graphics::Format::ASTC_8x5_UNORM_BLOCK:
    case Graphics::Format::ASTC_8x6_UNORM_BLOCK:
    case Graphics::Format::ASTC_8x8_UNORM_BLOCK:
    case Graphics::Format::ASTC_10x5_UNORM_BLOCK:
    case Graphics::Format::ASTC_10x6_UNORM_BLOCK:
    case Graphics::Format::ASTC_10x8_UNORM_BLOCK:
    case Graphics::Format::ASTC_10x10_UNORM_BLOCK:
    case Graphics::Format::ASTC_12x10_UNORM_BLOCK:
    case Graphics::Format::ASTC_12x12_UNORM_BLOCK:
    case Graphics::Format::ASTC_4x4_SRGB_BLOCK:
    case Graphics::Format::ASTC_5x4_SRGB_BLOCK:
    case Graphics::Format::ASTC_5x5_SRGB_BLOCK:
    case Graphics::Format::ASTC_6x5_SRGB_BLOCK:
    case Graphics::Format::ASTC_6x6_SRGB_BLOCK:
    case Graphics::Format::ASTC_8x5_SRGB_BLOCK:
    case Graphics::Format::ASTC_8x8_SRGB_BLOCK:
    case Graphics::Format::ASTC_10x5_SRGB_BLOCK:
    case Graphics::Format::ASTC_10x6_SRGB_BLOCK:
    case Graphics::Format::ASTC_10x8_SRGB_BLOCK:
    case Graphics::Format::ASTC_10x10_SRGB_BLOCK:
    case Graphics::Format::ASTC_12x10_SRGB_BLOCK:
    case Graphics::Format::ASTC_12x12_SRGB_BLOCK:
      return true;
    default:
      return false;
  }
  return false;
}

struct ColorConversion
{
  vk::Format oldFormat;
  vk::Format newFormat;
  std::vector<uint8_t> (*pConversionFunc)(const void*, uint32_t, uint32_t, uint32_t, uint32_t);
  void (*pConversionWriteFunc)(const void*, uint32_t, uint32_t, uint32_t, uint32_t, void*);
};

/**
 * Converts RGB to RGBA
 */
inline std::vector<uint8_t> ConvertRGB32ToRGBA32(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride)
{
  //@todo: use stride if non-zero
  std::vector<uint8_t> rgbaBuffer{};

  auto inData = reinterpret_cast<const uint8_t*>(pData);

  rgbaBuffer.resize(width * height * 4);
  auto outData = rgbaBuffer.data();
  auto outIdx  = 0u;
  for(auto i = 0u; i < sizeInBytes; i += 3)
  {
    outData[outIdx]     = inData[i];
    outData[outIdx + 1] = inData[i + 1];
    outData[outIdx + 2] = inData[i + 2];
    outData[outIdx + 3] = 0xff;
    outIdx += 4;
  }
  return rgbaBuffer;
}

inline void WriteRGB32ToRGBA32(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride, void* pOutput)
{
  auto inData  = reinterpret_cast<const uint8_t*>(pData);
  auto outData = reinterpret_cast<uint8_t*>(pOutput);
  auto outIdx  = 0u;
  for(auto i = 0u; i < sizeInBytes; i += 3)
  {
    outData[outIdx]     = inData[i];
    outData[outIdx + 1] = inData[i + 1];
    outData[outIdx + 2] = inData[i + 2];
    outData[outIdx + 3] = 0xff;
    outIdx += 4;
  }
}

inline float decodePackedComponent(uint32_t value, int bits)
{
  if(value == 0)
    return 0;
  int exponentBits = (bits == 10) ? 5 : 6;
  int mantissaBits = bits - exponentBits - 1;
  int exponent     = (value >> mantissaBits) & ((1 << exponentBits) - 1);
  int mantissa     = value & ((1 << mantissaBits) - 1);
  if(exponent == 0)
  {
    return ldexpf((float)mantissa / (1 << mantissaBits), 1 - (1 << (exponentBits - 1)));
  }
  if(exponent == ((1 << exponentBits) - 1))
  {
    return mantissa ? NAN : INFINITY;
  }

  float significand = 1.0f + (float)mantissa / (1 << mantissaBits);
  return ldexpf(significand, exponent - ((1 << exponentBits) - 1));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

inline uint16_t floatToHalf(float f)
{
  uint32_t x        = *((uint32_t*)&f);
  uint32_t sign     = (x >> 31) & 0x01;
  uint32_t exponent = (x >> 23) & 0xFF;
  uint32_t mantissa = (x & 0x7FFF);
  if(exponent == 0xff)
  {
    return (sign << 15) | (0x7c00) | (mantissa ? 1 : 0);
  }
  if(exponent == 0)
  {
    return sign << 15;
  }
  int newExp = exponent - 127 + 15;
  if(newExp > 31)
  {
    return (sign << 15) | 0x7c00;
  }
  if(newExp <= 0)
  {
    return sign << 15;
  }
  uint32_t newMantissa = mantissa >> 13;
  return (sign << 15) | (newExp << 10) | newMantissa;
}
#pragma GCC diagnostic pop

inline std::vector<uint8_t> ConvertRGBPackedFloatToRGBA16(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride)
{
  //@todo: use stride if non-zero
  std::vector<uint8_t> rgbaBuffer{};

  auto inData = reinterpret_cast<const uint8_t*>(pData);

  rgbaBuffer.resize(width * height * 8);
  auto outData = reinterpret_cast<uint16_t*>(rgbaBuffer.data());
  auto outIdx  = 0u;
  for(auto i = 0u; i < sizeInBytes; i += 4)
  {
    uint32_t packed = *reinterpret_cast<const uint32_t*>(inData + i);
    uint32_t r      = (packed >> 0) & 0x7ff;
    uint32_t g      = (packed >> 11) & 0x7ff;
    uint32_t b      = (packed >> 22) & 0x3ff;
    float    rf     = decodePackedComponent(r, 11);
    float    gf     = decodePackedComponent(g, 11);
    float    bf     = decodePackedComponent(b, 10);

    outData[outIdx]     = floatToHalf(rf);
    outData[outIdx + 1] = floatToHalf(gf);
    outData[outIdx + 2] = floatToHalf(bf);
    outData[outIdx + 3] = floatToHalf(1.0f);

    outIdx += 4;
  }
  return rgbaBuffer;
}

inline void WriteRGBPackedFloatToRGBA16(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride, void* pOutput)
{
  auto inData  = reinterpret_cast<const uint8_t*>(pData);
  auto outData = reinterpret_cast<uint16_t*>(pOutput);
  auto outIdx  = 0u;

  for(auto i = 0u; i < sizeInBytes; i += 4)
  {
    uint32_t packed = *reinterpret_cast<const uint32_t*>(inData + i);
    uint32_t r      = (packed >> 0) & 0x7ff;
    uint32_t g      = (packed >> 11) & 0x7ff;
    uint32_t b      = (packed >> 22) & 0x3ff;
    float    rf     = decodePackedComponent(r, 11);
    float    gf     = decodePackedComponent(g, 11);
    float    bf     = decodePackedComponent(b, 10);

    outData[outIdx]     = floatToHalf(rf);
    outData[outIdx + 1] = floatToHalf(gf);
    outData[outIdx + 2] = floatToHalf(bf);
    outData[outIdx + 3] = floatToHalf(1.0f);

    outIdx += 4;
  }
}

inline std::vector<uint8_t> ConvertRGB16FloatToRGBA16(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride)
{
  //@todo: use stride if non-zero
  std::vector<uint8_t> rgbaBuffer{};

  auto inData = reinterpret_cast<const uint16_t*>(pData);

  rgbaBuffer.resize(width * height * 4 * sizeof(uint16_t));
  auto outData     = reinterpret_cast<uint16_t*>(rgbaBuffer.data());
  auto outIdx      = 0u;
  auto sizeInWords = sizeInBytes / 2;
  for(auto i = 0u; i < sizeInWords; i += 3)
  {
    outData[outIdx]     = inData[i];
    outData[outIdx + 1] = inData[i + 1];
    outData[outIdx + 2] = inData[i + 2];
    outData[outIdx + 3] = floatToHalf(1.0f);

    outIdx += 4;
  }
  return rgbaBuffer;
}

inline void WriteRGB16FloatToRGBA16(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, uint32_t rowStride, void* pOutput)
{
  auto inData  = reinterpret_cast<const uint16_t*>(pData);
  auto outData = reinterpret_cast<uint16_t*>(pOutput);
  auto outIdx  = 0u;

  auto sizeInWords = sizeInBytes / 2;
  for(auto i = 0u; i < sizeInWords; i += 3)
  {
    outData[outIdx]     = inData[i];
    outData[outIdx + 1] = inData[i + 1];
    outData[outIdx + 2] = inData[i + 2];
    outData[outIdx + 3] = floatToHalf(1.0f);

    outIdx += 4;
  }
}

/**
 * Format conversion table
 */
static const std::vector<ColorConversion> COLOR_CONVERSION_TABLE =
  {
    {vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm, ConvertRGB32ToRGBA32, WriteRGB32ToRGBA32},
    {vk::Format::eB10G11R11UfloatPack32, vk::Format::eR16G16B16A16Sfloat, ConvertRGBPackedFloatToRGBA16, WriteRGBPackedFloatToRGBA16},
    {vk::Format::eR16G16B16Sfloat, vk::Format::eR16G16B16A16Sfloat, ConvertRGB16FloatToRGBA16, WriteRGB16FloatToRGBA16}};

/**
 * This function tests whether format is supported by the driver. If possible it applies
 * format conversion to suitable supported pixel format.
 */
bool Texture::TryConvertPixelData(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, std::vector<uint8_t>& outputBuffer)
{
  // No need to convert
  if(mConvertFromFormat == vk::Format::eUndefined)
  {
    return false;
  }

  auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item)
  { return item.oldFormat == mConvertFromFormat; });

  // No suitable format, return empty array
  if(it == COLOR_CONVERSION_TABLE.end())
  {
    return false;
  }

  auto begin = reinterpret_cast<const uint8_t*>(pData);

  outputBuffer = std::move(it->pConversionFunc(begin, sizeInBytes, width, height, 0u));
  return !outputBuffer.empty();
}

bool Texture::TryConvertPixelData(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, void* pOutputBuffer)
{
  // No need to convert
  if(mConvertFromFormat == vk::Format::eUndefined)
  {
    return false;
  }

  auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item)
  { return item.oldFormat == mConvertFromFormat; });

  // No suitable format, return empty array
  if(it == COLOR_CONVERSION_TABLE.end())
  {
    return false;
  }

  auto begin = reinterpret_cast<const uint8_t*>(pData);

  it->pConversionWriteFunc(begin, sizeInBytes, width, height, 0u, pOutputBuffer);

  return true;
}

bool Texture::TryConvertPixelData(const void* pData, Graphics::Format srcFormat, uint32_t sizeInBytes, uint32_t width, uint32_t height, void* pOutputBuffer)
{
  auto convertFromFormat = ConvertApiToVkConst(srcFormat);
  if(convertFromFormat != vk::Format::eUndefined)
  {
    auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item)
    { return item.oldFormat == convertFromFormat; });

    // No suitable format, return empty array
    if(it != COLOR_CONVERSION_TABLE.end())
    {
      auto begin = reinterpret_cast<const uint8_t*>(pData);

      it->pConversionWriteFunc(begin, sizeInBytes, width, height, 0u, pOutputBuffer);

      return true;
    }
  }
  return false;
}

vk::Format Texture::ValidateFormat(vk::Format sourceFormat)
{
  auto                   formatProperties = mDevice.GetPhysicalDevice().getFormatProperties(sourceFormat);
  vk::FormatFeatureFlags formatFlags      = (mDisableStagingBuffer ? formatProperties.linearTilingFeatures : formatProperties.optimalTilingFeatures);

  auto retval = vk::Format::eUndefined;

  // if format isn't supported, see whether suitable conversion is implemented
  if(!formatFlags || sourceFormat == vk::Format::eB10G11R11UfloatPack32)
  {
    auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item)
    { return item.oldFormat == sourceFormat; });

    // No suitable format, return empty array
    if(it != COLOR_CONVERSION_TABLE.end())
    {
      retval = it->newFormat;
    }
  }
  else
  {
    retval = sourceFormat;
  }

  return retval;
}

vk::Format Texture::ValidateCompressedFormat(vk::Format sourceFormat)
{
  auto                   formatProperties = mDevice.GetPhysicalDevice().getFormatProperties(sourceFormat);
  vk::FormatFeatureFlags formatFlags      = formatProperties.optimalTilingFeatures;

  auto retval = vk::Format::eUndefined;
  // @todo: should test for: eFormatFeatureSampledImageBit | eFormatFeatureTransferDstBit
  if(formatFlags)
  {
    retval = sourceFormat;
  }
  else
  {
    DALI_LOG_ERROR("Compressed format %s not supported by GPU\n", vk::to_string(sourceFormat).c_str());
  }
  return retval;
}

Texture::Texture(const Dali::Graphics::TextureCreateInfo& createInfo, VulkanGraphicsController& controller)
: Resource(createInfo, controller),
  mDevice(controller.GetGraphicsDevice()),
  mImage(nullptr),
  mImageView(nullptr),
  mCurrentSurface(nullptr),
  mHasSurfaceReference(false)
{
  // Check env variable in order to enable staging buffers
  auto var = getenv("DALI_DISABLE_TEXTURE_STAGING_BUFFERS");
  if(var && var[0] != '0')
  {
    mDisableStagingBuffer = true;
    mTiling               = TextureTiling::LINEAR;
  }

  mIsNativeImage = (createInfo.nativeImagePtr != nullptr);

  if(mIsNativeImage)
  {
    if(dynamic_cast<Dali::NativeImageSource*>(createInfo.nativeImagePtr.Get()))
    {
      mNativeImageType = NativeImageType::NATIVE_IMAGE_SOURCE;
    }
    else if(dynamic_cast<Dali::NativeImageSourceQueue*>(createInfo.nativeImagePtr.Get()))
    {
      mNativeImageType = NativeImageType::NATIVE_IMAGE_SOURCE_QUEUE;
    }
  }

  DALI_LOG_ERROR("Vulkan::Texture::Texture: createInfo.nativeImagePtr: %p, mIsNativeImage: %d, width: %u, height: %u\n", createInfo.nativeImagePtr, mIsNativeImage, createInfo.nativeImagePtr ? createInfo.nativeImagePtr->GetWidth() : 0u, createInfo.nativeImagePtr ? createInfo.nativeImagePtr->GetHeight() : 0u);

  //  mIsNativeImage = true;

  if(mIsNativeImage)
  {
    auto device = mDevice.GetLogicalDevice();

    if(!gPfnBindImageMemory2KHR && !gPfnGetImageMemoryRequirements2KHR && !gPfnGetMemoryFdPropertiesKHR && !gPfnCreateSamplerYcbcrConversionKHR)
    {
      gPfnBindImageMemory2KHR = reinterpret_cast<PFN_vkBindImageMemory2KHR>(
        device.getProcAddr("vkBindImageMemory2KHR"));

      gPfnGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(
        device.getProcAddr("vkGetImageMemoryRequirements2KHR"));

      gPfnGetMemoryFdPropertiesKHR = reinterpret_cast<PFN_vkGetMemoryFdPropertiesKHR>(
        device.getProcAddr("vkGetMemoryFdPropertiesKHR"));

      gPfnCreateSamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversionKHR>(
        device.getProcAddr("vkCreateSamplerYcbcrConversionKHR"));
    }
  }

  mSampler = controller.GetDefaultSampler();
}

Texture::~Texture()
{
  DALI_LOG_ERROR("Vulkan::Texture::~Texture: mIsNativeImage: %d\n", mIsNativeImage);
  if(mIsNativeImage)
  {
    // Release surface reference before destruction
    ReleaseCurrentSurfaceReference();
    DALI_LOG_ERROR("Texture::~Texture: Released surface reference\n");

    delete mSampler;
  }

  delete mImageView;
  delete mImage;
}

ResourceBase::InitializationResult Texture::InitializeResource()
{
  if(!mIsNativeImage || (mIsNativeImage && mNativeImageType == NativeImageType::NATIVE_IMAGE_SOURCE))
  {
    bool initialized = Initialize();

    if(initialized)
    {
      return InitializationResult::INITIALIZED;
    }
  }

  return InitializationResult::NOT_INITIALIZED_YET;
}

void Texture::DestroyResource()
{
  DALI_LOG_ERROR("Vulkan::Texture::DestroyResource: mIsNativeImage: %d\n", mIsNativeImage);

  if(mIsNativeImage)
  {
    // Release surface reference before destroying resources
    ReleaseCurrentSurfaceReference();

    // Destroy native image Vulkan resources.
    mSampler->Destroy();
    mSampler = nullptr;

    auto device = mDevice.GetLogicalDevice();

    if(mYcbcrConversion != VK_NULL_HANDLE)
    {
      device.destroySamplerYcbcrConversion(static_cast<vk::SamplerYcbcrConversion>(mYcbcrConversion));
      mYcbcrConversion = VK_NULL_HANDLE;
    }

    for(auto& memory : mNativeMemories)
    {
      device.freeMemory(memory);
    }

    mNativeMemories.clear();

    if(mNativeImage != VK_NULL_HANDLE)
    {
      device.destroyImage(mNativeImage);
      mNativeImage = VK_NULL_HANDLE;
    }

    mPlaneFds.clear();

    // Release any remaining BO references
    ReleaseSurfaceBufferObjectReferences();
  }

  if(mImageView)
  {
    mImageView->Destroy();
    mImageView = nullptr;
  }

  if(mImage)
  {
    mImage->Destroy();
    mImage = nullptr;
  }
}

void Texture::DiscardResource()
{
  mController.DiscardResource(this);
}

bool Texture::Initialize()
{
  SetFormatAndUsage();

  if(mFormat == vk::Format::eUndefined)
  {
    DALI_LOG_ERROR("Vulkan::Texture::InitializeResource: Invalid texture format\n", static_cast<int>(mFormat));
    // not supported!
    return false;
  }

  bool initialized = mIsNativeImage ? InitializeNativeTexture() : InitializeTexture();

  DALI_LOG_ERROR("Vulkan::Texture::InitializeResource: initialized: %d\n", initialized);

  if(initialized)
  {
    // force generating properties
    GetProperties();
  }

  return initialized;
}

void Texture::SetFormatAndUsage()
{
  auto size = mCreateInfo.size;
  mWidth    = uint32_t(size.width);
  mHeight   = uint32_t(size.height);
  mLayout   = vk::ImageLayout::eUndefined;

  vk::Format format = vk::Format::eUndefined;

  DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage: mIsNativeImage: %d\n", mIsNativeImage);

  if(mIsNativeImage)
  {
    DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage for native image\n");

    NativeImageInterfacePtr nativeImage       = mCreateInfo.nativeImagePtr;
    Dali::Any               nativeImageSource = nativeImage->GetNativeImageHandle();
    if(nativeImageSource.GetType() == typeid(tbm_surface_h))
    {
      tbm_surface_h tbmSurface = AnyCast<tbm_surface_h>(nativeImageSource);
      if(tbm_surface_internal_is_valid(tbmSurface))
      {
        DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage: Valid TBM surface\n");

        mTbmSurface          = tbmSurface;
        tbm_format tbmFormat = tbm_surface_get_format(tbmSurface);
        format               = GetVulkanFormat(tbmFormat);
        mIsYUVFormat         = RequiresYcbcrConversion(tbmFormat);
        vk::ImageUsageFlags usage{};
        if(mIsYUVFormat)
        {
          mUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        }
        else
        {
          mUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        }
        mTiling = Dali::Graphics::TextureTiling::LINEAR;

        DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage for native image: tbmFormat: %d, format: %d, mIsYUVFormat: %d\n", static_cast<int>(tbmFormat), static_cast<int>(format), mIsYUVFormat);
      }
      else
      {
        DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage: Invalid TBM surface\n");
      }
    }
    else
    {
      DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage: nativeImageSource.GetType() != typeid(tbm_surface_h)\n");
    }
  }
  else
  {
    DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage for NON-native image\n");

    if(mCreateInfo.usageFlags & (0 | TextureUsageFlagBits::COLOR_ATTACHMENT))
    {
      mUsage  = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
      mTiling = TextureTiling::OPTIMAL; // force always OPTIMAL tiling
    }
    else if(mCreateInfo.usageFlags & (0 | TextureUsageFlagBits::DEPTH_STENCIL_ATTACHMENT))
    {
      mUsage  = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
      mTiling = TextureTiling::OPTIMAL; // force always OPTIMAL tiling
    }
    else if(mCreateInfo.usageFlags & (0 | TextureUsageFlagBits::SAMPLE))
    {
      mUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    }

    format = ConvertApiToVk(mCreateInfo.format);

    DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage for NON-native image: mCreateInfo.format: %d, format: %d\n", static_cast<int>(mCreateInfo.format), static_cast<int>(format));
  }

  if(IsCompressed(mCreateInfo.format))
  {
    mFormat = ValidateCompressedFormat(format);
  }
  else
  {
    mFormat = ValidateFormat(format); // reconvert from eB10G11R11,astc, etc1 to eR16G16B16A16
  }

  mConvertFromFormat = vk::Format::eUndefined;

  DALI_LOG_ERROR("Vulkan::Texture::SetFormatAndUsage ValidateFormat: format: %d, mFormat: %d\n", static_cast<int>(format), static_cast<int>(mFormat));

  if(format != mFormat)
  {
    mConvertFromFormat = format;
  }
  mComponentMapping = GetVkComponentMapping(mCreateInfo.format);
}

bool Texture::InitializeNativeTexture()
{
  DALI_LOG_ERROR("Vulkan::Texture::InitializeNativeTexture: BEGIN\n");

  if(mNativeImage != VK_NULL_HANDLE || !mNativeMemories.empty())
  {
    DALI_LOG_ERROR("InitializeNativeTexture: Cleaning up old native image resources\n");

    auto device = mDevice.GetLogicalDevice();

    // Free old device memories
    for(auto& memory : mNativeMemories)
    {
      DALI_LOG_ERROR("InitializeNativeTexture: Freeing old VkDeviceMemory %p\n",
                     static_cast<VkDeviceMemory>(memory));
      device.freeMemory(memory);
    }
    mNativeMemories.clear();

    // Destroy old image
    if(mNativeImage != VK_NULL_HANDLE)
    {
      DALI_LOG_ERROR("InitializeNativeTexture: Destroying old VkImage %p\n",
                     static_cast<VkImage>(mNativeImage));
      device.destroyImage(mNativeImage);
      mNativeImage = VK_NULL_HANDLE;
    }

    // Release old BO references
    ReleaseSurfaceBufferObjectReferences();
  }

  NativeImageInterfacePtr nativeImage = mCreateInfo.nativeImagePtr;

  tbm_surface_h tbmSurface;

  if(!mTbmSurface)
  {
    DALI_LOG_ERROR("Invalid TBM surface\n");
    return false;
  }
  else
  {
    tbmSurface = reinterpret_cast<tbm_surface_h>(mTbmSurface);
    if(!tbm_surface_internal_is_valid(tbmSurface))
    {
      DALI_LOG_ERROR("Invalid TBM surface\n");
      return false;
    }

    DALI_LOG_ERROR("Vulkan::Texture::TBM surface valid\n");
  }

  if(mFormat == vk::Format::eUndefined)
  {
    DALI_LOG_ERROR("Unsupported TBM forma\n");
    return false;
  }

  DALI_LOG_ERROR("Vulkan::Texture::native image vulkan format: %d\n", static_cast<int>(mFormat));

  bool created = nativeImage->CreateResource();

  DALI_LOG_ERROR("Vulkan::Texture::native image CreateResource created: %d\n", created);

  if(created)
  {
    // Acquire surface reference before using it
    AcquireCurrentSurfaceReference();

    // 1. Export plane file descriptors
    if(!ExportPlaneFds())
    {
      DALI_LOG_ERROR("Failed to export plane FDs\n");
      ReleaseCurrentSurfaceReference();
      return false;
    }

    DALI_LOG_ERROR("Vulkan::Texture::native image ExportPlaneFds succeeded: mIsYUVFormat: %d\n", mIsYUVFormat);

    if(mIsYUVFormat && !mDevice.IsKHRSamplerYCbCrConversionSupported())
    {
      DALI_LOG_ERROR("SamplerYcbcrConversion feature required for YUV texture is not supported\n");
      ReleaseCurrentSurfaceReference();
      return false;
    }

    // 2. Create Vulkan image from external memory
    if(!CreateNativeImage())
    {
      DALI_LOG_ERROR("Failed to create Vulkan image\n");
      ReleaseCurrentSurfaceReference();
      return false;
    }

    DALI_LOG_ERROR("Vulkan::Texture::CreateNativeImage succeeded\n");

    // 3. Create SamplerYcbcrConversion (if needed)
    if(mIsYUVFormat)
    {
      if(!CreateYcbcrConversion())
      {
        DALI_LOG_ERROR("Failed to create Ycbcr Conversion\n");
        ReleaseCurrentSurfaceReference();
        return false;
      }
    }

    // 4. Create image view for the imported image
    if(!CreateNativeImageView())
    {
      DALI_LOG_ERROR("Failed to create image view\n");
      ReleaseCurrentSurfaceReference();
      return false;
    }

    DALI_LOG_ERROR("Vulkan::Texture::CreateNativeImageView succeeded\n");

    // 5. Create sampler with optional YCbCr conversion
    if(!CreateNativeSampler())
    {
      DALI_LOG_ERROR("Failed to create sampler\n");
      ReleaseCurrentSurfaceReference();
      return false;
    }

    DALI_LOG_ERROR("Vulkan::Texture::CreateNativeSampler succeeded\n");
  }
  else
  {
    DALI_LOG_ERROR("Native Image: InitializeNativeTexture, CreateResource() failed\n");
  }

  return created; // WARNING! May be false! Needs handling! (Well, initialized on bind)}
}

// creates image with pre-allocated memory and default sampler, no data
// uploaded at this point
bool Texture::InitializeTexture()
{
  if(mImage || (mWidth == 0 || mHeight == 0))
  {
    return false;
  }

  auto tiling = ((mDisableStagingBuffer || mTiling == Dali::Graphics::TextureTiling::LINEAR) ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal);

  mMaxMipMapLevel = 1;
  if(mCreateInfo.mipMapFlag == TextureMipMapFlag::ENABLED)
  {
    // Mip levels: bit width of dims-3; so should cap at 4x4.
    mMaxMipMapLevel = std::min(1.0f, logf(std::max(1u, std::min(mWidth, mHeight))) / logf(2.0f) - 3);
  }
  // create image
  auto imageCreateInfo = vk::ImageCreateInfo{}
                           .setFormat(mFormat)
                           .setInitialLayout(mLayout)
                           .setSamples(vk::SampleCountFlagBits::e1)
                           .setSharingMode(vk::SharingMode::eExclusive)
                           .setUsage(mUsage)
                           .setExtent({mWidth, mHeight, 1})
                           .setArrayLayers(1)
                           .setImageType(vk::ImageType::e2D)
                           .setTiling(tiling)
                           .setMipLevels(mMaxMipMapLevel);

  bool cpuVisible = (mTiling == Dali::Graphics::TextureTiling::LINEAR);
  if(mCreateInfo.textureType == Dali::Graphics::TextureType::TEXTURE_CUBEMAP)
  {
    imageCreateInfo.setArrayLayers(6);
    imageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    cpuVisible = false;
  }
  vk::ImageFormatProperties props;
  auto                      result = mDevice.GetPhysicalDevice().getImageFormatProperties(mFormat, vk::ImageType::e2D, tiling, mUsage, imageCreateInfo.flags, &props);
  VkTest(result, vk::Result::eSuccess);

  vk::MemoryPropertyFlags memoryProperties{};
  if(mDisableStagingBuffer)
  {
    memoryProperties |= vk::MemoryPropertyFlagBits::eDeviceLocal;
  }

  if(cpuVisible)
    memoryProperties |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
  else
    memoryProperties |= vk::MemoryPropertyFlagBits::eDeviceLocal;

  // Create the image handle
  mImage = Image::New(mDevice, imageCreateInfo, memoryProperties);

  if(!mImage)
  {
    return false;
  }

  // Non sampled image will be lazily initialised
  if(!(mUsage & vk::ImageUsageFlagBits::eTransferDst))
  {
    InitializeImageView();
  }
  return true;
}

void Texture::InitializeImageView()
{
  if(!mImageView)
  {
    // Create image view
    mImageView = ImageView::NewFromImage(mDevice, *mImage, mComponentMapping);
  }
}

std::unique_ptr<Vulkan::ImageView> Texture::CreateImageView()
{
  if(!mImageView)
  {
    // Ensure we have initialized the image:
    InitializeImageView();
  }
  //@todo: Can we just return mImageView? Why create 2nd?
  std::unique_ptr<Vulkan::ImageView> imageView(ImageView::NewFromImage(mDevice, *mImage, mComponentMapping));
  return imageView;
}

Vulkan::Image* Texture::GetImage() const
{
  return mImage;
}

Vulkan::ImageView* Texture::GetImageView() const
{
  return mImageView;
}

Vulkan::SamplerImpl* Texture::GetDefaultSampler() const
{
  return mSampler;
}

vk::Format Texture::ConvertApiToVk(Dali::Graphics::Format format)
{
  return ConvertApiToVkConst(format);
}

vk::ComponentMapping Texture::GetVkComponentMapping(Dali::Graphics::Format format)
{
  return GetVkComponentMappingInlined(format);
}

bool Texture::IsSamplerImmutable() const
{
  return false;
}

MemoryRequirements Texture::GetMemoryRequirements() const
{
  auto requirements = mDevice.GetLogicalDevice().getImageMemoryRequirements(mImage->GetVkHandle());
  return MemoryRequirements{size_t(requirements.size), size_t(requirements.alignment)};
}

const TextureProperties& Texture::GetProperties()
{
  if(!mProperties)
  {
    mProperties = std::move(std::make_unique<Dali::Graphics::TextureProperties>());

    auto formatInfo                       = GetFormatInfo(mFormat);
    mProperties->compressed               = formatInfo.compressed;
    mProperties->packed                   = formatInfo.packed;
    mProperties->emulated                 = mConvertFromFormat != vk::Format::eUndefined;
    mProperties->format                   = mCreateInfo.format;
    mProperties->format1                  = mCreateInfo.format;
    mProperties->extent2D                 = {mWidth, mHeight};
    mProperties->directWriteAccessEnabled = (mTiling == Dali::Graphics::TextureTiling::LINEAR);
    mProperties->nativeHandle             = 0; //@todo change to Dali::Any, and pass vkImage handle
  }
  return *mProperties;
}

uint32_t Texture::FindMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags flags) const
{
  auto memProperties = mDevice.GetMemoryProperties();

  for(uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
  {
    if((typeBits & (1u << i)) &&
       (memProperties.memoryTypes[i].propertyFlags & flags) == flags)
    {
      return i;
    }
  }

  DALI_LOG_ERROR("Failed to find suitable memory type\n");
  return 0;
}

bool Texture::ExportPlaneFds()
{
  DALI_LOG_ERROR("=== ExportPlaneFds: BEGIN ===\n");

  if(!mTbmSurface)
  {
    DALI_LOG_ERROR("ExportPlaneFds: mTbmSurface is NULL, returning false\n");
    return false;
  }

  DALI_LOG_ERROR("ExportPlaneFds: Clearing old FD/BO containers - mPlaneFds.size()=%zu, mTbmBos.size()=%zu\n",
                 mPlaneFds.size(),
                 mTbmBos.size());

  mPlaneFds.clear(); // clear any old, stale FDs
  mTbmBos.clear();   // clear any old BO references

  tbm_surface_h      tbmSurface = reinterpret_cast<tbm_surface_h>(mTbmSurface);
  tbm_surface_info_s tbmSurfaceInfo;

  DALI_LOG_ERROR("ExportPlaneFds: Getting TBM surface info for surface %p\n", tbmSurface);

  if(tbm_surface_get_info(tbmSurface, &tbmSurfaceInfo) != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("ExportPlaneFds: Failed to get TBM surface info\n");
    return false;
  }

  int num_bos = tbm_surface_internal_get_num_bos(tbmSurface);
  DALI_LOG_ERROR("ExportPlaneFds: Found %d buffer objects\n", num_bos);

  for(int i = 0; i < num_bos; ++i)
  {
    DALI_LOG_ERROR("ExportPlaneFds: Processing BO %d/%d\n", i + 1, num_bos);

    tbm_bo bo = tbm_surface_internal_get_bo(tbmSurface, i);
    if(!bo)
    {
      DALI_LOG_ERROR("ExportPlaneFds: BO %d is NULL, skipping\n", i);
      continue;
    }

    DALI_LOG_ERROR("ExportPlaneFds: BO %d pointer=%p\n", i, bo);

    // Export original FD from TBM
    int originalFd = tbm_bo_export_fd(bo);
    DALI_LOG_ERROR("ExportPlaneFds: tbm_bo_export_fd(bo=%p) returned FD=%d\n", bo, originalFd);

    if(originalFd < 0)
    {
      DALI_LOG_ERROR("ExportPlaneFds: Failed to export FD for BO %d (returned %d)\n", i, originalFd);
      return false;
    }

    // Duplicate the FD
    int dupFd = dup(originalFd);
    DALI_LOG_ERROR("ExportPlaneFds: dup(%d) returned %d\n", originalFd, dupFd);

    if(dupFd < 0)
    {
      DALI_LOG_ERROR("ExportPlaneFds: Failed to duplicate FD %d (errno=%d: %s)\n",
                     originalFd,
                     errno,
                     strerror(errno));
      close(originalFd); // Close the exported FD we won't use
      return false;
    }

    // Close the original exported FD since we have a duplicate
    DALI_LOG_ERROR("ExportPlaneFds: Closing original FD %d (we're using duplicate %d)\n",
                   originalFd,
                   dupFd);
    close(originalFd);

    // Keep TBM BO alive beyond Vulkan import
    DALI_LOG_ERROR("ExportPlaneFds: Calling tbm_bo_ref(bo=%p)\n", bo);
    tbm_bo_ref(bo);
    mTbmBos.push_back(static_cast<void*>(bo));
    DALI_LOG_ERROR("ExportPlaneFds: Added BO %p to mTbmBos (new size=%zu)\n", bo, mTbmBos.size());

    mPlaneFds.push_back(dupFd);
    DALI_LOG_ERROR("ExportPlaneFds: Added FD %d to mPlaneFds (new size=%zu)\n", dupFd, mPlaneFds.size());

    // Verify the duplicated FD is valid
    if(fcntl(dupFd, F_GETFD) == -1)
    {
      DALI_LOG_ERROR("ExportPlaneFds: WARNING - duplicated FD %d is already invalid after creation! errno=%d: %s\n",
                     dupFd,
                     errno,
                     strerror(errno));
    }
    else
    {
      DALI_LOG_ERROR("ExportPlaneFds: Verified duplicated FD %d is valid\n", dupFd);
    }
  }

  bool success = !mPlaneFds.empty();
  DALI_LOG_ERROR("=== ExportPlaneFds: END - returning %s (exported %zu FDs) ===\n",
                 success ? "TRUE" : "FALSE",
                 mPlaneFds.size());

  return success;
}

vk::DeviceMemory Texture::ImportPlaneMemory(int fd)
{
  DALI_LOG_ERROR("=== ImportPlaneMemory: BEGIN with FD=%d ===\n", fd);

  // Validate file descriptor before use
  if(fd < 0)
  {
    DALI_LOG_ERROR("ImportPlaneMemory: Invalid file descriptor: %d\n", fd);
    return VK_NULL_HANDLE;
  }

  DALI_LOG_ERROR("ImportPlaneMemory: FD %d passed initial validation (>= 0)\n", fd);

  // Use fcntl to check FD validity
  int fcntl_result = fcntl(fd, F_GETFD);
  if(fcntl_result == -1)
  {
    DALI_LOG_ERROR("ImportPlaneMemory: fcntl(F_GETFD) failed for FD %d - errno=%d: %s\n",
                   fd,
                   errno,
                   strerror(errno));
    return VK_NULL_HANDLE;
  }

  DALI_LOG_ERROR("ImportPlaneMemory: fcntl(F_GETFD) succeeded for FD %d (result=%d)\n",
                 fd,
                 fcntl_result);

  auto device = mDevice.GetLogicalDevice();

  try
  {
    DALI_LOG_ERROR("ImportPlaneMemory: Getting DMA buffer size via lseek for FD %d\n", fd);

    // Use lseek to get actual DMA buffer size
    const off_t dma_buf_size = lseek(fd, 0, SEEK_END);
    if(dma_buf_size < 0)
    {
      DALI_LOG_ERROR("ImportPlaneMemory: lseek(SEEK_END) failed for FD %d - errno=%d: %s\n",
                     fd,
                     errno,
                     strerror(errno));
      return VK_NULL_HANDLE;
    }

    DALI_LOG_ERROR("ImportPlaneMemory: DMA buffer size for FD %d is %lld bytes\n",
                   fd,
                   (long long)dma_buf_size);

    // Reset file offset
    off_t seek_result = lseek(fd, 0, SEEK_SET);
    DALI_LOG_ERROR("ImportPlaneMemory: lseek(SEEK_SET) returned %lld for FD %d\n",
                   (long long)seek_result,
                   fd);

    //Get memory properties from FD
    DALI_LOG_ERROR("ImportPlaneMemory: Calling vkGetMemoryFdPropertiesKHR for FD %d\n", fd);
    auto memFdProps  = VkMemoryFdPropertiesKHR{};
    memFdProps.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;

    if(!gPfnGetMemoryFdPropertiesKHR)
    {
      DALI_LOG_ERROR("ImportPlaneMemory: gPfnGetMemoryFdPropertiesKHR is NULL!\n");
      return VK_NULL_HANDLE;
    }

    VkResult fdPropsResult = gPfnGetMemoryFdPropertiesKHR(device,
                                                          VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
                                                          fd,
                                                          &memFdProps);

    if(fdPropsResult != VK_SUCCESS)
    {
      DALI_LOG_ERROR("ImportPlaneMemory: vkGetMemoryFdPropertiesKHR failed for FD %d - result=%d\n",
                     fd,
                     fdPropsResult);
      return VK_NULL_HANDLE;
    }

    DALI_LOG_ERROR("ImportPlaneMemory: vkGetMemoryFdPropertiesKHR succeeded for FD %d - memoryTypeBits=0x%x\n",
                   fd,
                   memFdProps.memoryTypeBits);

    // Import memory with FD-specific memory type
    DALI_LOG_ERROR("ImportPlaneMemory: Creating import info for FD %d\n", fd);
    auto importInfo = vk::ImportMemoryFdInfoKHR{}
                        .setHandleType(vk::ExternalMemoryHandleTypeFlagBits::eDmaBufEXT)
                        .setFd(fd);

    uint32_t memoryTypeIndex = FindMemoryType(memFdProps.memoryTypeBits,
                                              vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent);

    DALI_LOG_ERROR("ImportPlaneMemory: Found memory type index %u for FD %d\n",
                   memoryTypeIndex,
                   fd);

    auto allocInfo = vk::MemoryAllocateInfo{}
                       .setPNext(&importInfo)
                       .setAllocationSize(static_cast<vk::DeviceSize>(dma_buf_size))
                       .setMemoryTypeIndex(memoryTypeIndex);

    DALI_LOG_ERROR("ImportPlaneMemory: Calling vkAllocateMemory for FD %d (size=%lld, typeIndex=%u)\n",
                   fd,
                   (long long)dma_buf_size,
                   memoryTypeIndex);

    vk::DeviceMemory memory = device.allocateMemory(allocInfo).value;

    DALI_LOG_ERROR("=== ImportPlaneMemory: SUCCESS - FD %d imported to memory handle %p ===\n",
                   fd,
                   static_cast<VkDeviceMemory>(memory));

    return memory;
  }
  catch(const std::system_error& e)
  {
    DALI_LOG_ERROR("ImportPlaneMemory: EXCEPTION caught for FD %d: %s\n", fd, e.what());
    return VK_NULL_HANDLE;
  }
  catch(...)
  {
    DALI_LOG_ERROR("ImportPlaneMemory: UNKNOWN EXCEPTION caught for FD %d\n", fd);
    return VK_NULL_HANDLE;
  }
}

bool Texture::CreateNativeImage()
{
  DALI_LOG_ERROR("=== CreateNativeImage: BEGIN ===\n");

  try
  {
    auto device = mDevice.GetLogicalDevice();

    DALI_LOG_ERROR("CreateNativeImage: Creating external memory image (format=%d, size=%ux%u)\n",
                   static_cast<int>(mFormat),
                   mWidth,
                   mHeight);

    // Create external memory image
    auto extMemCreateInfo = vk::ExternalMemoryImageCreateInfo{}
                              .setHandleTypes(vk::ExternalMemoryHandleTypeFlagBits::eDmaBufEXT);

    auto imageCreateInfo = vk::ImageCreateInfo{}
                             .setPNext(static_cast<void*>(&extMemCreateInfo))
                             .setImageType(vk::ImageType::e2D)
                             .setFormat(mFormat)
                             .setExtent({mWidth, mHeight, 1})
                             .setMipLevels(1)
                             .setArrayLayers(1)
                             .setSamples(vk::SampleCountFlagBits::e1)
                             .setTiling(vk::ImageTiling::eLinear)
                             .setUsage(mUsage)
                             .setSharingMode(vk::SharingMode::eExclusive)
                             .setInitialLayout(mIsYUVFormat ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined);

    mNativeImage = device.createImage(imageCreateInfo).value;
    DALI_LOG_ERROR("CreateNativeImage: Created VkImage handle %p\n", static_cast<VkImage>(mNativeImage));

    mImage = new Image(mDevice, imageCreateInfo, mNativeImage);

    DALI_LOG_ERROR("CreateNativeImage: Checking plane configuration - mPlaneFds.size()=%zu\n",
                   mPlaneFds.size());

    // Check for disjoint vs non-disjoint multi-plane layout
    bool isDisjoint = false;
    if(mPlaneFds.size() > 1)
    {
      DALI_LOG_ERROR("CreateNativeImage: Multiple planes detected, checking if disjoint\n");
      for(size_t i = 1; i < mPlaneFds.size(); ++i)
      {
        DALI_LOG_ERROR("CreateNativeImage: Comparing FD[0]=%d with FD[%zu]=%d\n",
                       mPlaneFds[0],
                       i,
                       mPlaneFds[i]);
        if(mPlaneFds[i] != mPlaneFds[0])
        {
          isDisjoint = true;
          DALI_LOG_ERROR("CreateNativeImage: Detected disjoint memory layout\n");
          break;
        }
      }
    }

    if(!isDisjoint)
    {
      DALI_LOG_ERROR("CreateNativeImage: Using non-disjoint/single-plane layout\n");
    }

    // Import memory for each plane
    mNativeMemories.clear();
    DALI_LOG_ERROR("CreateNativeImage: Cleared mNativeMemories\n");

    if(!isDisjoint && !mPlaneFds.empty())
    {
      DALI_LOG_ERROR("CreateNativeImage: Single memory binding path - importing FD %d\n",
                     mPlaneFds[0]);

      // Verify FD is still valid before import
      if(fcntl(mPlaneFds[0], F_GETFD) == -1)
      {
        DALI_LOG_ERROR("CreateNativeImage: ERROR - FD %d is INVALID before import! errno=%d: %s\n",
                       mPlaneFds[0],
                       errno,
                       strerror(errno));
        return false;
      }

      DALI_LOG_ERROR("CreateNativeImage: FD %d verified valid before import\n", mPlaneFds[0]);

      // Single memory binding for non-disjoint or single-plane
      auto memory = ImportPlaneMemory(mPlaneFds[0]);
      if(memory == VK_NULL_HANDLE)
      {
        DALI_LOG_ERROR("CreateNativeImage: ImportPlaneMemory failed for FD %d\n", mPlaneFds[0]);
        return false;
      }

      DALI_LOG_ERROR("CreateNativeImage: Successfully imported memory %p from FD %d\n",
                     static_cast<VkDeviceMemory>(memory),
                     mPlaneFds[0]);

      mNativeMemories.push_back(memory);
      DALI_LOG_ERROR("CreateNativeImage: Added memory to mNativeMemories (size=%zu)\n",
                     mNativeMemories.size());

      DALI_LOG_ERROR("CreateNativeImage: Binding image to memory\n");
      VkAssert(device.bindImageMemory(mNativeImage, memory, 0));
      DALI_LOG_ERROR("CreateNativeImage: Successfully bound image to memory\n");

      // Close the duplicated FD after successful import
      DALI_LOG_ERROR("CreateNativeImage: Closing duplicated FD %d\n", mPlaneFds[0]);
      close(mPlaneFds[0]);
      mPlaneFds.clear();
    }
    else if(isDisjoint)
    {
      DALI_LOG_ERROR("CreateNativeImage: Disjoint multi-plane binding path\n");

      tbm_surface_h      tbmSurface = reinterpret_cast<tbm_surface_h>(mTbmSurface);
      tbm_surface_info_s tbmSurfaceInfo;

      if(tbm_surface_get_info(tbmSurface, &tbmSurfaceInfo) != TBM_SURFACE_ERROR_NONE)
      {
        DALI_LOG_ERROR("CreateNativeImage: Failed to get TBM surface info\n");
        return false;
      }

      // Multi-plane binding with VkBindImageMemory2
      std::vector<vk::BindImageMemoryInfo>      bindInfos;
      std::vector<vk::BindImagePlaneMemoryInfo> planeInfos;

      for(size_t i = 0; i < mPlaneFds.size(); ++i)
      {
        DALI_LOG_ERROR("CreateNativeImage: Importing plane %zu FD %d\n", i, mPlaneFds[i]);

        // Verify FD is still valid
        if(fcntl(mPlaneFds[i], F_GETFD) == -1)
        {
          DALI_LOG_ERROR("CreateNativeImage: ERROR - FD %d for plane %zu is INVALID! errno=%d: %s\n",
                         mPlaneFds[i],
                         i,
                         errno,
                         strerror(errno));
          return false;
        }

        auto memory = ImportPlaneMemory(mPlaneFds[i]);
        if(memory == VK_NULL_HANDLE)
        {
          DALI_LOG_ERROR("CreateNativeImage: Failed to import memory for plane %zu FD %d\n",
                         i,
                         mPlaneFds[i]);
          return false;
        }

        DALI_LOG_ERROR("CreateNativeImage: Successfully imported memory %p for plane %zu\n",
                       static_cast<VkDeviceMemory>(memory),
                       i);

        mNativeMemories.push_back(memory);

        // Setup plane binding info
        planeInfos.emplace_back(vk::BindImagePlaneMemoryInfo{}
                                  .setPlaneAspect(PLANE_ASPECT_FLAGS[i]));

        bindInfos.emplace_back(vk::BindImageMemoryInfo{}
                                 .setPNext(&planeInfos[i])
                                 .setImage(mNativeImage)
                                 .setMemory(memory)
                                 .setMemoryOffset(tbmSurfaceInfo.planes[i].offset));

        DALI_LOG_ERROR("CreateNativeImage: Created bind info for plane %zu (offset=%zu)\n",
                       i,
                       tbmSurfaceInfo.planes[i].offset);
      }

      DALI_LOG_ERROR("CreateNativeImage: Binding %zu planes to image\n", bindInfos.size());

      vk::Result result = device.bindImageMemory2(bindInfos);

      // Close all duplicated FDs after binding
      for(size_t i = 0; i < mPlaneFds.size(); ++i)
      {
        DALI_LOG_ERROR("CreateNativeImage: Closing FD %d for plane %zu\n", mPlaneFds[i], i);
        close(mPlaneFds[i]);
      }
      mPlaneFds.clear();

      if(result != vk::Result::eSuccess)
      {
        DALI_LOG_ERROR("CreateNativeImage: vkBindImageMemory2 failed with result=%d\n",
                       static_cast<int>(result));

        // Release BO references after Vulkan import completes
        ReleaseSurfaceBufferObjectReferences();
        return false;
      }

      DALI_LOG_ERROR("CreateNativeImage: Successfully bound all planes\n");
    }

    DALI_LOG_ERROR("=== CreateNativeImage: SUCCESS ===\n");
    return true;
  }
  catch(const std::system_error& e)
  {
    DALI_LOG_ERROR("CreateNativeImage: EXCEPTION - %s\n", e.what());

    // Release BO references after Vulkan import completes
    ReleaseSurfaceBufferObjectReferences();
    return false;
  }
  catch(...)
  {
    DALI_LOG_ERROR("CreateNativeImage: UNKNOWN EXCEPTION\n");

    // Release BO references after Vulkan import completes
    ReleaseSurfaceBufferObjectReferences();
    return false;
  }
}

bool Texture::CreateYcbcrConversion()
{
  bool support_cosited_chroma_sampling = false;
  bool support_linearfilter            = false;

  // Check whether format is supported by the platform
  auto formatProperties = mDevice.GetPhysicalDevice().getFormatProperties(mFormat);

  if(!(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eCositedChromaSamples) || !(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eCositedChromaSamples))
  {
    support_cosited_chroma_sampling = true;
  }

  if(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageYcbcrConversionLinearFilter)
  {
    support_linearfilter = true;
  }

  // Create YCbCr conversion
  auto conversionCreateInfo = vk::SamplerYcbcrConversionCreateInfo{}
                                .setFormat(mFormat)
                                .setYcbcrModel(vk::SamplerYcbcrModelConversion::eYcbcr709)
                                .setYcbcrRange(vk::SamplerYcbcrRange::eItuFull)
                                .setComponents(vk::ComponentMapping()
                                                 .setR(vk::ComponentSwizzle::eIdentity)
                                                 .setG(vk::ComponentSwizzle::eIdentity)
                                                 .setB(vk::ComponentSwizzle::eIdentity)
                                                 .setA(vk::ComponentSwizzle::eIdentity))
                                .setXChromaOffset(support_cosited_chroma_sampling ? vk::ChromaLocation::eCositedEven : vk::ChromaLocation::eMidpoint)
                                .setYChromaOffset(support_cosited_chroma_sampling ? vk::ChromaLocation::eCositedEven : vk::ChromaLocation::eMidpoint)
                                .setChromaFilter(support_linearfilter ? vk::Filter::eLinear : vk::Filter::eNearest)
                                .setForceExplicitReconstruction(false);

  if(!gPfnCreateSamplerYcbcrConversionKHR || VK_SUCCESS != gPfnCreateSamplerYcbcrConversionKHR(mDevice.GetLogicalDevice(), reinterpret_cast<const VkSamplerYcbcrConversionCreateInfo*>(static_cast<const void*>(&conversionCreateInfo)), nullptr, &mYcbcrConversion))
  {
    DALI_LOG_ERROR("vkCreateSamplerYcbcrConversion failed\n");
    return false;
  }

  return true;
}

bool Texture::CreateNativeImageView()
{
  try
  {
    auto viewInfo = vk::ImageViewCreateInfo{}
                      .setImage(mNativeImage)
                      .setViewType(vk::ImageViewType::e2D)
                      .setFormat(mFormat)
                      .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    // Chain YCbCr conversion info for YUV formats
    if(mIsYUVFormat && mYcbcrConversion != VK_NULL_HANDLE)
    {
      mYcbcrConversionInfo = vk::SamplerYcbcrConversionInfo{}
                               .setConversion(static_cast<vk::SamplerYcbcrConversion>(mYcbcrConversion));

      viewInfo.setPNext(&mYcbcrConversionInfo)
        .setComponents(vk::ComponentMapping()
                         .setR(vk::ComponentSwizzle::eIdentity)
                         .setG(vk::ComponentSwizzle::eIdentity)
                         .setB(vk::ComponentSwizzle::eIdentity)
                         .setA(vk::ComponentSwizzle::eIdentity));
    }

    mImageView = ImageView::New(mDevice, *mImage, viewInfo);

    return true;
  }
  catch(const std::system_error& e)
  {
    DALI_LOG_ERROR("Failed to create image view: %s\n", e.what());
    return false;
  }
}

bool Texture::CreateNativeSampler()
{
  try
  {
    vk::SamplerCreateInfo samplerCreateInfo{};

    if(mIsYUVFormat)
    {
      // Create sampler with YCbCr conversion
      samplerCreateInfo = vk::SamplerCreateInfo{}
                            .setPNext(static_cast<void*>(&mYcbcrConversionInfo))
                            .setMagFilter(vk::Filter::eLinear)
                            .setMinFilter(vk::Filter::eLinear)
                            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                            .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
                            .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                            .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                            .setMipLodBias(0.0f)
                            .setAnisotropyEnable(false)
                            .setMaxAnisotropy(1.0f)
                            .setCompareEnable(false)
                            .setCompareOp(vk::CompareOp::eLessOrEqual)
                            .setMinLod(0.0f)
                            .setMaxLod(1.0f)
                            .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
                            .setUnnormalizedCoordinates(false);
    }
    else
    {
      const auto& properties = mDevice.GetPhysicalDeviceProperties();

      // Create regular sampler for RGBA formats
      samplerCreateInfo = vk::SamplerCreateInfo{}
                            .setMagFilter(vk::Filter::eLinear)
                            .setMinFilter(vk::Filter::eLinear)
                            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                            .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
                            .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                            .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                            .setAnisotropyEnable(false)
                            .setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
                            .setCompareEnable(false)
                            .setCompareOp(vk::CompareOp::eAlways)
                            .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
                            .setUnnormalizedCoordinates(false);
    }

    mSampler = SamplerImpl::New(mDevice, samplerCreateInfo);

    return true;
  }
  catch(const std::system_error& e)
  {
    DALI_LOG_ERROR("Failed to create sampler: %s\n", e.what());
    return false;
  }
}

void Texture::PrepareTexture()
{
  DALI_LOG_ERROR("Texture::PrepareTexture: mIsNativeImage: %d\n", mIsNativeImage);

  if(mIsNativeImage && mNativeImageHandler && mNativeImageType == NativeImageType::NATIVE_IMAGE_SOURCE_QUEUE)
  {
    // Store current surface before calling PrepareTexture
    void* oldSurface = mCurrentSurface;

    // Call the native image's PrepareTexture
    auto result = mCreateInfo.nativeImagePtr->PrepareTexture();

    if(result == Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED)
    {
      // Release reference to old surface before reinitializing
      if(oldSurface && mHasSurfaceReference)
      {
        ReleaseCurrentSurfaceReference();
      }

      // Reinitialize with new surface
      bool initialized = Initialize();

      if(initialized)
      {
        DALI_LOG_ERROR("Texture::PrepareTexture: Successfully reinitialized with new surface\n");
      }
      else
      {
        DALI_LOG_ERROR("Texture::PrepareTexture: Failed to reinitialize with new surface\n");
      }
    }
  }
}

void Texture::ReleaseSurfaceBufferObjectReferences()
{
  for(auto bo : mTbmBos)
  {
    if(bo)
    {
      tbm_bo_unref(static_cast<tbm_bo>(bo));
      DALI_LOG_ERROR("Texture::ReleaseSurfaceBufferObjectReferences: Released BO %p\n", bo);
    }
  }

  mTbmBos.clear();
}

SurfaceReferenceManager* Texture::GetSurfaceReferenceManager() const
{
  if(!mCreateInfo.nativeImagePtr)
  {
    return nullptr;
  }

  auto extension = mCreateInfo.nativeImagePtr->GetExtension();

  if(!extension)
  {
    // This native image type does not support extensions.
    return nullptr;
  }

  return reinterpret_cast<SurfaceReferenceManager*>(extension);
}

void Texture::AcquireCurrentSurfaceReference()
{
  if(mCurrentSurface && mHasSurfaceReference)
  {
    // Already have reference
    return;
  }

  if(!mTbmSurface)
  {
    return;
  }

  auto surfaceRefManager = GetSurfaceReferenceManager();
  if(surfaceRefManager)
  {
    surfaceRefManager->AcquireSurfaceReference(mTbmSurface);
    mCurrentSurface      = mTbmSurface;
    mHasSurfaceReference = true;

    DALI_LOG_ERROR("Texture::AcquireCurrentSurfaceReference: Acquired reference to surface %p\n", mTbmSurface);
  }
}

void Texture::ReleaseCurrentSurfaceReference()
{
  if(!mCurrentSurface || !mHasSurfaceReference)
  {
    return;
  }

  auto surfaceRefManager = GetSurfaceReferenceManager();
  if(surfaceRefManager)
  {
    surfaceRefManager->ReleaseSurfaceReference(mCurrentSurface);

    DALI_LOG_ERROR("Texture::ReleaseCurrentSurfaceReference: Released reference to surface %p\n", mCurrentSurface);
  }

  mCurrentSurface      = nullptr;
  mHasSurfaceReference = false;
}

} // namespace Dali::Graphics::Vulkan
