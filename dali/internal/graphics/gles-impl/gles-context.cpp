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

#include "gles-context.h"
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include "gles-graphics-buffer.h"
#include "gles-graphics-command-buffer.h"
#include "gles-graphics-pipeline.h"

namespace Dali::Graphics::GLES
{
struct Context::Impl
{
  Impl(EglGraphicsController& controller)
  : mController(controller)
  {
  }

  ~Impl() = default;

  EglGraphicsController& mController;

  const GLES::Pipeline* mCurrentPipeline{nullptr}; ///< Currently bound pipeline
  const GLES::Pipeline* mNewPipeline{nullptr};     ///< New pipeline to be set on flush

  std::vector<Graphics::TextureBinding> mCurrentTextureBindings{};
  std::vector<Graphics::SamplerBinding> mCurrentSamplerBindings{};
  GLES::IndexBufferBindingDescriptor    mCurrentIndexBufferBinding{};

  struct VertexBufferBinding
  {
    GLES::Buffer* buffer{nullptr};
    uint32_t      offset{0u};
  };

  // Currently bound buffers
  std::vector<VertexBufferBindingDescriptor> mCurrentVertexBufferBindings{};
};

Context::Context(EglGraphicsController& controller)
{
  mImpl = std::make_unique<Impl>(controller);
}

Context::~Context() = default;

void Context::Flush(bool reset, const GLES::DrawCallDescriptor& drawCall)
{
  auto& gl = *mImpl->mController.GetGL();

  // Change pipeline
  if(mImpl->mNewPipeline)
  {
    // Execute states if different
    mImpl->mCurrentPipeline = mImpl->mNewPipeline;
    mImpl->mNewPipeline     = nullptr;
  }

  // Blend state
  ResolveBlendState();

  // Resolve rasterization state
  ResolveRasterizationState();

  // Bind textures
  for(const auto& binding : mImpl->mCurrentTextureBindings)
  {
    gl.ActiveTexture(GL_TEXTURE0 + binding.binding);
    gl.BindTexture(GL_TEXTURE_2D,
                   static_cast<const GLES::Texture*>(binding.texture)
                     ->GetGLTexture());
  }

  // for each attribute bind vertices
  const auto& pipelineState = mImpl->mCurrentPipeline->GetCreateInfo();
  const auto& vi            = pipelineState.vertexInputState;
  for(const auto& attr : vi->attributes)
  {
    // Enable location
    gl.EnableVertexAttribArray(attr.location);
    const auto& bufferSlot    = mImpl->mCurrentVertexBufferBindings[attr.binding];
    const auto& bufferBinding = vi->bufferBindings[attr.binding];

    auto glesBuffer = bufferSlot.buffer->GetGLBuffer();

    // Bind buffer
    gl.BindBuffer(GL_ARRAY_BUFFER, glesBuffer);
    gl.VertexAttribPointer(attr.location,
                           GLVertexFormat(attr.format).size,
                           GLVertexFormat(attr.format).format,
                           GL_FALSE,
                           bufferBinding.stride,
                           reinterpret_cast<void*>(attr.offset));
  }

  // Resolve topology
  const auto& ia = mImpl->mCurrentPipeline->GetCreateInfo().inputAssemblyState;

  // Resolve draw call
  switch(drawCall.type)
  {
    case DrawCallDescriptor::Type::DRAW:
    {
      gl.DrawArrays(GLESTopology(ia->topology),
                    drawCall.draw.firstVertex,
                    drawCall.draw.vertexCount);
      break;
    }
    case DrawCallDescriptor::Type::DRAW_INDEXED:
    {
      const auto& binding    = mImpl->mCurrentIndexBufferBinding;
      const auto* glesBuffer = binding.buffer;
      gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, glesBuffer->GetGLBuffer());
      auto indexBufferFormat = GLIndexFormat(binding.format).format;
      gl.DrawElements(GLESTopology(ia->topology),
                      drawCall.drawIndexed.indexCount,
                      indexBufferFormat,
                      reinterpret_cast<void*>(binding.offset));
      break;
    }
    case DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT:
    {
      break;
    }
  }
}

