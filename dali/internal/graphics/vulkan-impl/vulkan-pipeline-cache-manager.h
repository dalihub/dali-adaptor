#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_PIPELINE_CACHE_MANAGER__H
#define DALI_INTERNAL_GRAPHICS_VULKAN_PIPELINE_CACHE_MANAGER__H

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
#include <shared_mutex>
#include <string>
#include <unordered_map>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class VulkanGraphicsController;

/**
 * @class PipelineCacheManager
 * @brief Manages Vulkan pipeline caching with application-level and driver-level caching.
 *
 * It provides optimized Vulkan pipeline creation by:
 *
 * - Maintaining an application-level hash map of created pipelines
 * - Utilizing Vulkan's internal pipeline cache mechanism
 * - Persisting cache data across application runs with CRC32 validation
 * - Ensuring thread-safe operations
 */
class PipelineCacheManager final
{
public:
  /**
   * @brief Constructs the pipeline cache manager
   * @param controller Vulkan graphics controller
   */
  PipelineCacheManager(VulkanGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~PipelineCacheManager();

  PipelineCacheManager(const PipelineCacheManager&) = delete;
  PipelineCacheManager& operator=(const PipelineCacheManager&) = delete;

  /**
   * @brief Retrieves an existing pipeline or creates a new one
   * @param createInfo Vulkan graphics pipeline creation parameters
   * @return Valid Vulkan pipeline handle
   * @throws std::runtime_error if pipeline creation fails
   */
  vk::Pipeline GetOrCreatePipeline(const vk::GraphicsPipelineCreateInfo& createInfo);

  /**
   * @brief Persists pipeline cache data to disk
   * @note Automatically called during destruction
   */
  void SaveCacheData();

  /**
   * @brief Invalidates the application-level pipeline cache
   */
  void InvalidatePipelineCache();

  /**
   * @brief Removes a pipeline from the main cache and moves it to orphaned list for safe destruction
   * @param pipeline The Vulkan pipeline to remove from cache
   */
  void RemovePipelineFromCache(vk::Pipeline pipeline);

private:
  /**
   * @brief Computes hash from pipeline creation parameters
   * @param createInfo Pipeline creation parameters
   * @return Unique hash value for the pipeline configuration
   */
  size_t ComputePipelineHash(const vk::GraphicsPipelineCreateInfo& createInfo) const;

  /**
   * @brief Initializes Vulkan pipeline cache object
   * @note Attempts to load existing cache data, falls back to new cache
   */
  void InitializeVulkanCache();

  /**
   * @brief Loads and validates cache data from disk
   * @param[out] data Output buffer for validated cache data
   * @return true if valid cache data was loaded, false otherwise
   */
  bool LoadAndValidateCache(std::vector<uint8_t>& data);

  /**
   * @brief Creates new empty Vulkan pipeline cache
   * @throws std::runtime_error if cache creation fails
   */
  void CreateNewVulkanCache();

private:
#pragma pack(push, 1)
  /**
   * @struct PipelineCacheHeader
   * @brief Binary header for cache file validation
   */
  struct PipelineCacheHeader
  {
    uint32_t magic{0};                          ///< Magic number
    uint32_t dataSize{0};                       ///< Size of pipeline cache data in bytes
    uint32_t crcChecksum{0};                    ///< CRC32 checksum of cache data
    uint32_t vendorID{0};                       ///< GPU vendor ID (VkPhysicalDeviceProperties::vendorID)
    uint32_t deviceID{0};                       ///< GPU device ID (VkPhysicalDeviceProperties::deviceID)
    uint32_t driverVersion{0};                  ///< Driver version (VkPhysicalDeviceProperties::driverVersion)
    uint32_t driverABI{0};                      ///< Size of pointer (32/64-bit validation)
    uint8_t  pipelineCacheUUID[VK_UUID_SIZE]{}; ///< Pipeline cache UUID from device properties
  };
#pragma pack(pop)

  Vulkan::Device&                                mDevice;      ///< Vulkan device
  vk::PhysicalDeviceProperties                   mDeviceProps; ///< Cached device properties for validation
  vk::UniquePipelineCache                        mVulkanCache; ///< Vulkan pipeline cache object
  std::unordered_map<size_t, vk::UniquePipeline> mPipelineMap; ///< Application-level pipeline cache
  std::vector<vk::UniquePipeline>                mOrphanedPipelines; ///< Temporarily stored pipelines for safe destruction
  mutable std::shared_mutex                      mCacheMutex;  ///< Mutex for thread-safe map access

  std::string mCacheFilePath; ///< Absolute path to cache file
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_PIPELINE_CACHE_MANAGER__H
