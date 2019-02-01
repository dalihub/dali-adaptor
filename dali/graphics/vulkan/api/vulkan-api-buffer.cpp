/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <dali/graphics/vulkan/api/vulkan-api-buffer.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>
#include <dali/graphics/vulkan/internal/vulkan-buffer.h>
#include <dali/graphics/vulkan/api/vulkan-api-controller.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{

Buffer::Buffer( Controller& controller,
                vk::BufferUsageFlags usage,
                Dali::Graphics::Buffer::UsageHint usageHints,
                uint32_t size )
        : mController( controller ),
          mGraphics( controller.GetGraphics() ),
          mUsage( usage ),
          mUsageHints( usageHints ),
          mSize( size )
{

}

bool Buffer::Initialise()
{
  // create buffer
  mBufferRef = mGraphics.CreateBuffer( mSize, mUsage );

  // allocate memory
  // todo: host visible should be only for dynamic buffers
  auto memory = mGraphics.AllocateMemory( mBufferRef, vk::MemoryPropertyFlagBits::eHostVisible );
  mGraphics.BindBufferMemory( mBufferRef, std::move(memory), 0 );

  return true;
}

void* Buffer::Map()
{
  return mBufferRef->GetMemory()->Map();
}

void Buffer::Unmap()
{
  mBufferRef->GetMemory()->Unmap();
}

void Buffer::Flush()
{
  mBufferRef->GetMemory()->Flush();
}

void Buffer::DestroyNow()
{
  mBufferRef->DestroyNow();
}

void Buffer::Write( void* src, uint32_t srcSize, uint32_t dstOffset )
{
  // depends whether the buffer is host visible or device local
  // TODO: implement in-GPU copying, for now all buffers are host-visible
  auto transfer = std::make_unique< VulkanAPI::BufferMemoryTransfer >();
  transfer->srcPtr = new char[srcSize];
  memcpy( transfer->srcPtr, src, srcSize );
  transfer->dstBuffer = mBufferRef;
  transfer->dstOffset = dstOffset;
  transfer->srcSize = srcSize;
  mController.ScheduleBufferMemoryTransfer( std::move( transfer ) );
}

Vulkan::RefCountedBuffer Buffer::GetBufferRef() const
{
  return mBufferRef;
}

}
}
}
