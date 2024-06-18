/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/legacy/common/tizen-platform-abstraction.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/resource-types.h>
#include <dali/public-api/signals/callback.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/file-loader.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/imaging/common/image-loader.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>
#include <dali/internal/system/common/file-reader.h>

namespace Dali
{
namespace TizenPlatform
{
struct TizenPlatformAbstraction::TimerCallback : ConnectionTracker
{
  Dali::Timer                   mTimer;
  TizenPlatformAbstraction*     mOwner;
  std::unique_ptr<CallbackBase> mCallback;
  const uint32_t                mIdNumber;

  static uint32_t sNextTimerId;

  TimerCallback(TizenPlatformAbstraction* owner, CallbackBase* callback, uint32_t ms)
  : mTimer(Dali::Timer::New(ms)),
    mOwner(owner),
    mCallback(std::unique_ptr<CallbackBase>(callback)),
    mIdNumber(++sNextTimerId)
  {
    mTimer.TickSignal().Connect(this, &TimerCallback::Tick);
    mTimer.Start();
  }
  ~TimerCallback()
  {
  }

  bool Tick()
  {
    mOwner->RunTimerFunction(*this);
    return false;
  }
};

uint32_t TizenPlatformAbstraction::TimerCallback::sNextTimerId = 0;

TizenPlatformAbstraction::TizenPlatformAbstraction()
: mDataStoragePath(""),
  mTimerPairsWaiting(),
  mTimerPairsSpent(),
  mCleanupIdleCallback(nullptr)
{
}

TizenPlatformAbstraction::~TizenPlatformAbstraction()
{
  if(mCleanupIdleCallback && Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RemoveIdle(mCleanupIdleCallback);
    mCleanupIdleCallback = nullptr;
  }
}

ImageDimensions TizenPlatformAbstraction::GetClosestImageSize(const std::string& filename,
                                                              ImageDimensions    size,
                                                              FittingMode::Type  fittingMode,
                                                              SamplingMode::Type samplingMode,
                                                              bool               orientationCorrection)
{
  return ImageLoader::GetClosestImageSize(filename, size, fittingMode, samplingMode, orientationCorrection);
}

ImageDimensions TizenPlatformAbstraction::GetClosestImageSize(Integration::ResourcePointer resourceBuffer,
                                                              ImageDimensions              size,
                                                              FittingMode::Type            fittingMode,
                                                              SamplingMode::Type           samplingMode,
                                                              bool                         orientationCorrection)
{
  return ImageLoader::GetClosestImageSize(resourceBuffer, size, fittingMode, samplingMode, orientationCorrection);
}

Integration::ResourcePointer TizenPlatformAbstraction::LoadImageSynchronously(const Integration::BitmapResourceType& resource, const std::string& resourcePath)
{
  return ImageLoader::LoadImageSynchronously(resource, resourcePath);
}

Integration::BitmapPtr TizenPlatformAbstraction::DecodeBuffer(const Integration::BitmapResourceType& resource, uint8_t* buffer, size_t size)
{
  Integration::BitmapPtr   resultBitmap;
  Dali::Devel::PixelBuffer bitmap;

  Dali::Internal::Platform::FileReader fileReader(buffer, size);
  FILE* const                          fp = fileReader.GetFile();
  if(fp)
  {
    bool result = ImageLoader::ConvertStreamToBitmap(resource, "", fp, bitmap);
    if(!result || !bitmap)
    {
      bitmap.Reset();
      DALI_LOG_WARNING("Unable to decode bitmap supplied as in-memory blob.\n");
    }
    else
    {
      Integration::Bitmap::Profile profile{Integration::Bitmap::Profile::BITMAP_2D_PACKED_PIXELS};

      // For backward compatibility the Bitmap must be created
      auto retval = Integration::Bitmap::New(profile, Dali::ResourcePolicy::OWNED_DISCARD);

      retval->GetPackedPixelsProfile()->ReserveBuffer(
        bitmap.GetPixelFormat(),
        bitmap.GetWidth(),
        bitmap.GetHeight(),
        bitmap.GetWidth(),
        bitmap.GetHeight());

      auto& impl = Dali::GetImplementation(bitmap);

      std::copy(impl.GetBuffer(), impl.GetBuffer() + impl.GetBufferSize(), retval->GetBuffer());
      resultBitmap.Reset(retval);
    }
  }

  return resultBitmap;
}

bool TizenPlatformAbstraction::LoadShaderBinaryFile(const std::string& filename, Dali::Vector<unsigned char>& buffer) const
{
  bool result = false;

#ifdef SHADERBIN_CACHE_ENABLED
  std::string path;

  // First check the system location where shaders are stored at install time:
  path = DALI_SHADERBIN_DIR;
  path += filename;
  result = Dali::FileLoader::ReadFile(path, buffer);

  // Fallback to the cache of shaders stored after previous runtime compilations:
  // On desktop this looks in the current working directory that the app was launched from.
  if(result == false)
  {
    path = mDataStoragePath;
    path += filename;
    result = Dali::FileLoader::ReadFile(path, buffer);
  }
#endif

  return result;
}

bool TizenPlatformAbstraction::SaveShaderBinaryFile(const std::string& filename, const unsigned char* buffer, unsigned int numBytes) const
{
  bool result = false;

#ifdef SHADERBIN_CACHE_ENABLED

  // Use the cache of shaders stored after previous runtime compilations:
  // On desktop this looks in the current working directory that the app was launched from.
  std::string path = mDataStoragePath;
  path += filename;
  result = SaveFile(path, buffer, numBytes);

#endif

  return result;
}

void TizenPlatformAbstraction::SetDataStoragePath(const std::string& path)
{
  mDataStoragePath = path;
}

uint32_t TizenPlatformAbstraction::StartTimer(uint32_t milliseconds, CallbackBase* callback)
{
  TimerCallback* timerCallbackPtr = new TimerCallback(this, callback, milliseconds);

  // Stick it in the list
  mTimerPairsWaiting.push_back(std::unique_ptr<TimerCallback>(timerCallbackPtr));

  return timerCallbackPtr->mIdNumber;
}

void TizenPlatformAbstraction::CancelTimer(uint32_t timerId)
{
  std::vector<std::unique_ptr<TimerCallback> >::iterator timerIter = std::find_if(mTimerPairsWaiting.begin(), mTimerPairsWaiting.end(), [&timerId](std::unique_ptr<TimerCallback>& timerCallbackPtr) {
    if(timerCallbackPtr->mIdNumber == timerId)
    {
      timerCallbackPtr->mTimer.Stop();
      return true;
    }
    else
    {
      return false;
    }
  });

  if(timerIter == std::end(mTimerPairsWaiting))
  {
    DALI_LOG_DEBUG_INFO("TimerId %u Cancelled duplicated.\n", timerId);
    return;
  }

  // Move the canceled item to the spent list.
  std::move(timerIter, timerIter + 1, std::back_inserter(mTimerPairsSpent));

  mTimerPairsWaiting.erase(timerIter, timerIter + 1);

  RequestCleanupTimers();
}

void TizenPlatformAbstraction::RunTimerFunction(TimerCallback& timerPtr)
{
  CallbackBase::Execute(*timerPtr.mCallback);

  std::vector<std::unique_ptr<TimerCallback> >::iterator timerIter = std::find_if(mTimerPairsWaiting.begin(), mTimerPairsWaiting.end(), [&](std::unique_ptr<TimerCallback>& p) { return p.get() == &timerPtr; });

  if(timerIter == std::end(mTimerPairsWaiting))
  {
    // It might be cancel during callback execute.
    DALI_LOG_DEBUG_INFO("Timer might be cancelled during execute.\n");
    return;
  }

  // Move the executed item to the spent list.
  std::move(timerIter, timerIter + 1, std::back_inserter(mTimerPairsSpent));

  mTimerPairsWaiting.erase(timerIter, timerIter + 1);

  RequestCleanupTimers();
}

void TizenPlatformAbstraction::CleanupTimers()
{
  // Idler callback called. Remove it.
  mCleanupIdleCallback = nullptr;

  mTimerPairsSpent.clear();
}

void TizenPlatformAbstraction::RequestCleanupTimers()
{
  if(!mCleanupIdleCallback)
  {
    mCleanupIdleCallback = MakeCallback(this, &TizenPlatformAbstraction::CleanupTimers);
    if(DALI_UNLIKELY(!Dali::Adaptor::IsAvailable() || !Dali::Adaptor::Get().AddIdle(mCleanupIdleCallback, false)))
    {
      DALI_LOG_ERROR("Fail to add idle callback for timer function. Call it synchronously.\n");
      CleanupTimers();
    }
  }
}

TizenPlatformAbstraction* CreatePlatformAbstraction()
{
  return new TizenPlatformAbstraction();
}

bool SaveFile(const std::string& filename, const unsigned char* buffer, unsigned int numBytes)
{
  DALI_ASSERT_DEBUG(0 != filename.length());

  bool result = false;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::out | std::ios_base::trunc | std::ios::binary);
  if(buf.is_open())
  {
    std::ostream stream(&buf);

    // determine size of buffer
    int length = static_cast<int>(numBytes);

    // write contents of buffer to the file
    stream.write(reinterpret_cast<const char*>(buffer), length);

    if(!stream.bad())
    {
      result = true;
    }
    else
    {
      DALI_LOG_ERROR("std::ostream.write failed!\n");
    }
  }
  else
  {
    DALI_LOG_ERROR("std::filebuf.open failed!\n");
  }

  if(!result)
  {
    const int errorMessageMaxLength               = 128;
    char      errorMessage[errorMessageMaxLength] = {}; // Initailze as null.

    // Return type of stderror_r is different between system type. We should not use return value.
    [[maybe_unused]] auto ret = strerror_r(errno, errorMessage, errorMessageMaxLength - 1);

    DALI_LOG_ERROR("Can't write to %s. buffer pointer : %p, length : %u, error message : [%s]\n", filename.c_str(), buffer, numBytes, errorMessage);
  }

  return result;
}

} // namespace TizenPlatform

} // namespace Dali
