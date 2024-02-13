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

// CLASS HEADER
#include "gles2-graphics-memory.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>
#include "egl-graphics-controller.h"
#include "gles-graphics-buffer.h"

namespace Dali::Graphics::GLES
{
Memory2::Memory2(const Graphics::MapBufferInfo& mapInfo, EglGraphicsController& controller)
: mController(controller)
{
  mMapBufferInfo = mapInfo;
  mMapObjectType = MapObjectType::BUFFER;
}

Memory2::Memory2(const Graphics::MapTextureInfo& mapInfo, EglGraphicsController& controller)
: mController(controller)
{
  mMapTextureInfo = mapInfo;
  mMapObjectType  = MapObjectType::TEXTURE;
}

Memory2::~Memory2()
{
  Unlock(true);
}

void* Memory2::LockRegion(uint32_t offset, uint32_t size)
{
  // allocate temporary buffer (reading back may not be supported)
  // emulated buffer is always mapped from beginning
  if(mMapObjectType == MapObjectType::BUFFER)
  {
    auto buffer = static_cast<GLES::Buffer*>(mMapBufferInfo.buffer);
    if(buffer->IsCPUAllocated())
    {
      using Ptr           = char*;
      mMappedPointer      = Ptr(buffer->GetCPUAllocatedAddress()) + offset;
      mIsAllocatedLocally = false;
    }
    else
    {
      auto retval         = malloc(size);
      mMappedPointer      = retval;
      mIsAllocatedLocally = true;
    }
  }

  return mMappedPointer;
}

void Memory2::Unlock(bool flush)
{
  if(auto gl = mController.GetGL())
  {
    // for buffer...
    if(mMapObjectType == MapObjectType::BUFFER && mMappedPointer)
    {
      auto buffer = static_cast<GLES::Buffer*>(mMapBufferInfo.buffer);
      if(!buffer->IsCPUAllocated())
      {
        buffer->Bind(BufferUsage::VERTEX_BUFFER);
        gl->BufferSubData(GL_ARRAY_BUFFER, GLintptr(mMapBufferInfo.offset), GLsizeiptr(mMapBufferInfo.size), mMappedPointer);
      }
    }

    if(mIsAllocatedLocally)
    {
      free(mMappedPointer);
      mMappedPointer = nullptr;
    }

    if(flush)
    {
      Flush();
    }
  }
}

void Memory2::Flush()
{
  // TODO:
}

} // namespace Dali::Graphics::GLES
