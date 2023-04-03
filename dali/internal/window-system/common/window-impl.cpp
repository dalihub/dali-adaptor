/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/window-impl.h>

// EXTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/orientation.h>
#include <dali/devel-api/events/key-event-devel.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/touch-integ.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/camera-actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/rendering/frame-buffer.h>
#include <thread>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW");
#endif

} // unnamed namespace

Window* Window::New(const PositionSize& positionSize, const std::string& name, const std::string& className, Dali::WindowType type, bool isTransparent)
{
  Any surface;
  return Window::New(surface, positionSize, name, className, type, isTransparent);
}

Window* Window::New(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className, Dali::WindowType type, bool isTransparent)
{
  Window* window         = new Window();
  window->mIsTransparent = isTransparent;
  window->Initialize(surface, positionSize, name, className, type);
  return window;
}

Window::Window()
: mWindowSurface(nullptr),
  mWindowBase(),
  mParentWindow(NULL),
  mPreferredAngle(static_cast<int>(WindowOrientation::NO_ORIENTATION_PREFERENCE)),
  mRotationAngle(0),
  mWindowWidth(0),
  mWindowHeight(0),
  mNativeWindowId(-1),
  mOrientationMode(Internal::Adaptor::Window::OrientationMode::PORTRAIT),
  mDeleteRequestSignal(),
  mFocusChangeSignal(),
  mResizeSignal(),
  mVisibilityChangedSignal(),
  mTransitionEffectEventSignal(),
  mKeyboardRepeatSettingsChangedSignal(),
  mAuxiliaryMessageSignal(),
  mMovedSignal(),
  mOrientationChangedSignal(),
  mLastKeyEvent(),
  mLastTouchEvent(),
  mIsTransparent(false),
  mIsFocusAcceptable(true),
  mIconified(false),
  mOpaqueState(false),
  mWindowRotationAcknowledgement(false),
  mFocused(false),
  mIsWindowRotating(false)
{
}

Window::~Window()
{
  if(mScene)
  {
    auto bridge     = Accessibility::Bridge::GetCurrentBridge();
    auto rootLayer  = mScene.GetRootLayer();
    auto accessible = Accessibility::Accessible::Get(rootLayer);
    bridge->RemoveTopLevelWindow(accessible);
    // Related to multi-window case. This is called for default window and non-default window, but it is effective for non-default window.
    bridge->Emit(accessible, Accessibility::WindowEvent::DESTROY);
  }

  if(DALI_LIKELY(mAdaptor))
  {
    mAdaptor->RemoveWindow(this);
  }

  if(mEventHandler)
  {
    mEventHandler->RemoveObserver(*this);
  }
}

void Window::Initialize(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className, WindowType type)
{
  // Create a window render surface
  auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
  mSurface                  = renderSurfaceFactory->CreateWindowRenderSurface(positionSize, surface, mIsTransparent);
  mWindowSurface            = static_cast<WindowRenderSurface*>(mSurface.get());

  // Get a window base
  mWindowBase = mWindowSurface->GetWindowBase();

  // Set Window Type
  mWindowBase->SetType(type);

  // Initialize for Ime window type
  if(type == WindowType::IME)
  {
    mWindowBase->InitializeIme();
    mWindowSurface->InitializeImeSurface();
  }

  // Connect signals
  mWindowBase->IconifyChangedSignal().Connect(this, &Window::OnIconifyChanged);
  mWindowBase->FocusChangedSignal().Connect(this, &Window::OnFocusChanged);
  mWindowBase->DeleteRequestSignal().Connect(this, &Window::OnDeleteRequest);
  mWindowBase->TransitionEffectEventSignal().Connect(this, &Window::OnTransitionEffectEvent);
  mWindowBase->KeyboardRepeatSettingsChangedSignal().Connect(this, &Window::OnKeyboardRepeatSettingsChanged);
  mWindowBase->WindowRedrawRequestSignal().Connect(this, &Window::OnWindowRedrawRequest);
  mWindowBase->UpdatePositionSizeSignal().Connect(this, &Window::OnUpdatePositionSize);
  mWindowBase->AuxiliaryMessageSignal().Connect(this, &Window::OnAuxiliaryMessage);

  mWindowSurface->OutputTransformedSignal().Connect(this, &Window::OnOutputTransformed);
  mWindowSurface->RotationFinishedSignal().Connect(this, &Window::OnRotationFinished);

  AddAuxiliaryHint("wm.policy.win.user.geometry", "1");

  SetClass(name, className);

  mOrientation = Orientation::New(this);

  // Get OrientationMode
  int screenWidth, screenHeight;
  WindowSystem::GetScreenSize(screenWidth, screenHeight);
  if(screenWidth > screenHeight)
  {
    mOrientationMode = Internal::Adaptor::Window::OrientationMode::LANDSCAPE;
  }
  else
  {
    mOrientationMode = Internal::Adaptor::Window::OrientationMode::PORTRAIT;
  }

  if(positionSize.width <= 0 || positionSize.height <= 0)
  {
    mWindowWidth  = screenWidth;
    mWindowHeight = screenHeight;
  }
  else
  {
    mWindowWidth  = positionSize.width;
    mWindowHeight = positionSize.height;
  }

  // For Debugging
  mNativeWindowId = mWindowBase->GetNativeWindowId();
}

