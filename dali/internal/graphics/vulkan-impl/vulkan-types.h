#ifndef DALI_GRAPHICS_VULKAN_TYPES
#define DALI_GRAPHICS_VULKAN_TYPES

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

// EXTERNAL INCLUDES
#include <atomic>
#include <bitset>
#include <memory>
#include <unordered_map>

#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

// Ensure we can use this type name safely.
// Something in the target compilation system is defining it.
#ifdef WAYLAND
#undef WAYLAND
#endif

namespace Dali::Graphics
{

namespace
{
// Default value use to clear the stencil buffer
constexpr auto STENCIL_DEFAULT_CLEAR_VALUE = 255u;
} // namespace

template<typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

namespace Vulkan
{

/**
 * Forward class declarations
 */
class Device;
class Queue;

class Buffer;
class CommandBufferImpl;
class CommandPool;
class DescriptorPool;
class DescriptorSet;
class Fence;
class FramebufferImpl;
class FramebufferAttachment;
class GpuMemoryBlock;
class Image;
class ImageView;
class Pipeline;
class Sampler;
class Shader;
class Surface;
class Swapchain;
class Texture;

/**
 * Unique pointers to Vulkan types
 */
using UniqueQueue = std::unique_ptr<Queue>;

/**
 * Reference wrappers
 */
using QueueRef = std::reference_wrapper<Queue>;

template<typename T>
T VkAssert(const vk::ResultValue<T>& result, vk::Result expected = vk::Result::eSuccess)
{
  assert(result.result == expected);
  return result.value;
}

inline vk::Result VkAssert(vk::Result result, vk::Result expected = vk::Result::eSuccess)
{
  assert(result == expected);
  return result;
}

inline vk::Result VkTest(vk::Result result, vk::Result expected = vk::Result::eSuccess)
{
  // todo: log if result different than expected?
  return result;
}

template<typename T>
inline uint32_t U32(T value)
{
  return static_cast<uint32_t>(value);
}

template<typename T>
inline int32_t I32(T value)
{
  return static_cast<int32_t>(value);
}

template<typename T>
inline float F32(T value)
{
  return static_cast<float>(value);
}

template<typename T>
inline double F64(T value)
{
  return static_cast<double>(value);
}

enum class Platform
{
  UNDEFINED,
  XLIB,
  XCB,
  WAYLAND,
};

struct VkLoadOpType
{
  constexpr explicit VkLoadOpType(Graphics::AttachmentLoadOp op)
  {
    switch(op)
    {
      case Graphics::AttachmentLoadOp::LOAD:
      {
        loadOp = vk::AttachmentLoadOp::eLoad;
        break;
      }
      case Graphics::AttachmentLoadOp::CLEAR:
      {
        loadOp = vk::AttachmentLoadOp::eClear;
        break;
      }
      case Graphics::AttachmentLoadOp::DONT_CARE:
      {
        loadOp = vk::AttachmentLoadOp::eDontCare;
        break;
      }
    }
  }
  vk::AttachmentLoadOp loadOp{vk::AttachmentLoadOp::eDontCare};
};

struct VkStoreOpType
{
  constexpr explicit VkStoreOpType(Graphics::AttachmentStoreOp op)
  {
    switch(op)
    {
      case Graphics::AttachmentStoreOp::STORE:
      {
        Assign(vk::AttachmentStoreOp::eStore);
        break;
      }
      case Graphics::AttachmentStoreOp::DONT_CARE:
      {
        Assign(vk::AttachmentStoreOp::eDontCare);
        break;
      }
    }
  }
  constexpr inline void Assign(vk::AttachmentStoreOp op)
  {
    storeOp = op;
  }
  vk::AttachmentStoreOp storeOp{vk::AttachmentStoreOp::eDontCare};
};

class VkManaged
{
public:
  VkManaged() = default;

  virtual ~VkManaged() = default;

  void Release()
  {
    OnRelease(--mRefCount);

    if(mRefCount == 0)
    {
      // orphaned
      if(!Destroy())
      {
        delete this;
      }
    }
  }

  void Retain()
  {
    OnRetain(++mRefCount);
  }

  uint32_t GetRefCount()
  {
    return mRefCount;
  }

  virtual bool Destroy()
  {
    return OnDestroy();
  }

  virtual void OnRetain(uint32_t refcount)
  {
  }

  virtual void OnRelease(uint32_t refcount)
  {
  }

  virtual bool OnDestroy()
  {
    return false;
  }

private:
  std::atomic_uint mRefCount{0u};
};

} // namespace Vulkan
} // namespace Dali::Graphics

#endif // DALI_GRAPHICS_VULKAN_TYPES
