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
class Pipeline;
class RenderPass;
class Framebuffer;
class CommandBuffer;
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
  SET_VIEWPORT,
  BEGIN_RENDERPASS,
  END_RENDERPASS,
  EXECUTE_COMMAND_BUFFERS,
  PRESENT_RENDER_TARGET,
  SET_COLOR_MASK,
  CLEAR_STENCIL_BUFFER,
  CLEAR_DEPTH_BUFFER,
  SET_STENCIL_TEST_ENABLE,
  SET_STENCIL_WRITE_MASK,
  SET_STENCIL_OP,
  SET_STENCIL_FUNC,
  SET_DEPTH_COMPARE_OP,
  SET_DEPTH_TEST_ENABLE,
  SET_DEPTH_WRITE_ENABLE,
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
      case CommandType::BEGIN_RENDERPASS:
      {
        // run destructor
        new(&beginRenderPass) decltype(beginRenderPass);
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
      case CommandType::BEGIN_RENDERPASS:
      {
        // run destructor
        InvokeDestructor(beginRenderPass);
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
      case CommandType::BEGIN_RENDERPASS:
      {
        new(&beginRenderPass) BeginRenderPassDescriptor(rhs.beginRenderPass);
        break;
      }
      case CommandType::END_RENDERPASS:
      {
        break;
      }
      case CommandType::EXECUTE_COMMAND_BUFFERS:
      {
        executeCommandBuffers = rhs.executeCommandBuffers;
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
      case CommandType::PRESENT_RENDER_TARGET:
      {
        presentRenderTarget = rhs.presentRenderTarget;
        break;
      }
      case CommandType::SET_COLOR_MASK:
      {
        colorMask.enabled = rhs.colorMask.enabled;
        break;
      }
      case CommandType::CLEAR_STENCIL_BUFFER:
      {
        break;
      }
      case CommandType::CLEAR_DEPTH_BUFFER:
      {
        break;
      }
      case CommandType::SET_STENCIL_TEST_ENABLE:
      {
        stencilTest.enabled = rhs.stencilTest.enabled;
        break;
      }
      case CommandType::SET_STENCIL_FUNC:
      {
        stencilFunc.compareMask = rhs.stencilFunc.compareMask;
        stencilFunc.compareOp   = rhs.stencilFunc.compareOp;
        stencilFunc.reference   = rhs.stencilFunc.reference;
        break;
      }
      case CommandType::SET_STENCIL_WRITE_MASK:
      {
        stencilWriteMask.mask = rhs.stencilWriteMask.mask;
        break;
      }
      case CommandType::SET_STENCIL_OP:
      {
        stencilOp.failOp      = rhs.stencilOp.failOp;
        stencilOp.depthFailOp = rhs.stencilOp.depthFailOp;
        stencilOp.passOp      = rhs.stencilOp.passOp;
        break;
      }

      case CommandType::SET_DEPTH_COMPARE_OP:
      {
        depth.compareOp = rhs.depth.compareOp;
        break;
      }
      case CommandType::SET_DEPTH_TEST_ENABLE:
      {
        depth.testEnabled = rhs.depth.testEnabled;
        break;
      }
      case CommandType::SET_DEPTH_WRITE_ENABLE:
      {
        depth.writeEnabled = rhs.depth.writeEnabled;
        break;
      }
    }
    type = rhs.type;
  }

  /**
   * @brief Move constructor
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
      case CommandType::BEGIN_RENDERPASS:
      {
        new(&beginRenderPass) BeginRenderPassDescriptor(std::move(rhs.beginRenderPass));
        break;
      }
      case CommandType::END_RENDERPASS:
      {
        break;
      }
      case CommandType::EXECUTE_COMMAND_BUFFERS:
      {
        executeCommandBuffers = std::move(rhs.executeCommandBuffers);
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
      case CommandType::PRESENT_RENDER_TARGET:
      {
        presentRenderTarget = rhs.presentRenderTarget;
        break;
      }
      case CommandType::SET_COLOR_MASK:
      {
        colorMask.enabled = rhs.colorMask.enabled;
        break;
      }
      case CommandType::CLEAR_STENCIL_BUFFER:
      {
        break;
      }
      case CommandType::CLEAR_DEPTH_BUFFER:
      {
        break;
      }
      case CommandType::SET_STENCIL_TEST_ENABLE:
      {
        stencilTest.enabled = rhs.stencilTest.enabled;
        break;
      }
      case CommandType::SET_STENCIL_FUNC:
      {
        stencilFunc.compareMask = rhs.stencilFunc.compareMask;
        stencilFunc.compareOp   = rhs.stencilFunc.compareOp;
        stencilFunc.reference   = rhs.stencilFunc.reference;
        break;
      }
      case CommandType::SET_STENCIL_WRITE_MASK:
      {
        stencilWriteMask.mask = rhs.stencilWriteMask.mask;
        break;
      }
      case CommandType::SET_STENCIL_OP:
      {
        stencilOp.failOp      = rhs.stencilOp.failOp;
        stencilOp.depthFailOp = rhs.stencilOp.depthFailOp;
        stencilOp.passOp      = rhs.stencilOp.passOp;
        break;
      }

      case CommandType::SET_DEPTH_COMPARE_OP:
      {
        depth.compareOp = rhs.depth.compareOp;
        break;
      }
      case CommandType::SET_DEPTH_TEST_ENABLE:
      {
        depth.testEnabled = rhs.depth.testEnabled;
        break;
      }
      case CommandType::SET_DEPTH_WRITE_ENABLE:
      {
        depth.writeEnabled = rhs.depth.writeEnabled;
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

    struct BeginRenderPassDescriptor
      beginRenderPass;

    struct
    {
    } endRenderPass;

    struct
    {
      std::vector<const GLES::CommandBuffer*> buffers;
    } executeCommandBuffers;

    struct
    {
      GLES::RenderTarget* targetToPresent;
    } presentRenderTarget;

    struct
    {
      Graphics::CompareOp compareOp;
      bool                testEnabled;
      bool                writeEnabled;
    } depth;

    struct
    {
      Graphics::StencilOp failOp;
      Graphics::StencilOp passOp;
      Graphics::StencilOp depthFailOp;
    } stencilOp;

    struct
    {
      uint32_t mask;
    } stencilWriteMask;

    struct
    {
      uint32_t            compareMask;
      Graphics::CompareOp compareOp;
      uint32_t            reference;
    } stencilFunc;

    struct
    {
      bool enabled;
    } stencilTest;

    struct
    {
      bool enabled;
    } colorMask;
  };
};

using CommandBufferResource = Resource<Graphics::CommandBuffer, Graphics::CommandBufferCreateInfo>;

class CommandBuffer : public CommandBufferResource
{
public:
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller);

  ~CommandBuffer() override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindVertexBuffers
   */
  void BindVertexBuffers(uint32_t                             firstBinding,
                         std::vector<const Graphics::Buffer*> buffers,
                         std::vector<uint32_t>                offsets) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindUniformBuffers
   */
  void BindUniformBuffers(const std::vector<Graphics::UniformBufferBinding>& bindings) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindPipeline
   */
  void BindPipeline(const Graphics::Pipeline& pipeline) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindTextures
   */
  void BindTextures(std::vector<TextureBinding>& textureBindings) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindSamplers
   */
  void BindSamplers(std::vector<SamplerBinding>& samplerBindings) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindPushConstants
   */
  void BindPushConstants(void*    data,
                         uint32_t size,
                         uint32_t binding) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindIndexBuffer
   */
  void BindIndexBuffer(const Graphics::Buffer& buffer,
                       uint32_t                offset,
                       Format                  format) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BeginRenderPass
   */
  void BeginRenderPass(
    Graphics::RenderPass*   renderPass,
    Graphics::RenderTarget* renderTarget,
    Rect2D                  renderArea,
    std::vector<ClearValue> clearValues) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::EndRenderPass
   */
  void EndRenderPass() override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::ExecuteCommandBuffers
   */
  void ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::Draw
   */
  void Draw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::DrawIndexed
   */
  void DrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t  vertexOffset,
    uint32_t firstInstance) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::DrawIndexedIndirect
   */
  void DrawIndexedIndirect(
    Graphics::Buffer& buffer,
    uint32_t          offset,
    uint32_t          drawCount,
    uint32_t          stride) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::Reset
   */
  void Reset() override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetScissor
   */
  void SetScissor(Graphics::Rect2D value) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetScissorTestEnable
   */
  void SetScissorTestEnable(bool value) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetViewport
   */
  void SetViewport(Viewport value) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetViewportEnable
   */
  void SetViewportEnable(bool value) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetColorMask
   */
  void SetColorMask(bool enabled) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::ClearStencilBuffer
   */
  void ClearStencilBuffer() override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetStencilTestEnable
   */
  void SetStencilTestEnable(bool stencilEnable) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetStencilWriteMask
   */
  void SetStencilWriteMask(uint32_t writeMask) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetStencilOp
   */
  void SetStencilOp(Graphics::StencilOp failOp,
                    Graphics::StencilOp passOp,
                    Graphics::StencilOp depthFailOp) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetStencilFunc
   */
  void SetStencilFunc(Graphics::CompareOp compareOp,
                      uint32_t            reference,
                      uint32_t            compareMask) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetDepthCompareOp
   */
  void SetDepthCompareOp(Graphics::CompareOp compareOp) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetDepthTestEnable
   */
  void SetDepthTestEnable(bool depthTestEnable) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::SetDepthWriteEnable
   */
  void SetDepthWriteEnable(bool depthWriteEnable) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::ClearDepthBuffer
   */
  void ClearDepthBuffer() override;

  /**
   * @brief Presents specified render target
   *
   * @param[in] renderTarget Valid pointer to a RenderTarget
   *
   * It's internal command that schedules presentation of
   * specified render target.
   */
  void PresentRenderTarget(GLES::RenderTarget* renderTarget);

  [[nodiscard]] const std::vector<Command>& GetCommands() const;

  /**
   * @brief Destroy the associated resources
   */
  void DestroyResource() override;

  /**
   * @brief Initialize associated resources
   */
  bool InitializeResource() override;

  /**
   * @brief Add this resource to the discard queue
   */
  void DiscardResource() override;

private:
  std::vector<Command> mCommands; ///< List of commands in this command buffer
};
} // namespace Dali::Graphics::GLES

#endif
