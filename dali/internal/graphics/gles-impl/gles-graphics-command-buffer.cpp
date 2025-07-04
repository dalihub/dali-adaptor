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
class CommandPool
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
  CommandPool() = default;
  CommandPool(uint32_t fixedCapacity)
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

/**
 * @brief Caches of GL state relative commands.
 * It will be used to reduce duplicated GL state commands
 */
struct CommandBuffer::GlStateCommandCache
{
  GlStateCommandCache()
  {
    ResetCache();
  }

  ~GlStateCommandCache() = default;

  void ResetCache()
  {
    mCachedFlags = 0u;
  }

  enum StateFlags
  {
    // boolean flags
    SET_SCISSOR_TEST_ENABLE,
    SET_COLOR_MASK,
    SET_STENCIL_TEST_ENABLE,
    SET_DEPTH_TEST_ENABLE,
    SET_DEPTH_WRITE_ENABLE,

    // uint32_t flags
    SET_STENCIL_WRITE_MASK,
    SET_STENCIL_STATE_COMPARE_OP,
    SET_STENCIL_STATE_REFERENCE,
    SET_STENCIL_STATE_COMPARE_MASK,
    SET_STENCIL_STATE_FAIL_OP,
    SET_STENCIL_STATE_PASS_OP,
    SET_STENCIL_STATE_DEPTH_FAIL_OP,
    SET_DEPTH_COMPARE_OP,

    MAX_STATE_FLAGS,
  };

  static_assert(StateFlags::MAX_STATE_FLAGS <= sizeof(uint32_t) * 8);

  /**
   * @brief Checks if the input value matched with the flag is equal to the cached value.
   * If the flag is not set in the cache, or the value is not equal to the cached value, return false.
   * After the check, the value is cached.
   */
  [[nodiscard]] inline bool CheckValueEqualsAndCache(StateFlags flag, uint32_t value)
  {
    if((mCachedFlags & (1u << flag)) == 0u || mCachedValues[flag] != value)
    {
      mCachedFlags |= (1u << flag);
      mCachedValues[flag] = value;
      return false; // not equal
    }
    return true; // equal
  }

private:
  uint32_t mCachedFlags{0u};
  uint32_t mCachedValues[MAX_STATE_FLAGS]{0u};
};

