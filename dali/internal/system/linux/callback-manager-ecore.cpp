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
#include <dali/internal/system/linux/callback-manager-ecore.h>

// EXTERNAL INCLUDES
#include <dali/internal/system/linux/dali-ecore.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Structure contains the callback function and control options
 */
struct EcoreCallbackData
{
  /**
   * Constructor
   */
  EcoreCallbackData(CallbackBase* callback, bool hasReturnValue)
  : mCallback(callback),
    mRemoveFromContainerFunction(NULL),
    mIdler(NULL),
    mIdleEnterer(NULL),
    mHasReturnValue(hasReturnValue)
  {
  }
  /**
   * Destructor
   */
  ~EcoreCallbackData()
  {
    delete mCallback;
    delete mRemoveFromContainerFunction;
  }

  CallbackBase*       mCallback;                    ///< call back
  CallbackBase*       mRemoveFromContainerFunction; ///< Called to remove the callbackdata from the callback container
  Ecore_Idler*        mIdler;                       ///< ecore idler
  Ecore_Idle_Enterer* mIdleEnterer;                 ///< ecore idle enterer
  bool                mHasReturnValue;              ///< true if the callback function has a return value.
};

namespace
{
/**
 * Called from the main thread while idle.
 */
Eina_Bool IdleCallback(void* data)
{
  Eina_Bool          ret          = ECORE_CALLBACK_CANCEL; // CALLBACK Cancel will delete the idler so we don't need to call ecore_idler_del
  EcoreCallbackData* callbackData = static_cast<EcoreCallbackData*>(data);

  if(callbackData->mHasReturnValue)
  {
    // run the function
    bool retValue = CallbackBase::ExecuteReturn<bool>(*callbackData->mCallback);
    if(retValue)
    {
      // keep the callback
      ret = ECORE_CALLBACK_RENEW;
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

EcoreCallbackManager::EcoreCallbackManager()
: mRunning(false)
{
}

void EcoreCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);

  mRunning = true;
}

void EcoreCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG(mRunning == true);

  RemoveAllCallbacks();

  mRunning = false;
}

bool EcoreCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning)
  {
    return false;
  }

  EcoreCallbackData* callbackData = new EcoreCallbackData(callback, hasReturnValue);

  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &EcoreCallbackManager::RemoveCallbackFromContainer);

  // add the call back to the container
  mCallbackContainer.push_front(callbackData);

  // add the idler
  callbackData->mIdler = ecore_idler_add(IdleCallback, callbackData);

  DALI_ASSERT_ALWAYS((callbackData->mIdler != NULL) && "Idle method not created");

  return true;
}

void EcoreCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  for(CallbackList::iterator it    = mCallbackContainer.begin(),
                             endIt = mCallbackContainer.end();
      it != endIt;
      ++it)
  {
    EcoreCallbackData* data = *it;

    if(data->mCallback == callback)
    {
      // remove callback data from the container.
      CallbackBase::Execute(*data->mRemoveFromContainerFunction, data);

      ecore_idler_del(data->mIdler);

      // delete our data
      delete data;

      return;
    }
  }
}

bool EcoreCallbackManager::ProcessIdle()
{
  // @todo To be implemented.
  return false;
}

void EcoreCallbackManager::ClearIdleCallbacks()
{
  // @todo To be implemented.
}

bool EcoreCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  if(!mRunning)
  {
    return false;
  }

  EcoreCallbackData* callbackData = new EcoreCallbackData(callback, true);

  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &EcoreCallbackManager::RemoveCallbackFromContainer);

  // add the call back to the container
  mCallbackContainer.push_front(callbackData);

  // add the idler
  callbackData->mIdleEnterer = ecore_idle_enterer_add(IdleCallback, callbackData);

  DALI_ASSERT_ALWAYS((callbackData->mIdleEnterer != NULL) && "Idle method not created");

  return true;
}

void EcoreCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  for(CallbackList::iterator it    = mCallbackContainer.begin(),
                             endIt = mCallbackContainer.end();
      it != endIt;
      ++it)
  {
    EcoreCallbackData* data = *it;

    if(data->mCallback == callback)
    {
      // remove callback data from the container.
      CallbackBase::Execute(*data->mRemoveFromContainerFunction, data);

      ecore_idle_enterer_del(data->mIdleEnterer);

      // delete our data
      delete data;

      return;
    }
  }
}

void EcoreCallbackManager::RemoveCallbackFromContainer(EcoreCallbackData* callbackData)
{
  mCallbackContainer.remove(callbackData);
}

void EcoreCallbackManager::RemoveAllCallbacks()
{
  // always called from main thread
  for(CallbackList::iterator iter = mCallbackContainer.begin(); iter != mCallbackContainer.end(); ++iter)
  {
    EcoreCallbackData* data = (*iter);

    if(data->mIdler)
    {
      ecore_idler_del(data->mIdler);
    }
    else if(data->mIdleEnterer)
    {
      ecore_idle_enterer_del(data->mIdleEnterer);
    }

    delete data;
  }
  mCallbackContainer.clear();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
