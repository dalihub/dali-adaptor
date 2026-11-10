#ifndef DALI_INTERNAL_NATIVE_IMAGE_RESOURCE_DESTRUCTION_RESULT_CALLBACK_H
#define DALI_INTERNAL_NATIVE_IMAGE_RESOURCE_DESTRUCTION_RESULT_CALLBACK_H

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
 */

// EXTERNAL INCLUDES
#include <dali/public-api/signals/callback.h>
#include <memory>
#include <mutex>
#include <utility>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-devel.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * Delivers one destruction result on the event thread. The image owns this
 * state, and deferred graphics work may retain it without retaining the image.
 * Registration and cancellation run on the event thread; Notify may run on the
 * render thread. Cancel disconnects the owner even if graphics retains the state.
 */
class NativeImageResourceDestructionResultCallback : public std::enable_shared_from_this<NativeImageResourceDestructionResultCallback>
{
public:
  using Result = DevelNativeImage::ResourceDestructionResult;

  explicit NativeImageResourceDestructionResultCallback(DevelNativeImage::ResourceDestructionCallback callback)
  : mCallback(std::move(callback)),
    mEventCallback(std::make_shared<EventThreadCallback>(MakeCallback(this, &NativeImageResourceDestructionResultCallback::Dispatch)))
  {
  }

  // The existing backend destruction path can trigger SUCCESS without a new
  // backend-specific implementation. Disconnect this borrowed pointer before Cancel.
  EventThreadCallback* GetEventThreadCallback() const
  {
    return mEventCallback.get();
  }

  void Notify(Result result)
  {
    std::scoped_lock lock(mMutex);
    if(mCallback && !mPending)
    {
      mResult  = result;
      mPending = true;
      mEventCallback->Trigger();
    }
  }

  void Cancel()
  {
    std::shared_ptr<EventThreadCallback>          event;
    DevelNativeImage::ResourceDestructionCallback callback;
    {
      std::scoped_lock lock(mMutex);
      callback = std::move(mCallback);
      event    = std::move(mEventCallback);
    }
    // Destroy captures and the event outside the mutex, on the event thread.
  }

private:
  void Dispatch()
  {
    auto                                          keepAlive = shared_from_this();
    std::shared_ptr<EventThreadCallback>          event;
    DevelNativeImage::ResourceDestructionCallback callback;
    Result                                        result;
    {
      std::scoped_lock lock(mMutex);
      event    = mEventCallback;
      callback = std::move(mCallback);
      result   = mResult;
    }
    // Keep the executing trigger and callable alive if the callback unregisters
    // itself, destroys its image, or installs a replacement callback.
    if(callback)
    {
      callback(result);
    }
  }

  std::mutex                                    mMutex;
  DevelNativeImage::ResourceDestructionCallback mCallback;
  std::shared_ptr<EventThreadCallback>          mEventCallback;
  Result                                        mResult{Result::SUCCESS};
  bool                                          mPending{false};
};

} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_NATIVE_IMAGE_RESOURCE_DESTRUCTION_RESULT_CALLBACK_H
