#ifndef DALI_APPLICATION_DEVEL_H
#define DALI_APPLICATION_DEVEL_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/application.h>

namespace Dali
{

namespace DevelApplication
{


/**
 * @brief Ensures that the function passed in is called from the main loop when it is idle.
 * @param[in] application A handle to the Application
 * @param[in] callback The function to call
 * @return @c true if added successfully, @c false otherwise
 *
 * @note Function must be called from main event thread only
 *
 * A callback of the following type should be used:
 * @code
 *   bool MyFunction();
 * @endcode
 * This callback will be called repeatedly as long as it returns true. A return of 0 deletes this callback.
 *
 * @note Ownership of the callback is passed onto this class.
 */
DALI_ADAPTOR_API bool AddIdleWithReturnValue( Application application, CallbackBase* callback );

/**
* @brief Gets the absolute path to the application's data directory which is used to store private data of the application.
* @return The absolute path to the application's data directory
*/
DALI_ADAPTOR_API std::string GetDataPath();


} // namespace DevelApplication

} // namespace Dali

/**
 * @brief This is used to improve application launch performance.
 * It preloads so files, initializes some functions in advance and makes a window in advance.
 * @param[in,out]  argc A pointer to the number of arguments
 * @param[in,out]  argv A pointer to the argument list
 *
 * @note Declared in C style for calling from app-launcher.
 *
 */
extern "C"
DALI_ADAPTOR_API void PreInitialize( int* argc, char** argv[] );

#endif // DALI_APPLICATION_DEVEL_H
