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
 *
 */

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline-impl.h>

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali::Graphics::Vulkan
{
namespace
{
inline uint32_t HashDepthStencilState(vk::PipelineDepthStencilStateCreateInfo& state)
{
  // Use bit mixing and prime multiplication to reduce collisions
  // Based on xxHash-like approach for good distribution

  // Well-chosen primes for good bit distribution
  const uint32_t PRIME1 = 2654435761U; // 2^32 / golden ratio
  const uint32_t PRIME2 = 2246822519U; // Large prime for mixing
  const uint32_t PRIME3 = 3266489917U; // Another large prime

  // Convert float to uint32_t without changing bit pattern
  struct FloatToInt
  {
    FloatToInt(float f)
    {
      v0 = f;
    }
    union
    {
      float    v0;
      uint32_t v1;
    };
  };

  // Start with prime seed for good initial distribution
  uint32_t hash = PRIME1;

  // Mix depth test state with bit shifting for better distribution
  hash = (hash * PRIME2) + (uint32_t(state.depthTestEnable) << 16);
  hash = (hash * PRIME3) + (uint32_t(state.depthWriteEnable) << 8);
  hash = (hash * PRIME2) + (uint32_t(state.depthCompareOp) << 24);
  hash = (hash * PRIME3) + (uint32_t(state.depthBoundsTestEnable) << 12);
  hash = (hash * PRIME2) + (uint32_t(state.stencilTestEnable) << 4);

  // Mix depth bounds using XOR for different mixing behavior
  hash = (hash * PRIME3) ^ FloatToInt(state.minDepthBounds).v1;
  hash = (hash * PRIME2) ^ FloatToInt(state.maxDepthBounds).v1;

  // Hash stencil front state with field-specific mixing
  auto HashStencilOpState = [&hash](const vk::StencilOpState& sop)
  {
    // Mix stencil operations at different bit positions
    hash = (hash * PRIME3) + (static_cast<uint32_t>(sop.failOp) << 24);
    hash = (hash * PRIME2) + (static_cast<uint32_t>(sop.passOp) << 16);
    hash = (hash * PRIME3) + (static_cast<uint32_t>(sop.depthFailOp) << 8);
    hash = (hash * PRIME2) + (static_cast<uint32_t>(sop.compareOp) << 20);

    // Mix stencil masks and reference (small values, no shift needed)
    hash = (hash * PRIME3) + sop.compareMask;
    hash = (hash * PRIME2) + sop.writeMask;
    hash = (hash * PRIME3) + sop.reference;
  };

  // Hash front and back with different mixing to distinguish them
  HashStencilOpState(state.front);
  hash = (hash * PRIME1) + 0x9e3779b9; // Add constant to distinguish front from back
  HashStencilOpState(state.back);

  // Final mixing to avalanche bits
  hash ^= hash >> 16;
  hash *= PRIME2;
  hash ^= hash >> 13;
  hash *= PRIME3;
  hash ^= hash >> 16;

  return hash;
}

constexpr vk::CompareOp ConvCompareOp(const CompareOp in)
{
  switch(in)
  {
    case CompareOp::NEVER:
    {
      return vk::CompareOp::eNever;
    }
    case CompareOp::LESS:
    {
      return vk::CompareOp::eLess;
    }
    case CompareOp::EQUAL:
    {
      return vk::CompareOp::eEqual;
    }
    case CompareOp::LESS_OR_EQUAL:
    {
      return vk::CompareOp::eLessOrEqual;
    }
    case CompareOp::GREATER:
    {
      return vk::CompareOp::eGreater;
    }
    case CompareOp::NOT_EQUAL:
    {
      return vk::CompareOp::eNotEqual;
    }
    case CompareOp::GREATER_OR_EQUAL:
    {
      return vk::CompareOp::eGreaterOrEqual;
    }
    case CompareOp::ALWAYS:
    {
      return vk::CompareOp::eAlways;
    }
  }
  return vk::CompareOp{};
};

constexpr vk::StencilOp ConvStencilOp(const StencilOp in)
{
  switch(in)
  {
    case StencilOp::DECREMENT_AND_CLAMP:
    {
      return vk::StencilOp::eDecrementAndClamp;
    }
    case StencilOp::DECREMENT_AND_WRAP:
    {
      return vk::StencilOp::eDecrementAndWrap;
    }
    case StencilOp::INCREMENT_AND_CLAMP:
    {
      return vk::StencilOp::eIncrementAndClamp;
    }
    case StencilOp::INCREMENT_AND_WRAP:
    {
      return vk::StencilOp::eIncrementAndWrap;
    }
    case StencilOp::INVERT:
    {
      return vk::StencilOp::eInvert;
    }
    case StencilOp::KEEP:
    {
      return vk::StencilOp::eKeep;
    }
    case StencilOp::REPLACE:
    {
      return vk::StencilOp::eReplace;
    }
    case StencilOp::ZERO:
    {
      return vk::StencilOp::eZero;
    }
  }
  return vk::StencilOp{};
};

constexpr vk::StencilOpState ConvStencilOpState(const StencilOpState& in)
{
  vk::StencilOpState out;
  out.compareOp   = ConvCompareOp(in.compareOp);
  out.depthFailOp = ConvStencilOp(in.depthFailOp);
  out.compareMask = in.compareMask;
  out.failOp      = ConvStencilOp(in.failOp);
  out.passOp      = ConvStencilOp(in.passOp);
  out.reference   = in.reference;
  out.writeMask   = in.writeMask;
  return out;
};
} // namespace

/**
 * Copy of pipeline state, can be also used for internal caching
 */
struct PipelineImpl::PipelineState
{
  PipelineState()  = default;
  ~PipelineState() = default;

  // for maintaining correct lifecycle, the owned program
  // wrapper must be created
  UniquePtr<Program> program;

  ColorBlendState    colorBlendState;
  DepthStencilState  depthStencilState;
  ProgramState       programState;
  ViewportState      viewportState;
  RasterizationState rasterizationState;
  VertexInputState   vertexInputState;
  InputAssemblyState inputAssemblyState;
  RenderTarget*      renderTarget{nullptr};
  PipelineCache*     pipelineCache{nullptr};
};

PipelineImpl::PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller, PipelineCache* pipelineCache)
: mController(controller)
{
  // the creation is deferred so it's needed to copy certain parts of the CreateInfo structure
  mPipelineState = std::make_unique<PipelineImpl::PipelineState>();

  // Make copies of structured pass by pointers and replace
  // stored create info structure fields
  CopyStateIfSet(createInfo.inputAssemblyState, mPipelineState->inputAssemblyState, &mCreateInfo.inputAssemblyState);
  CopyStateIfSet(createInfo.vertexInputState, mPipelineState->vertexInputState, &mCreateInfo.vertexInputState);
  CopyStateIfSet(createInfo.rasterizationState, mPipelineState->rasterizationState, &mCreateInfo.rasterizationState);
  CopyStateIfSet(createInfo.programState, mPipelineState->programState, &mCreateInfo.programState);
  CopyStateIfSet(createInfo.colorBlendState, mPipelineState->colorBlendState, &mCreateInfo.colorBlendState);
  CopyStateIfSet(createInfo.depthStencilState, mPipelineState->depthStencilState, &mCreateInfo.depthStencilState);
  CopyStateIfSet(createInfo.viewportState, mPipelineState->viewportState, &mCreateInfo.viewportState);
  mCreateInfo.renderTarget = createInfo.renderTarget;

  InitializePipeline();
}

