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
#include "gles-graphics-buffer.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
Buffer::Buffer(const Graphics::BufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: BufferResource(createInfo, controller),
  mCpuAllocated(false),
  mTransient(false)
{
  // Check if buffer is CPU allocated
  if(((0 | BufferUsage::UNIFORM_BUFFER) & mCreateInfo.usage) &&
     (0 | BufferPropertiesFlagBit::CPU_ALLOCATED) & mCreateInfo.propertiesFlags)
  {
    // cpu allocated buffer
    mCpuAllocated = true;
  }

  // Check if buffer is transient
  if((0 | BufferPropertiesFlagBit::TRANSIENT_MEMORY) & mCreateInfo.propertiesFlags)
  {
    // memory is transient, may be lazily allocated when
    // mapped, together with cpu allocated it may create emulated uniform
    // buffer in this implementation
    mTransient = true;
  }

  // Check the target enum for this buffer
  if((0 | BufferUsage::UNIFORM_BUFFER) & mCreateInfo.usage)
  {
    mBufferTarget = GL_UNIFORM_BUFFER;
  }
  else if((0 | BufferUsage::INDEX_BUFFER) & mCreateInfo.usage)
  {
    mBufferTarget = GL_ELEMENT_ARRAY_BUFFER;
  }
  // else if((0 | BufferUsage::VERTEX_BUFFER) & mCreateInfo.usage) default enum : GL_ARRAY_BUFFER

  controller.AddBuffer(*this);
}

bool Buffer::TryRecycle(const Graphics::BufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
{
  if(DALI_UNLIKELY(EglGraphicsController::IsShuttingDown()))
  {
    // Cannot recycle buffer if shutting down.
    return false;
  }

  // if different buffer spec, we need new buffer
  if(!(createInfo.size == mCreateInfo.size &&
       createInfo.allocationCallbacks == mCreateInfo.allocationCallbacks &&
       createInfo.propertiesFlags == mCreateInfo.propertiesFlags &&
       createInfo.usage == mCreateInfo.usage &&
       createInfo.nextExtension == mCreateInfo.nextExtension))
  {
    return false;
  }

  // GL resource hasn't been allocated yet, we need new buffer
  if(mBufferId == 0)
  {
    return false;
  }

  // Make sure the buffer will be reinitialized
  controller.AddBuffer(*this);

  ++mSetForGLRecyclingCount;

  return true;
}

bool Buffer::InitializeResource()
{
  // Fast-skip multiple initialize resource
  if(DALI_UNLIKELY(mSetForGLRecyclingCount > 1u))
  {
    --mSetForGLRecyclingCount;
    return true;
  }

  // CPU allocated uniform buffer is a special "compatibility" mode
  // for older GLES
  if(mCpuAllocated && !mTransient)
  {
    InitializeCPUBuffer();
  }
  else if(!mCpuAllocated)
  {
    InitializeGPUBuffer();
  }

  IncreaseBufferChangedCount();

  // make sure recycling mode is disabled after (re)initializing resource
  if(mSetForGLRecyclingCount)
  {
    --mSetForGLRecyclingCount;
  }
  return true;
}

void Buffer::InitializeCPUBuffer()
{
  // Just allocate memory
  // @TODO put better CPU memory management in place
  const auto allocators = GetCreateInfo().allocationCallbacks;

  // Early out if we recycle the buffer
  if(mBufferPtr && mSetForGLRecyclingCount)
  {
    return;
  }

  if(allocators)
  {
    mBufferPtr = allocators->allocCallback(mCreateInfo.size, 0, allocators->userData);
  }
  else
  {
    mBufferPtr = malloc(mCreateInfo.size);
    if(DALI_UNLIKELY(mBufferPtr == nullptr))
    {
      DALI_LOG_ERROR("malloc is failed. request malloc size : %u\n", mCreateInfo.size);
    }
  }
}

void Buffer::InitializeGPUBuffer()
{
  if(DALI_UNLIKELY(EglGraphicsController::IsShuttingDown()))
  {
    return;
  }

  auto* context = mController.GetCurrentContext();
  auto* gl      = mController.GetGL();
  if(DALI_UNLIKELY(!gl || !context))
  {
    return;
  }

  // If mBufferId is already set and we recycling the buffer (orphaning)
  if(!mSetForGLRecyclingCount && !mBufferId)
  {
    gl->GenBuffers(1, &mBufferId);
  }
  context->BindBuffer(mBufferTarget, mBufferId);
  gl->BufferData(mBufferTarget, GLsizeiptr(mCreateInfo.size), nullptr, GL_STATIC_DRAW);
}

void Buffer::DestroyResource()
{
  // Destroy CPU allocated buffer
  if(mCpuAllocated && mBufferPtr)
  {
    const auto allocators = GetCreateInfo().allocationCallbacks;
    if(allocators)
    {
      allocators->freeCallback(mBufferPtr, allocators->userData);
    }
    else
    {
      free(mBufferPtr);
    }
    mBufferPtr = nullptr;
  }
  // Deestroy GPU allocation
  else
  {
    if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
    {
      auto* gl = mController.GetGL();
      if(DALI_LIKELY(gl))
      {
        gl->DeleteBuffers(1, &mBufferId);
      }
    }
  }
}

void Buffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void Buffer::Bind(Graphics::BufferUsage bindingTarget) const
{
  auto* context = mController.GetCurrentContext();
  auto* gl      = mController.GetGL();
  if(DALI_UNLIKELY(!gl || !context))
  {
    return;
  }

  // CPU allocated buffer may be bound only as Uniform Buffer
  // on special binding point
  if(mCpuAllocated && mBufferPtr)
  {
    if(bindingTarget == Graphics::BufferUsage::UNIFORM_BUFFER)
    {
      // TODO: probably nothing to do, the GLES Context
      //       we may use CPU backed buffer for future data
      //       transfers (copy operations)
    }
  }
  else
  {
    switch(bindingTarget)
    {
      case Graphics::BufferUsage::VERTEX_BUFFER:
      {
        context->BindBuffer(GL_ARRAY_BUFFER, mBufferId);
        break;
      }
      case Graphics::BufferUsage::INDEX_BUFFER:
      {
        context->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
        break;
      }
      default:
      {
        // Nothing to do
      }
    }
  }
}

} // namespace Dali::Graphics::GLES
