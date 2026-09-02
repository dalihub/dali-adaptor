#ifndef DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_DATA_IMPL_H
#define DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_DATA_IMPL_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-data.h>

namespace Dali
{
/**
 * @brief The data a WindowData carries.
 *
 * Kept out of the public header so the buffer settings can record whether the
 * application asked for them at all. A negative value means the window follows
 * the system-wide setting given by the environment variables.
 *
 * Copyable, so a WindowData can be duplicated without listing every field.
 */
struct WindowData::Impl
{
  Impl()
  : mPositionSize(0, 0, 0, 0),
    mIsTransparent(true),
    mWindowType(WindowType::NORMAL),
    mIsFrontBufferRendering(false),
    mDepthBufferEnabled(-1),
    mStencilBufferEnabled(-1),
    mMultiSamplingLevel(-1)
  {
  }

  Dali::BoundsInteger mPositionSize;           ///< The position and size of the Window
  bool                mIsTransparent;          ///< The transparency of the Window
  WindowType          mWindowType;             ///< The window type of the Window
  bool                mIsFrontBufferRendering; ///< The front buffer rendering of the Window
  Dali::String        mScreen;                 ///< The current screen for supporting multiple screen

  int8_t  mDepthBufferEnabled;   ///< Whether the Window has a depth buffer, negative to follow the system
  int8_t  mStencilBufferEnabled; ///< Whether the Window has a stencil buffer, negative to follow the system
  int16_t mMultiSamplingLevel;   ///< Samples per pixel for MSAA, negative to follow the system
};

} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_DATA_IMPL_H
