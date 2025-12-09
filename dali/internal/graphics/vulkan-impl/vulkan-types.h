#ifndef DALI_GRAPHICS_VULKAN_TYPES
#define DALI_GRAPHICS_VULKAN_TYPES

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
#include <atomic>
#include <bitset>
#include <memory>
#include <unordered_map>

#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>

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

class DescriptorPool;
class DescriptorSet;
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
  if(DALI_UNLIKELY(result.result != expected))
  {
    DALI_LOG_ERROR("result : %d %s, expect : %d %s\n", static_cast<int>(result.result), vk::to_string(result.result).c_str(), static_cast<int>(expected), vk::to_string(expected).c_str());
    Dali::DaliPrintBackTrace();
  }
  assert(result.result == expected);
  return result.value;
}

inline vk::Result VkAssert(vk::Result result, vk::Result expected = vk::Result::eSuccess)
{
  if(DALI_UNLIKELY(result != expected))
  {
    DALI_LOG_ERROR("result : %d %s, expect : %d %s\n", static_cast<int>(result), vk::to_string(result).c_str(), static_cast<int>(expected), vk::to_string(expected).c_str());
    Dali::DaliPrintBackTrace();
  }
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
  PLATFORM_ANDROID,
};

struct FormatInfo
{
  bool         packed{false};
  bool         compressed{false};
  unsigned int paletteSizeInBits{0u};
  unsigned int blockSizeInBits{0u};
  unsigned int blockWidth{0u};
  unsigned int blockHeight{0u};
  unsigned int blockDepth{0u};
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

struct VkCompareOpType
{
  constexpr explicit VkCompareOpType(Graphics::CompareOp compareOp)
  {
    switch(compareOp)
    {
      case Graphics::CompareOp::NEVER:
        op = vk::CompareOp::eNever;
        break;
      case Graphics::CompareOp::LESS:
        op = vk::CompareOp::eLess;
        break;
      case Graphics::CompareOp::EQUAL:
        op = vk::CompareOp::eEqual;
        break;
      case Graphics::CompareOp::LESS_OR_EQUAL:
        op = vk::CompareOp::eLessOrEqual;
        break;
      case Graphics::CompareOp::GREATER:
        op = vk::CompareOp::eGreater;
        break;
      case Graphics::CompareOp::NOT_EQUAL:
        op = vk::CompareOp::eNotEqual;
        break;
      case Graphics::CompareOp::GREATER_OR_EQUAL:
        op = vk::CompareOp::eGreaterOrEqual;
        break;
      case Graphics::CompareOp::ALWAYS:
        op = vk::CompareOp::eAlways;
        break;
    }
  }
  vk::CompareOp op{vk::CompareOp::eLess};
};

struct VkStencilOpType
{
  constexpr explicit VkStencilOpType(Graphics::StencilOp stencilOp)
  {
    switch(stencilOp)
    {
      case Graphics::StencilOp::KEEP:
        op = vk::StencilOp::eKeep;
        break;
      case Graphics::StencilOp::ZERO:
        op = vk::StencilOp::eZero;
        break;
      case Graphics::StencilOp::REPLACE:
        op = vk::StencilOp::eReplace;
        break;
      case Graphics::StencilOp::INCREMENT_AND_CLAMP:
        op = vk::StencilOp::eIncrementAndClamp;
        break;
      case Graphics::StencilOp::DECREMENT_AND_CLAMP:
        op = vk::StencilOp::eDecrementAndClamp;
        break;
      case Graphics::StencilOp::INVERT:
        op = vk::StencilOp::eInvert;
        break;
      case Graphics::StencilOp::INCREMENT_AND_WRAP:
        op = vk::StencilOp::eIncrementAndWrap;
        break;
      case Graphics::StencilOp::DECREMENT_AND_WRAP:
        op = vk::StencilOp::eDecrementAndWrap;
        break;
    }
  }
  vk::StencilOp op{vk::StencilOp::eZero};
};

namespace DepthStencilFlagBits
{
static constexpr uint32_t DEPTH_BUFFER_BIT   = 1; // depth buffer enabled
static constexpr uint32_t STENCIL_BUFFER_BIT = 2; // stencil buffer enabled
} // namespace DepthStencilFlagBits

// State of the depth-stencil buffer
using DepthStencilFlags = uint32_t;

constexpr uint32_t GetDepthStencilState(bool enableDepth, bool enableStencil)
{
  return (enableDepth ? DepthStencilFlagBits::DEPTH_BUFFER_BIT : 0u) |
         (enableStencil ? DepthStencilFlagBits::STENCIL_BUFFER_BIT : 0u);
}

// Formats
const std::array<vk::Format, 4> DEPTH_STENCIL_FORMATS = {
  vk::Format::eUndefined,     // no depth nor stencil needed
  vk::Format::eD16Unorm,      // only depth buffer
  vk::Format::eS8Uint,        // only stencil buffer
  vk::Format::eD24UnormS8Uint // depth and stencil buffers
};

} // namespace Vulkan
} // namespace Dali::Graphics

#endif // DALI_GRAPHICS_VULKAN_TYPES
