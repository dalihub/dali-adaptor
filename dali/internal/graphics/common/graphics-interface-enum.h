#ifndef DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_ENUM_H
#define DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_ENUM_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <cstdint>
#include <limits>

namespace Dali
{
enum ColorDepth
{
  COLOR_DEPTH_24 = 24,
  COLOR_DEPTH_32 = 32
};

namespace Graphics
{
/**
 * @brief Surface identifier
 *
 * The surface id is used as the index for windows in the vulkan implementation
 */
using SurfaceId                    = uint32_t;
const SurfaceId INVALID_SURFACE_ID = std::numeric_limits<SurfaceId>::max();

enum class DepthStencilMode
{
  /**
   * No depth/stencil at all
   */
  NONE,

  /**
   * Optimal depth ( chosen by the implementation )
   */
  DEPTH_OPTIMAL,

  /**
   * Optimal depth and stencil ( chosen by the implementation )
   */
  DEPTH_STENCIL_OPTIMAL,

  /**
   * Depth and stencil with explicit format set in depthStencilFormat
   */
  DEPTH_STENCIL_EXPLICIT,
};

enum class SwapchainBufferingMode
{
  OPTIMAL = 0,

  DOUBLE_BUFFERING = 2,

  TRIPLE_BUFFERING = 3,
};

enum class ContextPriority
{
  /**
   * Below the system default priority. Useful for non-interactive tasks.
   */
  LOW = 0,

  /**
   * System default priority.
   */
  MEDIUM = 1,

  /**
   * Above the system default priority.
   */
  HIGH = 2,

  /**
   * Highest priority. Useful for critical tasks.
   */
  REALTIME = 3,

  DEFAULT = MEDIUM,
};

struct GraphicsCreateInfo
{
  uint32_t               surfaceWidth;
  uint32_t               surfaceHeight;
  DepthStencilMode       depthStencilMode;
  SwapchainBufferingMode swapchainBufferingMode;
  ColorDepth             colorDepth;
};

} //namespace Graphics

} //namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_ENUM_H