void Window::OnAdaptorSet(Dali::Adaptor& adaptor)
{
  mEventHandler = EventHandlerPtr(new EventHandler(mWindowSurface->GetWindowBase(), *mAdaptor));
  mEventHandler->AddObserver(*this);

  // Add Window to bridge for ATSPI
  auto bridge = Accessibility::Bridge::GetCurrentBridge();
  if(bridge->IsUp())
  {
    auto rootLayer  = mScene.GetRootLayer();
    auto accessible = Accessibility::Accessible::Get(rootLayer);
    bridge->AddTopLevelWindow(accessible);
  }

  bridge->EnabledSignal().Connect(this, &Window::OnAccessibilityEnabled);
  bridge->DisabledSignal().Connect(this, &Window::OnAccessibilityDisabled);

  // If you call the 'Show' before creating the adaptor, the application cannot know the app resource id.
  // The show must be called after the adaptor is initialized.
  Show();
}

void Window::OnSurfaceSet(Dali::RenderSurfaceInterface* surface)
{
  mWindowSurface = static_cast<WindowRenderSurface*>(surface);
}

void Window::SetClass(std::string name, std::string className)
{
  mName      = name;
  mClassName = className;
  mWindowBase->SetClass(name, className);
}

std::string Window::GetClassName() const
{
  return mClassName;
}

void Window::Raise()
{
  mWindowBase->Raise();

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Raise() \n", this, mNativeWindowId);
}

void Window::Lower()
{
  mWindowBase->Lower();

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Lower() \n", this, mNativeWindowId);
}

void Window::Activate()
{
  mWindowBase->Activate();

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Activate() \n", this, mNativeWindowId);
}

void Window::Maximize(bool maximize)
{
  mWindowBase->Maximize(maximize);

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Maximize: %d\n", this, mNativeWindowId, maximize);
}

bool Window::IsMaximized() const
{
  return mWindowBase->IsMaximized();
}

void Window::SetMaximumSize(Dali::Window::WindowSize size)
{
  mWindowBase->SetMaximumSize(size);
}

void Window::Minimize(bool minimize)
{
  mWindowBase->Minimize(minimize);

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Minimize: %d\n", this, mNativeWindowId, minimize);
}

bool Window::IsMinimized() const
{
  return mWindowBase->IsMinimized();
}

void Window::SetMimimumSize(Dali::Window::WindowSize size)
{
  mWindowBase->SetMimimumSize(size);
}

uint32_t Window::GetLayerCount() const
{
  return mScene.GetLayerCount();
}

Dali::Layer Window::GetLayer(uint32_t depth) const
{
  return mScene.GetLayer(depth);
}

Dali::RenderTaskList Window::GetRenderTaskList() const
{
  return mScene.GetRenderTaskList();
}

std::string Window::GetNativeResourceId() const
{
  return mWindowBase->GetNativeWindowResourceId();
}

void Window::AddAvailableOrientation(WindowOrientation orientation)
{
  if(IsOrientationAvailable(orientation) == false)
  {
    return;
  }

  bool found          = false;
  int  convertedAngle = ConvertToAngle(orientation);
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), AddAvailableOrientation: %d\n", this, mNativeWindowId, convertedAngle);
  for(std::size_t i = 0; i < mAvailableAngles.size(); i++)
  {
    if(mAvailableAngles[i] == convertedAngle)
    {
      found = true;
      break;
    }
  }

  if(!found)
  {
    mAvailableAngles.push_back(convertedAngle);
    SetAvailableAnlges(mAvailableAngles);
  }
}

void Window::RemoveAvailableOrientation(WindowOrientation orientation)
{
  if(IsOrientationAvailable(orientation) == false)
  {
    return;
  }

  int convertedAngle = ConvertToAngle(orientation);
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), RemoveAvailableOrientation: %d\n", this, mNativeWindowId, convertedAngle);
  for(std::vector<int>::iterator iter = mAvailableAngles.begin();
      iter != mAvailableAngles.end();
      ++iter)
  {
    if(*iter == convertedAngle)
    {
      mAvailableAngles.erase(iter);
      break;
    }
  }

  SetAvailableAnlges(mAvailableAngles);
}

