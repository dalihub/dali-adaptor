#ifndef __DALI_CALLBACK_MANAGER_H__
#define __DALI_CALLBACK_MANAGER_H__

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
#include <boost/function.hpp>
#include <dali/public-api/common/dali-common.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Abstract interface to install call backs in to an applications main loop.
 */
class DALI_IMPORT_API CallbackManager
{

public:

    typedef boost::function<void(void)> Callback;   ///< Callback typedef

    /**
     * Determines the priority of the call back
     */
    enum Priority
    {
      IDLE_PRIORITY,     ///< idle priority
      DEFAULT_PRIORITY,  ///< priority of the callback will be the same as input handlers and timer callbacks.
    };

    /**
     * Controls whether an event once processed by the handler is passed on to other
     * handlers, or not.
     */
    enum EventControl
    {
      CALLBACK_PASS_ON,   ///< Pass the event on to any other handlers registered for this event
      CALLBACK_DONE,      ///< Don't pass the event to any other handlers
    };

    /**
     * Create a new call back interface
     */
    static CallbackManager* New();

    /**
     * Virtual destructor
     */
    virtual ~CallbackManager() {}

    /**
     * Adds a call back asynchronously.
     * Can be called from any thread.
     * @param callback custom call back function
     * @param priority call back priority
     * @return true on success
     */
    virtual bool AddCallback( Callback callback, Priority priority ) = 0;

    /**
     * Adds a call back asynchronously to handle an event.
     * E.g. to handle a CTRL-C event.
     * Can be called from any thread.
     * @param callback custom call back function
     * @return true on success
     */
    virtual bool AddEventCallback( Callback callback, int type, EventControl control ) = 0;

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

#endif // __DALI_CALLBACK_MANAGER_H__
