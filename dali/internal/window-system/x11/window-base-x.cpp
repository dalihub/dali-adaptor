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
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/x11/window-system-x.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/public-api/object/any.h>

using Dali::Internal::Adaptor::WindowSystem::WindowSystemX;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const char*                  DEFAULT_DEVICE_NAME     = "";
const Device::Class::Type    DEFAULT_DEVICE_CLASS    = Device::Class::NONE;
const Device::Subclass::Type DEFAULT_DEVICE_SUBCLASS = Device::Subclass::NONE;

const char* DESKTOP_STARTUP_ID_ENV = "DESKTOP_STARTUP_ID";

const unsigned int PRIMARY_TOUCH_BUTTON_ID(1);

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW_BASE");
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static bool EventWindowConfigureNotify(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnConfigure(event);
  }

  return false;
}

static bool EventWindowPropertyChanged(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    return windowBase->OnWindowPropertyChanged(data, type, event);
  }

  return false;
}

/**
 * Called when the window receives a delete request
 */
static bool EventWindowDeleteRequest(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnDeleteRequest();
  }
  return true;
}

/**
 * Called when the window gains focus.
 */
static bool EventWindowFocusIn(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
static bool EventSelectionClear(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
static bool EventSelectionNotify(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
static bool EventMouseButtonDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonDown(data, type, event);
  }
  return false;
}

/**
 * Called when a touch up is received.
 */
static bool EventMouseButtonUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonUp(data, type, event);
  }
  return false;
}

/**
 * Called when a touch motion is received.
 */
static bool EventMouseButtonMove(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
static bool EventMouseWheel(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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
static bool EventKeyDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
  if(windowBase)
  {
    windowBase->OnKeyDown(data, type, event);
  }
  return false;
}

/**
 * Called when a key up is received.
 */
static bool EventKeyUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  WindowBaseX* windowBase = static_cast<WindowBaseX*>(data);
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

  {
    const char* id = Dali::EnvironmentVariable::GetEnvironmentVariable(DESKTOP_STARTUP_ID_ENV);
    if(id != nullptr)
    {
      windowSystem.SetStringProperty(mWindow, WindowSystemX::ATOM_NET_STARTUP_ID, std::string(id));
    }
  }

  windowSystem.SetWindowHints(mWindow, true);
  windowSystem.Sync();

  EnableMultipleSelection();
  EnableWindowClose();
  EnableDragAndDrop();

  SetupEvents();
}

void WindowBaseX::SetWindowHints()
{
}

void WindowBaseX::EnableMultipleSelection()
{
  WindowSystem::GetImplementation().InputMultiSelect(mWindow);
}

void WindowBaseX::EnableWindowClose()
{
  WindowSystem::GetImplementation().SetProtocol(mWindow, WindowSystemX::ATOM_WM_DELETE_WINDOW, true);
}

void WindowBaseX::EnableDragAndDrop()
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
  if(propertyNotifyEvent->window == mWindow && Dali::Adaptor::IsAvailable())
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
  if(configureEvent->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::OnConfigureNotify\n");
    Dali::PositionSize positionSize;
    positionSize.x      = configureEvent->x;
    positionSize.y      = configureEvent->y;
    positionSize.width  = configureEvent->width;
    positionSize.height = configureEvent->height;
    /// @note Can also get the window below this one if raise/lower was called.
    mUpdatePositionSizeSignal.Emit(positionSize);
  }
}

void WindowBaseX::OnDeleteRequest()
{
  if(Dali::Adaptor::IsAvailable())
  {
    mDeleteRequestSignal.Emit();
  }
}

void WindowBaseX::OnFocusIn(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event)
{
  auto x11Event = static_cast<WindowSystemX::X11Event*>(event);

  if(x11Event->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::OnFocusIn\n");

    mFocusChangedSignal.Emit(true);
  }
}

void WindowBaseX::OnFocusOut(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event)
{
  auto x11Event = static_cast<WindowSystemX::X11Event*>(event);
  // If the window loses focus then hide the keyboard.
  if(x11Event->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window::FocusOut\n");

    mFocusChangedSignal.Emit(false);
  }
}

