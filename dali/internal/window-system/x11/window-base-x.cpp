/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/x11/window-base-x.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/x11/window-system-x.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/public-api/object/any.h>

#if !defined(VULKAN_ENABLED)
#include <dali/internal/graphics/gles/egl-implementation.h>
#endif

using Dali::Internal::Adaptor::WindowSystem::WindowSystemX;

namespace Dali::Internal::Adaptor
{
namespace
{
const char*                  DEFAULT_DEVICE_NAME     = "";
const Device::Class::Type    DEFAULT_DEVICE_CLASS    = Device::Class::NONE;
const Device::Subclass::Type DEFAULT_DEVICE_SUBCLASS = Device::Subclass::NONE;

const unsigned int PRIMARY_TOUCH_BUTTON_ID(1);

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW_BASE");
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

bool EventWindowConfigureNotify(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnConfigure(event);
  }

  return false;
}

bool EventWindowPropertyChanged(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    return windowBase->OnWindowPropertyChanged(data, type, event);
  }

  return false;
}

/**
 * Called when the window receives a delete request
 */
bool EventWindowDeleteRequest(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnDeleteRequest();
  }
  return true;
}

/**
 * Called when the window gains focus.
 */
bool EventWindowFocusIn(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnFocusIn(data, type, event);
  }
  return false;
}

/**
 * Called when the window loses focus.
 */
static bool EventWindowFocusOut(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnFocusOut(data, type, event);
  }
  return false;
}

/**
 * Called when the window is damaged.
 */
static bool EventWindowDamaged(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnWindowDamaged(data, type, event);
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Selection Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the source window notifies us the content in clipboard is selected.
 */
bool EventSelectionClear(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnSelectionClear(data, type, event);
  }
  return false;
}

/**
 * Called when the source window sends us about the selected content.
 * For example, when dragged items are dragged INTO our window or when items are selected in the clipboard.
 */
bool EventSelectionNotify(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnSelectionNotify(data, type, event);
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a touch down is received.
 */
bool EventMouseButtonDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonDown(data, type, event);
  }
  return false;
}

/**
 * Called when a touch up is received.
 */
bool EventMouseButtonUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonUp(data, type, event);
  }
  return false;
}

/**
 * Called when a touch motion is received.
 */
bool EventMouseButtonMove(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonMove(data, type, event);
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Wheel Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a mouse wheel is received.
 */
bool EventMouseWheel(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnMouseWheel(data, type, event);
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Key Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a key down is received.
 */
bool EventKeyDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnKeyDown(data, type, event);
  }
  return false;
}

/**
 * Called when a key up is received.
 */
bool EventKeyUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnKeyUp(data, type, event);
  }
  return false;
}

} // unnamed namespace

WindowBaseX::WindowBaseX(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mEventHandlers(),
  mWindow(0),
  mOwnSurface(false),
  mIsTransparent(false), // Should only be set to true once we actually create a transparent window regardless of what isTransparent is.
  mRotationAppSet(false),
  mWindowRotationAngle(0)
{
  Initialize(positionSize, surface, isTransparent);
}

WindowBaseX::~WindowBaseX()
{
  DeleteEvents();
  mEventHandlers.Clear();

  if(mOwnSurface)
  {
    XDestroyWindow(WindowSystem::GetImplementation().GetXDisplay(), mWindow);
  }
}

void WindowBaseX::Initialize(PositionSize positionSize, Any surface, bool isTransparent)
{
  // see if there is a surface in Any surface
  unsigned int surfaceId = GetSurfaceId(surface);

  // if the surface is empty, create a new one.
  if(surfaceId == 0)
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateInternalWindow(positionSize, isTransparent);
  }
  else
  {
    // XLib should already be initialized so no point in calling XInitThreads
    mWindow = static_cast<::Window>(surfaceId);
  }

  auto& windowSystem = WindowSystem::GetImplementation();

  char* id = NULL;
  if((id = getenv("DESKTOP_STARTUP_ID")))
  {
    windowSystem.SetStringProperty(mWindow, WindowSystemX::ATOM_NET_STARTUP_ID, std::string(id));
  }

  windowSystem.SetWindowHints(mWindow, true);
  windowSystem.Sync();

  EnableMultipleSelection();
  EnableWindowClose();
  EnableDragAndDrop();

  SetupEvents();
}

