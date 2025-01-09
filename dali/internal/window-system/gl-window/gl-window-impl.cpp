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
#include <dali/internal/window-system/gl-window/gl-window-impl.h>

// EXTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/devel-api/adaptor-framework/orientation.h>
#include <dali/devel-api/events/key-event-devel.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-integ.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#include <dali/internal/window-system/common/display-utils.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/orientation-impl.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/common/window-visibility-observer.h>
#include <dali/internal/window-system/gl-window/gl-window-render-thread.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const int MINIMUM_DIMENSION_CHANGE(1);

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW");
#endif

} // unnamed namespace

GlWindow* GlWindow::New(const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent)
{
  GlWindow* window       = new GlWindow();
  window->mIsTransparent = isTransparent;
  window->Initialize(positionSize, name, className);
  return window;
}

GlWindow::GlWindow()
: mWindowBase(),
  mGraphics(),
  mDisplayConnection(nullptr),
  mGlWindowRenderThread(nullptr),
  mEventHandler(nullptr),
  mIsTransparent(false),
  mIsFocusAcceptable(false),
  mIconified(false),
  mOpaqueState(false),
  mResizeEnabled(false),
  mVisible(false),
  mIsWindowRotated(false),
  mIsTouched(false),
  mIsEGLInitialized(false),
  mDepth(false),
  mStencil(false),
  mPositionSize(),
  mAvailableAngles(),
  mColorDepth(COLOR_DEPTH_24),
  mRenderingMode(Dali::GlWindow::RenderingMode::CONTINUOUS),
  mPreferredAngle(0),
  mTotalRotationAngle(0),
  mWindowRotationAngle(0),
  mScreenRotationAngle(0),
  mOrientationMode(0),
  mWindowWidth(0),
  mWindowHeight(0),
  mNativeWindowId(-1),
  mMSAA(0),
  mKeyEventSignal(),
  mTouchedSignal(),
  mFocusChangeSignal(),
  mResizeSignal(),
  mVisibilityChangedSignal()
{
}

GlWindow::~GlWindow()
{
  if(mEventHandler)
  {
    mEventHandler->RemoveObserver(*this);
  }

  if(mGlWindowRenderThread)
  {
    mGlWindowRenderThread->Stop();
    mGlWindowRenderThread->Join();
  }

  if(mIsEGLInitialized)
  {
    mGraphics->Destroy();
  }
}

void GlWindow::Initialize(const PositionSize& positionSize, const std::string& name, const std::string& className)
{
  int screenWidth, screenHeight;

  mPositionSize = positionSize;
  WindowSystem::GetScreenSize(screenWidth, screenHeight);
  if((mPositionSize.width == 0) || (mPositionSize.height == 0))
  {
    mPositionSize.x      = 0;
    mPositionSize.y      = 0;
    mPositionSize.width  = screenWidth;
    mPositionSize.height = screenHeight;
  }

  if(screenWidth > screenHeight)
  {
    mOrientationMode = 1; // Default mode is landscape
  }
  else
  {
    mOrientationMode = 0; // Default mode is portrate
  }

  // Create a window base
  auto windowFactory = Dali::Internal::Adaptor::GetWindowFactory();
  Any  surface;
  mWindowBase = windowFactory->CreateWindowBase(mPositionSize, surface, (mIsTransparent ? true : false));
  mWindowBase->IconifyChangedSignal().Connect(this, &GlWindow::OnIconifyChanged);
  mWindowBase->FocusChangedSignal().Connect(this, &GlWindow::OnFocusChanged);
  mWindowBase->OutputTransformedSignal().Connect(this, &GlWindow::OnOutputTransformed);

  if(Dali::Adaptor::IsAvailable())
  {
    SetEventHandler();
  }

  if(!mPositionSize.IsEmpty())
  {
    AddAuxiliaryHint("wm.policy.win.user.geometry", "1");
    mResizeEnabled = true;
  }

  mWindowBase->Show();

  if(mIsTransparent)
  {
    mColorDepth = COLOR_DEPTH_32;
  }
  else
  {
    mColorDepth = COLOR_DEPTH_24;
  }

  SetClass(name, className);

  // For Debugging
  mNativeWindowId = mWindowBase->GetNativeWindowId();
}

void GlWindow::SetEventHandler()
{
  mEventHandler = EventHandlerPtr(new EventHandler(mWindowBase.get(), *this));
  mEventHandler->AddObserver(*this);
}

