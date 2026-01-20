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
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline-cache-manager.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/integration-api/debug.h>
#include <filesystem>
#include <fstream>
#include <limits>
#if defined(DEBUG_ENABLED)
#include <chrono>
#endif
#include <string_view>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

extern std::string GetSystemProgramBinaryPath();

namespace
{
#if defined(DEBUG_ENABLED)

Debug::Filter* gVulkanPipelineLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VULKAN_PIPELINE");

#endif // DEBUG_ENABLED

static const uint32_t PIPELINE_CACHE_HEADER_MAGIC = 0x44414C49; // "DALI" magic number (0x44414C49 in hex)

// CRC-32 checksum calculation for data validation

// clang-format off

// Precomputed CRC32 lookup table (polynomial 0xEDB88320)
const uint32_t CRC32_TABLE[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

// clang-format on

/**
 * @brief Computes CRC32 checksum for a data buffer
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @return 32-bit CRC checksum
 */
uint32_t CRC32Calculate(const uint8_t* data, size_t length)
{
  uint32_t crc = ~0u;
  for(size_t i = 0; i < length; ++i)
    crc = CRC32_TABLE[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  return (crc ^ ~0u);
}

// Type trait to detect vk::Flags
template<typename>
struct is_vk_flags : std::false_type
{
};

template<typename E>
struct is_vk_flags<vk::Flags<E>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_vk_flags_v = is_vk_flags<T>::value;

/**
 * @brief Combines a hash value with another value
 * @tparam T Type of value to combine
 * @param seed Current hash (modified in-place)
 * @param value Value to incorporate into hash
 */
template<typename T>
void HashCombine(size_t& seed, const T& value) noexcept
{
  // Handle Vulkan types by casting to their underlying Vk handle type
  if constexpr(std::is_same_v<T, vk::PipelineLayout>)
  {
    HashCombine(seed, static_cast<VkPipelineLayout>(value));
  }
  else if constexpr(std::is_same_v<T, vk::RenderPass>)
  {
    HashCombine(seed, static_cast<VkRenderPass>(value));
  }
  else if constexpr(std::is_same_v<T, vk::ShaderModule>)
  {
    HashCombine(seed, static_cast<VkShaderModule>(value));
  }
  else if constexpr(is_vk_flags_v<T>)
  {
    // Handle vk::Flags
    using MaskType = typename T::MaskType;
    HashCombine(seed, static_cast<MaskType>(value));
  }
  else
  {
    // Generic hashing for standard types
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
}

// Specialization for uint64_t (base case)
template<>
void HashCombine<uint64_t>(size_t& seed, const uint64_t& value) noexcept
{
  seed ^= std::hash<uint64_t>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/**
 * @brief Computes hash from critical pipeline state parameters
 * @param seed Initial hash value (modified in-place)
 * @param ci Pipeline creation parameters to hash
 */
void HashPipelineState(size_t& seed, const vk::GraphicsPipelineCreateInfo& ci)
{
  // Core pipeline configuration
  HashCombine(seed, ci.flags);
  HashCombine(seed, ci.layout);
  HashCombine(seed, ci.renderPass);
  HashCombine(seed, ci.subpass);

  // Shader stages (modules + entry points + specialization)
  for(uint32_t i = 0; i < ci.stageCount; ++i)
  {
    const auto& stage = ci.pStages[i];
    HashCombine(seed, stage.stage);
    HashCombine(seed, stage.module);
    HashCombine(seed, stage.pName ? std::string_view(stage.pName) : std::string_view());

    if(stage.pSpecializationInfo)
    {
      HashCombine(seed, stage.pSpecializationInfo->mapEntryCount);
      HashCombine(seed, stage.pSpecializationInfo->dataSize);
      HashCombine(seed, std::string_view(reinterpret_cast<const char*>(stage.pSpecializationInfo->pData), stage.pSpecializationInfo->dataSize));
    }
  }

  // Vertex input state
  if(ci.pVertexInputState)
  {
    for(uint32_t i = 0; i < ci.pVertexInputState->vertexBindingDescriptionCount; ++i)
    {
      const auto& binding = ci.pVertexInputState->pVertexBindingDescriptions[i];
      HashCombine(seed, binding.binding);
      HashCombine(seed, binding.stride);
      HashCombine(seed, binding.inputRate);
    }
    for(uint32_t i = 0; i < ci.pVertexInputState->vertexAttributeDescriptionCount; ++i)
    {
      const auto& attr = ci.pVertexInputState->pVertexAttributeDescriptions[i];
      HashCombine(seed, attr.location);
      HashCombine(seed, attr.binding);
      HashCombine(seed, attr.format);
      HashCombine(seed, attr.offset);
    }
  }

  // Input assembly
  if(ci.pInputAssemblyState)
  {
    HashCombine(seed, ci.pInputAssemblyState->topology);
    HashCombine(seed, ci.pInputAssemblyState->primitiveRestartEnable);
  }

  // Tessellation state
  if(ci.pTessellationState)
  {
    HashCombine(seed, ci.pTessellationState->sType);
    HashCombine(seed, ci.pTessellationState->patchControlPoints);
  }

  // Viewport state
  if(ci.pViewportState)
  {
    HashCombine(seed, ci.pViewportState->viewportCount);
    HashCombine(seed, ci.pViewportState->scissorCount);
  }

  // Rasterization state
  if(ci.pRasterizationState)
  {
    const auto& rs = *ci.pRasterizationState;
    HashCombine(seed, rs.depthBiasConstantFactor);
    HashCombine(seed, rs.depthBiasClamp);
    HashCombine(seed, rs.depthBiasSlopeFactor);
    HashCombine(seed, rs.depthClampEnable);
    HashCombine(seed, rs.rasterizerDiscardEnable);
    HashCombine(seed, rs.polygonMode);
    HashCombine(seed, rs.cullMode);
    HashCombine(seed, rs.frontFace);
    HashCombine(seed, rs.depthBiasEnable);
    HashCombine(seed, rs.lineWidth);
  }

  // Multisample state
  if(ci.pMultisampleState)
  {
    const auto& ms = *ci.pMultisampleState;

    if(ms.pSampleMask)
    {
      const uint32_t wordCount = (static_cast<uint32_t>(ms.rasterizationSamples) + 31) / 32;
      for(uint32_t i = 0; i < wordCount; ++i)
      {
        HashCombine(seed, ms.pSampleMask[i]);
      }
    }
    HashCombine(seed, ms.rasterizationSamples);
    HashCombine(seed, ms.sampleShadingEnable);
    HashCombine(seed, ms.minSampleShading);
    HashCombine(seed, ms.alphaToCoverageEnable);
    HashCombine(seed, ms.alphaToOneEnable);
  }

  // Depth/stencil state
  if(ci.pDepthStencilState)
  {
    const auto& ds = *ci.pDepthStencilState;
    HashCombine(seed, ds.depthTestEnable);
    HashCombine(seed, ds.depthWriteEnable);
    HashCombine(seed, ds.depthCompareOp);
    HashCombine(seed, ds.depthBoundsTestEnable);
    HashCombine(seed, ds.stencilTestEnable);
    HashCombine(seed, ds.front.failOp);
    HashCombine(seed, ds.front.passOp);
    HashCombine(seed, ds.front.depthFailOp);
    HashCombine(seed, ds.front.compareOp);
    HashCombine(seed, ds.back.failOp);
    HashCombine(seed, ds.back.passOp);
    HashCombine(seed, ds.back.depthFailOp);
    HashCombine(seed, ds.back.compareOp);
  }

  // Color blend state
  if(ci.pColorBlendState)
  {
    const auto& cb = *ci.pColorBlendState;
    HashCombine(seed, cb.logicOpEnable);
    HashCombine(seed, cb.logicOp);
    HashCombine(seed, cb.attachmentCount);

    for(uint32_t i = 0; i < cb.attachmentCount; ++i)
    {
      const auto& attach = cb.pAttachments[i];
      HashCombine(seed, attach.blendEnable);
      HashCombine(seed, attach.srcColorBlendFactor);
      HashCombine(seed, attach.dstColorBlendFactor);
      HashCombine(seed, attach.colorBlendOp);
      HashCombine(seed, attach.srcAlphaBlendFactor);
      HashCombine(seed, attach.dstAlphaBlendFactor);
      HashCombine(seed, attach.alphaBlendOp);
      HashCombine(seed, attach.colorWriteMask);
    }
  }

  // Dynamic state
  if(ci.pDynamicState)
  {
    for(uint32_t i = 0; i < ci.pDynamicState->dynamicStateCount; ++i)
    {
      HashCombine(seed, ci.pDynamicState->pDynamicStates[i]);
    }
  }
}

} // anonymous namespace

namespace Dali::Graphics::Vulkan
{
PipelineCacheManager::PipelineCacheManager(VulkanGraphicsController& controller)
: mDevice(controller.GetGraphicsDevice()),
  mDeviceProps(mDevice.GetPhysicalDeviceProperties())
{
  std::string dataPath = GetSystemProgramBinaryPath();

  mCacheFilePath = dataPath + "pipeline-cache.bin";

  DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose, "Pipeline Cache File Path: %s\n", mCacheFilePath.c_str());

  InitializeVulkanCache();

  DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose, "Pipeline cache initialized\n");
}

PipelineCacheManager::~PipelineCacheManager()
{
  try
  {
    SaveCacheData();
    DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose, "Pipeline cache saved\n");
  }
  catch(const std::exception& e)
  {
    DALI_LOG_ERROR("Failed to save cache during destruction: %s", e.what());
  }
}

vk::Pipeline PipelineCacheManager::GetOrCreatePipeline(const vk::GraphicsPipelineCreateInfo& createInfo)
{
  const size_t hash = ComputePipelineHash(createInfo);

  // Performance measurement start
#if defined(DEBUG_ENABLED)
  auto startTime = std::chrono::high_resolution_clock::now();
#endif

  {
    // Read lock (shared ownership) - using std::mutex for compatibility
    mCacheMutex.lock();
    auto it = mPipelineMap.find(hash);
    if(it != mPipelineMap.end())
    {
      mCacheMutex.unlock();
#if defined(DEBUG_ENABLED)
      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][Memory] pipeline cache HIT : hash:%zu, lookup_time:%ld μs\n", hash, duration.count());
#endif
      return it->second.get();
    }
    mCacheMutex.unlock();
  }

  // For write operations (exclusive ownership)
  mCacheMutex.lock();

  // Double-check after acquiring write lock
  auto it = mPipelineMap.find(hash);
  if(it != mPipelineMap.end())
  {
    mCacheMutex.unlock();
#if defined(DEBUG_ENABLED)
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][Memory] pipeline cache HIT(late) : hash:%zu, lookup_time:%ld μs\n", hash, duration.count());
#endif
    return it->second.get();
  }

  DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][Memory] pipeline cache MISS, need to create file cache: hash:%zu, cache_size:%zu\n", hash, mPipelineMap.size());

  // Create and store pipeline
#if defined(DEBUG_ENABLED)
  auto creationStartTime = std::chrono::high_resolution_clock::now();
#endif
  auto [result, pipeline] = mDevice.GetLogicalDevice().createGraphicsPipelineUnique(*mVulkanCache, createInfo, mDevice.GetAllocator());

#if defined(DEBUG_ENABLED)
  auto creationEndTime = std::chrono::high_resolution_clock::now();
  auto creationDuration = std::chrono::duration_cast<std::chrono::microseconds>(creationEndTime - creationStartTime);
#endif

  if(result != vk::Result::eSuccess)
  {
    mCacheMutex.unlock();
    throw std::runtime_error("Pipeline creation failed: " + vk::to_string(result));
  }

  auto [newIt, _] = mPipelineMap.emplace(hash, std::move(pipeline));
  mCacheMutex.unlock();

  // Performance measurement for cache miss
#if defined(DEBUG_ENABLED)
  auto endTime = std::chrono::high_resolution_clock::now();
  auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

  DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][File] create pipeline using cash: hash:%zu, total_time:%ld μs, creation_time:%ld μs, cache_size:%zu\n",
                 hash, totalDuration.count(), creationDuration.count(), mPipelineMap.size());