void Window::SetPreferredOrientation(WindowOrientation orientation)
{
  if(orientation < WindowOrientation::NO_ORIENTATION_PREFERENCE || orientation > WindowOrientation::LANDSCAPE_INVERSE)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::CheckOrientation: Invalid input orientation [%d]\n", orientation);
    return;
  }
  mPreferredAngle = ConvertToAngle(orientation);
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SetPreferredOrientation: %d\n", this, mNativeWindowId, mPreferredAngle);
  mWindowBase->SetPreferredAngle(mPreferredAngle);
}

WindowOrientation Window::GetPreferredOrientation()
{
  WindowOrientation preferredOrientation = ConvertToOrientation(mPreferredAngle);
  return preferredOrientation;
}

void Window::SetPositionSizeWithOrientation(PositionSize positionSize, WindowOrientation orientation)
{
  int angle = ConvertToAngle(orientation);
  mWindowBase->SetPositionSizeWithAngle(positionSize, angle);
}

void Window::EmitAccessibilityHighlightSignal(bool highlight)
{
  Dali::Window handle(this);
  mAccessibilityHighlightSignal.Emit(handle, highlight);
}

void Window::SetAvailableAnlges(const std::vector<int>& angles)
{
  if(angles.size() > 4)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::SetAvailableAnlges: Invalid vector size! [%d]\n", angles.size());
    return;
  }

  mWindowBase->SetAvailableAnlges(angles);
}

int Window::ConvertToAngle(WindowOrientation orientation)
{
  int convertAngle = static_cast<int>(orientation);
  if(mOrientationMode == Internal::Adaptor::Window::OrientationMode::LANDSCAPE)
  {
    switch(orientation)
    {
      case WindowOrientation::LANDSCAPE:
      {
        convertAngle = 0;
        break;
      }
      case WindowOrientation::PORTRAIT:
      {
        convertAngle = 90;
        break;
      }
      case WindowOrientation::LANDSCAPE_INVERSE:
      {
        convertAngle = 180;
        break;
      }
      case WindowOrientation::PORTRAIT_INVERSE:
      {
        convertAngle = 270;
        break;
      }
      case WindowOrientation::NO_ORIENTATION_PREFERENCE:
      {
        convertAngle = -1;
        break;
      }
    }
  }
  return convertAngle;
}

WindowOrientation Window::ConvertToOrientation(int angle) const
{
  WindowOrientation orientation = static_cast<WindowOrientation>(angle);
  if(mOrientationMode == Internal::Adaptor::Window::OrientationMode::LANDSCAPE)
  {
    switch(angle)
    {
      case 0:
      {
        orientation = WindowOrientation::LANDSCAPE;
        break;
      }
      case 90:
      {
        orientation = WindowOrientation::PORTRAIT;
        break;
      }
      case 180:
      {
        orientation = WindowOrientation::LANDSCAPE_INVERSE;
        break;
      }
      case 270:
      {
        orientation = WindowOrientation::PORTRAIT_INVERSE;
        break;
      }
      case -1:
      {
        orientation = WindowOrientation::NO_ORIENTATION_PREFERENCE;
        break;
      }
    }
  }
  return orientation;
}

bool Window::IsOrientationAvailable(WindowOrientation orientation) const
{
  if(orientation <= WindowOrientation::NO_ORIENTATION_PREFERENCE || orientation > WindowOrientation::LANDSCAPE_INVERSE)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::IsOrientationAvailable: Invalid input orientation [%d]\n", orientation);
    return false;
  }
  return true;
}

Dali::Any Window::GetNativeHandle() const
{
  return mWindowSurface->GetNativeWindow();
}

void Window::SetAcceptFocus(bool accept)
{
  mIsFocusAcceptable = accept;

  mWindowBase->SetAcceptFocus(accept);
}

bool Window::IsFocusAcceptable() const
{
  return mIsFocusAcceptable;
}

void Window::Show()
{
  mVisible = true;

  mWindowBase->Show();

  if(!mIconified)
  {
    Dali::Window handle(this);
    mVisibilityChangedSignal.Emit(handle, true);
    Dali::Accessibility::Bridge::GetCurrentBridge()->WindowShown(handle);

    WindowVisibilityObserver* observer(mAdaptor);
    observer->OnWindowShown();
  }

  mSurface->SetFullSwapNextFrame();

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Show(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible);
}