void WindowBaseX::EnableMultipleSelection() const
{
  WindowSystem::GetImplementation().InputMultiSelect(mWindow);
}

void WindowBaseX::EnableWindowClose() const
{
  WindowSystem::GetImplementation().SetProtocol(mWindow, WindowSystemX::ATOM_WM_DELETE_WINDOW, true);
}

void WindowBaseX::EnableDragAndDrop() const
{
  WindowSystem::GetImplementation().EnableDragAndDrop(mWindow, true);
}

void WindowBaseX::SetupEvents()
{
  auto& windowSystem = WindowSystem::GetImplementation();
  windowSystem.AddEventHandler(WindowSystemBase::Event::CONFIGURE_NOTIFY, EventWindowConfigureNotify, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::PROPERTY_NOTIFY, EventWindowPropertyChanged, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::DELETE_REQUEST, EventWindowDeleteRequest, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::FOCUS_IN, EventWindowFocusIn, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::FOCUS_OUT, EventWindowFocusOut, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::DAMAGE, EventWindowDamaged, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::MOUSE_BUTTON_DOWN, EventMouseButtonDown, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::MOUSE_BUTTON_UP, EventMouseButtonUp, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::MOUSE_OUT, EventMouseButtonUp, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::MOUSE_MOVE, EventMouseButtonMove, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::MOUSE_WHEEL, EventMouseWheel, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::KEY_DOWN, EventKeyDown, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::KEY_UP, EventKeyUp, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::SELECTION_CLEAR, EventSelectionClear, this);
  windowSystem.AddEventHandler(WindowSystemBase::Event::SELECTION_NOTIFY, EventSelectionNotify, this);
}

void WindowBaseX::DeleteEvents()
{
}

bool WindowBaseX::OnWindowPropertyChanged(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  bool handled(false);

  auto propertyNotifyEvent = static_cast<WindowSystem::WindowSystemX::X11PropertyNotifyEvent*>(event);
  if(propertyNotifyEvent->window == mWindow)
  {
    WindowSystemX::WindowState state = WindowSystem::GetImplementation().GetWindowState(mWindow);

    switch(state)
    {
      case WindowSystemX::WindowState::WITHDRAWN:
      {
        // Window was hidden.
        mIconifyChangedSignal.Emit(true);
        handled = true;
        break;
      }
      case WindowSystemX::WindowState::ICONIC:
      {
        // Window was iconified (minimised).
        mIconifyChangedSignal.Emit(true);
        handled = true;
        break;
      }
      case WindowSystemX::WindowState::NORMAL:
      {
        // Window was shown.
        mIconifyChangedSignal.Emit(false);
        handled = true;
        break;
      }
      default:
      {
        // Ignore
        break;
      }
    }
  }

  return handled;
}

void WindowBaseX::OnConfigure(WindowSystemBase::EventBase* event)
{
  auto configureEvent = static_cast<WindowSystemX::X11ConfigureNotifyEvent*>(event);
  if(configureEvent->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::OnConfigureNotify\n");
    Dali::PositionSize positionSize;
    positionSize.x      = configureEvent->x;
    positionSize.y      = configureEvent->y;
    positionSize.width  = configureEvent->width;
    positionSize.height = configureEvent->height;
  }
}

void WindowBaseX::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void WindowBaseX::OnFocusIn(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event)
{
  auto x11Event = static_cast<WindowSystemX::X11Event*>(event);

  if(x11Event->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::OnFocusIn\n");

    mFocusChangedSignal.Emit(true);
  }
}