CommandBuffer::CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller)
: CommandBufferResource(createInfo, controller)
{
  mCommandPool         = std::make_unique<CommandPool>(createInfo.fixedCapacity);
  mGlStateCommandCache = std::make_unique<GlStateCommandCache>();
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::Begin(const Graphics::CommandBufferBeginInfo& info)
{
  mGlStateCommandCache->ResetCache();
}

void CommandBuffer::End()
{
  mGlStateCommandCache->ResetCache();
}

void CommandBuffer::BindVertexBuffers(uint32_t                                    firstBinding,
                                      const std::vector<const Graphics::Buffer*>& buffers,
                                      const std::vector<uint32_t>&                offsets)
{
  auto command                                         = mCommandPool->AllocateCommand(CommandType::BIND_VERTEX_BUFFERS);
  command->bindVertexBuffers.vertexBufferBindingsCount = firstBinding + static_cast<uint32_t>(buffers.size());
  auto pBindings                                       = mCommandPool->Allocate<GLES::VertexBufferBindingDescriptor>(firstBinding + buffers.size());

  command->bindVertexBuffers.vertexBufferBindings = pBindings;
  auto index                                      = firstBinding;
  for(auto& buf : buffers)
  {
    pBindings[index].buffer = static_cast<const GLES::Buffer*>(buf);
    pBindings[index].offset = offsets[index - firstBinding];
    index++;
  }
}

void CommandBuffer::BindUniformBuffers(const std::vector<Graphics::UniformBufferBinding>& bindings)
{
  auto command = mCommandPool->AllocateCommand(CommandType::BIND_UNIFORM_BUFFER);

  auto& cmd     = *command;
  auto& bindCmd = cmd.bindUniformBuffers;

  // temporary static set of binding slots (thread local)
  static constexpr auto MAX_UNIFORM_BUFFER_BINDINGS = 64; // TODO: this should be read from introspection

  static constexpr UniformBufferBindingDescriptor                 NULL_DESCRIPTOR{nullptr, 0, 0, 0};
  static thread_local std::vector<UniformBufferBindingDescriptor> sTempBindings(MAX_UNIFORM_BUFFER_BINDINGS, NULL_DESCRIPTOR);
  static thread_local std::vector<uint8_t>                        sTempBindingsUsed(MAX_UNIFORM_BUFFER_BINDINGS, 0);

  memset(&bindCmd.standaloneUniformsBufferBinding, 0, sizeof(UniformBufferBindingDescriptor));

  // find max binding and standalone UBO
  auto maxBinding  = 0u;
  bool hasBindings = false;
  for(const auto& binding : bindings)
  {
    if(binding.buffer)
    {
      const auto* glesBuffer = static_cast<const GLES::Buffer*>(binding.buffer);
      if(glesBuffer->IsCPUAllocated()) // standalone uniforms
      {
        bindCmd.standaloneUniformsBufferBinding.buffer  = glesBuffer;
        bindCmd.standaloneUniformsBufferBinding.offset  = binding.offset;
        bindCmd.standaloneUniformsBufferBinding.binding = binding.binding;
      }
      else // Bind regular UBO
      {
        auto& slot = sTempBindings[binding.binding];

        slot.buffer   = glesBuffer;
        slot.offset   = binding.offset;
        slot.dataSize = binding.dataSize;
        slot.binding  = binding.binding;

        sTempBindingsUsed[binding.binding] = true;

        maxBinding  = std::max(maxBinding, binding.binding);
        hasBindings = true;
      }
    }
  }

  // reset unused bindings slots
  for(auto i = 0u; i <= maxBinding; ++i)
  {
    if(sTempBindingsUsed[i])
    {
      sTempBindingsUsed[i] = false;
    }
    else
    {
      sTempBindings[i] = NULL_DESCRIPTOR;
    }
  }

  bindCmd.uniformBufferBindings      = nullptr;
  bindCmd.uniformBufferBindingsCount = 0u;

  // copy data
  if(hasBindings)
  {
    ++maxBinding;

    auto destBindings = mCommandPool->Allocate<UniformBufferBindingDescriptor>(maxBinding);

    // copy
    memcpy(destBindings.Ptr(), &sTempBindings[0], sizeof(UniformBufferBindingDescriptor) * (maxBinding));
    bindCmd.uniformBufferBindings      = destBindings;
    bindCmd.uniformBufferBindingsCount = maxBinding;
  }
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  auto command                   = mCommandPool->AllocateCommand(CommandType::BIND_PIPELINE);
  command->bindPipeline.pipeline = static_cast<const GLES::Pipeline*>(&pipeline);
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
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

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  // Unused in core
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
  auto command                    = mCommandPool->AllocateCommand(CommandType::BIND_INDEX_BUFFER);
  command->bindIndexBuffer.buffer = static_cast<const GLES::Buffer*>(&buffer);
  command->bindIndexBuffer.offset = offset;
  command->bindIndexBuffer.format = format;
}

void CommandBuffer::BeginRenderPass(
  Graphics::RenderPass*          renderPass,
  Graphics::RenderTarget*        renderTarget,
  Rect2D                         renderArea,
  const std::vector<ClearValue>& clearValues)
{
  auto  command                    = mCommandPool->AllocateCommand(CommandType::BEGIN_RENDERPASS);
  auto& cmd                        = *command;
  cmd.beginRenderPass.renderPass   = static_cast<GLES::RenderPass*>(renderPass);
  cmd.beginRenderPass.renderTarget = static_cast<GLES::RenderTarget*>(renderTarget);
  cmd.beginRenderPass.renderArea   = renderArea;

  cmd.beginRenderPass.clearValues = mCommandPool->Allocate<ClearValue>(clearValues.size());
  memcpy(cmd.beginRenderPass.clearValues.Ptr(), clearValues.data(), sizeof(ClearValue) * clearValues.size());
  cmd.beginRenderPass.clearValuesCount = clearValues.size();

  mGlStateCommandCache->ResetCache(); // Reset GL state cache after begin render pass
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  auto command                      = mCommandPool->AllocateCommand(CommandType::END_RENDERPASS);
  command->endRenderPass.syncObject = static_cast<GLES::SyncObject*>(syncObject);

  mGlStateCommandCache->ResetCache(); // Reset GL state cache after end render pass
}

void CommandBuffer::ReadPixels(uint8_t* buffer)
{
  auto command                     = mCommandPool->AllocateCommand(CommandType::READ_PIXELS);
  command->readPixelsBuffer.buffer = buffer;
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers)
{
  auto  command    = mCommandPool->AllocateCommand(CommandType::EXECUTE_COMMAND_BUFFERS);
  auto& cmd        = command->executeCommandBuffers;
  cmd.buffers      = mCommandPool->Allocate<const GLES::CommandBuffer*>(commandBuffers.size());
  cmd.buffersCount = commandBuffers.size();
  for(auto i = 0u; i < cmd.buffersCount; ++i)
  {
    cmd.buffers[i] = static_cast<const GLES::CommandBuffer*>(commandBuffers[i]);
  }
}

void CommandBuffer::Draw(
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

void CommandBuffer::DrawIndexed(
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

void CommandBuffer::DrawIndexedIndirect(
  Graphics::Buffer& buffer,
  uint32_t          offset,
  uint32_t          drawCount,
  uint32_t          stride)
{
  auto  command                     = mCommandPool->AllocateCommand(CommandType::DRAW_INDEXED_INDIRECT);
  auto& cmd                         = command->draw;
  cmd.type                          = DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT;
  cmd.drawIndexedIndirect.buffer    = static_cast<const GLES::Buffer*>(&buffer);
  cmd.drawIndexedIndirect.offset    = offset;
  cmd.drawIndexedIndirect.drawCount = drawCount;
  cmd.drawIndexedIndirect.stride    = stride;
}

void CommandBuffer::DrawNative(const DrawNativeInfo* drawNativeInfo)
{
  auto  command = mCommandPool->AllocateCommand(CommandType::DRAW_NATIVE);
  auto& cmd     = command->drawNative;
  memcpy(&cmd.drawNativeInfo, drawNativeInfo, sizeof(DrawNativeInfo));
  mGlStateCommandCache->ResetCache(); // Reset GL state cache after draw native
}

void CommandBuffer::Reset()
{
  mCommandPool->Rollback(false);
  mGlStateCommandCache->ResetCache(); // Reset GL state cache
}

void CommandBuffer::SetScissor(Graphics::Rect2D value)
{
  auto command            = mCommandPool->AllocateCommand(CommandType::SET_SCISSOR);
  command->scissor.region = value;
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_SCISSOR_TEST_ENABLE, value))
  {
    return;
  }

  auto command                = mCommandPool->AllocateCommand(CommandType::SET_SCISSOR_TEST);
  command->scissorTest.enable = value;
}

void CommandBuffer::SetViewport(Viewport value)
{
  auto command             = mCommandPool->AllocateCommand(CommandType::SET_VIEWPORT);
  command->viewport.region = value;
}

void CommandBuffer::SetViewportEnable(bool value)
{
  // There is no GL equivalent
}

void CommandBuffer::SetColorMask(bool enabled)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_COLOR_MASK, enabled))
  {
    return;
  }

  auto command               = mCommandPool->AllocateCommand(CommandType::SET_COLOR_MASK);
  command->colorMask.enabled = enabled;
}

