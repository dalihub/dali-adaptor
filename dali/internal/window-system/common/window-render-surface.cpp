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
#include <dali/internal/window-system/common/window-render-surface.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-factory.h>
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/common/window-factory.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const int   MINIMUM_DIMENSION_CHANGE(1); ///< Minimum change for window to be considered to have moved
const float FULL_UPDATE_RATIO(0.8f);     ///< Force full update when the dirty area is larget than this ratio

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_WINDOW_RENDER_SURFACE");
#endif

void InsertRects(WindowRenderSurface::DamagedRectsContainer& damagedRectsList, const Rect<int>& damagedRects)
{
  damagedRectsList.insert(damagedRectsList.begin(), damagedRects);
  if(damagedRectsList.size() > 4) // past triple buffers + current
  {
    damagedRectsList.pop_back();
  }
}

Rect<int32_t> RecalculateRect0(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  return rect;
}

Rect<int32_t> RecalculateRect90(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  Rect<int32_t> newRect;
  newRect.x      = surfaceSize.height - (rect.y + rect.height);
  newRect.y      = rect.x;
  newRect.width  = rect.height;
  newRect.height = rect.width;
  return newRect;
}

Rect<int32_t> RecalculateRect180(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  Rect<int32_t> newRect;
  newRect.x      = surfaceSize.width - (rect.x + rect.width);
  newRect.y      = surfaceSize.height - (rect.y + rect.height);
  newRect.width  = rect.width;
  newRect.height = rect.height;
  return newRect;
}

Rect<int32_t> RecalculateRect270(Rect<int32_t>& rect, const Rect<int32_t>& surfaceSize)
{
  Rect<int32_t> newRect;
  newRect.x      = rect.y;
  newRect.y      = surfaceSize.width - (rect.x + rect.width);
  newRect.width  = rect.height;
  newRect.height = rect.width;
  return newRect;
}

using RecalculateRectFunction = Rect<int32_t> (*)(Rect<int32_t>&, const Rect<int32_t>&);

RecalculateRectFunction RecalculateRect[4] = {RecalculateRect0, RecalculateRect90, RecalculateRect180, RecalculateRect270};

void MergeIntersectingRectsAndRotate(Rect<int>& mergingRect, std::vector<Rect<int>>& damagedRects, int orientation, const Rect<int32_t>& surfaceRect)
{
  const int n = damagedRects.size();
  for(int i = 0; i < n - 1; i++)
  {
    if(damagedRects[i].IsEmpty())
    {
      continue;
    }

    for(int j = i + 1; j < n; j++)
    {
      if(damagedRects[j].IsEmpty())
      {
        continue;
      }

      if(damagedRects[i].Intersects(damagedRects[j]))
      {
        damagedRects[i].Merge(damagedRects[j]);
        damagedRects[j].width  = 0;
        damagedRects[j].height = 0;
      }
    }
  }

  int j = 0;
  for(int i = 0; i < n; i++)
  {
    if(!damagedRects[i].IsEmpty())
    {
      // Merge rects before rotate
      if(mergingRect.IsEmpty())
      {
        mergingRect = damagedRects[i];
      }
      else
      {
        mergingRect.Merge(damagedRects[i]);
      }

      damagedRects[j++] = RecalculateRect[orientation](damagedRects[i], surfaceRect);
    }
  }

  if(j != 0)
  {
    damagedRects.resize(j);
  }
}

} // unnamed namespace