void Window::Hide()
{
  mVisible = false;

  mWindowBase->Hide();

  if(!mIconified)
  {
    Dali::Window handle(this);
    mVisibilityChangedSignal.Emit(handle, false);
    Dali::Accessibility::Bridge::GetCurrentBridge()->WindowHidden(handle);

    WindowVisibilityObserver* observer(mAdaptor);
    observer->OnWindowHidden();
  }

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Hide(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible);
}

bool Window::IsVisible() const
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), IsVisible(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible);
  return mVisible && !mIconified;
}

unsigned int Window::GetSupportedAuxiliaryHintCount() const
{
  return mWindowBase->GetSupportedAuxiliaryHintCount();
}

std::string Window::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return mWindowBase->GetSupportedAuxiliaryHint(index);
}

unsigned int Window::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  return mWindowBase->AddAuxiliaryHint(hint, value);
}

bool Window::RemoveAuxiliaryHint(unsigned int id)
{
  return mWindowBase->RemoveAuxiliaryHint(id);
}

bool Window::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  return mWindowBase->SetAuxiliaryHintValue(id, value);
}

std::string Window::GetAuxiliaryHintValue(unsigned int id) const
{
  return mWindowBase->GetAuxiliaryHintValue(id);
}

unsigned int Window::GetAuxiliaryHintId(const std::string& hint) const
{
  return mWindowBase->GetAuxiliaryHintId(hint);
}

void Window::SetInputRegion(const Rect<int>& inputRegion)
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SetInputRegion, (%d,%d), (%d x %d)\n", this, mNativeWindowId, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height);
  mWindowBase->SetInputRegion(inputRegion);
}

void Window::SetType(WindowType type)
{
  mWindowBase->SetType(type);
}

WindowType Window::GetType() const
{
  return mWindowBase->GetType();
}

WindowOperationResult Window::SetNotificationLevel(WindowNotificationLevel level)
{
  WindowType type = mWindowBase->GetType();
  if(type != WindowType::NOTIFICATION)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::SetNotificationLevel: Not supported window type [%d]\n", type);
    return WindowOperationResult::INVALID_OPERATION;
  }

  return mWindowBase->SetNotificationLevel(level);
}

WindowNotificationLevel Window::GetNotificationLevel() const
{
  WindowType type = mWindowBase->GetType();
  if(type != WindowType::NOTIFICATION)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::GetNotificationLevel: Not supported window type [%d]\n", type);
    return WindowNotificationLevel::NONE;
  }

  return mWindowBase->GetNotificationLevel();
}

void Window::SetOpaqueState(bool opaque)
{
  mOpaqueState = opaque;

  mWindowBase->SetOpaqueState(opaque);

  DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::SetOpaqueState: opaque = %d\n", opaque);
}

bool Window::IsOpaqueState() const
{
  return mOpaqueState;
}

WindowOperationResult Window::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  return mWindowBase->SetScreenOffMode(screenOffMode);
}

WindowScreenOffMode Window::GetScreenOffMode() const
{
  return mWindowBase->GetScreenOffMode();
}

WindowOperationResult Window::SetBrightness(int brightness)
{
  if(brightness < 0 || brightness > 100)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::SetBrightness: Invalid brightness value [%d]\n", brightness);
    return WindowOperationResult::INVALID_OPERATION;
  }

  return mWindowBase->SetBrightness(brightness);
}

int Window::GetBrightness() const
{
  return mWindowBase->GetBrightness();
}

void Window::SetSize(Dali::Window::WindowSize size)
{
  PositionSize oldRect = GetPositionSize();

  PositionSize newRect;
  newRect.width  = size.GetWidth();
  newRect.height = size.GetHeight();

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  if((oldRect.width != newRect.width) || (oldRect.height != newRect.height))
  {
    mWindowSurface->MoveResize(PositionSize(oldRect.x, oldRect.y, newRect.width, newRect.height));

    Uint16Pair newSize(newRect.width, newRect.height);

    mWindowWidth  = newRect.width;
    mWindowHeight = newRect.height;

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), current angle (%d), SetSize(): (%d, %d), [%d x %d]\n", this, mNativeWindowId, mRotationAngle, oldRect.x, oldRect.y, newRect.width, newRect.height);

    SurfaceResized(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));

    mAdaptor->SurfaceResizePrepare(mSurface.get(), newSize);

    Dali::Window handle(this);
    mResizeSignal.Emit(handle, newSize);

    mAdaptor->SurfaceResizeComplete(mSurface.get(), newSize);
  }

  mSurface->SetFullSwapNextFrame();

  Dali::Accessibility::Accessible::Get(mScene.GetRootLayer())->EmitBoundsChanged(Dali::Rect<>(oldRect.x, oldRect.y, size.GetWidth(), size.GetHeight()));
}

