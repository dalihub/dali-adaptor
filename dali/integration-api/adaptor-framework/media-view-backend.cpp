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
#include <dali/integration-api/adaptor-framework/media-view-backend.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlfcn.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <mutex>
#include <utility>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/media-frame-source-plugin.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/public-api/adaptor-framework/native-image.h>

namespace DALI_NAMESPACE
{
namespace Integration
{
/**
 * @brief Keeps the producer resource and its release callback under unique ownership.
 */
struct MediaFrame::Impl
{
  Impl() = default;

  Impl(Any surface, Vector2 size, MediaFrameTransform transform, ReleaseFunction release, void* data)
  : mSurface(std::move(surface)),
    mSize(size),
    mTransform(transform),
    mRelease(release),
    mData(data)
  {
  }

  ~Impl()
  {
    if(mRelease)
    {
      mRelease(mData);
    }
  }

  Impl(const Impl&)            = delete;
  Impl& operator=(const Impl&) = delete;

  Any                 mSurface;
  Vector2             mSize;
  MediaFrameTransform mTransform{MediaFrameTransform::IDENTITY};
  ReleaseFunction     mRelease{nullptr};
  void*               mData{nullptr};
};

MediaFrame::MediaFrame() = default;

MediaFrame::MediaFrame(Any surface, Vector2 size, MediaFrameTransform transform, ReleaseFunction release, void* data)
: mImpl(MakeUnique<Impl>(std::move(surface), size, transform, release, data))
{
}

MediaFrame::~MediaFrame() = default;

MediaFrame::MediaFrame(MediaFrame&&) noexcept = default;

MediaFrame& MediaFrame::operator=(MediaFrame&&) noexcept = default;

Any MediaFrame::GetSurface() const
{
  return mImpl ? mImpl->mSurface : Any();
}

Vector2 MediaFrame::GetSize() const
{
  return mImpl ? mImpl->mSize : Vector2::ZERO;
}

MediaFrameTransform MediaFrame::GetTransform() const
{
  return mImpl ? mImpl->mTransform : MediaFrameTransform::IDENTITY;
}

MediaFrame::operator bool() const
{
  return mImpl && !mImpl->mSurface.Empty();
}

/**
 * @brief Stores presentation state and synchronizes producer callbacks with event-thread updates.
 */
struct MediaViewBackend::Impl : MediaFrameConsumer
{
  Impl()
  : mTrigger(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &Impl::Process)))
  {
  }

  ~Impl() override
  {
    ClearSource();
    mTrigger.reset();
    if(mPlugin)
    {
      mDestroyPlugin(mPlugin);
    }
    if(mLibrary)
    {
      dlclose(mLibrary);
    }
  }

  Impl(const Impl&)            = delete;
  Impl& operator=(const Impl&) = delete;

  bool LoadPlugin()
  {
    if(mPlugin)
    {
      return true;
    }
    mLibrary = dlopen(DALI_LIBRARY_PREFIX "media-frame-source-plugin.so", RTLD_LAZY);
    if(!mLibrary)
    {
      DALI_LOG_ERROR("MediaView: frame source plugin unavailable: %s\n", dlerror());
      return false;
    }
    // POSIX dlsym exposes function symbols as void*. Match the existing adaptor plugin loaders.
    auto create    = reinterpret_cast<CreateMediaFrameSourcePlugin>(dlsym(mLibrary, "CreateMediaFrameSourcePlugin"));
    mDestroyPlugin = reinterpret_cast<DestroyMediaFrameSourcePlugin>(dlsym(mLibrary, "DestroyMediaFrameSourcePlugin"));
    if(create && mDestroyPlugin)
    {
      mPlugin = create();
    }
    if(!mPlugin)
    {
      dlclose(mLibrary);
      mLibrary = nullptr;
      return false;
    }
    return true;
  }