void WindowBaseX::OnFocusOut(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event)
{
  auto x11Event = static_cast<WindowSystemX::X11Event*>(event);
  // If the window loses focus then hide the keyboard.
  if(x11Event->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::FocusOut\n");

    mFocusChangedSignal.Emit(false);
  }
}

void WindowBaseX::OnWindowDamaged(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event)
{
  auto windowExposeEvent = static_cast<WindowSystemX::X11ExposeEvent*>(event);
  if(windowExposeEvent->window == mWindow)
  {
    DamageArea area;
    area.x      = windowExposeEvent->x;
    area.y      = windowExposeEvent->y;
    area.width  = windowExposeEvent->width;
    area.height = windowExposeEvent->height;

    mWindowDamagedSignal.Emit(area);
  }
}

void WindowBaseX::OnMouseButtonDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto touchEvent = static_cast<WindowSystemX::X11MouseEvent*>(event);

  if(touchEvent->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::ButtonDown\n");
    PointState::Type state(PointState::DOWN);

    Integration::Point point;
    point.SetDeviceId(touchEvent->device);
    point.SetState(state);
    point.SetScreenPosition(Vector2(touchEvent->x, touchEvent->y));
    point.SetRadius(touchEvent->multi.radius, Vector2(touchEvent->multi.radiusX, touchEvent->multi.radiusY));
    point.SetPressure(touchEvent->multi.pressure);
    point.SetAngle(Degree(touchEvent->multi.angle));
    if(touchEvent->buttons)
    {
      point.SetMouseButton(static_cast<MouseButton::Type>(touchEvent->buttons));
    }

    mTouchEventSignal.Emit(point, touchEvent->timestamp);
  }
}

void WindowBaseX::OnMouseButtonUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto touchEvent = static_cast<WindowSystemX::X11MouseEvent*>(event);

  if(touchEvent->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::ButtonUp\n");
    Integration::Point point;
    point.SetDeviceId(touchEvent->device);
    point.SetState(PointState::UP);
    point.SetScreenPosition(Vector2(touchEvent->x, touchEvent->y));
    point.SetRadius(touchEvent->multi.radius, Vector2(touchEvent->multi.radiusX, touchEvent->multi.radiusY));
    point.SetPressure(touchEvent->multi.pressure);
    point.SetAngle(Degree(static_cast<float>(touchEvent->multi.angle)));
    if(touchEvent->buttons)
    {
      point.SetMouseButton(static_cast<MouseButton::Type>(touchEvent->buttons));
    }

    mTouchEventSignal.Emit(point, touchEvent->timestamp);
  }
}

void WindowBaseX::OnMouseButtonMove(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto touchEvent = static_cast<WindowSystemX::X11MouseEvent*>(event);

  if(touchEvent->window == mWindow)
  {
    Integration::Point point;
    point.SetDeviceId(touchEvent->device);
    point.SetState(PointState::MOTION);
    point.SetScreenPosition(Vector2(static_cast<float>(touchEvent->x), static_cast<float>(touchEvent->y)));
    point.SetRadius(static_cast<float>(touchEvent->multi.radius), Vector2(static_cast<float>(touchEvent->multi.radiusX), static_cast<float>(touchEvent->multi.radiusY)));
    point.SetPressure(static_cast<float>(touchEvent->multi.pressure));
    point.SetAngle(Degree(static_cast<float>(touchEvent->multi.angle)));

    mTouchEventSignal.Emit(point, touchEvent->timestamp);
  }
}

void WindowBaseX::OnMouseWheel(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto mouseWheelEvent = static_cast<WindowSystemX::X11MouseWheelEvent*>(event);

  if(mouseWheelEvent->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseX::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z);

    WheelEvent wheelEvent(WheelEvent::MOUSE_WHEEL,
                          mouseWheelEvent->direction,
                          mouseWheelEvent->modifiers,
                          Vector2(static_cast<float>(mouseWheelEvent->x),
                                  static_cast<float>(mouseWheelEvent->y)),
                          mouseWheelEvent->z, mouseWheelEvent->timestamp);

    mWheelEventSignal.Emit(wheelEvent);
  }
}

