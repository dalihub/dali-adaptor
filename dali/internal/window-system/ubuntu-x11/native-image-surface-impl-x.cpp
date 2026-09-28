/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/ubuntu-x11/native-image-surface-impl-x.h>

// EXTERNAL INCLUDES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <X11/Xlib.h>
#include <cstring>

#include <dali/integration-api/debug.h>

using namespace Dali::Internal::Adaptor;

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr uint32_t BYTES_PER_PIXEL = 4u;

/**
 * The surface owns its own EGL objects rather than borrowing DALi's.
 *
 * EglImplementation only knows how to make window and pixmap surfaces, and the
 * client needs a pbuffer - a default framebuffer that is not tied to any
 * window-system drawable. Bending the shared implementation to cover that would
 * put the window path at risk for no gain, so the handful of EGL calls this
 * needs live here.
 *
 * The context is deliberately unshared with DALi's, so the client's GL state
 * cannot disturb the UI.
 */
EGLDisplay GetOwnEglDisplay(Display*& xDisplay)
{
  // Its own X connection, which gives it its own EGLDisplay: that is what makes it
  // safe to terminate in TerminateGraphics() without disturbing the UI's.
  xDisplay = XOpenDisplay(nullptr);
  if(!xDisplay)
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: XOpenDisplay failed\n");
    return EGL_NO_DISPLAY;
  }

  EGLDisplay display = eglGetDisplay(static_cast<EGLNativeDisplayType>(xDisplay));
  if(display == EGL_NO_DISPLAY)
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: eglGetDisplay failed\n");
    return EGL_NO_DISPLAY;
  }

  EGLint major = 0;
  EGLint minor = 0;
  if(!eglInitialize(display, &major, &minor))
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: eglInitialize failed. eglGetError[0x%x]\n", eglGetError());
    return EGL_NO_DISPLAY;
  }

  return display;
}

GLenum ToReadFormat(NativeImageQueueProducerX::ChannelOrder channelOrder)
{
  // Reading in the order the buffer was allocated for keeps the copy below a
  // per-row memcpy; swizzling per pixel would cost far more than the readback.
  return (channelOrder == NativeImageQueueProducerX::ChannelOrder::BGRA) ? GL_BGRA_EXT : GL_RGBA;
}
} // namespace

NativeImageSurfaceX::NativeImageSurfaceX(Dali::NativeImageQueuePtr queue)
: mQueue(queue)
{
  if(!queue)
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: NativeImageQueue is null\n");
    return;
  }

  // Yields the interface this class has to drive by hand, or nothing at all if
  // the queue could not be backed - no render node, or no dma_buf import.
  Any producer = queue->GetNativeImageQueue();
  if(producer.Empty())
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: the queue has no producer interface, offscreen rendering is unavailable\n");
    return;
  }

  mProducer = AnyCast<NativeImageQueueProducerX*>(producer);
}

NativeImageSurfaceX::~NativeImageSurfaceX()
{
  // TerminateGraphics() is the documented teardown, but do not leak if the
  // caller never got that far.
  if(mGraphicsInitialized)
  {
    TerminateGraphics();
  }
}

Any NativeImageSurfaceX::GetNativeRenderable()
{
  // There is no window-system drawable behind a pbuffer.
  return Any();
}

bool NativeImageSurfaceX::SetGraphicsConfig(bool depth, bool stencil, int msaa, int version)
{
  mDepth       = depth;
  mStencil     = stencil;
  mMSAA        = msaa;
  mGlesVersion = version;

  // Reports whether offscreen rendering is possible at all, which is what the
  // caller uses this return value for. The config itself is only chosen in
  // InitializeGraphics().
  return mProducer != nullptr;
}

