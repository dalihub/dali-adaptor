#ifndef DALI_APPLICATION_DEVEL_H
#define DALI_APPLICATION_DEVEL_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <dali/public-api/signals/dali-signal.h>

namespace Dali
{
namespace DevelApplication
{
using CustomCommandReceivedSignalType = Signal<void(const std::string&)>; ///< Signal signature for CustomCommandReceivedSignal

/**
   * @brief This is the constructor for applications.
   * Especially, it is for keyboard application.
   * If you want to create Ime window, use this API with WindowType::IME.
   *
   * @param[in,out]  argc                A pointer to the number of arguments
   * @param[in,out]  argv                A pointer to the argument list
   * @param[in]      stylesheet          The path to user defined theme file
   * @param[in]      windowMode          A member of WINDOW_MODE
   * @param[in]      positionSize        A position and a size of the window
   * @param[in]      type                It is window type for default window.
   * @return A handle to the Application
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
   */
DALI_ADAPTOR_API Application New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize, WindowType type);

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
DALI_ADAPTOR_API bool AddIdleWithReturnValue(Application application, CallbackBase* callback);

/**
* @brief Gets the absolute path to the application's data directory which is used to store private data of the application.
* @return The absolute path to the application's data directory
*/
DALI_ADAPTOR_API std::string GetDataPath();

/**
 * @brief Downcasts a ref object to Application handle
 *
 * If handle points to an Application object, the downcast produces valid base handle
 * If not, the returned base handle is left uninitialized
 *
 * @param[in] refObject to an Application
 * @return handle to an Application object or an uninitialized base handle
 */
DALI_ADAPTOR_API Application DownCast(Dali::RefObject* refObject);

/**
 * @brief This signal will be triggered when a custom command is received.
 *
 * For this signal to be triggered, the adaptor must be built with -DENABLE_NETWORK_LOGGING=ON
 * and when running, DALI_NETWORK_CONTROL=1 must also be set.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName(const std::string&);
 * @endcode
 *
 * @param[in] application A handle to the Application
 * @return The signal when a custom command is received
 */
DALI_ADAPTOR_API CustomCommandReceivedSignalType& CustomCommandReceivedSignal(Application application);

/**
 * @brief Gets the render thread id of DALi.
 * @note If render thread id getter doesn't supported, it will return 0 as default.
 * @param application A handle to the Application
 * @return The render thread id.
 */
DALI_ADAPTOR_API int32_t GetRenderThreadId(Application application);

} // namespace DevelApplication

} // namespace Dali

/**
 * @brief This is used to improve application launch performance.
 * Initializes some functions in advance and makes a window in advance.
 * @param[in,out]  argc A pointer to the number of arguments
 * @param[in,out]  argv A pointer to the argument list
 *
 * @note Declared in C style for calling from app-launcher.
 *
 */
extern "C" DALI_ADAPTOR_API void ApplicationPreInitialize(int* argc, char** argv[]);

#endif // DALI_APPLICATION_DEVEL_H