Dali::Window::WindowSize Window::GetSize() const
{
  return Dali::Window::WindowSize(mWindowWidth, mWindowHeight);
}

void Window::SetPosition(Dali::Window::WindowPosition position)
{
  PositionSize oldRect = mSurface->GetPositionSize();
  int32_t      newX    = position.GetX();
  int32_t      newY    = position.GetY();

  mWindowSurface->MoveResize(PositionSize(newX, newY, oldRect.width, oldRect.height));

  if((oldRect.x != newX) || (oldRect.y != newY))
  {
    Dali::Window                 handle(this);
    Dali::Window::WindowPosition newPosition(newX, newY);

    DALI_LOG_RELEASE_INFO("send moved signal with new position: %d, %d\n", newPosition.GetX(), newPosition.GetY());
    mMovedSignal.Emit(handle, newPosition);
  }

  mSurface->SetFullSwapNextFrame();

  Dali::Accessibility::Accessible::Get(mScene.GetRootLayer())->EmitBoundsChanged(Dali::Rect<>(position.GetX(), position.GetY(), oldRect.width, oldRect.height));
}

Dali::Window::WindowPosition Window::GetPosition() const
{
  PositionSize positionSize = GetPositionSize();
  return Dali::Window::WindowPosition(positionSize.x, positionSize.y);
}

PositionSize Window::GetPositionSize() const
{
  PositionSize positionSize = mSurface->GetPositionSize();
  positionSize.width        = mWindowWidth;
  positionSize.height       = mWindowHeight;
  return positionSize;
}

void Window::SetPositionSize(PositionSize positionSize)
{
  bool moved  = false;
  bool resize = false;

  PositionSize oldRect = GetPositionSize();
  Dali::Window handle(this);

  if((oldRect.x != positionSize.x) || (oldRect.y != positionSize.y))
  {
    moved = true;
  }

  if((oldRect.width != positionSize.width) || (oldRect.height != positionSize.height))
  {
    resize = true;
  }

  if(moved || resize)
  {
    mWindowSurface->MoveResize(positionSize);
  }

  // When window is moved, emit Moved Signal
  if(moved)
  {
    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Moved signal emit (%d, %d)\n", this, mNativeWindowId, positionSize.x, positionSize.y);
    Dali::Window::WindowPosition position(positionSize.x, positionSize.y);
    mMovedSignal.Emit(handle, position);
  }

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  if(resize)
  {
    Uint16Pair newSize(positionSize.width, positionSize.height);

    mWindowWidth  = positionSize.width;
    mWindowHeight = positionSize.height;

    SurfaceResized(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));

    mAdaptor->SurfaceResizePrepare(mSurface.get(), newSize);

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Resize signal emit [%d x %d]\n", this, mNativeWindowId, positionSize.width, positionSize.height);

    mResizeSignal.Emit(handle, newSize);
    mAdaptor->SurfaceResizeComplete(mSurface.get(), newSize);
  }

  mSurface->SetFullSwapNextFrame();

  Dali::Accessibility::Accessible::Get(mScene.GetRootLayer())->EmitBoundsChanged(Dali::Rect<>(positionSize.x, positionSize.y, positionSize.width, positionSize.height));
}

Dali::Layer Window::GetRootLayer() const
{
  return mScene.GetRootLayer();
}

void Window::SetTransparency(bool transparent)
{
  mWindowSurface->SetTransparency(transparent);
}

bool Window::GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode)
{
  return mWindowBase->GrabKey(key, grabMode);
}

bool Window::UngrabKey(Dali::KEY key)
{
  return mWindowBase->UngrabKey(key);
}

bool Window::GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result)
{
  return mWindowBase->GrabKeyList(key, grabMode, result);
}

bool Window::UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result)
{
  return mWindowBase->UngrabKeyList(key, result);
}