void CommandBuffer::ClearStencilBuffer()
{
  mCommandPool->AllocateCommand(CommandType::CLEAR_STENCIL_BUFFER);
}

void CommandBuffer::SetStencilTestEnable(bool stencilEnable)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_TEST_ENABLE, stencilEnable))
  {
    return;
  }

  auto command                 = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_TEST_ENABLE);
  command->stencilTest.enabled = stencilEnable;
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_WRITE_MASK, writeMask))
  {
    return;
  }

  auto command                   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_WRITE_MASK);
  command->stencilWriteMask.mask = writeMask;
}

void CommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                    uint32_t            reference,
                                    uint32_t            compareMask,
                                    Graphics::StencilOp failOp,
                                    Graphics::StencilOp passOp,
                                    Graphics::StencilOp depthFailOp)
{
  // Need to call CheckValueEqualsAndCache for each values
  bool isValueEqual = mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_STATE_COMPARE_OP, (uint32_t)compareOp);
  isValueEqual      = mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_STATE_REFERENCE, reference) && isValueEqual;
  isValueEqual      = mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_STATE_COMPARE_MASK, compareMask) && isValueEqual;
  isValueEqual      = mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_STATE_FAIL_OP, (uint32_t)failOp) && isValueEqual;
  isValueEqual      = mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_STATE_PASS_OP, (uint32_t)passOp) && isValueEqual;
  isValueEqual      = mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_STENCIL_STATE_DEPTH_FAIL_OP, (uint32_t)depthFailOp) && isValueEqual;
  if(isValueEqual)
  {
    return;
  }

  auto  command   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_STATE);
  auto& cmd       = command->stencilState;
  cmd.failOp      = failOp;
  cmd.passOp      = passOp;
  cmd.depthFailOp = depthFailOp;
  cmd.compareOp   = compareOp;
  cmd.compareMask = compareMask;
  cmd.reference   = reference;
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_DEPTH_COMPARE_OP, (uint32_t)compareOp))
  {
    return;
  }

  auto command             = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_COMPARE_OP);
  command->depth.compareOp = compareOp;
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_DEPTH_TEST_ENABLE, depthTestEnable))
  {
    return;
  }

  auto command               = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_TEST_ENABLE);
  command->depth.testEnabled = depthTestEnable;
}
void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  if(mGlStateCommandCache->CheckValueEqualsAndCache(CommandBuffer::GlStateCommandCache::SET_DEPTH_WRITE_ENABLE, depthWriteEnable))
  {
    return;
  }

  auto command                = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_WRITE_ENABLE);
  command->depth.writeEnabled = depthWriteEnable;
}
void CommandBuffer::ClearDepthBuffer()
{
  mCommandPool->AllocateCommand(CommandType::CLEAR_DEPTH_BUFFER);
}

void CommandBuffer::PresentRenderTarget(GLES::RenderTarget* renderTarget)
{
  auto command                                 = mCommandPool->AllocateCommand(CommandType::PRESENT_RENDER_TARGET);
  command->presentRenderTarget.targetToPresent = renderTarget;
}

[[nodiscard]] const Command* CommandBuffer::GetCommands(uint32_t& size) const
{
  return mCommandPool->GetCommands(size);
}

void CommandBuffer::DestroyResource()
{
  if(DALI_LIKELY(mCommandPool))
  {
    mCommandPool->Rollback(true); // Discard memory here!
  }
  mCommandPool.reset();
  mGlStateCommandCache.reset();
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

std::size_t CommandBuffer::GetCapacity()
{
  std::size_t total{0u};
  if(mCommandPool)
  {
    total = mCommandPool->GetTotalCapacity();
  }
  return total;
}

} // namespace Dali::Graphics::GLES
