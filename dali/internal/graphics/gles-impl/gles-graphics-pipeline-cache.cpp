/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include "gles-graphics-pipeline-cache.h"
#include <algorithm>
#include "egl-graphics-controller.h"
#include "gles-graphics-pipeline.h"
#include "gles-graphics-program.h"

namespace Dali::Graphics::GLES
{
/**
 * @brief custom delete function for cached object
 */
template<class T>
struct CachedObjectDeleter
{
  CachedObjectDeleter() = default;

  void operator()(T* object)
  {
    // Discard resource (add it to discard queue)
    object->DiscardResource();
  }
};

template<>
struct CachedObjectDeleter<GLES::Program>
{
  CachedObjectDeleter() = default;

  void operator()(GLES::Program* object)
  {
    // Program deleter should skip discard queue if controller shutting down
    if(!object->GetController().IsShuttingDown())
    {
      object->DiscardResource();
    }
    else
    {
      // delete object otherwise
      delete object;
    }
  }
};

/**
 * @brief The order of states being stored in the cache and mask
 */
enum class StateLookupIndex : uint32_t
{
  COLOR_BLEND_STATE_BIT    = 0,
  VIEWPORT_STATE_BIT       = 1,
  FRAMEBUFFER_STATE_BIT    = 2,
  BASE_PIPELINE_STATE_BIT  = 3,
  DEPTH_STENCIL_STATE_BIT  = 4,
  RASTERIZATION_STATE_BIT  = 5,
  VERTEX_INPUT_STATE_BIT   = 6,
  INPUT_ASSEMBLY_STATE_BIT = 7,
  MAX_STATE                = 8
};

/**
 * Helper float compare function
 */
static bool cmpf(float A, float B, float epsilon = 0.005f)
{
  return (fabs(A - B) < epsilon);
}

/**
 * Helper operators
 */
static bool operator==(const Graphics::Viewport& lhs, const Graphics::Viewport& rhs)
{
  return cmpf(lhs.x, rhs.x) &&
         cmpf(lhs.y, rhs.y) &&
         cmpf(lhs.width, rhs.width) &&
         cmpf(lhs.height, rhs.height) &&
         cmpf(lhs.minDepth, rhs.minDepth) &&
         cmpf(lhs.maxDepth, rhs.maxDepth);
}

static bool operator==(const Graphics::Rect2D& lhs, const Graphics::Rect2D& rhs)
{
  return cmpf(lhs.x, rhs.x) &&
         cmpf(lhs.y, rhs.y) &&
         cmpf(lhs.width, rhs.width) &&
         cmpf(lhs.height, rhs.height);
}

static bool operator==(const Graphics::StencilOpState& lhs, const Graphics::StencilOpState& rhs)
{
  return lhs.failOp == rhs.failOp &&
         lhs.passOp == rhs.passOp &&
         lhs.depthFailOp == rhs.depthFailOp &&
         lhs.compareOp == rhs.compareOp &&
         lhs.compareMask == rhs.compareMask &&
         lhs.writeMask == rhs.writeMask &&
         lhs.reference == rhs.reference;
}

static bool
operator==(const Dali::Graphics::VertexInputState::Attribute& lhs,
           const Dali::Graphics::VertexInputState::Attribute& rhs)
{
  return lhs.location == rhs.location &&
         lhs.binding == rhs.binding &&
         lhs.offset == rhs.offset &&
         lhs.format == rhs.format;
}

static bool
operator==(const Dali::Graphics::VertexInputState::Binding& lhs, const Dali::Graphics::VertexInputState::Binding& rhs)
{
  return lhs.stride == rhs.stride &&
         lhs.inputRate == rhs.inputRate;
}

using PipelineStateCompateFunctionType = bool(const Graphics::PipelineCreateInfo*,
                                              const Graphics::PipelineCreateInfo*);

static std::vector<PipelineStateCompateFunctionType*> STATE_COMPARE_FUNC_TABLE{};

/**
 * @brief Initialises compare function lookup table
 */
void InitialiseStateCompareLookupTable()
{
  STATE_COMPARE_FUNC_TABLE = {
    [](const auto* lhs, const auto* rhs) -> bool // colorBlendState
    {
      const auto& lcb = *lhs->colorBlendState;
      const auto& rcb = *rhs->colorBlendState;
      return lcb.logicOpEnable == rcb.logicOpEnable &&
             lcb.logicOp == rcb.logicOp &&
             cmpf(lcb.blendConstants[0], rcb.blendConstants[0]) &&
             cmpf(lcb.blendConstants[1], rcb.blendConstants[1]) &&
             cmpf(lcb.blendConstants[2], rcb.blendConstants[2]) &&
             cmpf(lcb.blendConstants[3], rcb.blendConstants[3]) &&
             lcb.blendEnable == rcb.blendEnable &&
             lcb.srcColorBlendFactor == rcb.srcColorBlendFactor &&
             lcb.dstColorBlendFactor == rcb.dstColorBlendFactor &&
             lcb.colorBlendOp == rcb.colorBlendOp &&
             lcb.srcAlphaBlendFactor == rcb.srcAlphaBlendFactor &&
             lcb.dstAlphaBlendFactor == rcb.dstAlphaBlendFactor &&
             lcb.alphaBlendOp == rcb.alphaBlendOp &&
             lcb.colorComponentWriteBits == rcb.colorComponentWriteBits;
    },
    [](const auto* lhs, const auto* rhs) -> bool // viewport state
    {
      const auto& lvp = *lhs->viewportState;
      const auto& rvp = *rhs->viewportState;
      return lvp.viewport == rvp.viewport &&
             lvp.scissor == rvp.scissor &&
             lvp.scissorTestEnable == rvp.scissorTestEnable;
    },
    [](const auto* lhs, const auto* rhs) -> bool // framebufferState
    {
      const auto& lfb = *lhs->framebufferState;
      const auto& rfb = *rhs->framebufferState;
      return lfb.framebuffer == rfb.framebuffer;
    },
    [](const auto* lhs, const auto* rhs) -> bool // basePipeline
    {
      return lhs->basePipeline == rhs->basePipeline;
    },
    [](const auto* lhs, const auto* rhs) -> bool // depthStencilState
    {
      const auto& lds = *lhs->depthStencilState;
      const auto& rds = *rhs->depthStencilState;
      return lds.depthTestEnable == rds.depthTestEnable &&
             lds.depthWriteEnable == rds.depthWriteEnable &&
             lds.depthCompareOp == rds.depthCompareOp &&
             lds.stencilTestEnable == rds.stencilTestEnable &&
             lds.front == rds.front &&
             lds.back == rds.back;
    },
    [](const auto* lhs, const auto* rhs) -> bool // rasterizationState
    {
      const auto& lrs = *lhs->rasterizationState;
      const auto& rrs = *rhs->rasterizationState;
      return lrs.cullMode == rrs.cullMode &&
             lrs.polygonMode == rrs.polygonMode &&
             lrs.frontFace == rrs.frontFace;
    },
    [](const auto* lhs, const auto* rhs) -> bool // vertexInputState
    {
      const auto& lvi = *lhs->vertexInputState;
      const auto& rvi = *rhs->vertexInputState;
      return lvi.bufferBindings.size() == rvi.bufferBindings.size() &&
             lvi.attributes.size() == rvi.attributes.size() &&
             std::equal(lvi.bufferBindings.begin(), lvi.bufferBindings.end(), rvi.bufferBindings.begin(), [](const auto& lhs, const auto& rhs) {
               return operator==(lhs, rhs);
             }) &&
             std::equal(lvi.attributes.begin(), lvi.attributes.end(), rvi.attributes.begin(), [](const auto& lhs, const auto& rhs) {
               return operator==(lhs, rhs);
             });
    },
    [](const auto* lhs, const auto* rhs) -> bool // inputAssemblyState
    {
      const auto& lia = *lhs->inputAssemblyState;
      const auto& ria = *rhs->inputAssemblyState;
      return lia.topology == ria.topology &&
             lia.primitiveRestartEnable == ria.primitiveRestartEnable;
    },
  };
}

/**
 * @brief Helper function calculating the bitmask of set states
 *
 * @param[in] info Valid PipelineCreateInfo structure
 * @return bitmask of set states
 */
inline uint32_t GetStateBitmask(const PipelineCreateInfo& info)
{
  uint32_t mask{0u};
  mask |= bool(info.colorBlendState) << int(StateLookupIndex::COLOR_BLEND_STATE_BIT);
  mask |= bool(info.viewportState) << int(StateLookupIndex::VIEWPORT_STATE_BIT);
  mask |= bool(info.framebufferState) << int(StateLookupIndex::FRAMEBUFFER_STATE_BIT);
  mask |= bool(info.basePipeline) << int(StateLookupIndex::BASE_PIPELINE_STATE_BIT);
  mask |= bool(info.depthStencilState) << int(StateLookupIndex::DEPTH_STENCIL_STATE_BIT);
  mask |= bool(info.rasterizationState) << int(StateLookupIndex::RASTERIZATION_STATE_BIT);
  mask |= bool(info.vertexInputState) << int(StateLookupIndex::VERTEX_INPUT_STATE_BIT);
  mask |= bool(info.inputAssemblyState) << int(StateLookupIndex::INPUT_ASSEMBLY_STATE_BIT);
  return mask;
}

/**
 * @brief Implementation of cache
 */
struct PipelineCache::Impl
{
  /**
   * @brief Constructor
   */
  explicit Impl(EglGraphicsController& _controller)
  : controller(_controller)
  {
    // Initialise lookup table
    InitialiseStateCompareLookupTable();
  }

