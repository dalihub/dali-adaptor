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
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/offscreen/common/offscreen-window-impl.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Internal
{
namespace
{
void EmitLifecycleControllerSignal(void (Internal::Adaptor::LifecycleController::*member)(Dali::Application&))
{
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  if(DALI_LIKELY(lifecycleController))
  {
    Dali::Application dummyApplication;
    (GetImplementation(lifecycleController).*member)(dummyApplication);
  }
}
} // namespace
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

  // Create environment options after environmnet value changed.
  mEnvironmentOptions = std::unique_ptr<Dali::Internal::Adaptor::EnvironmentOptions>(new Dali::Internal::Adaptor::EnvironmentOptions());

  auto& adaptorBuilder = Dali::Internal::Adaptor::AdaptorBuilder::Get(*mEnvironmentOptions);

  // Now we assume separated main loop for the offscreen application
  mFrameworkFactory = std::unique_ptr<Adaptor::FrameworkFactory>(Dali::Internal::Adaptor::CreateFrameworkFactory());
  mFramework        = mFrameworkFactory->CreateFramework(Internal::Adaptor::FrameworkBackend::GLIB, *this, *this, nullptr, nullptr, Adaptor::Framework::NORMAL, false);

  // Generate a default window
  IntrusivePtr<Internal::OffscreenWindow> impl = Internal::OffscreenWindow::New(width, height, surface, isTranslucent);
  mDefaultWindow                               = Dali::OffscreenWindow(impl.Get());

  auto& graphicsFactory = adaptorBuilder.GetGraphicsFactory();

  mAdaptor.reset(Dali::Internal::Adaptor::Adaptor::New(graphicsFactory, Dali::Integration::SceneHolder(impl.Get()), impl->GetSurface(), mEnvironmentOptions.get(), renderMode == RenderMode::AUTO ? Dali::Internal::Adaptor::ThreadMode::NORMAL : Dali::Internal::Adaptor::ThreadMode::RUN_IF_REQUESTED));

  // adaptorBuilder invalidate after now.
  Dali::Internal::Adaptor::AdaptorBuilder::Finalize();

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
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).AddIdle(MakeCallback(this, &OffscreenApplication::QuitFromMainLoop), false);
}

Dali::OffscreenWindow OffscreenApplication::GetWindow()
{
  return mDefaultWindow;
}

void OffscreenApplication::RenderOnce()
{
  mAdaptor->RenderOnce();
}

Any OffscreenApplication::GetFrameworkContext() const
{
  return mFramework->GetMainLoopContext();
}

void OffscreenApplication::OnInit()
{
  // Start the adaptor
  mAdaptor->Start();

  Dali::OffscreenApplication application(this);
  mInitSignal.Emit();
  EmitLifecycleControllerSignal(&Internal::Adaptor::LifecycleController::OnInit);

  mAdaptor->NotifySceneCreated();
}

void OffscreenApplication::OnTerminate()
{
  Dali::OffscreenApplication application(this);
  mTerminateSignal.Emit();
  EmitLifecycleControllerSignal(&Internal::Adaptor::LifecycleController::OnTerminate);

  // Stop the adaptor
  mAdaptor->Stop();

  mDefaultWindow.Reset();
}

void OffscreenApplication::OnPause()
{
  Dali::OffscreenApplication application(this);
  mPauseSignal.Emit();
  EmitLifecycleControllerSignal(&Internal::Adaptor::LifecycleController::OnPause);
}

void OffscreenApplication::OnResume()
{
  Dali::OffscreenApplication application(this);
  mResumeSignal.Emit();
  EmitLifecycleControllerSignal(&Internal::Adaptor::LifecycleController::OnResume);

  // DALi just delivers the framework Resume event to the application.
  // Resuming DALi core only occurs on the Window Show framework event

  // Trigger processing of events queued up while paused
  Internal::Adaptor::CoreEventInterface& coreEventInterface = Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor);
  coreEventInterface.ProcessCoreEvents();
}

void OffscreenApplication::OnReset()
{
  /*
   * usually, reset callback was called when a caller request to launch this application via aul.
   * because Application class already handled initialization in OnInit(), OnReset do nothing.
   */
  Dali::OffscreenApplication application(this);
  mResetSignal.Emit();
  EmitLifecycleControllerSignal(&Internal::Adaptor::LifecycleController::OnReset);
}

void OffscreenApplication::OnLanguageChanged()
{
  mAdaptor->NotifyLanguageChanged();

  Dali::OffscreenApplication application(this);
  mLanguageChangedSignal.Emit();
  EmitLifecycleControllerSignal(&Internal::Adaptor::LifecycleController::OnLanguageChanged);
}

void OffscreenApplication::QuitFromMainLoop()
{
  mAdaptor->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
}

} // namespace Internal

} // namespace Dali
