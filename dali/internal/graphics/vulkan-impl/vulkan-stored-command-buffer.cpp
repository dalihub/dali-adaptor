/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
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
// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gLogCmdBufferFilter;
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
const uint32_t EXCESS_BUFFER_COUNT = 4;

template<typename VT, typename GT>
VT* ConstGraphicsCast(const GT* object)
{
  return const_cast<VT*>(static_cast<const VT*>(object));
}

class StoredCommandPool
{
  static constexpr uint32_t COMMAND_POOL_DEFAULT_INCREMENT = 1024 * 32 / sizeof(Command); // 32kb banks
  static const uint32_t     MEMORY_POOL_DEFAULT_INCREMENT  = 1024;                        // 1kb memory pool increment
  static const uint32_t     MEMORY_POOL_DEFAULT_ALIGNMENT  = 64;                          // 64bytes alignment

  template<class T>
  struct Block
  {
    Block() = default;
    ~Block()
    {
      if(ptr)
      {
        free(ptr);
      }
    }

    T*        ptr{nullptr};
    uint32_t  dataSize{0u};
    uint32_t  capacity{0u};
    inline T& operator[](int index) const
    {
      return ptr[index];
    }

    inline void clear()
    {
      free(ptr);
      ptr      = nullptr;
      capacity = 0;
      dataSize = 0;
    }

    inline void resize(int newSize)
    {
      ptr      = reinterpret_cast<T*>(realloc(ptr, newSize * sizeof(T)));
      capacity = newSize * sizeof(T);
      dataSize = newSize;
    }

    inline T* data() const
    {
      return ptr;
    }

    inline uint32_t size() const
    {
      return dataSize;
    }
  };
  // This memory pool guarantees all items will be placed
  // in the continuous memory area but returned pointers are relative
  // and require translation before using
  template<class T, int Increment, int Alignment = 0>
  struct MemoryPool
  {
    Block<T>  data;
    inline T& operator[](int index)
    {
      return data[index];
    }
    MemoryPool() = default;

    IndirectPtr<T> Allocate(uint32_t count)
    {
      // Set fixed capacity
      if(fixedCapacity)
      {
        // resize data size when capacity is not setuped.
        // Note if totalCapacity is bigger than fixedCapacity,
        // just skip here and resize dynamically
        if(DALI_UNLIKELY(totalCapacity < fixedCapacity))
        {
          data.resize(fixedCapacity);
          totalCapacity = data.size();
        }
      }

      // Resize dynamically
      if(DALI_UNLIKELY(totalCapacity < offset + count))
      {
        // Resize the memory size as ceil((offset + count - totalCapacity)) / Increment) * Increment
        // So the incremented size of data is always multiplied of the value Increment.
        data.resize(data.size() + ((offset + count - totalCapacity - 1) / Increment + 1) * Increment);

        // update base pointer, required for address translation
        totalCapacity = data.size();
      }

      basePtr = data.data();

      IndirectPtr<T> retval{uint32_t(uintptr_t(&data[offset]) - uintptr_t(basePtr)), &basePtr};
      size += count;
      offset += count;

      // align offset if needed (only if type size is 1)
      if(Alignment && sizeof(T) == 1)
      {
        offset = ((offset / Alignment) * Alignment) + ((offset % Alignment) ? Alignment : 0);
      }
      return retval;
    }

    // Rolls back pool
    void Rollback()
    {
      offset = 0;
      size   = 0;
    }

    // Discards all data and storage
    void Clear()
    {
      data.clear();
      totalCapacity = 0;
      offset        = 0;
      size          = 0;
    }

    uint32_t offset{0u};
    uint32_t totalCapacity{0u};
    uint32_t size{0u};
    uint32_t increment{Increment};
    uint32_t alignment{Alignment};
    uint32_t fixedCapacity{0u};
    void*    basePtr{nullptr};
  };

  MemoryPool<uint8_t, MEMORY_POOL_DEFAULT_INCREMENT, MEMORY_POOL_DEFAULT_ALIGNMENT>  memoryPool;
  MemoryPool<Command, COMMAND_POOL_DEFAULT_INCREMENT, MEMORY_POOL_DEFAULT_ALIGNMENT> commandPool;

public:
  StoredCommandPool() = default;
  StoredCommandPool(uint32_t fixedCapacity)
  {
    commandPool.fixedCapacity = fixedCapacity;
    memoryPool.fixedCapacity  = fixedCapacity * 1024;
  }

  /**
   * Return value may become invalid if pool is resized (by allocating another command)
   * @param type
   * @return
   */
  Command* AllocateCommand(CommandType type)
  {
    auto command  = commandPool.Allocate(1);
    command->type = type;
    auto* cmd     = command.Ptr();
    return cmd;
  }

