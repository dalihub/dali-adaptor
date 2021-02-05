#ifndef DALI_GRAPHICS_GLES_TYPES_H
#define DALI_GRAPHICS_GLES_TYPES_H

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

namespace Dali::Graphics::GLES
{
// Conversion functions
/**
 * Stucture delivers format and type that can be used
 * when creating GL texture
 */
struct GLTextureFormatType
{
  /**
   * Constuctor from Graphics::Format
   * @param value
   */
  constexpr GLTextureFormatType(Graphics::Format value)
  {
    // lookup table
    switch(value)
    {
      case Graphics::Format::UNDEFINED:
      {
        Assign(0, 0);
        break;
      }

      // Luminance formats
      case Graphics::Format::L8:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::L8A8:
      {
        Assign(0, 0);
        break;
      }

      // Packed formats (TODO: find supported)
      case Graphics::Format::R4G4_UNORM_PACK8:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R4G4B4A4_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B4G4R4A4_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R5G6B5_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B5G6R5_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R5G5B5A1_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B5G5R5A1_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A1R5G5B5_UNORM_PACK16:
      {
        Assign(0, 0);
        break;
      }

      // Formats
      case Graphics::Format::R8_UNORM:
      {
        Assign(GL_RED, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8_SNORM:
      {
        Assign(GL_RED, GL_BYTE);
        break;
      }
      case Graphics::Format::R8_USCALED:
      {
        Assign(GL_RED, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8_SSCALED:
      {
        Assign(GL_RED, GL_BYTE);
        break;
      }
      case Graphics::Format::R8_UINT:
      {
        Assign(GL_RED, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8_SINT:
      {
        Assign(GL_RED, GL_BYTE);
        break;
      }
      case Graphics::Format::R8_SRGB:
      {
        Assign(GL_RED, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8_UNORM:
      {
        Assign(GL_RG, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8_SNORM:
      {
        Assign(GL_RG, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8_USCALED:
      {
        Assign(GL_RG, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8_SSCALED:
      {
        Assign(GL_RG, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8_UINT:
      {
        Assign(GL_RG, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8_SINT:
      {
        Assign(GL_RG, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8_SRGB:
      {
        Assign(GL_RG, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_UNORM:
      {
        Assign(GL_RGB, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_SNORM:
      {
        Assign(GL_RGB, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_USCALED:
      {
        Assign(GL_RGB, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_SSCALED:
      {
        Assign(GL_RGB, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_UINT:
      {
        Assign(GL_RGB, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_SINT:
      {
        Assign(GL_RGB, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8_SRGB:
      {
        Assign(GL_RGB, GL_BYTE);
        break;
      }

      // BGR not supported in GLES
      case Graphics::Format::B8G8R8_UNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B8G8R8_SNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B8G8R8_USCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B8G8R8_SSCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B8G8R8_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B8G8R8_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B8G8R8_SRGB:
      {
        Assign(0, 0);
        break;
      }

      // RGBA
      case Graphics::Format::R8G8B8A8_UNORM:
      {
        Assign(GL_RGBA, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8A8_SNORM:
      {
        Assign(GL_RGBA, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8A8_USCALED:
      {
        Assign(GL_RGBA, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8A8_SSCALED:
      {
        Assign(GL_RGBA, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8A8_UINT:
      {
        Assign(GL_RGBA, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8A8_SINT:
      {
        Assign(GL_RGBA, GL_BYTE);
        break;
      }
      case Graphics::Format::R8G8B8A8_SRGB:
      {
        Assign(GL_RGBA, GL_BYTE);
        break;
      }

      // BGRA (seems to be not supported)
      case Graphics::Format::B8G8R8A8_UNORM:
      {
        Assign(0, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::B8G8R8A8_SNORM:
      {
        Assign(0, GL_BYTE);
        break;
      }
      case Graphics::Format::B8G8R8A8_USCALED:
      {
        Assign(0, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::B8G8R8A8_SSCALED:
      {
        Assign(0, GL_BYTE);
        break;
      }
      case Graphics::Format::B8G8R8A8_UINT:
      {
        Assign(0, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::B8G8R8A8_SINT:
      {
        Assign(0, GL_BYTE);
        break;
      }
      case Graphics::Format::B8G8R8A8_SRGB:
      {
        Assign(0, GL_BYTE);
        break;
      }

      // ABGR not supported
      case Graphics::Format::A8B8G8R8_UNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A8B8G8R8_SNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A8B8G8R8_USCALED_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A8B8G8R8_SSCALED_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A8B8G8R8_UINT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A8B8G8R8_SINT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A8B8G8R8_SRGB_PACK32:
      {
        Assign(0, 0);
        break;
      }
      // TBD which of the formats are supported
      case Graphics::Format::A2R10G10B10_UNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2R10G10B10_SNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2R10G10B10_USCALED_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2R10G10B10_SSCALED_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2R10G10B10_UINT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2R10G10B10_SINT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2B10G10R10_UNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2B10G10R10_SNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2B10G10R10_USCALED_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2B10G10R10_SSCALED_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2B10G10R10_UINT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::A2B10G10R10_SINT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_UNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_SNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_USCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_SSCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_UNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_SNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_USCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_SSCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_UNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_SNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_USCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_SSCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_UNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_SNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_USCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_SSCALED:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R16G16B16A16_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32B32_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32B32_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32B32_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32B32A32_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32B32A32_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R32G32B32A32_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64B64_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64B64_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64B64_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64B64A64_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64B64A64_SINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::R64G64B64A64_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::B10G11R11_UFLOAT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::E5B9G9R9_UFLOAT_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::D16_UNORM:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::X8_D24_UNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::D32_SFLOAT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::S8_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::D16_UNORM_S8_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::D24_UNORM_S8_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::D32_SFLOAT_S8_UINT:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC1_RGB_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC1_RGB_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC1_RGBA_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC1_RGBA_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC2_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC2_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC3_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC3_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC4_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC4_SNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC5_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC5_SNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC6H_UFLOAT_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC6H_SFLOAT_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC7_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::BC7_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8A8_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8A8_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::EAC_R11_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::EAC_R11_SNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::EAC_R11G11_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::EAC_R11G11_SNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_4x4_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_4x4_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_5x4_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_5x4_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_5x5_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_5x5_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_6x5_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_6x5_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_6x6_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_6x6_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_8x5_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_8x5_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_8x6_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_8x6_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_8x8_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_8x8_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x5_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x5_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x6_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x6_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x8_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x8_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x10_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_10x10_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_12x10_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_12x10_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_12x12_UNORM_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::ASTC_12x12_SRGB_BLOCK:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
    }
  }

  constexpr inline void Assign(uint32_t f, uint32_t t)
  {
    format = f;
    type   = t;
  }

  uint32_t format{0};
  uint32_t type{0};
};

struct GLSamplerFilterAndMipMapMode
{
  GLSamplerFilterAndMipMapMode() = default;
  GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter     filter,
                               Graphics::SamplerMipmapMode mipMapMode)
  {
    switch(filter)
    {
      case Graphics::SamplerFilter::NEAREST:
      {
        switch(mipMapMode)
        {
          case Graphics::SamplerMipmapMode::NONE:
          {
            glFilter = GL_NEAREST;
            break;
          }
          case Graphics::SamplerMipmapMode::NEAREST:
          {
            glFilter = GL_NEAREST_MIPMAP_NEAREST;
            break;
          }
          case Graphics::SamplerMipmapMode::LINEAR:
          {
            glFilter = GL_NEAREST_MIPMAP_NEAREST;
            break;
          }
        }
      }
      case Graphics::SamplerFilter::LINEAR:
      {
        switch(mipMapMode)
        {
          case Graphics::SamplerMipmapMode::NONE:
          {
            glFilter = GL_LINEAR;
            break;
          }
          case Graphics::SamplerMipmapMode::NEAREST:
          {
            glFilter = GL_LINEAR_MIPMAP_NEAREST;
            break;
          }
          case Graphics::SamplerMipmapMode::LINEAR:
          {
            glFilter = GL_LINEAR_MIPMAP_NEAREST;
            break;
          }
        }
      }
    }
  }

  inline explicit operator uint32_t() const
  {
    return glFilter;
  }

  inline operator int() const
  {
    return static_cast<int>(glFilter);
  }

  uint32_t glFilter{0};
};

} // namespace Dali::Graphics::GLES

#endif //DALI_GRAPHICS_API_TYPES_H
