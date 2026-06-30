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
#include <dali/public-api/adaptor-framework/clipboard-data.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>

#define DALI_ASSERT_VALID_CLIPBOARD_DATA(impl) \
  DALI_ASSERT_ALWAYS((impl) && "Cannot use a moved-from ClipboardData object")

namespace Dali
{
struct ClipboardData::Impl
{
  Impl() = default;

  Impl(const Dali::String& mimeType, const Dali::String& content)
  : mMimeType(mimeType),
    mContent(content)
  {
  }

  Dali::String mMimeType;
  Dali::String mContent;
};

ClipboardData::ClipboardData()
: mImpl(MakeUnique<Impl>())
{
}

ClipboardData::ClipboardData(const Dali::String& mimeType, const Dali::String& content)
: mImpl(MakeUnique<Impl>(mimeType, content))
{
}

ClipboardData::ClipboardData(const ClipboardData& rhs)
: mImpl(nullptr)
{
  DALI_ASSERT_VALID_CLIPBOARD_DATA(rhs.mImpl);
  mImpl = MakeUnique<Impl>(*rhs.mImpl);
}

ClipboardData::ClipboardData(ClipboardData&& rhs) noexcept = default;

ClipboardData& ClipboardData::operator=(const ClipboardData& rhs)
{
  if(this != &rhs)
  {
    DALI_ASSERT_VALID_CLIPBOARD_DATA(rhs.mImpl);
    mImpl = MakeUnique<Impl>(*rhs.mImpl);
  }
  return *this;
}

ClipboardData& ClipboardData::operator=(ClipboardData&& rhs) noexcept = default;

ClipboardData::~ClipboardData() = default;

void ClipboardData::SetMimeType(const Dali::String& mimeType)
{
  DALI_ASSERT_VALID_CLIPBOARD_DATA(mImpl);
  mImpl->mMimeType = mimeType;
}

Dali::String ClipboardData::GetMimeType() const
{
  DALI_ASSERT_VALID_CLIPBOARD_DATA(mImpl);
  return mImpl->mMimeType;
}

void ClipboardData::SetContent(const Dali::String& content)
{
  DALI_ASSERT_VALID_CLIPBOARD_DATA(mImpl);
  mImpl->mContent = content;
}

Dali::String ClipboardData::GetContent() const
{
  DALI_ASSERT_VALID_CLIPBOARD_DATA(mImpl);
  return mImpl->mContent;
}

} // namespace Dali

#undef DALI_ASSERT_VALID_CLIPBOARD_DATA
