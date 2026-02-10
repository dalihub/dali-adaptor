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
#include <dali/internal/offscreen/egl/offscreen-render-surface-egl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/camera-actor.h>
#include <unistd.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gOffscreenSurfaceLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_OFFSCREEN_RENDER_SURFACE");
#endif
} // unnamed namespace

OffscreenRenderSurfaceEgl::OffscreenRenderSurfaceEgl()
{
}

OffscreenRenderSurfaceEgl::~OffscreenRenderSurfaceEgl()
{
  if(mFenceFd != -1)
  {
    close(mFenceFd);
  }
}

void OffscreenRenderSurfaceEgl::OnAdaptorSet(Dali::RenderTask renderTask)
{
  mRenderTask = renderTask;
}

void OffscreenRenderSurfaceEgl::SetNativeImage(NativeImageSourcePtr nativeImage)
{
  mNativeImage = nativeImage;

  mNativeTexture = Dali::Texture::New(*mNativeImage);

  // If the surface size is different, make a new framebuffer
  if(mWidth != mNativeTexture.GetWidth() || mHeight != mNativeTexture.GetHeight())
  {
    mWidth  = mNativeTexture.GetWidth();
    mHeight = mNativeTexture.GetHeight();

    mFrameBuffer = Dali::FrameBuffer::New(mWidth, mHeight, Dali::FrameBuffer::Attachment::DEPTH_STENCIL);

    mRenderTask.SetFrameBuffer(mFrameBuffer);
    mRenderTask.GetCameraActor().SetInvertYAxis(true);
  }

  mFrameBuffer.AttachColorTexture(mNativeTexture);

  DALI_LOG_INFO(gOffscreenSurfaceLogFilter, Debug::Verbose, "Native image [%d x %d]\n", mNativeImage->GetWidth(), mNativeImage->GetHeight());
}

void OffscreenRenderSurfaceEgl::AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback)
{
  if(!mPostRenderTrigger)
  {
    mPostRenderTrigger = TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &OffscreenRenderSurfaceEgl::ProcessPostRender), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);

    DALI_LOG_DEBUG_INFO("mPostRenderTrigger Trigger Id(%u)\n", mPostRenderTrigger->GetId());
  }

  mPostRenderSyncCallback = std::move(callback);
}

void OffscreenRenderSurfaceEgl::AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback)
{
  if(!mPostRenderTrigger)
  {
    mPostRenderTrigger = TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &OffscreenRenderSurfaceEgl::ProcessPostRender), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);

    DALI_LOG_DEBUG_INFO("mPostRenderTrigger Trigger Id(%u)\n", mPostRenderTrigger->GetId());
  }

  mPostRenderAsyncCallback = std::move(callback);
}

PositionSize OffscreenRenderSurfaceEgl::GetPositionSize() const
{
  return PositionSize(0, 0, static_cast<int>(mWidth), static_cast<int>(mHeight));
}

void OffscreenRenderSurfaceEgl::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  dpiHorizontal = 96;
  dpiVertical   = 96;
}

int OffscreenRenderSurfaceEgl::GetSurfaceOrientation() const
{
  return 0;
}

int OffscreenRenderSurfaceEgl::GetScreenOrientation() const
{
  return 0;
}

void OffscreenRenderSurfaceEgl::InitializeGraphics()
{
  if(!mEglGraphics)
  {
    mEglGraphics = static_cast<EglGraphics*>(&mAdaptor->GetGraphicsInterface());
    DALI_ASSERT_ALWAYS(mEglGraphics && "Graphics interface is not created");

    CreateSurface();
  }
}

void OffscreenRenderSurfaceEgl::CreateSurface()
{
  EglImplementation& eglImpl = mEglGraphics->GetEglImplementation();

  eglImpl.ChooseConfig(true, Dali::COLOR_DEPTH_32);

  // Create surfaceless context
  eglImpl.CreateOffscreenContext(mEGLContext);
}

void OffscreenRenderSurfaceEgl::DestroySurface()
{
  if(mEGLContext)
  {
    mEglGraphics->GetEglImplementation().DestroyContext(mEGLContext);
    mEGLContext = nullptr;
  }
}

