/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/window-impl.h>

#ifdef DALI_ADAPTOR_COMPILATION
#include <dali/integration-api/scene-holder.h>
#else
#include <dali/integration-api/adaptors/scene-holder.h>
#endif

namespace Dali
{

Adaptor& Adaptor::New( Window window )
{
  return New( window, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
}

Adaptor& Adaptor::New( Window window, Configuration::ContextLoss configuration )
{
  Internal::Adaptor::SceneHolder* sceneHolder = &Dali::GetImplementation( window );
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New( Dali::Integration::SceneHolder( sceneHolder ), configuration, NULL );
  return *adaptor;
}

Adaptor& Adaptor::New( Window window, const Dali::RenderSurfaceInterface& surface )
{
  return New( window, surface, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
}

Adaptor& Adaptor::New( Window window, const Dali::RenderSurfaceInterface& surface, Configuration::ContextLoss configuration )
{
  Internal::Adaptor::SceneHolder* sceneHolder = &Dali::GetImplementation( window );
  Dali::RenderSurfaceInterface* pSurface = const_cast<Dali::RenderSurfaceInterface *>(&surface);
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New( Dali::Integration::SceneHolder( sceneHolder ), pSurface, configuration, NULL );
  return *adaptor;
}

Adaptor& Adaptor::New( Dali::Integration::SceneHolder window )
{
  return New( window, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
}

Adaptor& Adaptor::New( Dali::Integration::SceneHolder window, Configuration::ContextLoss configuration )
{
  Adaptor* adaptor = Internal::Adaptor::Adaptor::New( window, configuration, NULL );
  return *adaptor;
}

Adaptor& Adaptor::New( Dali::Integration::SceneHolder window, const Dali::RenderSurfaceInterface& surface )
{
  return New( window, surface, Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS );
}

Adaptor& Adaptor::New( Dali::Integration::SceneHolder window, const Dali::RenderSurfaceInterface& surface, Configuration::ContextLoss configuration )
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

bool Adaptor::AddWindow( Dali::Integration::SceneHolder childWindow, const std::string& childWindowName, const std::string& childWindowClassName, bool childWindowMode )
{
  return mImpl->AddWindow( childWindow, childWindowName, childWindowClassName, childWindowMode );
}

void Adaptor::RemoveIdle( CallbackBase* callback )
{
  mImpl->RemoveIdle( callback );
}

void Adaptor::ReplaceSurface( Window window, Dali::RenderSurfaceInterface& surface )
{
  Internal::Adaptor::SceneHolder* sceneHolder = &Dali::GetImplementation( window );
  mImpl->ReplaceSurface( Dali::Integration::SceneHolder( sceneHolder ), surface );
}

void Adaptor::ReplaceSurface( Dali::Integration::SceneHolder window, Dali::RenderSurfaceInterface& surface )
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

Adaptor::WindowCreatedSignalType& Adaptor::WindowCreatedSignal()
{
  return mImpl->WindowCreatedSignal();
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

void Adaptor::SurfaceResizePrepare( Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize )
{
  mImpl->SurfaceResizePrepare( surface, surfaceSize );
}

void Adaptor::SurfaceResizeComplete( Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize )
{
  mImpl->SurfaceResizeComplete( surface, surfaceSize );
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

Dali::WindowContainer Adaptor::GetWindows() const
{
  return mImpl->GetWindows();
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
: mImpl( NULL )
{
}

} // namespace Dali
