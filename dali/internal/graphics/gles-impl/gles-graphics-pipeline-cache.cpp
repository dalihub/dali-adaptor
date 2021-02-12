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

// CLASS HEADER
#include "gles-graphics-pipeline-cache.h"

// INTERNAL INCLUDES
#include "gles-graphics-pipeline.h"

namespace Dali::Graphics::GLES
{
/**
 * @brief Implementation of pipeline cache
 */
struct PipelineCache::Impl
{
  explicit Impl(EglGraphicsController& _controller)
  : controller(_controller)
  {
  }

  ~Impl() = default;

  EglGraphicsController& controller;

  std::vector<UniquePtr<PipelineImpl>> mPipelines;
};

PipelineCache::PipelineCache(EglGraphicsController& controller)
{
  mImpl = std::make_unique<Impl>(controller);
}

PipelineCache::~PipelineCache() = default;

PipelineImpl* PipelineCache::FindPipelineImpl(const PipelineCreateInfo& info)
{
  // hashing by shaders and then states, shaders should use the same pointers
  for(auto& pipeline : mImpl->mPipelines)
  {
    auto& cacheInfo = pipeline->GetCreateInfo();

    if(!info.shaderState)
    {
      continue;
    }

    // TODO: Hash the structures. In GLES the order should not matter
    //       (it matters now to keep it simple)
    if(info.shaderState->size() == cacheInfo.shaderState->size())
    {
      if(memcmp(info.shaderState->data(), cacheInfo.shaderState->data(), sizeof(info.shaderState->size() * sizeof(ShaderState))) != 0)
      {
        continue; // early exit
      }

      // test null states
      // TODO: create and store such bitmask when pipeline
      //       is being constructed!
      uint32_t infoBits =
        (info.inputAssemblyState ? 1 << 0 : 0) |
        (info.vertexInputState ? 1 << 1 : 0) |
        (info.viewportState ? 1 << 2 : 0) |
        (info.depthStencilState ? 1 << 3 : 0) |
        (info.colorBlendState ? 1 << 4 : 0) |
        (info.framebufferState ? 1 << 5 : 0) |
        (info.rasterizationState ? 1 << 6 : 0) |
        (info.basePipeline ? 1 << 7 : 0);

      uint32_t cacheBits =
        (cacheInfo.inputAssemblyState ? 1 << 0 : 0) |
        (cacheInfo.vertexInputState ? 1 << 1 : 0) |
        (cacheInfo.viewportState ? 1 << 2 : 0) |
        (cacheInfo.depthStencilState ? 1 << 3 : 0) |
        (cacheInfo.colorBlendState ? 1 << 4 : 0) |
        (cacheInfo.framebufferState ? 1 << 5 : 0) |
        (cacheInfo.rasterizationState ? 1 << 6 : 0) |
        (cacheInfo.basePipeline ? 1 << 7 : 0);

      if(cacheBits != infoBits)
      {
        continue; // early exit
      }

      // Now compare states
      // TODO: hash the binary content on pipeline creation
      // TODO: optimize by adding a lookup table storing size
      //       of each field and generalizing the type (void*)

      auto updateResult = [](const auto& lhs, const auto& rhs) {
        if(!rhs) return 0;
        return memcmp(lhs, rhs, sizeof(decltype(*rhs)));
      };

      auto result = 0u;
      result |= updateResult(info.vertexInputState, cacheInfo.vertexInputState);
      result |= updateResult(info.inputAssemblyState, cacheInfo.inputAssemblyState);
      result |= updateResult(info.rasterizationState, cacheInfo.rasterizationState);
      result |= updateResult(info.framebufferState, cacheInfo.framebufferState);
      result |= updateResult(info.colorBlendState, cacheInfo.colorBlendState);
      result |= updateResult(info.depthStencilState, cacheInfo.depthStencilState);
      result |= updateResult(info.viewportState, cacheInfo.viewportState);

      // early exit
      if(!result)
      {
        continue;
      }

      // For now ignoring dynamic state mask and allocator

      // Getting as far as here, we have found our pipeline impl
      return pipeline.get();
    }
  }
  return nullptr;
}

Graphics::UniquePtr<Graphics::Pipeline> PipelineCache::GetPipeline(const PipelineCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  // create or get from cache
  auto cachedPipeline = FindPipelineImpl(pipelineCreateInfo);

  if(!cachedPipeline)
  {
    // create new pipeline
    auto pipeline = MakeUnique<GLES::PipelineImpl>(pipelineCreateInfo, mImpl->controller);

    cachedPipeline = pipeline.get();

    // add it to cache
    mImpl->mPipelines.emplace_back(std::move(pipeline));
  }
  else
  {
  }
  // create new pipeline wrapper n
  auto wrapper = MakeUnique<GLES::Pipeline>(*cachedPipeline);
  return std::move(wrapper);
}

} // namespace Dali::Graphics::GLES