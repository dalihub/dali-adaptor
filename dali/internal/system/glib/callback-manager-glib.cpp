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
#include <dali/internal/system/glib/callback-manager-glib.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <glib.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern GMainContext* GetMainLoopContext();

/**
 * Structure contains the callback function and control options
 */
struct GlibCallbackData
{
  typedef gboolean (*CallbackFunction)(gpointer userData);

  /**
   * Constructor
   */
  GlibCallbackData(CallbackBase* callback, bool hasReturnValue)
  : mCallback(callback),
    mHasReturnValue(hasReturnValue)
  {
  }

  /**
   * Add the idle callback
   */
  void AddIdle()
  {
    GMainContext* context = GetMainLoopContext();
    if(context != nullptr)
    {
      mSource = g_idle_source_new();
      g_source_set_priority(mSource, G_PRIORITY_HIGH_IDLE);
      g_source_set_callback(mSource, &GlibCallbackData::IdleCallback, this, nullptr); // No destroyNotify
      g_source_attach(mSource, context);
    }
  }

  /**
   * Destructor
   */
  ~GlibCallbackData()
  {
    g_source_destroy(mSource);
    g_source_unref(mSource);
    mSource = nullptr;

    delete mCallback;
    delete mRemoveFromContainerFunction;
  }

  static gboolean IdleCallback(gpointer userData)
  {
    gboolean          retValue     = G_SOURCE_REMOVE;
    GlibCallbackData* callbackData = static_cast<GlibCallbackData*>(userData);

    if(callbackData->mHasReturnValue)
    {
      // run the function
      bool cont = CallbackBase::ExecuteReturn<bool>(*callbackData->mCallback);
      if(!cont)
      {
        // remove callback data from the container
        CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);
        // will clear up the handle
        delete callbackData;
      }
      else
      {
        retValue = G_SOURCE_CONTINUE;
      }
    }
    else
    {
      // remove callback data from the container first in case our callback tries to modify the container
      CallbackBase::Execute(*callbackData->mRemoveFromContainerFunction, callbackData);

      // run the function
      CallbackBase::Execute(*callbackData->mCallback);

      // will clear up the handle
      delete callbackData;
    }
    return retValue;
  }

  // Data
  CallbackBase* mCallback;                             ///< call back
  CallbackBase* mRemoveFromContainerFunction{nullptr}; ///< Called to remove the callbackdata from the callback container
  GSource*      mSource{nullptr};                      ///< idle handle
  bool          mExecute{true};                        ///< whether to run the callback
  bool          mHasReturnValue{false};                ///< true if the callback function has a return value.
};

GlibCallbackManager::GlibCallbackManager()
: mRunning(false)
{
}

void GlibCallbackManager::Start()
{
  DALI_ASSERT_DEBUG(mRunning == false);
  mRunning = true;
}

void GlibCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG(mRunning == true);

  mRunning = false;

  for(CallbackList::iterator iter = mCallbackContainer.begin(); iter != mCallbackContainer.end(); ++iter)
  {
    GlibCallbackData* data = (*iter);

    delete data;
  }
  mCallbackContainer.clear();
}

bool GlibCallbackManager::AddIdleCallback(CallbackBase* callback, bool hasReturnValue)
{
  if(!mRunning)
  {
    return false;
  }

  GlibCallbackData* callbackData = new GlibCallbackData(callback, hasReturnValue);

  // To inform the manager a callback has finished, we get it to call RemoveCallbackFromContainer
  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &GlibCallbackManager::RemoveCallbackFromContainer);

  // add the call back to the container
  mCallbackContainer.push_front(callbackData);

  // init the callback
  callbackData->AddIdle();

  return true;
}

void GlibCallbackManager::RemoveIdleCallback(CallbackBase* callback)
{
  for(CallbackList::iterator it    = mCallbackContainer.begin(),
                             endIt = mCallbackContainer.end();
      it != endIt;
      ++it)
  {
    GlibCallbackData* data = *it;

    if(data->mCallback == callback)
    {
      // remove callback data from the container.
      CallbackBase::Execute(*data->mRemoveFromContainerFunction, data);

      delete data;
    }
  }
}

bool GlibCallbackManager::ProcessIdle()
{
  // TBD
  return false;
}

void GlibCallbackManager::ClearIdleCallbacks()
{
  // TBD
}

bool GlibCallbackManager::AddIdleEntererCallback(CallbackBase* callback)
{
  if(!mRunning)
  {
    return false;
  }

  GlibCallbackData* callbackData = new GlibCallbackData(callback, true);

  // To inform the manager a callback has finished, we get it to call RemoveCallbackFromContainer
  callbackData->mRemoveFromContainerFunction = MakeCallback(this, &GlibCallbackManager::RemoveCallbackFromContainer);

  // add the call back to the front of the container
  mCallbackContainer.push_front(callbackData);

  // init the callback
  callbackData->AddIdle();

  return true;
}

void GlibCallbackManager::RemoveIdleEntererCallback(CallbackBase* callback)
{
  RemoveIdleCallback(callback);
}

void GlibCallbackManager::RemoveCallbackFromContainer(GlibCallbackData* callbackData)
{
  mCallbackContainer.remove(callbackData);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