WindowRenderSurface::WindowRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mEGL(nullptr),
  mDisplayConnection(nullptr),
  mPositionSize(positionSize),
  mWindowBase(),
  mThreadSynchronization(nullptr),
  mRenderNotification(nullptr),
  mPostRenderTrigger(),
  mFrameRenderedTrigger(),
  mGraphics(nullptr),
  mEGLSurface(nullptr),
  mEGLContext(nullptr),
  mColorDepth(isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24),
  mOutputTransformedSignal(),
  mWindowRotationFinishedSignal(),
  mFrameCallbackInfoContainer(),
  mBufferDamagedRects(),
  mMutex(),
  mWindowRotationAngle(0),
  mScreenRotationAngle(0),
  mDpiHorizontal(0),
  mDpiVertical(0),
  mOwnSurface(false),
  mIsImeWindowSurface(false),
  mNeedWindowRotationAcknowledgement(false),
  mIsWindowOrientationChanging(false),
  mIsFrontBufferRendering(false),
  mIsFrontBufferRenderingChanged(false)
{
  DALI_LOG_INFO(gWindowRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n");
  Initialize(surface);
}

WindowRenderSurface::~WindowRenderSurface()
{
}

void WindowRenderSurface::Initialize(Any surface)
{
  // If width or height are zero, go full screen.
  if((mPositionSize.width == 0) || (mPositionSize.height == 0))
  {
    // Default window size == screen size
    mPositionSize.x = 0;
    mPositionSize.y = 0;
    WindowSystem::GetScreenSize(mPositionSize.width, mPositionSize.height);
  }

  // Create a window base
  auto windowFactory = Dali::Internal::Adaptor::GetWindowFactory();
  mWindowBase        = windowFactory->CreateWindowBase(mPositionSize, surface, (mColorDepth == COLOR_DEPTH_32 ? true : false));

  // Connect signals
  mWindowBase->OutputTransformedSignal().Connect(this, &WindowRenderSurface::OutputTransformed);

  // Check screen rotation
  int screenRotationAngle = mWindowBase->GetScreenRotationAngle();
  if(screenRotationAngle != 0)
  {
    OutputTransformed();
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::Initialize, screen rotation is enabled, screen rotation angle:[%d]\n", screenRotationAngle);
  }
}

Any WindowRenderSurface::GetNativeWindow()
{
  return mWindowBase->GetNativeWindow();
}

int WindowRenderSurface::GetNativeWindowId()
{
  return mWindowBase->GetNativeWindowId();
}

void WindowRenderSurface::Map()
{
  mWindowBase->Show();
}

void WindowRenderSurface::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

void WindowRenderSurface::SetTransparency(bool transparent)
{
  mWindowBase->SetTransparency(transparent);
}

void WindowRenderSurface::RequestRotation(int angle, PositionSize positionSize)
{
  if(!mPostRenderTrigger)
  {
    mPostRenderTrigger = std::unique_ptr<TriggerEventInterface>(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &WindowRenderSurface::ProcessPostRender),
                                                                                                        TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
  }

  mPositionSize.x = positionSize.x;
  mPositionSize.y = positionSize.y;

  mWindowBase->SetWindowRotationAngle(angle);

  DALI_LOG_RELEASE_INFO("start window rotation angle = %d screen rotation = %d\n", angle, mScreenRotationAngle);
}

WindowBase* WindowRenderSurface::GetWindowBase()
{
  return mWindowBase.get();
}

WindowBase::OutputSignalType& WindowRenderSurface::OutputTransformedSignal()
{
  return mOutputTransformedSignal;
}

WindowRenderSurface::RotationFinishedSignalType& WindowRenderSurface::RotationFinishedSignal()
{
  return mWindowRotationFinishedSignal;
}

PositionSize WindowRenderSurface::GetPositionSize() const
{
  return mPositionSize;
}

void WindowRenderSurface::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  if(mDpiHorizontal == 0 || mDpiVertical == 0)
  {
    const char* environmentDpiHorizontal = std::getenv(DALI_ENV_DPI_HORIZONTAL);
    mDpiHorizontal                       = environmentDpiHorizontal ? std::atoi(environmentDpiHorizontal) : 0;

    const char* environmentDpiVertical = std::getenv(DALI_ENV_DPI_VERTICAL);
    mDpiVertical                       = environmentDpiVertical ? std::atoi(environmentDpiVertical) : 0;

    if(mDpiHorizontal == 0 || mDpiVertical == 0)
    {
      mWindowBase->GetDpi(mDpiHorizontal, mDpiVertical);
    }
  }

  dpiHorizontal = mDpiHorizontal;
  dpiVertical   = mDpiVertical;
}

int WindowRenderSurface::GetSurfaceOrientation() const
{
  return mWindowBase->GetWindowRotationAngle();
}

int WindowRenderSurface::GetScreenOrientation() const
{
  return mWindowBase->GetScreenRotationAngle();
}

