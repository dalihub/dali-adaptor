/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/camera-actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/rendering/frame-buffer.h>
#include <thread>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/actor-accessible.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
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

Window* Window::New(const std::string& name, const std::string& className, const WindowData& windowData)
{
  Any surface;
  return Window::New(surface, name, className, windowData);
}

Window* Window::New(Any surface, const std::string& name, const std::string& className, const WindowData& windowData)
{
  Window* window                  = new Window();
  window->mIsTransparent          = windowData.GetTransparency();
  window->mIsFrontBufferRendering = windowData.GetFrontBufferRendering();
  window->Initialize(surface, windowData.GetPositionSize(), name, className, windowData.GetWindowType());
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
  mMouseInOutEventSignal(),
  mMouseRelativeEventSignal(),
  mMoveCompletedSignal(),
  mResizeCompletedSignal(),
  mInsetsChangedSignal(),
  mPointerConstraintsSignal(),
  mLastKeyEvent(),
  mIsTransparent(false),
  mIsFocusAcceptable(true),
  mIconified(false),
  mMaximized(false),
  mOpaqueState(false),
  mWindowRotationAcknowledgement(false),
  mFocused(false),
  mIsWindowRotating(false),
  mIsEnabledUserGeometry(false),
  mIsEmittedWindowCreatedEvent(false),
  mIsFrontBufferRendering(false)
{
}

Window::~Window()
{
  if(mAdaptor)
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
  DALI_ASSERT_DEBUG(renderSurfaceFactory && "Cannot create render surface factory\n");

  mSurface       = renderSurfaceFactory->CreateWindowRenderSurface(positionSize, surface, mIsTransparent);
  mWindowSurface = static_cast<WindowRenderSurface*>(mSurface.get());

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
  mWindowBase->MaximizeChangedSignal().Connect(this, &Window::OnMaximizeChanged);
  mWindowBase->FocusChangedSignal().Connect(this, &Window::OnFocusChanged);
  mWindowBase->DeleteRequestSignal().Connect(this, &Window::OnDeleteRequest);
  mWindowBase->TransitionEffectEventSignal().Connect(this, &Window::OnTransitionEffectEvent);
  mWindowBase->KeyboardRepeatSettingsChangedSignal().Connect(this, &Window::OnKeyboardRepeatSettingsChanged);
  mWindowBase->WindowRedrawRequestSignal().Connect(this, &Window::OnWindowRedrawRequest);
  mWindowBase->UpdatePositionSizeSignal().Connect(this, &Window::OnUpdatePositionSize);
  mWindowBase->AuxiliaryMessageSignal().Connect(this, &Window::OnAuxiliaryMessage);
  mWindowBase->MouseInOutEventSignal().Connect(this, &Window::OnMouseInOutEvent);
  mWindowBase->MouseRelativeEventSignal().Connect(this, &Window::OnMouseRelativeEvent);
  mWindowBase->MoveCompletedSignal().Connect(this, &Window::OnMoveCompleted);
  mWindowBase->ResizeCompletedSignal().Connect(this, &Window::OnResizeCompleted);
  mWindowBase->PointerConstraintsSignal().Connect(this, &Window::OnPointerConstraints);

  mWindowSurface->OutputTransformedSignal().Connect(this, &Window::OnOutputTransformed);
  mWindowSurface->RotationFinishedSignal().Connect(this, &Window::OnRotationFinished);

  mWindowBase->InsetsChangedSignal().Connect(this, &Window::OnInsetsChanged);

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

  mWindowWidth  = positionSize.width;
  mWindowHeight = positionSize.height;

  bool isSetWithScreenSize = false;
  if(mWindowWidth <= 0 || mWindowHeight <= 0)
  {
    mWindowWidth        = screenWidth;
    mWindowHeight       = screenHeight;
    isSetWithScreenSize = true;
    DALI_LOG_RELEASE_INFO("Window size is set with screen size(%d x %d)\n", mWindowWidth, mWindowHeight);
  }

  if(isSetWithScreenSize == false || positionSize.x != 0 || positionSize.y != 0)
  {
    SetUserGeometryPolicy();
  }

  // For Debugging
  mNativeWindowId = mWindowBase->GetNativeWindowId();

  if(mIsFrontBufferRendering)
  {
    SetFrontBufferRendering(mIsFrontBufferRendering);
  }
}

void Window::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  if(!mWindowSurface)
  {
    return;
  }

  mWindowSurface->SetRenderNotification(renderNotification);
}