bool NativeImageSurfaceX::ChooseConfig()
{
  EGLint renderableType = (mGlesVersion >= 30) ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT;

  std::vector<EGLint> attribs{
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE, renderableType,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, mDepth ? 24 : 0,
    EGL_STENCIL_SIZE, mStencil ? 8 : 0};

  // A single sample is not anti-aliasing, so only ask for a multisample config
  // from two up, matching what the window path does.
  if(mMSAA > 1)
  {
    attribs.insert(attribs.end(), {EGL_SAMPLES, mMSAA, EGL_SAMPLE_BUFFERS, 1});
  }

  attribs.push_back(EGL_NONE);

  EGLConfig config    = nullptr;
  EGLint    numConfig = 0;
  if(!eglChooseConfig(static_cast<EGLDisplay>(mEglDisplay), attribs.data(), &config, 1, &numConfig) || numConfig < 1)
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: no pbuffer config for gles[%d] depth[%d] stencil[%d] msaa[%d]. eglGetError[0x%x]\n",
                   mGlesVersion,
                   mDepth ? 24 : 0,
                   mStencil ? 8 : 0,
                   mMSAA,
                   eglGetError());
    return false;
  }

  mEglConfig = config;
  return true;
}

bool NativeImageSurfaceX::CreatePbuffer(uint32_t width, uint32_t height)
{
  DestroyPbuffer();

  if(width == 0u || height == 0u)
  {
    return false;
  }

  const EGLint attribs[] = {EGL_WIDTH, static_cast<EGLint>(width), EGL_HEIGHT, static_cast<EGLint>(height), EGL_NONE};

  EGLSurface surface = eglCreatePbufferSurface(static_cast<EGLDisplay>(mEglDisplay),
                                               static_cast<EGLConfig>(mEglConfig),
                                               attribs);
  if(surface == EGL_NO_SURFACE)
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: eglCreatePbufferSurface(%ux%u) failed. eglGetError[0x%x]\n", width, height, eglGetError());
    return false;
  }

  mEglSurface    = surface;
  mSurfaceWidth  = width;
  mSurfaceHeight = height;
  mReadbackBuffer.resize(static_cast<size_t>(width) * height * BYTES_PER_PIXEL);
  return true;
}

void NativeImageSurfaceX::DestroyPbuffer()
{
  if(mEglSurface)
  {
    eglMakeCurrent(static_cast<EGLDisplay>(mEglDisplay), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(static_cast<EGLDisplay>(mEglDisplay), static_cast<EGLSurface>(mEglSurface));
    mEglSurface = nullptr;
  }
  mSurfaceWidth  = 0u;
  mSurfaceHeight = 0u;
}

void NativeImageSurfaceX::InitializeGraphics()
{
  if(!mProducer || mGraphicsInitialized)
  {
    return;
  }

  Display* xDisplay = nullptr;
  mEglDisplay       = GetOwnEglDisplay(xDisplay);
  mXDisplay         = xDisplay;
  if(!mEglDisplay)
  {
    if(mXDisplay)
    {
      XCloseDisplay(static_cast<Display*>(mXDisplay));
      mXDisplay = nullptr;
    }
    return;
  }

  eglBindAPI(EGL_OPENGL_ES_API);

  if(!ChooseConfig())
  {
    return;
  }

  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, (mGlesVersion >= 30) ? 3 : 2, EGL_NONE};

  EGLContext context = eglCreateContext(static_cast<EGLDisplay>(mEglDisplay),
                                        static_cast<EGLConfig>(mEglConfig),
                                        EGL_NO_CONTEXT,
                                        contextAttribs);
  if(context == EGL_NO_CONTEXT)
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: eglCreateContext failed. eglGetError[0x%x]\n", eglGetError());
    return;
  }

  mEglContext          = context;
  mGraphicsInitialized = true;
}

