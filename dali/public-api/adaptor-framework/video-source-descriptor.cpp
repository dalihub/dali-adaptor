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
#include <dali/public-api/adaptor-framework/video-source-descriptor.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>

#define DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(impl) \
  DALI_ASSERT_ALWAYS((impl) && "Cannot use a moved-from VideoSourceDescriptor object")

namespace Dali
{
struct VideoSourceDescriptor::Impl
{
  uint32_t             version{1u};
  Dali::String         providerId;
  Any                  nativeSession;
  VideoSourceOwnership ownership{VideoSourceOwnership::EXTERNAL};
  VideoRenderingMode   renderingMode{VideoRenderingMode::UNDERLAY};
};

VideoSourceDescriptor::VideoSourceDescriptor()
: mImpl(MakeUnique<Impl>())
{
}

VideoSourceDescriptor::VideoSourceDescriptor(const VideoSourceDescriptor& rhs)
: mImpl(nullptr)
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(rhs.mImpl);
  mImpl = MakeUnique<Impl>(*rhs.mImpl);
}

VideoSourceDescriptor::VideoSourceDescriptor(VideoSourceDescriptor&& rhs) noexcept = default;

VideoSourceDescriptor& VideoSourceDescriptor::operator=(const VideoSourceDescriptor& rhs)
{
  if(this != &rhs)
  {
    DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(rhs.mImpl);
    mImpl = MakeUnique<Impl>(*rhs.mImpl);
  }
  return *this;
}

VideoSourceDescriptor& VideoSourceDescriptor::operator=(VideoSourceDescriptor&& rhs) noexcept = default;

VideoSourceDescriptor::~VideoSourceDescriptor() = default;

void VideoSourceDescriptor::SetProviderId(Dali::StringView providerId)
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  mImpl->providerId = Dali::String(providerId);
}

Dali::String VideoSourceDescriptor::GetProviderId() const
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  return mImpl->providerId;
}

void VideoSourceDescriptor::SetNativeSession(Any nativeSession)
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  mImpl->nativeSession = nativeSession;
}

const Any& VideoSourceDescriptor::GetNativeSession() const
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  return mImpl->nativeSession;
}

void VideoSourceDescriptor::SetOwnership(VideoSourceOwnership ownership)
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  mImpl->ownership = ownership;
}

VideoSourceOwnership VideoSourceDescriptor::GetOwnership() const
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  return mImpl->ownership;
}

void VideoSourceDescriptor::SetRenderingMode(VideoRenderingMode renderingMode)
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  mImpl->renderingMode = renderingMode;
}

VideoRenderingMode VideoSourceDescriptor::GetRenderingMode() const
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  return mImpl->renderingMode;
}

uint32_t VideoSourceDescriptor::GetVersion() const
{
  DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR(mImpl);
  return mImpl->version;
}

} // namespace Dali

#undef DALI_ASSERT_VALID_VIDEO_SOURCE_DESCRIPTOR
