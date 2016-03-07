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
#include "compositor-output-region/compositor-output.h"

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
 * This class uses the Wayland thread safe API's because the TPL (Tizen Platform Layer) will
 * be communicating with the Wayland compositor at the same time in the DALi render thread.
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
   * @brief Reads and dispatches any events from the Wayland compositor
   * We have a file descriptor monitor active to decide when to call this function
   */
  void ReadAndDispatchEvents();

  /**
   * @brief helper to get wayland interfaces
   */
  void GetWaylandInterfaces();

public:


  InputManager  mInputManager;
  CompositorOutput mCompositorOutput;     ///< handles monitor information and DPI
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