vk::Pipeline PipelineImpl::CloneInheritedVkPipeline(vk::PipelineDepthStencilStateCreateInfo& dsState)
{
  // Check for render pass compatibility and remove incompatible pipelines from the caches
  ValidateRenderPassCompatibility();

  auto vkDevice = mController.GetGraphicsDevice().GetLogicalDevice();

  auto currentRenderPassImpl = GetCurrentRenderPassImpl();

  // Try to find an existing pipeline that matches the depth state and render pass
  vk::Pipeline existingPipeline = FindExistingPipeline(dsState, currentRenderPassImpl);
  if(existingPipeline)
  {
    return existingPipeline;
  }

  // If no reusable pipeline found, create a new one
  // Copy original info
  vk::GraphicsPipelineCreateInfo gfxPipelineInfo = mVkPipelineCreateInfo;

  // override depth stencil
  gfxPipelineInfo.setPDepthStencilState(&dsState);

  // make sure dynamic depth stencil states are not on
  decltype(mDynamicStates) newDynamicStates = {};

  // Make sure depth and stencil dynamic states are not included
  // in the list of dynamic states
  for(auto& state : mDynamicStates)
  {
    if(!(vk::DynamicState::eDepthWriteEnable == state ||
         vk::DynamicState::eDepthTestEnable == state ||
         vk::DynamicState::eDepthCompareOp == state ||
         vk::DynamicState::eDepthBounds == state ||
         vk::DynamicState::eDepthBoundsTestEnable == state ||
         vk::DynamicState::eStencilTestEnable == state ||
         vk::DynamicState::eStencilOp == state ||
         vk::DynamicState::eStencilCompareMask == state ||
         vk::DynamicState::eStencilWriteMask == state ||
         vk::DynamicState::eStencilReference == state))
    {
      newDynamicStates.emplace_back(state);
    }
  }

  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  dynamicStateInfo.setDynamicStates(newDynamicStates);
  gfxPipelineInfo.setPDynamicState(&dynamicStateInfo);

  // create pipeline
  auto& allocator = mController.GetGraphicsDevice().GetAllocator();

  vk::Pipeline vkPipeline;
  VkAssert(vkDevice.createGraphicsPipelines(VK_NULL_HANDLE,
                                            1,
                                            &gfxPipelineInfo,
                                            &allocator,
                                            &vkPipeline));

  // Store the pipeline and render pass in mVkPipelines for future reuse
  if(currentRenderPassImpl)
  {
    RenderPassPipelinePair pipelinePair;
    pipelinePair.renderPass = currentRenderPassImpl;
    pipelinePair.pipeline   = vkPipeline;
    mVkPipelines.emplace_back(pipelinePair);
  }

  // Push pipeline to the depth state cache for future reuse
  auto hash = HashDepthStencilState(dsState);

  DepthStatePipelineHashed item;
  item.hash     = hash;
  item.pipeline = vkPipeline;
  item.ds       = dsState;
  mPipelineForDepthStateCache.emplace_back(item);

  return vkPipeline;
}

