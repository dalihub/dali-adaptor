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
#include <dali/internal/graphics/vulkan-impl/vulkan-native-image-handler.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-utils.h>

// EXTERNAL INCLUDES
#include <errno.h>
#include <fcntl.h>
#include <tbm_bo.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <tbm_type_common.h>
#include <unistd.h>
#include <vulkan/vulkan.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace
{
// Vulkan extension function pointers for native image support
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

// Initialize Vulkan extension function pointers for native image support
void InitializeVulkanExtensions(vk::Device device)
{
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

} // namespace

namespace Dali::Graphics::Vulkan
{
/**
 * @brief Tizen-specific implementation of VulkanNativeImageHandler
 *
 * This class handles Tizen-specific native image operations for Vulkan,
 * including TBM surface management, DMA-BUF file descriptor export/import,
 * and YCbCr conversion support.
 */
class VulkanNativeImageHandlerTizen : public VulkanNativeImageHandler
{
public:
  /**
   * @brief Sets the format and usage flags for a native texture based on TBM surface properties.
   *
   * Extracts format information from the TBM surface and determines appropriate
   * Vulkan format and usage flags. Handles both RGB and YUV formats.
   *
   * @param[in] createInfo The texture creation information containing native image interface
   * @param[in] device The Vulkan device reference
   * @return NativeTextureData containing format, usage, and validity information
   */
  NativeTextureData SetFormatAndUsage(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device) override;

  /**
   * @brief Initializes a native texture by creating Vulkan resources from TBM surface.
   *
   * Performs the complete initialization process including:
   * - Exporting DMA-BUF file descriptors from TBM surface
   * - Creating Vulkan image from external memory
   * - Setting up YCbCr conversion for YUV formats
   * - Creating image view and sampler
   *
   * @param[in] createInfo The texture creation information
   * @param[in] device The Vulkan device reference
   * @param[in] width The texture width in pixels
   * @param[in] height The texture height in pixels
   * @param[in,out] textureData The texture data containing format and usage information
   * @return std::unique_ptr<NativeImageResources> containing all created Vulkan resources, or nullptr on failure
   */
  std::unique_ptr<NativeImageResources> InitializeNativeTexture(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device, uint32_t width, uint32_t height, NativeTextureData& textureData) override;

  /**
   * @brief Acquires a reference to the current surface to prevent premature destruction.
   *
   * Uses the surface reference manager from the native image interface to
   * increment the reference count for the current surface.
   *
   * @param[in,out] textureData The texture data containing surface handle information
   * @param[in] nativeImagePtr The native image interface pointer
   * @return true if the reference was successfully acquired, false if already acquired or failed
   */
  bool AcquireCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr) override;

  /**
   * @brief Releases the reference to the current surface.
   *
   * Uses the surface reference manager from the native image interface to
   * decrement the reference count for the current surface.
   *
   * @param[in,out] textureData The texture data containing surface handle information
   * @param[in] nativeImagePtr The native image interface pointer
   * @return true if the reference was successfully released, false if no reference was held or failed
   */
  bool ReleaseCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr) override;

  /**
   * @brief Destroys all native Vulkan resources associated with the texture.
   *
   * Cleans up sampler, YCbCr conversion, and calls ResetNativeResources
   * to handle image and memory cleanup.
   *
   * @param[in] device The Vulkan device reference
   * @param[in] resources The native image resources to destroy (will be moved and reset)
   */
  void DestroyNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources) override;

  /**
   * @brief Resets native Vulkan resources while preserving the resource container.
   *
   * Destroys Vulkan image, device memory, and releases TBM buffer object
   * references but keeps the NativeImageResources object valid for reuse.
   *
   * @param[in] device The Vulkan device reference
   * @param[in] resources The native image resources to reset
   */
  void ResetNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources) override;