  void OnFrame(MediaFrame frame) override
  {
    MediaFrame dropped;
    bool       notify = false;
    {
      std::lock_guard<std::mutex> lock(mMutex);
      if(!mConnected || !mAccepting)
      {
        return;
      }
      dropped  = std::move(mPending);
      mPending = std::move(frame);
      notify   = NotifyLocked();
    }
    if(notify)
    {
      mTrigger->Trigger();
    }
  }

  void OnReturnBuffer() override
  {
    ReturnBuffers();
  }

  void ReturnBuffers()
  {
    MediaFrame             discarded;
    std::deque<MediaFrame> returned;
    bool                   notify = false;
    {
      std::lock_guard<std::mutex> lock(mMutex);
      discarded = std::move(mPending);
      returned.swap(mRetained);
      mClearRequested = true;
      notify          = NotifyLocked();
    }
    // Producer release callbacks run outside the mutex. No event/GPU wait here.
    if(notify)
    {
      mTrigger->Trigger();
    }
  }

  bool NotifyLocked()
  {
    if(!mNotificationPending)
    {
      mNotificationPending = true;
      return true;
    }
    return false;
  }

  bool ClearImage()
  {
    const bool changed = static_cast<bool>(mPresentation.texture);
    mPresentation.texture.Reset();
    mNativeImage.Reset();
    mFrameSize = Vector2::ZERO;
    UpdatePresentation();
    return changed;
  }

  void ClearSource()
  {
    {
      std::lock_guard<std::mutex> lock(mMutex);
      mAccepting = false;
    }
    if(mPlugin)
    {
      mPlugin->Detach();
    }
    ReturnBuffers();
    mSource = Any();
    if(ClearImage())
    {
      mPresentationChangedSignal.Emit();
    }
  }

  void Process()
  {
    constexpr std::size_t RETAINED_FRAME_COUNT = 3u;

    MediaFrame oldest;
    bool       presentationChanged = false;
    {
      std::lock_guard<std::mutex> lock(mMutex);
      mNotificationPending = false;
      if(mClearRequested)
      {
        presentationChanged = ClearImage();
        mClearRequested     = false;
      }
      if(mPending && mConnected && mAccepting)
      {
        if(!mNativeImage)
        {
          mNativeImage = NativeImage::New(mPending.GetSurface());
          if(mNativeImage)
          {
            mPresentation.texture = Texture::New(*mNativeImage);
            presentationChanged   = true;
          }
        }
        else
        {
          mNativeImage->SetSource(mPending.GetSurface());
        }
        if(mNativeImage)
        {
          const Vector2             frameSize = mPending.GetSize();
          const MediaFrameTransform transform = mPending.GetTransform();
          presentationChanged                 = presentationChanged || frameSize != mFrameSize || transform != mTransform;
          mFrameSize                          = frameSize;
          mTransform                          = transform;
          mRetained.push_back(std::move(mPending));
          if(mRetained.size() > RETAINED_FRAME_COUNT)
          {
            oldest = std::move(mRetained.front());
            mRetained.pop_front();
          }
          UpdatePresentation();
        }
        else
        {
          oldest = std::move(mPending);
        }
      }
    }
    if(presentationChanged)
    {
      mPresentationChangedSignal.Emit();
    }
    if(Adaptor::IsAvailable())
    {
      Adaptor::Get().RequestProcessEventsAndUpdate();
    }
  }

  void UpdatePresentation()
  {
    mPresentation.sourceCrop      = mCrop;
    mPresentation.sourceTransform = mTransform;
    mPresentation.horizontalFlip  = mHorizontalFlip;
    mPresentation.verticalFlip    = mVerticalFlip;
    mPresentation.contentSize     = Vector2(mFrameSize.x * mCrop.width, mFrameSize.y * mCrop.height);
    if(mTransform == MediaFrameTransform::ROTATE_90 || mTransform == MediaFrameTransform::ROTATE_270)
    {
      std::swap(mPresentation.contentSize.x, mPresentation.contentSize.y);
    }
  }

