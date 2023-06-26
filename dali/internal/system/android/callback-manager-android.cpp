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
#include <dali/internal/system/android/callback-manager-android.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/android/android-framework.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/android/android-framework-impl.h>
#include <dali/internal/adaptor/common/framework.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Structure contains the callback function and control options
 */
struct CallbackData
{
  /**
   * Constructor
   */
  CallbackData(CallbackBase* callback, bool hasReturnValue)
  : mIdleId(0),
    mCallback(callback),
    mRemoveFromContainerFunction(NULL),
    mHasReturnValue(hasReturnValue)
  {
  }
  /**
   * Destructor
   */
  ~CallbackData()
  {
    delete mCallback;
    delete mRemoveFromContainerFunction;
  }

  unsigned int  mIdleId;
  CallbackBase* mCallback;                    ///< call back
  CallbackBase* mRemoveFromContainerFunction; ///< Called to remove the callbackdata from the callback container
  bool          mHasReturnValue;              ///< true if the callback function has a return value.
};

namespace
{
/**
 * Called from the main thread while idle.
 */
bool IdleCallback(void* data)
{
  bool          ret          = false;
  CallbackData* callbackData = static_cast<CallbackData*>(data);

  if(callbackData->mHasReturnValue)
  {
    // run the function
    bool retValue = CallbackBase::ExecuteReturn<bool>(*callbackData->mCallback);
    if(retValue)
    {
      // keep the callback
      ret = true;
    }
    else
    {
      // remove callback data from the container
      CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);

      // delete our data
      delete callbackData;
    }
  }
  else
  {
    // remove callback data from the container
    CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);

    // run the function
    CallbackBase::Execute(*callbackData->mCallback);

    // delete our data
    delete callbackData;
  }

  return ret;
}

} // unnamed namespace

AndroidCallbackManager::AndroidCallbackManager()
: mRunning(false)
{
}

void AndroidCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);

  mRunning = true;
}

void AndroidCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG(mRunning == true);

  RemoveAllCallbacks();

  mRunning = false;
}

bool AndroidCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning)
  {
    return false;
  }

  CallbackData* callbackData                 = new CallbackData(callback, hasReturnValue);
  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &AndroidCallbackManager::RemoveCallbackFromContainer);
  callbackData->mIdleId                      = AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).AddIdle(0, callbackData, IdleCallback);

  // add the call back to the container
  mCallbackContainer.push_front(callbackData);
  return true;
}

void AndroidCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  for(CallbackList::iterator it    = mCallbackContainer.begin(),
                             endIt = mCallbackContainer.end();
      it != endIt;
      ++it)
  {
    CallbackData* data = *it;

    if(data->mCallback == callback)
    {
      // remove callback data from the container.
      CallbackBase::Execute(*data->mRemoveFromContainerFunction, data);
      AndroidFramework::GetFramework(AndroidFramework::Get()).RemoveIdle(data->mIdleId);
      return;
    }
  }
}

bool AndroidCallbackManager::ProcessIdle()
{
  // @todo To be implemented.
  return false;
}

void AndroidCallbackManager::ClearIdleCallbacks()
{
  // @todo To be implemented.
}

bool AndroidCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  if(!mRunning)
  {
    return false;
  }

  CallbackData* callbackData = new CallbackData(callback, true);

  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &AndroidCallbackManager::RemoveCallbackFromContainer);
  callbackData->mIdleId                      = AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).AddIdle(0, callbackData, IdleCallback);

  // add the call back to the container
  mCallbackContainer.push_front(callbackData);
  return true;
}

void AndroidCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  for(CallbackList::iterator it    = mCallbackContainer.begin(),
                             endIt = mCallbackContainer.end();
      it != endIt;
      ++it)
  {
    CallbackData* data = *it;

    if(data->mCallback == callback)
    {
      // remove callback data from the container.
      CallbackBase::Execute(*data->mRemoveFromContainerFunction, data);
      AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).RemoveIdle(data->mIdleId);
      return;
    }
  }
}

void AndroidCallbackManager::RemoveCallbackFromContainer(CallbackData* callbackData)
{
  mCallbackContainer.remove(callbackData);
}

void AndroidCallbackManager::RemoveAllCallbacks()
{
  // always called from main thread
  for(CallbackList::iterator iter = mCallbackContainer.begin(); iter != mCallbackContainer.end(); ++iter)
  {
    CallbackData* data = (*iter);
    AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).RemoveIdle(data->mIdleId);
    delete data;
  }
  mCallbackContainer.clear();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