RenderPassHandle PipelineImpl::GetCurrentRenderPassImpl()
{
  auto rtImpl      = static_cast<Vulkan::RenderTarget*>(mCreateInfo.renderTarget);
  auto framebuffer = rtImpl->GetFramebuffer();
  auto surface     = rtImpl->GetSurface();

  FramebufferImpl* fbImpl = nullptr;
  if(surface)
  {
    auto& gfxDevice = mController.GetGraphicsDevice();
    auto  surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto  swapchain = gfxDevice.GetSwapchainForSurfaceId(surfaceId);
    fbImpl          = swapchain->GetCurrentFramebuffer();
  }
  else if(framebuffer)
  {
    fbImpl = framebuffer->GetImpl();
  }

  if(fbImpl && fbImpl->GetRenderPassCount() > 0)
  {
    return fbImpl->GetRenderPass(0); // Return the Handle directly
  }

  return {}; // Return empty handle
}

vk::Pipeline PipelineImpl::FindExistingPipeline(vk::PipelineDepthStencilStateCreateInfo& dsState, RenderPassHandle currentRenderPassImpl)
{
  auto hash = HashDepthStencilState(dsState);

  // First, check the depth state cache for an exact match
  auto it = std::find_if(mPipelineForDepthStateCache.begin(), mPipelineForDepthStateCache.end(), [hash, &dsState](const auto& item)
                         { return item.hash == hash && item.ds == dsState; });

  if(it != mPipelineForDepthStateCache.end())
  {
    // Return early if found in depth state cache
    return it->pipeline;
  }

  // If not found in depth state cache, check if we can reuse from main pipeline cache
  // Look for pipelines with compatible render passes AND matching depth state that could be reused
  auto pipelineIt = std::find_if(mVkPipelines.begin(), mVkPipelines.end(), [&currentRenderPassImpl, &hash, this](const auto& pipelinePair)
                                 {
                                   if(!pipelinePair.pipeline || !currentRenderPassImpl || !pipelinePair.renderPass)
                                   {
                                     return false;
                                   }

                                   // Check if the existing pipeline's render pass is compatible with current render pass
                                   if(!pipelinePair.renderPass->IsCompatible(currentRenderPassImpl))
                                   {
                                     return false;
                                   }

                                   // Check if this pipeline was created with the same depth state
                                   auto depthIt = std::find_if(mPipelineForDepthStateCache.begin(), mPipelineForDepthStateCache.end(),
                                                         [&pipelinePair, &hash](const auto& depthEntry)
                                                         {
                                                           return depthEntry.pipeline == pipelinePair.pipeline && depthEntry.hash == hash;
                                                         });

                                   return depthIt != mPipelineForDepthStateCache.end(); });

  if(pipelineIt != mVkPipelines.end())
  {
    // Found a pipeline with compatible render pass AND matching depth state
    return pipelineIt->pipeline;
  }

  return nullptr; // No matching pipeline found
}

void PipelineImpl::ClearPipelineCaches()
{
  auto vkDevice = mController.GetGraphicsDevice().GetLogicalDevice();

  // Track all pipelines we've destroyed to avoid double destruction
  std::vector<vk::Pipeline> destroyedPipelines;

  // Clear depth state pipeline cache and destroy pipelines
  for(auto& entry : mPipelineForDepthStateCache)
  {
    if(entry.pipeline)
    {
      vkDevice.destroyPipeline(entry.pipeline);
      destroyedPipelines.push_back(entry.pipeline);
      entry.pipeline = nullptr;
    }
  }
  mPipelineForDepthStateCache.clear();

  // Clear main pipeline cache, skipping already destroyed pipelines
  for(auto& entry : mVkPipelines)
  {
    if(entry.pipeline)
    {
      // Only destroy if we haven't already destroyed this pipeline
      bool alreadyDestroyed = std::any_of(destroyedPipelines.begin(), destroyedPipelines.end(), [&entry](vk::Pipeline destroyedPipeline)
                                          { return entry.pipeline == destroyedPipeline; });

      if(!alreadyDestroyed)
      {
        vkDevice.destroyPipeline(entry.pipeline);
      }
      entry.pipeline = nullptr;
    }
  }
  mVkPipelines.clear();
}

void PipelineImpl::RemoveIncompatiblePipelines(RenderPassHandle currentRenderPass)
{
  auto vkDevice = mController.GetGraphicsDevice().GetLogicalDevice();

  // Track all pipelines we've destroyed to avoid double destruction
  std::vector<vk::Pipeline> destroyedPipelines;

  // First, identify and remove incompatible pipelines from main cache
  auto newEnd = std::remove_if(mVkPipelines.begin(), mVkPipelines.end(), [&currentRenderPass, &vkDevice, &destroyedPipelines](const auto& pipelinePair)
                               {
                                 if(pipelinePair.pipeline && pipelinePair.renderPass)
                                 {
                                   // Check if this pipeline's render pass is compatible with current render pass
                                   if(!pipelinePair.renderPass->IsCompatible(currentRenderPass))
                                   {
                                     // Incompatible - destroy the pipeline
                                     vkDevice.destroyPipeline(pipelinePair.pipeline);
                                     destroyedPipelines.push_back(pipelinePair.pipeline);
                                     return true; // Remove from vector
                                   }
                                 }
                                 return false; // Keep compatible pipelines
                               });

  mVkPipelines.erase(newEnd, mVkPipelines.end());

  // Now remove corresponding entries from depth state cache for destroyed pipelines
  auto depthNewEnd = std::remove_if(mPipelineForDepthStateCache.begin(), mPipelineForDepthStateCache.end(), [&destroyedPipelines](const auto& depthEntry)
                                    {
                                     // Check if this pipeline was destroyed
                                     return std::any_of(destroyedPipelines.begin(), destroyedPipelines.end(),
                                                      [&depthEntry](vk::Pipeline destroyedPipeline)
                                                      {
                                                        return depthEntry.pipeline == destroyedPipeline;
                                                      }); });

  mPipelineForDepthStateCache.erase(depthNewEnd, mPipelineForDepthStateCache.end());
}

