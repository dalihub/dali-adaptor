/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "ecore-callback-manager.h"

// EXTERNAL INCLUDES
#include <Ecore.h>

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
struct CallbackData
{
  /**
   * the type of callback
   */
  enum CallbackType
  {
    STANDARD_CALLBACK,  ///< either an idle call back, or a default call back
    EVENT_HANDLER       ///< event handler
  };

  /**
   * Constructor
   */
  CallbackData( CallbackBase* callback, CallbackType type )
  :  mCallback(callback),
     mType(type),
     mIdler(NULL),
     mPriority(CallbackManager::DEFAULT_PRIORITY),
     mExecute(true),
     mEventHandler(NULL),
     mEvent(0),
     mEventControl(CallbackManager::CALLBACK_PASS_ON),
     mRemoveFromContainerFunction(NULL)
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

  // Data
  CallbackBase*                   mCallback;      ///< call back
  CallbackType                    mType;          ///< type of call back

    // Data for idle / default call backs
  Ecore_Idler*                    mIdler;         ///< ecore idler
  CallbackManager::Priority       mPriority;      ///< Priority (idle or normal)
  bool                            mExecute;       ///< whether to run the callback

  // Data for event handlers
  Ecore_Event_Handler*            mEventHandler;  ///< ecore handler
  int                             mEvent;         ///< ecore event id
  CallbackManager::EventControl   mEventControl;  ///< event control

  CallbackBase*                   mRemoveFromContainerFunction; ///< Called to remove the callbackdata from the callback container
};

namespace
{

/**
 * Called from the main thread while idle.
 */
Eina_Bool IdleCallback(void *data)
{
  CallbackData *callbackData = static_cast<CallbackData *>(data);

  // remove callback data from the container first in case our callback tries to modify the container
  CallbackBase::Execute( *callbackData->mRemoveFromContainerFunction, callbackData );

  // run the function
  CallbackBase::Execute( *callbackData->mCallback );

  // remove the idle call back
  ecore_idler_del(callbackData->mIdler);

  delete callbackData;

  return ECORE_CALLBACK_CANCEL;
}

/**
 * Ecore callback event handler, called from the main thread
 * @param data  user data
 * @param type event type, e.g. ECORE_EVENT_SIGNAL_EXIT
 * @param event pointer to ecore event
 */
Eina_Bool EventHandler(void *data, int type, void *event)
{
  CallbackData* callbackData = static_cast<CallbackData*>(data);

  // make sure the type is for the right event
  DALI_ASSERT_ALWAYS( type == callbackData->mEvent && "Callback data does not match event" );

  // remove callback data from the container first in case our callback tries to modify the container
  CallbackBase::Execute( *callbackData->mRemoveFromContainerFunction, callbackData );

  // run the call back
  CallbackBase::Execute( *callbackData->mCallback );

  Eina_Bool returnVal;

  if (callbackData->mEventControl == CallbackManager::CALLBACK_PASS_ON)
  {
    returnVal = ECORE_CALLBACK_PASS_ON;
  }
  else
  {
    returnVal = ECORE_CALLBACK_DONE;
  }

  delete callbackData;

  return returnVal;
}

/**
 * called from MainLoopCallback to process standard callbacks
 */
void AddStandardCallback(CallbackData *callbackData)
{
  if (callbackData->mPriority == CallbackManager::IDLE_PRIORITY)
  {
    // run the call back on idle
    callbackData->mIdler = ecore_idler_add(IdleCallback, callbackData);
    DALI_ASSERT_ALWAYS( callbackData->mIdler != NULL && "Idle method not created" );
  }
  else
  {
    // run the call back now, then delete it from the container
    if ( callbackData->mExecute )
    {
      CallbackBase::Execute( *callbackData->mCallback );
    }
    CallbackBase::Execute( *callbackData->mRemoveFromContainerFunction, callbackData );
    delete callbackData;
  }
}

/**
 * called from MainLoopCallback to add event callbacks
 */
void AddEventCallback(CallbackData *callbackData)
{
  callbackData->mEventHandler = ecore_event_handler_add(callbackData->mEvent, &EventHandler, callbackData);
}

/**
 * main loop call back to process call back data.
 */
void MainLoopCallback(void *data)
{
  CallbackData *callbackData = static_cast< CallbackData* >(data);

  if (callbackData->mType ==  CallbackData::STANDARD_CALLBACK)
  {
    AddStandardCallback(callbackData);
  }
  else if (callbackData->mType ==  CallbackData::EVENT_HANDLER)
  {
    AddEventCallback(callbackData);
  }
}

/**
 * Main loop call back to remove all call back data
 */
void* MainRemoveAllCallback(void* data)
{
  EcoreCallbackManager *callbackManager = static_cast<EcoreCallbackManager *>(data);

  callbackManager->RemoveAllCallbacksFromMainThread();

  return NULL;
}

} // unnamed namespace

