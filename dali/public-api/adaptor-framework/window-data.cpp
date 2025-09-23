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

// CLASS HEADER
#include <dali/public-api/adaptor-framework/window-data.h>

namespace Dali
{
struct WindowData::Impl
{
  Impl()
  : mPositionSize(0, 0, 0, 0),
    mIsTransparent(true),
    mWindowType(WindowType::NORMAL),
    mIsFrontBufferRendering(false)
  {
  }

  Dali::Rect<int> mPositionSize;           ///< The position and size of the Window
  bool            mIsTransparent;          ///< The transparency of the Window
  WindowType      mWindowType;             ///< The window type of the Window
  bool            mIsFrontBufferRendering; ///< The front buffer rendering of the Window
  std::string     mScreen;                 ///< The current screen for supporting multiple screen
};

WindowData::WindowData()
: mImpl(std::make_unique<Impl>())
{
}

WindowData::~WindowData() = default;

void WindowData::SetPositionSize(Dali::Rect<int>& positionSize)
{
  mImpl->mPositionSize = positionSize;
}

Dali::Rect<int> WindowData::GetPositionSize() const
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

void WindowData::SetFrontBufferRendering(bool enable)
{
  mImpl->mIsFrontBufferRendering = enable;
}

bool WindowData::GetFrontBufferRendering() const
{
  return mImpl->mIsFrontBufferRendering;
}

void WindowData::SetScreen(const std::string& screen)
{
  mImpl->mScreen = screen;
}

std::string WindowData::GetScreen() const
{
  return mImpl->mScreen;
}

} // namespace Dali