#endif

  return newIt->second.get();
}


void PipelineCacheManager::RemovePipelineFromCache(vk::Pipeline pipeline)
{
  // Move pipeline from main cache to orphaned list for safe destruction
  mCacheMutex.lock();
  for(auto it = mPipelineMap.begin(); it != mPipelineMap.end(); ++it)
  {
    if(it->second.get() == pipeline)
    {
      // Move ownership to orphaned list instead of destroying immediately
      mOrphanedPipelines.push_back(std::move(it->second));
      mPipelineMap.erase(it);
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][Memory] Moved pipeline %p to orphaned list for safe destruction\n", static_cast<VkPipeline>(pipeline));
      break;
    }
  }
  mCacheMutex.unlock();
}

size_t PipelineCacheManager::ComputePipelineHash(const vk::GraphicsPipelineCreateInfo& createInfo) const
{
  size_t hash = 0;
  HashPipelineState(hash, createInfo);
  return hash;
}

void PipelineCacheManager::InitializeVulkanCache()
{
  DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][File] Enable Pipeline Cache \n");

  std::vector<uint8_t> cacheData;
  if(LoadAndValidateCache(cacheData))
  {
    vk::PipelineCacheCreateInfo createInfo;
    createInfo.initialDataSize = cacheData.size();
    createInfo.pInitialData    = cacheData.data();

    auto [result, pipelineCache] = mDevice.GetLogicalDevice().createPipelineCacheUnique(createInfo, mDevice.GetAllocator());
    if(result == vk::Result::eSuccess)
    {
      mVulkanCache = std::move(pipelineCache);
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][File] Cache HIT, Create PipelineCache using existing cache, data'size :%d \n", cacheData.size());
    }
    else
    {
      DALI_LOG_WARNING("Vulkan cache creation failed: %s\n", vk::to_string(result));
      CreateNewVulkanCache();
    }
  }
  else
  {
    CreateNewVulkanCache();
  }
}