void GlWindow::SetClass(const std::string& name, const std::string className)
{
  mName      = name;
  mClassName = className;
  mWindowBase->SetClass(name, className);
}

void GlWindow::SetGraphicsConfig(bool depth, bool stencil, int msaa, Dali::GlWindow::GlesVersion version)
{
  // Init Graphics
  mDepth   = depth;
  mStencil = stencil;
  mMSAA    = msaa;

  InitializeGraphics();

  int rVersion = 30;

  if(version == Dali::GlWindow::GlesVersion::VERSION_2_0)
  {
    rVersion = 20;
  }
  else if(version == Dali::GlWindow::GlesVersion::VERSION_3_0)
  {
    rVersion = 30;
  }

  mGlWindowRenderThread->SetGraphicsConfig(depth, stencil, msaa, rVersion);
}

void GlWindow::Raise()
{
  mWindowBase->Raise();
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Raise() \n", this, mNativeWindowId);
}

void GlWindow::Lower()
{
  mWindowBase->Lower();
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Lower() \n", this, mNativeWindowId);
}

void GlWindow::Activate()
{
  mWindowBase->Activate();
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Activate() \n", this, mNativeWindowId);
}

void GlWindow::Show()
{
  mVisible = true;

  mWindowBase->Show();

  if(!mIconified)
  {
    Dali::GlWindow handle(this);
    mVisibilityChangedSignal.Emit(handle, true);
  }

  if(mEventHandler)
  {
    mEventHandler->Resume();
  }

  if(mGlWindowRenderThread)
  {
    mGlWindowRenderThread->Resume();
  }

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Show(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible);
}

void GlWindow::Hide()
{
  mVisible = false;

  mWindowBase->Hide();

  if(!mIconified)
  {
    Dali::GlWindow handle(this);
    mVisibilityChangedSignal.Emit(handle, false);
  }

  if(mEventHandler)
  {
    mEventHandler->Pause();
  }

  if(mGlWindowRenderThread)
  {
    mGlWindowRenderThread->Pause();
  }

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Hide(): iconified = %d, visible = %d\n", this, mNativeWindowId, mIconified, mVisible);
}

unsigned int GlWindow::GetSupportedAuxiliaryHintCount() const
{
  return mWindowBase->GetSupportedAuxiliaryHintCount();
}

std::string GlWindow::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return mWindowBase->GetSupportedAuxiliaryHint(index);
}

unsigned int GlWindow::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  return mWindowBase->AddAuxiliaryHint(hint, value);
}

bool GlWindow::RemoveAuxiliaryHint(unsigned int id)
{
  return mWindowBase->RemoveAuxiliaryHint(id);
}

bool GlWindow::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  return mWindowBase->SetAuxiliaryHintValue(id, value);
}

std::string GlWindow::GetAuxiliaryHintValue(unsigned int id) const
{
  return mWindowBase->GetAuxiliaryHintValue(id);
}

unsigned int GlWindow::GetAuxiliaryHintId(const std::string& hint) const
{
  return mWindowBase->GetAuxiliaryHintId(hint);
}

void GlWindow::SetInputRegion(const Rect<int>& inputRegion)
{
  mWindowBase->SetInputRegion(inputRegion);

  DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "GlWindow::SetInputRegion: x = %d, y = %d, w = %d, h = %d\n", inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height);
}

void GlWindow::SetOpaqueState(bool opaque)
{
  mOpaqueState = opaque;

  mWindowBase->SetOpaqueState(opaque);

  DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "GlWindow::SetOpaqueState: opaque = %d\n", opaque);
}

bool GlWindow::IsOpaqueState() const
{
  return mOpaqueState;
}

