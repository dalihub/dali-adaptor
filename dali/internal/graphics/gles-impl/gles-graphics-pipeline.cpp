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
#include "gles-graphics-pipeline.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-defines.h>
#include <memory>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"
#include "gles-graphics-pipeline-cache.h"
#include "gles-graphics-program.h"

namespace Dali::Graphics::GLES
{
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

  PipelineCache* pipelineCache{};
};

PipelineImpl::PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, Graphics::EglGraphicsController& controller, PipelineCache& pipelineCache)
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

  // This program doesn't need custom deleter
  auto programImpl        = static_cast<const GLES::Program*>(createInfo.programState->program)->GetImplementation();
  mPipelineState->program = MakeUnique<GLES::Program>(programImpl);

  // To make sure the program is alive as long as the pipeline is!
  mCreateInfo.programState->program = mPipelineState->program.get();

  // Set pipeline cache
  mPipelineState->pipelineCache = &pipelineCache;
}

const PipelineCreateInfo& PipelineImpl::GetCreateInfo() const
{
  return mCreateInfo;
}

auto& PipelineImpl::GetController() const
{
  return mController;
}

void PipelineImpl::Bind(const uint32_t glProgram) const
{
  if(DALI_UNLIKELY(EglGraphicsController::IsShuttingDown()))
  {
    return; // Early out if shutting down
  }

  auto* gl = GetController().GetGL();
  if(DALI_LIKELY(gl))
  {
    gl->UseProgram(glProgram);
  }
}

void PipelineImpl::Retain()
{
  ++mRefCount;
}

void PipelineImpl::Release()
{
  --mRefCount;
}

uint32_t PipelineImpl::GetRefCount() const
{
  return mRefCount;
}

PipelineImpl::~PipelineImpl() = default;

// PIPELINE WRAPPER

const PipelineCreateInfo& Pipeline::GetCreateInfo() const
{
  return mPipeline.GetCreateInfo();
}

EglGraphicsController& Pipeline::GetController() const
{
  return mPipeline.GetController();
}

Pipeline::~Pipeline()
{
  // decrease refcount
  if(mPipeline.GetRefCount())
  {
    mPipeline.Release();
  }
}

void Pipeline::DiscardResource()
{
  // Send pipeline to discard queue if refcount is 0
  GetController().DiscardResource(this);
}

} // namespace Dali::Graphics::GLES
