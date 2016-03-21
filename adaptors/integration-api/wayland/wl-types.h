#ifndef __DALI_WL_TYPES_H__
#define __DALI_WL_TYPES_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <wayland-client.h> // from wayland-devel package

#ifdef WAYLAND_EXTENSIONS_SUPPORTED
#include <wayland-extension/xdg-shell-client-protocol.h>   // from wayland
#endif

namespace Dali
{

typedef ::wl_display WlDisplay;
typedef ::wl_surface WlSurface;
typedef ::wl_array WlArray;
typedef ::wl_output WlOutput;

typedef ::wl_shell WlShell;
typedef ::wl_shell_surface WlShellSurface;

typedef ::wl_registry WlRegistry;
typedef ::wl_compositor WlCompositor;
typedef ::wl_seat WlSeat;
typedef ::wl_pointer WlPointer;
typedef ::wl_keyboard WlKeyboard;
typedef ::wl_touch WlTouch;

typedef ::wl_seat_listener WlSeatListener;
typedef ::wl_pointer_listener WlPointerListener;
typedef ::wl_touch_listener WlTouchListener;
typedef ::wl_keyboard_listener WlKeyboardListener;

#ifdef WAYLAND_EXTENSIONS_SUPPORTED
typedef ::xdg_shell WlXdgShell;
typedef ::xdg_surface WlXdgShellSurface;
#else
typedef void* WlXdgShell;
typedef void* WlXdgShellSurface;


#endif

} // namespace Dali

#endif /* __DALI_WL_TYPES_H__ */
