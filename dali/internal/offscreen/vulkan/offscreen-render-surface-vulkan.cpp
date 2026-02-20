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
#include <dali/internal/offscreen/vulkan/offscreen-render-surface-vulkan.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

OffscreenRenderSurfaceVulkan::OffscreenRenderSurfaceVulkan()
{
}

OffscreenRenderSurfaceVulkan::~OffscreenRenderSurfaceVulkan()
{
}

void OffscreenRenderSurfaceVulkan::OnAdaptorSet(Dali::RenderTask renderTask)
{
}

void OffscreenRenderSurfaceVulkan::SetNativeImage(Dali::NativeImageInterfacePtr nativeImage)
{
  mNativeImage = nativeImage;
}

Dali::NativeImageInterfacePtr OffscreenRenderSurfaceVulkan::GetNativeImage() const
{
  return mNativeImage;
}

void OffscreenRenderSurfaceVulkan::AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback)
{
}

void OffscreenRenderSurfaceVulkan::AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback)
{
}

PositionSize OffscreenRenderSurfaceVulkan::GetPositionSize() const
{
  return PositionSize(0, 0, static_cast<int>(mNativeImage->GetWidth()), static_cast<int>(mNativeImage->GetHeight()));
}

void OffscreenRenderSurfaceVulkan::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  dpiHorizontal = 96;
  dpiVertical   = 96;
}

int OffscreenRenderSurfaceVulkan::GetSurfaceOrientation() const
{
  return 0;
}

int OffscreenRenderSurfaceVulkan::GetScreenOrientation() const
{
  return 0;
}

void OffscreenRenderSurfaceVulkan::InitializeGraphics()
{
}

void OffscreenRenderSurfaceVulkan::CreateSurface()
{
}

void OffscreenRenderSurfaceVulkan::DestroySurface()
{
}

bool OffscreenRenderSurfaceVulkan::ReplaceGraphicsSurface()
{
  return false;
}

void OffscreenRenderSurfaceVulkan::MoveResize(Dali::PositionSize positionSize)
{
}

void OffscreenRenderSurfaceVulkan::Resize(Uint16Pair size)
{
}

void OffscreenRenderSurfaceVulkan::StartRender()
{
}

bool OffscreenRenderSurfaceVulkan::PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  return true;
}

void OffscreenRenderSurfaceVulkan::PostRender()
{
}

void OffscreenRenderSurfaceVulkan::StopRender()
{
}

void OffscreenRenderSurfaceVulkan::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
}

void OffscreenRenderSurfaceVulkan::ReleaseLock()
{
}

Dali::Integration::RenderSurfaceInterface::Type OffscreenRenderSurfaceVulkan::GetSurfaceType()
{
  return RenderSurfaceInterface::WINDOW_RENDER_SURFACE;
}

void OffscreenRenderSurfaceVulkan::MakeContextCurrent()
{
}

Integration::DepthBufferAvailable OffscreenRenderSurfaceVulkan::GetDepthBufferRequired()
{
  return Integration::DepthBufferAvailable::TRUE;
}

Integration::StencilBufferAvailable OffscreenRenderSurfaceVulkan::GetStencilBufferRequired()
{
  return Integration::StencilBufferAvailable::TRUE;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
