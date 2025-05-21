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
#include <dali/public-api/actors/layer.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface-factory.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

namespace Dali
{
namespace Internal
{
OffscreenWindow* OffscreenWindow::New(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent)
{
  OffscreenWindow* window = new OffscreenWindow(width, height, surface, isTranslucent);
  return window;
}

OffscreenWindow::OffscreenWindow(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent)
: mRenderNotification()
{
  // Create surface
  mSurface = std::unique_ptr<Integration::RenderSurfaceInterface>(CreateNativeSurface(SurfaceSize(width, height), surface, isTranslucent));
}

void OffscreenWindow::Initialize(bool isDefaultWindow)
{
  if(isDefaultWindow)
  {
    return;
  }

  Dali::Integration::SceneHolder sceneHolderHandler = Dali::Integration::SceneHolder(this);
  Dali::Adaptor::Get().AddWindow(sceneHolderHandler);
}

OffscreenWindow::~OffscreenWindow()
{
  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(surface)
  {
    // To prevent notification triggering in NativeRenderSurface::PostRender while deleting SceneHolder
    surface->SetRenderNotification(nullptr);
  }
}

uint32_t OffscreenWindow::GetLayerCount() const
{
  return mScene.GetLayerCount();
}

Dali::Layer OffscreenWindow::GetLayer(uint32_t depth) const
{
  return mScene.GetLayer(depth);
}

OffscreenWindow::WindowSize OffscreenWindow::GetSize() const
{
  Size size = mScene.GetSize();

  return OffscreenWindow::WindowSize(static_cast<uint16_t>(size.width), static_cast<uint16_t>(size.height));
}

Dali::Any OffscreenWindow::GetNativeHandle() const
{
  NativeRenderSurface* surface = GetNativeRenderSurface();
  DALI_ASSERT_ALWAYS(surface && "surface handle is empty");

  return surface->GetNativeRenderable();
}

void OffscreenWindow::SetPostRenderCallback(CallbackBase* callback)
{
  // Connect callback to be notified when the surface is rendered
  mPostRenderCallback = std::unique_ptr<CallbackBase>(callback);
  TriggerEventFactory triggerEventFactory;

  if(!mRenderNotification)
  {
    mRenderNotification = std::unique_ptr<TriggerEventInterface>(triggerEventFactory.CreateTriggerEvent(MakeCallback(this, &OffscreenWindow::OnPostRender), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
    DALI_LOG_DEBUG_INFO("mRenderNotification Trigger Id(%u)\n", mRenderNotification->GetId());
  }

  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(!surface)
  {
    DALI_LOG_ERROR("NativeRenderSurface is null.");
    return;
  }

  surface->SetRenderNotification(mRenderNotification.get());
}

void OffscreenWindow::SetFrameRenderedCallback(CallbackBase* callback)
{
  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(!surface)
  {
    DALI_LOG_ERROR("NativeRenderSurface is null.");
    return;
  }

  surface->SetFrameRenderedCallback(callback);
}

NativeRenderSurface* OffscreenWindow::GetNativeRenderSurface() const
{
  return dynamic_cast<NativeRenderSurface*>(mSurface.get());
}

void OffscreenWindow::OnPostRender()
{
  NativeRenderSurface* surface = GetNativeRenderSurface();

  if(!surface)
  {
    DALI_LOG_ERROR("NativeRenderSurface is null.");
    return;
  }

  Dali::OffscreenWindow handle(this);
  CallbackBase::Execute(*mPostRenderCallback, handle, surface->GetNativeRenderable());

  surface->ReleaseLock();
}

} // namespace Internal

} // namespace Dali