void PipelineImpl::ValidateRenderPassCompatibility()
{
  // Check for render pass compatibility and force recreation if needed
  // This ensures we never use incompatible pipelines
  auto rtImpl      = static_cast<Vulkan::RenderTarget*>(mCreateInfo.renderTarget);
  auto framebuffer = rtImpl->GetFramebuffer();
  auto surface     = rtImpl->GetSurface();

  FramebufferImpl* fbImpl = nullptr;
  if(surface)
  {
    auto& gfxDevice = mController.GetGraphicsDevice();
    auto  surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto  swapchain = gfxDevice.GetSwapchainForSurfaceId(surfaceId);
    fbImpl          = swapchain->GetCurrentFramebuffer();
  }
  else if(framebuffer)
  {
    fbImpl = framebuffer->GetImpl();
  }

  // Check if current cached render pass is compatible with current framebuffer
  bool renderPassIncompatible = true;
  if(fbImpl && fbImpl->GetRenderPassCount() > 0)
  {
    RenderPassHandle currentRenderPass = fbImpl->GetRenderPass(0);

    // Check if any cached pipeline has an incompatible render pass
    auto it = std::find_if(mVkPipelines.begin(), mVkPipelines.end(), [&currentRenderPass](const auto& pipelinePair)
                           { return pipelinePair.pipeline && pipelinePair.renderPass &&
                                    !pipelinePair.renderPass->IsCompatible(currentRenderPass); });

    // If no incompatible pipeline found, then all are compatible
    renderPassIncompatible = (it != mVkPipelines.end());
  }

  if(renderPassIncompatible)
  {
    // Remove incompatible pipelines from the caches
    if(fbImpl && fbImpl->GetRenderPassCount() > 0)
    {
      RenderPassHandle currentRenderPass = fbImpl->GetRenderPass(0);
      RemoveIncompatiblePipelines(currentRenderPass);

      // Update cached render pass to current one
      mVkPipelineCreateInfo.renderPass = currentRenderPass->GetVkHandle();
      mVkPipelineCreateInfo.subpass    = 0;
    }
  }
}

const PipelineCreateInfo& PipelineImpl::GetCreateInfo() const
{
  return mCreateInfo;
}

VulkanGraphicsController& PipelineImpl::GetController() const
{
  return mController;
}

void PipelineImpl::Bind()
{
}

vk::Pipeline PipelineImpl::GetVkPipeline() const
{
  // this returns base VkPipeline only now but may return null
  if(mVkPipelines.empty())
  {
    return {};
  }
  return mVkPipelines[0].pipeline;
}

void PipelineImpl::Retain()
{
  //++mRefCount;
}

void PipelineImpl::Release()
{
  //--mRefCount;
}

uint32_t PipelineImpl::GetRefCount() const
{
  return 0; // mRefCount;
}

PipelineImpl::~PipelineImpl()
{
  // Clear all pipeline caches
  ClearPipelineCaches();
}

