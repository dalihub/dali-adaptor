#ifndef __DALI_KEY_GRAB_H__
#define __DALI_KEY_GRAB_H__

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/common/dali-vector.h>

// EXTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/key.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

class Window;

/**
 * @brief Key grab functions.
 * @SINCE_1_0.0
 */
namespace KeyGrab
{

/**
 * @brief Grabs the key specified by @a key for @a window only when @a window is the topmost window.
 *
 * This function can be used for following example scenarios:
 * - Mobile - Using volume up/down as zoom up/down in camera apps.
 *
 * @SINCE_1_0.0
 * @param[in] window The window to set
 * @param[in] daliKey The key code to grab (defined in key.h)
 * @return true if the grab succeeds
 */
DALI_ADAPTOR_API bool GrabKeyTopmost( Window window, Dali::KEY daliKey );

/**
 * @brief Ungrabs the key specified by @a key for @a window.
 *
 * @SINCE_1_0.0
 * @param[in] window The window to set
 * @param[in] daliKey The key code to ungrab (defined in key.h)
 * @return true if the ungrab succeeds
 * @note If this function is called between key down and up events of a grabbed key,
 * an application doesn't receive the key up event.
 */
DALI_ADAPTOR_API bool UngrabKeyTopmost( Window window, Dali::KEY daliKey );

/**
 * @brief Enumeration for key grab mode for platform-level APIs.
 * @SINCE_1_0.0
 */
enum KeyGrabMode
{
  TOPMOST = 0,             ///< Grab a key only when on the top of the grabbing-window stack mode. @SINCE_1_0.0
  SHARED,                  ///< Grab a key together with the other client window(s) mode. @SINCE_1_0.0
  OVERRIDE_EXCLUSIVE,      ///< Grab a key exclusively regardless of the grabbing-window's position on the window stack with the possibility of overriding the grab by the other client window mode. @SINCE_1_0.0
  EXCLUSIVE                ///< Grab a key exclusively regardless of the grabbing-window's position on the window stack mode. @SINCE_1_0.0
};

/**
 * @PLATFORM
 * @brief Grabs the key specified by @a key for @a window in @a grabMode.
 *
 * @details This function can be used for following example scenarios:
 * - TV - A user might want to change the volume or channel of the background TV contents while focusing on the foreground app.
 * - Mobile - When a user presses Home key, the homescreen appears regardless of current foreground app.
 * - Mobile - Using volume up/down as zoom up/down in camera apps.
 *
 * @SINCE_1_0.0
 * @PRIVLEVEL_PLATFORM
 * @PRIVILEGE_KEYGRAB
 * @param[in] window The window to set
 * @param[in] daliKey The key code to grab (defined in key.h)
 * @param[in] grabMode The grab mode for the key
 * @return true if the grab succeeds
 */
DALI_ADAPTOR_API bool GrabKey( Window window, Dali::KEY daliKey, KeyGrabMode grabMode );

/**
 * @PLATFORM
 * @brief Ungrabs the key specified by @a key for @a window.
 *
 * @SINCE_1_0.0
 * @PRIVLEVEL_PLATFORM
 * @PRIVILEGE_KEYGRAB
 * @param[in] window The window to set
 * @param[in] daliKey The key code to ungrab (defined in key.h)
 * @return true if the ungrab succeeds
 * @note If this function is called between key down and up events of a grabbed key,
 * an application doesn't receive the key up event.
 */
DALI_ADAPTOR_API bool UngrabKey( Window window, Dali::KEY daliKey );


/**
 * @PLATFORM
 * @brief Grabs the list of keys specified by Dali::Vector of keys for @a window in Dali::Vector of grabModes.
 *
 * @details This function can be used for following example scenarios:
 * - TV - A user might want to change the volume or channel of the background TV contents while focusing on the foreground app.
 * - Mobile - When a user presses Home key, the homescreen appears regardless of current foreground app.
 * - Mobile - Using volume up/down as zoom up/down in camera apps.
 *
 * @SINCE_1_2.0
 * @PRIVLEVEL_PLATFORM
 * @PRIVILEGE_KEYGRAB
 * @param[in] window The window to set
 * @param[in] daliKeyVector The Dali::Vector of key codes to grab (defined in key.h)
 * @param[in] grabModeVector The Dali::Vector of grab modes for the keys
 * @param[in] returnVector The Dali::Vector of return boolean values for the results of multiple grab succeeds/fails
 * @return bool false when error occurs
 */
DALI_ADAPTOR_API bool GrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, const Dali::Vector<KeyGrabMode>& grabModeVector, Dali::Vector<bool>& returnVector);


/**
 * @PLATFORM
 * @brief Ungrabs the list of keys specified by Dali::Vector of keys for @a window.
 *
 * @SINCE_1_2.0
 * @PRIVLEVEL_PLATFORM
 * @PRIVILEGE_KEYGRAB
 * @param[in] window The window to set
 * @param[in] daliKeyVector The Dali::Vector of key codes to ungrab (defined in key.h)
 * @param[in] returnVector The Dali::Vector of return boolean values for the results of multiple ungrab succeeds/fails
 * @return bool false when error occurs
 * @note If this function is called between key down and up events of a grabbed key,
 * an application doesn't receive the key up event.
 */
DALI_ADAPTOR_API bool UngrabKeyList( Window window, const Dali::Vector<Dali::KEY>& daliKeyVector, Dali::Vector<bool>& returnVector);


} // namespace KeyGrab

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_KEY_GRAB_H__