void WindowBaseX::OnWindowDamaged(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event)
{
  auto windowExposeEvent = static_cast<WindowSystemX::X11ExposeEvent*>(event);
  if(windowExposeEvent->window == mWindow && Dali::Adaptor::IsAvailable())
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

  if(touchEvent->window == mWindow && Dali::Adaptor::IsAvailable())
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

    mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseX::OnMouseButtonUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto touchEvent = static_cast<WindowSystemX::X11MouseEvent*>(event);

  if(touchEvent->window == mWindow && Dali::Adaptor::IsAvailable())
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

    mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseX::OnMouseButtonMove(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto touchEvent = static_cast<WindowSystemX::X11MouseEvent*>(event);

  if(touchEvent->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    Integration::Point point;
    point.SetDeviceId(touchEvent->device);
    point.SetState(PointState::MOTION);
    point.SetScreenPosition(Vector2(static_cast<float>(touchEvent->x), static_cast<float>(touchEvent->y)));
    point.SetRadius(static_cast<float>(touchEvent->multi.radius), Vector2(static_cast<float>(touchEvent->multi.radiusX), static_cast<float>(touchEvent->multi.radiusY)));
    point.SetPressure(static_cast<float>(touchEvent->multi.pressure));
    point.SetAngle(Degree(static_cast<float>(touchEvent->multi.angle)));

    mTouchEventSignal.Emit(point, touchEvent->timestamp);

    mMouseFrameEventSignal.Emit();
  }
}

void WindowBaseX::OnMouseWheel(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto mouseWheelEvent = static_cast<WindowSystemX::X11MouseWheelEvent*>(event);

  if(mouseWheelEvent->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseX::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z);

    Integration::WheelEvent wheelEvent(Integration::WheelEvent::MOUSE_WHEEL, mouseWheelEvent->direction, mouseWheelEvent->modifiers, Vector2(static_cast<float>(mouseWheelEvent->x), static_cast<float>(mouseWheelEvent->y)), mouseWheelEvent->z, mouseWheelEvent->timestamp);

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
  daliKeyEvent.windowId = GetNativeWindowId();
  return daliKeyEvent;
}

void WindowBaseX::OnKeyDown(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto keyEvent = static_cast<WindowSystemX::X11KeyEvent*>(event);

  if(keyEvent->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseX::OnKeyDown\n");

    auto daliKeyEvent = CreateKeyEvent(keyEvent, Integration::KeyEvent::DOWN);

    mKeyEventSignal.Emit(daliKeyEvent);
  }
}

void WindowBaseX::OnKeyUp(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event)
{
  auto keyEvent = static_cast<WindowSystemX::X11KeyEvent*>(event);

  if(keyEvent->window == mWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, " WindowBaseX::OnKeyUp\n");

    auto daliKeyEvent = CreateKeyEvent(keyEvent, Integration::KeyEvent::UP);

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

std::string WindowBaseX::GetNativeWindowResourceId()
{
  return std::string();
}

Dali::Any WindowBaseX::CreateWindow(int width, int height)
{
  return reinterpret_cast<void*>(mWindow);
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

void WindowBaseX::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
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

void WindowBaseX::Maximize(bool maximize)
{
}

bool WindowBaseX::IsMaximized() const
{
  return false;
}

void WindowBaseX::Minimize(bool minimize)
{
}

bool WindowBaseX::IsMinimized() const
{
  return false;
}

void WindowBaseX::SetMimimumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseX::SetMaximumSize(Dali::Window::WindowSize size)
{
}

void WindowBaseX::SetAvailableAnlges(const std::vector<int>& angles)
{
}

void WindowBaseX::SetPreferredAngle(int angle)
{
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

void WindowBaseX::SetType(Dali::WindowType type)
{
}

Dali::WindowType WindowBaseX::GetType() const
{
  return Dali::WindowType::NORMAL;
}

Dali::WindowOperationResult WindowBaseX::SetNotificationLevel(Dali::WindowNotificationLevel level)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

Dali::WindowNotificationLevel WindowBaseX::GetNotificationLevel() const
{
  return Dali::WindowNotificationLevel::NONE;
}

void WindowBaseX::SetOpaqueState(bool opaque)
{
}

Dali::WindowOperationResult WindowBaseX::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
}

WindowScreenOffMode WindowBaseX::GetScreenOffMode() const
{
  return WindowScreenOffMode::TIMEOUT;
}

Dali::WindowOperationResult WindowBaseX::SetBrightness(int brightness)
{
  return Dali::WindowOperationResult::NOT_SUPPORTED;
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
  mWindow = WindowSystem::GetImplementation().CreateWindow(depth, positionSize.x, positionSize.y, positionSize.width, positionSize.height);

  if(mWindow == 0)
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to create X window");
  }
}

void WindowBaseX::SetParent(WindowBase* parentWinBase, bool belowParent)
{
  if(parentWinBase)
  {
    WindowBaseX* windowBaseX  = static_cast<WindowBaseX*>(parentWinBase);
    ::Window     parentWindow = windowBaseX->mWindow;
    WindowSystem::GetImplementation().SetTransientForHint(mWindow, parentWindow);
  }
  else
  {
    WindowSystem::GetImplementation().UnsetTransientFor(mWindow);
  }
}

int WindowBaseX::CreateFrameRenderedSyncFence()
{
  return -1;
}

int WindowBaseX::CreateFramePresentedSyncFence()
{
  return -1;
}

void WindowBaseX::SetPositionSizeWithAngle(PositionSize positionSize, int angle)
{
}

void WindowBaseX::InitializeIme()
{
}

void WindowBaseX::ImeWindowReadyToRender()
{
}

void WindowBaseX::RequestMoveToServer()
{
}

void WindowBaseX::RequestResizeToServer(WindowResizeDirection direction)
{
}

void WindowBaseX::EnableFloatingMode(bool enable)
{
}

bool WindowBaseX::IsFloatingModeEnabled() const
{
  return false;
}

void WindowBaseX::IncludeInputRegion(const Rect<int>& inputRegion)
{
}

void WindowBaseX::ExcludeInputRegion(const Rect<int>& inputRegion)
{
}

bool WindowBaseX::PointerConstraintsLock()
{
  return false;
}

bool WindowBaseX::PointerConstraintsUnlock()
{
  return false;
}

void WindowBaseX::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
}

void WindowBaseX::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
}

