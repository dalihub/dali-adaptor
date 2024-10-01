#ifndef DALI_GRAPHICS_GLES_COMMAND_BUFFER_H
#define DALI_GRAPHICS_GLES_COMMAND_BUFFER_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include "gles-sync-object.h"

#include <cstring>

namespace Dali::Graphics::GLES
{
class Pipeline;
class RenderPass;
class Framebuffer;
class CommandBuffer;
class CommandPool;
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
  SET_STENCIL_STATE,
  SET_DEPTH_COMPARE_OP,
  SET_DEPTH_TEST_ENABLE,
  SET_DEPTH_WRITE_ENABLE,
  DRAW_NATIVE,
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
  Command()
  {
  }

  explicit Command(CommandType commandType)
  {
    type = commandType;
  }

  ~Command() = default;

  /**
   * @brief Copy constructor
   * @param[in] rhs Command
   */
  Command(const Command& rhs)            = default;
  Command& operator=(const Command& rhs) = default;
  Command(Command&& rhs) noexcept        = delete;
  Command& operator=(Command&& rhs)      = delete;

  CommandType type{CommandType::FLUSH}; ///< Type of command

  union
  {
    struct
    {
      IndirectPtr<Graphics::TextureBinding> textureBindings;
      uint32_t                              textureBindingsCount;
    } bindTextures{};

    // BindSampler command
    struct
    {
      IndirectPtr<Graphics::SamplerBinding> samplerBindings;
      uint32_t                              samplerBindingsCount;
    } bindSamplers;

    struct
    {
      using Binding = GLES::VertexBufferBindingDescriptor;
      IndirectPtr<Binding> vertexBufferBindings;
      uint32_t             vertexBufferBindingsCount;
    } bindVertexBuffers;

    struct : public IndexBufferBindingDescriptor
    {
    } bindIndexBuffer;

    struct
    {
      IndirectPtr<UniformBufferBindingDescriptor> uniformBufferBindings;
      uint32_t                                    uniformBufferBindingsCount;
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
      Graphics::SyncObject* syncObject;
    } endRenderPass;

    struct
    {
      IndirectPtr<const GLES::CommandBuffer*> buffers;
      uint32_t                                buffersCount;
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
      uint32_t mask;
    } stencilWriteMask;

    struct
    {
      Graphics::CompareOp compareOp;
      uint32_t            compareMask;
      uint32_t            reference;

      Graphics::StencilOp failOp;
      Graphics::StencilOp passOp;
      Graphics::StencilOp depthFailOp;
    } stencilState;

    struct
    {
      bool enabled;
    } stencilTest;

    struct
    {
      bool enabled;
    } colorMask;

    struct
    {
      DrawNativeInfo drawNativeInfo;
    } drawNative;
  };
};

using CommandBufferResource = Resource<Graphics::CommandBuffer, Graphics::CommandBufferCreateInfo>;

class CommandBuffer : public CommandBufferResource
{
public:
  CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller);

  ~CommandBuffer() override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::Begin
   */
  void Begin(const Graphics::CommandBufferBeginInfo& info) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::End
   */
  void End() override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindVertexBuffers
   */
  void BindVertexBuffers(uint32_t                                    firstBinding,
                         const std::vector<const Graphics::Buffer*>& buffers,
                         const std::vector<uint32_t>&                offsets) override;

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
  void BindTextures(const std::vector<TextureBinding>& textureBindings) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::BindSamplers
   */
  void BindSamplers(const std::vector<SamplerBinding>& samplerBindings) override;

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
    Graphics::RenderPass*          renderPass,
    Graphics::RenderTarget*        renderTarget,
    Rect2D                         renderArea,
    const std::vector<ClearValue>& clearValues) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::EndRenderPass
   */
  void EndRenderPass(Graphics::SyncObject* syncObject) override;

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
   * @copydoc Dali::Graphics::CommandBuffer::DrawNative
   */
  void DrawNative(const DrawNativeInfo* drawNativeInfo) override;

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
   * @copydoc Dali::Graphics::CommandBuffer::SetStencilState
   */
  void SetStencilState(Graphics::CompareOp compareOp,
                       uint32_t            reference,
                       uint32_t            compareMask,
                       Graphics::StencilOp failOp,
                       Graphics::StencilOp passOp,
                       Graphics::StencilOp depthFailOp) override;

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

  /**
   * @brief Returns pointer to the list of command and size of the list
   *
   * @param[out] size Size of the list
   * @return Valid pointer to the list of commands
   */
  [[nodiscard]] const Command* GetCommands(uint32_t& size) const;

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

  // Get the total memory usage of this command buffer
  std::size_t GetCapacity();

private:
  std::unique_ptr<CommandPool> mCommandPool; ///< Pool of commands and transient memory
};
} // namespace Dali::Graphics::GLES

#endif
