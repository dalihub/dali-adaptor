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

#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_NATIVE_IMAGE_HANDLER_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_NATIVE_IMAGE_HANDLER_H

// EXTERNAL INCLUDES
#include <memory>
#include <vector>

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-texture-create-info.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class Texture;
class Device;
class Image;
class ImageView;
class SamplerImpl;

/**
 * @brief Data structure for native image texture properties
 */
struct NativeTextureData
{
  void*                         surfaceHandle{nullptr};                         ///< Native surface handle
  vk::Format                    format{vk::Format::eUndefined};                 ///< Vulkan format
  vk::ImageUsageFlags           usage{};                                        ///< Image usage flags
  Dali::Graphics::TextureTiling tiling{Dali::Graphics::TextureTiling::OPTIMAL}; ///< Tiling mode
  bool                          isYUVFormat{false};                             ///< Whether format is YUV
  bool                          isValid{false};                                 ///< Whether data is valid
  void*                         currentSurface{nullptr};                        ///< Currently referenced surface
  bool                          hasSurfaceReference{false};                     ///< Whether a surface reference is acquired already
};

/**
 * @brief Data structure for native image resources
 */
struct NativeImageResources
{
  vk::Image                      nativeImage{VK_NULL_HANDLE};     ///< Native image handle
  std::vector<vk::DeviceMemory>  memories;                        ///< Device memories per plane
  VkSamplerYcbcrConversion       ycbcrConversion{VK_NULL_HANDLE}; ///< YCbCr conversion (if needed)
  vk::SamplerYcbcrConversionInfo ycbcrConversionInfo{};           ///< YCbCr conversion info
  Image*                         image{nullptr};                  ///< Vulkan Image
  ImageView*                     imageView{nullptr};              ///< Vulkan Image view
  SamplerImpl*                   sampler{nullptr};                ///< Vulkan Sampler
  std::vector<int>               planeFds;                        ///< File descriptors per plane
  std::vector<void*>             tbmBos;                          ///< TBM buffer objects
};

/**
 * @brief Interface for handling platform-specific native image operations
 *
 * This interface encapsulates all platform-specific native image functionality,
 * allowing the main Vulkan texture implementation to remain platform-agnostic.
 */
class VulkanNativeImageHandler
{
public:
  /**
   * @brief Destructor
   */
  virtual ~VulkanNativeImageHandler() = default;

  /**
   * @brief Set format and usage flags for native texture
   *
   * @param[in] createInfo The texture creation info
   * @param[in] device The Vulkan device for resource creation
   * @return Native texture data with format and usage information
   */
  virtual NativeTextureData SetFormatAndUsage(const Dali::Graphics::TextureCreateInfo& createInfo,
                                              Device&                                  device) = 0;

  /**
   * @brief Initialize native texture with platform-specific logic
   *
   * @param[in] createInfo The texture creation info
   * @param[in] device The Vulkan device for resource creation
   * @param[in] width Texture width
   * @param[in] height Texture height
   * @param[in] textureData Native texture data from SetFormatAndUsage
   * @return Native image resources or nullptr on failure
   */
  virtual std::unique_ptr<NativeImageResources> InitializeNativeTexture(const Dali::Graphics::TextureCreateInfo& createInfo,
                                                                        Device&                                  device,
                                                                        uint32_t                                 width,
                                                                        uint32_t                                 height,
                                                                        NativeTextureData&                       textureData) = 0;

  /**
   * @brief Acquire reference to current surface
   *
   * @param[in] textureData Native texture data
   * @param[in] nativeImagePtr The native image interface pointer
   * @return true if reference was acquired, false otherwise
   */
  virtual bool AcquireCurrentSurfaceReference(NativeTextureData&                   textureData,
                                              const Dali::NativeImageInterfacePtr& nativeImagePtr) = 0;

  /**
   * @brief Release reference to current surface
   *
   * @param[in] textureData Native texture data
   * @param[in] nativeImagePtr The native image interface pointer
   * @return true if reference was released, false otherwise
   */
  virtual bool ReleaseCurrentSurfaceReference(NativeTextureData&                   textureData,
                                              const Dali::NativeImageInterfacePtr& nativeImagePtr) = 0;

  /**
   * @brief Destroy all native resources associated with the texture
   *
   * @param[in] device The Vulkan device for resource cleanup
   * @param[in] resources The native image resources to destroy
   */
  virtual void DestroyNativeResources(Device&                               device,
                                      std::unique_ptr<NativeImageResources> resources) = 0;

  /**
   * @brief Reset the native resources associated with the texture
   *
   * @param[in] device The Vulkan device for resource cleanup
   * @param[in] resources The native image resources to reset
   */
  virtual void ResetNativeResources(Device&                               device,
                                    std::unique_ptr<NativeImageResources> resources) = 0;

  /**
   * @brief Factory method to create platform-specific handler
   *
   * @return Unique pointer to the appropriate handler implementation
   */
  static std::unique_ptr<VulkanNativeImageHandler> CreateHandler();
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_NATIVE_IMAGE_HANDLER_H
