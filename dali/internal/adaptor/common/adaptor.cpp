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
#include <dali/integration-api/adaptor-framework/adaptor.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/object-registry.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/style-monitor.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/window-system/common/window-impl.h>

namespace Dali
{
Adaptor& Adaptor::New(Window window)
{
  Internal::Adaptor::SceneHolder* sceneHolder = &Dali::GetImplementation(window);
  Adaptor*                        adaptor     = Internal::Adaptor::Adaptor::New(Dali::Integration::SceneHolder(sceneHolder), NULL);
  return *adaptor;
}

Adaptor& Adaptor::New(Window window, const Dali::RenderSurfaceInterface& surface)
{
  Internal::Adaptor::SceneHolder* sceneHolder = &Dali::GetImplementation(window);
  Dali::RenderSurfaceInterface*   pSurface    = const_cast<Dali::RenderSurfaceInterface*>(&surface);
  Adaptor*                        adaptor     = Internal::Adaptor::Adaptor::New(Dali::Integration::SceneHolder(sceneHolder), pSurface, NULL, Dali::Internal::Adaptor::ThreadMode::NORMAL);
  return *adaptor;
}

Adaptor& Adaptor::New(Dali::Integration::SceneHolder window)
{
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New(window, NULL);
  return *adaptor;
}

Adaptor& Adaptor::New(Dali::Integration::SceneHolder window, const Dali::RenderSurfaceInterface& surface)
{
  Dali::RenderSurfaceInterface* pSurface = const_cast<Dali::RenderSurfaceInterface*>(&surface);
  Adaptor*                      adaptor  = Internal::Adaptor::Adaptor::New(window, pSurface, NULL, Dali::Internal::Adaptor::ThreadMode::NORMAL);
  return *adaptor;
}

Adaptor::~Adaptor()
{
  delete mImpl;
}

void Adaptor::Start()
{
  mImpl->Start();
}

void Adaptor::Pause()
{
  mImpl->Pause();
}

void Adaptor::Resume()
{
  mImpl->Resume();
}

void Adaptor::Stop()
{
  mImpl->Stop();
}

bool Adaptor::AddIdle(CallbackBase* callback, bool hasReturnValue)
{
  if(IsAvailable())
  {
    return mImpl->AddIdle(callback, hasReturnValue);
  }
  else
  {
    DALI_LOG_ERROR("Adaptor not instantiated");
    return false;
  }
}

bool Adaptor::AddWindow(Dali::Integration::SceneHolder childWindow)
{
  if(IsAvailable())
  {
    return mImpl->AddWindow(childWindow);
  }
  else
  {
    DALI_LOG_ERROR("Adaptor not instantiated");
    return false;
  }
}

void Adaptor::RemoveIdle(CallbackBase* callback)
{
  if(IsAvailable())
  {
    mImpl->RemoveIdle(callback);
  }
  else
  {
    DALI_LOG_ERROR("Adaptor not instantiated");
  }
}

void Adaptor::ProcessIdle()
{
  if(IsAvailable())
  {
    mImpl->ProcessIdle();
  }
  else
  {
    DALI_LOG_ERROR("Adaptor not instantiated");
  }
}

void Adaptor::ReplaceSurface(Window window, Dali::RenderSurfaceInterface& surface)
{
  Internal::Adaptor::SceneHolder* sceneHolder = &Dali::GetImplementation(window);
  mImpl->ReplaceSurface(Dali::Integration::SceneHolder(sceneHolder), surface);
}

void Adaptor::ReplaceSurface(Dali::Integration::SceneHolder window, Dali::RenderSurfaceInterface& surface)
{
  mImpl->ReplaceSurface(window, surface);
}

Adaptor::AdaptorSignalType& Adaptor::ResizedSignal()
{
  return mImpl->ResizedSignal();
}

Adaptor::AdaptorSignalType& Adaptor::LanguageChangedSignal()
{
  return mImpl->LanguageChangedSignal();
}

Adaptor::WindowCreatedSignalType& Adaptor::WindowCreatedSignal()
{
  return mImpl->WindowCreatedSignal();
}

Adaptor::LocaleChangedSignalType& Adaptor::LocaleChangedSignal()
{
  return mImpl->LocaleChangedSignal();
}

Dali::RenderSurfaceInterface& Adaptor::GetSurface()
{
  return mImpl->GetSurface();
}

Any Adaptor::GetNativeWindowHandle()
{
  return mImpl->GetNativeWindowHandle();
}

Any Adaptor::GetNativeWindowHandle(Actor actor)
{
  return mImpl->GetNativeWindowHandle(actor);
}

Any Adaptor::GetGraphicsDisplay()
{
  return mImpl->GetGraphicsDisplay();
}

void Adaptor::ReleaseSurfaceLock()
{
  mImpl->ReleaseSurfaceLock();
}

void Adaptor::SetRenderRefreshRate(unsigned int numberOfVSyncsPerRender)
{
  mImpl->SetRenderRefreshRate(numberOfVSyncsPerRender);
}

void Adaptor::SetPreRenderCallback(CallbackBase* callback)
{
  mImpl->SetPreRenderCallback(callback);
}

Adaptor& Adaptor::Get()
{
  return Internal::Adaptor::Adaptor::Get();
}

bool Adaptor::IsAvailable()
{
  return Internal::Adaptor::Adaptor::IsAvailable();
}

void Adaptor::NotifySceneCreated()
{
  mImpl->NotifySceneCreated();
}

void Adaptor::NotifyLanguageChanged()
{
  mImpl->NotifyLanguageChanged();
}

void Adaptor::FeedTouchPoint(TouchPoint& point, int timeStamp)
{
  mImpl->FeedTouchPoint(point, timeStamp);
}

void Adaptor::FeedWheelEvent(WheelEvent& wheelEvent)
{
  mImpl->FeedWheelEvent(wheelEvent);
}

void Adaptor::FeedKeyEvent(KeyEvent& keyEvent)
{
  mImpl->FeedKeyEvent(keyEvent);
}

void Adaptor::SceneCreated()
{
  mImpl->SceneCreated();
}

void Adaptor::SurfaceResizePrepare(Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize)
{
  mImpl->SurfaceResizePrepare(surface, surfaceSize);
}

void Adaptor::SurfaceResizeComplete(Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize)
{
  mImpl->SurfaceResizeComplete(surface, surfaceSize);
}

void Adaptor::UpdateOnce()
{
  mImpl->RequestUpdateOnce();
}

void Adaptor::RenderOnce()
{
  mImpl->RenderOnce();
}

void Adaptor::FlushUpdateMessages()
{
  mImpl->FlushUpdateMessages();
}

void Adaptor::RequestProcessEventsOnIdle()
{
  mImpl->RequestProcessEventsOnIdle();
}

const LogFactoryInterface& Adaptor::GetLogFactory()
{
  return mImpl->GetLogFactory();
}

const TraceFactoryInterface& Adaptor::GetTraceFactory()
{
  return mImpl->GetTraceFactory();
}

void Adaptor::RegisterProcessor(Integration::Processor& processor, bool postProcessor)
{
  mImpl->RegisterProcessor(processor, postProcessor);
}

void Adaptor::UnregisterProcessor(Integration::Processor& processor, bool postProcessor)
{
  mImpl->UnregisterProcessor(processor, postProcessor);
}

void Adaptor::RegisterProcessorOnce(Integration::Processor& processor, bool postProcessor)
{
  mImpl->RegisterProcessorOnce(processor, postProcessor);
}

void Adaptor::UnregisterProcessorOnce(Integration::Processor& processor, bool postProcessor)
{
  mImpl->UnregisterProcessorOnce(processor, postProcessor);
}

Dali::WindowContainer Adaptor::GetWindows() const
{
  return mImpl->GetWindows();
}

SceneHolderList Adaptor::GetSceneHolders() const
{
  return mImpl->GetSceneHolders();
}

Dali::ObjectRegistry Adaptor::GetObjectRegistry() const
{
  return mImpl->GetObjectRegistry();
}

void Adaptor::OnWindowShown()
{
  mImpl->OnWindowShown();
}

void Adaptor::OnWindowHidden()
{
  mImpl->OnWindowHidden();
}

Adaptor::Adaptor()
: mImpl(NULL)
{
}

} // namespace Dali
