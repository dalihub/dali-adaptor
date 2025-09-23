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
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>

// INTERNAL HEADERS
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/common/shader-parser.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

// EXTERNAL HEADERS
#include <iostream>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gGraphicsProgramLogFilter;
#endif

namespace Dali::Graphics::Vulkan
{
struct ProgramImpl::Impl
{
  explicit Impl(VulkanGraphicsController& _controller, const ProgramCreateInfo& info)
  : controller(_controller)
  {
    createInfo = info;
    if(info.shaderState)
    {
      createInfo.shaderState = new std::vector<ShaderState>(*info.shaderState);
    }

    // Create new reference of std::string_view.
    name            = std::string(info.name);
    createInfo.name = name;
  }

  ~Impl()
  {
    // Tear down all VkDescriptorPool handles we ever created
    auto& gfxDevice = controller.GetGraphicsDevice();
    auto  vkDevice  = gfxDevice.GetLogicalDevice();
    auto& allocator = gfxDevice.GetAllocator();

    for(auto& frame : frameResources)
    {
      for(auto& pool : frame.descriptorPools)
      {
        vkDevice.destroyDescriptorPool(pool, &allocator);
      }
    }

    delete createInfo.shaderState;
  }

  std::vector<vk::DescriptorPoolSize> CalculatePoolSizes(uint32_t setCount)
  {
    // Build poolSizes from reflection
    std::vector<vk::DescriptorPoolSize> poolSizes;
    auto                                uniformBlockCount = reflection->GetUniformBlockCount() - 1; // skip GLES emulation
    auto                                samplersCount     = reflection->GetSamplers().size();

    if(uniformBlockCount)
    {
      vk::DescriptorPoolSize item;
      item.setType(vk::DescriptorType::eUniformBuffer);
      item.setDescriptorCount(uniformBlockCount * setCount);
      poolSizes.emplace_back(item);
    }
    if(samplersCount)
    {
      vk::DescriptorPoolSize item;
      item.setType(vk::DescriptorType::eCombinedImageSampler);
      item.setDescriptorCount(samplersCount * setCount);
      poolSizes.emplace_back(item);
    }

    return poolSizes;
  }

  VulkanGraphicsController& controller;
  ProgramCreateInfo         createInfo;
  std::string               name;
  uint32_t                  refCount{0u};

  std::unique_ptr<Vulkan::Reflection> reflection{nullptr};

  std::vector<vk::PipelineShaderStageCreateInfo> mPipelineShaderStageCreateInfoList;
  std::vector<ShaderHandle>                      mShaderList;

  struct DescriptorPool
  {
    vk::DescriptorPoolCreateInfo createInfo;
    vk::DescriptorPool           vkPool;
  };

  std::vector<DescriptorPool> poolList;            ///< List of descriptor pools. Each element corresponds to overall bufferIndex.
  uint32_t                    currentPoolIndex{0}; ///< Current pool index matches bufferIndex

  struct FrameResources
  {
    std::vector<vk::DescriptorPool> descriptorPools;    // List of descriptor pools
    std::vector<vk::DescriptorSet>  freeSets;           // Available sets for use
    std::vector<vk::DescriptorSet>  usedSets;           // Sets currently in use
    uint32_t                        currentCapacity{0}; // Current pool capacity
  };