private:
  /**
   * Find suitable memory type index.
   * @param[in] device The Vulkan device reference
   * @param[in] typeBits Memory type requirements
   * @param[in] flags Required memory properties
   * @return Memory type index
   */
  uint32_t FindMemoryType(Device& device, uint32_t typeBits, vk::MemoryPropertyFlags flags) const;

  /**
   * @brief Export DMA-BUF file descriptors from TBM surface.
   *
   * Extracts file descriptors from all buffer objects in the TBM surface
   * and duplicates them for Vulkan import. Also maintains references to
   * TBM buffer objects to keep them alive during Vulkan usage.
   *
   * @param[out] planeFds Vector to store the exported file descriptors
   * @param[out] tbmBos Vector to store TBM buffer object references
   * @param[in] tbmSurface The TBM surface to export from
   * @return true if export was successful, false on failure
   */
  bool ExportPlaneFds(std::vector<int>& planeFds, std::vector<void*>& tbmBos, void* tbmSurface);

  /**
   * @brief Import DMA-BUF memory into Vulkan device memory.
   *
   * Creates Vulkan device memory from a DMA-BUF file descriptor.
   * Validates the file descriptor, gets memory properties, and imports
   * the external memory with appropriate memory type.
   *
   * @param[in] device The Vulkan device reference
   * @param[in] fd The DMA-BUF file descriptor to import
   * @return vk::DeviceMemory The imported Vulkan device memory, or VK_NULL_HANDLE on failure
   */
  vk::DeviceMemory ImportPlaneMemory(Device& device, int fd);

  /**
   * @brief Create Vulkan image from external memory.
   *
   * Creates a Vulkan image that can be bound to external DMA-BUF memory.
   * Handles both single-plane and multi-plane (disjoint) memory layouts.
   * Imports memory for each plane and binds it to the image.
   *
   * @param[in,out] resources The native image resources to store the created image
   * @param[in] device The Vulkan device reference
   * @param[in] tbmSurface The TBM surface for plane information
   * @param[in] planeFds Vector of file descriptors for each plane
   * @param[in] width The image width in pixels
   * @param[in] height The image height in pixels
   * @param[in] format The Vulkan image format
   * @param[in] usage The image usage flags
   * @param[in] isYUVFormat Whether this is a YUV format requiring special handling
   * @return true if image creation was successful, false on failure
   */
  bool CreateNativeImage(std::unique_ptr<NativeImageResources>& resources, Device& device, void* tbmSurface, std::vector<int>& planeFds, uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage, bool isYUVFormat);

  /**
   * @brief Create the YCbCr conversion for YUV format sampling.
   *
   * Creates a Vulkan sampler YCbCr conversion object that handles the
   * conversion from YUV color space to RGB during sampling. Configures
   * conversion parameters based on device capabilities.
   *
   * @param[in,out] resources The native image resources to store the conversion
   * @param[in] device The Vulkan device reference
   * @param[in] format The YUV format to create conversion for
   * @return true if conversion creation was successful, false on failure
   */
  bool CreateYcbcrConversion(std::unique_ptr<NativeImageResources>& resources, Device& device, vk::Format format);

  /**
   * @brief Create image view for the imported image.
   *
   * Creates a Vulkan image view for the imported native image.
   * For YUV formats, chains the YCbCr conversion info to enable
   * automatic color space conversion during sampling.
   *
   * @param[in,out] resources The native image resources to store the image view
   * @param[in] device The Vulkan device reference
   * @param[in] image The Vulkan image to create view for
   * @param[in] format The image format
   * @param[in] isYUVFormat Whether this is a YUV format requiring conversion
   * @return true if image view creation was successful, false on failure
   */
  bool CreateNativeImageView(std::unique_ptr<NativeImageResources>& resources, Device& device, vk::Image image, vk::Format format, bool isYUVFormat);

  /**
   * @brief Create sampler with optional YCbCr conversion.
   *
   * Creates a Vulkan sampler for sampling the native image.
   * For YUV formats, includes the YCbCr conversion in the sampler
   * creation to enable automatic color space conversion.
   *
   * @param[in,out] resources The native image resources to store the sampler
   * @param[in] device The Vulkan device reference
   * @param[in] format The image format
   * @param[in] isYUVFormat Whether this is a YUV format requiring conversion
   * @return true if sampler creation was successful, false on failure
   */
  bool CreateNativeSampler(std::unique_ptr<NativeImageResources>& resources, Device& device, vk::Format format, bool isYUVFormat);

  /**
   * @brief Release buffer object references of the current surface.
   *
   * Decrements the reference count for all TBM buffer objects
   * that were kept alive during Vulkan usage. Called when
   * Vulkan resources are no longer needed.
   *
   * @param[in,out] tbmBos Vector of TBM buffer object references to release
   */
  void ReleaseSurfaceBufferObjectReferences(std::vector<void*>& tbmBos);

  /**
   * @brief Get surface reference manager from native image interface.
   *
   * Extracts the surface reference manager from the native image
   * interface extension. This manager is used to acquire and release
   * surface references to prevent premature destruction.
   *
   * @param[in] nativeImagePtr The native image interface pointer
   * @return SurfaceReferenceManager* Pointer to surface reference manager, or nullptr if not available
   */
  SurfaceReferenceManager* GetSurfaceReferenceManager(const Dali::NativeImageInterfacePtr& nativeImagePtr) const;
};