void WindowRenderSurface::InitializeGraphics()
{
  if(mEGLContext == NULL)
  {
    mGraphics = &mAdaptor->GetGraphicsInterface();

    DALI_ASSERT_ALWAYS(mGraphics && "Graphics interface is not created");

    auto eglGraphics = static_cast<EglGraphics*>(mGraphics);
    mEGL             = &eglGraphics->GetEglInterface();

    if(mEGL)
    {
      // Create the OpenGL context for this window
      mEGL->ChooseConfig(true, mColorDepth);
      mEGL->CreateWindowContext(mEGLContext);
    }

    // Create the OpenGL surface
    CreateSurface();
  }
}

void WindowRenderSurface::CreateSurface()
{
  DALI_LOG_TRACE_METHOD(gWindowRenderSurfaceLogFilter);

  int width, height;
  if(mScreenRotationAngle == 0 || mScreenRotationAngle == 180)
  {
    width  = mPositionSize.width;
    height = mPositionSize.height;
  }
  else
  {
    width  = mPositionSize.height;
    height = mPositionSize.width;
  }

  // Create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow(width, height);

  if(mWindowBase->GetType() == WindowType::IME)
  {
    InitializeImeSurface();
  }

  if(mEGL)
  {
    mEGLSurface = mEGL->CreateSurfaceWindow(window, mColorDepth);
  }

  DALI_LOG_RELEASE_INFO("WindowRenderSurface::CreateSurface: WinId (%d), EGLSurface (%p), w = %d h = %d angle = %d screen rotation = %d\n",
                        mWindowBase->GetNativeWindowId(),
                        mEGLSurface,
                        mPositionSize.width,
                        mPositionSize.height,
                        mWindowRotationAngle,
                        mScreenRotationAngle);
}

void WindowRenderSurface::DestroySurface()
{
  DALI_LOG_TRACE_METHOD(gWindowRenderSurfaceLogFilter);

  if(mEGL)
  {
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::DestroySurface: WinId (%d)\n", mWindowBase->GetNativeWindowId());

    mEGL->DestroySurface(mEGLSurface);
    mEGLSurface = nullptr;

    // Destroy context also
    mEGL->DestroyContext(mEGLContext);
    mEGLContext = nullptr;

    mWindowBase->DestroyEglWindow();
  }
}

bool WindowRenderSurface::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD(gWindowRenderSurfaceLogFilter);

  // Destroy the old one
  mWindowBase->DestroyEglWindow();

  int width, height;
  if(mScreenRotationAngle == 0 || mScreenRotationAngle == 180)
  {
    width  = mPositionSize.width;
    height = mPositionSize.height;
  }
  else
  {
    width  = mPositionSize.height;
    height = mPositionSize.width;
  }

  // Create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow(width, height);

  if(mEGL)
  {
    return mEGL->ReplaceSurfaceWindow(window, mEGLSurface, mEGLContext);
  }
  return false;
}

void WindowRenderSurface::UpdatePositionSize(Dali::PositionSize positionSize)
{
  // Check moving
  if((fabs(positionSize.x - mPositionSize.x) >= MINIMUM_DIMENSION_CHANGE) ||
     (fabs(positionSize.y - mPositionSize.y) >= MINIMUM_DIMENSION_CHANGE))
  {
    mPositionSize.x = positionSize.x;
    mPositionSize.y = positionSize.y;

    DALI_LOG_RELEASE_INFO("Update Position by server (%d, %d)\n", mPositionSize.x, mPositionSize.y);
  }
}

void WindowRenderSurface::Move(Dali::PositionSize positionSize)
{
  mPositionSize.x = positionSize.x;
  mPositionSize.y = positionSize.y;

  DALI_LOG_RELEASE_INFO("Update Position by client (%d, %d)\n", positionSize.x, positionSize.y);

  mWindowBase->Move(positionSize);
}

void WindowRenderSurface::MoveResize(Dali::PositionSize positionSize)
{
  mPositionSize.x = positionSize.x;
  mPositionSize.y = positionSize.y;

  DALI_LOG_RELEASE_INFO("Update Position by client (%d, %d)\n", positionSize.x, positionSize.y);

  mWindowBase->MoveResize(positionSize);
}

void WindowRenderSurface::StartRender()
{
}

