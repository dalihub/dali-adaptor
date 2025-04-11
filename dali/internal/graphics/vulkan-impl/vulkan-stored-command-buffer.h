#ifndef DALI_GRAPHICS_VULKAN_STORED_COMMAND_BUFFER_H
#define DALI_GRAPHICS_VULKAN_STORED_COMMAND_BUFFER_H

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
 */

#include <dali/graphics-api/graphics-command-buffer-create-info.h>
#include <dali/graphics-api/graphics-command-buffer.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <cstdint>

namespace Dali::Graphics::Vulkan
{
class Buffer;
class RenderPass;
class RenderTarget;
class CommandBuffer;
class Pipeline;
class StoredCommandPool;
class Texture;

enum class CommandType
{
  BEGIN,
  END,
  BEGIN_RENDERPASS,
  END_RENDERPASS,
  BIND_TEXTURES,
  BIND_SAMPLERS,
  BIND_VERTEX_BUFFERS,
  BIND_INDEX_BUFFER,
  BIND_UNIFORM_BUFFER,
  BIND_PUSH_CONSTANTS,
  BIND_PIPELINE,
  DRAW,
  DRAW_INDEXED,
  DRAW_INDEXED_INDIRECT,
  DRAW_NATIVE,
  SET_SCISSOR,
  SET_SCISSOR_TEST,
  SET_VIEWPORT,
  SET_STENCIL_TEST_ENABLE,
  SET_STENCIL_WRITE_MASK,
  SET_STENCIL_STATE,
  SET_DEPTH_COMPARE_OP,
  SET_DEPTH_TEST_ENABLE,
  SET_DEPTH_WRITE_ENABLE,
  SET_COLOR_WRITE_MASK,
  SET_COLOR_BLEND_ENABLE,
  SET_COLOR_BLEND_EQUATION,
  SET_COLOR_BLEND_ADVANCED,
  NULL_COMMAND
};

/**
 * This class stores indirect pointer, used by the CommandBuffer
 * to store data allocated within resizeable pool
 */
template<class T>
struct IndirectPtr
{
  uint32_t ptr{0u};       // relative pointer
  void**   base{nullptr}; // base pointer

  inline T* operator->()
  {
    return reinterpret_cast<T*>((reinterpret_cast<uint8_t*>(*base) + ptr));
  }

  inline T& operator*()
  {
    return *reinterpret_cast<T*>((reinterpret_cast<uint8_t*>(*base) + ptr));
  }

  // Returns indirect pointer cast to requested type
  [[nodiscard]] T* Ptr() const
  {
    auto val = reinterpret_cast<T*>((reinterpret_cast<uint8_t*>(*base) + ptr));
    return val;
  }

  inline T& operator[](int index)
  {
    return reinterpret_cast<T*>((reinterpret_cast<uint8_t*>(*base) + ptr))[index];
  }

  inline const T& At(int index) const
  {
    return reinterpret_cast<const T*>((reinterpret_cast<const uint8_t*>(*base) + ptr))[index];
  }

  // Fake assignment operator for void* type
  inline IndirectPtr<T>& operator=(void* p)
  {
    ptr  = 0;
    base = nullptr;
    return *this;
  }
};

/**
 * The descriptor of Begin
 */
struct BeginDescriptor
{
  CommandBufferBeginInfo beginInfo;
};

/**
 * The descriptor of BeginRenderPass command
 */
struct BeginRenderPassDescriptor
{
  const RenderPass*       renderPass;
  const RenderTarget*     renderTarget;
  Rect2D                  renderArea;
  IndirectPtr<ClearValue> clearValues;
  uint32_t                clearValuesCount;
};

/**
 * @brief Descriptor of single buffer binding within
 * command buffer.
 */
struct VertexBufferBindingDescriptor
{
  const Buffer* buffer{nullptr};
  uint32_t      offset{0u};
};

/**
 * @brief Descriptor of ix buffer binding within
 * command buffer.
 */
struct IndexBufferBindingDescriptor
{
  const Buffer*    buffer{nullptr};
  uint32_t         offset{};
  Graphics::Format format{};
};

/**
 * @brief Descriptor of uniform buffer binding within
 * command buffer.
 */
struct UniformBufferBindingDescriptor
{
  const Buffer* buffer;
  uint32_t      binding;
  uint32_t      offset;
  uint32_t      dataSize;
};

struct PushConstantsBindingDescriptor
{
  void*    data;
  uint32_t size;
  uint32_t binding;
};

/**
 * @brief The descriptor of draw call
 */
struct DrawCallDescriptor
{
  /**
   * @brief Enum specifying type of the draw call
   */
  enum class Type
  {
    DRAW,
    DRAW_INDEXED,
    DRAW_INDEXED_INDIRECT,
  };