void GlWindow::SetPositionSize(PositionSize positionSize)
{
  if(!mResizeEnabled)
  {
    AddAuxiliaryHint("wm.policy.win.user.geometry", "1");
    mResizeEnabled = true;
  }

  bool needToMove   = false;
  bool needToResize = false;

  // Check moving
  if((fabs(positionSize.x - mPositionSize.x) > MINIMUM_DIMENSION_CHANGE) ||
     (fabs(positionSize.y - mPositionSize.y) > MINIMUM_DIMENSION_CHANGE))
  {
    needToMove = true;
  }

  // Check resizing
  if((fabs(positionSize.width - mPositionSize.width) > MINIMUM_DIMENSION_CHANGE) ||
     (fabs(positionSize.height - mPositionSize.height) > MINIMUM_DIMENSION_CHANGE))
  {
    needToResize = true;
  }

  if(needToResize)
  {
    if(needToMove)
    {
      mWindowBase->MoveResize(positionSize);
    }
    else
    {
      mWindowBase->Resize(positionSize);
    }
    mPositionSize = positionSize;
  }
  else
  {
    if(needToMove)
    {
      mWindowBase->Move(positionSize);
      mPositionSize = positionSize;
    }
  }

  // If window's size or position is changed, the signal will be emitted to user.
  if(needToMove || needToResize)
  {
    Uint16Pair     newSize(mPositionSize.width, mPositionSize.height);
    Dali::GlWindow handle(this);
    mResizeSignal.Emit(newSize);

    if(mGlWindowRenderThread)
    {
      mGlWindowRenderThread->RequestWindowResize(mPositionSize.width, mPositionSize.height);
    }
  }
}

PositionSize GlWindow::GetPositionSize() const
{
  PositionSize positionSize(mPositionSize);
  if(mTotalRotationAngle == 90 || mTotalRotationAngle == 270)
  {
    positionSize.height = mPositionSize.width;
    positionSize.width  = mPositionSize.height;
  }

  return positionSize;
}

void GlWindow::OnIconifyChanged(bool iconified)
{
  if(iconified)
  {
    mIconified = true;

    if(mVisible)
    {
      Dali::GlWindow handle(this);
      mVisibilityChangedSignal.Emit(handle, false);
    }

    if(mEventHandler)
    {
      mEventHandler->Pause();
    }

    if(mGlWindowRenderThread)
    {
      mGlWindowRenderThread->Pause();
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Iconified: visible = %d\n", this, mNativeWindowId, mVisible);
  }
  else
  {
    mIconified = false;

    if(mVisible)
    {
      Dali::GlWindow handle(this);
      mVisibilityChangedSignal.Emit(handle, true);
    }

    if(mEventHandler)
    {
      mEventHandler->Resume();
    }

    if(mGlWindowRenderThread)
    {
      mGlWindowRenderThread->Resume();
    }

    DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), Deiconified: visible = %d\n", this, mNativeWindowId, mVisible);
  }
}

void GlWindow::OnFocusChanged(bool focusIn)
{
  Dali::GlWindow handle(this);
  mFocusChangeSignal.Emit(handle, focusIn);
}

void GlWindow::OnOutputTransformed()
{
  int newScreenRotationAngle = mWindowBase->GetScreenRotationAngle();
  DALI_LOG_RELEASE_INFO("GlWindow::OnOutputTransformed(), screen rotation occurs, old[%d], new[%d\n", mScreenRotationAngle, newScreenRotationAngle);

  if(newScreenRotationAngle != mScreenRotationAngle)
  {
    UpdateScreenRotation(newScreenRotationAngle);
  }
}

void GlWindow::OnTouchPoint(Dali::Integration::Point& point, int timeStamp)
{
  PointState::Type state = point.GetState();

  if(state == PointState::DOWN)
  {
    mIsTouched = true;
  }

  if(state == PointState::UP)
  {
    mIsTouched = false;
  }

  if(!mIsTouched && state == PointState::MOTION)
  {
    return;
  }

  Vector2 convertedPosition = RecalculatePosition(point.GetScreenPosition());
  point.SetScreenPosition(convertedPosition);

  Dali::TouchEvent touchEvent = Dali::Integration::NewTouchEvent(timeStamp, point);
  Dali::GlWindow   handle(this);
  mTouchedSignal.Emit(touchEvent);
}

void GlWindow::OnMouseFrameEvent()
{
}

void GlWindow::OnWheelEvent(Dali::Integration::WheelEvent& wheelEvent)
{
  // TODO:
  //FeedWheelEvent( wheelEvent );
}

void GlWindow::OnKeyEvent(Dali::Integration::KeyEvent& keyEvent)
{
  Dali::KeyEvent event = Dali::DevelKeyEvent::New(keyEvent.keyName, keyEvent.logicalKey, keyEvent.keyString, keyEvent.keyCode, keyEvent.keyModifier, keyEvent.time, static_cast<Dali::KeyEvent::State>(keyEvent.state), keyEvent.compose, keyEvent.deviceName, keyEvent.deviceClass, keyEvent.deviceSubclass);
  Dali::DevelKeyEvent::SetWindowId(event, keyEvent.windowId);
  Dali::GlWindow handle(this);
  mKeyEventSignal.Emit(event);
}

