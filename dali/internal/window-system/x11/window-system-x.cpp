/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/x11/window-system-x.h>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/keyboard.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/file-descriptor-monitor.h>
#include <dali/internal/window-system/common/window-system.h>

// EXTERNAL_HEADERS
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrender.h>

#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const int MOUSE_SCROLL_WHEEL_UP{4};
const int MOUSE_SCROLL_WHEEL_DOWN{5};
const int MOUSE_SCROLL_WHEEL_LEFT{6};
const int MOUSE_SCROLL_WHEEL_RIGHT{7};

/**
 * @brief Get an XWindow property
 *
 * @tparam T - The type of data to return
 * @param[in] display The display containing the window
 * @param[in] window The window to get the property for
 * @param[in] property The property to acquire
 * @param[in] type The property type
 * @param[out] data The property data
 *
 * @return true if the property was successfully retrieved
 */
template<typename T>
bool GetWindowProperty(::Display* display, ::Window window, ::Atom property, ::Atom type, std::vector<T>& data)
{
  ::Atom         actualType;
  int            actualFormat;
  unsigned long  numberOfItems = 0, bytesRemaining = 0;
  unsigned char* propertyData{nullptr};

  if(!window)
  {
    window = DefaultRootWindow(display);
  }

  XSync(display, false);
  XGetWindowProperty(display, window, property, 0, LONG_MAX, False, type, &actualType, &actualFormat, &numberOfItems, &bytesRemaining, &propertyData);

  if(actualFormat == 0 || numberOfItems == 0 || actualType != type)
  {
    XFree(propertyData);
    return false;
  }

  switch(actualFormat)
  {
    case 8:
    {
      if(sizeof(T) == sizeof(unsigned char))
      {
        data.resize(numberOfItems + 1); // Allow space for c-string terminator
        for(int i = 0; i < numberOfItems + 1; ++i)
        {
          data[i] = static_cast<T>(propertyData[i]);
        }
      }
      break;
    }
    case 16:
    {
      if(sizeof(T) == sizeof(unsigned short))
      {
        data.resize(numberOfItems);
        for(int i = 0; i < numberOfItems; ++i)
        {
          data[i] = static_cast<T>(((unsigned short*)propertyData)[i]);
        }
      }
      break;
    }
    case 32:
    {
      if(sizeof(T) == sizeof(unsigned long))
      {
        data.resize(numberOfItems);
        for(unsigned long i = 0; i < numberOfItems; ++i)
        {
          // X returns native long type, regardless of whether it's actually 32 bits or not
          data[i] = static_cast<T>(((unsigned long*)propertyData)[i]);
        }
      }
      break;
    }
  }
  XFree(propertyData);
  return data.size() != 0;
}

}; // namespace

namespace WindowSystem
{
static WindowSystemX* gWindowSystem{nullptr};
static bool           gGeometryHittest = false;

/**
 * Initialize the window system (currently run from the first window that gets created)
 */
void Initialize()
{
  if(nullptr == gWindowSystem)
  {
    gWindowSystem = new WindowSystemX();
  }
}

/**
 * Shutdown the window system (Currently run from the first window
 */
void Shutdown()
{
  if(nullptr != gWindowSystem)
  {
    delete gWindowSystem;
    gWindowSystem = nullptr;
  }
}

Atom WindowSystemX::ATOM_WM_PROTOCOLS{0};
Atom WindowSystemX::ATOM_WM_STATE{0};
Atom WindowSystemX::ATOM_WM_DELETE_WINDOW{0};
Atom WindowSystemX::ATOM_WM_TRANSIENT_FOR{0};
Atom WindowSystemX::ATOM_NET_ACTIVE_WINDOW{0};
Atom WindowSystemX::ATOM_NET_STARTUP_ID{0};
Atom WindowSystemX::ATOM_NET_WM_PID{0};
Atom WindowSystemX::ATOM_NET_WM_WINDOW_TYPE{0};
Atom WindowSystemX::ATOM_NET_WM_WINDOW_TYPE_NORMAL{0};
Atom WindowSystemX::ATOM_NET_WM_NAME{0};
Atom WindowSystemX::ATOM_UTF8_STRING{0};

struct AtomItem
{
  std::string name;
  Atom*       atom;
};

const AtomItem atomItems[] =
  {
    {"UTF8_STRING", &WindowSystemX::ATOM_UTF8_STRING},
    {"WM_DELETE_WINDOW", &WindowSystemX::ATOM_WM_DELETE_WINDOW},
    {"WM_PROTOCOLS", &WindowSystemX::ATOM_WM_PROTOCOLS},
    {"WM_STATE", &WindowSystemX::ATOM_WM_STATE},
    {"WM_TRANSIENT_FOR", &WindowSystemX::ATOM_WM_TRANSIENT_FOR},
    {"_NET_ACTIVE_WINDOW", &WindowSystemX::ATOM_NET_ACTIVE_WINDOW},
    {"_NET_STARTUP_ID", &WindowSystemX::ATOM_NET_STARTUP_ID},
    {"_NET_WM_NAME", &WindowSystemX::ATOM_NET_WM_NAME},
    {"_NET_WM_PID", &WindowSystemX::ATOM_NET_WM_PID},
    {"_NET_WM_WINDOW_TYPE", &WindowSystemX::ATOM_NET_WM_WINDOW_TYPE},
    {"_NET_WM_WINDOW_TYPE_NORMAL", &WindowSystemX::ATOM_NET_WM_WINDOW_TYPE_NORMAL},
};

const int NUMBER_OF_ATOMS = sizeof(atomItems) / sizeof(AtomItem);

struct DeleteWindowRequest
{
  ::Window* window;
};

void ConfigureNotifyEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11ConfigureNotifyEvent configureNotify;
  configureNotify.window = xevent->xconfigure.window;
  configureNotify.event  = xevent;