  Type type{}; ///< Type of the draw call

  /**
   * Union contains data for all types of draw calls.
   */
  union
  {
    /**
     * @brief Vertex array draw
     */
    struct
    {
      uint32_t vertexCount;
      uint32_t instanceCount;
      uint32_t firstVertex;
      uint32_t firstInstance;
    } draw;

    /**
     * @brief Indexed draw
     */
    struct
    {
      uint32_t indexCount;
      uint32_t instanceCount;
      uint32_t firstIndex;
      int32_t  vertexOffset;
      uint32_t firstInstance;
    } drawIndexed;

    /**
     * @brief Indexed draw indirect
     */
    struct
    {
      const Buffer* buffer;
      uint32_t      offset;
      uint32_t      drawCount;
      uint32_t      stride;
    } drawIndexedIndirect;
  };
};

/**
 * Command structure allocates memory to store a single command
 */
struct Command
{
  Command() = delete;

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
  Command(Command&& rhs)                 = delete;
  Command& operator=(Command&& rhs)      = delete;

  CommandType type{CommandType::NULL_COMMAND}; ///< Type of command

  union
  {
    BeginDescriptor begin;

    struct
    {
      IndirectPtr<Graphics::TextureBinding> textureBindings; ///< Sorted by binding index.
      uint32_t                              textureBindingsCount;
    } bindTextures{};

    // BindSampler command
    struct
    {
      IndirectPtr<Graphics::SamplerBinding> samplerBindings; ///< Sorted by binding index.
      uint32_t                              samplerBindingsCount;
    } bindSamplers;

    struct
    {
      using Binding = VertexBufferBindingDescriptor;
      IndirectPtr<Binding> vertexBufferBindings;
      uint32_t             vertexBufferBindingsCount;
      uint32_t             firstBinding;
    } bindVertexBuffers;

    IndexBufferBindingDescriptor bindIndexBuffer;

    struct
    {
      IndirectPtr<UniformBufferBindingDescriptor> uniformBufferBindings; ///< Sorted by binding index.
      uint32_t                                    uniformBufferBindingsCount;
      UniformBufferBindingDescriptor              standaloneUniformsBufferBinding{};
    } bindUniformBuffers;

    PushConstantsBindingDescriptor bindPushConstants;

    struct
    {
      const Vulkan::Pipeline* pipeline{nullptr};
    } bindPipeline;

    DrawCallDescriptor draw;

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

    BeginRenderPassDescriptor beginRenderPass;

    struct
    {
      Graphics::SyncObject* syncObject;
    } endRenderPass;

    struct
    {
      uint8_t* buffer;
    } readPixelsBuffer;

    struct
    {
      IndirectPtr<const CommandBuffer*> buffers;
      uint32_t                          buffersCount;
    } executeCommandBuffers;

    struct
    {
      RenderTarget* targetToPresent;
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

    struct
    {
      bool enabled;
      ColorBlendEquation equation;
      struct
      {
        bool srcPremultiplied;
        bool dstPremultiplied;
        Graphics::BlendOp blendOp;
      } advanced;
    } colorBlend;
  };
};

class StoredCommandBuffer : public Graphics::CommandBuffer
{
public:
  StoredCommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, uint32_t capacity);
  ~StoredCommandBuffer() override;

  [[nodiscard]] const Command* GetCommands(uint32_t& size) const;

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
   * @copydoc Dali::Graphics::CommandBuffer::ReadPixels
   */
  void ReadPixels(uint8_t* buffer) override;

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

  void SetColorBlendEnable(uint32_t attachment, bool enabled) override;
  void SetColorBlendEquation(uint32_t attachment,
                             Graphics::BlendFactor srcColorBlendFactor,
                             Graphics::BlendFactor dstColorBlendFactor,
                             Graphics::BlendOp colorBlendOp,
                             Graphics::BlendFactor srcAlphaBlendFactor,
                             Graphics::BlendFactor dstAlphaBlendFactor,
                             Graphics::BlendOp alphaBlendOp) override;

  void SetColorBlendAdvanced(uint32_t attachment, bool srcPremultiplied, bool dstPremultiplied, Graphics::BlendOp blendOp) override;

  /**
   * @copydoc Dali::Graphics::CommandBuffer::ClearDepthBuffer
   */
  void ClearDepthBuffer() override;

  /**
   * Logging method for tracking pool sizes
   * @return the capacity of the stored command pool
   */
  std::size_t GetCapacity();

  const Graphics::CommandBufferCreateInfo& GetCreateInfo() const
  {
    return mCreateInfo;
  }

private:
  const Graphics::CommandBufferCreateInfo& mCreateInfo;
  std::unique_ptr<StoredCommandPool>       mCommandPool;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_STORED_COMMAND_BUFFER_H
