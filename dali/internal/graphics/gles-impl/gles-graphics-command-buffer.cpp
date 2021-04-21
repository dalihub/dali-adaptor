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
#include "gles-graphics-command-buffer.h"

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"
#include "gles-graphics-buffer.h"
#include "gles-graphics-framebuffer.h"
#include "gles-graphics-pipeline.h"
#include "gles-graphics-render-pass.h"
#include "gles-graphics-render-target.h"
#include "gles-graphics-texture.h"

namespace Dali::Graphics::GLES
{
CommandBuffer::CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller)
: CommandBufferResource(createInfo, controller)
{
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::BindVertexBuffers(uint32_t                             firstBinding,
                                      std::vector<const Graphics::Buffer*> buffers,
                                      std::vector<uint32_t>                offsets)
{
  mCommands.emplace_back(CommandType::BIND_VERTEX_BUFFERS);
  auto& bindings = mCommands.back().bindVertexBuffers.vertexBufferBindings;
  if(bindings.size() < firstBinding + buffers.size())
  {
    bindings.resize(firstBinding + buffers.size());
    auto index = firstBinding;
    for(auto& buf : buffers)
    {
      bindings[index].buffer = static_cast<const GLES::Buffer*>(buf);
      bindings[index].offset = offsets[index - firstBinding];
      index++;
    }
  }
}

void CommandBuffer::BindUniformBuffers(const std::vector<Graphics::UniformBufferBinding>& bindings)
{
  mCommands.emplace_back(CommandType::BIND_UNIFORM_BUFFER);
  auto& cmd     = mCommands.back();
  auto& bindCmd = cmd.bindUniformBuffers;
  for(const auto& binding : bindings)
  {
    if(binding.buffer)
    {
      auto glesBuffer = static_cast<const GLES::Buffer*>(binding.buffer);
      if(glesBuffer->IsCPUAllocated()) // standalone uniforms
      {
        bindCmd.standaloneUniformsBufferBinding.buffer   = glesBuffer;
        bindCmd.standaloneUniformsBufferBinding.offset   = binding.offset;
        bindCmd.standaloneUniformsBufferBinding.binding  = binding.binding;
        bindCmd.standaloneUniformsBufferBinding.emulated = true;
      }
      else // Bind regular UBO
      {
        // resize binding slots
        if(binding.binding >= bindCmd.uniformBufferBindings.size())
        {
          bindCmd.uniformBufferBindings.resize(binding.binding + 1);
        }
        auto& slot    = bindCmd.uniformBufferBindings[binding.binding];
        slot.buffer   = glesBuffer;
        slot.offset   = binding.offset;
        slot.binding  = binding.binding;
        slot.emulated = false;
      }
    }
  }
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  mCommands.emplace_back(CommandType::BIND_PIPELINE);
  mCommands.back().bindPipeline.pipeline = static_cast<const GLES::Pipeline*>(&pipeline);
}

void CommandBuffer::BindTextures(std::vector<TextureBinding>& textureBindings)
{
  mCommands.emplace_back(CommandType::BIND_TEXTURES);
  mCommands.back().bindTextures.textureBindings = std::move(textureBindings);
}

void CommandBuffer::BindSamplers(std::vector<SamplerBinding>& samplerBindings)
{
  mCommands.emplace_back(CommandType::BIND_SAMPLERS);
  mCommands.back().bindSamplers.samplerBindings = std::move(samplerBindings);
}

void CommandBuffer::BindPushConstants(void*    data,
                                      uint32_t size,
                                      uint32_t binding)
{
}

void CommandBuffer::BindIndexBuffer(const Graphics::Buffer& buffer,
                                    uint32_t                offset,
                                    Format                  format)
{
  mCommands.emplace_back(CommandType::BIND_INDEX_BUFFER);
  mCommands.back().bindIndexBuffer.buffer = static_cast<const GLES::Buffer*>(&buffer);
  mCommands.back().bindIndexBuffer.offset = offset;
  mCommands.back().bindIndexBuffer.format = format;
}

void CommandBuffer::BeginRenderPass(
  Graphics::RenderPass*   renderPass,
  Graphics::RenderTarget* renderTarget,
  Rect2D                  renderArea,
  std::vector<ClearValue> clearValues)
{
  mCommands.emplace_back(CommandType::BEGIN_RENDERPASS);
  auto& cmd                        = mCommands.back();
  cmd.beginRenderPass.renderPass   = static_cast<GLES::RenderPass*>(renderPass);
  cmd.beginRenderPass.renderTarget = static_cast<GLES::RenderTarget*>(renderTarget);
  cmd.beginRenderPass.renderArea   = renderArea;
  cmd.beginRenderPass.clearValues  = clearValues;
}

void CommandBuffer::EndRenderPass()
{
  mCommands.emplace_back(CommandType::END_RENDERPASS);
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers)
{
  mCommands.emplace_back(CommandType::EXECUTE_COMMAND_BUFFERS);
  auto& cmd = mCommands.back();
  cmd.executeCommandBuffers.buffers.reserve(commandBuffers.size());
  for(auto&& item : commandBuffers)
  {
    cmd.executeCommandBuffers.buffers.emplace_back(static_cast<const GLES::CommandBuffer*>(item));
  }
}

void CommandBuffer::Draw(
  uint32_t vertexCount,
  uint32_t instanceCount,
  uint32_t firstVertex,
  uint32_t firstInstance)
{
  mCommands.emplace_back(CommandType::DRAW);
  auto& cmd              = mCommands.back().draw;
  cmd.type               = DrawCallDescriptor::Type::DRAW;
  cmd.draw.vertexCount   = vertexCount;
  cmd.draw.instanceCount = instanceCount;
  cmd.draw.firstInstance = firstInstance;
  cmd.draw.firstVertex   = firstVertex;
}

void CommandBuffer::DrawIndexed(
  uint32_t indexCount,
  uint32_t instanceCount,
  uint32_t firstIndex,
  int32_t  vertexOffset,
  uint32_t firstInstance)
{
  mCommands.emplace_back(CommandType::DRAW_INDEXED);
  auto& cmd                     = mCommands.back().draw;
  cmd.type                      = DrawCallDescriptor::Type::DRAW_INDEXED;
  cmd.drawIndexed.firstIndex    = firstIndex;
  cmd.drawIndexed.firstInstance = firstInstance;
  cmd.drawIndexed.indexCount    = indexCount;
  cmd.drawIndexed.vertexOffset  = vertexOffset;
  cmd.drawIndexed.instanceCount = instanceCount;
}

void CommandBuffer::DrawIndexedIndirect(
  Graphics::Buffer& buffer,
  uint32_t          offset,
  uint32_t          drawCount,
  uint32_t          stride)
{
  mCommands.emplace_back(CommandType::DRAW_INDEXED_INDIRECT);
  auto& cmd                         = mCommands.back().draw;
  cmd.type                          = DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT;
  cmd.drawIndexedIndirect.buffer    = static_cast<const GLES::Buffer*>(&buffer);
  cmd.drawIndexedIndirect.offset    = offset;
  cmd.drawIndexedIndirect.drawCount = drawCount;
  cmd.drawIndexedIndirect.stride    = stride;
}

void CommandBuffer::Reset()
{
  mCommands.clear();
}

void CommandBuffer::SetScissor(Graphics::Rect2D value)
{
  mCommands.emplace_back(CommandType::SET_SCISSOR);
  mCommands.back().scissor.region = value;
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
  mCommands.emplace_back(CommandType::SET_SCISSOR_TEST);
  mCommands.back().scissorTest.enable = value;
}

void CommandBuffer::SetViewport(Viewport value)
{
  mCommands.emplace_back(CommandType::SET_VIEWPORT);
  mCommands.back().viewport.region = value;
}

void CommandBuffer::SetViewportEnable(bool value)
{
  // There is no GL equivalent
}

void CommandBuffer::PresentRenderTarget(GLES::RenderTarget* renderTarget)
{
  mCommands.emplace_back(CommandType::PRESENT_RENDER_TARGET);
  mCommands.back().presentRenderTarget.targetToPresent = renderTarget;
}

[[nodiscard]] const std::vector<Command>& CommandBuffer::GetCommands() const
{
  return mCommands;
}

void CommandBuffer::DestroyResource()
{
  // Nothing to do
}

bool CommandBuffer::InitializeResource()
{
  // Nothing to do
  return true;
}

void CommandBuffer::DiscardResource()
{
  GetController().DiscardResource(this);
}

} // namespace Dali::Graphics::GLES