  configureNotify.x      = xevent->xconfigure.x;
  configureNotify.y      = xevent->xconfigure.y;
  configureNotify.width  = xevent->xconfigure.width;
  configureNotify.height = xevent->xconfigure.height;

  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::CONFIGURE_NOTIFY, configureNotify);
}

void PropertyNotifyEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11PropertyNotifyEvent propertyNotifyEvent;
  propertyNotifyEvent.window    = xevent->xproperty.window;
  propertyNotifyEvent.event     = xevent;
  propertyNotifyEvent.timestamp = xevent->xproperty.time;
  propertyNotifyEvent.atom      = xevent->xproperty.atom;
  propertyNotifyEvent.state     = xevent->xproperty.state;
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::PROPERTY_NOTIFY, propertyNotifyEvent);
}

void ClientMessageEventHandler(const XEvent* xevent)
{
  if((xevent->xclient.message_type == WindowSystemX::ATOM_WM_PROTOCOLS) &&
     (xevent->xclient.format == 32) &&
     (static_cast<Atom>(xevent->xclient.data.l[0]) == WindowSystemX::ATOM_WM_DELETE_WINDOW))
  {
    WindowSystemX::X11Event x11Event;
    x11Event.window = xevent->xclient.window;
    x11Event.event  = xevent;

    GetImplementation().TriggerEventHandler(WindowSystemBase::Event::DELETE_REQUEST, x11Event);
  }
}

void FocusInEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11Event x11Event;
  x11Event.window = xevent->xclient.window;
  x11Event.event  = xevent;
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::FOCUS_IN, x11Event);
}

void FocusOutEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11Event x11Event;
  x11Event.window = xevent->xclient.window;
  x11Event.event  = xevent;
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::FOCUS_OUT, x11Event);
}

void ExposeEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11ExposeEvent x11ExposeEvent;
  x11ExposeEvent.window = xevent->xclient.window;
  x11ExposeEvent.event  = xevent;
  x11ExposeEvent.x      = xevent->xexpose.x;
  x11ExposeEvent.y      = xevent->xexpose.y;
  x11ExposeEvent.width  = xevent->xexpose.width;
  x11ExposeEvent.height = xevent->xexpose.height;
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::DAMAGE, x11ExposeEvent);
}

void HandlePointerMove(int x, int y, unsigned long timestamp, ::Window window)
{
  WindowSystemX::X11MouseEvent mouseEvent;
  mouseEvent.window         = window;
  mouseEvent.timestamp      = timestamp;
  mouseEvent.x              = x;
  mouseEvent.y              = y;
  mouseEvent.buttons        = 0;
  mouseEvent.device         = 0;
  mouseEvent.multi.pressure = 1.0f;
  mouseEvent.multi.angle    = 0.0f;
  mouseEvent.multi.radius   = 1;
  mouseEvent.multi.radiusX  = 1;
  mouseEvent.multi.radiusY  = 1;
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::MOUSE_MOVE, mouseEvent);
}

void ConvertButtonEvent(const XEvent* xevent, WindowSystemX::X11MouseEvent& mouseEvent)
{
  mouseEvent.window         = xevent->xbutton.subwindow ? xevent->xbutton.subwindow : xevent->xbutton.window;
  mouseEvent.timestamp      = xevent->xbutton.time;
  mouseEvent.x              = xevent->xbutton.x;
  mouseEvent.y              = xevent->xbutton.y;
  mouseEvent.buttons        = xevent->xbutton.button;
  mouseEvent.device         = 0;
  mouseEvent.multi.pressure = 1.0f;
  mouseEvent.multi.angle    = 0.0f;
  mouseEvent.multi.radius   = 1;
  mouseEvent.multi.radiusX  = 1;
  mouseEvent.multi.radiusY  = 1;
}

