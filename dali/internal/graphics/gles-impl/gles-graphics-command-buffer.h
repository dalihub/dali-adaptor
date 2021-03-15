#ifndef DALI_GRAPHICS_GLES_COMMAND_BUFFER_H
#define DALI_GRAPHICS_GLES_COMMAND_BUFFER_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-command-buffer.h>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"
#include "gles-graphics-buffer.h"
#include "gles-graphics-pipeline.h"
#include "gles-graphics-types.h"

namespace Dali::Graphics::GLES
{
class Texture;
class Pipeline;

enum class CommandType
{
  FLUSH,
  BIND_TEXTURES,
  BIND_SAMPLERS,
  BIND_VERTEX_BUFFERS,
  BIND_INDEX_BUFFER,
  BIND_UNIFORM_BUFFER,
  BIND_PIPELINE,
  DRAW,
  DRAW_INDEXED,
  DRAW_INDEXED_INDIRECT,
  SET_SCISSOR,
  SET_SCISSOR_TEST,
  SET_VIEWPORT
};

/**
 * Command structure allocates memory to store a single command
 */
struct Command
{
  Command()
  {
  }

  ~Command()
  {
  }

  /**
   * @brief Copy constructor
   * @param[in] rhs Command
   */
  Command(const Command& rhs)
  {
    switch(rhs.type)
    {
      case CommandType::BIND_VERTEX_BUFFERS:
      {
        bindVertexBuffers = rhs.bindVertexBuffers;
        break;
      }
      case CommandType::BIND_INDEX_BUFFER:
      {
        bindIndexBuffer = rhs.bindIndexBuffer;
        break;
      }
      case CommandType::BIND_SAMPLERS:
      {
        bindSamplers = rhs.bindSamplers;
        break;
      }
      case CommandType::BIND_TEXTURES:
      {
        bindTextures = rhs.bindTextures;
        break;
      }
      case CommandType::BIND_PIPELINE:
      {
        bindPipeline = rhs.bindPipeline;
        break;
      }
      case CommandType::BIND_UNIFORM_BUFFER:
      {
        bindUniformBuffers = rhs.bindUniformBuffers;
        break;
      }
      case CommandType::DRAW:
      {
        draw.type = rhs.draw.type;
        draw.draw = rhs.draw.draw;
        break;
      }
      case CommandType::DRAW_INDEXED:
      {
        draw.type        = rhs.draw.type;
        draw.drawIndexed = rhs.draw.drawIndexed;
        break;
      }
      case CommandType::DRAW_INDEXED_INDIRECT:
      {
        draw.type                = rhs.draw.type;
        draw.drawIndexedIndirect = rhs.draw.drawIndexedIndirect;
        break;
      }
      case CommandType::FLUSH:
      {
        // Nothing to do
        break;
      }
      case CommandType::SET_SCISSOR:
      {
        scissor.region = rhs.scissor.region;
        break;
      }
      case CommandType::SET_SCISSOR_TEST:
      {
        scissorTest.enable = rhs.scissorTest.enable;
        break;
      }
      case CommandType::SET_VIEWPORT:
      {
        viewport.region = rhs.viewport.region;
        break;
      }
    }
    type = rhs.type;
  }

