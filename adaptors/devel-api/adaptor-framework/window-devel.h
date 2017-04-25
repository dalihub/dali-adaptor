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

// EXTERNAL INCLUDES
#include <string>
#include <dali/public-api/math/rect.h>

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

/**
 * @brief Gets the count of supported auxiliary hints of the window.
 * @param[in] window The window to get the hint count
 * @return The number of supported auxiliary hints.
 *
 * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
 * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
 */
DALI_IMPORT_API unsigned int GetSupportedAuxiliaryHintCount( Window window );

/**
 * @brief Gets the supported auxiliary hint string of the window.
 * @param[in] window The window to get the hint
 * @param[in] index The index of the supported auxiliary hint lists
 * @return The auxiliary hint string of the index.
 *
 * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
 * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
 */
DALI_IMPORT_API std::string GetSupportedAuxiliaryHint( Window window, unsigned int index );

/**
 * @brief Creates an auxiliary hint of the window.
 * @param[in] window The window to create a hint
 * @param[in] hint The auxiliary hint string.
 * @param[in] value The value string.
 * @return The ID of created auxiliary hint, or @c 0 on failure.
 */
DALI_IMPORT_API unsigned int AddAuxiliaryHint( Window window, const std::string& hint, const std::string& value );

/**
 * @brief Removes an auxiliary hint of the window.
 * @param[in] window The window to remove a hint
 * @param[in] id The ID of the auxiliary hint.
 * @return True if no error occurred, false otherwise.
 */
DALI_IMPORT_API bool RemoveAuxiliaryHint( Window window, unsigned int id );

/**
 * @brief Changes a value of the auxiliary hint.
 * @param[in] window The window to set a value
 * @param[in] id The auxiliary hint ID.
 * @param[in] value The value string to be set.
 * @return True if no error occurred, false otherwise.
 */
DALI_IMPORT_API bool SetAuxiliaryHintValue( Window window, unsigned int id, const std::string& value );

/**
 * @brief Gets a value of the auxiliary hint.
 * @param[in] window The window to get a value
 * @param[in] id The auxiliary hint ID.
 * @return The string value of the auxiliary hint ID, or an empty string if none exists.
 */
DALI_IMPORT_API std::string GetAuxiliaryHintValue( Window window, unsigned int id );

/**
 * @brief Gets a ID of the auxiliary hint string.
 * @param[in] window The window to get an ID
 * @param[in] hint The auxiliary hint string.
 * @return The ID of the auxiliary hint string, or @c 0 if none exists.
 */
DALI_IMPORT_API unsigned int GetAuxiliaryHintId( Window window, const std::string& hint );

/**
 * @brief Sets a region to accept input events.
 * @param[in] window The window to set a region
 * @param[in] inputRegion The region to accept input events.
 */
DALI_IMPORT_API void SetInputRegion( Window window, const Rect< int >& inputRegion );

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