// Factory implementation
std::unique_ptr<VulkanNativeImageHandler> VulkanNativeImageHandler::CreateHandler()
{
  return std::make_unique<VulkanNativeImageHandlerTizen>();
}

uint32_t VulkanNativeImageHandlerTizen::FindMemoryType(Device& device, uint32_t typeBits, vk::MemoryPropertyFlags flags) const
{
  auto memProperties = device.GetMemoryProperties();

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

NativeTextureData VulkanNativeImageHandlerTizen::SetFormatAndUsage(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device)
{
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "SetFormatAndUsage for native image\n");

  NativeTextureData textureData;
  textureData.isValid = false;

  NativeImageInterfacePtr nativeImage       = createInfo.nativeImagePtr;
  Dali::Any               nativeImageSource = nativeImage->GetNativeImageHandle();
  if(nativeImageSource.GetType() == typeid(tbm_surface_h))
  {
    tbm_surface_h tbmSurface = AnyCast<tbm_surface_h>(nativeImageSource);
    if(tbm_surface_internal_is_valid(tbmSurface))
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "SetFormatAndUsage: Valid TBM surface\n");

      textureData.surfaceHandle = tbmSurface;
      tbm_format tbmFormat      = tbm_surface_get_format(tbmSurface);
      textureData.format        = GetVulkanFormat(tbmFormat);
      textureData.isYUVFormat   = RequiresYcbcrConversion(tbmFormat);

      if(textureData.isYUVFormat)
      {
        textureData.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
      }
      else
      {
        textureData.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
      }
      textureData.tiling  = Dali::Graphics::TextureTiling::LINEAR;
      textureData.isValid = true;

      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "SetFormatAndUsage for native image: tbmFormat: %d, format: %d, isYUVFormat: %d\n", static_cast<int>(tbmFormat), static_cast<int>(textureData.format), textureData.isYUVFormat);
    }
    else
    {
      DALI_LOG_ERROR("VulkanNativeImageHandlerTizen::SetFormatAndUsage: Invalid TBM surface\n");
    }
  }
  else
  {
    DALI_LOG_ERROR("VulkanNativeImageHandlerTizen::SetFormatAndUsage: nativeImageSource.GetType() != typeid(tbm_surface_h)\n");
  }

  return textureData;
}