  MediaViewPresentation                           mPresentation;
  MediaViewBackend::PresentationChangedSignalType mPresentationChangedSignal;
  TriggerEventFactory::TriggerEventPtr            mTrigger;
  NativeImagePtr                                  mNativeImage;
  Any                                             mSource;
  void*                                           mLibrary{nullptr};
  MediaFrameSourcePlugin*                         mPlugin{nullptr};
  DestroyMediaFrameSourcePlugin                   mDestroyPlugin{nullptr};
  std::mutex                                      mMutex;
  MediaFrame                                      mPending;
  std::deque<MediaFrame>                          mRetained;
  bool                                            mConnected{false};
  bool                                            mAccepting{false};
  bool                                            mNotificationPending{false};
  bool                                            mClearRequested{false};
  Vector2                                         mFrameSize;
  Rect<float>                                     mCrop{0.0f, 0.0f, 1.0f, 1.0f};
  MediaFrameTransform                             mTransform{MediaFrameTransform::IDENTITY};
  bool                                            mHorizontalFlip{false};
  bool                                            mVerticalFlip{false};
};

MediaViewBackend::MediaViewBackend()
: mImpl(MakeUnique<Impl>())
{
}

MediaViewBackend::~MediaViewBackend() = default;

MediaViewPresentation MediaViewBackend::GetPresentation() const
{
  return mImpl->mPresentation;
}

MediaViewBackend::PresentationChangedSignalType& MediaViewBackend::PresentationChangedSignal()
{
  return mImpl->mPresentationChangedSignal;
}

bool MediaViewBackend::SetSource(Any source)
{
  mImpl->ClearSource();
  if(source.Empty() || !mImpl->LoadPlugin())
  {
    return false;
  }
  {
    std::lock_guard<std::mutex> lock(mImpl->mMutex);
    mImpl->mAccepting = true;
  }
  if(!mImpl->mPlugin->Attach(source, *mImpl))
  {
    mImpl->ClearSource();
    return false;
  }
  mImpl->mSource = std::move(source);
  return true;
}

Any MediaViewBackend::GetSource() const
{
  return mImpl->mSource;
}

void MediaViewBackend::ClearSource()
{
  mImpl->ClearSource();
}

void MediaViewBackend::SetSceneConnected(bool connected)
{
  {
    std::lock_guard<std::mutex> lock(mImpl->mMutex);
    mImpl->mConnected = connected;
  }
  if(!connected)
  {
    mImpl->ReturnBuffers();
    if(mImpl->ClearImage())
    {
      mImpl->mPresentationChangedSignal.Emit();
    }
  }
}

bool MediaViewBackend::SetSourceCrop(Rect<float> crop)
{
  if(!std::isfinite(crop.x) || !std::isfinite(crop.y) || !std::isfinite(crop.width) || !std::isfinite(crop.height) ||
     crop.x < 0.0f || crop.y < 0.0f || crop.width <= 0.0f || crop.height <= 0.0f ||
     crop.x + crop.width > 1.0f || crop.y + crop.height > 1.0f)
  {
    return false;
  }
  mImpl->mCrop = crop;
  mImpl->UpdatePresentation();
  mImpl->mPresentationChangedSignal.Emit();
  return true;
}

Rect<float> MediaViewBackend::GetSourceCrop() const
{
  return mImpl->mCrop;
}

void MediaViewBackend::SetHorizontalFlipEnabled(bool enabled)
{
  mImpl->mHorizontalFlip = enabled;
  mImpl->UpdatePresentation();
  mImpl->mPresentationChangedSignal.Emit();
}

bool MediaViewBackend::IsHorizontalFlipEnabled() const
{
  return mImpl->mHorizontalFlip;
}

void MediaViewBackend::SetVerticalFlipEnabled(bool enabled)
{
  mImpl->mVerticalFlip = enabled;
  mImpl->UpdatePresentation();
  mImpl->mPresentationChangedSignal.Emit();
}

bool MediaViewBackend::IsVerticalFlipEnabled() const
{
  return mImpl->mVerticalFlip;
}
} // namespace Integration
} // namespace DALI_NAMESPACE
