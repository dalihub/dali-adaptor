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
#include <dali/internal/window-system/x11/pixmap-render-surface-x.h>

// EXTERNAL INCLUDES
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h> // for damage notify
#include <X11/extensions/Xfixes.h>  // for damage notify
#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/x11/window-system-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gPixmapRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_PIXMAP_RENDER_SURFACE_X");
#endif

namespace
{
static const int INITIAL_PRODUCE_BUFFER_INDEX = 0;
static const int INITIAL_CONSUME_BUFFER_INDEX = 1;
} // namespace

PixmapRenderSurfaceX::PixmapRenderSurfaceX(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mGraphics(nullptr),
  mDisplayConnection(nullptr),
  mPosition(positionSize),
  mRenderNotification(NULL),
  mColorDepth(isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24),
  mOwnSurface(false),
  mProduceBufferIndex(INITIAL_PRODUCE_BUFFER_INDEX),
  mConsumeBufferIndex(INITIAL_CONSUME_BUFFER_INDEX),
  mX11Pixmaps(),
  mEglSurfaces(),
  mThreadSynchronization(nullptr),
  mPixmapCondition()
{
  for(int i = 0; i != BUFFER_COUNT; ++i)
  {
    mX11Pixmaps[i]  = 0;
    mEglSurfaces[i] = 0;
  }

  Initialize(surface);
}

PixmapRenderSurfaceX::~PixmapRenderSurfaceX()
{
  DestroySurface();

  // release the surface if we own one
  if(mOwnSurface)
  {
    for(int i = 0; i < BUFFER_COUNT; ++i)
    {
      ::Pixmap pixmap = mX11Pixmaps[i];

      // if we did create the pixmap, delete the pixmap
      DALI_LOG_INFO(gPixmapRenderSurfaceLogFilter, Debug::General, "Own pixmap (%x) freed\n", pixmap);
      XFreePixmap(WindowSystem::GetImplementation().GetXDisplay(), pixmap);
    }
  }
}

void PixmapRenderSurfaceX::Initialize(Any surface)
{
  // see if there is a surface in Any surface
  unsigned int surfaceId = GetSurfaceId(surface);

  // if the surface is empty, create a new one.
  if(surfaceId == 0)
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateRenderable();
  }
  else
  {
    // XLib should already be initialized so no point in calling XInitThreads
    UseExistingRenderable(surfaceId);
  }
}

Any PixmapRenderSurfaceX::GetSurface()
{
  ::Pixmap pixmap = 0;
  {
    ConditionalWait::ScopedLock lock(mPixmapCondition);
    pixmap = mX11Pixmaps[mProduceBufferIndex];
  }

  return Any(pixmap);
}

void PixmapRenderSurfaceX::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

PositionSize PixmapRenderSurfaceX::GetPositionSize() const
{
  return mPosition;
}

void PixmapRenderSurfaceX::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  WindowSystem::GetImplementation().GetDPI(dpiHorizontal, dpiVertical);
}

int PixmapRenderSurfaceX::GetOrientation() const
{
  return 0;
}

void PixmapRenderSurfaceX::InitializeGraphics()
{
  mGraphics          = &mAdaptor->GetGraphicsInterface();
  mDisplayConnection = &mAdaptor->GetDisplayConnectionInterface();

  auto                                  eglGraphics = static_cast<EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();
  eglImpl.ChooseConfig(false, mColorDepth);
}

void PixmapRenderSurfaceX::CreateSurface()
{
  DALI_LOG_TRACE_METHOD(gPixmapRenderSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  for(int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the EGL surface
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    ::Pixmap pixmap = static_cast<::Pixmap>(mX11Pixmaps[i]);
    mEglSurfaces[i] = eglImpl.CreateSurfacePixmap(EGLNativePixmapType(pixmap), mColorDepth); // reinterpret_cast does not compile
  }
}

void PixmapRenderSurfaceX::DestroySurface()
{
  DALI_LOG_TRACE_METHOD(gPixmapRenderSurfaceLogFilter);

  auto eglGraphics = static_cast<EglGraphics*>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for(int i = 0; i < BUFFER_COUNT; ++i)
  {
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    ::Pixmap pixmap = static_cast<::Pixmap>(mX11Pixmaps[i]);
    eglImpl.MakeCurrent(EGLNativePixmapType(pixmap), mEglSurfaces[i]);
    eglImpl.DestroySurface(mEglSurfaces[i]);
  }
}

bool PixmapRenderSurfaceX::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD(gPixmapRenderSurfaceLogFilter);

  bool contextLost = false;

  auto eglGraphics = static_cast<EglGraphics*>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for(int i = 0; i < BUFFER_COUNT; ++i)
  {
    // a new surface for the new pixmap
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    ::Pixmap pixmap = static_cast<::Pixmap>(mX11Pixmaps[i]);
    contextLost     = eglImpl.ReplaceSurfacePixmap(EGLNativePixmapType(pixmap), mEglSurfaces[i]); // reinterpret_cast does not compile
  }

  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  ::Pixmap pixmap = static_cast<::Pixmap>(mX11Pixmaps[mProduceBufferIndex]);
  eglImpl.MakeCurrent(EGLNativePixmapType(pixmap), mEglSurfaces[mProduceBufferIndex]);

  return contextLost;
}