bool OffscreenRenderSurfaceEgl::ReplaceGraphicsSurface()
{
  return false;
}

void OffscreenRenderSurfaceEgl::MoveResize(Dali::PositionSize positionSize)
{
}

void OffscreenRenderSurfaceEgl::Resize(Uint16Pair size)
{
}

void OffscreenRenderSurfaceEgl::StartRender()
{
}

bool OffscreenRenderSurfaceEgl::PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  return true;
}

void OffscreenRenderSurfaceEgl::PostRender()
{
  if(mPostRenderTrigger)
  {
    if(mPostRenderSyncCallback)
    {
      mEglGraphics->GetGlAbstraction().Finish();

      if(mThreadSynchronization && mPostRenderSyncCallback)
      {
        // Enable PostRender flag
        mThreadSynchronization->PostRenderStarted();
      }

      mPostRenderTrigger->Trigger();

      if(mThreadSynchronization && mPostRenderSyncCallback)
      {
        // Wait until the event-thread completion
        mThreadSynchronization->PostRenderWaitForCompletion();
      }

      return;
    }
    else if(mPostRenderAsyncCallback)
    {
      Internal::Adaptor::EglSyncObject* syncObject = static_cast<Internal::Adaptor::EglSyncObject*>(mEglGraphics->GetSyncImplementation().CreateSyncObject(Integration::GraphicsSyncAbstraction::SyncObject::SyncType::NATIVE_FENCE_SYNC));
      if(!syncObject)
      {
        DALI_LOG_ERROR("CreateSyncObject failed [%d]\n");
        mEglGraphics->GetGlAbstraction().Flush();
        return;
      }

      // Close the old fd if it exists
      if(mFenceFd != -1)
      {
        close(mFenceFd);
      }

      mFenceFd = syncObject->DuplicateNativeFenceFD();

      DALI_LOG_INFO(gOffscreenSurfaceLogFilter, Debug::Verbose, "Fence FD [%d]\n", mFenceFd);

      // We can remove EglSyncObject after the fd is duplicated.
      mEglGraphics->GetSyncImplementation().DestroySyncObject(syncObject);
    }

    mEglGraphics->GetGlAbstraction().Flush();

    mPostRenderTrigger->Trigger();
  }
  else
  {
    mEglGraphics->GetGlAbstraction().Flush();
  }
}

void OffscreenRenderSurfaceEgl::StopRender()
{
}

void OffscreenRenderSurfaceEgl::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
  mThreadSynchronization = &threadSynchronization;
}

void OffscreenRenderSurfaceEgl::ReleaseLock()
{
}

Dali::Integration::RenderSurfaceInterface::Type OffscreenRenderSurfaceEgl::GetSurfaceType()
{
  return RenderSurfaceInterface::WINDOW_RENDER_SURFACE;
}

void OffscreenRenderSurfaceEgl::MakeContextCurrent()
{
  mEglGraphics->GetEglImplementation().MakeContextCurrent(nullptr, mEGLContext);
}

Integration::DepthBufferAvailable OffscreenRenderSurfaceEgl::GetDepthBufferRequired()
{
  return Integration::DepthBufferAvailable::TRUE;
}

Integration::StencilBufferAvailable OffscreenRenderSurfaceEgl::GetStencilBufferRequired()
{
  return Integration::StencilBufferAvailable::TRUE;
}

void OffscreenRenderSurfaceEgl::ProcessPostRender()
{
  if(mPostRenderSyncCallback)
  {
    CallbackBase::Execute(*mPostRenderSyncCallback);

    if(mThreadSynchronization)
    {
      mThreadSynchronization->PostRenderComplete();
    }
  }

  if(mPostRenderAsyncCallback)
  {
    DALI_LOG_INFO(gOffscreenSurfaceLogFilter, Debug::Verbose, "Fence FD [%d]\n", mFenceFd);

    CallbackBase::Execute(*mPostRenderAsyncCallback, mFenceFd);

    // Reset fd. FD will be closed during the callback.
    mFenceFd = -1;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
