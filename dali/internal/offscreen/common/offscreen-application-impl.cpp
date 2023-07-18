/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/offscreen/common/offscreen-window-impl.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Internal
{
using RenderMode = Dali::OffscreenApplication::RenderMode;

IntrusivePtr<OffscreenApplication> OffscreenApplication::New(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent, RenderMode renderMode)
{
  IntrusivePtr<OffscreenApplication> offscreenApplication = new OffscreenApplication(width, height, surface, isTranslucent, renderMode);
  return offscreenApplication;
}

OffscreenApplication::OffscreenApplication(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent, RenderMode renderMode)
{
  // Disable partial update
  EnvironmentVariable::SetEnvironmentVariable(DALI_ENV_DISABLE_PARTIAL_UPDATE, "1");

  // Disable ATSPI
  Dali::Accessibility::Bridge::DisableAutoInit();

  // Now we assume separated main loop for the offscreen application
  mFramework = Internal::Adaptor::GetFrameworkFactory()->CreateFramework(Internal::Adaptor::FrameworkBackend::GLIB, *this, *this, nullptr, nullptr, Adaptor::Framework::NORMAL, false);

  // Generate a default window
  IntrusivePtr<Internal::OffscreenWindow> impl = Internal::OffscreenWindow::New(width, height, surface, isTranslucent);
  mDefaultWindow                               = Dali::OffscreenWindow(impl.Get());

  mAdaptor.reset(Dali::Internal::Adaptor::Adaptor::New(Dali::Integration::SceneHolder(impl.Get()), impl->GetSurface(), NULL, renderMode == RenderMode::AUTO ? Dali::Internal::Adaptor::ThreadMode::NORMAL : Dali::Internal::Adaptor::ThreadMode::RUN_IF_REQUESTED));

  // Initialize default window
  impl->Initialize(true);
}

OffscreenApplication::~OffscreenApplication()
{
}

void OffscreenApplication::MainLoop()
{
  mFramework->Run();
}

void OffscreenApplication::Quit()
{
  // Actually quit the application.
  // Force a call to Quit even if adaptor is not running.
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).AddIdle(MakeCallback(this, &OffscreenApplication::QuitFromMainLoop), false, true);
}

Dali::OffscreenWindow OffscreenApplication::GetWindow()
{
  return mDefaultWindow;
}

void OffscreenApplication::RenderOnce()
{
  mAdaptor->RenderOnce();
}

void OffscreenApplication::OnInit()
{
  // Start the adaptor
  mAdaptor->Start();

  mInitSignal.Emit();

  mAdaptor->NotifySceneCreated();
}

void OffscreenApplication::OnTerminate()
{
  mTerminateSignal.Emit();

  // Stop the adaptor
  mAdaptor->Stop();

  mDefaultWindow.Reset();
}

void OffscreenApplication::QuitFromMainLoop()
{
  mAdaptor->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
}

} // namespace Internal

} // namespace Dali
