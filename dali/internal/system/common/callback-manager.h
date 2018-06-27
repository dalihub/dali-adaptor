#ifndef __DALI_INTERNAL_CALLBACK_MANAGER_H__
#define __DALI_INTERNAL_CALLBACK_MANAGER_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/callback.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Abstract interface to install call backs in to an applications main loop.
 */
class CallbackManager
{

public:

    /**
     * Create a new call back interface
     */
    static CallbackManager* New();

    /**
     * Virtual destructor
     */
    virtual ~CallbackManager() {}

    /**
     * @brief Adds a @p callback to be run on idle.
     * @note Must be called from the main thread only.
     *
     * Callbacks of the following types may be used:
     * @code
     *   void MyFunction();
     * @endcode
     * This callback will be deleted once it is called.
     *
     * @code
     *   bool MyFunction();
     * @endcode
     * This callback will be called repeatedly as long as it returns true. A return of 0 deletes this callback.
     *
     * @param[in] callback custom callback function.
     * @param[in] hasReturnValue Sould be set to true if the callback function has a return value.
     *
     * @return true on success
     */
    virtual bool AddIdleCallback( CallbackBase* callback, bool hasReturnValue ) = 0;

    /**
     * @brief Removes a previously added @p callback.
     * @note Must be called from main thread only.
     *
     * Does nothing if the @p callback doesn't exist.
     *
     * @param[in] callback The callback to be removed.
     */
    virtual void RemoveIdleCallback( CallbackBase* callback ) = 0;

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
    virtual bool AddIdleEntererCallback( CallbackBase* callback ) = 0;

    /**
     * @brief Removes a previously added the idle enterer callback.
     * @note Must be called from main thread only.
     *
     * Does nothing if the @p callback doesn't exist.
     *
     * @param[in] callback The callback to be removed.
     */
    virtual void RemoveIdleEntererCallback( CallbackBase* callback ) = 0;

    /**
     * Starts the callback manager.
     */
    virtual void Start() = 0;

    /**
     * Stop the callback manager and can remove all pending callbacks synchronously.
     * This call will synchronise with the main loop and not return
     * until all call backs have been deleted.
     */
    virtual void Stop() = 0;

protected:

    /**
     * constructor
     */
    CallbackManager() {}

private:

    // Undefined copy constructor.
    CallbackManager( const CallbackManager& );

    // Undefined assignment operator.
    CallbackManager& operator=( const CallbackManager& );

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_CALLBACK_MANAGER_H__