  template<class T>
  IndirectPtr<T> Allocate(uint32_t count)
  {
    const auto typeSize       = sizeof(T);
    const auto memoryRequired = typeSize * count;
    auto       ptr            = memoryPool.Allocate(memoryRequired);

    // Convert generic pointer and return
    return IndirectPtr<T>{ptr.ptr, ptr.base};
  }

  // New (should not be needed)
  template<class T>
  T* New(uint32_t count)
  {
    auto ptr = Allocate<T>(count);
    for(auto i = 0u; i < count; ++i)
    {
      new(&ptr[i]) T();
    }
    return ptr;
  }

  // TODO: explicit delete?
  void Rollback(bool discard)
  {
    if(discard)
    {
      commandPool.Clear();
      memoryPool.Clear();
    }
    else
    {
      commandPool.Rollback();
      memoryPool.Rollback();
    }
  }

  const Command* GetCommands(uint32_t& size) const
  {
    size = commandPool.size;
    return commandPool.data.ptr;
  }

  std::size_t GetTotalCapacity() const
  {
    return commandPool.data.capacity + memoryPool.data.capacity;
  }
};

StoredCommandBuffer::StoredCommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, uint32_t fixedCapacity)
: mCreateInfo(createInfo)
{
  mCommandPool = std::make_unique<StoredCommandPool>(fixedCapacity);
}

StoredCommandBuffer::~StoredCommandBuffer() = default;

void StoredCommandBuffer::Begin(const Graphics::CommandBufferBeginInfo& info)
{
  auto command             = mCommandPool->AllocateCommand(CommandType::BEGIN);
  command->begin.beginInfo = info;
}

void StoredCommandBuffer::End()
{
  mCommandPool->AllocateCommand(CommandType::END);
}

void StoredCommandBuffer::BindVertexBuffers(
  uint32_t                                    firstBinding,
  const std::vector<const Graphics::Buffer*>& buffers,
  const std::vector<uint32_t>&                offsets)
{
  auto command                                         = mCommandPool->AllocateCommand(CommandType::BIND_VERTEX_BUFFERS);
  command->bindVertexBuffers.firstBinding              = firstBinding;
  command->bindVertexBuffers.vertexBufferBindingsCount = firstBinding + static_cast<uint32_t>(buffers.size());

  auto pBindings = mCommandPool->Allocate<Vulkan::VertexBufferBindingDescriptor>(firstBinding + buffers.size());

  command->bindVertexBuffers.vertexBufferBindings = pBindings;

  auto index = firstBinding;
  for(auto& buf : buffers)
  {
    pBindings[index].buffer = static_cast<const Vulkan::Buffer*>(buf);
    pBindings[index].offset = offsets[index - firstBinding];
    index++;
  }
}

void StoredCommandBuffer::BindUniformBuffers(const std::vector<Graphics::UniformBufferBinding>& bindings)
{
  auto  command = mCommandPool->AllocateCommand(CommandType::BIND_UNIFORM_BUFFER);
  auto& cmd     = *command;
  auto& bindCmd = cmd.bindUniformBuffers;

  if(DALI_LIKELY(!bindings.empty()))
  {
    auto bindingCount                  = bindings.size();
    auto destBindings                  = mCommandPool->Allocate<UniformBufferBindingDescriptor>(bindingCount);
    bindCmd.uniformBufferBindings      = destBindings;
    bindCmd.uniformBufferBindingsCount = bindingCount;

    int bindingIndex = 0;
    for(const auto& binding : bindings)
    {
      auto& slot    = destBindings[bindingIndex++];
      slot.buffer   = static_cast<Vulkan::Buffer*>(binding.buffer);
      slot.binding  = binding.binding;
      slot.offset   = binding.offset;
      slot.dataSize = binding.dataSize;
    }
  }
  else
  {
    bindCmd.uniformBufferBindings      = nullptr;
    bindCmd.uniformBufferBindingsCount = 0u;
  }
}

void StoredCommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  auto command = mCommandPool->AllocateCommand(CommandType::BIND_PIPELINE);

  command->bindPipeline.pipeline = static_cast<const Vulkan::Pipeline*>(&pipeline);
}

void StoredCommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  auto  command         = mCommandPool->AllocateCommand(CommandType::BIND_TEXTURES);
  auto& bindTexturesCmd = command->bindTextures;

  if(textureBindings.empty())
  {
    bindTexturesCmd.textureBindings      = nullptr;
    bindTexturesCmd.textureBindingsCount = 0u;
  }
  else
  {
    const uint32_t bindingCount = static_cast<uint32_t>(textureBindings.size());

    auto destBindings = mCommandPool->Allocate<TextureBinding>(bindingCount);

    // copy
    memcpy(destBindings.Ptr(), textureBindings.data(), sizeof(TextureBinding) * (bindingCount));

    bindTexturesCmd.textureBindings      = destBindings;
    bindTexturesCmd.textureBindingsCount = bindingCount;

    // Check binding is continuous, and throw exception for debug mode
#if defined(DEBUG_ENABLED)
    uint32_t lastBinding = 0u;
    for(const auto& binding : textureBindings)
    {
      DALI_ASSERT_DEBUG(lastBinding == binding.binding && "Texture binding order not matched!");
      ++lastBinding;
    }
#endif
  }
}

void StoredCommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  // Unused in core
}

void StoredCommandBuffer::BindPushConstants(
  void*    data,
  uint32_t size,
  uint32_t binding)
{
  auto command                       = mCommandPool->AllocateCommand(CommandType::BIND_PUSH_CONSTANTS);
  command->bindPushConstants.data    = data;
  command->bindPushConstants.size    = size;
  command->bindPushConstants.binding = binding;
}

void StoredCommandBuffer::BindIndexBuffer(
  const Graphics::Buffer& buffer,
  uint32_t                offset,
  Format                  format)
{
  auto command = mCommandPool->AllocateCommand(CommandType::BIND_INDEX_BUFFER);

  command->bindIndexBuffer.buffer = static_cast<const Vulkan::Buffer*>(&buffer);
  command->bindIndexBuffer.offset = offset;
  command->bindIndexBuffer.format = format;
}

void StoredCommandBuffer::BeginRenderPass(
  Graphics::RenderPass*          renderPass,
  Graphics::RenderTarget*        renderTarget,
  Rect2D                         renderArea,
  const std::vector<ClearValue>& clearValues)
{
  auto  command = mCommandPool->AllocateCommand(CommandType::BEGIN_RENDERPASS);
  auto& cmd     = *command;

  cmd.beginRenderPass.renderPass   = static_cast<Vulkan::RenderPass*>(renderPass);
  cmd.beginRenderPass.renderTarget = static_cast<Vulkan::RenderTarget*>(renderTarget);
  cmd.beginRenderPass.renderArea   = renderArea;

  cmd.beginRenderPass.clearValues = mCommandPool->Allocate<ClearValue>(clearValues.size());
  memcpy(cmd.beginRenderPass.clearValues.Ptr(), clearValues.data(), sizeof(ClearValue) * clearValues.size());
  cmd.beginRenderPass.clearValuesCount = clearValues.size();
}

void StoredCommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  auto command = mCommandPool->AllocateCommand(CommandType::END_RENDERPASS);

  command->endRenderPass.syncObject = nullptr;
}

void StoredCommandBuffer::ReadPixels(uint8_t* buffer)
{
}

void StoredCommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers)
{
  DALI_LOG_ERROR("Secondary command buffers no longer supported\n");
}

void StoredCommandBuffer::Draw(
  uint32_t vertexCount,
  uint32_t instanceCount,
  uint32_t firstVertex,
  uint32_t firstInstance)
{
  auto  command          = mCommandPool->AllocateCommand(CommandType::DRAW);
  auto& cmd              = command->draw;
  cmd.type               = DrawCallDescriptor::Type::DRAW;
  cmd.draw.vertexCount   = vertexCount;
  cmd.draw.instanceCount = instanceCount;
  cmd.draw.firstInstance = firstInstance;
  cmd.draw.firstVertex   = firstVertex;
}

void StoredCommandBuffer::DrawIndexed(
  uint32_t indexCount,
  uint32_t instanceCount,
  uint32_t firstIndex,
  int32_t  vertexOffset,
  uint32_t firstInstance)
{
  auto  command                 = mCommandPool->AllocateCommand(CommandType::DRAW_INDEXED);
  auto& cmd                     = command->draw;
  cmd.type                      = DrawCallDescriptor::Type::DRAW_INDEXED;
  cmd.drawIndexed.firstIndex    = firstIndex;
  cmd.drawIndexed.firstInstance = firstInstance;
  cmd.drawIndexed.indexCount    = indexCount;
  cmd.drawIndexed.vertexOffset  = vertexOffset;
  cmd.drawIndexed.instanceCount = instanceCount;
}

