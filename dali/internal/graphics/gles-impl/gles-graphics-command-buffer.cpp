/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
      capacity = 0;
      dataSize = 0;
    }

    inline void resize(int newSize)
    {
      ptr      = reinterpret_cast<T*>(realloc(ptr, newSize * sizeof(T)));
      capacity = newSize;
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
};

CommandBuffer::CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, EglGraphicsController& controller)
: CommandBufferResource(createInfo, controller)
{
  mCommandPool = std::make_unique<CommandPool>(createInfo.fixedCapacity);
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::BindVertexBuffers(uint32_t                                    firstBinding,
                                      const std::vector<const Graphics::Buffer*>& buffers,
                                      const std::vector<uint32_t>&                offsets)
{
  auto command                                         = mCommandPool->AllocateCommand(CommandType::BIND_VERTEX_BUFFERS);
  command->bindVertexBuffers.vertexBufferBindingsCount = firstBinding + buffers.size();
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
  static const auto MAX_UNIFORM_BUFFER_BINDINGS = 64; // TODO: this should be read from introspection

  // TODO: could use vector?
  static thread_local UniformBufferBindingDescriptor sTempBindings[MAX_UNIFORM_BUFFER_BINDINGS];

  auto maxBindingCount = 0u;

  // find max binding and standalone UBO
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
        if(DALI_UNLIKELY(maxBindingCount == 0u))
        {
          // We can assume here comes first time. Reset temp bindings
          std::fill_n(sTempBindings, MAX_UNIFORM_BUFFER_BINDINGS, UniformBufferBindingDescriptor());
        }
        auto& slot    = sTempBindings[binding.binding];
        slot.buffer   = glesBuffer;
        slot.offset   = binding.offset;
        slot.binding  = binding.binding;
        slot.emulated = false;

        maxBindingCount = std::max(maxBindingCount, binding.binding + 1u);
      }
    }
  }
  bindCmd.uniformBufferBindings      = nullptr;
  bindCmd.uniformBufferBindingsCount = 0u;

  // copy data
  if(maxBindingCount)
  {
    auto destBindings = mCommandPool->Allocate<UniformBufferBindingDescriptor>(maxBindingCount);
    // copy
    memcpy(destBindings.Ptr(), sTempBindings, sizeof(UniformBufferBindingDescriptor) * (maxBindingCount));
    bindCmd.uniformBufferBindings      = destBindings;
    bindCmd.uniformBufferBindingsCount = maxBindingCount;
  }
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  auto command                   = mCommandPool->AllocateCommand(CommandType::BIND_PIPELINE);
  command->bindPipeline.pipeline = static_cast<const GLES::Pipeline*>(&pipeline);
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  auto  command                        = mCommandPool->AllocateCommand(CommandType::BIND_TEXTURES);
  auto& bindTexturesCmd                = command->bindTextures;
  bindTexturesCmd.textureBindings      = mCommandPool->Allocate<TextureBinding>(textureBindings.size());
  bindTexturesCmd.textureBindingsCount = textureBindings.size();
  memcpy(bindTexturesCmd.textureBindings.Ptr(), textureBindings.data(), sizeof(TextureBinding) * textureBindings.size());
}

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  auto  command                        = mCommandPool->AllocateCommand(CommandType::BIND_SAMPLERS);
  auto& bindSamplersCmd                = command->bindSamplers;
  bindSamplersCmd.samplerBindings      = mCommandPool->Allocate<SamplerBinding>(samplerBindings.size());
  bindSamplersCmd.samplerBindingsCount = samplerBindings.size();
  memcpy(bindSamplersCmd.samplerBindings.Ptr(), samplerBindings.data(), sizeof(TextureBinding) * samplerBindings.size());
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
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  auto command                      = mCommandPool->AllocateCommand(CommandType::END_RENDERPASS);
  command->endRenderPass.syncObject = static_cast<GLES::SyncObject*>(syncObject);
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
}

void CommandBuffer::Reset()
{
  mCommandPool->Rollback(false);
}

void CommandBuffer::SetScissor(Graphics::Rect2D value)
{
  auto command            = mCommandPool->AllocateCommand(CommandType::SET_SCISSOR);
  command->scissor.region = value;
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
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
  auto command               = mCommandPool->AllocateCommand(CommandType::SET_COLOR_MASK);
  command->colorMask.enabled = enabled;
}

void CommandBuffer::ClearStencilBuffer()
{
  mCommandPool->AllocateCommand(CommandType::CLEAR_STENCIL_BUFFER);
}

void CommandBuffer::SetStencilTestEnable(bool stencilEnable)
{
  auto command                 = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_TEST_ENABLE);
  command->stencilTest.enabled = stencilEnable;
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  auto command                   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_WRITE_MASK);
  command->stencilWriteMask.mask = writeMask;
}

void CommandBuffer::SetStencilOp(Graphics::StencilOp failOp,
                                 Graphics::StencilOp passOp,
                                 Graphics::StencilOp depthFailOp)
{
  auto  command   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_OP);
  auto& cmd       = command->stencilOp;
  cmd.failOp      = failOp;
  cmd.passOp      = passOp;
  cmd.depthFailOp = depthFailOp;
}

void CommandBuffer::SetStencilFunc(Graphics::CompareOp compareOp,
                                   uint32_t            reference,
                                   uint32_t            compareMask)
{
  auto  command   = mCommandPool->AllocateCommand(CommandType::SET_STENCIL_FUNC);
  auto& cmd       = command->stencilFunc;
  cmd.compareOp   = compareOp;
  cmd.compareMask = compareMask;
  cmd.reference   = reference;
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  auto command             = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_COMPARE_OP);
  command->depth.compareOp = compareOp;
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  auto command               = mCommandPool->AllocateCommand(CommandType::SET_DEPTH_TEST_ENABLE);
  command->depth.testEnabled = depthTestEnable;
}
void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
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
