#ifndef __DALI_ECORE_CALLBACK_MANAGER_H__
#define __DALI_ECORE_CALLBACK_MANAGER_H__

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

// EXTERNAL INCLUDES
#include <boost/thread.hpp>
#include <list>

// INTERNAL INCLUDES
#include <callback-manager.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

struct CallbackData;

/**
 * Ecore interface to install call backs in the applications main loop.
 */
class EcoreCallbackManager : public CallbackManager
{

public:

     /**
     * constructor
     */
    EcoreCallbackManager();

    /**
     * destructor
     */
    ~EcoreCallbackManager()
    {
    }

    /**
     * @copydoc CallbackManager::AddCallback()
     */
    virtual bool AddCallback(CallbackBase* callback, Priority priority);

    /**
     * @copydoc CallbackManager::AddEventCallback()
     */
    virtual bool AddEventCallback(CallbackBase* callback, int type, EventControl control);

    /**
     * @copydoc CallbackManager::Start()
     */
    virtual void Start();

    /**
     * @copydoc CallbackManager::Stop()
     */
    virtual void Stop();

    /**
     * Remove all call backs
     * Always called from the main thread
     */
    void RemoveAllCallbacksFromMainThread();

private:

    /**
     * Deletes any expired callbacks in the callback container
     */
    void RefreshContainer();

    /**
     * Removes a single call back from the container
     * Always called from main thread
     * @param callbackData callback data
     */
    void RemoveCallbackFromContainer(CallbackData *callbackData);

    /**
     * Remove a standard call back from ecore
     * Always called from main thread
     * @param callbackData callback data
     */
    void RemoveStandardCallback(CallbackData *callbackData);

    /**
     * Remove an event handler from ecore
     * Always called from main thread
     * @param callbackData callback data
     */
    void RemoveEventCallback(CallbackData *callbackData);



    typedef std::list<CallbackData *>  CallbackList;

    bool                           mRunning;            ///< flag is set to true if when running
    CallbackList                   mCallbackContainer;  ///< container of live callbacks
    boost::mutex                   mMutex;              ///< protect access to shared data
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_ECORE_CALLBACK_MANAGER_H__
