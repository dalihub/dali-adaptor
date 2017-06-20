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

namespace NotificationLevel
{

/**
 * @brief An enum of notification window's priority level.
 */
enum Type
{
  NONE    = -1,    ///< No notification level. Default level. This value makes the notification window place in the layer of the normal window.
  BASE    = 10,    ///< Base notification level.
  MEDIUM  = 20,    ///< Higher notification level than base.
  HIGH    = 30,    ///< Higher notification level than medium.
  TOP     = 40     ///< The highest notification level.
};

} // namespace NotificationLevel

namespace ScreenMode
{

/**
 * @brief An enum of screen mode.
 */
enum Type
{
  DEFAULT,      ///< The mode which turns the screen off after a timeout.
  ALWAYS_ON     ///< The mode which keeps the screen turned on.
};

} // namespace ScreenMode

/**
 * @brief An enum of Window types.
 */
enum Type
{
  NORMAL,           ///< A default window type. Indicates a normal, top-level window. Almost every window will be created with this type.
  NOTIFICATION,     ///< A notification window, like a warning about battery life or a new E-Mail received.
  UTILITY,          ///< A persistent utility window, like a toolbox or palette.
  DIALOG            ///< Used for simple dialog windows.
};

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

/**
 * @brief Sets a window type.
 * @param[in] window The window to set a type
 * @param[in] type The window type.
 * @remarks The default window type is NORMAL.
 */
DALI_IMPORT_API void SetType( Window window, Type type );

/**
 * @brief Gets a window type.
 * @param[in] window The window to get a type
 * @return A window type.
 */
DALI_IMPORT_API Type GetType( Window window );

/**
 * @brief Sets a priority level for the specified notification window.
 * @param[in] window The window to set a notification level
 * @param[in] level The notification window level.
 * @return True if no error occurred, false otherwise.
 * @PRIVLEVEL_PUBLIC
 * @PRIVILEGE_WINDOW_PRIORITY
 * @remarks This can be used for a notification type window only. The default level is NotificationLevel::NONE.
 */
DALI_IMPORT_API bool SetNotificationLevel( Window window, NotificationLevel::Type level );

/**
 * @brief Gets a priority level for the specified notification window.
 * @param[in] window The window to get a notification level
 * @return The notification window level.
 * @remarks This can be used for a notification type window only.
 */
DALI_IMPORT_API NotificationLevel::Type GetNotificationLevel( Window window );

/**
 * @brief Sets a transparent window's visual state to opaque.
 * @details If a visual state of a transparent window is opaque,
 * then the window manager could handle it as an opaque window when calculating visibility.
 * @param[in] window The window to set a state
 * @param[in] opaque Whether the window's visual state is opaque.
 * @remarks This will have no effect on an opaque window.
 * It doesn't change transparent window to opaque window but lets the window manager know the visual state of the window.
 */
DALI_IMPORT_API void SetOpaqueState( Window window, bool opaque );

/**
 * @brief Returns whether a transparent window's visual state is opaque or not.
 * @param[in] window The window to get a state
 * @return True if the window's visual state is opaque, false otherwise.
 * @remarks The return value has no meaning on an opaque window.
 */
DALI_IMPORT_API bool IsOpaqueState( Window window );

/**
 * @brief Sets a window's screen mode.
 * @details This API is useful when the application needs to keep the display turned on.
 * If the application sets the screen mode to #ScreenMode::ALWAYS_ON to its window and the window is shown,
 * the window manager requests the display system to keep the display on as long as the window is shown.
 * If the window is no longer shown, then the window manager requests the display system to go back to normal operation.
 * @param[in] window The window to set a screen mode
 * @param[in] screenMode The screen mode.
 * @return True if no error occurred, false otherwise.
 * @PRIVLEVEL_PUBLIC
 * @PRIVILEGE_DISPLAY
 */
DALI_IMPORT_API bool SetScreenMode( Window window, ScreenMode::Type screenMode );

/**
 * @brief Gets a screen mode of the window.
 * @param[in] window The window to get a screen mode
 * @return The screen mode.
 */
DALI_IMPORT_API ScreenMode::Type GetScreenMode( Window window );

/**
 * @brief Sets preferred brightness of the window.
 * @details This API is useful when the application needs to change the brightness of the screen when it is appeared on the screen.
 * If the brightness has been set and the window is shown, the window manager requests the display system to change the brightness to the provided value.
 * If the window is no longer shown, then the window manager requests the display system to go back to default brightness.
 * A value less than 0 results in default brightness and a value greater than 100 results in maximum brightness.
 * @param[in] window The window to set a brightness
 * @param[in] brightness The preferred brightness (0 to 100).
 * @return True if no error occurred, false otherwise.
 * @PRIVLEVEL_PUBLIC
 * @PRIVILEGE_DISPLAY
 */
DALI_IMPORT_API bool SetBrightness( Window window, int brightness );

/**
 * @brief Gets preffered brightness of the window.
 * @param[in] window The window to get brightness
 * @return The preffered brightness.
 */
DALI_IMPORT_API int GetBrightness( Window window );

/**
 * @brief Returns native Ecore Wayland Window handle only for c# binding
 * @param[in] window The window where Ecore Wayland window handle is extracted
 * @return void * of native Ecore Wayland Window
 */
DALI_IMPORT_API void * GetNativeWindowHandler( Window window );

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