void PixmapRenderSurfaceX::StartRender()
{
}

bool PixmapRenderSurfaceX::PreRender(bool, const std::vector<Rect<int>>&, Rect<int>&)
{
  // Nothing to do for pixmaps
  return true;
}

void PixmapRenderSurfaceX::PostRender()
{
  auto eglGraphics = static_cast<EglGraphics*>(mGraphics);

  // flush gl instruction queue
  Integration::GlAbstraction& glAbstraction = eglGraphics->GetGlAbstraction();
  glAbstraction.Flush();

  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderStarted();
  }

  {
    ConditionalWait::ScopedLock lock(mPixmapCondition);
    mConsumeBufferIndex = __sync_fetch_and_xor(&mProduceBufferIndex, 1); // Swap buffer indexes.

    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    ::Pixmap pixmap = static_cast<::Pixmap>(mX11Pixmaps[mProduceBufferIndex]);
    eglImpl.MakeCurrent(EGLNativePixmapType(pixmap), mEglSurfaces[mProduceBufferIndex]);
  }

  // create damage for client applications which wish to know the update timing
  if(mRenderNotification)
  {
    // use notification trigger
    // Tell the event-thread to render the pixmap
    mRenderNotification->Trigger();
  }
  else
  {
    // as a fallback, send damage event.
    ::Drawable drawable = ::Drawable(mX11Pixmaps[mProduceBufferIndex]);

    if(drawable)
    {
      XRectangle    rect;
      XserverRegion region;

      rect.x      = 0;
      rect.y      = 0;
      rect.width  = mPosition.width;
      rect.height = mPosition.height;

      ::Display* display = AnyCast<::Display*>(mDisplayConnection->GetDisplay());

      // make a fixes region as updated area
      region = XFixesCreateRegion(display, &rect, 1);
      // add damage event to updated drawable
      ::Drawable xdrawable(drawable); // ecore type is unsigned int whereas in 64bit linux Drawable is long unsigned int
      XDamageAdd(display, xdrawable, region);
      XFixesDestroyRegion(display, region);

      XFlush(display);
    }
  }

  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderWaitForCompletion();
  }
}

void PixmapRenderSurfaceX::StopRender()
{
  ReleaseLock();
}

void PixmapRenderSurfaceX::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
  mThreadSynchronization = &threadSynchronization;
}

void PixmapRenderSurfaceX::ReleaseLock()
{
  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

Dali::RenderSurfaceInterface::Type PixmapRenderSurfaceX::GetSurfaceType()
{
  return Dali::RenderSurfaceInterface::PIXMAP_RENDER_SURFACE;
}

void PixmapRenderSurfaceX::MakeContextCurrent()
{
}

void PixmapRenderSurfaceX::CreateRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS(mPosition.width > 0 && mPosition.height > 0 && "Pixmap size is invalid");

  auto display = WindowSystem::GetImplementation().GetXDisplay();

  int colorDepth = (mColorDepth != 0 ? mColorDepth : DefaultDepth(display, DefaultScreen(display)));

  for(int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the pixmap
    mX11Pixmaps[i] = XCreatePixmap(display, DefaultRootWindow(display), mPosition.width, mPosition.height, colorDepth);

    // clear the pixmap
    GC        graphicsContext;
    XGCValues graphicsContextValues{0};
    graphicsContextValues.foreground = 0;

    graphicsContext = XCreateGC(display, mX11Pixmaps[i], GCForeground, &graphicsContextValues);

    DALI_ASSERT_ALWAYS(graphicsContext && "CreateRenderable(): failed to get graphics context");

    XFillRectangle(display, mX11Pixmaps[i], graphicsContext, 0, 0, mPosition.width, mPosition.height);
    DALI_ASSERT_ALWAYS(mX11Pixmaps[i] && "Failed to create X pixmap");

    // we SHOULD guarantee the xpixmap/x11 window was created in x server.
    XSync(display, False);
    XFreeGC(display, graphicsContext);
  }
}

void PixmapRenderSurfaceX::UseExistingRenderable(unsigned int surfaceId)
{
}

unsigned int PixmapRenderSurfaceX::GetSurfaceId(Any surface) const
{
  unsigned int surfaceId = 0;

  if(surface.Empty() == false)
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS(surface.GetType() == typeid(::Window) && "Surface type is invalid");
    surfaceId = AnyCast<::Window>(surface);
  }
  return surfaceId;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