  /**
   * @brief Copy constructor
   * @param[in] rhs Command
   */
  Command(Command&& rhs) noexcept
  {
    switch(rhs.type)
    {
      case CommandType::BIND_VERTEX_BUFFERS:
      {
        bindVertexBuffers = std::move(rhs.bindVertexBuffers);
        break;
      }
      case CommandType::BIND_INDEX_BUFFER:
      {
        bindIndexBuffer = rhs.bindIndexBuffer;
        break;
      }
      case CommandType::BIND_UNIFORM_BUFFER:
      {
        bindUniformBuffers = std::move(rhs.bindUniformBuffers);
        break;
      }
      case CommandType::BIND_SAMPLERS:
      {
        bindSamplers = std::move(rhs.bindSamplers);
        break;
      }
      case CommandType::BIND_TEXTURES:
      {
        bindTextures = std::move(rhs.bindTextures);
        break;
      }
      case CommandType::BIND_PIPELINE:
      {
        bindPipeline = rhs.bindPipeline;
        break;
      }
      case CommandType::DRAW:
      {
        draw.type = rhs.draw.type;
        draw.draw = rhs.draw.draw;
        break;
      }
      case CommandType::DRAW_INDEXED:
      {
        draw.type        = rhs.draw.type;
        draw.drawIndexed = rhs.draw.drawIndexed;
        break;
      }
      case CommandType::DRAW_INDEXED_INDIRECT:
      {
        draw.type                = rhs.draw.type;
        draw.drawIndexedIndirect = rhs.draw.drawIndexedIndirect;
        break;
      }
      case CommandType::FLUSH:
      {
        // Nothing to do
        break;
      }
      case CommandType::SET_SCISSOR:
      {
        scissor.region = rhs.scissor.region;
        break;
      }
      case CommandType::SET_SCISSOR_TEST:
      {
        scissorTest.enable = rhs.scissorTest.enable;
        break;
      }
      case CommandType::SET_VIEWPORT:
      {
        viewport.region = rhs.viewport.region;
        break;
      }
    }
    type = rhs.type;
  }

  CommandType type{CommandType::FLUSH}; ///< Type of command

  union
  {
    struct
    {
      std::vector<Graphics::TextureBinding> textureBindings;
    } bindTextures{};

    // BindSampler command
    struct
    {
      std::vector<Graphics::SamplerBinding> samplerBindings;
    } bindSamplers;

    struct
    {
      using Binding = GLES::VertexBufferBindingDescriptor;
      std::vector<Binding> vertexBufferBindings;
    } bindVertexBuffers;

    struct : public IndexBufferBindingDescriptor
    {
    } bindIndexBuffer;

    struct
    {
      std::vector<UniformBufferBindingDescriptor> uniformBufferBindings{};
      UniformBufferBindingDescriptor              standaloneUniformsBufferBinding{};
    } bindUniformBuffers;

    struct
    {
      const GLES::Pipeline* pipeline{nullptr};
    } bindPipeline;

    struct : public DrawCallDescriptor
    {
    } draw;

    struct
    {
      Graphics::Rect2D region;
    } scissor;

    struct
    {
      bool enable;
    } scissorTest;

    struct
    {
      Graphics::Viewport region;
    } viewport;
  };
};

using CommandBufferResource = Resource<Graphics::CommandBuffer, Graphics::CommandBufferCreateInfo>;

class CommandBuffer : public CommandBufferResource
{
public:
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller)
  : CommandBufferResource(createInfo, controller)
  {
  }

  ~CommandBuffer() override = default;