  std::vector<FrameResources> frameResources; ///< Per-frame resources
};

ProgramImpl::ProgramImpl(const Graphics::ProgramCreateInfo& createInfo, VulkanGraphicsController& controller)
{
  // Create implementation
  mImpl = std::make_unique<Impl>(controller, createInfo);

  // Preprocess source code, if successful, compile shaders
  bool success = true;
  if(Preprocess())
  {
    for(const auto& state : *createInfo.shaderState)
    {
      auto shader     = static_cast<const Vulkan::Shader*>(state.shader);
      auto shaderImpl = shader->GetImplementation();

      // Keep shader object alive
      mImpl->mShaderList.push_back(shaderImpl);

      if(!shaderImpl->Compile())
      {
        DALI_LOG_ERROR("SPIRV Compilation failed!\n");
        success = false;
      }
    }
  }

  if(success)
  {
    // Build reflection
    mImpl->reflection = std::make_unique<Vulkan::Reflection>(*this, controller);
  }

  // Create shader stages for pipeline creation
  for(const auto& state : *createInfo.shaderState)
  {
    mImpl->mPipelineShaderStageCreateInfoList.emplace_back();
    auto& info = mImpl->mPipelineShaderStageCreateInfoList.back();
    info.setModule(static_cast<const Vulkan::Shader*>(state.shader)->GetImplementation()->GetVkShaderModule());
    if(state.pipelineStage == PipelineStage::VERTEX_SHADER)
    {
      info.setStage(vk::ShaderStageFlagBits::eVertex);
      info.setPName("main");
    }
    if(state.pipelineStage == PipelineStage::FRAGMENT_SHADER)
    {
      info.setStage(vk::ShaderStageFlagBits::eFragment);
      info.setPName("main");
    }
    else
    {
      DALI_ASSERT_ALWAYS(true && "Invalid pipeline shader stage!");
    }
  }
}

ProgramImpl::~ProgramImpl() = default;

bool ProgramImpl::Destroy()
{
  mImpl.reset();
  return false;
}

bool ProgramImpl::Preprocess()
{
  // For now only Vertex and Fragment shader stages supported
  // and one per stage
  std::string  vertexString;
  std::string  fragmentString;
  std::string* currentString = nullptr;

  const Vulkan::Shader* vsh = nullptr;
  const Vulkan::Shader* fsh = nullptr;

  const auto& info = mImpl->createInfo;

  for(const auto& state : *info.shaderState)
  {
    const auto* shader = static_cast<const Vulkan::Shader*>(state.shader);
    if(state.pipelineStage == PipelineStage::VERTEX_SHADER)
    {
      // Only TEXT source mode can be processed
      currentString = &vertexString;
      vsh           = shader;
    }
    else if(state.pipelineStage == PipelineStage::FRAGMENT_SHADER)
    {
      // Only TEXT source mode can be processed
      currentString = &fragmentString;
      fsh           = shader;
    }
    else
    {
      // no valid stream to push
      currentString = nullptr;
      DALI_LOG_ERROR("Shader state contains invalid shader source (most likely binary)! Can't process!");
    }

    // Check if stream valid
    if(currentString && currentString->empty() && shader->GetCreateInfo().sourceMode == ShaderSourceMode::TEXT)
    {
      *currentString = std::string(reinterpret_cast<const char*>(shader->GetCreateInfo().sourceData),
                                   shader->GetCreateInfo().sourceSize);
    }
    else
    {
      DALI_LOG_ERROR("Preprocessing of binary shaders isn't allowed!");
    }
  }

  // if we have both streams ready
  if(!vertexString.empty() && !fragmentString.empty())
  {
    // In case we have one modern shader and one legacy counterpart we need to enforce
    // output language.
    Internal::ShaderParser::ShaderParserInfo parseInfo{};
    parseInfo.vertexShaderCode            = &vertexString;
    parseInfo.fragmentShaderCode          = &fragmentString;
    parseInfo.vertexShaderLegacyVersion   = vsh->GetGLSLVersion();
    parseInfo.fragmentShaderLegacyVersion = fsh->GetGLSLVersion();
    parseInfo.language                    = Internal::ShaderParser::OutputLanguage::SPIRV_GLSL;
    parseInfo.outputVersion               = std::max(vsh->GetGLSLVersion(), fsh->GetGLSLVersion());

    std::vector<std::string> newShaders;

    Internal::ShaderParser::Parse(parseInfo, newShaders);

    // substitute shader code
    vsh->GetImplementation()->SetPreprocessedCode(newShaders[0].data(), newShaders[0].size());
    fsh->GetImplementation()->SetPreprocessedCode(newShaders[1].data(), newShaders[1].size());

    return true;
  }
  else
  {
    DALI_LOG_ERROR("Preprocessing shader code failed!");
    return false;
  }
}

bool ProgramImpl::Create()
{
  // TODO: redirect to the reflection builder
  return true;
}

uint32_t ProgramImpl::GetSPIRVProgram() const
{
  return {};
}

uint32_t ProgramImpl::Retain()
{
  return ++mImpl->refCount;
}

uint32_t ProgramImpl::Release()
{
  return --mImpl->refCount;
}

uint32_t ProgramImpl::GetRefCount() const
{
  return mImpl->refCount;
}

const Vulkan::Reflection& ProgramImpl::GetReflection() const
{
  return *mImpl->reflection;
}

bool ProgramImpl::GetParameter(uint32_t parameterId, void* out)
{
  return false;
}

VulkanGraphicsController& ProgramImpl::GetController() const
{
  return mImpl->controller;
}

const ProgramCreateInfo& ProgramImpl::GetCreateInfo() const
{
  return mImpl->createInfo;
}

[[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& ProgramImpl::GetVkPipelineShaderStageCreateInfoList() const
{
  return mImpl->mPipelineShaderStageCreateInfoList;
}

void ProgramImpl::AddDescriptorPool(uint32_t initialCapacity)
{
  auto& gfxDevice         = mImpl->controller.GetGraphicsDevice();
  auto& allocator         = gfxDevice.GetAllocator();
  auto  vkDevice          = gfxDevice.GetLogicalDevice();
  auto  maxFramesInFlight = gfxDevice.GetBufferCount();

  mImpl->frameResources.resize(maxFramesInFlight);

  auto poolSizes = mImpl->CalculatePoolSizes(initialCapacity);

  for(uint32_t i = 0; i < maxFramesInFlight; ++i)
  {
    auto& frame = mImpl->frameResources[i];

    if(initialCapacity > frame.currentCapacity)
    {
      // Create new pool with initial capacity
      vk::DescriptorPoolCreateInfo poolInfo;
      poolInfo.setMaxSets(initialCapacity);
      poolInfo.setPoolSizes(poolSizes);

      vk::DescriptorPool newPool;
      VkAssert(vkDevice.createDescriptorPool(&poolInfo, &allocator, &newPool));

      frame.descriptorPools.push_back(newPool);
      frame.currentCapacity += initialCapacity;

      PreAllocateDescriptorSetsFromPool(i, newPool, initialCapacity);
    }

    ResetDescriptorSetsForFrame(i); // Prepare free set list
  }
}

void ProgramImpl::PreAllocateDescriptorSetsFromPool(uint32_t frameIndex, vk::DescriptorPool pool, uint32_t setCount)
{
  if(frameIndex >= mImpl->frameResources.size())
  {
    return;
  }

  auto& frame     = mImpl->frameResources[frameIndex];
  auto& gfxDevice = mImpl->controller.GetGraphicsDevice();
  auto  vkDevice  = gfxDevice.GetLogicalDevice();
  auto& layouts   = GetReflection().GetVkDescriptorSetLayouts();

  // Prepare layouts for batch allocation
  std::vector<vk::DescriptorSetLayout> setLayouts(setCount, layouts[0]);

  vk::DescriptorSetAllocateInfo allocInfo;
  allocInfo.setDescriptorPool(pool);
  allocInfo.setDescriptorSetCount(setCount);
  allocInfo.setSetLayouts(setLayouts);

  // Allocate all sets in one batch
  std::vector<vk::DescriptorSet> newSets(setCount);
  VkAssert(vkDevice.allocateDescriptorSets(&allocInfo, newSets.data()));

  // Update free list to include all pre-allocated sets
  frame.freeSets.insert(frame.freeSets.end(), newSets.begin(), newSets.end());
}

[[nodiscard]] bool ProgramImpl::GrowDescriptorPool(uint32_t frameIndex, uint32_t newCapacity)
{
  if(frameIndex >= mImpl->frameResources.size())
  {
    return false;
  }

  auto& frame = mImpl->frameResources[frameIndex];

  uint32_t growSize = newCapacity - frame.currentCapacity;
  if(growSize == 0)
  {
    return false;
  }

  auto& gfxDevice = mImpl->controller.GetGraphicsDevice();
  auto& allocator = gfxDevice.GetAllocator();
  auto  vkDevice  = gfxDevice.GetLogicalDevice();

  // Create descriptor pool sizes for new capacity
  auto poolSizes = mImpl->CalculatePoolSizes(newCapacity);

  vk::DescriptorPoolCreateInfo poolInfo;
  poolInfo.setMaxSets(growSize);
  poolInfo.setPoolSizes(poolSizes);

  // Create new descriptor pool to meet the new capacity
  vk::DescriptorPool newPool;
  VkAssert(vkDevice.createDescriptorPool(&poolInfo, &allocator, &newPool));

  frame.descriptorPools.push_back(newPool);
  frame.currentCapacity += growSize;

  // Pre-allocate all sets for the new capacity
  PreAllocateDescriptorSetsFromPool(frameIndex, newPool, growSize);

  return true;
}

vk::DescriptorSet ProgramImpl::GetNextDescriptorSetForFrame(uint32_t frameIndex)
{
  if(frameIndex >= mImpl->frameResources.size())
  {
    return VK_NULL_HANDLE;
  }

  auto& frame = mImpl->frameResources[frameIndex];

  // Check if we need to grow the pool
  if(frame.freeSets.empty())
  {
    // Grow the pool capacity by 50% each time
    uint32_t newCapacity = static_cast<uint32_t>(frame.currentCapacity * 1.5);
    if(newCapacity > frame.currentCapacity)
    {
      if(!GrowDescriptorPool(frameIndex, newCapacity))
      {
        return VK_NULL_HANDLE;
      }
    }
    else
    {
      return VK_NULL_HANDLE; // Hit maximum capacity
    }
  }

  // Pop from free list
  vk::DescriptorSet set = frame.freeSets.back();
  frame.freeSets.pop_back();
  frame.usedSets.push_back(set);

  return set;
}

void ProgramImpl::ResetDescriptorSetsForFrame(uint32_t frameIndex)
{
  if(frameIndex < mImpl->frameResources.size())
  {
    auto& frame = mImpl->frameResources[frameIndex];

    // Refresh free/used lists (mark the descriptor sets available for reuse)
    frame.freeSets.reserve(frame.currentCapacity);

    for(auto& set : frame.usedSets)
    {
      frame.freeSets.push_back(set);
    }
    frame.usedSets.clear();
  }
}

}; // namespace Dali::Graphics::Vulkan