void NativeImageSurfaceX::TerminateGraphics()
{
  if(!mGraphicsInitialized)
  {
    return;
  }

  CancelPendingBuffer();
  DestroyPbuffer();

  if(mEglContext)
  {
    eglMakeCurrent(static_cast<EGLDisplay>(mEglDisplay), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(static_cast<EGLDisplay>(mEglDisplay), static_cast<EGLContext>(mEglContext));
    mEglContext = nullptr;
  }

  mEglConfig = nullptr;

  if(mEglDisplay)
  {
    // Paired with the eglInitialize() in InitializeGraphics(). This display belongs to
    // this surface's own X connection, so terminating it cannot disturb DALi's.
    eglTerminate(static_cast<EGLDisplay>(mEglDisplay));
    mEglDisplay = nullptr;
  }

  if(mXDisplay)
  {
    XCloseDisplay(static_cast<Display*>(mXDisplay));
    mXDisplay = nullptr;
  }

  mGraphicsInitialized = false;
  mReadbackBuffer.clear();
  mReadbackBuffer.shrink_to_fit();
}

void NativeImageSurfaceX::CancelPendingBuffer()
{
  if(mCurrentBuffer.IsValid() && mProducer)
  {
    mProducer->CancelDequeued(mCurrentBuffer);
  }
  mCurrentBuffer = NativeImageQueueProducerX::Buffer{};
}

void NativeImageSurfaceX::PreRender()
{
  if(!mGraphicsInitialized)
  {
    return;
  }

  // A frame the client declined to produce leaves its buffer checked out, so
  // hand it back before taking another.
  CancelPendingBuffer();

  mCurrentBuffer = mProducer->Dequeue();
  if(!mCurrentBuffer.IsValid())
  {
    return;
  }

  // The queue may be resized from another thread, so the buffer is where a new
  // size first shows up here.
  if(mCurrentBuffer.width != mSurfaceWidth || mCurrentBuffer.height != mSurfaceHeight)
  {
    if(!CreatePbuffer(mCurrentBuffer.width, mCurrentBuffer.height))
    {
      CancelPendingBuffer();
      return;
    }
  }

  if(!eglMakeCurrent(static_cast<EGLDisplay>(mEglDisplay),
                     static_cast<EGLSurface>(mEglSurface),
                     static_cast<EGLSurface>(mEglSurface),
                     static_cast<EGLContext>(mEglContext)))
  {
    DALI_LOG_ERROR("NativeImageSurfaceX: eglMakeCurrent failed. eglGetError[0x%x]\n", eglGetError());
    CancelPendingBuffer();
  }
}

bool NativeImageSurfaceX::CanRender()
{
  // PreRender() has already tried to dequeue, so this reports whether it got a
  // buffer.
  return mCurrentBuffer.IsValid();
}

void NativeImageSurfaceX::PostRender()
{
  if(!mCurrentBuffer.IsValid())
  {
    return;
  }

  const uint32_t width    = mCurrentBuffer.width;
  const uint32_t height   = mCurrentBuffer.height;
  const size_t   rowBytes = static_cast<size_t>(width) * BYTES_PER_PIXEL;
  const size_t   needed   = rowBytes * height;

  if(mReadbackBuffer.size() < needed)
  {
    mReadbackBuffer.resize(needed);
  }

  // This is the enqueue the graphics driver performs on target. Everything the
  // client drew has to be complete and readable before the buffer is published,
  // and glReadPixels is itself the synchronisation point.
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), ToReadFormat(mCurrentBuffer.channelOrder), GL_UNSIGNED_BYTE, mReadbackBuffer.data());

  // glReadPixels returns the bottom row first while the consumer samples the
  // buffer top-down, so the rows are reversed on the way in. Doing it as a copy
  // rather than in place also absorbs the buffer's row padding, which can be
  // wider than a tightly packed row and which glReadPixels cannot target.
  const uint8_t* source = mReadbackBuffer.data();
  uint8_t*       target = mCurrentBuffer.data;
  for(uint32_t y = 0u; y < height; ++y)
  {
    std::memcpy(target + static_cast<size_t>(y) * mCurrentBuffer.stride,
                source + static_cast<size_t>(height - 1u - y) * rowBytes,
                rowBytes);
  }

  mProducer->Enqueue(mCurrentBuffer);
  mCurrentBuffer = NativeImageQueueProducerX::Buffer{};
}

} // namespace Adaptor
} // namespace Internal
} //namespace DALI_NAMESPACE
