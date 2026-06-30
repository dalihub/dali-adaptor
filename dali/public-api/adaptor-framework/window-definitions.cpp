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

#include <dali/public-api/adaptor-framework/window-definitions.h>

namespace Dali
{
class WindowInsetsInfo::Impl
{
public:
  WindowInsetsPartType  partType  = WindowInsetsPartType::STATUS_BAR;
  WindowInsetsPartState partState = WindowInsetsPartState::INVISIBLE;
  Extents               extents   = Extents(0, 0, 0, 0);
};

WindowInsetsInfo::WindowInsetsInfo()
: mImpl(Dali::MakeUnique<Impl>())
{
}

WindowInsetsInfo::WindowInsetsInfo(WindowInsetsPartType partType, WindowInsetsPartState partState, const Extents& extents)
: mImpl(Dali::MakeUnique<Impl>())
{
  mImpl->partType  = partType;
  mImpl->partState = partState;
  mImpl->extents   = extents;
}

WindowInsetsInfo::~WindowInsetsInfo() = default;

WindowInsetsInfo::WindowInsetsInfo(const WindowInsetsInfo& other)
: mImpl(Dali::MakeUnique<Impl>(*other.mImpl))
{
}

WindowInsetsInfo& WindowInsetsInfo::operator=(const WindowInsetsInfo& other)
{
  if(this != &other)
  {
    mImpl = Dali::MakeUnique<Impl>(*other.mImpl);
  }
  return *this;
}

WindowInsetsInfo::WindowInsetsInfo(WindowInsetsInfo&& other) noexcept
: mImpl(std::move(other.mImpl))
{
}

WindowInsetsInfo& WindowInsetsInfo::operator=(WindowInsetsInfo&& other) noexcept
{
  mImpl = std::move(other.mImpl);
  return *this;
}

WindowInsetsPartType WindowInsetsInfo::GetPartType() const
{
  return mImpl->partType;
}

WindowInsetsPartState WindowInsetsInfo::GetPartState() const
{
  return mImpl->partState;
}

const Extents& WindowInsetsInfo::GetExtents() const
{
  return mImpl->extents;
}

} // namespace Dali
