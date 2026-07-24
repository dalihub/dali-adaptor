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
#include <dali/internal/system/windows/callback-manager-win.h>

// EXTERNAL INCLUDES
#include <windows.h>

// Need to undef the following constants as they are defined in one of the headers in Windows.h but used in DALi (via debug.h)
#undef TRANSPARENT // Used in constants.h
#undef CopyMemory  // Used in dali-vector.h

#include <dali/integration-api/debug.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Structure contains the callback function and control options
 */
struct WindowsCallbackData
{
  /**
   * Constructor
   */
  WindowsCallbackData(CallbackBase* callback, bool hasReturnValue)
  : mCallback(callback),
    mHasReturnValue(hasReturnValue)
  {
  }
  // Data
  std::shared_ptr<CallbackBase> mCallback;       ///< callback with execution-safe ownership
  bool                          mHasReturnValue; ///< true if the callback function has a return value.
};

WinCallbackManager::WinCallbackManager()
: mSelfCallbackToken(WindowsPlatform::RegisterWinCallback(MakeCallback(this, &WinCallbackManager::ProcessIdleFromFramework))),
  mRunning(false)
{
}

WinCallbackManager::~WinCallbackManager()
{
  WindowsPlatform::UnregisterWinCallback(mSelfCallbackToken);
  mSelfCallbackToken = 0u;
  ClearIdleCallbacks();
}

void WinCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);
  mRunning = true;
}

void WinCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG(mRunning == true);

  ClearIdleCallbacks();

  mRunning = false;
}

bool WinCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning || !callback)
  {
    return false;
  }

  if(!mSelfCallbackRegistered)
  {
    if(!WindowsPlatform::PostWinCallback(mSelfCallbackToken))
    {
      DALI_LOG_ERROR("Failed to queue the Windows idle callback\n");
      return false; // Ownership of callback remains with the caller.
    }
    mSelfCallbackRegistered = true;
  }

  mCallbackContainer.push_back(new WindowsCallbackData(callback, hasReturnValue));
  return true;
}

void WinCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  if(mExecutingCallback && mExecutingCallback->mCallback.get() == callback)
  {
    mExecutingCallbackRemoved = true;
    return;
  }

  for(auto iter = mCallbackContainer.begin(), endIter = mCallbackContainer.end(); iter != endIter; ++iter)
  {
    auto* callbackData = *iter;

    if(callbackData->mCallback.get() == callback)
    {
      // delete our data
      delete callbackData;

      // Set stored value as nullptr. It will be removed from container after ProcessIdle()
      (*iter) = nullptr;

      return;
    }
  }
}

bool WinCallbackManager::ProcessIdle()
{
  if(mExecutingCallback)
  {
    return false; // Do not permit recursive processing to replace execution state.
  }

  mSelfCallbackRegistered = false;

  const bool idleProcessed = !mCallbackContainer.empty();
  size_t     callbacksLeft = mCallbackContainer.size();

  // Pop each entry before invoking user code. Stop(), ClearIdleCallbacks(), or
  // RemoveIdleCallback() may then mutate the list without invalidating an
  // iterator held across callback execution.
  while(callbacksLeft-- > 0u && !mCallbackContainer.empty())
  {
    auto* callbackData = mCallbackContainer.front();
    mCallbackContainer.pop_front();
    bool removed = true;
    if(callbackData)
    {
      mExecutingCallback        = callbackData;
      mExecutingCallbackRemoved = false;
      auto callback             = callbackData->mCallback;
      if(callbackData->mHasReturnValue)
      {
        const bool retValue = Dali::CallbackBase::ExecuteReturn<bool>(*callback);

        // Do not remove callback if return value is true.
        removed = !retValue;
      }
      else
      {
        Dali::CallbackBase::Execute(*callback);
      }

      removed = removed || mExecutingCallbackRemoved || !mRunning;
      mExecutingCallback        = nullptr;
      mExecutingCallbackRemoved = false;
    }

    if(removed)
    {
      delete callbackData;
    }
    else
    {
      mCallbackContainer.push_back(callbackData);
    }
  }

  // Re-register WIN_CALLBACK_EVENT when some idle callback remained.
  if(mRunning && !mCallbackContainer.empty())
  {
    if(!mSelfCallbackRegistered)
    {
      // Post only one times.
      mSelfCallbackRegistered = true;
      if(!WindowsPlatform::PostWinCallback(mSelfCallbackToken))
      {
        mSelfCallbackRegistered = false;
        DALI_LOG_ERROR("Failed to requeue the Windows idle callback\n");
        // Repeating callbacks cannot make progress without a queue message;
        // release them instead of leaving them permanently registered.
        ClearIdleCallbacks();
      }
    }
  }

  return idleProcessed;
}

void WinCallbackManager::ClearIdleCallbacks()
{
  if(mExecutingCallback)
  {
    mExecutingCallbackRemoved = true;
  }

  for(auto iter = mCallbackContainer.begin(), endIter = mCallbackContainer.end(); iter != endIter; ++iter)
  {
    auto* callbackData = *iter;
    delete callbackData;
  }
  mCallbackContainer.clear();
}

bool WinCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  return AddIdleCallback(callback, true);
}

void WinCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  RemoveIdleCallback(callback);
}

void WinCallbackManager::ProcessIdleFromFramework()
{
  ProcessIdle();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
