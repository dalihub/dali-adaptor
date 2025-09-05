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
#include <dali/integration-api/pixel-data-integ.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer-request.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-utils.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

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
      return vk::Format::eUndefined;
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

/**
 * Format conversion table
 */
static const std::vector<ColorConversion> COLOR_CONVERSION_TABLE =
  {
    {vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm, ConvertRGB32ToRGBA32, WriteRGB32ToRGBA32}};

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

  auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item) { return item.oldFormat == mConvertFromFormat; });

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

  auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item) { return item.oldFormat == mConvertFromFormat; });

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
    auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item) { return item.oldFormat == convertFromFormat; });

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
  if(!formatFlags)
  {
    auto it = std::find_if(COLOR_CONVERSION_TABLE.begin(), COLOR_CONVERSION_TABLE.end(), [&](auto& item) { return item.oldFormat == sourceFormat; });

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

Texture::Texture(const Dali::Graphics::TextureCreateInfo& createInfo, VulkanGraphicsController& controller)
: Resource(createInfo, controller),
  mDevice(controller.GetGraphicsDevice()),
  mImage(),
  mImageView()
{
  // Check env variable in order to enable staging buffers
  auto var = getenv("DALI_DISABLE_TEXTURE_STAGING_BUFFERS");
  if(var && var[0] != '0')
  {
    mDisableStagingBuffer = true;
    mTiling               = TextureTiling::LINEAR;
  }
}

Texture::~Texture()
{
  delete mImageView;
  delete mImage;
}

bool Texture::InitializeResource()
{
  SetFormatAndUsage();

  if(mFormat == vk::Format::eUndefined)
  {
    // not supported!
    return false;
  }

  if(InitializeTexture())
  {
    // force generating properties
    GetProperties();

    return true;
  }

  return false;
}

void Texture::DestroyResource()
{
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

void Texture::SetFormatAndUsage()
{
  auto size = mCreateInfo.size;
  mWidth    = uint32_t(size.width);
  mHeight   = uint32_t(size.height);
  mLayout   = vk::ImageLayout::eUndefined;

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

  auto format = ConvertApiToVk(mCreateInfo.format);

  mFormat            = ValidateFormat(format);
  mConvertFromFormat = vk::Format::eUndefined;

  if(format != mFormat)
  {
    mConvertFromFormat = format;
  }
  mComponentMapping = GetVkComponentMapping(mCreateInfo.format);
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
                           .setMipLevels(1);

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
  return mController.GetDefaultSampler();
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

} // namespace Dali::Graphics::Vulkan