void PipelineCacheManager::CreateNewVulkanCache()
{
  vk::PipelineCacheCreateInfo createInfo;
  auto [result, pipelineCache] = mDevice.GetLogicalDevice().createPipelineCacheUnique(createInfo, mDevice.GetAllocator());
  if(result == vk::Result::eSuccess)
  {
    mVulkanCache = std::move(pipelineCache);
    DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][File] Cache MISS, Create PipelineCache without exisiting cache \n");
  }
  else
  {
    throw std::runtime_error("Failed to create Vulkan pipeline cache: " + vk::to_string(result));
  }
}

bool PipelineCacheManager::LoadAndValidateCache(std::vector<uint8_t>& data)
{
  std::ifstream file(mCacheFilePath, std::ios::binary | std::ios::ate);
  if(!file.is_open())
  {
    DALI_LOG_ERROR("No existing pipeline cache found\n");
    return false;
  }

  const size_t fileSize = static_cast<size_t>(file.tellg());
  file.seekg(0);

  // Read and validate header
  PipelineCacheHeader header;
  if(fileSize < sizeof(header))
  {
    DALI_LOG_ERROR("Cache file too small, invalid header\n");
    return false;
  }
  file.read(reinterpret_cast<char*>(&header), sizeof(header));

  // Validate magic number
  if(header.magic != PIPELINE_CACHE_HEADER_MAGIC)
  {
    DALI_LOG_ERROR("Invalid cache file magic number\n");
    return false;
  }

  // Validate device compatibility
  const bool deviceMatch =
    header.vendorID == mDeviceProps.vendorID &&
    header.deviceID == mDeviceProps.deviceID &&
    header.driverVersion == mDeviceProps.driverVersion &&
    header.driverABI == sizeof(void*) &&
    memcmp(header.pipelineCacheUUID, mDeviceProps.pipelineCacheUUID, VK_UUID_SIZE) == 0;

  if(!deviceMatch)
  {
    DALI_LOG_ERROR("Cache device compatibility check failed\n");
    return false;
  }

  // Read and validate data
  const size_t dataSize = fileSize - sizeof(header);
  if(dataSize != header.dataSize)
  {
    DALI_LOG_ERROR("Cache data size mismatch\n");
    return false;
  }

  // Check that dataSize fits in std::streamsize
  if(dataSize > static_cast<size_t>(std::numeric_limits<std::streamsize>::max()))
  {
    DALI_LOG_ERROR("Data size exceeds maximum streamsize\n");
    return false;
  }

  std::vector<uint8_t> fileData(dataSize);
  if(!file.read(reinterpret_cast<char*>(fileData.data()), dataSize))
  {
    DALI_LOG_ERROR("Failed to read cache data\n");
    return false;
  }

  // Validate CRC
  const uint32_t computedCRC = CRC32Calculate(fileData.data(), dataSize);
  if(computedCRC != header.crcChecksum)
  {
    DALI_LOG_ERROR("Cache data CRC validation failed\n");
    return false;
  }

  data.swap(fileData);
  DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][File] Find exisiting cache from file(%s) for creating Vulkan Pipeline Cache, data'size :%d\n", mCacheFilePath.c_str(), data.size());
  return true;
}

