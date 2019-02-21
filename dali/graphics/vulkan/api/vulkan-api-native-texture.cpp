/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#ifdef NATIVE_IMAGE_SUPPORT
#include <unistd.h>
#include <errno.h>
#include <tbm_type_common.h>
#include <tbm_surface.h>
#include <tbm_bo.h>
#include <tbm_surface_internal.h>
#include <vulkan/vulkan.h>

#ifdef EXPORT_API
#undef EXPORT_API
#endif

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

#endif
#include <dali/graphics/vulkan/api/vulkan-api-native-texture.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/graphics/vulkan/vulkan-graphics.h>
#include <dali/graphics/vulkan/internal/vulkan-buffer.h>
#include <dali/graphics/vulkan/internal/vulkan-command-buffer.h>
#include <dali/graphics/vulkan/internal/vulkan-command-pool.h>
#include <dali/graphics/vulkan/internal/vulkan-image.h>
#include <dali/graphics/vulkan/internal/vulkan-image-view.h>
#include <dali/graphics/vulkan/internal/vulkan-fence.h>
#include <dali/graphics/vulkan/internal/vulkan-sampler.h>
#include <dali/graphics/vulkan/internal/vulkan-utils.h>
#include <dali/graphics/vulkan/api/vulkan-api-controller.h>
#include <dali/graphics/vulkan/api/vulkan-api-buffer.h>
#include <dali/graphics/vulkan/api/vulkan-api-texture-factory.h>
#include <dali/devel-api/images/native-image-interface-extension.h>

#include <algorithm>