void PipelineImpl::InitializePipeline()
{
  auto  programImpl = static_cast<const Vulkan::Program*>(mCreateInfo.programState->program)->GetImplementation();
  auto& reflection  = programImpl->GetReflection();

  vk::GraphicsPipelineCreateInfo& gfxPipelineInfo = mVkPipelineCreateInfo;
  gfxPipelineInfo.setLayout(reflection.GetVkPipelineLayout());
  gfxPipelineInfo.setStageCount(programImpl->GetVkPipelineShaderStageCreateInfoList().size());
  gfxPipelineInfo.setStages(programImpl->GetVkPipelineShaderStageCreateInfoList());
  gfxPipelineInfo.setBasePipelineHandle(nullptr);
  gfxPipelineInfo.setBasePipelineIndex(0);

  // 1. PipelineVertexInputStateCreateInfo
  vk::PipelineVertexInputStateCreateInfo& visInfo = mVertexInputState;
  InitializeVertexInputState(visInfo);
  gfxPipelineInfo.setPVertexInputState(&visInfo);

  // 2. PipelineInputAssemblyStateCreateInfo
  vk::PipelineInputAssemblyStateCreateInfo& iasInfo = mInputAssemblyState;
  InitializeInputAssemblyState(iasInfo);
  gfxPipelineInfo.setPInputAssemblyState(&iasInfo);

  // 3. PipelineTessellationStateCreateInfo - We don't support this one
  gfxPipelineInfo.setPTessellationState(nullptr);

  // 4. PipelineViewportStateCreateInfo
  vk::PipelineViewportStateCreateInfo& viewInfo = mViewportState;
  InitializeViewportState(viewInfo);
  gfxPipelineInfo.setPViewportState(&viewInfo);

  // 5. PipelineRasterizationStateCreateInfo
  vk::PipelineRasterizationStateCreateInfo& rsInfo = mRasterizationState;
  InitializeRasterizationState(rsInfo);
  gfxPipelineInfo.setPRasterizationState(&rsInfo);

  // 6. PipelineMultisampleStateCreateInfo
  vk::PipelineMultisampleStateCreateInfo& msInfo = mMultisampleState;
  gfxPipelineInfo.setPMultisampleState(&msInfo);

  // 7. PipelineDepthStencilStateCreateInfo
  vk::PipelineDepthStencilStateCreateInfo& dsInfo = mDepthStencilState;
  gfxPipelineInfo.setPDepthStencilState(InitializeDepthStencilState(dsInfo) ? &dsInfo : nullptr);

  // 8. PipelineColorBlendStateCreateInfo
  vk::PipelineColorBlendStateCreateInfo& bsInfo = mColorBlendState;
  InitializeColorBlendState(bsInfo);
  gfxPipelineInfo.setPColorBlendState(&bsInfo);

  // 9. PipelineDynamicStateCreateInfo
  vk::PipelineDynamicStateCreateInfo& dynInfo = mDynamicState;
  dynInfo.setDynamicStates(mDynamicStates);
  gfxPipelineInfo.setPDynamicState(&dynInfo);

  auto rtImpl = static_cast<Vulkan::RenderTarget*>(mCreateInfo.renderTarget);

  auto framebuffer = rtImpl->GetFramebuffer();
  auto surface     = rtImpl->GetSurface();

  FramebufferImpl* fbImpl = nullptr;
  if(surface)
  {
    auto& gfxDevice = mController.GetGraphicsDevice();
    auto  surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto  swapchain = gfxDevice.GetSwapchainForSurfaceId(surfaceId);
    fbImpl          = swapchain->GetCurrentFramebuffer();
  }
  else if(framebuffer)
  {
    fbImpl = framebuffer->GetImpl();
  }
  else
  {
    // Can't really get here - render target has either framebuffer or surface.
    return;
  }
  auto renderPassCount = fbImpl->GetRenderPassCount();
  for(auto i = 0u; i < renderPassCount; ++i)
  {
    RenderPassHandle impl      = fbImpl->GetRenderPass(i);
    gfxPipelineInfo.renderPass = impl->GetVkHandle();
    gfxPipelineInfo.subpass    = 0;

    if(gfxPipelineInfo.pColorBlendState)
    {
      auto attachmentCount = impl->GetAttachmentCount();
      if(impl->HasDepthAttachment())
      {
        attachmentCount--;
      }
      if(attachmentCount != mBlendStateAttachments.size())
      {
        // Make sure array is 1
        mBlendStateAttachments.resize(1);

        // make it the right size
        mBlendStateAttachments.resize(attachmentCount);

        const_cast<vk::PipelineColorBlendStateCreateInfo*>(gfxPipelineInfo.pColorBlendState)->attachmentCount = attachmentCount;
        const_cast<vk::PipelineColorBlendStateCreateInfo*>(gfxPipelineInfo.pColorBlendState)->pAttachments    = mBlendStateAttachments.data();

        // Fill with defaults
        if(attachmentCount > 1u)
        {
          std::fill(mBlendStateAttachments.begin() + 1, mBlendStateAttachments.end(), mBlendStateAttachments[0]);
        }
      }
      // This code must not run if Vulkan API version is < 1.3
      // We don't pass dynamic states anymore but neither any depth/stencil
      // state (no default state).
      // TODO: fix it by implementing proper caching
      /*
      vk::Pipeline vkPipeline;
      VkAssert(vkDevice.createGraphicsPipelines(VK_NULL_HANDLE,
                                                1,
                                                &gfxPipelineInfo,
                                                &allocator,
                                                &vkPipeline));

      RenderPassPipelinePair item;
      item.renderPass = nullptr;
      item.pipeline   = vkPipeline;
      mVkPipelines.emplace_back(item);
      */
    }
  }

  // Update the cached pipeline create info with the current render pass
  // This ensures that when CloneInheritedVkPipeline uses mVkPipelineCreateInfo it has the correct render pass
  if(renderPassCount > 0)
  {
    RenderPassHandle currentRenderPass = fbImpl->GetRenderPass(0);
    mVkPipelineCreateInfo.renderPass   = currentRenderPass->GetVkHandle();
    mVkPipelineCreateInfo.subpass      = 0;
  }
}

const Vulkan::Program* PipelineImpl::GetProgram() const
{
  return static_cast<const Vulkan::Program*>(mCreateInfo.programState->program);
}

bool PipelineImpl::ComparePipelineDepthStencilState(vk::PipelineDepthStencilStateCreateInfo& state)
{
  auto hashToCompare = HashDepthStencilState(state);
  auto currentHash   = HashDepthStencilState(mDepthStencilState);
  if(hashToCompare == currentHash)
  {
    return mDepthStencilState == state;
  }
  return false;
}