void StoredCommandBuffer::DrawIndexedIndirect(
  Graphics::Buffer& buffer,
  uint32_t          offset,
  uint32_t          drawCount,
  uint32_t          stride)
{
  auto  command                     = mCommandPool->AllocateCommand(CommandType::DRAW_INDEXED_INDIRECT);
  auto& cmd                         = command->draw;
  cmd.type                          = DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT;
  cmd.drawIndexedIndirect.buffer    = static_cast<const Vulkan::Buffer*>(&buffer);
  cmd.drawIndexedIndirect.offset    = offset;
  cmd.drawIndexedIndirect.drawCount = drawCount;
  cmd.drawIndexedIndirect.stride    = stride;
}

void StoredCommandBuffer::DrawNative(const DrawNativeInfo* drawNativeInfo)
{
  auto  command = mCommandPool->AllocateCommand(CommandType::DRAW_NATIVE);
  auto& cmd     = command->drawNative;
  memcpy(&cmd.drawNativeInfo, drawNativeInfo, sizeof(DrawNativeInfo));
}

void StoredCommandBuffer::Reset()
{
  mCommandPool->Rollback(false);
}

void StoredCommandBuffer::SetScissor(Graphics::Rect2D value)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_SCISSOR);

  command->scissor.region = value;
}

void StoredCommandBuffer::SetScissorTestEnable(bool value)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_SCISSOR_TEST);

  command->scissorTest.enable = value;
}

void StoredCommandBuffer::SetViewport(Viewport value)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_VIEWPORT);

  command->viewport.region = value;
}

void StoredCommandBuffer::SetViewportEnable(bool value)
{
}

void StoredCommandBuffer::SetColorMask(bool enabled)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_COLOR_WRITE_MASK);

  command->colorMask.enabled = enabled;
}

void StoredCommandBuffer::ClearStencilBuffer()
{
}

void StoredCommandBuffer::SetStencilTestEnable(bool stencilEnable)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_TEST_ENABLE);

  command->stencilTest.enabled = stencilEnable;
}

void StoredCommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  auto command                   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_WRITE_MASK);
  command->stencilWriteMask.mask = writeMask;
}

void StoredCommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                          uint32_t            reference,
                                          uint32_t            compareMask,
                                          Graphics::StencilOp failOp,
                                          Graphics::StencilOp passOp,
                                          Graphics::StencilOp depthFailOp)
{
  auto  command   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_STATE);
  auto& cmd       = command->stencilState;
  cmd.failOp      = failOp;
  cmd.passOp      = passOp;
  cmd.depthFailOp = depthFailOp;
  cmd.compareOp   = compareOp;
  cmd.compareMask = compareMask;
  cmd.reference   = reference;
}

void StoredCommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_COMPARE_OP);

  command->depth.compareOp = compareOp;
}

void StoredCommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_TEST_ENABLE);

  command->depth.testEnabled = depthTestEnable;
}

void StoredCommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  auto command                = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_WRITE_ENABLE);
  command->depth.writeEnabled = depthWriteEnable;
}

void StoredCommandBuffer::SetColorBlendEnable(uint32_t attachment, bool enabled)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_COLOR_BLEND_ENABLE);

  command->colorBlend.enabled = enabled;
}

void StoredCommandBuffer::SetColorBlendEquation(uint32_t attachment,
  Graphics::BlendFactor srcColorBlendFactor,
  Graphics::BlendFactor dstColorBlendFactor,
  Graphics::BlendOp colorBlendOp,
  Graphics::BlendFactor srcAlphaBlendFactor,
  Graphics::BlendFactor dstAlphaBlendFactor,
  Graphics::BlendOp alphaBlendOp)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_COLOR_BLEND_EQUATION);

  ColorBlendEquation equation{
    srcColorBlendFactor,
    dstColorBlendFactor,
    colorBlendOp,
    srcAlphaBlendFactor,
    dstAlphaBlendFactor,
    alphaBlendOp
  };
  command->colorBlend.equation = equation;
}

void StoredCommandBuffer::SetColorBlendAdvanced(uint32_t attachment, bool srcPremultiplied, bool dstPremultiplied, Graphics::BlendOp blendOp)
{
  auto command = mCommandPool->AllocateCommand(CommandType::SET_COLOR_BLEND_ADVANCED);
  auto& advanced = command->colorBlend.advanced;
  advanced.srcPremultiplied = srcPremultiplied;
  advanced.dstPremultiplied = dstPremultiplied;
  advanced.blendOp = blendOp;
}

void StoredCommandBuffer::ClearDepthBuffer()
{
}

[[nodiscard]] const Command* StoredCommandBuffer::GetCommands(uint32_t& size) const
{
  return mCommandPool->GetCommands(size);
}

std::size_t StoredCommandBuffer::GetCapacity()
{
  std::size_t total{0u};
  if(mCommandPool)
  {
    total = mCommandPool->GetTotalCapacity();
  }
  return total;
}

} // namespace Dali::Graphics::Vulkan