void Window::OnIconifyChanged(bool iconified)
{
  if(iconified)
  {
    mIconified = true;

    if(mVisible)
    {
      Dali::Window handle(this);
      mVisibilityChangedSignal.Emit(handle, false);
      Dali::Accessibility::Bridge::GetCurrentBridge()->WindowHidden(handle);

      if(DALI_LIKELY(mAdaptor))
      {
        WindowVisibilityObserver* observer(mAdaptor);
        observer->OnWindowHidden();
      }
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Iconified: visible = %d\n", this, mNativeWindowId, mVisible);
  }
  else
  {
    mIconified = false;

    if(mVisible)
    {
      Dali::Window handle(this);
      mVisibilityChangedSignal.Emit(handle, true);
      Dali::Accessibility::Bridge::GetCurrentBridge()->WindowShown(handle);

      if(DALI_LIKELY(mAdaptor))
      {
        WindowVisibilityObserver* observer(mAdaptor);
        observer->OnWindowShown();
      }
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Deiconified: visible = %d\n", this, mNativeWindowId, mVisible);
  }

  mSurface->SetFullSwapNextFrame();
}

void Window::OnFocusChanged(bool focusIn)
{
  Dali::Window handle(this);
  mFocusChangeSignal.Emit(handle, focusIn);

  mSurface->SetFullSwapNextFrame();

  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    if(focusIn)
    {
      bridge->WindowFocused(handle);
    }
    else
    {
      bridge->WindowUnfocused(handle);
    }
  }
  mFocused = focusIn;
}

void Window::OnOutputTransformed()
{
  PositionSize positionSize = GetPositionSize();

  SurfaceRotated(static_cast<float>(positionSize.width), static_cast<float>(positionSize.height), mRotationAngle, mWindowBase->GetScreenRotationAngle());

  if(DALI_LIKELY(mAdaptor))
  {
    mAdaptor->SurfaceResizePrepare(mSurface.get(), Adaptor::SurfaceSize(positionSize.width, positionSize.height));
    mAdaptor->SurfaceResizeComplete(mSurface.get(), Adaptor::SurfaceSize(positionSize.width, positionSize.height));
  }
}

void Window::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void Window::OnTransitionEffectEvent(WindowEffectState state, WindowEffectType type)
{
  Dali::Window handle(this);
  mTransitionEffectEventSignal.Emit(handle, state, type);
}

void Window::OnKeyboardRepeatSettingsChanged()
{
  Dali::Window handle(this);
  mKeyboardRepeatSettingsChangedSignal.Emit();
}

void Window::OnWindowRedrawRequest()
{
  if(DALI_LIKELY(mAdaptor))
  {
    mAdaptor->RenderOnce();
  }
}

void Window::OnUpdatePositionSize(Dali::PositionSize& positionSize)
{
  bool moved  = false;
  bool resize = false;

  Dali::Window handle(this);

  PositionSize oldRect = GetPositionSize();
  PositionSize newRect = positionSize;

  if((oldRect.x != newRect.x) || (oldRect.y != newRect.y))
  {
    moved = true;
  }

  if((oldRect.width != newRect.width) || (oldRect.height != newRect.height))
  {
    resize = true;
  }

  if(moved || resize)
  {
    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), current angle (%d), position or size is updated by server , (%d, %d) [%d x %d]\n", this, mNativeWindowId, mRotationAngle, newRect.x, newRect.y, newRect.width, newRect.height);
    mWindowSurface->UpdatePositionSize(positionSize);
  }

  if((oldRect.x != newRect.x) || (oldRect.y != newRect.y))
  {
    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Moved signal emit (%d, %d)\n", this, mNativeWindowId, newRect.x, newRect.y);
    Dali::Window::WindowPosition position(newRect.x, newRect.y);
    mMovedSignal.Emit(handle, position);
  }

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  if((oldRect.width != newRect.width) || (oldRect.height != newRect.height))
  {
    Uint16Pair newSize(newRect.width, newRect.height);

    mWindowWidth  = newRect.width;
    mWindowHeight = newRect.height;

    SurfaceResized(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));

    if(DALI_LIKELY(mAdaptor))
    {
      mAdaptor->SurfaceResizePrepare(mSurface.get(), newSize);
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Resized signal emit [%d x %d]\n", this, mNativeWindowId, newRect.width, newRect.height);
    mResizeSignal.Emit(handle, newSize);
    if(DALI_LIKELY(mAdaptor))
    {
      mAdaptor->SurfaceResizeComplete(mSurface.get(), newSize);
    }
  }

  mSurface->SetFullSwapNextFrame();

  if(DALI_LIKELY(mScene))
  {
    Dali::Accessibility::Accessible::Get(mScene.GetRootLayer())->EmitBoundsChanged(Dali::Rect<>(positionSize.x, positionSize.y, positionSize.width, positionSize.height));
  }
}

void Window::OnTouchPoint(Dali::Integration::Point& point, int timeStamp)
{
  mLastTouchEvent = Dali::Integration::NewTouchEvent(timeStamp, point);
  FeedTouchPoint(point, timeStamp);
}

void Window::OnWheelEvent(Dali::Integration::WheelEvent& wheelEvent)
{
  FeedWheelEvent(wheelEvent);
}

