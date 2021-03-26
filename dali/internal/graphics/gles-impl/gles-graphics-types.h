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
        Assign(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
        break;
      }
      case Graphics::Format::B4G4R4A4_UNORM_PACK16:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_UNSIGNED_SHORT_4_4_4_4);
#else
        Assign(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
#endif
        break;
      }
      case Graphics::Format::R5G6B5_UNORM_PACK16:
      {
        Assign(GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
        break;
      }
      case Graphics::Format::B5G6R5_UNORM_PACK16:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_UNSIGNED_SHORT_5_6_5);
#else
        Assign(GL_RGBA, GL_UNSIGNED_SHORT_5_6_5);
#endif
        break;
      }
      case Graphics::Format::R5G5B5A1_UNORM_PACK16:
      {
        Assign(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
        break;
      }
      case Graphics::Format::B5G5R5A1_UNORM_PACK16:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_UNSIGNED_SHORT_5_5_5_1);
#else
        Assign(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
#endif
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

      // BGRA when only support GL_BGRA_EXT. otherwise, use RGBA
      case Graphics::Format::B8G8R8A8_UNORM:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_UNSIGNED_BYTE);
#else
        Assign(GL_RGBA, GL_UNSIGNED_BYTE);
#endif
        break;
      }
      case Graphics::Format::B8G8R8A8_SNORM:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_BYTE);
#else
        Assign(GL_RGBA, GL_BYTE);
#endif
        break;
      }
      case Graphics::Format::B8G8R8A8_USCALED:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_UNSIGNED_BYTE);
#else
        Assign(GL_RGBA, GL_UNSIGNED_BYTE);
#endif
        break;
      }
      case Graphics::Format::B8G8R8A8_SSCALED:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_BYTE);
#else
        Assign(GL_RGBA, GL_BYTE);
#endif
        break;
      }
      case Graphics::Format::B8G8R8A8_UINT:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_UNSIGNED_BYTE);
#else
        Assign(GL_RGBA, GL_UNSIGNED_BYTE);
#endif
        break;
      }
      case Graphics::Format::B8G8R8A8_SINT:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_BYTE);
#else
        Assign(GL_RGBA, GL_BYTE);