bool WindowRenderSurface::PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  InitializeGraphics();

  Dali::Integration::Scene::FrameCallbackContainer callbacks;

  Dali::Integration::Scene scene = mScene.GetHandle();

  if(scene)
  {
    bool needFrameRenderedTrigger = false;

    scene.GetFrameRenderedCallback(callbacks);
    if(!callbacks.empty())
    {
      int frameRenderedSync = mWindowBase->CreateFrameRenderedSyncFence();
      if(frameRenderedSync != -1)
      {
        Dali::Mutex::ScopedLock lock(mMutex);

        DALI_LOG_RELEASE_INFO("WindowRenderSurface::PreRender: CreateFrameRenderedSyncFence [%d]\n", frameRenderedSync);

        mFrameCallbackInfoContainer.push_back(std::unique_ptr<FrameCallbackInfo>(new FrameCallbackInfo(callbacks, frameRenderedSync)));

        needFrameRenderedTrigger = true;
      }
      else
      {
        DALI_LOG_ERROR("WindowRenderSurface::PreRender: CreateFrameRenderedSyncFence is failed\n");
      }

      // Clear callbacks
      callbacks.clear();
    }

    scene.GetFramePresentedCallback(callbacks);
    if(!callbacks.empty())
    {
      int framePresentedSync = mWindowBase->CreateFramePresentedSyncFence();
      if(framePresentedSync != -1)
      {
        Dali::Mutex::ScopedLock lock(mMutex);

        DALI_LOG_RELEASE_INFO("WindowRenderSurface::PreRender: CreateFramePresentedSyncFence [%d]\n", framePresentedSync);

        mFrameCallbackInfoContainer.push_back(std::unique_ptr<FrameCallbackInfo>(new FrameCallbackInfo(callbacks, framePresentedSync)));

        needFrameRenderedTrigger = true;
      }
      else
      {
        DALI_LOG_ERROR("WindowRenderSurface::PreRender: CreateFramePresentedSyncFence is failed\n");
      }

      // Clear callbacks
      callbacks.clear();
    }

    if(needFrameRenderedTrigger)
    {
      if(!mFrameRenderedTrigger)
      {
        mFrameRenderedTrigger = std::unique_ptr<TriggerEventInterface>(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &WindowRenderSurface::ProcessFrameCallback),
                                                                                                               TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
      }
      mFrameRenderedTrigger->Trigger();
    }
  }

  /**
    * wl_egl_window_tizen_set_rotation(SetEglWindowRotation)                -> PreRotation
    * wl_egl_window_tizen_set_buffer_transform(SetEglWindowBufferTransform) -> Screen Rotation
    * wl_egl_window_tizen_set_window_transform(SetEglWindowTransform)       -> Window Rotation
    * These function should be called before calling first drawing gl Function.
    * Notice : PreRotation is not used in the latest tizen,
    *          because output transform event should be occured before egl window is not created.
    */
  if(scene && resizingSurface)
  {
    int  totalAngle                  = 0;
    bool isScreenOrientationChanging = false;

    if(mWindowRotationAngle != scene.GetCurrentSurfaceOrientation())
    {
      mWindowRotationAngle         = scene.GetCurrentSurfaceOrientation();
      mIsWindowOrientationChanging = true;
    }

    if(mScreenRotationAngle != scene.GetCurrentScreenOrientation())
    {
      mScreenRotationAngle        = scene.GetCurrentScreenOrientation();
      isScreenOrientationChanging = true;
    }
    totalAngle = (mWindowRotationAngle + mScreenRotationAngle) % 360;

    DALI_LOG_RELEASE_INFO("Window/Screen orientation ard changed, WinOrientation[%d],flag[%d], ScreenOrientation[%d],flag[%d], total[%d]\n", mWindowRotationAngle, mIsWindowOrientationChanging, mScreenRotationAngle, isScreenOrientationChanging, totalAngle);

    Rect<int> surfaceSize = scene.GetCurrentSurfaceRect();
    //update surface size
    mPositionSize.width  = surfaceSize.width;
    mPositionSize.height = surfaceSize.height;

    DALI_LOG_RELEASE_INFO("Window is resizing, (%d, %d), [%d x %d], IMEWindow [%d]\n", mPositionSize.x, mPositionSize.y, mPositionSize.width, mPositionSize.height, mIsImeWindowSurface);

    // Window rotate or screen rotate
    if(mIsWindowOrientationChanging || isScreenOrientationChanging)
    {
      mWindowBase->SetEglWindowBufferTransform(totalAngle);
    }

    // Only window rotate
    if(mIsWindowOrientationChanging)
    {
      mWindowBase->SetEglWindowTransform(mWindowRotationAngle);
    }

    // Resize case
    Dali::PositionSize positionSize;

    // Some native resize API(wl_egl_window_resize) has the input parameters of x, y, width and height.
    // So, position data should be set.
    positionSize.x = mPositionSize.x;
    positionSize.y = mPositionSize.y;
    if(totalAngle == 0 || totalAngle == 180)
    {
      positionSize.width  = mPositionSize.width;
      positionSize.height = mPositionSize.height;
    }
    else
    {
      positionSize.width  = mPositionSize.height;
      positionSize.height = mPositionSize.width;
    }

    mWindowBase->ResizeEglWindow(positionSize);

    SetFullSwapNextFrame();
  }

  // When mIsFrontBufferRendering is not equal to mWindowBase's
  if(mIsFrontBufferRenderingChanged)
  {
    mIsFrontBufferRenderingChanged = false;
    mWindowBase->SetEglWindowFrontBufferMode(mIsFrontBufferRendering);
    SetFullSwapNextFrame();
  }

  SetBufferDamagedRects(damagedRects, clippingRect);

  if(scene)
  {
    Rect<int> surfaceRect = scene.GetCurrentSurfaceRect();
    if(clippingRect == surfaceRect)
    {
      int32_t totalAngle = scene.GetCurrentSurfaceOrientation() + scene.GetCurrentScreenOrientation();
      if(totalAngle >= 360)
      {
        totalAngle -= 360;
      }
      mDamagedRects.assign(1, RecalculateRect[std::min(totalAngle / 90, 3)](surfaceRect, surfaceRect));
    }
  }

  return true;
}

