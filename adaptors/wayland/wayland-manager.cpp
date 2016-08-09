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
// CLASS HEADER
#include "wayland-manager.h"

// EXTERNAL INCLUDES
#include <cstring>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <base/interfaces/window-event-interface.h>
#include <base/interfaces/performance-interface.h>


namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace
{

const int NO_EVENTS_ALREADY_IN_QUEUE = 0;


void ShellSurfacePing( void* data, struct wl_shell_surface* shell_surface, uint32_t serial)
{
  // The ping event comes from the compositor to checks if we're still alive.
  wl_shell_surface_pong(shell_surface, serial);
}

void ShellSurfaceConfigure( void* data, struct wl_shell_surface* shell_surface, uint32_t edges, int32_t width, int32_t height)
{
}

const struct wl_shell_surface_listener ShellSurfaceListener =
{
  ShellSurfacePing,
  ShellSurfaceConfigure,
};


void XdgShellPing( void* data, Dali::WlXdgShell* shell, uint32_t serial)
{
  // The ping event comes from the compositor to checks if we're still alive.
  xdg_shell_pong( shell, serial);
}

struct xdg_shell_listener XdgShellListener=
{
  XdgShellPing,
};


void RegistryGlobalCallback( void *data,
           struct wl_registry *wlRegistry,
           uint32_t name,
           const char *interface,
           uint32_t version)
{

  WaylandManager* client = static_cast<   WaylandManager* >( data );

  if( strcmp( interface, wl_compositor_interface.name ) == 0 )
  {
    client->mCompositor = static_cast< Dali::WlCompositor* >( wl_registry_bind( wlRegistry, name, &wl_compositor_interface, version ) );
  }
  else if( strcmp( interface, wl_seat_interface.name ) == 0 )
  {
    // register for seat callbacks and add a new seat to the input manager
    Dali::WlSeat* seatInterface = static_cast< Dali::WlSeat* >( wl_registry_bind( wlRegistry, name, &wl_seat_interface, version ) );

    client->mInputManager.AddSeatListener( seatInterface );
  }
  else if( strcmp( interface, wl_output_interface.name ) == 0 )
  {
    // get the interface and add the listener
    Dali::WlOutput* output = static_cast< Dali::WlOutput* >( wl_registry_bind( wlRegistry, name, &wl_output_interface, version ) );
    client->mCompositorOutput.AddListener( output );
  }
  else if( strcmp( interface, wl_shell_interface.name ) == 0 )
  {
    client->mShell = static_cast< Dali::WlShell* >( wl_registry_bind( wlRegistry, name, &wl_shell_interface, version ) );
  }
  else if( strcmp( interface, xdg_shell_interface.name ) == 0 )
  {
    client->mXdgShell = static_cast< struct xdg_shell* >( wl_registry_bind( wlRegistry, name, &xdg_shell_interface, version ) );
    // without this line Tizen 3 reports...
    // xdg_shell@7: error 0: Must call use_unstable_version first
    xdg_shell_use_unstable_version( client->mXdgShell, 5 );
  }
  else if( strcmp( interface, wl_text_input_manager_interface.name ) == 0 )
  {
    Dali::WlTextInputManager* inputManager = static_cast< Dali::WlTextInputManager* >( wl_registry_bind( wlRegistry, name, &wl_text_input_manager_interface, version ) );

    client->mInputManager.AddTextInputManager( inputManager );
  }


}


void RegistryGlobalCallbackRemove(void *data, struct wl_registry *registry, uint32_t id)
{
  // occurs when a device is unplugged
}

const struct wl_registry_listener RegistryListener =
{
   RegistryGlobalCallback,
   RegistryGlobalCallbackRemove,
};

} //unnamed namespace

WaylandManager::WaylandManager()
:mDisplay( NULL ),
 mShell( NULL ),
 mCompositor( NULL ),
 mDisplayFileDescriptor( 0 ),
 mFileDescriptorMonitor( NULL ),
 mXdgShell( NULL ),
 mSurface( NULL ),
 mShellSurface( NULL ),
 mXdgSurface( NULL )
{
}
WaylandManager::~WaylandManager()
{
  if( mXdgShell)
  {
    xdg_shell_destroy( mXdgShell );
  }
  if( mShell )
  {
    wl_shell_destroy( mShell );
  }
  if( mCompositor )
  {
    wl_compositor_destroy( mCompositor );
  }
  if( mDisplay )
  {
    wl_display_disconnect( mDisplay );
  }

  delete mFileDescriptorMonitor;
}