void Window::OnKeyEvent(Dali::Integration::KeyEvent& keyEvent)
{
  mLastKeyEvent = Dali::DevelKeyEvent::New(keyEvent.keyName, keyEvent.logicalKey, keyEvent.keyString, keyEvent.keyCode, keyEvent.keyModifier, keyEvent.time, static_cast<Dali::KeyEvent::State>(keyEvent.state), keyEvent.compose, keyEvent.deviceName, keyEvent.deviceClass, keyEvent.deviceSubclass);
  FeedKeyEvent(keyEvent);
}

void Window::OnRotation(const RotationEvent& rotation)
{
  PositionSize newPositionSize(rotation.x, rotation.y, rotation.width, rotation.height);

  mRotationAngle = rotation.angle;
  mWindowWidth   = rotation.width;
  mWindowHeight  = rotation.height;

  mIsWindowRotating = true;
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), angle(%d), Window Rotation (%d , %d) [%d x %d]\n", this, mNativeWindowId, mRotationAngle, newPositionSize.x, newPositionSize.y, mWindowWidth, mWindowHeight);

  // Notify that the orientation is changed
  mOrientation->OnOrientationChange(rotation);

  mWindowSurface->RequestRotation(mRotationAngle, newPositionSize);

  SurfaceRotated(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight), mRotationAngle, mWindowBase->GetScreenRotationAngle());

  if(DALI_LIKELY(mAdaptor))
  {
    mAdaptor->SurfaceResizePrepare(mSurface.get(), Adaptor::SurfaceSize(mWindowWidth, mWindowHeight));
  }

  Dali::Window handle(this);
  mResizeSignal.Emit(handle, Dali::Window::WindowSize(mWindowWidth, mWindowHeight));
  mOrientationChangedSignal.Emit(handle, GetCurrentOrientation());

  if(DALI_LIKELY(mAdaptor))
  {
    mAdaptor->SurfaceResizeComplete(mSurface.get(), Adaptor::SurfaceSize(mWindowWidth, mWindowHeight));
  }
}

void Window::OnRotationFinished()
{
  mIsWindowRotating = false;
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), window rotation is finised\n", this, mNativeWindowId);
}

void Window::OnPause()
{
  if(mEventHandler)
  {
    mEventHandler->Pause();
  }
}

void Window::OnResume()
{
  if(mEventHandler)
  {
    mEventHandler->Resume();
  }

  mSurface->SetFullSwapNextFrame();
}

void Window::OnAuxiliaryMessage(const std::string& key, const std::string& value, const Property::Array& options)
{
  mAuxiliaryMessageSignal.Emit(key, value, options);
}

void Window::OnAccessibilityEnabled()
{
  auto bridge     = Accessibility::Bridge::GetCurrentBridge();
  auto rootLayer  = mScene.GetRootLayer();
  auto accessible = Accessibility::Accessible::Get(rootLayer);
  bridge->AddTopLevelWindow(accessible);

  if(!mVisible || mIconified)
  {
    return;
  }

  Dali::Window handle(this);
  bridge->WindowShown(handle);

  if(mFocused)
  {
    bridge->WindowFocused(handle);
  }
}

void Window::OnAccessibilityDisabled()
{
  auto bridge     = Accessibility::Bridge::GetCurrentBridge();
  auto rootLayer  = mScene.GetRootLayer();
  auto accessible = Accessibility::Accessible::Get(rootLayer);
  bridge->RemoveTopLevelWindow(accessible);
}

Vector2 Window::RecalculatePosition(const Vector2& position)
{
  Vector2 convertedPosition;

  switch(mRotationAngle)
  {
    case 90:
    {
      convertedPosition.x = static_cast<float>(mWindowWidth) - position.y;
      convertedPosition.y = position.x;
      break;
    }
    case 180:
    {
      convertedPosition.x = static_cast<float>(mWindowWidth) - position.x;
      convertedPosition.y = static_cast<float>(mWindowHeight) - position.y;
      break;
    }
    case 270:
    {
      convertedPosition.x = position.y;
      convertedPosition.y = static_cast<float>(mWindowHeight) - position.x;
      break;
    }
    default:
    {
      convertedPosition = position;
      break;
    }
  }
  return convertedPosition;
}

Dali::Window Window::Get(Dali::Actor actor)
{
  Internal::Adaptor::Window* windowImpl = nullptr;

  if(Internal::Adaptor::Adaptor::IsAvailable())
  {
    Dali::Internal::Adaptor::Adaptor& adaptor = Internal::Adaptor::Adaptor::GetImplementation(Internal::Adaptor::Adaptor::Get());
    windowImpl                                = dynamic_cast<Internal::Adaptor::Window*>(adaptor.GetWindow(actor));
    if(windowImpl)
    {
      return Dali::Window(windowImpl);
    }
  }

  return Dali::Window();
}

