#ifndef DALI_WIN_CALLBACK_MANAGER_H
#define DALI_WIN_CALLBACK_MANAGER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <list>

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{
/**
 * @brief LibUV callback manager used to install call backs in the applications main loop.
 * The manager keeps track of all callbacks, so that if Stop() is called it can remove them.
 */
class WinCallbackManager : public CallbackManager
{

public:

     /**
     * @brief constructor
     */
    WinCallbackManager();

    /**
     * @brief destructor
     */
    ~WinCallbackManager(){}

    /**
     * @copydoc CallbackManager::AddIdleCallback()
     */
    virtual bool AddIdleCallback( CallbackBase* callback, bool hasReturnValue );

    /**
     * @copydoc CallbackManager::RemoveIdleCallback()
     */
    virtual void RemoveIdleCallback( CallbackBase* callback );

    /**
    * @brief Adds a @p callback to be run when entering an idle state.
    * @note Must be called from the main thread only.
    *
    * A callback of the following type should be used:
    * @code
    *   bool MyFunction();
    * @endcode
    * This callback will be called repeatedly as long as it returns true. A return of 0 deletes this callback.
    *
    * @param[in] callback custom callback function.
    *
    * @return true on success
    */
    virtual bool AddIdleEntererCallback( CallbackBase* callback );

    /**
    * @brief Removes a previously added the idle enterer callback.
    * @note Must be called from main thread only.
    *
    * Does nothing if the @p callback doesn't exist.
    *
    * @param[in] callback The callback to be removed.
    */
    virtual void RemoveIdleEntererCallback( CallbackBase* callback );

    /**
     * @copydoc CallbackManager::Start()
     */
    virtual void Start();

    /**
     * @copydoc CallbackManager::Stop()
     */
    virtual void Stop();

private:
    bool                           mRunning;            ///< flag is set to true if when running
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_WIN_CALLBACK_MANAGER_H