void WindowRenderSurface::PostRender()
{
  // Inform the gl implementation that rendering has finished before informing the surface
  auto eglGraphics = static_cast<EglGraphics*>(mGraphics);
  if(eglGraphics)
  {
    auto& gl = eglGraphics->GetGlAbstraction();
    gl.PostRender();

    bool needWindowRotationCompleted = false;

    if(mIsWindowOrientationChanging)
    {
      if(mNeedWindowRotationAcknowledgement)
      {
        Dali::Integration::Scene scene = mScene.GetHandle();
        if(scene)
        {
          if(scene.IsRotationCompletedAcknowledgementSet())
          {
            needWindowRotationCompleted = true;
          }
        }
      }
      else
      {
        needWindowRotationCompleted = true;
      }
    }

    if(needWindowRotationCompleted || mIsImeWindowSurface)
    {
      if(mThreadSynchronization)
      {
        // Enable PostRender flag
        mThreadSynchronization->PostRenderStarted();
      }

      if(mIsWindowOrientationChanging || mIsImeWindowSurface)
      {
        mPostRenderTrigger->Trigger();
      }

      if(mThreadSynchronization)
      {
        // Wait until the event-thread complete the rotation event processing
        mThreadSynchronization->PostRenderWaitForCompletion();
      }
    }

    SwapBuffers(mDamagedRects);

    if(mRenderNotification)
    {
      mRenderNotification->Trigger();
    }
  }
}

void WindowRenderSurface::StopRender()
{
}

void WindowRenderSurface::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
  DALI_LOG_INFO(gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::SetThreadSynchronization: called\n");

  mThreadSynchronization = &threadSynchronization;
}

void WindowRenderSurface::ReleaseLock()
{
  // Nothing to do.
}

Dali::RenderSurfaceInterface::Type WindowRenderSurface::GetSurfaceType()
{
  return Dali::RenderSurfaceInterface::WINDOW_RENDER_SURFACE;
}

void WindowRenderSurface::MakeContextCurrent()
{
  if(mEGL != nullptr)
  {
    mEGL->MakeContextCurrent(mEGLSurface, mEGLContext);
  }
}

Integration::DepthBufferAvailable WindowRenderSurface::GetDepthBufferRequired()
{
  return mGraphics ? mGraphics->GetDepthBufferRequired() : Integration::DepthBufferAvailable::FALSE;
}

