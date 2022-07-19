#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_LOCATORS_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_LOCATORS_H

/*
 * Copyright 2019  Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace dbusLocators
{
namespace callmgr
{
static constexpr const char* BUS       = "org.tizen.callmgr";
static constexpr const char* OBJ_PATH  = "/org/tizen/callmgr";
static constexpr const char* INTERFACE = "org.tizen.callmgr";
} // namespace callmgr

namespace accessibilityEMod
{
static constexpr const char* BUS       = "org.enlightenment.wm-screen-reader";
static constexpr const char* OBJ_PATH  = "/org/tizen/GestureNavigation";
static constexpr const char* INTERFACE = "org.tizen.GestureNavigation";

static constexpr const char* ACCESSORIES_SP_ENABLED = "AccessoriesSwitchProviderEnabled";
static constexpr const char* KEY_DOWN_SIGNAL        = "KeyDown";
static constexpr const char* KEY_UP_SIGNAL          = "KeyUp";

static constexpr const char* SCREEN_SP_ENABLED = "ScreenSwitchProviderEnabled";
static constexpr const char* MOUSE_DOWN_SIGNAL = "MouseDown";
static constexpr const char* MOUSE_UP_SIGNAL   = "MouseUp";

static constexpr const char* BACK_BUTTON_INTERCEPTION_ENABLED = "BackButtonInterceptionEnabled";
static constexpr const char* BACK_BUTTON_DOWN_SIGNAL          = "BackButtonDown";
static constexpr const char* BACK_BUTTON_UP_SIGNAL            = "BackButtonUp";
} // namespace accessibilityEMod

namespace freeDesktop
{
static constexpr const char* BUS                            = "org.freedesktop.DBus";
static constexpr const char* OBJ_PATH                       = "/org/freedesktop/DBus";
static constexpr const char* INTERFACE                      = "org.freedesktop.DBus";
static constexpr const char* PROPERTIES_INTERFACE           = "org.freedesktop.DBus.Properties";
static constexpr const char* GET_CONNECTION_UNIX_PROCESS_ID = "GetConnectionUnixProcessID";
static constexpr const char* SET                            = "Set";
static constexpr const char* GET                            = "Get";
} // namespace freeDesktop

namespace windowManager
{
static constexpr const char* BUS                  = "org.enlightenment.wm";
static constexpr const char* OBJ_PATH             = "/org/enlightenment/wm";
static constexpr const char* INTERFACE            = "org.enlightenment.wm.proc";
static constexpr const char* GET_VISIBLE_WIN_INFO = "GetVisibleWinInfo";
static constexpr const char* GET_FOCUS_PROC       = "GetFocusProc";
} // namespace windowManager

namespace atspi
{
static constexpr const char* BUS              = "org.a11y.Bus";
static constexpr const char* OBJ_PATH         = "/org/a11y/bus";
static constexpr const char* BUS_INTERFACE    = "org.a11y.Bus";
static constexpr const char* STATUS_INTERFACE = "org.a11y.Status";

static constexpr const char* GET_ADDRESS            = "GetAddress";
static constexpr const char* IS_ENABLED             = "IsEnabled";
static constexpr const char* GET_ATTRIBUTES         = "GetAttributes";
static constexpr const char* DO_ACTION_NAME         = "DoActionName";
static constexpr const char* PARENT                 = "Parent";
static constexpr const char* GET_MATCHES            = "GetMatches";
static constexpr const char* GET_INDEX_IN_PARENT    = "GetIndexInParent";
static constexpr const char* SELECT_CHILD           = "SelectChild";
static constexpr const char* NAME                   = "Name";
static constexpr const char* GET_ROLE               = "GetRole";
static constexpr const char* CHILD_COUNT            = "ChildCount";
static constexpr const char* GET_CHILD_AT_INDEX     = "GetChildAtIndex";
static constexpr const char* GET_STATE              = "GetState";
static constexpr const char* GET_RELATION_SET       = "GetRelationSet";
static constexpr const char* GET_EXTENTS            = "GetExtents";
static constexpr const char* CURRENT_VALUE          = "CurrentValue";
static constexpr const char* MAXIMUM_VALUE          = "MaximumValue";
static constexpr const char* MINIMUM_VALUE          = "MinimumValue";
static constexpr const char* GET_INTERFACES         = "GetInterfaces";
static constexpr const char* GET_NAVIGABLE_AT_POINT = "GetNavigableAtPoint";
} // namespace atspi
} // namespace dbusLocators

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_DBUS_LOCATORS_H
