#ifndef DALI_GRAPHICS_GLES_TYPES_H
#define DALI_GRAPHICS_GLES_TYPES_H

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

namespace Dali::Graphics::GLES
{
class Buffer;
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
        Assign(GL_LUMINANCE, GL_UNSIGNED_BYTE);
        break;
      }
      case Graphics::Format::L8A8:
      {
        Assign(GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE);
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

/** Converts vertex format to GL */
struct GLVertexFormat
{
  constexpr explicit GLVertexFormat(Graphics::VertexInputFormat gfxFormat)
  {
    switch(gfxFormat)
    {
      case VertexInputFormat::FVECTOR2:
      {
        format = GL_FLOAT;
        size   = 2;
        break;
      }
      case VertexInputFormat::FVECTOR3:
      {
        format = GL_FLOAT;
        size   = 3;
        break;
      }
      case VertexInputFormat::FVECTOR4:
      {
        format = GL_FLOAT;
        size   = 4;
        break;
      }
      case VertexInputFormat::FLOAT:
      {
        format = GL_FLOAT;
        size   = 1;
        break;
      }
      case VertexInputFormat::INTEGER:
      {
        format = GL_INT;
        size   = 1;
        break;
      }
      case VertexInputFormat::IVECTOR2:
      {
        format = GL_INT;
        size   = 2;
        break;
      }
      case VertexInputFormat::IVECTOR3:
      {
        format = GL_INT;
        size   = 3;
        break;
      }
      case VertexInputFormat::IVECTOR4:
      {
        format = GL_INT;
        size   = 4;
        break;
      }
      case VertexInputFormat::UNDEFINED:
      {
        format = 0;
        size   = 0;
        break;
      }
    }
  }

  GLenum   format{0u};
  uint32_t size{0u};
};

/**
 * @brief Descriptor of single buffer binding within
 * command buffer.
 */
struct VertexBufferBindingDescriptor
{
  const GLES::Buffer* buffer{nullptr};
  uint32_t            offset{0u};
};

/**
 * @brief Descriptor of ix buffer binding within
 * command buffer.
 */
struct IndexBufferBindingDescriptor
{
  const GLES::Buffer* buffer{nullptr};
  uint32_t            offset{};
  Graphics::Format    format{};
};

/**
 * @brief The descriptor of draw call
 */
struct DrawCallDescriptor
{
  /**
   * @brief Enum specifying type of the draw call
   */
  enum class Type
  {
    DRAW,
    DRAW_INDEXED,
    DRAW_INDEXED_INDIRECT
  };

  Type type{}; ///< Type of the draw call

  /**
   * Union contains data for all types of draw calls.
   */
  union
  {
    /**
     * @brief Vertex array draw
     */
    struct
    {
      uint32_t vertexCount;
      uint32_t instanceCount;
      uint32_t firstVertex;
      uint32_t firstInstance;
    } draw;

    /**
     * @brief Indexed draw
     */
    struct
    {
      uint32_t indexCount;
      uint32_t instanceCount;
      uint32_t firstIndex;
      int32_t  vertexOffset;
      uint32_t firstInstance;
    } drawIndexed;

    /**
     * @brief Indexed draw indirect
     */
    struct
    {
      const GLES::Buffer* buffer;
      uint32_t            offset;
      uint32_t            drawCount;
      uint32_t            stride;
    } drawIndexedIndirect;
  };
};

/**
 * @brief Topologu conversion from Graphics to GLES
 */
struct GLESTopology
{
  explicit constexpr GLESTopology(PrimitiveTopology topology)
  {
    switch(topology)
    {
      case PrimitiveTopology::POINT_LIST:
      {
        primitiveTopology = GL_POINTS;
        break;
      }
      case PrimitiveTopology::LINE_LIST:
      {
        primitiveTopology = GL_LINES;
        break;
      }
      case PrimitiveTopology::LINE_LOOP:
      {
        primitiveTopology = GL_LINE_LOOP;
        break;
      }
      case PrimitiveTopology::LINE_STRIP:
      {
        primitiveTopology = GL_LINE_STRIP;
        break;
      }
      case PrimitiveTopology::TRIANGLE_LIST:
      {
        primitiveTopology = GL_TRIANGLES;
        break;
      }
      case PrimitiveTopology::TRIANGLE_STRIP:
      {
        primitiveTopology = GL_TRIANGLE_STRIP;
        break;
      }
      case PrimitiveTopology::TRIANGLE_FAN:
      {
        primitiveTopology = GL_TRIANGLE_FAN;
        break;
      }
    }
  }

  /**
   * @brief Explicit type conversion operator
   * @return converted value
   */
  constexpr inline operator GLenum() const
  {
    return primitiveTopology;
  }

  GLenum primitiveTopology{0}; ///< Topology
};

/**
 * @brief Index format conversion structure
 */
struct GLIndexFormat
{
  explicit constexpr GLIndexFormat(Format _format)
  {
    switch(_format)
    {
      // TODO: add more formats
      case Format::R16_UINT:
      {
        format = GL_UNSIGNED_SHORT;
        break;
      }
      default:
      {
        format = 0;
      }
    }
  }

  /**
   * @brief Explicit type conversion operator
   * @return converted value
   */
  constexpr inline operator GLenum() const
  {
    return format;
  }

  GLenum format{0}; ///< Converted format
};

/**
 * @brief Conversion of blending function factor
 */
struct GLBlendFunc
{
  constexpr explicit GLBlendFunc(Graphics::BlendFactor factor)
  {
    switch(factor)
    {
      case Graphics::BlendFactor::ZERO:
      {
        glFactor = GL_ZERO;
        break;
      }
      case Graphics::BlendFactor::ONE:
      {
        glFactor = GL_ONE;
        break;
      }
      case Graphics::BlendFactor::SRC_COLOR:
      {
        glFactor = GL_SRC_COLOR;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_SRC_COLOR:
      {
        glFactor = GL_ONE_MINUS_SRC_COLOR;
        break;
      }
      case Graphics::BlendFactor::DST_COLOR:
      {
        glFactor = GL_DST_COLOR;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_DST_COLOR:
      {
        glFactor = GL_ONE_MINUS_DST_COLOR;
        break;
      }
      case Graphics::BlendFactor::SRC_ALPHA:
      {
        glFactor = GL_SRC_ALPHA;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_SRC_ALPHA:
      {
        glFactor = GL_ONE_MINUS_SRC_ALPHA;
        break;
      }
      case Graphics::BlendFactor::DST_ALPHA:
      {
        glFactor = GL_DST_ALPHA;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_DST_ALPHA:
      {
        glFactor = GL_ONE_MINUS_DST_ALPHA;
        break;
      }
      case Graphics::BlendFactor::CONSTANT_COLOR:
      {
        glFactor = GL_CONSTANT_COLOR;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_CONSTANT_COLOR:
      {
        glFactor = GL_ONE_MINUS_CONSTANT_COLOR;
        break;
      }
      case Graphics::BlendFactor::CONSTANT_ALPHA:
      {
        glFactor = GL_CONSTANT_ALPHA;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
      {
        glFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
        break;
      }
      case Graphics::BlendFactor::SRC_ALPHA_SATURATE:
      {
        glFactor = GL_SRC_ALPHA_SATURATE;
        break;
      }
      // Below may be unsupported without extension
      case Graphics::BlendFactor::SRC1_COLOR:
      {
        glFactor = 0u;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_SRC1_COLOR:
      {
        glFactor = 0u;
        break;
      }
      case Graphics::BlendFactor::SRC1_ALPHA:
      {
        glFactor = 0u;
        break;
      }
      case Graphics::BlendFactor::ONE_MINUS_SRC1_ALPHA:
      {
        glFactor = 0u;
        break;
      }
    }
  }

  /**
   * @brief Explicit type conversion operator
   * @return converted value
   */
  constexpr inline operator GLenum() const
  {
    return glFactor;
  }

  GLenum glFactor{0u};
};

/**
 * @brief Converts Blend Op to GL
 */
struct GLBlendOp
{
  constexpr explicit GLBlendOp(Graphics::BlendOp blendOp)
  {
    switch(blendOp)
    {
      case Graphics::BlendOp::ADD:
      {
        glBlendOp = GL_FUNC_ADD;
        break;
      }
      case Graphics::BlendOp::SUBTRACT:
      {
        glBlendOp = GL_FUNC_SUBTRACT;
        break;
      }
      case Graphics::BlendOp::REVERSE_SUBTRACT:
      {
        glBlendOp = GL_FUNC_REVERSE_SUBTRACT;
        break;
      }
      case Graphics::BlendOp::MIN:
      {
        glBlendOp = GL_MIN;
        break;
      }
      case Graphics::BlendOp::MAX:
      {
        glBlendOp = GL_MAX;
        break;
      }
    }
  }

  /**
   * @brief Explicit type conversion operator
   * @return converted value
   */
  constexpr inline operator GLenum() const
  {
    return glBlendOp;
  }

  GLenum glBlendOp{0u};
};

/**
 * @brief Converts GL cull mode
 */
struct GLCullMode
{
  constexpr explicit GLCullMode(Graphics::CullMode cullMode)
  {
    switch(cullMode)
    {
      case Graphics::CullMode::NONE: // this isn't really accepted by GL!
      {
        glCullMode = GL_NONE;
        break;
      }
      case Graphics::CullMode::FRONT:
      {
        glCullMode = GL_FRONT;
        break;
      }
      case Graphics::CullMode::BACK:
      {
        glCullMode = GL_BACK;
        break;
      }
      case Graphics::CullMode::FRONT_AND_BACK:
      {
        glCullMode = GL_FRONT_AND_BACK;
        break;
      }
    }
  }

  /**
   * @brief Explicit type conversion operator
   * @return converted value
   */
  constexpr inline operator GLenum() const
  {
    return glCullMode;
  }

  GLenum glCullMode{0u};
};

} // namespace Dali::Graphics::GLES

#endif //DALI_GRAPHICS_API_TYPES_H