void ButtonPressEventHandler(const XEvent* xevent)
{
  if(xevent->xbutton.button < MOUSE_SCROLL_WHEEL_UP || xevent->xbutton.button > MOUSE_SCROLL_WHEEL_RIGHT)
  {
    HandlePointerMove(xevent->xbutton.x, xevent->xbutton.y, xevent->xbutton.time, xevent->xbutton.subwindow ? xevent->xbutton.subwindow : xevent->xbutton.window);

    WindowSystemX::X11MouseEvent mouseEvent;
    ConvertButtonEvent(xevent, mouseEvent);
    GetImplementation().TriggerEventHandler(WindowSystemBase::Event::MOUSE_BUTTON_DOWN, mouseEvent);
  }
  else // Otherwise, it's a mouse wheel event
  {
    WindowSystemX::X11MouseWheelEvent mouseWheelEvent;
    mouseWheelEvent.x         = xevent->xbutton.x;
    mouseWheelEvent.y         = xevent->xbutton.y;
    mouseWheelEvent.window    = (xevent->xbutton.subwindow ? xevent->xbutton.subwindow : xevent->xbutton.window);
    mouseWheelEvent.timestamp = xevent->xbutton.time;

    switch(xevent->xbutton.button)
    {
      case MOUSE_SCROLL_WHEEL_UP:
      {
        mouseWheelEvent.direction = 0;
        mouseWheelEvent.z         = -1;
        break;
      }
      case MOUSE_SCROLL_WHEEL_DOWN:
      {
        mouseWheelEvent.direction = 0;
        mouseWheelEvent.z         = 1;
        break;
      }
      case MOUSE_SCROLL_WHEEL_LEFT:
      {
        mouseWheelEvent.direction = 1;
        mouseWheelEvent.z         = -1;
        break;
      }
      case MOUSE_SCROLL_WHEEL_RIGHT:
      {
        mouseWheelEvent.direction = 1;
        mouseWheelEvent.z         = 1;
        break;
      }
    }
    GetImplementation().TriggerEventHandler(WindowSystemBase::Event::MOUSE_WHEEL, mouseWheelEvent);
  }
}

void ButtonReleaseEventHandler(const XEvent* xevent)
{
  // Check it's a normal button, not a mouse wheel button
  if(xevent->xbutton.button < MOUSE_SCROLL_WHEEL_UP || xevent->xbutton.button > MOUSE_SCROLL_WHEEL_RIGHT)
  {
    HandlePointerMove(xevent->xbutton.x, xevent->xbutton.y, xevent->xbutton.time, xevent->xbutton.subwindow ? xevent->xbutton.subwindow : xevent->xbutton.window);

    WindowSystemX::X11MouseEvent mouseEvent;
    ConvertButtonEvent(xevent, mouseEvent);
    GetImplementation().TriggerEventHandler(WindowSystemBase::Event::MOUSE_BUTTON_UP, mouseEvent);
  }
  // ignore wheel release events, they are sent immediately prior to another press event
}

void MotionNotifyEventHandler(const XEvent* xevent)
{
  HandlePointerMove(xevent->xmotion.x, xevent->xmotion.y, xevent->xmotion.time, xevent->xmotion.subwindow ? xevent->xmotion.subwindow : xevent->xmotion.window);
}

void EnterNotifyEventHandler(const XEvent* xevent)
{
  HandlePointerMove(xevent->xcrossing.x, xevent->xcrossing.y, xevent->xcrossing.time, xevent->xcrossing.subwindow ? xevent->xcrossing.subwindow : xevent->xcrossing.window);
}
void LeaveNotifyEventHandler(const XEvent* xevent)
{
  HandlePointerMove(xevent->xcrossing.x, xevent->xcrossing.y, xevent->xcrossing.time, xevent->xcrossing.subwindow ? xevent->xcrossing.subwindow : xevent->xcrossing.window);
}