void Window::SetParent(Dali::Window& parent)
{
  if(DALI_UNLIKELY(parent))
  {
    mParentWindow     = parent;
    Dali::Window self = Dali::Window(this);
    // check circular parent window setting
    if(Dali::DevelWindow::GetParent(parent) == self)
    {
      Dali::DevelWindow::Unparent(parent);
    }
    mWindowBase->SetParent(GetImplementation(mParentWindow).mWindowBase, false);
  }
}

void Window::SetParent(Dali::Window& parent, bool belowParent)
{
  if(DALI_UNLIKELY(parent))
  {
    mParentWindow     = parent;
    Dali::Window self = Dali::Window(this);
    // check circular parent window setting
    if(Dali::DevelWindow::GetParent(parent) == self)
    {
      Dali::DevelWindow::Unparent(parent);
    }
    mWindowBase->SetParent(GetImplementation(mParentWindow).mWindowBase, belowParent);
  }
}

void Window::Unparent()
{
  mWindowBase->SetParent(nullptr, false);
  mParentWindow.Reset();
}

Dali::Window Window::GetParent()
{
  return mParentWindow;
}

WindowOrientation Window::GetCurrentOrientation() const
{
  return ConvertToOrientation(mRotationAngle);
}

int Window::GetPhysicalOrientation() const
{
  return (mRotationAngle + mWindowBase->GetScreenRotationAngle()) % 360;
}

void Window::SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations)
{
  Dali::Vector<float>::SizeType count = orientations.Count();
  for(Dali::Vector<float>::SizeType index = 0; index < count; ++index)
  {
    if(IsOrientationAvailable(orientations[index]) == false)
    {
      DALI_LOG_ERROR("Window::SetAvailableOrientations, invalid orientation: %d\n", orientations[index]);
      continue;
    }

    bool found          = false;
    int  convertedAngle = ConvertToAngle(orientations[index]);

    for(std::size_t i = 0; i < mAvailableAngles.size(); i++)
    {
      if(mAvailableAngles[i] == convertedAngle)
      {
        found = true;
        break;
      }
    }

    if(!found)
    {
      DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SetAvailableOrientations: %d\n", this, mNativeWindowId, convertedAngle);
      mAvailableAngles.push_back(convertedAngle);
    }
  }
  SetAvailableAnlges(mAvailableAngles);
}

int32_t Window::GetNativeId() const
{
  return mWindowBase->GetNativeWindowId();
}

void Window::RequestMoveToServer()
{
  mWindowBase->RequestMoveToServer();
}

void Window::RequestResizeToServer(WindowResizeDirection direction)
{
  mWindowBase->RequestResizeToServer(direction);
}

void Window::EnableFloatingMode(bool enable)
{
  mWindowBase->EnableFloatingMode(enable);
}

void Window::IncludeInputRegion(const Rect<int>& inputRegion)
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), IncludeInputRegion, (%d,%d), (%d x %d)\n", this, mNativeWindowId, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height);
  mWindowBase->IncludeInputRegion(inputRegion);
}

void Window::ExcludeInputRegion(const Rect<int>& inputRegion)
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), ExcludeInputRegion, (%d,%d), (%d x %d)\n", this, mNativeWindowId, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height);
  mWindowBase->ExcludeInputRegion(inputRegion);
}

void Window::SetNeedsRotationCompletedAcknowledgement(bool needAcknowledgement)
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), needAcknowledgement(%d) Set needs Rotation Completed Acknowledgement\n", this, mNativeWindowId, needAcknowledgement);
  mWindowSurface->SetNeedsRotationCompletedAcknowledgement(needAcknowledgement);
  mWindowRotationAcknowledgement = needAcknowledgement;
}

void Window::SendRotationCompletedAcknowledgement()
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SendRotationCompletedAcknowledgement(): orientation: %d, mWindowRotationAcknowledgement: %d\n", this, mNativeWindowId, mRotationAngle, mWindowRotationAcknowledgement);
  if(mWindowRotationAcknowledgement)
  {
    SetRotationCompletedAcknowledgement();
  }
}

bool Window::IsWindowRotating() const
{
  return mIsWindowRotating;
}

const Dali::KeyEvent& Window::GetLastKeyEvent() const
{
  return mLastKeyEvent;
}

const Dali::TouchEvent& Window::GetLastTouchEvent() const
{
  return mLastTouchEvent;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