void Window::OnAdaptorSet(Dali::Adaptor& adaptor)
{
  mEventHandler = EventHandlerPtr(new EventHandler(mWindowSurface->GetWindowBase(), *mAdaptor));
  mEventHandler->AddObserver(*this);

  if(mWindowBase->GetType() == WindowType::IME)
  {
    mWindowBase->InitializeIme();
    mWindowSurface->InitializeImeSurface();
  }

  // Add Window to bridge for ATSPI
  auto bridge = Accessibility::Bridge::GetCurrentBridge();

  bridge->EnabledSignal().Connect(this, &Window::OnAccessibilityEnabled);
  bridge->DisabledSignal().Connect(this, &Window::OnAccessibilityDisabled);

  if(bridge->IsUp())
  {
    OnAccessibilityEnabled();
  }
  else
  {
    OnAccessibilityDisabled();
  }

  if(mScene)
  {
    bool isGeometry = Dali::Internal::Adaptor::WindowSystem::IsGeometryHittestEnabled();
    mScene.SetGeometryHittestEnabled(isGeometry);

    mScene.SetNativeId(GetNativeId());
  }

  // If you call the 'Show' before creating the adaptor, the application cannot know the app resource id.
  // The show must be called after the adaptor is initialized.
  Show();
}

void Window::OnSurfaceSet(Dali::Integration::RenderSurfaceInterface* surface)
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

void Window::KeepRendering(float durationSeconds)
{
  mScene.KeepRendering(durationSeconds);
}

void Window::SetPartialUpdateEnabled(bool enabled)
{
  mScene.SetPartialUpdateEnabled(enabled);
}

bool Window::IsPartialUpdateEnabled() const
{
  return mScene.IsPartialUpdateEnabled();
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

    // mScene should be shown here because Scene::IsVisible need to be returned true in the Window::VisibilityChangedSignal
    mScene.Show();

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

    // mScene should be hidden here because Scene::IsVisible need to be returned false in the Window::VisibilityChangedSignal
    mScene.Hide();

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

  SetUserGeometryPolicy();

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

    if(Dali::Accessibility::IsUp())
    {
      if(auto accessible = dynamic_cast<Accessibility::ActorAccessible*>(Accessibility::Accessible::Get(mScene.GetRootLayer())))
      {
        accessible->EmitBoundsChanged(Dali::Rect<>(oldRect.x, oldRect.y, size.GetWidth(), size.GetHeight()));
      }
    }
  }

  mSurface->SetFullSwapNextFrame();
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

  SetUserGeometryPolicy();

  mWindowSurface->Move(PositionSize(newX, newY, oldRect.width, oldRect.height));

  if((oldRect.x != newX) || (oldRect.y != newY))
  {
    Dali::Window                 handle(this);
    Dali::Window::WindowPosition newPosition(newX, newY);

    DALI_LOG_RELEASE_INFO("send moved signal with new position: %d, %d\n", newPosition.GetX(), newPosition.GetY());
    mMovedSignal.Emit(handle, newPosition);

    if(Dali::Accessibility::IsUp())
    {
      if(auto accessible = dynamic_cast<Accessibility::ActorAccessible*>(Accessibility::Accessible::Get(mScene.GetRootLayer())))
      {
        accessible->EmitBoundsChanged(Dali::Rect<>(position.GetX(), position.GetY(), oldRect.width, oldRect.height));
      }
    }
  }

  mSurface->SetFullSwapNextFrame();
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
  SetUserGeometryPolicy();

  // It is asked by application to change window position and size.
  // The related API is called to ask to display server in the backend.
  UpdatePositionSize(positionSize, true);
}

void Window::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
  SetUserGeometryPolicy();
  mWindowBase->SetLayout(numCols, numRows, column, row, colSpan, rowSpan);
}

Dali::Layer Window::GetRootLayer() const
{
  return mScene.GetRootLayer();
}

void Window::SetBackgroundColor(const Vector4& color)
{
  mBackgroundColor = color;
  if(mIsTransparent)
  {
    Vector4 backgroundColor = color;
    backgroundColor.r *= backgroundColor.a;
    backgroundColor.g *= backgroundColor.a;
    backgroundColor.b *= backgroundColor.a;
    SceneHolder::SetBackgroundColor(backgroundColor);
  }
  else
  {
    SceneHolder::SetBackgroundColor(color);
  }
}

