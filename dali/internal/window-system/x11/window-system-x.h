#ifndef DALI_INTERNAL_WINDOW_SYSTEM_X11_WINDOW_SYSTEM_H
#define DALI_INTERNAL_WINDOW_SYSTEM_X11_WINDOW_SYSTEM_H

/*
 * COPYRIGHT (c) 2025 Samsung Electronics Co., Ltd.
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
 */

#include <X11/Xlib.h>
#include <dali/devel-api/adaptor-framework/screen-information.h>
#include <dali/internal/window-system/common/window-system.h>
#include <climits>
#include <cstdint>
#include <string>
#include <vector>

namespace Dali::Internal::Adaptor::WindowSystem
{
/**
 * Class to encapsulate the actual window system calls to X11
 * Needs exporting as it's called by the graphics libraries
 */
class DALI_ADAPTOR_API WindowSystemX : public WindowSystemBase
{
public:
  static Atom ATOM_WM_PROTOCOLS;
  static Atom ATOM_WM_DELETE_WINDOW;
  static Atom ATOM_WM_STATE;
  static Atom ATOM_WM_TRANSIENT_FOR;
  static Atom ATOM_NET_ACTIVE_WINDOW;
  static Atom ATOM_NET_STARTUP_ID;
  static Atom ATOM_NET_WM_PID;
  static Atom ATOM_NET_WM_WINDOW_TYPE;
  static Atom ATOM_NET_WM_WINDOW_TYPE_NORMAL;
  static Atom ATOM_NET_WM_NAME;
  static Atom ATOM_UTF8_STRING;

  enum class WindowState
  {
    WITHDRAWN,
    NORMAL,
    ICONIC
  };

public:
  /**
   * constructor
   */
  WindowSystemX();

  /**
   * destructor
   */
  virtual ~WindowSystemX();

  /**
   * Get the screen size
   */
  void GetScreenSize(int& width, int& height) override;

  /**
   * Get the connected display
   */
  Dali::Any GetDisplay() override;

  /**
   * Get the list of screen informations for this window system.
   * It is for multiple screen environment.
   *
   * @return The list of the screen information
   */
  std::vector<Dali::ScreenInformation> GetAvailableScreens() override;

  /**
   * Get the display for this specific implementation
   */
  ::Display* GetXDisplay();

  /**
   * Synchronize with the display server if needed
   */
  void Sync();

  /**
   * Struct that describes a generic X11 event
   */
  struct X11Event : public EventBase
  {
    ::Window      window;
    const XEvent* event;
  };

  /**
   * Event struct that is sent when a window is moved/resized/lowered/raised
   */
  struct X11ConfigureNotifyEvent : public X11Event
  {
    int      x; // Relative to parent window's origin.
    int      y;
    int      width;
    int      height;
    ::Window above; // The window that this is now above.
  };

  /**
   * Event struct that is sent when a window property is changed.
   */
  struct X11PropertyNotifyEvent : public X11Event
  {
    unsigned long timestamp;
    Atom          atom;
    int           state;
  };

  /**
   * Event struct that is sent when the window needs to be redrawn
   */
  struct X11ExposeEvent : public X11Event
  {
    int x;
    int y;
    int width;
    int height;
  };

  /**
   * Event struct that is sent when mouse movement/button press occurs
   */
  struct X11MouseEvent : public X11Event
  {
    unsigned long timestamp; ///< time in milliseconds
    int           x;
    int           y;
    int           buttons;
    int           device;

    struct Multi
    {
      float pressure;
      float angle;
      int   radius;
      int   radiusX;
      int   radiusY;
    } multi;
  };

  /**
   * Event struct that is sent when the mouse wheel is scrolled or pressed
   */
  struct X11MouseWheelEvent : public X11Event
  {
    unsigned long timestamp; ///< time in milliseconds
    int           direction;
    int           x;
    int           y;
    int           z;
    unsigned int  modifiers;
  };

  /**
   * Event struct that is sent when a keypress or keyrelease occurs
   */
  struct X11KeyEvent : public X11Event
  {
    unsigned long timestamp; ///< time in milliseconds
    std::string   compose;
    std::string   keyname;
    std::string   key;
    int           keyCode;
    unsigned int  modifiers;
  };

  /**
   * Event struct that is sent when a selection is cleared
   */
  struct X11SelectionClearEvent : public X11Event
  {
    enum class SelectionType
    {
      PRIMARY,
      SECONDARY,
      XDND,
      CLIPBOARD,
      OTHER
    } selection;
  };

