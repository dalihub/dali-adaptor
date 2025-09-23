#ifndef DALI_GRAPHICS_GLES_MEMORY_H
#define DALI_GRAPHICS_GLES_MEMORY_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-memory.h>
#include <cstdint>

// INTERNAL INCLUDES
#include "gles-graphics-buffer.h"
#include "gles-graphics-texture.h"

namespace Dali::Graphics
{
class EglGraphicsController;
namespace GLES
{
class Memory2 : public Dali::Graphics::Memory
{
public:
  Memory2(const Graphics::MapBufferInfo& mapInfo, EglGraphicsController& controller);

  Memory2(const Graphics::MapTextureInfo& mapInfo, EglGraphicsController& controller);

  ~Memory2() override;

  void* LockRegion(uint32_t offset, uint32_t size) override;

  void Unlock(bool flush) override;

  void Flush() override;

  EglGraphicsController& mController;

  enum class MapObjectType
  {
    NONE,
    BUFFER,
    TEXTURE
  };

  MapObjectType mMapObjectType{MapObjectType::NONE};

  union
  {
    Graphics::MapTextureInfo mMapTextureInfo{};
    Graphics::MapBufferInfo  mMapBufferInfo;
  };

  void* mMappedPointer{nullptr};
  bool  mIsAllocatedLocally{false};
};
} // namespace GLES
} // namespace Dali::Graphics

#endif