Integration::StencilBufferAvailable WindowRenderSurface::GetStencilBufferRequired()
{
  return mGraphics ? mGraphics->GetStencilBufferRequired() : Integration::StencilBufferAvailable::FALSE;
}

void WindowRenderSurface::InitializeImeSurface()
{
  if(!mIsImeWindowSurface)
  {
    mIsImeWindowSurface = true;
    if(!mPostRenderTrigger)
    {
      mPostRenderTrigger = std::unique_ptr<TriggerEventInterface>(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &WindowRenderSurface::ProcessPostRender),
                                                                                                          TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
    }
  }
}

void WindowRenderSurface::SetNeedsRotationCompletedAcknowledgement(bool needAcknowledgement)
{
  mNeedWindowRotationAcknowledgement = needAcknowledgement;
}

void WindowRenderSurface::OutputTransformed()
{
  int screenRotationAngle = mWindowBase->GetScreenRotationAngle();

  if(mScreenRotationAngle != screenRotationAngle)
  {
    mOutputTransformedSignal.Emit();

    DALI_LOG_RELEASE_INFO("WindowRenderSurface::OutputTransformed: window = %d new screen angle = %d\n", mWindowRotationAngle, screenRotationAngle);
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::OutputTransformed: Ignore output transform [%d]\n", mScreenRotationAngle);
  }
}

void WindowRenderSurface::ProcessPostRender()
{
  if(mIsWindowOrientationChanging)
  {
    mWindowRotationFinishedSignal.Emit();
    mWindowBase->WindowRotationCompleted(mWindowRotationAngle, mPositionSize.width, mPositionSize.height);
    mIsWindowOrientationChanging = false;
    DALI_LOG_RELEASE_INFO("WindowRenderSurface::ProcessPostRender: Rotation Done, flag = %d\n", mIsWindowOrientationChanging);
  }

  if(mIsImeWindowSurface)
  {
    mWindowBase->ImeWindowReadyToRender();
  }

  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

void WindowRenderSurface::ProcessFrameCallback()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  for(auto&& iter : mFrameCallbackInfoContainer)
  {
    if(!iter->fileDescriptorMonitor)
    {
      iter->fileDescriptorMonitor = Dali::Internal::Adaptor::GetSystemFactory()->CreateFileDescriptorMonitor(iter->fileDescriptor, MakeCallback(this, &WindowRenderSurface::OnFileDescriptorEventDispatched), FileDescriptorMonitor::FD_READABLE);

      DALI_LOG_RELEASE_INFO("WindowRenderSurface::ProcessFrameCallback: Add handler [%d]\n", iter->fileDescriptor);
    }
  }
}

void WindowRenderSurface::OnFileDescriptorEventDispatched(FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor)
{
  DALI_LOG_RELEASE_INFO("WindowRenderSurface::OnFileDescriptorEventDispatched: Frame rendered [%d]\n", fileDescriptor);

  std::unique_ptr<FrameCallbackInfo> callbackInfo;
  {
    Dali::Mutex::ScopedLock lock(mMutex);
    auto                    frameCallbackInfo = std::find_if(mFrameCallbackInfoContainer.begin(), mFrameCallbackInfoContainer.end(), [fileDescriptor](std::unique_ptr<FrameCallbackInfo>& callbackInfo) {
      return callbackInfo->fileDescriptor == fileDescriptor;
    });
    if(frameCallbackInfo != mFrameCallbackInfoContainer.end())
    {
      callbackInfo = std::move(*frameCallbackInfo);

      mFrameCallbackInfoContainer.erase(frameCallbackInfo);
    }
  }

  // Call the connected callback
  if(callbackInfo && (eventBitMask & FileDescriptorMonitor::FD_READABLE))
  {
    for(auto&& iter : (callbackInfo)->callbacks)
    {
      CallbackBase::Execute(*(iter.first), iter.second);
    }
  }
}