void ConvertKeyEvent(const XEvent* xEvent, WindowSystemX::X11KeyEvent& keyEvent, bool down)
{
  const XKeyEvent* xKeyEvent = &(xEvent->xkey);

  keyEvent.keyCode = xKeyEvent->keycode;

  KeySym keySymbol = XkbKeycodeToKeysym(xKeyEvent->display, xKeyEvent->keycode, 0, 0);
  char*  keyname   = XKeysymToString(keySymbol);
  if(!keyname)
  {
    asprintf(&keyname, "Keycode-%i", xKeyEvent->keycode);
    keyEvent.keyname = keyname;
    free(keyname);
  }
  else
  {
    keyEvent.keyname = keyname;
  }

  static int modifiers = 0;
  switch(keySymbol)
  {
    case XK_Shift_L:
    {
      modifiers &= 0xFE;
      modifiers |= down ? 0x01 : 0;
      break;
    }
    case XK_Shift_R:
    {
      modifiers &= 0xFD;
      modifiers |= down ? 0x02 : 0;
      break;
    }
    case XK_Control_L:
    {
      modifiers &= 0xFB;
      modifiers |= down ? 0x04 : 0;
      break;
    }
    case XK_Control_R:
    {
      modifiers &= 0xF7;
      modifiers |= down ? 0x08 : 0;
      break;
    }
    case XK_Alt_L:
    {
      modifiers &= 0xEF;
      modifiers |= down ? 0x10 : 0;
      break;
    }
    case XK_Alt_R:
    {
      modifiers &= 0xDF;
      modifiers |= down ? 0x20 : 0;
      break;
    }
    case XK_ISO_Level3_Shift:
    {
      modifiers &= 0xDF;
      modifiers |= down ? 0x20 : 0;
      break;
    } // Alt Graphics
  }
  int shiftModifier   = (modifiers & 0x03) != 0;
  int controlModifier = (modifiers & 0x0C) != 0;
  int altModifier     = (modifiers & 0x30) != 0;
  keyEvent.timestamp  = xKeyEvent->time;
  keyEvent.modifiers  = shiftModifier | controlModifier << 1 | altModifier << 2;
  keyEvent.window     = xKeyEvent->window;
  keyEvent.event      = xEvent;

  KeySym         keySymbol2;
  XComposeStatus composeStatus;
  const int      BUFFER_LENGTH{256};
  char           buffer[BUFFER_LENGTH];
  int            stringLength = XLookupString(const_cast<XKeyEvent*>(xKeyEvent), buffer, BUFFER_LENGTH, &keySymbol2, &composeStatus);

  const char* key{nullptr};
  if(keySymbol != keySymbol2)
  {
    key = XKeysymToString(keySymbol2);
  }
  if(!key)
  {
    key = keyEvent.keyname.c_str();
  }
  keyEvent.key = key;

  buffer[std::max(0, std::min(BUFFER_LENGTH - 1, stringLength))] = '\0';

  keyEvent.compose = stringLength ? buffer : "";
}

void KeyPressEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11KeyEvent x11KeyEvent;
  ConvertKeyEvent(xevent, x11KeyEvent, true);
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::KEY_DOWN, x11KeyEvent);
}

void KeyReleaseEventHandler(const XEvent* xevent)
{
  WindowSystemX::X11KeyEvent x11KeyEvent;
  ConvertKeyEvent(xevent, x11KeyEvent, false);
  GetImplementation().TriggerEventHandler(WindowSystemBase::Event::KEY_UP, x11KeyEvent);
}

void SelectionClearEventHandler(const XEvent* xevent)
{
  // NOT IMPLEMENTED
}

void SelectionNotifyEventHandler(const XEvent* xevent)
{
  // NOT IMPLEMENTED
}

struct WindowSystemX::Impl
{
  Impl()
  {
    XInitThreads();
    mDisplay = XOpenDisplay(nullptr); // Note, DisplayConnection now reads this var.

    mXEventMonitor = new FileDescriptorMonitor(
      ConnectionNumber(mDisplay),
      MakeCallback(this, &WindowSystemX::Impl::XPollCallback),
      FileDescriptorMonitor::FD_READABLE|FileDescriptorMonitor::FD_WRITABLE);
    // libuv hacks: add FD_WRITABLE.

    InitializeAtoms();
    SetupEventHandlers();
    InitializeInput();
  }

  ~Impl()
  {
    // @todo Flush events, delete fd handlers, shutdown other X subsystems
    XCloseDisplay(mDisplay);
    delete mXEventMonitor;
  }

  ::Display* GetXDisplay()
  {
    return mDisplay;
  }

  void XPollCallback(FileDescriptorMonitor::EventType eventType)
  {
    if(eventType & (FileDescriptorMonitor::FD_READABLE | FileDescriptorMonitor::FD_WRITABLE))
    {
      while(XPending(mDisplay))
      {
        XEvent event;
        XNextEvent(mDisplay, &event);
        HandleXEvent(event);
      }
    }
  }

  void InitializeAtoms()
  {
    std::vector<Atom>        atoms;
    std::vector<const char*> names;
    atoms.resize(NUMBER_OF_ATOMS);
    names.resize(NUMBER_OF_ATOMS);
    for(unsigned int i = 0; i < NUMBER_OF_ATOMS; ++i)
    {
      names[i] = atomItems[i].name.c_str();
    }
    Status status = XInternAtoms(mDisplay, const_cast<char**>(&names[0]), NUMBER_OF_ATOMS, false, &atoms[0]);
    if(status != 0)
    {
      for(unsigned int i = 0; i < NUMBER_OF_ATOMS; ++i)
      {
        *(atomItems[i].atom) = atoms[i];
      }
    }
  }

