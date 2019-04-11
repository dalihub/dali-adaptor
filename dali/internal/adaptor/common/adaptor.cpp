/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-adaptor.h>
#include <dali/devel-api/adaptor-framework/style-monitor.h>
#include <dali/integration-api/render-surface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{

Adaptor& Adaptor::New( Window window )
{
  return New( window, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
}

Adaptor& Adaptor::New( Window window, Configuration::ContextLoss configuration )
{
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New( window, configuration, NULL );
  return *adaptor;
}

Adaptor& Adaptor::New( Window window, const Dali::RenderSurfaceInterface& surface )
{
  return New( window, surface, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
}

Adaptor& Adaptor::New( Window window, const Dali::RenderSurfaceInterface& surface, Configuration::ContextLoss configuration )
{
  Dali::RenderSurfaceInterface* pSurface = const_cast<Dali::RenderSurfaceInterface *>(&surface);
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New( window, pSurface, configuration, NULL );
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

bool Adaptor::AddIdle( CallbackBase* callback, bool hasReturnValue )
{
  return mImpl->AddIdle( callback, hasReturnValue, false );
}

void Adaptor::RemoveIdle( CallbackBase* callback )
{
  mImpl->RemoveIdle( callback );
}

void Adaptor::ReplaceSurface( Window window, Dali::RenderSurfaceInterface& surface )
{
  mImpl->ReplaceSurface( window, surface );
}

Adaptor::AdaptorSignalType& Adaptor::ResizedSignal()
{
  return mImpl->ResizedSignal();
}

Adaptor::AdaptorSignalType& Adaptor::LanguageChangedSignal()
{
  return mImpl->LanguageChangedSignal();
}

Dali::RenderSurfaceInterface& Adaptor::GetSurface()
{
  return mImpl->GetSurface();
}

Any Adaptor::GetNativeWindowHandle()
{
  return mImpl->GetNativeWindowHandle();
}

Any Adaptor::GetGraphicsDisplay()
{
  return mImpl->GetGraphicsDisplay();
}

void Adaptor::ReleaseSurfaceLock()
{
  mImpl->ReleaseSurfaceLock();
}

void Adaptor::SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender )
{
  mImpl->SetRenderRefreshRate( numberOfVSyncsPerRender );
}

void Adaptor::SetPreRenderCallback( CallbackBase* callback )
{
  mImpl->SetPreRenderCallback( callback );
}

void Adaptor::SetUseHardwareVSync(bool useHardware)
{
  mImpl->SetUseHardwareVSync( useHardware );
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

void Adaptor::SetMinimumPinchDistance(float distance)
{
  mImpl->SetMinimumPinchDistance(distance);
}

void Adaptor::FeedTouchPoint( TouchPoint& point, int timeStamp )
{
  mImpl->FeedTouchPoint(point, timeStamp);
}

void Adaptor::FeedWheelEvent( WheelEvent& wheelEvent )
{
  mImpl->FeedWheelEvent(wheelEvent);
}

void Adaptor::FeedKeyEvent( KeyEvent& keyEvent )
{
  mImpl->FeedKeyEvent(keyEvent);
}

void Adaptor::SceneCreated()
{
  mImpl->SceneCreated();
}

void Adaptor::RenderOnce()
{
  mImpl->RenderOnce();
}

const LogFactoryInterface& Adaptor::GetLogFactory()
{
  return mImpl->GetLogFactory();
}

void Adaptor::RegisterProcessor( Integration::Processor& processor )
{
  mImpl->RegisterProcessor( processor );
}

void Adaptor::UnregisterProcessor( Integration::Processor& processor )
{
  mImpl->UnregisterProcessor( processor );
}

Adaptor::Adaptor()
: mImpl( NULL )
{
}

} // namespace Dali