  void BindVertexBuffers(uint32_t                             firstBinding,
                         std::vector<const Graphics::Buffer*> buffers,
                         std::vector<uint32_t>                offsets) override
  {
    mCommands.emplace_back();
    mCommands.back().type = CommandType::BIND_VERTEX_BUFFERS;
    auto& bindings        = mCommands.back().bindVertexBuffers.vertexBufferBindings;
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

  void BindUniformBuffers(const std::vector<Graphics::UniformBufferBinding>& bindings) override
  {
    mCommands.emplace_back();
    auto& cmd     = mCommands.back();
    cmd.type      = CommandType::BIND_UNIFORM_BUFFER;
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

  void BindPipeline(const Graphics::Pipeline& pipeline) override
  {
    mCommands.emplace_back();
    mCommands.back().type                  = CommandType::BIND_PIPELINE;
    mCommands.back().bindPipeline.pipeline = static_cast<const GLES::Pipeline*>(&pipeline);
  }

  void BindTextures(std::vector<TextureBinding>& textureBindings) override
  {
    mCommands.emplace_back();
    mCommands.back().type                         = CommandType::BIND_TEXTURES;
    mCommands.back().bindTextures.textureBindings = std::move(textureBindings);
  }

  void BindSamplers(std::vector<SamplerBinding>& samplerBindings) override
  {
    mCommands.emplace_back();
    mCommands.back().bindSamplers.samplerBindings = std::move(samplerBindings);
  }

  void BindPushConstants(void*    data,
                         uint32_t size,
                         uint32_t binding) override
  {
  }

  void BindIndexBuffer(const Graphics::Buffer& buffer,
                       uint32_t                offset,
                       Format                  format) override
  {
    mCommands.emplace_back();
    mCommands.back().type                   = CommandType::BIND_INDEX_BUFFER;
    mCommands.back().bindIndexBuffer.buffer = static_cast<const GLES::Buffer*>(&buffer);
    mCommands.back().bindIndexBuffer.offset = offset;
    mCommands.back().bindIndexBuffer.format = format;
  }

  void BeginRenderPass(
    Graphics::RenderPass&   renderPass,
    Graphics::RenderTarget& renderTarget,
    Extent2D                renderArea,
    std::vector<ClearValue> clearValues) override
  {
  }

  /**
   * @brief Ends current render pass
   *
   * This command must be issued in order to finalize the render pass.
   * It's up to the implementation whether anything has to be done but
   * the Controller may use end RP marker in order to resolve resource
   * dependencies (for example, to know when target texture is ready
   * before passing it to another render pass).
   */
  void EndRenderPass() override
  {
  }

  void Draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) override
  {
    mCommands.emplace_back();
    mCommands.back().type  = CommandType::DRAW;
    auto& cmd              = mCommands.back().draw;
    cmd.type               = DrawCallDescriptor::Type::DRAW;
    cmd.draw.vertexCount   = vertexCount;
    cmd.draw.instanceCount = instanceCount;
    cmd.draw.firstInstance = firstInstance;
    cmd.draw.firstVertex   = firstVertex;
  }

  void DrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t  vertexOffset,
    uint32_t firstInstance) override
  {
    mCommands.emplace_back();
    mCommands.back().type         = CommandType::DRAW_INDEXED;
    auto& cmd                     = mCommands.back().draw;
    cmd.type                      = DrawCallDescriptor::Type::DRAW_INDEXED;
    cmd.drawIndexed.firstIndex    = firstIndex;
    cmd.drawIndexed.firstInstance = firstInstance;
    cmd.drawIndexed.indexCount    = indexCount;
    cmd.drawIndexed.vertexOffset  = vertexOffset;
    cmd.drawIndexed.instanceCount = instanceCount;
  }

  void DrawIndexedIndirect(
    Graphics::Buffer& buffer,
    uint32_t          offset,
    uint32_t          drawCount,
    uint32_t          stride) override
  {
    mCommands.emplace_back();
    mCommands.back().type             = CommandType::DRAW_INDEXED_INDIRECT;
    auto& cmd                         = mCommands.back().draw;
    cmd.type                          = DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT;
    cmd.drawIndexedIndirect.buffer    = static_cast<const GLES::Buffer*>(&buffer);
    cmd.drawIndexedIndirect.offset    = offset;
    cmd.drawIndexedIndirect.drawCount = drawCount;
    cmd.drawIndexedIndirect.stride    = stride;
  }

  void Reset() override
  {
    mCommands.clear();
  }

  void SetScissor(Graphics::Rect2D value) override
  {
    mCommands.emplace_back();
    mCommands.back().type           = CommandType::SET_SCISSOR;
    mCommands.back().scissor.region = value;
  }

  void SetScissorTestEnable(bool value) override
  {
    mCommands.emplace_back();
    mCommands.back().type               = CommandType::SET_SCISSOR_TEST;
    mCommands.back().scissorTest.enable = value;
  }

  void SetViewport(Viewport value) override
  {
    mCommands.emplace_back();
    mCommands.back().type            = CommandType::SET_VIEWPORT;
    mCommands.back().viewport.region = value;
  }

  void SetViewportEnable(bool value) override
  {
    // There is no GL equivalent
  }

  [[nodiscard]] const std::vector<Command>& GetCommands() const
  {
    return mCommands;
  }

  void DestroyResource() override
  {
    // Nothing to do
  }

  bool InitializeResource() override
  {
    // Nothing to do
    return true;
  }

  void DiscardResource() override
  {
    // Nothing to do
  }

private:
  std::vector<Command> mCommands;
};
} // namespace Dali::Graphics::GLES

#endif