  void SetupEventHandlers()
  {
    mXEventHandlers[PropertyNotify]  = &PropertyNotifyEventHandler;
    mXEventHandlers[ClientMessage]   = &ClientMessageEventHandler;
    mXEventHandlers[FocusIn]         = &FocusInEventHandler;
    mXEventHandlers[FocusOut]        = &FocusOutEventHandler;
    mXEventHandlers[Expose]          = &ExposeEventHandler;
    mXEventHandlers[ButtonPress]     = &ButtonPressEventHandler;
    mXEventHandlers[ButtonRelease]   = &ButtonReleaseEventHandler;
    mXEventHandlers[MotionNotify]    = &MotionNotifyEventHandler;
    mXEventHandlers[EnterNotify]     = &EnterNotifyEventHandler;
    mXEventHandlers[LeaveNotify]     = &LeaveNotifyEventHandler;
    mXEventHandlers[KeyPress]        = &KeyPressEventHandler;
    mXEventHandlers[KeyRelease]      = &KeyReleaseEventHandler;
    mXEventHandlers[SelectionClear]  = &SelectionClearEventHandler;
    mXEventHandlers[SelectionNotify] = &SelectionNotifyEventHandler;
    mXEventHandlers[ConfigureNotify] = &ConfigureNotifyEventHandler;
  }

  void InitializeInput()
  {
    int event;
    int error;
    if(XQueryExtension(mDisplay, "XInputExtension", &mXi2OpCode, &event, &error))
    {
      // Extension is present
      int majorVersion = XI_2_Major;
      int minorVersion = XI_2_Minor;

      Status status = XIQueryVersion(mDisplay, &majorVersion, &minorVersion);
      if(status == Success)
      {
        // @todo May need to enable DeviceChanged, HierarchyChanged and PropertyEvent masks.
        mXi2Devices = XIQueryDevice(mDisplay, XIAllDevices, &mXi2NumberOfDevices);
      }
    }
  }

  void ShutdownInput()
  {
    // @todo Clear any events set in InitializeInput()
    if(mXi2Devices)
    {
      XIFreeDeviceInfo(mXi2Devices);
      mXi2Devices = nullptr;
    }
    mXi2NumberOfDevices = 0;
    mXi2OpCode          = -1;
  }

  void InputMultiSelect(::Window window)
  {
    // NOT IMPLEMENTED. See ecore_x_input_multi_select(mEcoreWindow);
  }
  void EnableDragAndDrop(::Window window, bool enable)
  {
    // NOT IMPLEMENTED. See ecore_x_dnd_aware_set(mEcoreWindow, enable);
  }

  // Call the internal X11 event handler. This will call TriggerEventHandler which
  // will call each registered handler's callback.
  void HandleXEvent(const XEvent& event)
  {
    auto iter = mXEventHandlers.find(event.type);
    if(iter != mXEventHandlers.end())
    {
      iter->second(&event);
    }
  }

  WindowSystemBase::EventHandler* AddEventHandler(WindowSystemBase::Event                event,
                                                  WindowSystemBase::EventHandlerCallback callback,
                                                  void*                                  data)
  {
    mHandlers.emplace_back(EventHandler{callback, data, event, ++mNextHandlerId});
    return &mHandlers.back();
  }

  void DeleteEventHandler(WindowSystemBase::EventHandler* eventHandler)
  {
    int  id   = eventHandler->handlerId;
    auto iter = std::find_if(mHandlers.begin(), mHandlers.end(), [id](const WindowSystemBase::EventHandler& eventHandler) { return eventHandler.handlerId == id; });
    if(iter != mHandlers.end())
    {
      mHandlers.erase(iter);
    }
  }

  void TriggerEventHandler(WindowSystemBase::Event eventType, WindowSystemX::X11Event& x11Event)
  {
    //@todo make this much more efficient!
    for(auto& element : mHandlers)
    {
      if(element.event == eventType)
      {
        bool stop = element.callback(element.data, eventType, &x11Event);
        if(stop)
        {
          break;
        }
      }
    }
  }

  void Move(::Window window, int x, int y)
  {
    XMoveWindow(mDisplay, window, x, y);
  }
  void Resize(::Window window, int width, int height)
  {
    XResizeWindow(mDisplay, window, std::max(1, width), std::max(1, height));
  }

  void MoveResize(::Window window, int x, int y, int width, int height)
  {
    XMoveResizeWindow(mDisplay, window, x, y, std::max(1, width), std::max(1, height));
  }

  void SetStringProperty(::Window window, Atom atom, const std::string& string)
  {
    XChangeProperty(mDisplay, window, atom, WindowSystemX::ATOM_UTF8_STRING, 8, PropModeReplace, (unsigned char*)string.c_str(), string.length());
  }

