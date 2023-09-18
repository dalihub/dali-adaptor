/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <Windows.h>

// Need to undef the following constants as they are defined in one of the headers in Windows.h but used in DALi (via debug.h)
#undef TRANSPARENT // Used in constants.h
#undef CopyMemory  // Used in dali-vector.h

#include <dali/integration-api/debug.h>

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
  /**
   * Destructor
   */
  ~WindowsCallbackData()
  {
    delete mCallback;
  }

  CallbackBase* mCallback;       ///< call back
  bool          mHasReturnValue; ///< true if the callback function has a return value.
};

WinCallbackManager::WinCallbackManager()
: mRunning(false)
{
  mSelfCallback = MakeCallback(this, &WinCallbackManager::ProcessIdleFromFramework);
}

WinCallbackManager::~WinCallbackManager()
{
  delete mSelfCallback;
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
  if(!mRunning)
  {
    return false;
  }

  WindowsCallbackData* callbackData = new WindowsCallbackData(callback, hasReturnValue);

  mCallbackContainer.push_back(callbackData);

  if(!mSelfCallbackRegistered)
  {
    // Post only one times.
    mSelfCallbackRegistered = true;
    WindowsPlatform::PostWinThreadMessage(WIN_CALLBACK_EVENT, reinterpret_cast<uint64_t>(mSelfCallback), 0);
  }

  return true;
}

void WinCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  for(auto iter = mCallbackContainer.begin(), endIter = mCallbackContainer.end(); iter != endIter; ++iter)
  {
    auto* callbackData = *iter;

    if(callbackData->mCallback == callback)
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
  mSelfCallbackRegistered = false;

  const bool idleProcessed = !mCallbackContainer.empty();

  for(auto iter = mCallbackContainer.begin(); iter != mCallbackContainer.end();)
  {
    auto* callbackData = *iter;
    bool  removed      = true;
    if(callbackData)
    {
      if(callbackData->mHasReturnValue)
      {
        const bool retValue = Dali::CallbackBase::ExecuteReturn<bool>(*(callbackData->mCallback));

        // Do not remove callback if return value is true.
        removed = !retValue;
      }
      else
      {
        Dali::CallbackBase::Execute(*(callbackData->mCallback));
      }
    }

    if(removed)
    {
      delete (*iter);
      iter = mCallbackContainer.erase(iter);
    }
    else
    {
      ++iter;
    }
  }

  // Re-register WIN_CALLBACK_EVENT when some idle callback remained.
  if(!mCallbackContainer.empty())
  {
    if(!mSelfCallbackRegistered)
    {
      // Post only one times.
      mSelfCallbackRegistered = true;
      WindowsPlatform::PostWinThreadMessage(WIN_CALLBACK_EVENT, reinterpret_cast<uint64_t>(mSelfCallback), 0);
    }
  }

  return idleProcessed;
}

void WinCallbackManager::ClearIdleCallbacks()
{
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