namespace Dali
{
namespace Graphics
{
using Vulkan::VkAssert;
namespace VulkanAPI
{
using namespace Dali::Graphics::Vulkan;

namespace
{
// @todo Move to a derived class as a member variable?
#ifdef NATIVE_IMAGE_SUPPORT
PFN_vkCreateSamplerYcbcrConversionKHR        gCreateSamplerYcbcrConversionKHR = 0;       // device
PFN_vkGetPhysicalDeviceFormatProperties2KHR  gGetPhysicalDeviceFormatProperties2KHR = 0; // instance
PFN_vkDestroySamplerYcbcrConversionKHR       gDestroySamplerYcbcrConversionKHR = 0;      // device
#endif

} // anonymous namespace

NativeTexture::NativeTexture( Dali::Graphics::TextureFactory& factory )
  : Texture( factory ),
    mSamplerIsImmutable( false ),
    mNativeImage{},
    mIsSupportNativeImage{ false }
#ifdef NATIVE_IMAGE_SUPPORT
    ,
    mYcbcrConvInfo{},
    mYcbcrConv(),
    mTbmSurface( NULL )
#endif
{
}

NativeTexture::~NativeTexture()
{
#ifdef NATIVE_IMAGE_SUPPORT
  auto device = mGraphics.GetDevice();
  auto ycbcr = mYcbcrConv;
  auto tbmSurface = reinterpret_cast<tbm_surface_h>(mTbmSurface);

  mSampler.Reset();
  mImageView.Reset();
  mImage.Reset();

  mGraphics.DiscardResource( [device, ycbcr, tbmSurface ]()
                             {
                               if( ycbcr )
                               {
                                 gDestroySamplerYcbcrConversionKHR(static_cast<VkDevice>(device), ycbcr, nullptr);
                               }

                               if( tbmSurface )
                               {
                                 tbm_surface_internal_unref( tbmSurface );
                               }
                             });
#endif
}

bool NativeTexture::Initialise()
{
  auto size = mTextureFactory->GetSize();
  mWidth = uint32_t( size.width );
  mHeight = uint32_t( size.height );
  mLayout = vk::ImageLayout::eUndefined;
  switch( mTextureFactory->GetUsage())
  {
    case Dali::Graphics::TextureDetails::Usage::COLOR_ATTACHMENT:
    {
      mUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
      break;
    }
    case Dali::Graphics::TextureDetails::Usage::DEPTH_ATTACHMENT:
    {
      mUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
      break;
    }
    case Dali::Graphics::TextureDetails::Usage::SAMPLE:
    {
      mUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
      break;
    }
  }

  mFormat = ConvertApiToVk( mTextureFactory->GetFormat() );
  mComponentMapping = GetVkComponentMapping( mTextureFactory->GetFormat() );

#ifdef NATIVE_IMAGE_SUPPORT
  NativeImageInterfacePtr nativeImage = mTextureFactory->GetNativeImage();
  if ( !gCreateSamplerYcbcrConversionKHR && !gGetPhysicalDeviceFormatProperties2KHR )
  {
    gCreateSamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversionKHR>(
      mGraphics.GetDeviceProcedureAddress( "vkCreateSamplerYcbcrConversionKHR" ) );

    gGetPhysicalDeviceFormatProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2KHR>(
      mGraphics.GetInstanceProcedureAddress( "vkGetPhysicalDeviceFormatProperties2KHR" ) );

    gDestroySamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversionKHR>(
      mGraphics.GetDeviceProcedureAddress( "vkDestroySamplerYcbcrConversionKHR" ) );
  }

  if ( gCreateSamplerYcbcrConversionKHR
       && gDestroySamplerYcbcrConversionKHR
       && gGetPhysicalDeviceFormatProperties2KHR
       && nativeImage )
  {
    mIsSupportNativeImage = true;
  }
  else
  {
    mIsSupportNativeImage = false;
  }
#endif
  bool result = false;
#ifdef NATIVE_IMAGE_SUPPORT
  if (mIsSupportNativeImage)
  {
    mUsage = vk::ImageUsageFlagBits::eSampled;
    mLayout = vk::ImageLayout::eUndefined;

    mComponentMapping.r = vk::ComponentSwizzle::eIdentity;
    mComponentMapping.g = vk::ComponentSwizzle::eIdentity;
    mComponentMapping.b = vk::ComponentSwizzle::eIdentity;
    mComponentMapping.a = vk::ComponentSwizzle::eIdentity;

    //mNativeImage = nativeImage->GetNativeImageHandle();
    NativeImageInterface::Extension* extension = nativeImage->GetExtension();
    if( extension != NULL )
    {
      mNativeImage = extension->GetNativeImageHandle();
    }

    if (InitialiseNativeImage())
    {
      CopyNativeImage( Dali::Graphics::TextureDetails::UpdateMode::IMMEDIATE );
      result = true;
    }
  }
  else
#endif
  {
    if( InitialiseTexture() )
    {
      // force generating properties
      GetProperties();
      result = true;
    }
  }

  return result;
}

void NativeTexture::CopyNativeImage( Dali::Graphics::TextureDetails::UpdateMode updateMode )
{
  ResourceTransferRequest transferRequest( TransferRequestType::USE_TBM_SURFACE );

  transferRequest.useTBMSurfaceInfo.srcImage = mImage;
  transferRequest.deferredTransferMode = !( updateMode == Dali::Graphics::TextureDetails::UpdateMode::IMMEDIATE );

  // schedule transfer
  mController.ScheduleResourceTransfer( std::move(transferRequest) );
}

bool NativeTexture::IsSamplerImmutable() const
{
  return mSamplerIsImmutable;
}


bool NativeTexture::InitialiseNativeImage()
{
#ifdef NATIVE_IMAGE_SUPPORT
  tbm_surface_h tbmSurface = 0;
  tbm_surface_info_s tbmSurface_info;

  std::vector <VkSubresourceLayout> plane_layout;
  VkDrmFormatModifierPropertiesEXT  drm_fmt_modifier;

  if ( mNativeImage.GetType() == typeid( tbm_surface_h ) )
  {
    tbmSurface =  AnyCast< tbm_surface_h >( mNativeImage );
  }

  if (!tbmSurface)
  {
    return false;
  }

  tbm_surface_internal_ref( tbmSurface );

  mTbmSurface = tbmSurface;

  tbm_surface_get_info( tbmSurface, &tbmSurface_info );

  // set format
  if ( tbmSurface_info.format == TBM_FORMAT_NV21
     || tbmSurface_info.format == TBM_FORMAT_NV12 )
  {
    // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR
    mFormat = vk::Format::eG8B8R82Plane420UnormKHR;
  }
  else if ( tbmSurface_info.format == TBM_FORMAT_RGB888
      || tbmSurface_info.format == TBM_FORMAT_XRGB8888
      || tbmSurface_info.format == TBM_FORMAT_RGBX8888
      || tbmSurface_info.format == TBM_FORMAT_ARGB8888
      || tbmSurface_info.format == TBM_FORMAT_RGBA8888)
  {
    // VK_FORMAT_R8G8B8A8_UNORM
    mFormat = vk::Format::eB8G8R8A8Unorm;
  }
  else if ( tbmSurface_info.format == TBM_FORMAT_BGR888
      || tbmSurface_info.format == TBM_FORMAT_XBGR8888
      || tbmSurface_info.format == TBM_FORMAT_BGRX8888
      || tbmSurface_info.format == TBM_FORMAT_ABGR8888
      || tbmSurface_info.format == TBM_FORMAT_BGRA8888)
  {
    // VK_FORMAT_B8G8R8A8_UNORM
    mFormat = vk::Format::eR8G8B8A8Unorm;
  }

  mLayout = vk::ImageLayout::eUndefined;

  GetFormatLinearDrmModifierNativeImage( static_cast<VkFormat>(mFormat), drm_fmt_modifier );

  plane_layout.resize ( drm_fmt_modifier.drmFormatModifierPlaneCount );

  for (uint32_t i = 0; i < tbmSurface_info.num_planes; i++)
  {
    plane_layout[i].offset = tbmSurface_info.planes[i].offset;
    plane_layout[i].size = tbmSurface_info.planes[i].size;
    plane_layout[i].rowPitch = tbmSurface_info.planes[i].stride;
    plane_layout[i].arrayPitch = 0;
    plane_layout[i].depthPitch = 0;
  }

  VkImageDrmFormatModifierExplicitCreateInfoEXT mod_create_info =
  {
    static_cast< VkStructureType >(VK_STRUCTURE_TYPE_IMAGE_EXCPLICIT_DRM_FORMAT_MODIFIER_CREATE_INFO_EXT), //VkStructureType sType;
    nullptr,                                                           //const void*                   pNext;
    DRM_FORMAT_MOD_LINEAR,                                             //uint64_t                      drmFormatModifier;
    drm_fmt_modifier.drmFormatModifierPlaneCount,                      //uint32_t                      drmFormatModifierPlaneCount;
    &plane_layout[0]                                                   //const VkSubresourceLayout*    pPlaneLayouts;
  };

  VkExternalMemoryImageCreateInfoKHR ext_mem_create_info =
  {
    static_cast< VkStructureType >(VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR), //VkStructureType  sType;
    &mod_create_info,                                                //const void*      pNext;
    VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT                   //VkExternalMemoryHandleTypeFlags    handleTypes;
  };

  auto imageCreateInfo = vk::ImageCreateInfo{}
    .setPNext( static_cast< void * >(&ext_mem_create_info) )
    .setImageType( vk::ImageType::e2D )
    .setFormat( mFormat )
    .setExtent( { mWidth, mHeight, 1 } )
    .setMipLevels( 1 )
    .setArrayLayers( 1 )
    .setSamples( vk::SampleCountFlagBits::e1 )
    .setTiling( static_cast< vk::ImageTiling >(VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) )
    .setUsage( mUsage )
    .setSharingMode( vk::SharingMode::eExclusive )
    .setQueueFamilyIndexCount( 0 )
    .setPQueueFamilyIndices( nullptr )
    .setInitialLayout( mLayout );

  mImage = mGraphics.CreateImage( imageCreateInfo );
  mImage->SetIsNativeImage( true );

  //allocate memory for the image
  uint32_t num_bos = static_cast<uint32_t>(tbm_surface_internal_get_num_bos(tbmSurface));

  VkDeviceSize import_size = 0;
  std::vector<std::unique_ptr<Memory>> tMemories;
  std::vector<vk::DeviceMemory> memories(num_bos);

  if ( num_bos > 1 )
  {
    for( uint32_t i = 0; i < num_bos; i++ )
    {
      tbm_bo bo = tbm_surface_internal_get_bo(tbmSurface, static_cast<int>(i) );
      uint32_t tbmFD = tbm_bo_get_handle(bo, TBM_DEVICE_3D).u32;
      int32_t newFD = dup(static_cast<int32_t>(tbmFD));

      if ( newFD < 0 )
      {
        char buf[512] = {0,};
        DALI_LOG_ERROR("Fail to dup(%d) for tbmFD(%d): %s", newFD, tbmFD, strerror_r(errno, buf, 512));
        return false;
      }

      import_size = static_cast<VkDeviceSize>(tbm_bo_size(bo));

      auto memory = mGraphics.AllocateMemory( mImage, vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent, static_cast<int>(newFD), import_size );
      if ( !memory )
      {
        return false;
      }
      memories[i] = memory->GetVkHandle();
      tMemories.push_back(std::move(memory));
    }

    std::vector<vk::BindImageMemoryInfo> bind_image_mem_info(tbmSurface_info.num_planes);
    std::vector<vk::BindImagePlaneMemoryInfo> bind_image_plane_mem_info(tbmSurface_info.num_planes);

    for (uint32_t i = 0; i < tbmSurface_info.num_planes; i++)
    {
      bind_image_plane_mem_info[i].setPNext( nullptr );

      bind_image_mem_info[i].setPNext( static_cast<void*>(&bind_image_plane_mem_info[i]) );
      bind_image_mem_info[i].setImage( mImage->GetVkHandle() );
      bind_image_mem_info[i].setMemory( memories[i] );

      bind_image_mem_info[i].setMemoryOffset( tbmSurface_info.planes[i].offset );
    }

    bind_image_plane_mem_info[0].setPlaneAspect( vk::ImageAspectFlagBits::ePlane0KHR );//  VK_IMAGE_ASPECT_PLANE_0_BIT_KHR;
    bind_image_plane_mem_info[1].setPlaneAspect( vk::ImageAspectFlagBits::ePlane1KHR );
    if (tbmSurface_info.num_planes > 2)
    {
      bind_image_plane_mem_info[2].setPlaneAspect( vk::ImageAspectFlagBits::ePlane2KHR );
    }

    mGraphics.BindImageMemory( mImage, bind_image_mem_info, tbmSurface_info.num_planes, tMemories );
  }
  else if ( num_bos == 1 )
  {
    std::vector<vk::BindImageMemoryInfo> bind_image_mem_info(0);
    tbm_bo bo = tbm_surface_internal_get_bo(tbmSurface, 0 );
    uint32_t tbmFD = tbm_bo_get_handle(bo, TBM_DEVICE_3D).u32;
    int32_t newFD = dup(static_cast<int32_t>(tbmFD));

    if ( newFD < 0 )
    {
      char buf[512] = {0,};
      DALI_LOG_ERROR("Fail to dup(%d) for tbmFD(%d): %s", newFD, tbmFD, strerror_r(errno, buf, 512));
      return false;
    }

    import_size = static_cast<VkDeviceSize>(tbmSurface_info.size);

    auto memory = mGraphics.AllocateMemory( mImage, vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent, static_cast<int>(newFD), import_size );
    if ( !memory )
    {
      return false;
    }

    tMemories.push_back(std::move(memory));
    mGraphics.BindImageMemory( mImage, bind_image_mem_info, 1, tMemories );
  }
  else
  {
    return false;
  }

  if ( mFormat == vk::Format::eG8B8R82Plane420UnormKHR )
  {
    // Check whether format is supported  by the platform
    auto properties = mGraphics.GetPhysicalDevice().getFormatProperties( mFormat );
    bool support_yuv_linearfilter = false;

    if( properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageYcbcrConversionLinearFilter )
    {
      support_yuv_linearfilter = true;
    }

    mSamplerIsImmutable = true;
    CreateSamplerYUVNativeImage( support_yuv_linearfilter );
    CreateImageViewYUVNativeImage();
  }
  else
  {
    CreateSampler();
    CreateImageView();
  }
#endif

  return true;
}

bool NativeTexture::GetFormatLinearDrmModifierNativeImage( VkFormat format, VkDrmFormatModifierPropertiesEXT &outMode )
{
#ifdef NATIVE_IMAGE_SUPPORT

  std::vector<VkDrmFormatModifierPropertiesEXT> drm_format_modifiers;
  VkDrmFormatModifierPropertiesListEXT mod_props =
  {
    static_cast< VkStructureType >(VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT),// VkStructureType sType;
    nullptr,        //  void*                              pNext;
    0,              //  uint32_t                           drmFormatModifierCount;
    nullptr         //  VkDrmFormatModifierPropertiesEXT*  pDrmFormatModifierProperties;
  };

  VkFormatProperties    formatProperties = {};
  VkFormatProperties2KHR format_props =
  {
    VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2_KHR, // VkStructureType       sType;
    &mod_props,                                // void*                 pNext;
    formatProperties                           // VkFormatProperties    formatProperties;
  };

  VkPhysicalDevice vkPhysicalDevice = static_cast<VkPhysicalDevice>(mGraphics.GetPhysicalDevice());
  gGetPhysicalDeviceFormatProperties2KHR( vkPhysicalDevice, format, &format_props);

  if (mod_props.drmFormatModifierCount <= 0)
  {
    return false;
  }

  drm_format_modifiers.resize( mod_props.drmFormatModifierCount);
  mod_props.pDrmFormatModifierProperties = &drm_format_modifiers[0];

  gGetPhysicalDeviceFormatProperties2KHR( vkPhysicalDevice, format, &format_props);

  for( VkDrmFormatModifierPropertiesEXT &mode : drm_format_modifiers )
  {
    if( mode.drmFormatModifier == DRM_FORMAT_MOD_LINEAR )
    {
      outMode = mode;
      return true;
    }
  }
#endif
  return false;
}

void NativeTexture::CreateImageViewYUVNativeImage()
{
#ifdef NATIVE_IMAGE_SUPPORT
  mImageView = mGraphics.CreateImageView(
    {}, mImage, vk::ImageViewType::e2D, mImage->GetFormat(), mComponentMapping,
    vk::ImageSubresourceRange{}
      .setAspectMask( mImage->GetAspectFlags() )
      .setBaseArrayLayer( 0 )
      .setBaseMipLevel( 0 )
      .setLevelCount( mImage->GetMipLevelCount() )
      .setLayerCount( mImage->GetLayerCount() ),
    static_cast<void*>(&mYcbcrConvInfo)
  );
#endif
}

bool NativeTexture::CreateSamplerYUVNativeImage( bool support_LinearFilter )
{
#ifdef NATIVE_IMAGE_SUPPORT
  VkSamplerYcbcrConversionCreateInfoKHR conv_create_info =
    {
      VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO_KHR, // VkStructureType                  sType;
      nullptr,                                                    //const void*                      pNext;
      static_cast<VkFormat>(mFormat),                             //VkFormat                         format;
      VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709_KHR,            //VkSamplerYcbcrModelConversion    ycbcrModel;
      VK_SAMPLER_YCBCR_RANGE_ITU_FULL_KHR,                        // VkSamplerYcbcrRange              ycbcrRange;
      {

          VK_COMPONENT_SWIZZLE_IDENTITY,                          //VkComponentSwizzle    r;
          VK_COMPONENT_SWIZZLE_IDENTITY,                          //VkComponentSwizzle    g;
          VK_COMPONENT_SWIZZLE_IDENTITY,                          //VkComponentSwizzle    b;
          VK_COMPONENT_SWIZZLE_IDENTITY,                          //VkComponentSwizzle    a;
      },                                                          // VkComponentMapping   components;
      VK_CHROMA_LOCATION_MIDPOINT_KHR,                            //VkChromaLocation      xChromaOffset;
      VK_CHROMA_LOCATION_MIDPOINT_KHR,                            //VkChromaLocation      yChromaOffset;
      VK_FILTER_NEAREST,                                          //VkFilter              chromaFilter;
      VK_FALSE                                                    //VkBool32              forceExplicitReconstruction;
    };

    if ( support_LinearFilter )
    {
      conv_create_info.chromaFilter = VK_FILTER_LINEAR;
    }

   if (VK_SUCCESS != gCreateSamplerYcbcrConversionKHR ( static_cast<VkDevice>(mGraphics.GetDevice()), &conv_create_info, nullptr, &mYcbcrConv))
   {
     return false;
   }

   mYcbcrConvInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO_KHR;
   mYcbcrConvInfo.pNext = nullptr;
   mYcbcrConvInfo.conversion = mYcbcrConv;

  auto samplerCreateInfo = vk::SamplerCreateInfo{}
    .setPNext(( static_cast< void * >(&mYcbcrConvInfo) ))
    .setMagFilter( vk::Filter::eLinear )
    .setMinFilter( vk::Filter::eLinear )
    .setMipmapMode( vk::SamplerMipmapMode::eLinear )
    .setAddressModeU( vk::SamplerAddressMode::eClampToEdge )
    .setAddressModeV( vk::SamplerAddressMode::eClampToEdge )
    .setAddressModeW( vk::SamplerAddressMode::eClampToEdge )
    .setMipLodBias( 0.0f )
    .setAnisotropyEnable( false )
    .setMaxAnisotropy( 1.0f )
    .setCompareEnable( false )
    .setCompareOp( vk::CompareOp::eLessOrEqual )
    .setMinLod( -1000.0f )
    .setMaxLod( 1000.0f )
    .setBorderColor( vk::BorderColor::eFloatTransparentBlack )
    .setUnnormalizedCoordinates( false);

  mSampler = mGraphics.CreateSampler( samplerCreateInfo );
#endif
  return true;
}

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