Integration::KeyEvent WindowBaseX::CreateKeyEvent(WindowSystemX::X11KeyEvent* keyEvent, Integration::KeyEvent::State state)
{
  std::string keyName(keyEvent->keyname);
  std::string logicalKey("");
  std::string keyString("");
  std::string compose("");

  // Ensure key compose string is not NULL as keys like SHIFT or arrow have a null string.
  if(!keyEvent->compose.empty())
  {
    compose   = keyEvent->compose;
    keyString = keyEvent->compose;
  }
  // Ensure key symbol is not NULL as keys like SHIFT have a null string.
  if(!keyEvent->key.empty())
  {
    logicalKey = keyEvent->key;
  }

  int           keyCode = keyEvent->keyCode;
  int           modifier(keyEvent->modifiers);
  unsigned long time(keyEvent->timestamp);

  Integration::KeyEvent daliKeyEvent{keyName, logicalKey, keyString, keyCode, modifier, time, state, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS};
  return daliKeyEvent;
}

void WindowBaseX::OnKeyDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto keyEvent = static_cast<WindowSystemX::X11KeyEvent*>(event);

  if(keyEvent->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseX::OnKeyDown\n");

    auto daliKeyEvent = CreateKeyEvent(keyEvent, Integration::KeyEvent::Down);

    mKeyEventSignal.Emit(daliKeyEvent);
  }
}

void WindowBaseX::OnKeyUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto keyEvent = static_cast<WindowSystemX::X11KeyEvent*>(event);

  if(keyEvent->window == mWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, " WindowBaseX::OnKeyUp\n");

    auto daliKeyEvent = CreateKeyEvent(keyEvent, Integration::KeyEvent::Up);

    mKeyEventSignal.Emit(daliKeyEvent);
  }
}

void WindowBaseX::OnSelectionClear(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
}

void WindowBaseX::OnSelectionNotify(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
}

Any WindowBaseX::GetNativeWindow()
{
  return mWindow;
}

int WindowBaseX::GetNativeWindowId()
{
  return mWindow;
}

Dali::Any WindowBaseX::CreateWindow(int width, int height)
{
#if !defined(VULKAN_ENABLED)
  return reinterpret_cast<EGLNativeWindowType>(mWindow);
#endif
}

void WindowBaseX::DestroyWindow()
{
}

void WindowBaseX::SetWindowRotation(int angle)
{
}

void WindowBaseX::SetWindowBufferTransform(int angle)
{
}

void WindowBaseX::SetWindowTransform(int angle)
{
}

void WindowBaseX::ResizeWindow(PositionSize positionSize)
{
}

bool WindowBaseX::IsWindowRotationSupported()
{
  return false;
}

void WindowBaseX::Move(PositionSize positionSize)
{
  WindowSystem::GetImplementation().Move(mWindow, positionSize.x, positionSize.y);
}

void WindowBaseX::Resize(PositionSize positionSize)
{
  WindowSystem::GetImplementation().Resize(mWindow, positionSize.width, positionSize.height);
}

void WindowBaseX::MoveResize(PositionSize positionSize)
{
  WindowSystem::GetImplementation().MoveResize(mWindow, positionSize.x, positionSize.y, positionSize.width, positionSize.height);
}

void WindowBaseX::SetClass(const std::string& name, const std::string& className)
{
  WindowSystem::GetImplementation().SetClass(mWindow, name, className);
}

void WindowBaseX::Raise()
{
  WindowSystem::GetImplementation().Raise(mWindow);
}

void WindowBaseX::Lower()
{
  WindowSystem::GetImplementation().Lower(mWindow);
}

