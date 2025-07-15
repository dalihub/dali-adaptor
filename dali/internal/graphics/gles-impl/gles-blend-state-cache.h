#ifndef DALI_GRAPHICS_GLES_BLEND_STATE_CACHE_H
#define DALI_GRAPHICS_GLES_BLEND_STATE_CACHE_H

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
#include <dali/graphics-api/graphics-types.h>

// INTERNAL INCLUDES

namespace Dali::Graphics
{
namespace GLES
{
/**
 * @brief Cache of GL Blend State per context
 */
struct BlendStateCache
{
  enum BlendCacheChangedFlag
  {
    BLEND_ENABLED_CHANGED  = 1 << 0,
    BLEND_FUNC_CHANGED     = 1 << 1,
    BLEND_EQUATION_CHANGED = 1 << 2,

    BLEND_CACHE_CHANGED_ALL = (1 << 3) - 1,
  };

  BlendStateCache()
  : mBlendFuncSeparateSrcRGB{BlendFactor::ONE},
    mBlendFuncSeparateDstRGB{BlendFactor::ZERO},
    mBlendFuncSeparateSrcAlpha{BlendFactor::ONE},
    mBlendFuncSeparateDstAlpha{BlendFactor::ZERO},
    mBlendEquationSeparateModeRGB{BlendOp::ADD},
    mBlendEquationSeparateModeAlpha{BlendOp::ADD},
    mBlendCacheChangedFlag{BlendCacheChangedFlag::BLEND_CACHE_CHANGED_ALL},
    mBlendEnabled{false}
  {
  }

  void InvalidateCache()
  {
    mBlendCacheChangedFlag = BlendCacheChangedFlag::BLEND_CACHE_CHANGED_ALL;
  }

  // glBlendFuncSeparate() state
  BlendFactor mBlendFuncSeparateSrcRGB : 6;   ///< The srcRGB parameter passed to glBlendFuncSeparate()
  BlendFactor mBlendFuncSeparateDstRGB : 6;   ///< The dstRGB parameter passed to glBlendFuncSeparate()
  BlendFactor mBlendFuncSeparateSrcAlpha : 6; ///< The srcAlpha parameter passed to glBlendFuncSeparate()
  BlendFactor mBlendFuncSeparateDstAlpha : 6; ///< The dstAlpha parameter passed to glBlendFuncSeparate()

  // glBlendEquationSeparate state
  BlendOp mBlendEquationSeparateModeRGB : 6;   ///< Controls RGB blend mode
  BlendOp mBlendEquationSeparateModeAlpha : 6; ///< Controls Alpha blend mode

  uint8_t mBlendCacheChangedFlag : 4;

  bool mBlendEnabled : 1;
};

} // namespace GLES

} // namespace Dali::Graphics

#endif // DALI_GRAPHICS_GLES_BLEND_STATE_CACHE_H