EcoreCallbackManager::EcoreCallbackManager()
:mRunning(false)
{
}

void EcoreCallbackManager::RemoveStandardCallback(CallbackData *callbackData)
{
  if (callbackData->mPriority == CallbackManager::IDLE_PRIORITY)
  {
    // delete the idle call back
    ecore_idler_del(callbackData->mIdler);
    delete callbackData;
  }
  else
  {
    // ecore doesn't give us a handle for functions we want executing on the
    // main thread, E.g. we can't do
    // handle = ecore_main_loop_thread_safe_call_async( myfunc )
    // ecore_main_loop_thread_remove_async_call(handle);  // doesn't exist
    //
    // We just have to set a flag to say do not execute.
    // Hence we can't delete the call back at this point.
    callbackData->mExecute = false;
  }
}

void EcoreCallbackManager::RemoveEventCallback(CallbackData *callbackData)
{
  ecore_event_handler_del(callbackData->mEventHandler);

  delete callbackData;
}

void EcoreCallbackManager::Start()
{
  DALI_ASSERT_DEBUG( mRunning == false );

  mRunning = true;
}

void EcoreCallbackManager::Stop()
{
  // make sure we're not called twice
  DALI_ASSERT_DEBUG( mRunning == true );

  // lock out any other call back functions
  boost::unique_lock< boost::mutex > lock( mMutex );

  mRunning = false;

  // the synchronous calls return data from the callback, which we ignore.
  ecore_main_loop_thread_safe_call_sync(MainRemoveAllCallback, this);
}

bool EcoreCallbackManager::AddCallback(CallbackBase* callback, Priority priority)
{
  bool added(false);

  if ( mRunning )
  {
    CallbackData *callbackData = new CallbackData(callback, CallbackData::STANDARD_CALLBACK);

    callbackData->mPriority = priority;

    callbackData->mRemoveFromContainerFunction =  MakeCallback( this, &EcoreCallbackManager::RemoveCallbackFromContainer );

    { // acquire lock to access container
      boost::unique_lock< boost::mutex > lock( mMutex );

      // add the call back to the container
      mCallbackContainer.push_front(callbackData);
    }

    // Get callbackData processed on the main loop..

    ecore_main_loop_thread_safe_call_async(MainLoopCallback, callbackData);

    added = true;
  }

  return added;
}

bool EcoreCallbackManager::AddEventCallback(CallbackBase* callback, int type, EventControl control)
{
  bool added(false);

  if( mRunning )
  {
    CallbackData *callbackData = new CallbackData(callback,CallbackData::EVENT_HANDLER);
    callbackData->mEventControl = control;
    callbackData->mEvent = type;

    callbackData->mRemoveFromContainerFunction =  MakeCallback( this, &EcoreCallbackManager::RemoveCallbackFromContainer );

    { // acquire lock to access container
      boost::unique_lock< boost::mutex > lock( mMutex );

      // add the call back to the container
      mCallbackContainer.push_front(callbackData);
    }

    // Get callbackData processed on the main loop..
    ecore_main_loop_thread_safe_call_async(MainLoopCallback, callbackData);

    added = true;
  }

  return added;
}

void EcoreCallbackManager::RemoveCallbackFromContainer(CallbackData *callbackData)
{
  // always called from main loop
  boost::unique_lock< boost::mutex > lock( mMutex );

  mCallbackContainer.remove(callbackData);
}

void EcoreCallbackManager::RemoveAllCallbacksFromMainThread()
{
   // always called from main thread
   // the mutex will already be locked at this point

   for( CallbackList::iterator  iter =  mCallbackContainer.begin(); iter != mCallbackContainer.end(); ++iter)
   {
     CallbackData* data = (*iter);

     if (data->mType ==  CallbackData::STANDARD_CALLBACK)
     {
       RemoveStandardCallback(data);
     }
     else if (data->mType ==  CallbackData::EVENT_HANDLER)
     {
       RemoveEventCallback(data);
     }
   }
   mCallbackContainer.clear();
}

// Creates a concrete interface for CallbackManager
CallbackManager* CallbackManager::New()
{
  return new EcoreCallbackManager;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
