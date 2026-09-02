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

// CLASS HEADER
#include <dali/public-api/adaptor-framework/window-data.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-data-impl.h>

namespace Dali
{
WindowData::WindowData()
: mImpl(MakeUnique<Impl>())
{
}

WindowData::~WindowData() = default;

void WindowData::SetPositionSize(Dali::BoundsInteger& positionSize)
{
  mImpl->mPositionSize = positionSize;
}

Dali::BoundsInteger WindowData::GetPositionSize() const
{
  return mImpl->mPositionSize;
}

void WindowData::SetTransparency(bool transparent)
{
  mImpl->mIsTransparent = transparent;
}

bool WindowData::GetTransparency() const
{
  return mImpl->mIsTransparent;
}

void WindowData::SetWindowType(WindowType type)
{
  mImpl->mWindowType = type;
}

WindowType WindowData::GetWindowType() const
{
  return mImpl->mWindowType;
}

void WindowData::SetFrontBufferRenderingEnabled(bool enable)
{
  mImpl->mIsFrontBufferRendering = enable;
}

bool WindowData::IsFrontBufferRenderingEnabled() const
{
  return mImpl->mIsFrontBufferRendering;
}

void WindowData::SetScreen(const Dali::String& screen)
{
  mImpl->mScreen = screen;
}

Dali::String WindowData::GetScreen() const
{
  return mImpl->mScreen;
}

void WindowData::SetDepthBufferEnabled(bool enabled)
{
  mImpl->mDepthBufferEnabled = enabled ? 1 : 0;
}

void WindowData::SetStencilBufferEnabled(bool enabled)
{
  mImpl->mStencilBufferEnabled = enabled ? 1 : 0;
}

void WindowData::SetMultiSampledAntiAliasingLevel(uint8_t level)
{
  mImpl->mMultiSamplingLevel = static_cast<int16_t>(level);
}

WindowData::Impl& WindowData::GetImplementation()
{
  return *mImpl;
}

const WindowData::Impl& WindowData::GetImplementation() const
{
  return *mImpl;
}

} // namespace Dali