std::unique_ptr<NativeImageResources> VulkanNativeImageHandlerTizen::InitializeNativeTexture(const Dali::Graphics::TextureCreateInfo& createInfo, Device& device, uint32_t width, uint32_t height, NativeTextureData& textureData)
{
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "InitializeNativeTexture: BEGIN\n");

  // Initialize Vulkan extensions if needed
  InitializeVulkanExtensions(device.GetLogicalDevice());

  auto resources = std::make_unique<NativeImageResources>();

  if(!textureData.isValid || textureData.format == vk::Format::eUndefined)
  {
    DALI_LOG_ERROR("Invalid texture data or format\n");
    return nullptr;
  }

  NativeImageInterfacePtr nativeImage = createInfo.nativeImagePtr;
  tbm_surface_h           tbmSurface  = reinterpret_cast<tbm_surface_h>(textureData.surfaceHandle);

  if(!tbm_surface_internal_is_valid(tbmSurface))
  {
    DALI_LOG_ERROR("Invalid TBM surface\n");
    return nullptr;
  }

  bool created = nativeImage->CreateResource();

  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "native image CreateResource created: %d\n", created);

  if(created)
  {
    // Acquire surface reference before using it
    AcquireCurrentSurfaceReference(textureData, nativeImage);

    // 1. Export plane file descriptors
    if(!ExportPlaneFds(resources->planeFds, resources->tbmBos, textureData.surfaceHandle))
    {
      DALI_LOG_ERROR("Failed to export plane FDs\n");
      ReleaseCurrentSurfaceReference(textureData, nativeImage);
      return nullptr;
    }

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "native image ExportPlaneFds succeeded: isYUVFormat: %d\n", textureData.isYUVFormat);

    if(textureData.isYUVFormat && !device.IsKHRSamplerYCbCrConversionSupported())
    {
      DALI_LOG_ERROR("SamplerYcbcrConversion feature required for YUV texture is not supported\n");
      ReleaseCurrentSurfaceReference(textureData, nativeImage);
      return nullptr;
    }

    // 2. Create Vulkan image from external memory
    if(!CreateNativeImage(resources, device, textureData.surfaceHandle, resources->planeFds, width, height, textureData.format, textureData.usage, textureData.isYUVFormat))
    {
      DALI_LOG_ERROR("Failed to create Vulkan image\n");
      ReleaseCurrentSurfaceReference(textureData, nativeImage);
      return nullptr;
    }

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImage succeeded\n");

    // 3. Create SamplerYcbcrConversion (if needed)
    if(textureData.isYUVFormat)
    {
      if(!CreateYcbcrConversion(resources, device, textureData.format))
      {
        DALI_LOG_ERROR("Failed to create Ycbcr Conversion\n");
        ReleaseCurrentSurfaceReference(textureData, nativeImage);
        return nullptr;
      }

      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateYcbcrConversion succeeded\n");
    }

    // 4. Create image view for the imported image
    if(!CreateNativeImageView(resources, device, resources->nativeImage, textureData.format, textureData.isYUVFormat))
    {
      DALI_LOG_ERROR("Failed to create image view\n");
      ReleaseCurrentSurfaceReference(textureData, nativeImage);
      return nullptr;
    }

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImageView succeeded\n");

    // 5. Create sampler with optional YCbCr conversion
    if(!CreateNativeSampler(resources, device, textureData.format, textureData.isYUVFormat))
    {
      DALI_LOG_ERROR("Failed to create sampler\n");
      ReleaseCurrentSurfaceReference(textureData, nativeImage);
      return nullptr;
    }

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeSampler succeeded\n");
  }
  else
  {
    DALI_LOG_ERROR("Native Image: InitializeNativeTexture, CreateResource() failed\n");
    return nullptr;
  }

  return resources;
}

bool VulkanNativeImageHandlerTizen::AcquireCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr)
{
  if(textureData.currentSurface && textureData.hasSurfaceReference)
  {
    // Already have reference
    return false;
  }

  if(!textureData.surfaceHandle)
  {
    return false;
  }

  auto surfaceRefManager = GetSurfaceReferenceManager(nativeImagePtr);
  if(surfaceRefManager)
  {
    surfaceRefManager->AcquireSurfaceReference(textureData.surfaceHandle);
    textureData.currentSurface      = textureData.surfaceHandle;
    textureData.hasSurfaceReference = true;

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "AcquireCurrentSurfaceReference: Acquired reference to surface %p\n", textureData.surfaceHandle);
    return true;
  }

  return false;
}