void PipelineImpl::InitializeVertexInputState(vk::PipelineVertexInputStateCreateInfo& out)
{
  std::vector<vk::VertexInputBindingDescription> bindings;
  std::transform(mCreateInfo.vertexInputState->bufferBindings.begin(),
                 mCreateInfo.vertexInputState->bufferBindings.end(),
                 std::back_inserter(bindings),
                 [](const VertexInputState::Binding& in) -> vk::VertexInputBindingDescription
  {
    vk::VertexInputBindingDescription out;
    out.setInputRate((in.inputRate == VertexInputRate::PER_VERTEX ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance));
    out.setBinding(0u); // To be filled later using indices
    out.setStride(in.stride);
    return out;
  });

  // Assign bindings
  for(auto i = 0u; i < bindings.size(); ++i)
  {
    bindings[i].binding = i;
  }

  std::vector<vk::VertexInputAttributeDescription> attrs;
  std::transform(mCreateInfo.vertexInputState->attributes.begin(),
                 mCreateInfo.vertexInputState->attributes.end(),
                 std::back_inserter(attrs),
                 [](const VertexInputState::Attribute& in) -> vk::VertexInputAttributeDescription
  {
    vk::VertexInputAttributeDescription out;
    out.setBinding(in.binding);
    out.setLocation(in.location);
    out.setOffset(in.offset);
    VertexInputFormat format = in.format;
    switch(format)
    {
      case VertexInputFormat::FVECTOR2:
      {
        out.setFormat(vk::Format::eR32G32Sfloat);
        break;
      }
      case VertexInputFormat::FVECTOR3:
      {
        out.setFormat(vk::Format::eR32G32B32Sfloat);
        break;
      }
      case VertexInputFormat::FVECTOR4:
      {
        out.setFormat(vk::Format::eR32G32B32A32Sfloat);
        break;
      }
      case VertexInputFormat::IVECTOR2:
      {
        out.setFormat(vk::Format::eR32G32Sint);
        break;
      }
      case VertexInputFormat::IVECTOR3:
      {
        out.setFormat(vk::Format::eR32G32B32Sint);
        break;
      }
      case VertexInputFormat::IVECTOR4:
      {
        out.setFormat(vk::Format::eR32G32B32A32Sint);
        break;
      }
      case VertexInputFormat::FLOAT:
      {
        out.setFormat(vk::Format::eR32Sfloat);
        break;
      }
      case VertexInputFormat::INTEGER:
      {
        out.setFormat(vk::Format::eR32Sint);
        break;
      }
      case VertexInputFormat::UNDEFINED:
      default:
      {
        out.setFormat(vk::Format::eUndefined);
        DALI_LOG_ERROR("Vulkan vertex format undefined!\n");
      }
    };
    return out;
  });

  mVertexInputAttributeDescriptionList = attrs;
  mVertexInputBindingDescriptionList   = bindings;
  out.setVertexAttributeDescriptions(mVertexInputAttributeDescriptionList);
  out.setVertexBindingDescriptions(mVertexInputBindingDescriptionList);
}

void PipelineImpl::InitializeInputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo& out) const
{
  auto gfxInputAssembly = mCreateInfo.inputAssemblyState;
  switch(gfxInputAssembly->topology)
  {
    case PrimitiveTopology::POINT_LIST:
    {
      out.setTopology(vk::PrimitiveTopology::ePointList);
      break;
    }
    case PrimitiveTopology::LINE_LIST:
    {
      out.setTopology(vk::PrimitiveTopology::eLineList);
      break;
    }
    case PrimitiveTopology::LINE_LOOP:
    {
      out.setTopology({});
      DALI_LOG_ERROR("LINE_LOOP topology isn't supported by Vulkan!\n");
      break;
    }
    case PrimitiveTopology::LINE_STRIP:
    {
      out.setTopology(vk::PrimitiveTopology::eLineStrip);
      break;
    }
    case PrimitiveTopology::TRIANGLE_LIST:
    {
      out.setTopology(vk::PrimitiveTopology::eTriangleList);
      break;
    }
    case PrimitiveTopology::TRIANGLE_STRIP:
    {
      out.setTopology(vk::PrimitiveTopology::eTriangleStrip);
      break;
    }
    case PrimitiveTopology::TRIANGLE_FAN:
    {
      out.setTopology(vk::PrimitiveTopology::eTriangleFan);
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Unknown topology!\n");
    }
  }
  out.setPrimitiveRestartEnable(gfxInputAssembly->primitiveRestartEnable);
}

void PipelineImpl::InitializeViewportState(vk::PipelineViewportStateCreateInfo& out)
{
  auto gfxViewportState = mCreateInfo.viewportState;

  // if there is no gfx viewport state provided then we assume
  // it's going to be a dynamic state
  if(gfxViewportState)
  {
    // build viewport
    mViewport.x        = gfxViewportState->viewport.x;
    mViewport.y        = gfxViewportState->viewport.y;
    mViewport.width    = gfxViewportState->viewport.width;
    mViewport.height   = gfxViewportState->viewport.height;
    mViewport.minDepth = gfxViewportState->viewport.minDepth;
    mViewport.maxDepth = gfxViewportState->viewport.maxDepth;

    mScissor.offset = vk::Offset2D{gfxViewportState->scissor.x, gfxViewportState->scissor.y};
    mScissor.extent = vk::Extent2D{gfxViewportState->scissor.width, gfxViewportState->scissor.height};

    out.setViewportCount(1);
    out.setPViewports(&mViewport);
    out.setScissorCount(1);
    out.setPScissors(&mScissor);
  }
  else
  {
    out.setViewportCount(1);
    out.setScissorCount(1);
    // enable dynamic state, otherwise it's an error
    mDynamicStates.emplace_back(vk::DynamicState::eViewport);
    mDynamicStates.emplace_back(vk::DynamicState::eScissor);
  }
}

void PipelineImpl::InitializeMultisampleState(vk::PipelineMultisampleStateCreateInfo& out)
{
  out = vk::PipelineMultisampleStateCreateInfo{}; // TODO: decide what to set when we start rendering something
}

