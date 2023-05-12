#ifndef DALI_GRAPHICS_GLES_BUFFER_H
#define DALI_GRAPHICS_GLES_BUFFER_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-buffer-create-info.h>
#include <dali/graphics-api/graphics-buffer.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

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

private:

  void InitializeCPUBuffer();

  void InitializeGPUBuffer();

  uint32_t mBufferId{};
  void*    mBufferPtr{nullptr}; // CPU allocated memory
  bool     mCpuAllocated{false};
  bool     mTransient{false};

  bool     mSetForGLRecycling{false}; ///< If flag set true the buffer will recycle
};
} // namespace GLES
} // namespace Dali::Graphics

template<>
struct std::default_delete<Dali::Graphics::Buffer>
{
  void operator()(Dali::Graphics::Buffer* object)
  {
    // Add to the queue
    auto resource = static_cast<Dali::Graphics::GLES::Buffer*>(object);
    resource->DiscardResource();
  }
};

#endif