void GlWindow::OnRotation(const RotationEvent& rotation)
{
  mWindowRotationAngle = rotation.angle;
  mTotalRotationAngle  = (mWindowRotationAngle + mScreenRotationAngle) % 360;
  if(mTotalRotationAngle == 90 || mTotalRotationAngle == 270)
  {
    mWindowWidth  = mPositionSize.height;
    mWindowHeight = mPositionSize.width;
  }
  else
  {
    mWindowWidth  = mPositionSize.width;
    mWindowHeight = mPositionSize.height;
  }

  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), OnRotation(): resize signal emit [%d x %d]\n", this, mNativeWindowId, mWindowWidth, mWindowHeight);

  // Emit Resize signal
  Dali::GlWindow handle(this);
  mResizeSignal.Emit(Dali::Uint16Pair(mWindowWidth, mWindowHeight));

  if(mGlWindowRenderThread)
  {
    mGlWindowRenderThread->RequestWindowRotate(mWindowRotationAngle);
  }
}

Vector2 GlWindow::RecalculatePosition(const Vector2& position)
{
  Vector2 convertedPosition;

  switch(mTotalRotationAngle)
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

void GlWindow::SetAvailableAnlges(const std::vector<int>& angles)
{
  if(angles.size() > 4)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::SetAvailableAnlges: Invalid vector size! [%d]\n", angles.size());
    return;
  }

  mWindowBase->SetAvailableAnlges(angles);
}

bool GlWindow::IsOrientationAvailable(WindowOrientation orientation) const
{
  if(orientation <= WindowOrientation::NO_ORIENTATION_PREFERENCE || orientation > WindowOrientation::LANDSCAPE_INVERSE)
  {
    DALI_LOG_INFO(gWindowLogFilter, Debug::Verbose, "Window::IsOrientationAvailable: Invalid input orientation [%d]\n", orientation);
    return false;
  }
  return true;
}