void PipelineImpl::InitializeRasterizationState(vk::PipelineRasterizationStateCreateInfo& out) const
{
  auto gfxRastState = mCreateInfo.rasterizationState;

  out.setFrontFace([gfxRastState]()
  { return gfxRastState->frontFace == FrontFace::CLOCKWISE ? vk::FrontFace::eClockwise : vk::FrontFace::eCounterClockwise; }());

  out.setPolygonMode([polygonMode = gfxRastState->polygonMode]()
  {
    switch(polygonMode)
    {
      case PolygonMode::FILL:
      {
        return vk::PolygonMode::eFill;
      }
      case PolygonMode::LINE:
      {
        return vk::PolygonMode::eLine;
      }
      case PolygonMode::POINT:
      {
        return vk::PolygonMode::ePoint;
      }
    }
    return vk::PolygonMode{}; }());

  out.setCullMode([cullMode = gfxRastState->cullMode]() -> vk::CullModeFlagBits
  {
    switch(cullMode)
    {
      case CullMode::NONE:
      {
        return vk::CullModeFlagBits::eNone;
      }
      case CullMode::BACK:
      {
        return vk::CullModeFlagBits::eBack;
      }
      case CullMode::FRONT:
      {
        return vk::CullModeFlagBits::eFront;
      }
      case CullMode::FRONT_AND_BACK:
      {
        return vk::CullModeFlagBits::eFrontAndBack;
      }
    }
    return {}; }());

  out.setLineWidth(1.0f);         // Line with hardcoded to 1.0f
  out.setDepthClampEnable(false); // no depth clamp
}

bool PipelineImpl::InitializeDepthStencilState(vk::PipelineDepthStencilStateCreateInfo& out)
{
  auto& in = mCreateInfo.depthStencilState;

  if(in)
  {
    out.setBack(ConvStencilOpState(in->back));
    out.setFront(ConvStencilOpState(in->front));
    out.setDepthTestEnable(in->depthTestEnable);
    out.setDepthWriteEnable(in->depthWriteEnable);
    out.setDepthBoundsTestEnable(false);
    out.setMinDepthBounds({});
    out.setMaxDepthBounds({});
    out.setStencilTestEnable(in->stencilTestEnable);
    out.setDepthCompareOp(ConvCompareOp(in->depthCompareOp));
    return true;
  }
  else
  {
    // If we're not setting the following through the createInfo struct, they must instead
    // come from command buffer commands.
    mDynamicStates.emplace_back(vk::DynamicState::eDepthTestEnable);
    mDynamicStates.emplace_back(vk::DynamicState::eDepthWriteEnable);
    mDynamicStates.emplace_back(vk::DynamicState::eDepthCompareOp);
    mDynamicStates.emplace_back(vk::DynamicState::eDepthBounds);
    mDynamicStates.emplace_back(vk::DynamicState::eDepthBoundsTestEnable);
    mDynamicStates.emplace_back(vk::DynamicState::eStencilTestEnable);
    mDynamicStates.emplace_back(vk::DynamicState::eStencilOp);
    mDynamicStates.emplace_back(vk::DynamicState::eStencilCompareMask);
    mDynamicStates.emplace_back(vk::DynamicState::eStencilWriteMask);
    mDynamicStates.emplace_back(vk::DynamicState::eStencilReference);
  }
  return false;
}