void WaylandManager::Initialise()
{
  if( mDisplay )
  {
    return;
  }
  // Connect to a Wayland socket on the Wayland server/
  // NULL = use default display aka "wayland-0"
  // returns a new display context object

  mDisplay =  wl_display_connect( NULL );

  DALI_ASSERT_ALWAYS( mDisplay && "wl_display_connect failed");

  // Monitor the display file descriptor used to communicate with Wayland server
  InstallFileDescriptorMonitor();

  mInputManager.AssignDisplay( mDisplay );

  // Get the interfaces to compositor / shell etc
  GetWaylandInterfaces();

}
void WaylandManager::ReadAndDispatchEvents()
{
  // Wayland client uses a single file descriptor to communicate with the compositor.
  // Because DALi can have multiple client threads ( event thread for input, render thread for Tizen buffer management / TPL )
  // it has to use the Wayland client thread safe API to prevent a dead lock

  // prepare_read announces the calling thread's intention to read from the file descriptor
  // If there is already events queued up in the default queue, then dispatch those first
  while( wl_display_prepare_read( mDisplay ) != NO_EVENTS_ALREADY_IN_QUEUE )
  {
    // dispatch the event, e.g. a touch event or a clipboard event
    wl_display_dispatch_pending( mDisplay );
  }

  // At this point the default queue is empty.
  // We read data from the file descriptor in their respective queues
  // This is thread safe. No other threads will read from the fd and queue events during this operation.
  int ret = wl_display_read_events( mDisplay );

  if( ret == 0 )
  {
    // dispatch the events from the default queue
    wl_display_dispatch_pending( mDisplay );
  }
  else
  {
    DALI_LOG_ERROR("wl_display_read_events error\n");
  }

}
void WaylandManager::AssignWindowEventInterface( WindowEventInterface* eventInterface)
{
  mInputManager.AssignWindowEventInterface( eventInterface );
}
void WaylandManager::GetWaylandInterfaces()
{
  // get and listen to the registry
  WlRegistry* registry = wl_display_get_registry( mDisplay );

  wl_registry_add_listener( registry, &RegistryListener, this);

  // adaptor wants the DPI instantly, so we have wait for the data
  while( !mCompositorOutput.DataReady() )
  {
    // This is the first and last time we use wl_display_roundtrip as its not thread safe
    // however at this point we haven't started rendering it is safe.
    wl_display_flush( mDisplay );
    wl_display_roundtrip( mDisplay );
  }
  wl_registry_destroy( registry );

}

void WaylandManager::InstallFileDescriptorMonitor()
{
  // get the file descriptor
  mDisplayFileDescriptor = wl_display_get_fd( mDisplay );

  // create the callback that gets triggered when a read / write event occurs
  CallbackBase* callback =  MakeCallback( this, &WaylandManager::FileDescriptorCallback);

  // monitor read and write events
  int events = FileDescriptorMonitor::FD_READABLE;

  mFileDescriptorMonitor = new FileDescriptorMonitor( mDisplayFileDescriptor, callback, events );

}

void WaylandManager::FileDescriptorCallback( FileDescriptorMonitor::EventType eventTypeMask )
{
  if( eventTypeMask & FileDescriptorMonitor::FD_READABLE )
  {
    // read and dispatch events
    ReadAndDispatchEvents();
  }
}


void WaylandManager::CreateSurface( Dali::Wayland::Window& window )
{
  // Create the surface
  // A Wayland surface is a rectangular area that is displayed on the screen.
  // It has a location, size and pixel contents.

  mSurface = wl_compositor_create_surface( mCompositor );

  // the Input panel (Virtual keyboard ) needs to know which surface it should display on.
  mInputManager.AssignSurface( mSurface );

  DALI_ASSERT_ALWAYS( mSurface && "wl_compositor_create_surface failed" );

  // keep track of the surface id
  window.mSurfaceId = wl_proxy_get_id( reinterpret_cast<struct wl_proxy *>( mSurface ));

  // A surface in Wayland needs to be assigned a role.
  // This gives the surface the role of a shell surface.
  // It allows the surface to be treated like a toplevel, fullscreen or popup window.
  // which can be moved, resize or maximized and have associate metadata like title and class.
  // Try the xdg shell first. XDG is a designed to create a desktop shell with features like minimise etc.
  if( mXdgShell )
  {
    mXdgSurface = xdg_shell_get_xdg_surface( mXdgShell, mSurface );

    DALI_ASSERT_ALWAYS( mXdgSurface&& "xdg_shell_get_xdg_surface failed" );

    if( window.mTitle.length() )
    {
      xdg_surface_set_title( mXdgSurface, window.mTitle.c_str() );
      xdg_surface_set_app_id(mXdgSurface, window.mTitle.c_str() );
    }
    xdg_shell_add_listener( mXdgShell, &XdgShellListener, 0);
  }
  else
  {
    // try the generic Wayland Shell
    mShellSurface = wl_shell_get_shell_surface( mShell, mSurface );
    DALI_ASSERT_ALWAYS( mShellSurface && "wl_shell_get_shell_surface failed" );

    if( window.mTitle.length() )
    {
       wl_shell_surface_set_title( mShellSurface, window.mTitle.c_str() );
    }

    wl_shell_surface_set_toplevel( mShellSurface);

    wl_shell_surface_add_listener( mShellSurface , &ShellSurfaceListener, 0);
  }

  wl_display_flush( mDisplay );

}

WlSurface* WaylandManager::GetSurface()
{
   return mSurface;
}

}
}
}