void WindowBaseX::Activate()
{
  WindowSystem::GetImplementation().Activate(mWindow);
}

void WindowBaseX::SetAcceptFocus(bool accept)
{
}

void WindowBaseX::Show()
{
  WindowSystem::GetImplementation().Show(mWindow);
}

void WindowBaseX::Hide()
{
  WindowSystem::GetImplementation().Hide(mWindow);
}

unsigned int WindowBaseX::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseX::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return std::string();
}

unsigned int WindowBaseX::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  return 0;
}

bool WindowBaseX::RemoveAuxiliaryHint(unsigned int id)
{
  return false;
}

bool WindowBaseX::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  return false;
}

std::string WindowBaseX::GetAuxiliaryHintValue(unsigned int id) const
{
  return std::string();
}

unsigned int WindowBaseX::GetAuxiliaryHintId(const std::string& hint) const
{
  return 0;
}

void WindowBaseX::SetInputRegion(const Rect<int>& inputRegion)
{
}

void WindowBaseX::SetType(Dali::Window::Type type)
{
}

Dali::Window::Type WindowBaseX::GetType() const
{
  return Dali::Window::Type::NORMAL;
}

bool WindowBaseX::SetNotificationLevel(Dali::Window::NotificationLevel::Type level)
{
  return false;
}

Dali::Window::NotificationLevel::Type WindowBaseX::GetNotificationLevel() const
{
  return Dali::Window::NotificationLevel::NONE;
}

void WindowBaseX::SetOpaqueState(bool opaque)
{
}

bool WindowBaseX::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  return false;
}

Dali::Window::ScreenOffMode::Type WindowBaseX::GetScreenOffMode() const
{
  return Dali::Window::ScreenOffMode::TIMEOUT;
}

bool WindowBaseX::SetBrightness(int brightness)
{
  return false;
}

int WindowBaseX::GetBrightness() const
{
  return 0;
}

bool WindowBaseX::GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode)
{
  return false;
}

bool WindowBaseX::UngrabKey(Dali::KEY key)
{
  return false;
}

bool WindowBaseX::GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result)
{
  return false;
}

bool WindowBaseX::UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result)
{
  return false;
}

void WindowBaseX::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // 1 inch = 25.4 millimeters
  WindowSystem::GetImplementation().GetDPI(dpiHorizontal, dpiVertical);
}

int WindowBaseX::GetWindowRotationAngle() const
{
  return 0;
}

int WindowBaseX::GetScreenRotationAngle()
{
  return 0;
}

void WindowBaseX::SetWindowRotationAngle(int degree)
{
  mWindowRotationAngle = degree;
}

void WindowBaseX::WindowRotationCompleted(int degree, int width, int height)
{
}

void WindowBaseX::SetAvailableOrientations(
  const std::vector<Dali::Window::WindowOrientation>& orientations)
{
}

void WindowBaseX::SetPreferredOrientation(Dali::Window::WindowOrientation orientation)
{
}

void WindowBaseX::SetTransparency(bool transparent)
{
}

unsigned int WindowBaseX::GetSurfaceId(Any surface) const
{
  unsigned int surfaceId = 0;

  if(surface.Empty() == false)
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS(((surface.GetType() == typeid(::Window))) && "Surface type is invalid");

    surfaceId = static_cast<unsigned int>(AnyCast<::Window>(surface));
  }
  return surfaceId;
}

void WindowBaseX::CreateInternalWindow(PositionSize positionSize, bool isTransparent)
{
  int depth = 3;

  if(isTransparent)
  {
    // create 32 bit window
    depth = 4;

    mIsTransparent = true;
  }
  auto& windowSystem = WindowSystem::GetImplementation();
  mWindow = windowSystem.CreateWindow(depth, positionSize.x, positionSize.y, positionSize.width, positionSize.height);

  if(mWindow == 0)
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to create X window");
  }
}

} // namespace Dali::Internal::Adaptor