#endif
        break;
      }
      case Graphics::Format::B8G8R8A8_SRGB:
      {
#ifdef GL_BGRA_EXT
        Assign(GL_BGRA_EXT, GL_BYTE);
#else
        Assign(GL_RGBA, GL_BYTE);
#endif
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
        // GLES 3.0 floating point formats.
        Assign(GL_RGB, GL_HALF_FLOAT);
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
        // GLES 3.0 floating point formats.
        Assign(GL_RGB, GL_FLOAT);
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
        // GLES 3.0 depth and stencil formats
        Assign(GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
        break;
      }
      case Graphics::Format::X8_D24_UNORM_PACK32:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::D32_SFLOAT:
      {
        // GLES 3.0 depth and stencil formats
        Assign(GL_DEPTH_COMPONENT, GL_FLOAT);
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
        // GLES 3.0 depth and stencil formats
        Assign(GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
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
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_RGB8_ETC2.
        Assign(GL_COMPRESSED_RGB8_ETC2, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8_SRGB_BLOCK:
      {
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_SRGB8_ETC2.
        Assign(GL_COMPRESSED_SRGB8_ETC2, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      {
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2.
        Assign(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0);
        break;
      }
      case Graphics::Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      {
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2.
        Assign(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0);
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
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_R11_EAC.
        Assign(GL_COMPRESSED_R11_EAC, 0);
        break;
      }
      case Graphics::Format::EAC_R11_SNORM_BLOCK:
      {
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_SIGNED_R11_EAC.
        Assign(GL_COMPRESSED_SIGNED_R11_EAC, 0);
        break;
      }
      case Graphics::Format::EAC_R11G11_UNORM_BLOCK:
      {
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_RG11_EAC.
        Assign(GL_COMPRESSED_RG11_EAC, 0);
        break;
      }
      case Graphics::Format::EAC_R11G11_SNORM_BLOCK:
      {
        ///! Using GLES 3.0 standard compressed pixel format COMPRESSED_SIGNED_RG11_EAC.
        Assign(GL_COMPRESSED_SIGNED_RG11_EAC, 0);
        break;
      }
      case Graphics::Format::ASTC_4x4_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_4x4_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_4x4_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_5x4_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_5x4_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_5x4_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_5x5_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_5x5_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_5x5_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_6x5_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_6x5_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_6x5_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_6x6_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_6x6_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_6x6_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_8x5_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_8x5_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_8x5_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_8x6_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_8x6_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_8x6_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_8x8_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_8x8_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_8x8_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x5_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_10x5_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x5_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x6_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_10x6_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x6_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x8_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_10x8_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x8_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x10_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_10x10_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_10x10_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_12x10_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_12x10_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_12x10_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_12x12_UNORM_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_RGBA_ASTC_12x12_KHR.
        Assign(GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 0);
        break;
      }
      case Graphics::Format::ASTC_12x12_SRGB_BLOCK:
      {
        ///! Using GLES 3.1 standard compressed pixel format COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR.
        Assign(GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 0);
        break;
      }
      case Graphics::Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      {
        Assign(0, 0);
        break;
      }
      case Graphics::Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      {
        ///! Using non-standard GLES 2.0 extension compressed pixel format COMPRESSED_RGB_PVRTC_4BPPV1.
        Assign(0x8C00, 0); ///! < Hardcoded so we can test before we move to GLES 3.0 or greater.

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

struct GLSamplerFilter
{
  constexpr explicit GLSamplerFilter(Graphics::SamplerFilter filter)
  {
    switch(filter)
    {
      case Graphics::SamplerFilter::NEAREST:
      {
        glFilter = GL_NEAREST;
        break;
      }
      case Graphics::SamplerFilter::LINEAR:
      {
        glFilter = GL_LINEAR;
        break;
      }
    }
  }
  uint32_t glFilter{0};
};

struct GLSamplerFilterAndMipMapMode
{
  constexpr explicit GLSamplerFilterAndMipMapMode(Graphics::SamplerFilter     filter,
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
            glFilter = GL_NEAREST_MIPMAP_LINEAR;
            break;
          }
        }
        break;
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
            glFilter = GL_LINEAR_MIPMAP_LINEAR;
            break;
          }
        }
        break;
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

struct GLTextureTarget
{
  constexpr explicit GLTextureTarget(Graphics::TextureType graphicsTextureType)
  {
    switch(graphicsTextureType)
    {
      case Graphics::TextureType::TEXTURE_2D:
      {
        target = GL_TEXTURE_2D;
        break;
      }
      case Graphics::TextureType::TEXTURE_3D:
      {
        target = GL_TEXTURE_3D;
        break;
      }
      case Graphics::TextureType::TEXTURE_CUBEMAP:
      {
        target = GL_TEXTURE_CUBE_MAP;
        break;
      }
    }
  }
  GLenum target{GL_TEXTURE_2D};
};

struct GLAddressMode
{
  constexpr explicit GLAddressMode(Graphics::SamplerAddressMode graphicsAddressMode)
  {
    switch(graphicsAddressMode)
    {
      case Graphics::SamplerAddressMode::REPEAT:
      {
        texParameter = GL_REPEAT;
        break;
      }
      case Graphics::SamplerAddressMode::MIRRORED_REPEAT:
      {
        texParameter = GL_MIRRORED_REPEAT;
        break;
      }
      case Graphics::SamplerAddressMode::CLAMP_TO_EDGE:
      {
        texParameter = GL_CLAMP_TO_EDGE;
        break;
      }
      case Graphics::SamplerAddressMode::CLAMP_TO_BORDER:
      {
        texParameter = GL_CLAMP_TO_EDGE;
        break;
      }
      case Graphics::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
      {
        texParameter = GL_CLAMP_TO_EDGE;
        break;
      }
    }
  }
  GLenum texParameter{GL_CLAMP_TO_EDGE};
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
 * @brief Descriptor of uniform buffer binding within
 * command buffer.
 */
struct UniformBufferBindingDescriptor
{
  const GLES::Buffer* buffer{nullptr};
  uint32_t            binding{0u};
  uint32_t            offset{0u};
  bool                emulated; ///<true if UBO is emulated for old gfx API
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

/**
 * @brief enum with GL types
 */
enum class GLType
{
  UNDEFINED      = 0x0,
  FLOAT_VEC2     = 0x8B50,
  FLOAT_VEC3     = 0x8B51,
  FLOAT_VEC4     = 0x8B52,
  INT_VEC2       = 0x8B53,
  INT_VEC3       = 0x8B54,
  INT_VEC4       = 0x8B55,
  BOOL           = 0x8B56,
  BOOL_VEC2      = 0x8B57,
  BOOL_VEC3      = 0x8B58,
  BOOL_VEC4      = 0x8B59,
  FLOAT_MAT2     = 0x8B5A,
  FLOAT_MAT3     = 0x8B5B,
  FLOAT_MAT4     = 0x8B5C,
  SAMPLER_2D     = 0x8B5E,
  SAMPLER_CUBE   = 0x8B60,
  BYTE           = 0x1400,
  UNSIGNED_BYTE  = 0x1401,
  SHORT          = 0x1402,
  UNSIGNED_SHORT = 0x1403,
  INT            = 0x1404,
  UNSIGNED_INT   = 0x1405,
  FLOAT          = 0x1406,
  FIXED          = 0x140C,
};

/**
 * @brief GL type conversion (used with reflection)
 */
struct GLTypeConversion
{
  constexpr explicit GLTypeConversion(GLenum value)
  {
    switch(value)
    {
      case GL_FLOAT_VEC2:
      {
        type = GLType::FLOAT_VEC2;
        break;
      }
      case GL_FLOAT_VEC3:
      {
        type = GLType::FLOAT_VEC3;
        break;
      }
      case GL_FLOAT_VEC4:
      {
        type = GLType::FLOAT_VEC4;
        break;
      }
      case GL_INT_VEC2:
      {
        type = GLType::INT_VEC2;
        break;
      }
      case GL_INT_VEC3:
      {
        type = GLType::INT_VEC3;
        break;
      }
      case GL_INT_VEC4:
      {
        type = GLType::INT_VEC4;
        break;
      }
      case GL_BOOL:
      {
        type = GLType::BOOL;
        break;
      }
      case GL_BOOL_VEC2:
      {
        type = GLType::BOOL_VEC2;
        break;
      }
      case GL_BOOL_VEC3:
      {
        type = GLType::BOOL_VEC3;
        break;
      }
      case GL_BOOL_VEC4:
      {
        type = GLType::BOOL_VEC4;
        break;
      }
      case GL_FLOAT_MAT2:
      {
        type = GLType::FLOAT_MAT2;
        break;
      }
      case GL_FLOAT_MAT3:
      {
        type = GLType::FLOAT_MAT3;
        break;
      }
      case GL_FLOAT_MAT4:
      {
        type = GLType::FLOAT_MAT4;
        break;
      }
      case GL_SAMPLER_2D:
      {
        type = GLType::SAMPLER_2D;
        break;
      }
      case GL_SAMPLER_CUBE:
      {
        type = GLType::SAMPLER_CUBE;
        break;
      }
      case GL_FLOAT:
      {
        type = GLType::FLOAT;
        break;
      }
      default:
      {
        type = GLType::UNDEFINED;
      }
    }
  }

  GLType type{GLType::UNDEFINED};
};

enum class GLESVersion
{
  GLES_20 = 20,
  GLES_30 = 30,
  GLES_31 = 31,
  GLES_32 = 32
};

} // namespace Dali::Graphics::GLES

#endif //DALI_GRAPHICS_API_TYPES_H