  /**
   * @brief destructor
   */
  ~Impl()
  {
    // First destroy pipelines
    entries.clear();

    // Now programs
    programEntries.clear();
  }

  /**
   * @brief Structure describes a single cache entry
   */
  struct CacheEntry
  {
    CacheEntry() = default;

    CacheEntry(UniquePtr<PipelineImpl>&& _pipeline, uint32_t _bitmask)
    : pipeline(std::move(_pipeline)),
      stateBitmask(_bitmask)
    {
    }

    ~CacheEntry() = default;

    CacheEntry(CacheEntry&&) = default;
    CacheEntry& operator=(CacheEntry&&) = default;

    UniquePtr<PipelineImpl> pipeline{nullptr};
    uint32_t                stateBitmask{0u};
  };

  EglGraphicsController&  controller;
  std::vector<CacheEntry> entries;

  /**
   * @brief Sorted array of shaders used to create program
   */
  struct ProgramCacheEntry
  {
    // sorted array of shaders
    std::vector<const Graphics::Shader*> shaders;
    UniquePtr<ProgramImpl>               program{nullptr};
  };

  std::vector<ProgramCacheEntry> programEntries;
};

PipelineCache::PipelineCache(EglGraphicsController& controller)
{
  mImpl = std::make_unique<Impl>(controller);
}

PipelineCache::~PipelineCache() = default;

PipelineImpl* PipelineCache::FindPipelineImpl(const PipelineCreateInfo& info)
{
  auto bitmask = GetStateBitmask(info);

  for(auto& entry : mImpl->entries)
  {
    auto& pipeline  = entry.pipeline;
    auto& cacheInfo = pipeline->GetCreateInfo();
    if(!info.programState)
    {
      continue;
    }

    // Check whether the program is the same
    if(info.programState->program)
    {
      const auto& lhsProgram = *static_cast<const GLES::Program*>(info.programState->program);
      const auto& rhsProgram = *static_cast<const GLES::Program*>(cacheInfo.programState->program);
      if(lhsProgram != rhsProgram)
      {
        continue;
      }

      // Test whether set states bitmask matches
      if(entry.stateBitmask != bitmask)
      {
        continue;
      }

      // Now test only for states that are set
      auto i = 0;
      for(i = 0; i < int(StateLookupIndex::MAX_STATE); ++i)
      {
        // Test only set states
        if((entry.stateBitmask & (1 << i)))
        {
          if(!STATE_COMPARE_FUNC_TABLE[i](&info, &cacheInfo))
          {
            break;
          }
        }
      }

      // TODO: For now ignoring dynamic state mask and allocator
      // Getting as far as here, we have found our pipeline impl
      if(i == int(StateLookupIndex::MAX_STATE))
      {
        return pipeline.get();
      }
    }
  }
  return nullptr;
}

ProgramImpl* PipelineCache::FindProgramImpl(const ProgramCreateInfo& info)
{
  if(mImpl->programEntries.empty())
  {
    return nullptr;
  }

  // assert if no shaders given
  std::vector<const Graphics::Shader*> shaders;
  shaders.reserve(info.shaderState->size());

  for(auto& state : *info.shaderState)
  {
    shaders.push_back(state.shader);
  }

  // sort
  std::sort(shaders.begin(), shaders.end());

  for(auto& item : mImpl->programEntries)
  {
    if(item.shaders.size() != shaders.size())
    {
      continue;
    }

    int k = shaders.size();
    while(--k >= 0 && item.shaders[k] == shaders[k])
      ;

    if(k < 0)
    {
      return item.program.get();
    }
  }
  return nullptr;
}

Graphics::UniquePtr<Graphics::Pipeline> PipelineCache::GetPipeline(const PipelineCreateInfo&                 pipelineCreateInfo,
                                                                   Graphics::UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  auto cachedPipeline = FindPipelineImpl(pipelineCreateInfo);

  // Return same pointer if nothing changed
  if(oldPipeline && *static_cast<GLES::Pipeline*>(oldPipeline.get()) == cachedPipeline)
  {
    return std::move(oldPipeline);
  }

  if(!cachedPipeline)
  {
    // create new pipeline
    auto pipeline = MakeUnique<GLES::PipelineImpl>(pipelineCreateInfo, mImpl->controller, *this);

    cachedPipeline = pipeline.get();

    // add it to cache
    mImpl->entries.emplace_back(std::move(pipeline), GetStateBitmask(pipelineCreateInfo));
  }

  auto wrapper = MakeUnique<GLES::Pipeline, CachedObjectDeleter<GLES::Pipeline>>(*cachedPipeline);
  return std::move(wrapper);
}

Graphics::UniquePtr<Graphics::Program> PipelineCache::GetProgram(const ProgramCreateInfo&                 programCreateInfo,
                                                                 Graphics::UniquePtr<Graphics::Program>&& oldProgram)
{
  ProgramImpl* cachedProgram = FindProgramImpl(programCreateInfo);

  // Return same pointer if nothing changed
  if(oldProgram && *static_cast<GLES::Program*>(oldProgram.get()) == cachedProgram)
  {
    return std::move(oldProgram);
  }

  if(!cachedProgram)
  {
    // create new pipeline
    auto program = MakeUnique<GLES::ProgramImpl>(programCreateInfo, mImpl->controller);

    program->Create();

    cachedProgram = program.get();

    // add it to cache
    mImpl->programEntries.emplace_back();
    auto& item   = mImpl->programEntries.back();
    item.program = std::move(program);
    for(auto& state : *programCreateInfo.shaderState)
    {
      item.shaders.push_back(state.shader);
    }

    std::sort(item.shaders.begin(), item.shaders.end());
  }

  auto wrapper = MakeUnique<GLES::Program, CachedObjectDeleter<GLES::Program>>(cachedProgram);
  return std::move(wrapper);
}

void PipelineCache::FlushCache()
{
  decltype(mImpl->entries) newEntries;
  newEntries.reserve(mImpl->entries.size());

  for(auto& entry : mImpl->entries)
  {
    // Move items which are still in use into the new array
    if(entry.pipeline->GetRefCount() != 0)
    {
      newEntries.emplace_back(std::move(entry));
    }
  }

  // Move temporary array in place of stored cache
  // Unused pipelines will be deleted automatically
  mImpl->entries = std::move(newEntries);

  // TODO: program cache may require similar action. However,
  //       since there is always one wrapper for Program object
  //       kept in the pipeline, then death of pipeline will result
  //       killing the program (if program isn't in use anymore)
}

} // namespace Dali::Graphics::GLES