  void SetClass(::Window window, const std::string& name, const std::string& className)
  {
    char*         list[] = {strdup(name.c_str())};
    XTextProperty textProperty;
    if(Xutf8TextListToTextProperty(mDisplay, list, 1, XUTF8StringStyle, &textProperty) >= Success)
    {
      XSetWMName(mDisplay, window, &textProperty);
      if(textProperty.value)
      {
        XFree(textProperty.value);
      }
    }

    SetStringProperty(window, WindowSystemX::ATOM_NET_WM_NAME, name);

    XClassHint* classHint = XAllocClassHint();
    classHint->res_name   = list[0];
    classHint->res_class  = strdup(className.c_str());
    XSetClassHint(mDisplay, window, classHint);
    free(classHint->res_class);
    XFree(classHint);
    free(list[0]);
  }

  ::Display* mDisplay;
  int        mNextHandlerId{0};
  using EventHandlerFunctionPointer = void (*)(const XEvent*);
  std::unordered_map<int, EventHandlerFunctionPointer> mXEventHandlers;
  std::vector<WindowSystemBase::EventHandler>          mHandlers;
  FileDescriptorMonitor*                               mXEventMonitor;
  XIDeviceInfo*                                        mXi2Devices{nullptr};
  int                                                  mXi2NumberOfDevices{0};
  int                                                  mXi2OpCode{-1};
};

WindowSystemX::WindowSystemX()
{
  mImpl = new Impl();
}

WindowSystemX::~WindowSystemX()
{
  delete mImpl;
}

Dali::Any WindowSystemX::GetDisplay()
{
  return Dali::Any(mImpl->mDisplay);
}

::Display* WindowSystemX::GetXDisplay()
{
  return mImpl->mDisplay;
}

void WindowSystemX::Sync()
{
  XSync(mImpl->mDisplay, false);
}

void WindowSystemX::GetScreenSize(int& width, int& height)
{
  ::Screen* screen = DefaultScreenOfDisplay(mImpl->mDisplay);
  if(screen != nullptr)
  {
    width  = screen->width;
    height = screen->height;
  }
}

WindowSystemBase::EventHandler* WindowSystemX::AddEventHandler(Event event, EventHandlerCallback callback, void* data)
{
  return mImpl->AddEventHandler(event, callback, data);
}

void WindowSystemX::DeleteEventHandler(WindowSystemBase::EventHandler* eventHandler)
{
  mImpl->DeleteEventHandler(eventHandler);
}

::Window WindowSystemX::CreateWindow(int depth, int x, int y, int width, int height)
{
  ::Window window;
  ::Window parent = DefaultRootWindow(mImpl->mDisplay);

  XSetWindowAttributes attributes;
  attributes.background_pixmap     = None;             /* background, None, or ParentRelative */
  attributes.border_pixel          = 0;                /* border pixel value */
  attributes.bit_gravity           = NorthWestGravity; /* one of bit gravity values */
  attributes.win_gravity           = NorthWestGravity; /* one of the window gravity values */
  attributes.backing_store         = NotUseful;        /* NotUseful, WhenMapped, Always */
  attributes.save_under            = false;            /* should bits under be saved? (popups) */
  attributes.event_mask            = (KeyPressMask |   /* set of events that should be saved */
                           KeyReleaseMask |
                           ButtonPressMask |
                           ButtonReleaseMask |
                           EnterWindowMask |
                           LeaveWindowMask |
                           PointerMotionMask |
                           StructureNotifyMask |
                           ExposureMask |
                           VisibilityChangeMask |
                           StructureNotifyMask |
                           FocusChangeMask |
                           PropertyChangeMask |
                           ColormapChangeMask);
  attributes.do_not_propagate_mask = NoEventMask; /* set of events that should not propagate */
  attributes.override_redirect     = false;       /* boolean value for override_redirect */
  attributes.cursor                = None;        /* cursor to be displayed (or None) */

  if(depth == 4)
  {
    Visual* visual{nullptr};

    XVisualInfo visualInfoRequest;
    visualInfoRequest.screen  = DefaultScreen(mImpl->mDisplay);
    visualInfoRequest.depth   = 32;
    visualInfoRequest.c_class = TrueColor;
    int          visualInfoCount;
    XVisualInfo* visualInfoList = XGetVisualInfo(mImpl->mDisplay,
                                                 (VisualScreenMask | VisualDepthMask | VisualClassMask),
                                                 &visualInfoRequest,
                                                 &visualInfoCount);
    for(int i = 0; i < visualInfoCount; ++i)
    {
      XRenderPictFormat* pictFormat = XRenderFindVisualFormat(mImpl->mDisplay, visualInfoList[i].visual);
      Sync();
      if(pictFormat->type == PictTypeDirect && pictFormat->direct.alphaMask)
      {
        visual = visualInfoList[i].visual;
        break;
      }
    }
    XFree(visualInfoList);

    attributes.colormap = XCreateColormap(mImpl->mDisplay, parent, visual, AllocNone); /* color map to be associated with window */

    window = XCreateWindow(mImpl->mDisplay, parent, x, y, width, height, 0, 32, InputOutput, visual, (CWBackingStore | CWOverrideRedirect | CWColormap | CWBorderPixel | CWBackPixmap | CWSaveUnder | CWDontPropagate | CWEventMask | CWBitGravity | CWWinGravity), &attributes);
  }
  else
  {
    window = XCreateWindow(mImpl->mDisplay, parent, x, y, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, (CWBackingStore | CWOverrideRedirect | CWBorderPixel | CWBackPixmap | CWSaveUnder | CWDontPropagate | CWEventMask | CWBitGravity | CWWinGravity), &attributes);
  }

  Sync();
  SetWindowDefaults(window);
  return window;
}