int GlWindow::ConvertToAngle(WindowOrientation orientation)
{
  int convertAngle = 0;
  if(mOrientationMode == 0)
  {
    convertAngle = static_cast<int>(orientation);
  }
  else if(mOrientationMode == 1)
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

WindowOrientation GlWindow::ConvertToOrientation(int angle) const
{
  WindowOrientation orientation = WindowOrientation::NO_ORIENTATION_PREFERENCE;
  if(mOrientationMode == 0) // Portrate mode
  {
    orientation = static_cast<WindowOrientation>(angle);
  }
  else if(mOrientationMode == 1) // Landscape mode
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

WindowOrientation GlWindow::GetCurrentOrientation() const
{
  return ConvertToOrientation(mTotalRotationAngle);
}

void GlWindow::SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations)
{
  Dali::Vector<float>::SizeType count = orientations.Count();
  for(Dali::Vector<float>::SizeType index = 0; index < count; ++index)
  {
    if(IsOrientationAvailable(orientations[index]) == false)
    {
      DALI_LOG_ERROR("Window::SetAvailableRotationAngles, invalid angle: %d\n", orientations[index]);
      continue;
    }

    bool found = false;
    int  angle = ConvertToAngle(orientations[index]);

    for(std::size_t i = 0; i < mAvailableAngles.size(); i++)
    {
      if(mAvailableAngles[i] == angle)
      {
        found = true;
        break;
      }
    }

    if(!found)
    {
      DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SetAvailableOrientations: %d\n", this, mNativeWindowId, angle);
      mAvailableAngles.push_back(angle);
    }
  }
  SetAvailableAnlges(mAvailableAngles);
}

void GlWindow::SetPreferredOrientation(WindowOrientation orientation)
{
  if(IsOrientationAvailable(orientation) == false)
  {
    DALI_LOG_ERROR("Window::SetPreferredOrientation, invalid orientation: %d\n", orientation);
    return;
  }
  mPreferredAngle = ConvertToAngle(orientation);
  DALI_LOG_RELEASE_INFO("Window (%p), WinId (%d), SetPreferredOrientation: %d\n", this, mNativeWindowId, mPreferredAngle);
  mWindowBase->SetPreferredAngle(mPreferredAngle);
}

void GlWindow::SetChild(Dali::Window& child)
{
  if(DALI_UNLIKELY(child))
  {
    mChildWindow                             = child;
    Internal::Adaptor::Window& windowImpl    = Dali::GetImplementation(mChildWindow);
    WindowRenderSurface*       renderSurface = static_cast<WindowRenderSurface*>(windowImpl.GetSurface());
    if(renderSurface)
    {
      WindowBase* childWindowBase = renderSurface->GetWindowBase();
      if(childWindowBase)
      {
        childWindowBase->SetParent(mWindowBase.get(), false);
      }
    }
  }
}

void GlWindow::RegisterGlCallbacks(CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback)
{
  if(mIsEGLInitialized == false)
  {
    InitializeGraphics();
  }
  mGlWindowRenderThread->RegisterGlCallbacks(initCallback, renderFrameCallback, terminateCallback);
  mGlWindowRenderThread->Start();
}

void GlWindow::RenderOnce()
{
  if(mGlWindowRenderThread)
  {
    mGlWindowRenderThread->RenderOnce();
  }
}

void GlWindow::SetRenderingMode(Dali::GlWindow::RenderingMode mode)
{
  mRenderingMode = mode;
  if(mGlWindowRenderThread)
  {
    bool onDemand = false;
    if(mRenderingMode == Dali::GlWindow::RenderingMode::ON_DEMAND)
    {
      onDemand = true;
    }
    mGlWindowRenderThread->SetOnDemandRenderMode(onDemand);
  }
}

Dali::GlWindow::RenderingMode GlWindow::GetRenderingMode() const
{
  return mRenderingMode;
}

void GlWindow::InitializeGraphics()
{
  if(!mIsEGLInitialized)
  {
    // Init Graphics
    std::unique_ptr<EglGraphicsFactory> graphicsFactoryPtr = Utils::MakeUnique<EglGraphicsFactory>(mEnvironmentOptions);
    auto                                graphicsFactory    = *graphicsFactoryPtr;

    mGraphics = std::unique_ptr<Graphics::GraphicsInterface>(&graphicsFactory.Create());

    Graphics::GraphicsInterface* graphics = mGraphics.get();

    mDisplayConnection = std::unique_ptr<Dali::DisplayConnection>(Dali::DisplayConnection::New(Dali::Integration::RenderSurfaceInterface::Type::WINDOW_RENDER_SURFACE));
    graphics->Initialize(*mDisplayConnection, mDepth, mStencil, false, mMSAA);

    // Create Render Thread
    mGlWindowRenderThread = std::unique_ptr<Dali::Internal::Adaptor::GlWindowRenderThread>(new GlWindowRenderThread(mPositionSize, mColorDepth));
    if(!mGlWindowRenderThread)
    {
      DALI_LOG_ERROR("Fail to create GlWindow Render Thread!!!!\n");
      return;
    }

    mGlWindowRenderThread->SetGraphicsInterface(graphics);
    mGlWindowRenderThread->SetWindowBase(mWindowBase.get());
    bool onDemand = false;
    if(mRenderingMode == Dali::GlWindow::RenderingMode::ON_DEMAND)
    {
      onDemand = true;
    }
    mGlWindowRenderThread->SetOnDemandRenderMode(onDemand);

    mIsEGLInitialized = true;

    // Check screen rotation
    int newScreenRotationAngle = mWindowBase->GetScreenRotationAngle();
    DALI_LOG_RELEASE_INFO("GlWindow::InitializeGraphics(), GetScreenRotationAngle(): %d\n", mScreenRotationAngle);
    if(newScreenRotationAngle != 0)
    {
      UpdateScreenRotation(newScreenRotationAngle);
    }
  }
}

void GlWindow::OnDamaged(const DamageArea& area)
{
}

void GlWindow::UpdateScreenRotation(int newAngle)
{
  mScreenRotationAngle = newAngle;
  mTotalRotationAngle  = (mWindowRotationAngle + mScreenRotationAngle) % 360;

  if(mTotalRotationAngle == 90 || mTotalRotationAngle == 270)
  {
    mWindowWidth  = mPositionSize.height;
    mWindowHeight = mPositionSize.width;
  }
  else
  {
    mWindowWidth  = mPositionSize.width;
    mWindowHeight = mPositionSize.height;
  }

  // Emit Resize signal
  Dali::GlWindow handle(this);
  mResizeSignal.Emit(Dali::Uint16Pair(mWindowWidth, mWindowHeight));

  if(mGlWindowRenderThread)
  {
    DALI_LOG_RELEASE_INFO("GlWindow::UpdateScreenRotation(), RequestScreenRotatem(), mScreenRotationAngle: %d\n", mScreenRotationAngle);
    mGlWindowRenderThread->RequestScreenRotate(mScreenRotationAngle);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
