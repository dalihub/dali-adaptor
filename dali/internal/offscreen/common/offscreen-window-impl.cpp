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
#include <dali/internal/offscreen/common/offscreen-window-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/public-api/render-tasks/render-task.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/offscreen/common/offscreen-render-surface.h>
#include <dali/internal/window-system/common/render-surface-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

OffscreenWindow* OffscreenWindow::New()
{
  OffscreenWindow* window = new OffscreenWindow();
  window->Initialize();
  return window;
}

void OffscreenWindow::Initialize()
{
  // Create a window render surface
  auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
  DALI_ASSERT_DEBUG(renderSurfaceFactory && "Cannot create render surface factory\n");

  mSurface          = renderSurfaceFactory->CreateOffscreenRenderSurface();
  mOffscreenSurface = static_cast<OffscreenRenderSurface*>(mSurface.get());
}

OffscreenWindow::~OffscreenWindow()
{
  if(mAdaptor)
  {
    mAdaptor->RemoveWindow(this);
  }
}

void OffscreenWindow::SetNativeImage(NativeImageSourcePtr nativeImage)
{
  if(nativeImage)
  {
    uint32_t width  = nativeImage->GetWidth();
    uint32_t height = nativeImage->GetHeight();

    mOffscreenSurface->SetNativeImage(nativeImage);

    if(width != mWidth || height != mHeight)
    {
      mWidth  = width;
      mHeight = height;

      // Set scene size
      SurfaceResized(static_cast<float>(mWidth), static_cast<float>(mHeight));

      Uint16Pair newSize(mWidth, mHeight);

      mAdaptor->SurfaceResizePrepare(mSurface.get(), newSize);
      mAdaptor->SurfaceResizeComplete(mSurface.get(), newSize);
    }
  }
}

OffscreenWindow::WindowSize OffscreenWindow::GetSize() const
{
  return OffscreenWindow::WindowSize(mWidth, mHeight);
}

void OffscreenWindow::AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback)
{
  mPostRenderSyncCallback = std::move(callback);
  mOffscreenSurface->AddPostRenderSyncCallback(std::unique_ptr<CallbackBase>(MakeCallback(this, &OffscreenWindow::PostRenderSyncCallback)));
}

void OffscreenWindow::AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback)
{
  mPostRenderAsyncCallback = std::move(callback);
  mOffscreenSurface->AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase>(MakeCallback(this, &OffscreenWindow::PostRenderAsyncCallback)));
}

void OffscreenWindow::OnAdaptorSet(Dali::Adaptor& adaptor)
{
  mOffscreenSurface->OnAdaptorSet(mScene.GetRenderTaskList().GetTask(0));
}

void OffscreenWindow::PostRenderSyncCallback()
{
  if(mPostRenderSyncCallback)
  {
    CallbackBase::Execute(*mPostRenderSyncCallback, Dali::OffscreenWindow(this));
  }
}

void OffscreenWindow::PostRenderAsyncCallback(int32_t fenceFd)
{
  if(mPostRenderAsyncCallback)
  {
    CallbackBase::Execute(*mPostRenderAsyncCallback, Dali::OffscreenWindow(this), fenceFd);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