bool VulkanNativeImageHandlerTizen::ReleaseCurrentSurfaceReference(NativeTextureData& textureData, const Dali::NativeImageInterfacePtr& nativeImagePtr)
{
  if(!textureData.currentSurface || !textureData.hasSurfaceReference)
  {
    return false;
  }

  auto surfaceRefManager = GetSurfaceReferenceManager(nativeImagePtr);
  if(surfaceRefManager)
  {
    surfaceRefManager->ReleaseSurfaceReference(textureData.surfaceHandle);
    textureData.currentSurface      = nullptr;
    textureData.hasSurfaceReference = false;

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "ReleaseCurrentSurfaceReference: Released reference to surface %p\n", textureData.surfaceHandle);
    return true;
  }

  return false;
}

void VulkanNativeImageHandlerTizen::DestroyNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources)
{
  if(!resources)
  {
    return;
  }

  // Destroy native image Vulkan resources.
  if(resources->sampler)
  {
    resources->sampler->Destroy();
    resources->sampler = nullptr;
  }

  auto logicalDevice = device.GetLogicalDevice();

  if(resources->ycbcrConversion != VK_NULL_HANDLE)
  {
    logicalDevice.destroySamplerYcbcrConversion(static_cast<vk::SamplerYcbcrConversion>(resources->ycbcrConversion));
    resources->ycbcrConversion = VK_NULL_HANDLE;
  }

  ResetNativeResources(device, std::move(resources));
}

void VulkanNativeImageHandlerTizen::ResetNativeResources(Device& device, std::unique_ptr<NativeImageResources> resources)
{
  if(!resources)
  {
    return;
  }

  auto logicalDevice = device.GetLogicalDevice();

  // Free old device memories
  if(!resources->memories.empty())
  {
    for(auto& memory : resources->memories)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "ResetNativeResources: Freeing old VkDeviceMemory %p\n", static_cast<VkDeviceMemory>(memory));

      logicalDevice.freeMemory(memory);
    }

    resources->memories.clear();
  }

  // Destroy old image
  if(resources->nativeImage != VK_NULL_HANDLE)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "ResetNativeResources: Destroying old VkImage %p\n", static_cast<VkImage>(resources->nativeImage));

    logicalDevice.destroyImage(resources->nativeImage);
    resources->nativeImage = VK_NULL_HANDLE;
  }

  // Clear old FDs
  resources->planeFds.clear();

  // Release old BO references
  ReleaseSurfaceBufferObjectReferences(resources->tbmBos);
}

bool VulkanNativeImageHandlerTizen::ExportPlaneFds(std::vector<int>& planeFds, std::vector<void*>& tbmBos, void* tbmSurface)
{
  if(!tbmSurface)
  {
    DALI_LOG_ERROR("ExportPlaneFds: tbmSurface is NULL, returning false\n");
    return false;
  }

  planeFds.clear(); // clear any old, stale FDs
  tbmBos.clear();   // clear any old BO references

  tbm_surface_h      surface = reinterpret_cast<tbm_surface_h>(tbmSurface);
  tbm_surface_info_s tbmSurfaceInfo;

  if(tbm_surface_get_info(surface, &tbmSurfaceInfo) != TBM_SURFACE_ERROR_NONE)
  {
    DALI_LOG_ERROR("ExportPlaneFds: Failed to get TBM surface info\n");
    return false;
  }

  int num_bos = tbm_surface_internal_get_num_bos(surface);

  for(int i = 0; i < num_bos; ++i)
  {
    tbm_bo bo = tbm_surface_internal_get_bo(surface, i);
    if(!bo)
    {
      DALI_LOG_ERROR("ExportPlaneFds: BO %d is NULL, skipping\n", i);
      continue;
    }

    // Export original FD from TBM
    int originalFd = tbm_bo_export_fd(bo);

    if(originalFd < 0)
    {
      DALI_LOG_ERROR("ExportPlaneFds: Failed to export FD for BO %d (returned %d)\n", i, originalFd);
      return false;
    }

    // Duplicate the FD
    int dupFd = dup(originalFd);

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
    close(originalFd);

    // Keep TBM BO alive beyond Vulkan import
    tbm_bo_ref(bo);
    tbmBos.push_back(static_cast<void*>(bo));

    planeFds.push_back(dupFd);

    // Verify the duplicated FD is valid
    if(fcntl(dupFd, F_GETFD) == -1)
    {
      DALI_LOG_ERROR("ExportPlaneFds: WARNING - duplicated FD %d is already invalid after creation! errno=%d: %s\n",
                     dupFd,
                     errno,
                     strerror(errno));
    }
  }

  return !planeFds.empty();
}