void PipelineImpl::InitializeColorBlendState(vk::PipelineColorBlendStateCreateInfo& out)
{
  auto in = mCreateInfo.colorBlendState;

  auto ConvLogicOp = [](LogicOp in)
  {
    switch(in)
    {
      case LogicOp::CLEAR:
      {
        return vk::LogicOp::eClear;
      }
      case LogicOp::AND:
      {
        return vk::LogicOp::eAnd;
      }
      case LogicOp::AND_REVERSE:
      {
        return vk::LogicOp::eAndReverse;
      }
      case LogicOp::COPY:
      {
        return vk::LogicOp::eCopy;
      }
      case LogicOp::AND_INVERTED:
      {
        return vk::LogicOp::eAndInverted;
      }
      case LogicOp::NO_OP:
      {
        return vk::LogicOp::eNoOp;
      }
      case LogicOp::XOR:
      {
        return vk::LogicOp::eXor;
      }
      case LogicOp::OR:
      {
        return vk::LogicOp::eOr;
      }
      case LogicOp::NOR:
      {
        return vk::LogicOp::eNor;
      }
      case LogicOp::EQUIVALENT:
      {
        return vk::LogicOp::eEquivalent;
      }
      case LogicOp::INVERT:
      {
        return vk::LogicOp::eInvert;
      }
      case LogicOp::OR_REVERSE:
      {
        return vk::LogicOp::eOrReverse;
      }
      case LogicOp::COPY_INVERTED:
      {
        return vk::LogicOp::eCopyInverted;
      }
      case LogicOp::OR_INVERTED:
      {
        return vk::LogicOp::eOrInverted;
      }
      case LogicOp::NAND:
      {
        return vk::LogicOp::eNand;
      }
      case LogicOp::SET:
      {
        return vk::LogicOp::eSet;
      }
    }
    return vk::LogicOp{};
  };

  auto ConvBlendOp = [](BlendOp in)
  {
    switch(in)
    {
      case BlendOp::ADD:
      {
        return vk::BlendOp::eAdd;
      }
      case BlendOp::SUBTRACT:
      {
        return vk::BlendOp::eSubtract;
      }
      case BlendOp::REVERSE_SUBTRACT:
      {
        return vk::BlendOp::eReverseSubtract;
      }
      case BlendOp::MIN:
      {
        return vk::BlendOp::eMin;
      }
      case BlendOp::MAX:
      {
        return vk::BlendOp::eMax;
      }
      case BlendOp::MULTIPLY:
      {
        return vk::BlendOp::eMultiplyEXT;
      }
      case BlendOp::SCREEN:
      {
        return vk::BlendOp::eScreenEXT;
      }
      case BlendOp::OVERLAY:
      {
        return vk::BlendOp::eOverlayEXT;
      }
      case BlendOp::DARKEN:
      {
        return vk::BlendOp::eDarkenEXT;
      }
      case BlendOp::LIGHTEN:
      {
        return vk::BlendOp::eLightenEXT;
      }
      case BlendOp::COLOR_DODGE:
      {
        return vk::BlendOp::eColordodgeEXT;
      }
      case BlendOp::COLOR_BURN:
      {
        return vk::BlendOp::eColorburnEXT;
      }
      case BlendOp::HARD_LIGHT:
      {
        return vk::BlendOp::eHardlightEXT;
      }
      case BlendOp::SOFT_LIGHT:
      {
        return vk::BlendOp::eSoftlightEXT;
      }
      case BlendOp::DIFFERENCE:
      {
        return vk::BlendOp::eDifferenceEXT;
      }
      case BlendOp::EXCLUSION:
      {
        return vk::BlendOp::eExclusionEXT;
      }
      case BlendOp::HUE:
      {
        return vk::BlendOp::eHslHueEXT;
      }
      case BlendOp::SATURATION:
      {
        return vk::BlendOp::eHslSaturationEXT;
      }
      case BlendOp::COLOR:
      {
        return vk::BlendOp::eHslColorEXT;
      }
      case BlendOp::LUMINOSITY:
      {
        return vk::BlendOp::eHslLuminosityEXT;
      }
    }
    return vk::BlendOp{};
  };

  auto ConvBlendFactor = [](BlendFactor in)
  {
    switch(in)
    {
      case BlendFactor::ZERO:
      {
        return vk::BlendFactor::eZero;
      }
      case BlendFactor::ONE:
      {
        return vk::BlendFactor::eOne;
      }
      case BlendFactor::SRC_COLOR:
      {
        return vk::BlendFactor::eSrcColor;
      }
      case BlendFactor::ONE_MINUS_SRC_COLOR:
      {
        return vk::BlendFactor::eOneMinusSrcColor;
      }
      case BlendFactor::DST_COLOR:
      {
        return vk::BlendFactor::eDstColor;
      }
      case BlendFactor::ONE_MINUS_DST_COLOR:
      {
        return vk::BlendFactor::eOneMinusDstColor;
      }
      case BlendFactor::SRC_ALPHA:
      {
        return vk::BlendFactor::eSrcAlpha;
      }
      case BlendFactor::ONE_MINUS_SRC_ALPHA:
      {
        return vk::BlendFactor::eOneMinusSrcAlpha;
      }
      case BlendFactor::DST_ALPHA:
      {
        return vk::BlendFactor::eDstAlpha;
      }
      case BlendFactor::ONE_MINUS_DST_ALPHA:
      {
        return vk::BlendFactor::eOneMinusDstAlpha;
      }
      case BlendFactor::CONSTANT_COLOR:
      {
        return vk::BlendFactor::eConstantColor;
      }
      case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
      {
        return vk::BlendFactor::eOneMinusConstantColor;
      }
      case BlendFactor::CONSTANT_ALPHA:
      {
        return vk::BlendFactor::eConstantAlpha;
      }
      case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
      {
        return vk::BlendFactor::eOneMinusConstantAlpha;
      }
      case BlendFactor::SRC_ALPHA_SATURATE:
      {
        return vk::BlendFactor::eSrcAlphaSaturate;
      }
      case BlendFactor::SRC1_COLOR:
      {
        return vk::BlendFactor::eSrc1Color;
      }
      case BlendFactor::ONE_MINUS_SRC1_COLOR:
      {
        return vk::BlendFactor::eOneMinusSrc1Color;
      }
      case BlendFactor::SRC1_ALPHA:
      {
        return vk::BlendFactor::eSrc1Alpha;
      }
      case BlendFactor::ONE_MINUS_SRC1_ALPHA:
      {
        return vk::BlendFactor::eOneMinusSrc1Alpha;
      }
    }
    return vk::BlendFactor{};
  };

  out.setLogicOpEnable(in->logicOpEnable);
  out.setLogicOp(ConvLogicOp(in->logicOp));

  // We don't know how many attachments we will blend but gfx api assumes single attachment
  mBlendStateAttachments.clear();
  mBlendStateAttachments.emplace_back();
  auto& att = mBlendStateAttachments.back();

  att.setAlphaBlendOp(ConvBlendOp(in->alphaBlendOp));
  att.setBlendEnable(in->blendEnable);
  // att.setColorWriteMask()
  att.setColorBlendOp(ConvBlendOp(in->colorBlendOp));
  att.setColorWriteMask(vk::ColorComponentFlags(in->colorComponentWriteBits));
  att.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  att.setDstAlphaBlendFactor(ConvBlendFactor(in->dstAlphaBlendFactor));
  att.setDstColorBlendFactor(ConvBlendFactor(in->dstColorBlendFactor));
  att.setSrcAlphaBlendFactor(ConvBlendFactor(in->srcAlphaBlendFactor));
  att.setSrcColorBlendFactor(ConvBlendFactor(in->srcColorBlendFactor));
  out.setAttachments(mBlendStateAttachments);

  std::copy(in->blendConstants, in->blendConstants + 4, out.blendConstants.begin());
}

} // namespace Dali::Graphics::Vulkan
