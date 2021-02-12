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
#include "gles-graphics-pipeline.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <memory>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"
#include "gles-graphics-shader.h"

namespace Dali::Graphics::GLES
{
/**
 * Copy of pipeline state, can be also used for internal caching
 */
struct PipelineImpl::PipelineState
{
  PipelineState()  = default;
  ~PipelineState() = default;
  ColorBlendState          colorBlendState;
  DepthStencilState        depthStencilState;
  std::vector<ShaderState> shaderState;
  ViewportState            viewportState;
  FramebufferState         framebufferState;
  RasterizationState       rasterizationState;
  VertexInputState         vertexInputState;
  InputAssemblyState       inputAssemblyState;
};

PipelineImpl::PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: mController(controller)
{
  // the creation is deferred so it's needed to copy certain parts of the CreateInfo structure
  mPipelineState = std::make_unique<PipelineImpl::PipelineState>();

  // Make copies of structured pass by pointers and replace
  // stored create info structure fields
  CopyStateIfSet(createInfo.inputAssemblyState, mPipelineState->inputAssemblyState, &mCreateInfo.inputAssemblyState);
  CopyStateIfSet(createInfo.vertexInputState, mPipelineState->vertexInputState, &mCreateInfo.vertexInputState);
  CopyStateIfSet(createInfo.rasterizationState, mPipelineState->rasterizationState, &mCreateInfo.rasterizationState);
  CopyStateIfSet(createInfo.framebufferState, mPipelineState->framebufferState, &mCreateInfo.framebufferState);
  CopyStateIfSet(createInfo.colorBlendState, mPipelineState->colorBlendState, &mCreateInfo.colorBlendState);
  CopyStateIfSet(createInfo.depthStencilState, mPipelineState->depthStencilState, &mCreateInfo.depthStencilState);
  CopyStateIfSet(createInfo.shaderState, mPipelineState->shaderState, &mCreateInfo.shaderState);
  CopyStateIfSet(createInfo.viewportState, mPipelineState->viewportState, &mCreateInfo.viewportState);
}

const PipelineCreateInfo& PipelineImpl::GetCreateInfo() const
{
  return mCreateInfo;
}

auto& PipelineImpl::GetController() const
{
  return mController;
}

bool PipelineImpl::InitializeResource()
{
  auto& gl   = *GetController().GetGL();
  mGlProgram = gl.CreateProgram();

  for(auto& shader : mPipelineState->shaderState)
  {
    // TODO: Compile shader (shouldn't compile if already did so)
    const auto glesShader = static_cast<const GLES::Shader*>(shader.shader);
    if(glesShader->Compile())
    {
      // Attach shader
      gl.AttachShader(mGlProgram, glesShader->GetGLShader());
    }
    else
    {
      // failure
      gl.DeleteProgram(mGlProgram);
      return false;
    }
  }

  // Link program
  gl.LinkProgram(mGlProgram);

  // Check for errors
  // TODO:

  return true;
}

void PipelineImpl::DestroyResource()
{
  if(mGlProgram)
  {
    auto& gl = *GetController().GetGL();
    gl.DeleteProgram(mGlProgram);
  }
}

void PipelineImpl::DiscardResource()
{
  // Pass program to discard queue
}

uint32_t PipelineImpl::GetGLProgram() const
{
  return mGlProgram;
}

// FIXME: THIS FUNCTION IS NOT IN USE YET, REQUIRES PROPER PIPELINE
void PipelineImpl::Bind(GLES::PipelineImpl* prevPipeline)
{
  // Same pipeline to bind, nothing to do
  if(prevPipeline == this)
  {
    return;
  }

  auto& gl = *GetController().GetGL();

  // ------------------ VIEWPORT

  const auto& viewportState    = mPipelineState->viewportState;
  auto        setViewportState = [this, &gl, viewportState]() {
    if(viewportState.scissorTestEnable)
    {
      gl.Enable(GL_SCISSOR_TEST);
      const auto& scissor = mPipelineState->viewportState.scissor;
      gl.Scissor(scissor.x, scissor.y, scissor.width, scissor.height);
    }
    else
    {
      gl.Disable(GL_SCISSOR_TEST);
      const auto& scissor = mPipelineState->viewportState.scissor;
      gl.Scissor(scissor.x, scissor.y, scissor.width, scissor.height);
    }
  };

  // Resolve viewport/scissor state change
  ExecuteStateChange(setViewportState, prevPipeline->GetCreateInfo().viewportState, &mPipelineState->viewportState);

  // ---------------------- PROGRAM
  auto program = mGlProgram;

  gl.UseProgram(program);
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

} // namespace Dali::Graphics::GLES