vk::DeviceMemory VulkanNativeImageHandlerTizen::ImportPlaneMemory(Device& device, int fd)
{
  // Validate file descriptor before use
  if(fd < 0)
  {
    DALI_LOG_ERROR("ImportPlaneMemory: Invalid file descriptor: %d\n", fd);
    return VK_NULL_HANDLE;
  }

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

  auto logicalDevice = device.GetLogicalDevice();

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

  // Reset file offset
  lseek(fd, 0, SEEK_SET);

  // Get memory properties from FD
  auto memFdProps  = VkMemoryFdPropertiesKHR{};
  memFdProps.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;

  if(!gPfnGetMemoryFdPropertiesKHR)
  {
    DALI_LOG_ERROR("ImportPlaneMemory: gPfnGetMemoryFdPropertiesKHR is NULL!\n");
    return VK_NULL_HANDLE;
  }

  VkResult fdPropsResult = gPfnGetMemoryFdPropertiesKHR(logicalDevice,
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

  // Import memory with FD-specific memory type
  auto importInfo = vk::ImportMemoryFdInfoKHR{}
                      .setHandleType(vk::ExternalMemoryHandleTypeFlagBits::eDmaBufEXT)
                      .setFd(fd);

  uint32_t memoryTypeIndex = FindMemoryType(device,
                                            memFdProps.memoryTypeBits,
                                            vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent);

  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "ImportPlaneMemory: Found memory type index %u for FD %d\n", memoryTypeIndex, fd);

  auto allocInfo = vk::MemoryAllocateInfo{}
                     .setPNext(&importInfo)
                     .setAllocationSize(static_cast<vk::DeviceSize>(dma_buf_size))
                     .setMemoryTypeIndex(memoryTypeIndex);

  vk::DeviceMemory memory = logicalDevice.allocateMemory(allocInfo).value;

  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "ImportPlaneMemory: SUCCESS - FD %d imported to memory handle %p\n", fd, static_cast<VkDeviceMemory>(memory));

  return memory;
}