void WindowSystemX::SetWindowDefaults(::Window window)
{
  char hostnameBuffer[HOST_NAME_MAX + 1];
  gethostname(hostnameBuffer, HOST_NAME_MAX);
  hostnameBuffer[HOST_NAME_MAX] = '\0';
  char* hostname[1];
  hostname[0] = hostnameBuffer;

  XTextProperty xTextProperty;

  if(XStringListToTextProperty(hostname, 1, &xTextProperty))
  {
    XSetWMClientMachine(mImpl->mDisplay, window, &xTextProperty);
    XFree(xTextProperty.value);
  }
  Sync();
  long pid = getpid();
  XChangeProperty(mImpl->mDisplay, window, ATOM_NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (uint8_t*)&pid, 1);
  Sync();

  Atom atom = ATOM_NET_WM_WINDOW_TYPE_NORMAL;
  XChangeProperty(mImpl->mDisplay, window, ATOM_NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (uint8_t*)&atom, 1);

  //ecore_app_args_get(&argc, &argv);
  //ecore_x_icccm_command_set(win, argc, argv);
}

void WindowSystemX::SetTransientForHint(::Window window, ::Window forWindow)
{
  XSetTransientForHint(mImpl->mDisplay, window, forWindow);
  Sync();
}

void WindowSystemX::UnsetTransientFor(::Window window)
{
  XDeleteProperty(mImpl->mDisplay, window, WindowSystemX::ATOM_WM_TRANSIENT_FOR);
  Sync();
}

void WindowSystemX::SetProtocol(::Window window, Atom protocol, bool value)
{
  Atom* protocols{nullptr};
  int   protocolsCount = 0;

  Status status = XGetWMProtocols(mImpl->mDisplay, window, &protocols, &protocolsCount);

  if(status > 0)
  {
    XSync(mImpl->mDisplay, false);

    bool found = false;
    int  index = 0;
    for(; index < protocolsCount; ++index)
    {
      if(protocols[index] == protocol)
      {
        found = true;
        break;
      }
    }
    if(value && !found)
    {
      std::vector<Atom> newProtocols;
      newProtocols.resize(protocolsCount + 1);

      for(int i = 0; i < protocolsCount; ++i)
      {
        newProtocols[i] = protocols[i];
      }
      newProtocols.back() = protocol;

      XSetWMProtocols(mImpl->mDisplay, window, &newProtocols[0], newProtocols.size());
      Sync();
    }
    else if(!value && found)
    {
      // Remove the protocol
      --protocolsCount;
      if(protocolsCount > 0)
      {
        for(int i = index; i < protocolsCount; ++i)
        {
          protocols[i] = protocols[i + 1];
        }
        XSetWMProtocols(mImpl->mDisplay, window, protocols, protocolsCount);
      }
      else
      {
        XDeleteProperty(mImpl->mDisplay, window, WindowSystemX::ATOM_WM_PROTOCOLS);
      }
    }
    XFree(protocols);
  }
}

void WindowSystemX::SetWindowHints(::Window window, bool acceptsFocus)
{
  XWMHints* hints = XAllocWMHints();
  if(hints != nullptr)
  {
    hints->flags         = InputHint | StateHint;
    hints->input         = acceptsFocus;
    hints->initial_state = NormalState;
    XSetWMHints(mImpl->mDisplay, window, hints);
    XFree(hints);
  }
}

WindowSystemX::WindowState WindowSystemX::GetWindowState(::Window window)
{
  std::vector<unsigned long> hints;
  if(GetWindowProperty<unsigned long>(mImpl->mDisplay, window, WindowSystemX::ATOM_WM_STATE, WindowSystemX::ATOM_WM_STATE, hints))
  {
    if(hints.size() == 2)
    {
      switch(hints[0])
      {
        case WithdrawnState:
        {
          return WindowSystemX::WindowState::WITHDRAWN;
        }
        case NormalState:
        {
          return WindowSystemX::WindowState::NORMAL;
        }
        case IconicState:
        {
          return WindowSystemX::WindowState::ICONIC;
        }
      }
    }
  }
  return WindowSystemX::WindowState::NORMAL;
}

