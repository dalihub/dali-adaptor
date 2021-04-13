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
#include <dali/graphics-api/graphics-command-buffer-create-info.h>
#include <dali/graphics-api/graphics-command-buffer.h>
#include <dali/graphics-api/graphics-types.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"
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
 * @brief Helper function to invoke destructor on anonymous struct
 */
template<class T>
static void InvokeDestructor(T& object)
{
  object.~T();
}

/**
 * Command structure allocates memory to store a single command
 */
struct Command
{
  Command() = delete;

  Command(CommandType commandType)
  {
    type = commandType;
    switch(type)
    {
      case CommandType::BIND_VERTEX_BUFFERS:
      {
        new(&bindVertexBuffers) decltype(bindVertexBuffers);
        break;
      }
      case CommandType::BIND_TEXTURES:
      {
        new(&bindTextures) decltype(bindTextures);
        break;
      }
      default:
      {
      }
    }
  }

  ~Command()
  {
    switch(type)
    {
      case CommandType::BIND_VERTEX_BUFFERS:
      {
        InvokeDestructor(bindVertexBuffers);
        break;
      }
      case CommandType::BIND_TEXTURES:
      {
        InvokeDestructor(bindTextures);
        break;
      }
      default:
      {
      }
    }
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
        new(&bindVertexBuffers) decltype(bindVertexBuffers);
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
        new(&bindTextures) decltype(bindTextures);
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
        new(&bindVertexBuffers) decltype(bindVertexBuffers);
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
        new(&bindTextures) decltype(bindTextures);
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
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller);

  ~CommandBuffer() override;

  void BindVertexBuffers(uint32_t                             firstBinding,
                         std::vector<const Graphics::Buffer*> buffers,
                         std::vector<uint32_t>                offsets) override;

  void BindUniformBuffers(const std::vector<Graphics::UniformBufferBinding>& bindings) override;

  void BindPipeline(const Graphics::Pipeline& pipeline) override;

  void BindTextures(std::vector<TextureBinding>& textureBindings) override;

  void BindSamplers(std::vector<SamplerBinding>& samplerBindings) override;

  void BindPushConstants(void*    data,
                         uint32_t size,
                         uint32_t binding) override;

  void BindIndexBuffer(const Graphics::Buffer& buffer,
                       uint32_t                offset,
                       Format                  format) override;

  void BeginRenderPass(
    Graphics::RenderPass&   renderPass,
    Graphics::RenderTarget& renderTarget,
    Extent2D                renderArea,
    std::vector<ClearValue> clearValues) override;

  /**
   * @brief Ends current render pass
   *
   * This command must be issued in order to finalize the render pass.
   * It's up to the implementation whether anything has to be done but
   * the Controller may use end RP marker in order to resolve resource
   * dependencies (for example, to know when target texture is ready
   * before passing it to another render pass).
   */
  void EndRenderPass() override;

  void Draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) override;

  void DrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t  vertexOffset,
    uint32_t firstInstance) override;

  void DrawIndexedIndirect(
    Graphics::Buffer& buffer,
    uint32_t          offset,
    uint32_t          drawCount,
    uint32_t          stride) override;

  void Reset() override;

  void SetScissor(Graphics::Rect2D value) override;

  void SetScissorTestEnable(bool value) override;

  void SetViewport(Viewport value) override;

  void SetViewportEnable(bool value) override;

  [[nodiscard]] const std::vector<Command>& GetCommands() const;

  void DestroyResource() override;
  bool InitializeResource() override;

  void DiscardResource() override;

private:
  std::vector<Command> mCommands;
};
} // namespace Dali::Graphics::GLES

#endif
