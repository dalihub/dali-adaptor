#ifndef __DALI_WAYLAND_MANAGER_H__
#define __DALI_WAYLAND_MANAGER_H__

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
#include <wayland-extension/xdg-shell-client-protocol.h>   // from wayland

// INTERNAL INCLUDES
#include <wl-types.h>
#include <file-descriptor-monitor.h>
#include "input-manager.h"
#include "wayland-window.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{



class WindowEventInterface;
class PerformanceInterface;

/**
 * Client used to talk to Wayland server over a UNIX domain stream socket.
 *
 * Brief overview of Wayland:
 *
 * Transport mechanism = Socket
 * Display = handles all the data sent from and to the compositor
 * Display has a file descriptor that can be monitored for read / write events
 *
 * wl client function calls will place messages in a queue
 * Calling wl_display_flush() will flush the messages to  the server
 *
 * Incoming data is handled in two steps: queueing and dispatching.
 * In the queue step, the data coming from the display fd is interpreted and
 * added to a queue. On the dispatch step, the handler for the incoming event is called.
 *
 * default queue is dispatched by calling wl_display_dispatch().
 *
 * The compositor sends out the frame event every time it draws a frame.
 * wl_display_frame_callback() to schedule a callback per frame.
 *
 *
 * wl_display_dispatch(). This will dispatch any events queued on the default queue and
 * attempt to read from the display fd if it's empty.
 * Events read are then queued on the appropriate queues according to the proxy assignment.
 *
 *
 */
class WaylandManager
{

public:

  /**
   * @brief Constructor
   */
  WaylandManager();

  /**
   * @brief Destructor
   */
  ~WaylandManager();

  /**
   * @brief Connect to Wayland server and setup internal data structures
   */
  void Initialise();

  /**
   * @brief Assign window event interface.
   * @param[in] eventInterface window event interface
   */
  void AssignWindowEventInterface( WindowEventInterface* eventInterface);

  /**
   * @brief create a surface for a window
   * @param[in] window window object
   */
  void CreateSurface( Dali::Wayland::Window& window );

  /**
   * @brief get the wayland surface
   * @return surface
   */
  WlSurface* GetSurface();

private: // change to private

  /**
   * @brief Install file descriptor monitor
   */
  void InstallFileDescriptorMonitor();

  /**
   * @brief File descriptor callback function, triggered when wayland compositor
   * sends an event to the client (us)
   * @param[in] eventTypeMask
   */
  void FileDescriptorCallback( FileDescriptorMonitor::EventType eventTypeMask );

  /**
   * @brief helper to get wayland interfaces
   */
  void GetWaylandInterfaces();

public:


  InputManager  mInputManager;
  WlDisplay* mDisplay;        ///< Wayland display, handles all the data sent from and to the compositor
  WlShell* mShell;            ///< shell
  WlCompositor* mCompositor;  ///< compositor
  int mDisplayFileDescriptor; ///< File descriptor used by wayland client socket
  FileDescriptorMonitor* mFileDescriptorMonitor;  ///< File descriptor monitor
  WlXdgShell* mXdgShell;                ///< XDG Shell
  WlSurface* mSurface;                  ///< Wayland surface
  WlShellSurface* mShellSurface;        ///< Shell surface
  WlXdgShellSurface*  mXdgSurface;      ///< XDG Shell surface


};
} // Internal
} // Adaptor
} // Dali

#endif
