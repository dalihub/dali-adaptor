/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/libuv/callback-manager-libuv.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <uv.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern uv_loop_t* GetUVMainLoop();

namespace
{
static void FreeHandleCallback(uv_handle_t* handle)
{
  delete handle;
}

} // namespace
/**
 * Structure contains the callback function and control options
 */
struct CallbackData
{
  typedef void (*CallbackFunction)(uv_idle_t*);

  /**
   * Constructor
   */
  CallbackData(CallbackBase* callback, bool hasReturnValue)
  : mCallback(callback),
    mHasReturnValue(hasReturnValue)
  {
  }

  /**
   * Add the idle callback
   */
  void AddIdle(CallbackFunction callback)
  {
    // heap allocate a handle as it will be alive after the CallbackData object is deleted.
    mIdleHandle = new uv_idle_t;

    uv_idle_init(GetUVMainLoop(), mIdleHandle);

    mIdleHandle->data = this;

    uv_idle_start(mIdleHandle, callback);
  }

  /**
   * Destructor
   */
  ~CallbackData()
  {
    // the handle will still be alive for a short period after calling uv_close
    // set the data to NULL to avoid a dangling pointer
    if(mIdleHandle)
    {
      mIdleHandle->data = NULL;
      uv_idle_stop(mIdleHandle);
      uv_close(reinterpret_cast<uv_handle_t*>(mIdleHandle), FreeHandleCallback);
    }

    delete mCallback;
    delete mRemoveFromContainerFunction;
  }

  // Data
  CallbackBase* mCallback;                             ///< call back
  CallbackBase* mRemoveFromContainerFunction{nullptr}; ///< Called to remove the callbackdata from the callback container
  uv_idle_t*    mIdleHandle{nullptr};                  ///< idle handle
  bool          mExecute{true};                        ///< whether to run the callback
  bool          mHasReturnValue{false};                ///< true if the callback function has a return value.
};

namespace
{
void IdleCallback(uv_idle_t* handle)
{
  CallbackData* callbackData = static_cast<CallbackData*>(handle->data);

  if(callbackData->mHasReturnValue)
  {
    // run the function
    bool retValue = CallbackBase::ExecuteReturn<bool>(*callbackData->mCallback);
    if(!retValue)
    {
      // remove callback data from the container
      CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);
    }
  }
  else
  {
    // remove callback data from the container first in case our callback tries to modify the container
    CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);

    // run the function
    CallbackBase::Execute(*callbackData->mCallback);
  }

  // will clear up the handle
  delete callbackData;
}
} // namespace

UvCallbackManager::UvCallbackManager()
: mRunning(false)
{
}

void UvCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);
  mRunning = true;
}

void UvCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG(mRunning == true);

  mRunning = false;

  for(CallbackList::iterator iter = mCallbackContainer.begin(); iter != mCallbackContainer.end(); ++iter)
  {
    CallbackData* data = (*iter);

    delete data;
  }
  mCallbackContainer.clear();
}

bool UvCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning)
  {
    return false;
  }

  CallbackData* callbackData = new CallbackData(callback, hasReturnValue);

  // To inform the manager a callback has finished, we get it to call RemoveCallbackFromContainer
  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &UvCallbackManager::RemoveCallbackFromContainer);

  // add the call back to the container
  mCallbackContainer.push_front(callbackData);

  // init the callback
  callbackData->AddIdle(&IdleCallback);

  return true;
}

void UvCallbackManager::RemoveIdleCallback(CallbackBase* callback)
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

      delete data;
    }
  }
}

bool UvCallbackManager::ProcessIdle()
{
  // TBD
  return false;
}

void UvCallbackManager::ClearIdleCallbacks()
{
  // TBD
}

bool UvCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  if(!mRunning)
  {
    return false;
  }

  CallbackData* callbackData = new CallbackData(callback, true);

  // To inform the manager a callback has finished, we get it to call RemoveCallbackFromContainer
  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &UvCallbackManager::RemoveCallbackFromContainer);

  // add the call back to the front of the container
  mCallbackContainer.push_front(callbackData);

  // init the callback
  callbackData->AddIdle(&IdleCallback);

  return true;
}

void UvCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  RemoveIdleCallback(callback);
}

void UvCallbackManager::RemoveCallbackFromContainer(CallbackData* callbackData)
{
  mCallbackContainer.remove(callbackData);
}

// Creates a concrete interface for CallbackManager
CallbackManager* CallbackManager::New()
{
  return new UvCallbackManager;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