  /**
   * @copydoc WindowSystemBase::AddEventHandler()
   */
  EventHandler* AddEventHandler(WindowSystemBase::Event                event,
                                WindowSystemBase::EventHandlerCallback callback,
                                void*                                  data) override;
  /**
   * @copydoc WindowSystemBase::DeleteEventHandler()
   */
  void DeleteEventHandler(EventHandler* eventHandler) override;

  /**
   * Create a window
   * @param depth The color depth (24/32)
   * @param x Horizontal position of the window
   * @param y Vertical position of the window
   * @param width Width of the new window
   * @param height Height of the new window
   * @return An X11 handle to the new window
   */
  ::Window CreateWindow(int depth, int x, int y, int width, int height);

  /**
   * Set window default parameters (client machine name, PID and window type)
   * @param window The window to set the default parameters for
   */
  void SetWindowDefaults(::Window window);

  /**
   * Tells the window manager if the window is transient (e.g. a dialog) for another window
   * @param window The window to set the hints on
   * @param forWindow The window that this is transient for
   */
  void SetTransientForHint(::Window window, ::Window forWindow);

  /**
   * Removes the WM hint that the window is transient for another
   * @param window The window to remove the hints on
   */
  void UnsetTransientFor(::Window window);

  /**
   * Set/clear the given Window manager protocol on the given window
   * @param window The window to modify
   * @param protocol The protocol to add/modify/remove
   * @param value True if the protocol should be added, false if it should be removed
   */
  void SetProtocol(::Window window, Atom protocol, bool value);

  /**
   * Add a hint to the window that it accepts focus
   * @param window The window to modify
   * @param acceptsFocus True if the window should accept focus, or false if not
   */
  void SetWindowHints(::Window window, bool acceptsFocus);

  /**
   * Get the window's current state
   * @param window The window to check
   * @return The window state (withdrawn, normal or iconic)
   */
  WindowState GetWindowState(::Window window);

  /**
   * Show the window
   * @param window The window to show
   */
  void Show(::Window window);

  /**
   * Hide the window
   * @param window The window to hide
   */
  void Hide(::Window window);

  /**
   * Activate the window
   * @param window The window to activate
   */
  void Activate(::Window window);

  /**
   * Raise the window to the top
   * @param window The window to raise
   */
  void Raise(::Window window);

  /**
   * Lower the window to the bottom
   * @param window The window to lower
   */
  void Lower(::Window window);

  /**
   * Enables the X window event handlers to trigger listeners
   * @param eventType The window event type
   * @param event The window event
   */
  void TriggerEventHandler(WindowSystemBase::Event eventType, X11Event& event);

  /**
   * Get the DPI of the screen
   * @param dpiHorizontal
   * @param dpiVertical
   */
  void GetDPI(unsigned int& dpiHorizontal, unsigned int& dpiVertical);

  /**
   * Move the window to the new screen coordinates
   * @param window The window to move
   * @param x The horizontal position
   * @param y The vertical position
   */
  void Move(::Window window, int x, int y);

  /**
   * Resize the window to the given width and height
   * @param window The window to resize
   * @param width The new window width
   * @param height  The new window height
   */
  void Resize(::Window window, int width, int height);

  /**
   * Move and resize the window in one operation
   * @param window The window to resize and move
   * @param x The horizontal position
   * @param y The vertical position
   * @param width The new window width
   * @param height The new window height
   */
  void MoveResize(::Window window, int x, int y, int width, int height);

  /**
   * Set the value of a string property
   * @param window The window to set the property on
   * @param atom The property to modify
   * @param string The new string to set on the property
   */
  void SetStringProperty(::Window window, Atom atom, const std::string& string);

  /**
   * Set the class of the window
   * @param window The window to modify
   * @param name The name (title) of the window
   * @param className The class of the window
   */
  void SetClass(::Window window, const std::string& name, const std::string& className);

  /**
   * Initialize multiselection input on the window
   * @Note NOT IMPLEMENTED
   * @param window The window to initialize
   */
  void InputMultiSelect(::Window window);

  /**
   * Initialize drag and drop on the window
   * @Note NOT IMPLEMENTED
   * @param window The window to initialize
   * @param enable True if drag and drop should be enabled, false otherwise
   */
  void EnableDragAndDrop(::Window window, bool enable);

private:
  struct Impl;
  Impl* mImpl;
};

/**
 * Get the platform implementation of the window system
 * @return the platform implementation of the window system
 * Needs exporting as it's called by the graphics libraries
 */
DALI_ADAPTOR_API WindowSystemX& GetImplementation();

} // namespace Dali::Internal::Adaptor::WindowSystem

#endif // DALI_INTERNAL_WINDOW_SYSTEM_X11_WINDOW_SYSTEM_H