bool WindowBaseX::PointerWarp(int32_t x, int32_t y)
{
  return false;
}

void WindowBaseX::CursorVisibleSet(bool visible)
{
}

bool WindowBaseX::KeyboardGrab(Device::Subclass::Type deviceSubclass)
{
  return false;
}

bool WindowBaseX::KeyboardUnGrab()
{
  return false;
}

void WindowBaseX::SetFullScreen(bool fullscreen)
{
  return;
}

bool WindowBaseX::GetFullScreen()
{
  return false;
}

void WindowBaseX::SetFrontBufferRendering(bool enable)
{
}

bool WindowBaseX::GetFrontBufferRendering()
{
  return false;
}

void WindowBaseX::SetWindowFrontBufferMode(bool enable)
{
}

void WindowBaseX::SetModal(bool modal)
{
}

bool WindowBaseX::IsModal()
{
  return false;
}

void WindowBaseX::SetAlwaysOnTop(bool alwaysOnTop)
{
}

bool WindowBaseX::IsAlwaysOnTop()
{
  return false;
}

void WindowBaseX::SetBottom(bool enable)
{
}

bool WindowBaseX::IsBottom()
{
  return false;
}

Any WindowBaseX::GetNativeBuffer() const
{
  return 0;
}

bool WindowBaseX::RelativeMotionGrab(uint32_t boundary)
{
  return false;
}

bool WindowBaseX::RelativeMotionUnGrab()
{
  return false;
}

void WindowBaseX::SetBackgroundBlur(int blurRadius, int cornerRadius)
{
}

int WindowBaseX::GetBackgroundBlur()
{
  return 0;
}

void WindowBaseX::SetBehindBlur(int blurRadius)
{
}

int WindowBaseX::GetBehindBlur()
{
  return 0;
}

Extents WindowBaseX::GetInsets()
{
  return Extents(0, 0, 0, 0);
}

Extents WindowBaseX::GetInsets(WindowInsetsPartFlags insetsFlags)
{
  return Extents(0, 0, 0, 0);
}

void WindowBaseX::SetScreen(const std::string& screenName)
{
}

std::string WindowBaseX::GetScreen() const
{
  return std::string();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
