#ifndef DALI_GRAPHICS_GLES_BUFFER_H
#define DALI_GRAPHICS_GLES_BUFFER_H

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
#include <dali/graphics-api/graphics-buffer-create-info.h>
#include <dali/graphics-api/graphics-buffer.h>
#include <dali/graphics-api/graphics-command-buffer.h> ///< for Graphics::ClearValue
#include <dali/graphics-api/graphics-types.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"
#include "gles-graphics-types.h" ///< for GLenum

namespace Dali::Graphics
{
class EglGraphicsController;

namespace GLES
{
using BufferResource = Resource<Graphics::Buffer, Graphics::BufferCreateInfo>;

/**
 * Buffer class represents a GPU buffer object. It may represent
 * vertex buffer, index buffer, pixel buffer, uniform buffer and
 * any other.
 */
class Buffer : public BufferResource
{
public:
  Buffer(const Graphics::BufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  ~Buffer() override = default;

  /**
   * @brief Called when resource is being destroyed
   */
  void DestroyResource() override;

  bool InitializeResource() override;

  void DiscardResource() override;

  void Bind(Graphics::BufferUsage bindingTarget) const;

  bool TryRecycle(const Graphics::BufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller) override;

  [[nodiscard]] uint32_t GetBufferChangedCount() const
  {
    return mBufferChangedCount;
  }

  void IncreaseBufferChangedCount()
  {
    ++mBufferChangedCount;
  }

  [[nodiscard]] uint32_t GetGLBuffer() const
  {
    return mBufferId;
  }

  [[nodiscard]] void* GetCPUAllocatedAddress() const
  {
    return mBufferPtr;
  }

  [[nodiscard]] bool IsTransient() const
  {
    return mTransient;
  }

  [[nodiscard]] bool IsCPUAllocated() const
  {
    return mCpuAllocated;
  }

  [[nodiscard]] GLenum GetBufferTarget() const
  {
    return mBufferTarget;
  }

private:
  void InitializeCPUBuffer();

  void InitializeGPUBuffer();

  uint32_t mBufferId{};
  void*    mBufferPtr{nullptr}; // CPU allocated memory
  GLenum   mBufferTarget{GL_ARRAY_BUFFER};
  bool     mCpuAllocated : 1;
  bool     mTransient : 1;

  uint32_t mBufferChangedCount{0u};
  uint32_t mSetForGLRecyclingCount{0u}; ///< If value is not zero, the buffer will recycle
};
} // namespace GLES
} // namespace Dali::Graphics

#endif
