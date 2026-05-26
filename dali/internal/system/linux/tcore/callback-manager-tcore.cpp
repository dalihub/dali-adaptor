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
#include <dali/internal/system/linux/tcore/callback-manager-tcore.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <tizen_core.h>
#include <tizen_core_internal.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Structure contains the callback function and control options
 */
struct TcoreCallbackData
{
  /**
   * Constructor
   */
  TcoreCallbackData(CallbackBase* callback, bool hasReturnValue)
  : mCallback(callback),
    mRemoveFromContainerFunction(NULL),
    mSource(NULL),
    mHasReturnValue(hasReturnValue)
  {
  }
  /**
   * Destructor
   */
  ~TcoreCallbackData()
  {
    delete mCallback;
    delete mRemoveFromContainerFunction;
  }

  CallbackBase*         mCallback;                    ///< call back
  CallbackBase*         mRemoveFromContainerFunction;  ///< Called to remove the callbackdata from the callback container
  tizen_core_source_h   mSource;                       ///< tizen core idle source
  bool                  mHasReturnValue;               ///< true if the callback function has a return value.
};

namespace
{
/**
 * Called from the main thread while idle.
 * Returns false to remove the source (one-shot behavior).
 */
bool IdleCallback(void* data)
{
  TcoreCallbackData* callbackData = static_cast<TcoreCallbackData*>(data);

  if(callbackData->mHasReturnValue)
  {
    bool retValue = CallbackBase::ExecuteReturn<bool>(*callbackData->mCallback);
    if(retValue)
    {
      return true;
    }
    else
    {
      CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);
      delete callbackData;
      return false;
    }
  }
  else
  {
    CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);
    CallbackBase::Execute(*callbackData->mCallback);
    delete callbackData;
    return false;
  }
}

tizen_core_h FindTizenCore()
{
  tizen_core_h core = nullptr;
  if(tizen_core_find("main", &core) != TIZEN_CORE_ERROR_NONE)
  {
    tizen_core_find_from_this_thread(&core);
  }
  return core;
}
} // unnamed namespace

TcoreCallbackManager::TcoreCallbackManager()
: mRunning(false),
  mCore(FindTizenCore())
{
}

void TcoreCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);

  if(!mCore)
  {
    mCore = FindTizenCore();
  }

  mRunning = true;
}

void TcoreCallbackManager::Stop()
{
  DALI_ASSERT_DEBUG(mRunning == true);

  RemoveAllCallbacks();

  mRunning = false;
}

bool TcoreCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning)
  {
    return false;
  }

  if(!mCore)
  {
    return false;
  }

  TcoreCallbackData* callbackData = new TcoreCallbackData(callback, hasReturnValue);

  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &TcoreCallbackManager::RemoveCallbackFromContainer);

  mCallbackContainer.push_front(callbackData);

  int ret = tizen_core_add_idle_job(mCore, IdleCallback, callbackData, &callbackData->mSource);

  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to add tizen core idle for UI: %d\n", ret);
    mCallbackContainer.remove(callbackData);
    delete callbackData;
    return false;
  }

  DALI_ASSERT_ALWAYS((callbackData->mSource != NULL) && "Idle method not created");

  return true;
}

void TcoreCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  for(CallbackList::iterator it    = mCallbackContainer.begin(),
                             endIt = mCallbackContainer.end();
      it != endIt;
      ++it)
  {
    TcoreCallbackData* data = *it;

    if(data->mCallback == callback)
    {
      CallbackBase::Execute(*data->mRemoveFromContainerFunction, data);

      if(mCore && data->mSource)
      {
        tizen_core_remove_source(mCore, data->mSource);
      }

      delete data;

      return;
    }
  }
}

bool TcoreCallbackManager::ProcessIdle()
{
  // Idle execution is driven by tizen_core main loop (IdleCallback), not this API.
  return false;
}

void TcoreCallbackManager::ClearIdleCallbacks()
{
}

bool TcoreCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  // tizen-core uses same idle mechanism for both idler and idle enterer
  if(!mRunning)
  {
    return false;
  }

  if(!mCore)
  {
    return false;
  }

  TcoreCallbackData* callbackData = new TcoreCallbackData(callback, true);

  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &TcoreCallbackManager::RemoveCallbackFromContainer);

  mCallbackContainer.push_front(callbackData);

  int ret = tizen_core_add_idle_job_with_priority(mCore, static_cast<tizen_core_priority_e>(TIZEN_CORE_PRIORITY_UI), IdleCallback, callbackData, &callbackData->mSource);

  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to add tizen core idle for UI: %d\n", ret);
    mCallbackContainer.remove(callbackData);
    delete callbackData;
    return false;
  }

  DALI_ASSERT_ALWAYS((callbackData->mSource != NULL) && "Idle method not created");

  return true;
}

void TcoreCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  RemoveIdleCallback(callback);
}

void TcoreCallbackManager::RemoveCallbackFromContainer(TcoreCallbackData* callbackData)
{
  mCallbackContainer.remove(callbackData);
}

void TcoreCallbackManager::RemoveAllCallbacks()
{
  for(CallbackList::iterator iter = mCallbackContainer.begin(); iter != mCallbackContainer.end(); ++iter)
  {
    TcoreCallbackData* data = (*iter);

    if(mCore && data->mSource)
    {
      tizen_core_remove_source(mCore, data->mSource);
    }

    delete data;
  }
  mCallbackContainer.clear();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