void WindowSystemX::Show(::Window window)
{
  XMapWindow(mImpl->mDisplay, window);
  Sync();
}

void WindowSystemX::Hide(::Window window)
{
  ::Window     rootWindow = window;
  int          x, y;
  unsigned int width, height, border, depth;
  if(ScreenCount(mImpl->mDisplay) == 1)
  {
    rootWindow = DefaultRootWindow(mImpl->mDisplay);
  }
  else
  {
    // Need to get the root window in a different way
    XGetGeometry(mImpl->mDisplay, window, &rootWindow, &x, &y, &width, &height, &border, &depth);
  }
  XUnmapWindow(mImpl->mDisplay, window);
  XEvent event;
  event.xunmap.type           = UnmapNotify;
  event.xunmap.serial         = 0;
  event.xunmap.send_event     = True;
  event.xunmap.display        = mImpl->mDisplay;
  event.xunmap.window         = window;
  event.xunmap.from_configure = False;

  XSendEvent(mImpl->mDisplay, rootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
  Sync();
}

void WindowSystemX::Activate(::Window window)
{
  XWindowAttributes attributes;
  Status            status = XGetWindowAttributes(mImpl->mDisplay, window, &attributes);
  ::Window          root   = (status > 0) ? attributes.root : DefaultRootWindow(mImpl->mDisplay);

  XEvent event;
  event.xclient.type         = ClientMessage;
  event.xclient.display      = mImpl->mDisplay;
  event.xclient.window       = window;
  event.xclient.message_type = WindowSystemX::ATOM_NET_ACTIVE_WINDOW;
  event.xclient.format       = 32;
  event.xclient.data.l[0]    = 1;
  event.xclient.data.l[1]    = CurrentTime;
  event.xclient.data.l[2]    = 0;
  event.xclient.data.l[3]    = 0;
  event.xclient.data.l[4]    = 0;
  XSendEvent(mImpl->mDisplay, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

void WindowSystemX::Raise(::Window window)
{
  XRaiseWindow(mImpl->mDisplay, window);
  Sync();
}

void WindowSystemX::Lower(::Window window)
{
  XLowerWindow(mImpl->mDisplay, window);
  Sync();
}

void WindowSystemX::TriggerEventHandler(WindowSystemBase::Event eventType, X11Event& event)
{
  mImpl->TriggerEventHandler(eventType, event);
}

void WindowSystemX::GetDPI(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  Screen* screen = DefaultScreenOfDisplay(mImpl->mDisplay);
  if(screen->mwidth <= 0)
  {
    dpiHorizontal = dpiVertical = 75;
  }
  else
  {
    dpiHorizontal = dpiVertical = (((screen->width * 254) / screen->mwidth) + 5) / 10;
  }
}

void WindowSystemX::Move(::Window window, int x, int y)
{
  mImpl->Move(window, x, y);
  Sync();
}

void WindowSystemX::Resize(::Window window, int width, int height)
{
  mImpl->Resize(window, width, height);
  Sync();
}

void WindowSystemX::MoveResize(::Window window, int x, int y, int width, int height)
{
  mImpl->MoveResize(window, x, y, width, height);
  Sync();
}

void WindowSystemX::SetStringProperty(::Window window, Atom atom, const std::string& string)
{
  mImpl->SetStringProperty(window, atom, string);
  Sync();
}

void WindowSystemX::SetClass(::Window window, const std::string& name, const std::string& className)
{
  mImpl->SetClass(window, name, className);
  Sync();
}

void WindowSystemX::InputMultiSelect(::Window window)
{
  mImpl->InputMultiSelect(window);
  Sync();
}

void WindowSystemX::EnableDragAndDrop(::Window window, bool enable)
{
  mImpl->EnableDragAndDrop(window, enable);
  Sync();
}

WindowSystemX& GetImplementation()
{
  if(nullptr != gWindowSystem)
  {
    Initialize();
  }
  return *gWindowSystem;
}

void GetScreenSize(int& width, int& height)
{
  if(gWindowSystem != nullptr)
  {
    gWindowSystem->GetScreenSize(width, height);
  }
}

void UpdateScreenSize()
{
}

bool SetKeyboardRepeatInfo(float rate, float delay)
{
  return false;
}

bool GetKeyboardRepeatInfo(float& rate, float& delay)
{
  return false;
}

bool SetKeyboardHorizontalRepeatInfo(float rate, float delay)
{
  return false;
}

bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay)
{
  return false;
}

bool SetKeyboardVerticalRepeatInfo(float rate, float delay)
{
  return false;
}

bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay)
{
  return false;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
