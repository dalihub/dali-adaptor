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
#include "gles3-graphics-memory.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
Memory3::Memory3(const Graphics::MapBufferInfo& mapInfo, EglGraphicsController& controller)
: mController(controller)
{
  mMapBufferInfo = mapInfo;
  mMapObjectType = MapObjectType::BUFFER;
}

Memory3::Memory3(const Graphics::MapTextureInfo& mapInfo, EglGraphicsController& controller)
: mController(controller)
{
  mMapTextureInfo = mapInfo;
  mMapObjectType  = MapObjectType::TEXTURE;
}

Memory3::~Memory3()
{
  Unlock(true);
}

void* Memory3::LockRegion(uint32_t offset, uint32_t size)
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    auto* gl = mController.GetGL();
    if(DALI_LIKELY(gl))
    {
      if(mMapObjectType == MapObjectType::BUFFER)
      {
        auto buffer = static_cast<GLES::Buffer*>(mMapBufferInfo.buffer);

        if(buffer->IsCPUAllocated())
        {
          using Ptr      = char*;
          mMappedPointer = Ptr(buffer->GetCPUAllocatedAddress()) + offset;
        }
        else
        {
          gl->BindBuffer(GL_COPY_WRITE_BUFFER, buffer->GetGLBuffer());
          void* ptr      = nullptr;
          ptr            = gl->MapBufferRange(GL_COPY_WRITE_BUFFER, GLintptr(mMapBufferInfo.offset), GLsizeiptr(mMapBufferInfo.size), GL_MAP_WRITE_BIT);
          mMappedPointer = ptr;
        }
        return mMappedPointer;
      }
    }
  }
  return nullptr;
}

void Memory3::Unlock(bool flush)
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    auto* gl = mController.GetGL();
    if(DALI_LIKELY(gl))
    {
      if(mMapObjectType == MapObjectType::BUFFER && mMappedPointer)
      {
        auto buffer = static_cast<GLES::Buffer*>(mMapBufferInfo.buffer);
        if(!buffer->IsCPUAllocated())
        {
          gl->BindBuffer(GL_COPY_WRITE_BUFFER, buffer->GetGLBuffer());
          gl->UnmapBuffer(GL_COPY_WRITE_BUFFER);
        }
      }

      if(flush)
      {
        Flush();
      }
    }

    mMappedPointer = nullptr;
  }
}

void Memory3::Flush()
{
  // TODO:
}

} // namespace Dali::Graphics::GLES
