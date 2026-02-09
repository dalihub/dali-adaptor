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
#include <dali/internal/adaptor/common/application-controller-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>
#include <dali/internal/adaptor/common/ui-context-impl.h>
#include <dali/internal/graphics/common/graphics-backend-impl.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_APPLICATION_CONTROLLER, false);
} // namespace

ApplicationControllerPtr ApplicationController::New(PositionSize windowPositionSize, bool useUiThread)
{
  ApplicationControllerPtr controller(new ApplicationController(windowPositionSize, useUiThread));
  return controller;
}

ApplicationController::ApplicationController(PositionSize windowPositionSize, bool useUiThread)
: mWindowPositionSize(windowPositionSize),
  mUseUiThread(useUiThread)
{
  const char* uiThreadEnabled = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ENABLE_UI_THREAD);
  if(uiThreadEnabled && std::atoi(uiThreadEnabled) != 0)
  {
    //TODO: What should we do?
    mUseUiThread = true;
  }

  mFrameworkFactory = std::unique_ptr<FrameworkFactory>(Dali::Internal::Adaptor::CreateFrameworkFactory());

  // We don't need Framework, just set backend type
  mFrameworkFactory->SetFrameworkBackend(FrameworkBackend::GLIB);
}

ApplicationController::~ApplicationController()
{
  mUiContext.Reset();

  SingletonService service = SingletonService::Get();
  if(service)
  {
    service.UnregisterAll();
  }

  if(mMainWindow)
  {
    mMainWindow.Reset();
  }
}

void ApplicationController::PreInitialize()
{
  UpdateEnvironmentOptions();

  CreateWindow();
  CreateAdaptor();

  // Run the adaptor
  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_ADAPTOR_START");
  mAdaptor->Start();
  DALI_TRACE_END(gTraceFilter, "DALI_APP_ADAPTOR_START");

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_PRE_INIT_SIGNAL");
  GetImplementation(lifecycleController).OnPreInit();
  DALI_TRACE_END(gTraceFilter, "DALI_APP_EMIT_PRE_INIT_SIGNAL");
}

void ApplicationController::PostInitialize()
{
  mAdaptor->NotifySceneCreated();

  // Ensure the join of Font thread at this point
  Dali::TextAbstraction::FontClientJoinFontThreads();
}

void ApplicationController::PrePause()
{
  if(auto bridge = Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->ApplicationPaused();
  }

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnPause();
}

void ApplicationController::PostPause()
{
}

void ApplicationController::PreResume()
{
  if(auto bridge = Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->ApplicationResumed();
  }

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnResume();
}

void ApplicationController::PostResume()
{
  CoreEventInterface& coreEventInterface = Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor.get());
  coreEventInterface.ProcessCoreEvents();
}

void ApplicationController::PreTerminate()
{
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnTerminate();
}

void ApplicationController::PostTerminate()
{
  if(mAdaptor)
  {
    // Ensure that the render-thread is not using the surface(window) after we delete it
    mAdaptor->Stop();
  }

  if(mMainWindow)
  {
    DALI_LOG_RELEASE_INFO("Main window reset at app terminated case\n");
    mMainWindow.Reset(); // This only resets (clears) the default Window
  }

  UiContext& uiControlImpl = Internal::Adaptor::GetImplementation(mUiContext);
  uiControlImpl.SetDefaultWindow(Dali::Window());
}

void ApplicationController::PreLanguageChanged(const std::string& language)
{
  mAdaptor->NotifyLanguageChanged(language);
}

void ApplicationController::CreateWindow()
{
  Internal::Adaptor::Window* window;

  DALI_LOG_RELEASE_INFO("Create Default Window\n");

  WindowSystem::Initialize();

  window = Internal::Adaptor::Window::New(mWindowPositionSize);

  mMainWindow = Dali::Window(window);

  DALI_LOG_RELEASE_INFO("Main window created done\n");
}

void ApplicationController::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS(mMainWindow && "Window required to create adaptor");

  DALI_LOG_RELEASE_INFO("CreateAdaptor!\n");

  DALI_ASSERT_ALWAYS(mEnvironmentOptions && "Should have environment options before create adaptor!");

  auto& adaptorBuilder  = AdaptorBuilder::Get(*mEnvironmentOptions);
  auto& graphicsFactory = adaptorBuilder.GetGraphicsFactory();

  Integration::SceneHolder sceneHolder = Integration::SceneHolder(&Dali::GetImplementation(mMainWindow));

  mAdaptor = std::unique_ptr<Dali::Adaptor>(Adaptor::New(graphicsFactory, sceneHolder, mEnvironmentOptions.get()));

  // adaptorBuilder invalidate after now.
  AdaptorBuilder::Finalize();

  mUiContext = UiContext::New(mAdaptor.get());

  UiContext& uiControlImpl = Internal::Adaptor::GetImplementation(mUiContext);
  uiControlImpl.SetDefaultWindow(mMainWindow);
}

void ApplicationController::UpdateEnvironmentOptions()
{
  if(!mEnvironmentOptions)
  {
    mEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());
  }

  // Call will be ignored if this function has already been called by the application.
  if(!Graphics::Internal::IsGraphicsBackendSet())
  {
    Graphics::SetGraphicsBackend(mEnvironmentOptions->GetGraphicsBackend());
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