void Context::BindTextures(const std::vector<Graphics::TextureBinding>& bindings)
{
  // for each texture allocate slot
  for(const auto& binding : bindings)
  {
    // Resize binding array if needed
    if(mImpl->mCurrentTextureBindings.size() <= binding.binding)
    {
      mImpl->mCurrentTextureBindings.resize(binding.binding + 1);
    }
    // Store the binding details
    mImpl->mCurrentTextureBindings[binding.binding] = binding;
  }
}

void Context::BindVertexBuffers(const std::vector<GLES::VertexBufferBindingDescriptor>& bindings)
{
  if(bindings.size() > mImpl->mCurrentVertexBufferBindings.size())
  {
    mImpl->mCurrentVertexBufferBindings.resize(bindings.size());
  }
  // Copy only set slots
  std::copy_if(bindings.begin(), bindings.end(), mImpl->mCurrentVertexBufferBindings.begin(), [](auto& item) {
    return (nullptr != item.buffer);
  });
}

void Context::BindIndexBuffer(const IndexBufferBindingDescriptor& indexBufferBinding)
{
  mImpl->mCurrentIndexBufferBinding = indexBufferBinding;
}

void Context::BindPipeline(const GLES::Pipeline* newPipeline)
{
  mImpl->mNewPipeline = newPipeline;
}

void Context::ResolveBlendState()
{
  const auto& state = mImpl->mCurrentPipeline->GetCreateInfo();
  const auto& bs    = state.colorBlendState;
  auto&       gl    = *mImpl->mController.GetGL();

  // TODO: prevent leaking the state
  if(!bs)
  {
    return;
  }

  bs->blendEnable ? gl.Enable(GL_BLEND) : gl.Disable(GL_BLEND);
  if(!bs->blendEnable)
  {
    return;
  }

  gl.BlendFunc(GLBlendFunc(bs->srcColorBlendFactor), GLBlendFunc(bs->dstColorBlendFactor));

  if((GLBlendFunc(bs->srcColorBlendFactor) == GLBlendFunc(bs->srcAlphaBlendFactor)) &&
     (GLBlendFunc(bs->dstColorBlendFactor) == GLBlendFunc(bs->dstAlphaBlendFactor)))
  {
    gl.BlendFunc(GLBlendFunc(bs->srcColorBlendFactor), GLBlendFunc(bs->dstColorBlendFactor));
  }
  else
  {
    gl.BlendFuncSeparate(GLBlendFunc(bs->srcColorBlendFactor),
                         GLBlendFunc(bs->dstColorBlendFactor),
                         GLBlendFunc(bs->srcAlphaBlendFactor),
                         GLBlendFunc(bs->dstAlphaBlendFactor));
  }
  if(GLBlendOp(bs->colorBlendOp) == GLBlendOp(bs->alphaBlendOp))
  {
    gl.BlendEquation(GLBlendOp(bs->colorBlendOp));
  }
  else
  {
    gl.BlendEquationSeparate(GLBlendOp(bs->colorBlendOp), GLBlendOp(bs->alphaBlendOp));
  }
}

void Context::ResolveRasterizationState()
{
  const auto& state = mImpl->mCurrentPipeline->GetCreateInfo();
  const auto& rs    = state.rasterizationState;
  auto&       gl    = *mImpl->mController.GetGL();

  // TODO: prevent leaking the state
  if(!rs)
  {
    return;
  }

  if(rs->cullMode == CullMode::NONE)
  {
    gl.Disable(GL_CULL_FACE);
  }
  else
  {
    gl.Enable(GL_CULL_FACE);
    gl.CullFace(GLCullMode(rs->cullMode));
  }

  // TODO: implement polygon mode (fill, line, points)
  //       seems like we don't support it (no glPolygonMode())
}

} // namespace Dali::Graphics::GLES