void WindowRenderSurface::SetBufferDamagedRects(const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  if(mEGL)
  {
    // If scene is not exist, just use stored mPositionSize.
    Rect<int> surfaceRect(0, 0, mPositionSize.width, mPositionSize.height);
    int32_t   orientation = 0;

    Dali::Integration::Scene scene = mScene.GetHandle();
    if(scene)
    {
      surfaceRect        = scene.GetCurrentSurfaceRect();
      int32_t totalAngle = scene.GetCurrentSurfaceOrientation() + scene.GetCurrentScreenOrientation();
      if(totalAngle >= 360)
      {
        totalAngle -= 360;
      }
      orientation = std::min(totalAngle / 90, 3);
    }

    if(!mEGL->IsPartialUpdateRequired() || mFullSwapNextFrame)
    {
      InsertRects(mBufferDamagedRects, surfaceRect);
      clippingRect = surfaceRect;
      return;
    }

    if(damagedRects.empty())
    {
      // Empty damaged rect. We don't need rendering
      clippingRect = Rect<int>();
      // Clean up current damanged rects.
      mDamagedRects.clear();
      return;
    }

    mGraphics->ActivateSurfaceContext(this);

    EGLint bufferAge = mEGL->GetBufferAge(mEGLSurface);

    // Buffer age 0 means the back buffer in invalid and requires full swap
    if(bufferAge == 0)
    {
      InsertRects(mBufferDamagedRects, surfaceRect);
      clippingRect = surfaceRect;
      return;
    }

    mDamagedRects.assign(damagedRects.begin(), damagedRects.end());

    // Merge intersecting rects, form an array of non intersecting rects to help driver a bit
    // Could be optional and can be removed, needs to be checked with and without on platform
    // And then, Make one clipping rect, and rotate rects by orientation.
    MergeIntersectingRectsAndRotate(clippingRect, mDamagedRects, orientation, surfaceRect);

    // We push current frame damaged rects here, zero index for current frame
    InsertRects(mBufferDamagedRects, clippingRect);

    // Merge damaged rects into clipping rect
    if(bufferAge <= static_cast<EGLint>(mBufferDamagedRects.size()))
    {
      // clippingRect is already the current frame's damaged rect. Merge from the second
      for(int i = 1; i < bufferAge; i++)
      {
        clippingRect.Merge(mBufferDamagedRects[i]);
      }
    }
    else
    {
      // The buffer age is too old. Need full update.
      clippingRect = surfaceRect;
      return;
    }

    if(!clippingRect.Intersect(surfaceRect) || clippingRect.Area() > surfaceRect.Area() * FULL_UPDATE_RATIO)
    {
      // clipping area too big or doesn't intersect surface rect
      clippingRect = surfaceRect;
      return;
    }

    if(!clippingRect.IsEmpty())
    {
      std::vector<Rect<int>> damagedRegion;
      if(scene)
      {
        damagedRegion.push_back(RecalculateRect[orientation](clippingRect, surfaceRect));
      }
      else
      {
        damagedRegion.push_back(clippingRect);
      }

      mEGL->SetDamageRegion(mEGLSurface, damagedRegion);
    }
  }
}

void WindowRenderSurface::SwapBuffers(const std::vector<Rect<int>>& damagedRects)
{
  if(mEGL)
  {
    if(!mEGL->IsPartialUpdateRequired() || mFullSwapNextFrame)
    {
      mFullSwapNextFrame = false;
      mEGL->SwapBuffers(mEGLSurface);
      return;
    }

    mFullSwapNextFrame = false;

    Rect<int32_t>            surfaceRect;
    Dali::Integration::Scene scene = mScene.GetHandle();
    if(scene)
    {
      surfaceRect = scene.GetCurrentSurfaceRect();
    }

    if(!damagedRects.size() || (damagedRects[0].Area() > surfaceRect.Area() * FULL_UPDATE_RATIO))
    {
      // In normal cases, WindowRenderSurface::SwapBuffers() will not be called if mergedRects.size() is 0.
      // For exceptional cases, swap full area.
      mEGL->SwapBuffers(mEGLSurface);
    }
    else
    {
      mEGL->SwapBuffers(mEGLSurface, damagedRects);
    }
  }
}

void WindowRenderSurface::SetFrontBufferRendering(bool enable)
{
  if(mIsFrontBufferRendering != enable)
  {
    mIsFrontBufferRendering        = enable;
    mIsFrontBufferRenderingChanged = !mIsFrontBufferRenderingChanged;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
