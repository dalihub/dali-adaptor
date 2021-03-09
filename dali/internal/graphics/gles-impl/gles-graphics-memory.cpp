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

// CLASS HEADER
#include "gles-graphics-memory.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
Memory::Memory(const Graphics::MapBufferInfo& mapInfo, EglGraphicsController& controller)
: mController(controller)
{
  mMapBufferInfo = mapInfo;
  mMapObjectType = MapObjectType::BUFFER;
}

Memory::Memory(const Graphics::MapTextureInfo& mapInfo, EglGraphicsController& controller)
: mController(controller)
{
  mMapTextureInfo = mapInfo;
  mMapObjectType  = MapObjectType::TEXTURE;
}

Memory::~Memory()
{
  Unlock(true);
}

void* Memory::LockRegion(uint32_t offset, uint32_t size)
{
  auto gl = mController.GetGL();

  if(mMapObjectType == MapObjectType::BUFFER)
  {
    auto buffer = static_cast<GLES::Buffer*>(mMapBufferInfo.buffer);

    if(buffer->IsCPUAllocated())
    {
      using Ptr = char*;
      return Ptr(buffer->GetCPUAllocatedAddress()) + offset;
    }
    else
    {
      // @TODO: trashing vertex binding, better find target that is rarely used
      buffer->Bind(Graphics::BufferUsage::VERTEX_BUFFER);
      void* ptr      = nullptr;
      ptr            = gl->MapBufferRange(GL_ARRAY_BUFFER, mMapBufferInfo.offset, mMapBufferInfo.size, GL_MAP_WRITE_BIT);
      mMappedPointer = ptr;
    }
    return mMappedPointer;
  }

  return nullptr;
}

void Memory::Unlock(bool flush)
{
  auto gl = mController.GetGL();

  if(mMapObjectType == MapObjectType::BUFFER && mMappedPointer)
  {
    auto buffer = static_cast<GLES::Buffer*>(mMapBufferInfo.buffer);
    buffer->Bind(Graphics::BufferUsage::VERTEX_BUFFER);
    gl->UnmapBuffer(GL_ARRAY_BUFFER);
  }

  if(flush)
  {
    Flush();
  }
}

void Memory::Flush()
{
  // TODO:
}

} // namespace Dali::Graphics::GLES