bool VulkanNativeImageHandlerTizen::CreateNativeImage(std::unique_ptr<NativeImageResources>& resources, Device& device, void* tbmSurface, std::vector<int>& planeFds, uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage, bool isYUVFormat)
{
  auto logicalDevice = device.GetLogicalDevice();

  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImage: Creating external memory image (format=%d, size=%ux%u)\n", static_cast<int>(format), width, height);

  // Create external memory image
  auto extMemCreateInfo = vk::ExternalMemoryImageCreateInfo{}
                            .setHandleTypes(vk::ExternalMemoryHandleTypeFlagBits::eDmaBufEXT);

  auto imageCreateInfo = vk::ImageCreateInfo{}
                           .setPNext(static_cast<void*>(&extMemCreateInfo))
                           .setImageType(vk::ImageType::e2D)
                           .setFormat(format)
                           .setExtent({width, height, 1})
                           .setMipLevels(1)
                           .setArrayLayers(1)
                           .setSamples(vk::SampleCountFlagBits::e1)
                           .setTiling(vk::ImageTiling::eLinear)
                           .setUsage(usage)
                           .setSharingMode(vk::SharingMode::eExclusive)
                           .setInitialLayout(isYUVFormat ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined);

  resources->nativeImage = logicalDevice.createImage(imageCreateInfo).value;

  resources->image = new Image(device, imageCreateInfo, resources->nativeImage);

  // Check for disjoint vs non-disjoint multi-plane layout
  bool isDisjoint = false;
  if(planeFds.size() > 1)
  {
    DALI_LOG_ERROR("CreateNativeImage: Multiple planes detected, checking if disjoint\n");
    for(size_t i = 1; i < planeFds.size(); ++i)
    {
      if(planeFds[i] != planeFds[0])
      {
        isDisjoint = true;
        break;
      }
    }
  }

  if(!isDisjoint)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImage: Using non-disjoint/single-plane layout\n");
  }

  // Import memory for each plane
  resources->memories.clear();

  if(!isDisjoint && !planeFds.empty())
  {
    // Verify FD is still valid before import
    if(fcntl(planeFds[0], F_GETFD) == -1)
    {
      DALI_LOG_ERROR("CreateNativeImage: ERROR - FD %d is INVALID before import! errno=%d: %s\n",
                     planeFds[0],
                     errno,
                     strerror(errno));
      return false;
    }

    // Single memory binding for non-disjoint or single-plane
    auto memory = ImportPlaneMemory(device, planeFds[0]);
    if(memory == VK_NULL_HANDLE)
    {
      DALI_LOG_ERROR("CreateNativeImage: ImportPlaneMemory failed for FD %d\n", planeFds[0]);
      return false;
    }

    resources->memories.push_back(memory);

    VkAssert(logicalDevice.bindImageMemory(resources->nativeImage, memory, 0));

    // Close the duplicated FD after successful import
    close(planeFds[0]);
    planeFds.clear();
  }
  else if(isDisjoint)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImage: Disjoint multi-plane binding path\n");

    tbm_surface_h      surface = reinterpret_cast<tbm_surface_h>(tbmSurface);
    tbm_surface_info_s tbmSurfaceInfo;

    if(tbm_surface_get_info(surface, &tbmSurfaceInfo) != TBM_SURFACE_ERROR_NONE)
    {
      DALI_LOG_ERROR("CreateNativeImage: Failed to get TBM surface info\n");
      return false;
    }

    // Multi-plane binding with VkBindImageMemory2
    std::vector<vk::BindImageMemoryInfo>      bindInfos;
    std::vector<vk::BindImagePlaneMemoryInfo> planeInfos;

    for(size_t i = 0; i < planeFds.size(); ++i)
    {
      // Verify FD is still valid
      if(fcntl(planeFds[i], F_GETFD) == -1)
      {
        DALI_LOG_ERROR("CreateNativeImage: ERROR - FD %d for plane %zu is INVALID! errno=%d: %s\n",
                       planeFds[i],
                       i,
                       errno,
                       strerror(errno));
        return false;
      }

      auto memory = ImportPlaneMemory(device, planeFds[i]);
      if(memory == VK_NULL_HANDLE)
      {
        DALI_LOG_ERROR("CreateNativeImage: Failed to import memory for plane %zu FD %d\n",
                       i,
                       planeFds[i]);
        return false;
      }

      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImage: Successfully imported memory %p for plane %zu\n", static_cast<VkDeviceMemory>(memory), i);

      resources->memories.push_back(memory);

      // Setup plane binding info
      planeInfos.emplace_back(vk::BindImagePlaneMemoryInfo{}
                                .setPlaneAspect(PLANE_ASPECT_FLAGS[i]));

      bindInfos.emplace_back(vk::BindImageMemoryInfo{}
                               .setPNext(&planeInfos[i])
                               .setImage(resources->nativeImage)
                               .setMemory(memory)
                               .setMemoryOffset(tbmSurfaceInfo.planes[i].offset));
    }

    vk::Result result = logicalDevice.bindImageMemory2(bindInfos);

    // Close all duplicated FDs after binding
    for(size_t i = 0; i < planeFds.size(); ++i)
    {
      close(planeFds[i]);
    }
    planeFds.clear();

    if(result != vk::Result::eSuccess)
    {
      DALI_LOG_ERROR("CreateNativeImage: vkBindImageMemory2 failed with result=%d\n",
                     static_cast<int>(result));

      // Release BO references after Vulkan import completes
      ReleaseSurfaceBufferObjectReferences(resources->tbmBos);
      return false;
    }

    DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "CreateNativeImage: Successfully bound all planes\n");
  }

  return true;
}

