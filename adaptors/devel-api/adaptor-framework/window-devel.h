#ifndef DALI_WINDOW_DEVEL_H
#define DALI_WINDOW_DEVEL_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#ifdef DALI_ADAPTOR_COMPILATION  // full path doesn't exist until adaptor is installed so we have to use relative
#include <window.h>
#else
#include <dali/public-api/adaptor-framework/window.h>
#endif

namespace Dali
{

namespace DevelWindow
{

typedef Signal< void (bool) > FocusSignalType;      ///< Window focus signal type

/**
 * @brief The user should connect to this signal to get a timing when window gains focus or loses focus.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( bool focusIn );
 * @endcode
 * The parameter is true if window gains focus, otherwise false.
 *
 * @param[in] window The window to get a signal
 * @return The signal to connect to
 */
DALI_IMPORT_API FocusSignalType& FocusChangedSignal( Window window );

/**
 * @brief Sets whether window accepts focus or not.
 *
 * @param[in] window The window to accept focus
 * @param[in] accept If focus is accepted or not. Default is true.
 */
DALI_IMPORT_API void SetAcceptFocus( Window window, bool accept );

/**
 * @brief Returns whether window accepts focus or not.
 *
 * @param[in] window The window to accept focus
 * @return True if the window accept focus, false otherwise
 */
DALI_IMPORT_API bool IsFocusAcceptable( Window window );

/**
 * @brief Shows the window if it is hidden.
 * @param[in] window The window to show
 */
DALI_IMPORT_API void Show( Window window );

/**
 * @brief Hides the window if it is showing.
 * @param[in] window The window to hide
 */
DALI_IMPORT_API void Hide( Window window );

/**
 * @brief Returns whether the window is visible or not.
 * @param[in] window The window to query
 * @return True if the window is visible, false otherwise.
 */
DALI_IMPORT_API bool IsVisible( Window window );

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