Vector4 Window::GetBackgroundColor() const
{
  return mBackgroundColor;
}

void Window::SetTransparency(bool transparent)
{
  if(mIsTransparent != transparent)
  {
    mIsTransparent = transparent;
    Window::SetBackgroundColor(mBackgroundColor);
  }
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SetTransparency = %d\n", this, mNativeWindowId, transparent);  
  mWindowSurface->SetTransparency(mIsTransparent);
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
  const bool   isActuallyChanged = (iconified != mIconified);
  auto         bridge            = Dali::Accessibility::Bridge::GetCurrentBridge();
  Dali::Window handle(this);

  if(iconified)
  {
    mIconified = true;

    if(mVisible)
    {
      mScene.Hide();

      mVisibilityChangedSignal.Emit(handle, false);
      bridge->WindowHidden(handle);

      WindowVisibilityObserver* observer(mAdaptor);
      observer->OnWindowHidden();
    }

    if(isActuallyChanged)
    {
      bridge->WindowMinimized(handle);
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Iconified: visible = %d\n", this, mNativeWindowId, mVisible);
  }
  else
  {
    mIconified = false;

    if(mVisible)
    {
      mScene.Show();

      mVisibilityChangedSignal.Emit(handle, true);
      bridge->WindowShown(handle);

      WindowVisibilityObserver* observer(mAdaptor);
      observer->OnWindowShown();
    }

    if(isActuallyChanged)
    {
      bridge->WindowRestored(handle, Dali::Accessibility::WindowRestoreType::RESTORE_FROM_ICONIFY);
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Deiconified: visible = %d\n", this, mNativeWindowId, mVisible);
  }

  mSurface->SetFullSwapNextFrame();

  // Need to update/render once if surface set full swaped after adaptor call ProcessCoreEvents().
  if(DALI_LIKELY(mAdaptor) && !iconified)
  {
    mAdaptor->RequestUpdateOnce();
  }
}

void Window::OnMaximizeChanged(bool maximized)
{
  const bool isActuallyChanged = (maximized != mMaximized);

  if(isActuallyChanged)
  {
    auto         bridge = Dali::Accessibility::Bridge::GetCurrentBridge();
    Dali::Window handle(this);

    if(maximized)
    {
      mMaximized = true;
      bridge->WindowMaximized(handle);
    }
    else
    {
      mMaximized = false;
      bridge->WindowRestored(handle, Dali::Accessibility::WindowRestoreType::RESTORE_FROM_MAXIMIZE);
    }
  }
}

void Window::OnFocusChanged(bool focusIn)
{
  Dali::Window handle(this);
  mFocusChangeSignal.Emit(handle, focusIn);

  mSurface->SetFullSwapNextFrame();
  auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge();

  if(focusIn)
  {
    bridge->WindowFocused(handle);
  }
  else
  {
    bridge->WindowUnfocused(handle);
  }

  mFocused = focusIn;
}

void Window::OnOutputTransformed()
{
  PositionSize positionSize = GetPositionSize();

  SurfaceRotated(static_cast<float>(positionSize.width), static_cast<float>(positionSize.height), mRotationAngle, mWindowBase->GetScreenRotationAngle());

  mAdaptor->SurfaceResizePrepare(mSurface.get(), Adaptor::SurfaceSize(positionSize.width, positionSize.height));
  mAdaptor->SurfaceResizeComplete(mSurface.get(), Adaptor::SurfaceSize(positionSize.width, positionSize.height));
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
  mAdaptor->RenderOnce();
}

void Window::OnUpdatePositionSize(Dali::PositionSize& positionSize)
{
  // It is from configure notification event, window move/resize signal will be emit.
  // Just Update position size in window render surface without emit move/resize signal.
  UpdatePositionSize(positionSize, false);
}

void Window::OnTouchPoint(Dali::Integration::Point& point, int timeStamp)
{
  FeedTouchPoint(point, timeStamp);
}

void Window::OnMouseFrameEvent()
{
  FeedMouseFrameEvent();
}

void Window::OnWheelEvent(Dali::Integration::WheelEvent& wheelEvent)
{
  FeedWheelEvent(wheelEvent);
}

void Window::OnKeyEvent(Dali::Integration::KeyEvent& keyEvent)
{
  mLastKeyEvent = Dali::DevelKeyEvent::New(keyEvent.keyName, keyEvent.logicalKey, keyEvent.keyString, keyEvent.keyCode, keyEvent.keyModifier, keyEvent.time, static_cast<Dali::KeyEvent::State>(keyEvent.state), keyEvent.compose, keyEvent.deviceName, keyEvent.deviceClass, keyEvent.deviceSubclass);
  DevelKeyEvent::SetWindowId(mLastKeyEvent, keyEvent.windowId);
  FeedKeyEvent(keyEvent);
}

void Window::OnMouseInOutEvent(const Dali::DevelWindow::MouseInOutEvent& mouseInOutEvent)
{
  Dali::Window handle(this);

  mMouseInOutEventSignal.Emit(handle, mouseInOutEvent);
}

void Window::OnMouseRelativeEvent(const Dali::DevelWindow::MouseRelativeEvent& mouseRelativeEvent)
{
  Dali::Window handle(this);

  mMouseRelativeEventSignal.Emit(handle, mouseRelativeEvent);
}

void Window::OnPointerConstraints(const Dali::Int32Pair& position, bool locked, bool confined)
{
  Dali::Window handle(this);

  Vector2                                    newPosition = RecalculatePosition(Vector2(position.GetX(), position.GetY()));
  Dali::DevelWindow::PointerConstraintsEvent pointerConstraintsEvent(static_cast<int32_t>(newPosition.x), static_cast<int32_t>(newPosition.y), locked, confined);

  mPointerConstraintsSignal.Emit(handle, pointerConstraintsEvent);
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

  mAdaptor->SurfaceResizePrepare(mSurface.get(), Adaptor::SurfaceSize(mWindowWidth, mWindowHeight));

  Dali::Window handle(this);
  mResizeSignal.Emit(handle, Dali::Window::WindowSize(mWindowWidth, mWindowHeight));
  mOrientationChangedSignal.Emit(handle, GetCurrentOrientation());

  mAdaptor->SurfaceResizeComplete(mSurface.get(), Adaptor::SurfaceSize(mWindowWidth, mWindowHeight));
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

void Window::OnInsetsChanged(WindowInsetsPartType partType, WindowInsetsPartState partState, const Extents& insets)
{
  mInsetsChangedSignal.Emit(partType, partState, insets);
}

void Window::OnAccessibilityEnabled()
{
  auto bridge    = Accessibility::Bridge::GetCurrentBridge();
  auto rootLayer = mScene.GetRootLayer();
  if(auto accessible = Accessibility::Accessible::Get(rootLayer))
  {
    bridge->AddTopLevelWindow(accessible);
  }
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Accessibility is enabled\n", this, mNativeWindowId);

  InterceptKeyEventSignal().Connect(this, &Window::OnAccessibilityInterceptKeyEvent);

  Dali::Window handle(this);
  if(!mIsEmittedWindowCreatedEvent)
  {
    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Emit Accessbility Window Created Event\n", this, mNativeWindowId);
    bridge->WindowCreated(handle);
    mIsEmittedWindowCreatedEvent = true;
  }

  if(IsVisible())
  {
    bridge->WindowShown(handle);

    if(mFocused)
    {
      DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Emit Accessbility Window Focused Event\n", this, mNativeWindowId);
      bridge->WindowFocused(handle);
    }
  }
  else
  {
    bridge->WindowHidden(handle);
  }
}

void Window::OnAccessibilityDisabled()
{
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Accessibility is disabled\n", this, mNativeWindowId);

  InterceptKeyEventSignal().Disconnect(this, &Window::OnAccessibilityInterceptKeyEvent);
}

bool Window::OnAccessibilityInterceptKeyEvent(const Dali::KeyEvent& keyEvent)
{
  auto bridge = Accessibility::Bridge::GetCurrentBridge();

  if(!bridge->IsUp() || keyEvent.IsNoInterceptModifier())
  {
    DALI_LOG_ERROR("This KeyEvent should not have been intercepted!");

    return false;
  }

  auto callback = [handle = Dali::Window(this)](Dali::KeyEvent keyEvent, bool consumed) {
    if(!consumed)
    {
      Dali::DevelKeyEvent::SetNoInterceptModifier(keyEvent, true);
      Dali::DevelWindow::FeedKeyEvent(handle, keyEvent);
    }
  };

  bool emitted = bridge->EmitKeyEvent(keyEvent, callback);

  // If emitted, consume the event in order not to have to wait for the D-Bus call
  // to finish. If the event turns out not to be consumed by the remote client,
  // then it is fed back to the window from the D-Bus callback.
  return emitted;
}

void Window::OnMoveCompleted(Dali::Window::WindowPosition& position)
{
  Dali::Window handle(this);

  PositionSize positionSize = GetPositionSize();
  positionSize.x = position.GetX();
  positionSize.y = position.GetY();

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Window move completed, (%d, %d)\n",  this, mNativeWindowId, positionSize.x, positionSize.y);

  // It is from window move completed event. This event is generated by displayer server
  // Window move signal is emitted, too.
  // Just Update position size in window render surface without emit move/resize signal.
  UpdatePositionSize(positionSize, false);

  mMoveCompletedSignal.Emit(handle, position);
}

void Window::OnResizeCompleted(Dali::Window::WindowSize& size)
{
  Dali::Window handle(this);

  PositionSize positionSize = GetPositionSize();
  positionSize.width = size.GetWidth();
  positionSize.height = size.GetHeight();

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Window resize completed, (%d x %d)\n",  this, mNativeWindowId, positionSize.width, positionSize.height);

  // It is from window resized completed event. This event is generated by displayer server.
  // Window resize signal is emitted, too.
  // Just Update position size in window render surface without emit move/resize signal.
  UpdatePositionSize(positionSize, false);

  mResizeCompletedSignal.Emit(handle, size);
}

Vector2 Window::RecalculatePosition(const Vector2& position)
{
  Vector2 convertedPosition;

  // Note: We need to subtract 1 from width and height because the range of touch coordinates is from to width - 1 or height - 1
  switch(mRotationAngle)
  {
    case 90:
    {
      convertedPosition.x = static_cast<float>(mWindowWidth - 1) - position.y;
      convertedPosition.y = position.x;
      break;
    }
    case 180:
    {
      convertedPosition.x = static_cast<float>(mWindowWidth - 1) - position.x;
      convertedPosition.y = static_cast<float>(mWindowHeight - 1) - position.y;
      break;
    }
    case 270:
    {
      convertedPosition.x = position.y;
      convertedPosition.y = static_cast<float>(mWindowHeight - 1) - position.x;
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
  SetUserGeometryPolicy();
  mWindowBase->RequestMoveToServer();
}

void Window::RequestResizeToServer(WindowResizeDirection direction)
{
  SetUserGeometryPolicy();
  mWindowBase->RequestResizeToServer(direction);
}

void Window::EnableFloatingMode(bool enable)
{
  mWindowBase->EnableFloatingMode(enable);
}

bool Window::IsFloatingModeEnabled()
{
  return mWindowBase->IsFloatingModeEnabled();
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

void Window::SetUserGeometryPolicy()
{
  if(mIsEnabledUserGeometry == true)
  {
    return;
  }

  mIsEnabledUserGeometry = true;
  AddAuxiliaryHint("wm.policy.win.user.geometry", "1");
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), window user.geometry is changed\n", this, mNativeWindowId);
}

bool Window::PointerConstraintsLock()
{
  return mWindowBase->PointerConstraintsLock();
}

bool Window::PointerConstraintsUnlock()
{
  return mWindowBase->PointerConstraintsUnlock();
}

void Window::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
  mWindowBase->LockedPointerRegionSet(x, y, width, height);
}

void Window::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
  mWindowBase->LockedPointerCursorPositionHintSet(x, y);
}

bool Window::PointerWarp(int32_t x, int32_t y)
{
  return mWindowBase->PointerWarp(x, y);
}

void Window::CursorVisibleSet(bool visible)
{
  mWindowBase->CursorVisibleSet(visible);
}

bool Window::KeyboardGrab(Device::Subclass::Type deviceSubclass)
{
  return mWindowBase->KeyboardGrab(deviceSubclass);
}

bool Window::KeyboardUnGrab()
{
  return mWindowBase->KeyboardUnGrab();
}

void Window::SetFullScreen(bool fullscreen)
{
  mWindowBase->SetFullScreen(fullscreen);
}

bool Window::GetFullScreen()
{
  return mWindowBase->GetFullScreen();
}

void Window::SetFrontBufferRendering(bool enable)
{
  mWindowBase->SetFrontBufferRendering(enable);
  mWindowSurface->SetFrontBufferRendering(enable);
}

bool Window::GetFrontBufferRendering()
{
  return mWindowBase->GetFrontBufferRendering();
}

void Window::SetModal(bool modal)
{
  mWindowBase->SetModal(modal);
}

bool Window::IsModal()
{
  return mWindowBase->IsModal();
}

void Window::SetAlwaysOnTop(bool alwaysOnTop)
{
  mWindowBase->SetAlwaysOnTop(alwaysOnTop);
}

bool Window::IsAlwaysOnTop()
{
  return mWindowBase->IsAlwaysOnTop();
}

void Window::SetBottom(bool enable)
{
  mWindowBase->SetBottom(enable);
}

bool Window::IsBottom()
{
  return mWindowBase->IsBottom();
}

Dali::Any Window::GetNativeBuffer() const
{
  return mWindowBase->GetNativeBuffer();
}

bool Window::RelativeMotionGrab(uint32_t boundary)
{
  return mWindowBase->RelativeMotionGrab(boundary);
}

bool Window::RelativeMotionUnGrab()
{
  return mWindowBase->RelativeMotionUnGrab();
}

void Window::SetBlur(const WindowBlurInfo& blurInfo)
{
  if(mBlurInfo == blurInfo)
  {
    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), same window blur Info\n", this, mNativeWindowId);
    return;
  }

  mBlurInfo = blurInfo;

  if(mBlurInfo.windowBlurType == WindowBlurType::BACKGROUND)
  {
    mWindowBase->SetBackgroundBlur(mBlurInfo.windowBlurRadius, mBlurInfo.backgroundCornerRadius);
  }
  else if(mBlurInfo.windowBlurType == WindowBlurType::BEHIND)
  {
    mWindowBase->SetBehindBlur(mBlurInfo.windowBlurRadius);
  }
  else
  {
    mWindowBase->SetBackgroundBlur(0, 0);
  }
}

WindowBlurInfo Window::GetBlur() const
{
  return mBlurInfo;
}

int Window::GetCurrentWindowRotationAngle() const
{
  return mRotationAngle;
}

int Window::GetCurrentScreenRotationAngle() const
{
  return mWindowBase->GetScreenRotationAngle();
}

void Window::UpdatePositionSize(Dali::PositionSize& positionSize, bool requestChangeGeometry)
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
    if(requestChangeGeometry)
    {
      DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), current angle (%d), position or size is updated by Application , (%d, %d) [%d x %d]\n", this, mNativeWindowId, mRotationAngle, newRect.x, newRect.y, newRect.width, newRect.height);
      mWindowSurface->MoveResize(positionSize);
    }
    else
    {
      DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), current angle (%d), position or size is updated by server , (%d, %d) [%d x %d]\n", this, mNativeWindowId, mRotationAngle, newRect.x, newRect.y, newRect.width, newRect.height);
      mWindowSurface->UpdatePositionSize(positionSize);
    }
  }

  if(moved)
  {
    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Moved signal emit (%d, %d)\n", this, mNativeWindowId, newRect.x, newRect.y);
    Dali::Window::WindowPosition position(newRect.x, newRect.y);
    mMovedSignal.Emit(handle, position);
  }

  // When surface size is updated, inform adaptor of resizing and emit ResizeSignal
  if(resize)
  {
    Uint16Pair newSize(newRect.width, newRect.height);

    mWindowWidth  = newRect.width;
    mWindowHeight = newRect.height;

    SurfaceResized(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));

    mAdaptor->SurfaceResizePrepare(mSurface.get(), newSize);

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Resized signal emit [%d x %d]\n", this, mNativeWindowId, newRect.width, newRect.height);
    mResizeSignal.Emit(handle, newSize);

    mAdaptor->SurfaceResizeComplete(mSurface.get(), newSize);
  }

  if((moved || resize) && Dali::Accessibility::IsUp())
  {
    if(auto accessible = dynamic_cast<Accessibility::ActorAccessible*>(Accessibility::Accessible::Get(mScene.GetRootLayer())))
    {
      accessible->EmitBoundsChanged(Dali::Rect<>(positionSize.x, positionSize.y, positionSize.width, positionSize.height));
    }
  }

  mSurface->SetFullSwapNextFrame();
}


} // namespace Adaptor

} // namespace Internal

} // namespace Dali