bool VulkanNativeImageHandlerTizen::CreateYcbcrConversion(std::unique_ptr<NativeImageResources>& resources, Device& device, vk::Format format)
{
  bool support_cosited_chroma_sampling = false;
  bool support_linearfilter            = false;

  // Check whether format is supported by the platform
  auto formatProperties = device.GetPhysicalDevice().getFormatProperties(format);

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
                                .setFormat(format)
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

  if(!gPfnCreateSamplerYcbcrConversionKHR || VK_SUCCESS != gPfnCreateSamplerYcbcrConversionKHR(device.GetLogicalDevice(), reinterpret_cast<const VkSamplerYcbcrConversionCreateInfo*>(static_cast<const void*>(&conversionCreateInfo)), nullptr, &resources->ycbcrConversion))
  {
    DALI_LOG_ERROR("vkCreateSamplerYcbcrConversion failed\n");
    return false;
  }

  return true;
}

bool VulkanNativeImageHandlerTizen::CreateNativeImageView(std::unique_ptr<NativeImageResources>& resources, Device& device, vk::Image image, vk::Format format, bool isYUVFormat)
{
  auto viewInfo = vk::ImageViewCreateInfo{}
                    .setImage(image)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(format)
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  // Chain YCbCr conversion info for YUV formats
  if(isYUVFormat && resources->ycbcrConversion != VK_NULL_HANDLE)
  {
    resources->ycbcrConversionInfo = vk::SamplerYcbcrConversionInfo{}
                                       .setConversion(static_cast<vk::SamplerYcbcrConversion>(resources->ycbcrConversion));

    viewInfo.setPNext(&resources->ycbcrConversionInfo)
      .setComponents(vk::ComponentMapping()
                       .setR(vk::ComponentSwizzle::eIdentity)
                       .setG(vk::ComponentSwizzle::eIdentity)
                       .setB(vk::ComponentSwizzle::eIdentity)
                       .setA(vk::ComponentSwizzle::eIdentity));
  }

  resources->imageView = ImageView::New(device, *resources->image, viewInfo);

  return true;
}

bool VulkanNativeImageHandlerTizen::CreateNativeSampler(std::unique_ptr<NativeImageResources>& resources, Device& device, vk::Format format, bool isYUVFormat)
{
  vk::SamplerCreateInfo samplerCreateInfo{};

  if(isYUVFormat)
  {
    // Create sampler with YCbCr conversion
    samplerCreateInfo = vk::SamplerCreateInfo{}
                          .setPNext(static_cast<void*>(&resources->ycbcrConversionInfo))
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
    const auto& properties = device.GetPhysicalDeviceProperties();

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

  resources->sampler = SamplerImpl::New(device, samplerCreateInfo);

  return true;
}

void VulkanNativeImageHandlerTizen::ReleaseSurfaceBufferObjectReferences(std::vector<void*>& tbmBos)
{
  for(auto bo : tbmBos)
  {
    if(bo)
    {
      tbm_bo_unref(static_cast<tbm_bo>(bo));
    }
  }

  tbmBos.clear();
}

SurfaceReferenceManager* VulkanNativeImageHandlerTizen::GetSurfaceReferenceManager(const Dali::NativeImageInterfacePtr& nativeImagePtr) const
{
  if(!nativeImagePtr)
  {
    return nullptr;
  }

  auto extension = nativeImagePtr->GetExtension();

  if(!extension)
  {
    // This native image type does not support extensions.
    return nullptr;
  }

  return reinterpret_cast<SurfaceReferenceManager*>(extension);
}

} // namespace Dali::Graphics::Vulkan