void PipelineCacheManager::SaveCacheData()
{
  if(mCacheFilePath.empty())
  {
    return;
  }

  try
  {
    if(!mVulkanCache)
    {
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose, "No Vulkan cache to save\n");
      return;
    }

    const auto [result, cacheData] = mDevice.GetLogicalDevice().getPipelineCacheData(*mVulkanCache);

    // Check if the operation was successful
    if(result != vk::Result::eSuccess || cacheData.empty())
    {
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose, "Failed to load Vulkan pipeline cache data: %s\n", vk::to_string(result));
      return;
    }

    // Check that cacheData.size() fits in std::streamsize
    if(cacheData.size() > static_cast<size_t>(std::numeric_limits<std::streamsize>::max()))
    {
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose, "Cache data size exceeds maximum streamsize\n");
      return;
    }

    // Prepare header
    PipelineCacheHeader header;
    header.magic         = PIPELINE_CACHE_HEADER_MAGIC;
    header.dataSize      = static_cast<uint32_t>(cacheData.size());
    header.vendorID      = mDeviceProps.vendorID;
    header.deviceID      = mDeviceProps.deviceID;
    header.driverVersion = mDeviceProps.driverVersion;
    header.driverABI     = sizeof(void*);
    memcpy(header.pipelineCacheUUID, mDeviceProps.pipelineCacheUUID, VK_UUID_SIZE);
    header.crcChecksum = CRC32Calculate(cacheData.data(), cacheData.size());

    // Write to cache file
    const std::string filePath = mCacheFilePath;
    {
      std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
      file.write(reinterpret_cast<const char*>(&header), sizeof(header));
      file.write(reinterpret_cast<const char*>(cacheData.data()), cacheData.size());
      DALI_LOG_INFO(gVulkanPipelineLogFilter, Debug::Verbose,"[Pipeline Cache][File] Save new Pipeline to cache Data size:%d, path:%s \n",cacheData.size(), mCacheFilePath.c_str());
    }
  }
  catch(const std::exception& e)
  {
    DALI_LOG_ERROR("[Pipeline Cache][File] Cache save failed: %s\n", e.what());
  }
}

} // namespace Dali::Graphics::Vulkan
