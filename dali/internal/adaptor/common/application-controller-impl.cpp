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
#include <dali/integration-api/string-utils.h>
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/graphics-backend.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>
#include <dali/internal/adaptor/common/ui-context-impl.h>
#include <dali/internal/graphics/common/graphics-backend-impl.h>
#include <dali/internal/graphics/common/graphics-factory.h>
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

ApplicationControllerPtr gLaunchpadApplicationController;
} // namespace

void ApplicationController::LaunchpadApplicationPreInitialize(int* argc, char** argv[])
{
  if(!gLaunchpadApplicationController)
  {
    Dali::TextAbstraction::FontClientPreInitialize();

    gLaunchpadApplicationController = ApplicationController::New(PositionSize());
    gLaunchpadApplicationController->LaunchpadPreInitialize();
  }
}

ApplicationControllerPtr ApplicationController::GetLaunchpadApplicationController()
{
  return gLaunchpadApplicationController;
}

ApplicationControllerPtr ApplicationController::New(PositionSize windowPositionSize)
{
  // If a launchpad pre-initialized controller exists, reuse it.
  if(gLaunchpadApplicationController)
  {
    return gLaunchpadApplicationController;
  }
  ApplicationControllerPtr controller(new ApplicationController(windowPositionSize));
  return controller;
}

ApplicationController::ApplicationController(PositionSize windowPositionSize)
: mWindowPositionSize(windowPositionSize)
{
  mEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());

  mFrameworkFactory = std::unique_ptr<FrameworkFactory>(Dali::Internal::Adaptor::CreateFrameworkFactory());

  // We don't need Framework, just set backend type
  mFrameworkFactory->SetFrameworkBackend(FrameworkBackend::DEFAULT);
}

ApplicationController::~ApplicationController()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::~ApplicationController\n");

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

  WindowSystem::Shutdown();
}

void ApplicationController::LaunchpadPreInitialize()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::LaunchpadPreInitialize\n");

  // Guard: already pre-initialized, nothing to do.
  if(mLaunchpadState == LaunchpadState::PRE_INITIALIZED)
  {
    DALI_LOG_RELEASE_INFO("ApplicationController::LaunchpadPreInitialize - already pre-initialized, skip\n");
    return;
  }

  // Set the preferred graphics backend before window creation.
  Graphics::Internal::SetPreferredGraphicsBackend(mEnvironmentOptions->GetGraphicsBackend());

  // Create window in pre-initialized (hidden) state.
  CreateWindow(true);

  // Create Adaptor
  CreateAdaptor();

  mLaunchpadState = LaunchpadState::PRE_INITIALIZED;
}

void ApplicationController::PreInitialize()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PreInitialize\n");

  UpdateEnvironmentOptions();

  if(mLaunchpadState == LaunchpadState::NONE)
  {
    // Create window as a normal (non-pre-initialized) window.
    CreateWindow(false);

    CreateAdaptor();
  }
  else
  {
    // PRE_INITIALIZED path

    // Send to Core that pre-initialized adaptor is ready to be used.
    Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).PreInitializeAdaptorCompleted();

    // Window was already created in LaunchpadPreInitialize().
    // Update window properties to match the real application's requirements.
    UpdatePreInitializedWindowInfo();
  }

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_ADAPTOR_START");
  mAdaptor->Start();
  DALI_TRACE_END(gTraceFilter, "DALI_APP_ADAPTOR_START");

  if(!mDeferCompletePreInitialize)
  {
    CompletePreInitialize();
  }
}

void ApplicationController::CompletePreInitialize()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::CompletePreInitialize\n");

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_PRE_INIT_SIGNAL");
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnPreInit();
  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_PRE_INIT_SIGNAL");
}

void ApplicationController::PostInitialize()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PostInitialize\n");

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnInit();

  mAdaptor->NotifySceneCreated();

  // Ensure the join of Font thread at this point
  Dali::TextAbstraction::FontClientJoinFontThreads();
}

void ApplicationController::PrePause()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PrePause\n");

  if(auto bridge = Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->ApplicationPaused();
  }

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnPause();
}

void ApplicationController::PostPause()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PostPause\n");
}

void ApplicationController::PreResume()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PreResume\n");

  if(auto bridge = Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->ApplicationResumed();
  }

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnResume();
}

void ApplicationController::PostResume()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PostResume\n");

  // DALi just delivers the framework Resume event to the application.
  // Resuming DALi core only occurs on the Window Show framework event

  // Trigger processing of events queued up while paused
  CoreEventInterface& coreEventInterface = Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor.get());
  coreEventInterface.ProcessCoreEvents();
}

void ApplicationController::PreTerminate()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PreTerminate\n");

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnTerminate();
}

void ApplicationController::PostTerminate()
{
  DALI_LOG_RELEASE_INFO("ApplicationController::PostTerminate\n");

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

void ApplicationController::PreReset()
{
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnReset();
}

void ApplicationController::PreLanguageChanged(const std::string& language)
{
  mAdaptor->NotifyLanguageChanged(language);

  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();
  GetImplementation(lifecycleController).OnLanguageChanged();
}

void ApplicationController::SetDeferCompletePreInitialize(bool defer)
{
  mDeferCompletePreInitialize = defer;
}

void ApplicationController::SetWindowPositionSize(PositionSize windowPositionSize)
{
  mWindowPositionSize = windowPositionSize;
}

void ApplicationController::SetWindowName(const std::string& windowName)
{
  mWindowName = windowName;
}

void ApplicationController::SetWindowData(const Dali::WindowData& windowData)
{
  mWindowData       = std::unique_ptr<Dali::WindowData>(new Dali::WindowData());
  PositionSize size = windowData.GetPositionSize();
  mWindowData->SetPositionSize(size);
  mWindowData->SetTransparency(windowData.GetTransparency());
  mWindowData->SetWindowType(windowData.GetWindowType());
  mWindowData->SetFrontBufferRendering(windowData.GetFrontBufferRendering());
  mWindowData->SetScreen(windowData.GetScreen());
  mWindowPositionSize = mWindowData->GetPositionSize();
}

void ApplicationController::CreateWindow(bool isPreInitialize)
{
  std::string finalWindowName{};
  std::string windowClassName{};

  if(!isPreInitialize)
  {
    PositionSize finalSize(0, 0, 0, 0);

    if(mWindowPositionSize.width > 0 && mWindowPositionSize.height > 0)
    {
      finalSize = mWindowPositionSize;
    }
    else if(mEnvironmentOptions && mEnvironmentOptions->GetWindowWidth() && mEnvironmentOptions->GetWindowHeight())
    {
      // In the normal path, try environment variable overrides before falling back.
      // Screen size fallback is handled by the underlying Window implementation.
      finalSize.width  = mEnvironmentOptions->GetWindowWidth();
      finalSize.height = mEnvironmentOptions->GetWindowHeight();
    }
    // If still (0,0), the window implementation will resolve the screen size.

    mWindowPositionSize = finalSize;

    // Window name and class are only resolved in the normal path.
    // During pre-initialization, the real application name is not yet known.
    windowClassName    = mEnvironmentOptions ? mEnvironmentOptions->GetWindowClassName() : "";
    auto envWindowName = mEnvironmentOptions ? mEnvironmentOptions->GetWindowName() : "";
    finalWindowName    = envWindowName.empty() ? mWindowName : envWindowName;
  }

  Dali::WindowData activeWindowData;

  activeWindowData.SetPositionSize(mWindowPositionSize);

  if(mWindowData)
  {
    activeWindowData.SetTransparency(mWindowData->GetTransparency());
    activeWindowData.SetWindowType(mWindowData->GetWindowType());
    activeWindowData.SetFrontBufferRendering(mWindowData->GetFrontBufferRendering());
    activeWindowData.SetScreen(mWindowData->GetScreen());
  }

#ifdef DALI_PROFILE_UBUNTU
  // Transparent window is not supported on Ubuntu platform.
  activeWindowData.SetTransparency(false);
#endif

  WindowSystem::Initialize();

  Dali::Any                        surface;
  Dali::Internal::Adaptor::Window* window = Dali::Internal::Adaptor::Window::New(surface, finalWindowName, windowClassName, activeWindowData, isPreInitialize);
  mMainWindow                             = Dali::Window(window);

  // No scene yet; too early to set depth/stencil/partialupdate/msaa params.
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

  mMainWindow.SetDepthBufferEnabled(mEnvironmentOptions->DepthBufferRequired());
  mMainWindow.SetStencilBufferEnabled(mEnvironmentOptions->StencilBufferRequired());
  mMainWindow.SetPartialUpdateEnabled(mEnvironmentOptions->PartialUpdateRequired());
  mMainWindow.SetMultiSampledAntiAliasingEnabled(mEnvironmentOptions->GetMultiSamplingLevel() > 0);
}

void ApplicationController::UpdateEnvironmentOptions()
{
  if(mLaunchpadState == LaunchpadState::PRE_INITIALIZED)
  {
    // PreInitialized case.
    DALI_ASSERT_ALWAYS(mEnvironmentOptions && "EnvironmentOptions should exist in the PRE_INITIALIZED state");
    DALI_ASSERT_ALWAYS(mMainWindow && "MainWindow should exist in the PRE_INITIALIZED state");

    // Compare with previous environment options, and replace it.
    auto latestEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());

    auto& mainWindowImpl = GetImplementation(mMainWindow);
    DALI_ASSERT_ALWAYS(mainWindowImpl.GetSurface() && "Surface should be initialized in the PRE_INITIALIZED state");

    if(DALI_UNLIKELY(mainWindowImpl.GetSurface()->GetSurfaceType() == Dali::Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE))
    {
      DALI_LOG_RELEASE_INFO("Re-create DisplayConnector by native render surface\n");
      Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GenerateDisplayConnector(mainWindowImpl.GetSurface()->GetSurfaceType());
    }

    Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).UpdateEnvironmentOptions(*latestEnvironmentOptions);

    mEnvironmentOptions->CopyEnvironmentOptions(*latestEnvironmentOptions);
  }

  // Call will be ignored if this function has already been called by the application.
  if(!Graphics::Internal::IsGraphicsBackendSet())
  {
    Graphics::SetGraphicsBackend(mEnvironmentOptions->GetGraphicsBackend());
  }

  DALI_ASSERT_ALWAYS(!(mLaunchpadState == LaunchpadState::NONE && Graphics::Internal::IsGraphicsResetRequired()) && "Normal launch case should never call preferred graphics backend!");

  // Reload graphics library if need
  if(DALI_UNLIKELY(mLaunchpadState == LaunchpadState::PRE_INITIALIZED && Graphics::Internal::IsGraphicsResetRequired()))
  {
    // dlclose for previous library and re-load if dynamic graphics backed case.
    Dali::Internal::Adaptor::ResetGraphicsLibrary(true);

    // Fix the graphics backend as current graphics now.
    Graphics::Internal::GraphicsResetCompleted();
  }
}

void ApplicationController::UpdatePreInitializedWindowInfo()
{
  // The real screen size may differ from the pre-initialized state.
  Dali::Internal::Adaptor::WindowSystem::UpdateScreenSize();

  PositionSize finalSize(0, 0, 0, 0);

  if(mWindowPositionSize.width > 0 && mWindowPositionSize.height > 0)
  {
    finalSize = mWindowPositionSize;
  }
  else if(mEnvironmentOptions && mEnvironmentOptions->GetWindowWidth() && mEnvironmentOptions->GetWindowHeight())
  {
    finalSize.width  = mEnvironmentOptions->GetWindowWidth();
    finalSize.height = mEnvironmentOptions->GetWindowHeight();
  }

  mWindowPositionSize = finalSize;

  if(finalSize.width == 0 || finalSize.height == 0)
  {
    int screenWidth, screenHeight;
    Dali::Internal::Adaptor::WindowSystem::GetScreenSize(screenWidth, screenHeight);

    if(screenWidth != mWindowPositionSize.width || screenHeight != mWindowPositionSize.height)
    {
      mWindowPositionSize.width  = screenWidth;
      mWindowPositionSize.height = screenHeight;
    }
  }

  auto windowClassName = mEnvironmentOptions ? mEnvironmentOptions->GetWindowClassName() : "";
  auto envWindowName   = mEnvironmentOptions ? mEnvironmentOptions->GetWindowName() : "";

  std::string finalWindowName = envWindowName.empty() ? mWindowName : envWindowName;
  mMainWindow.SetClass(Dali::Integration::ToDaliString(finalWindowName), Dali::Integration::ToDaliString(windowClassName));

  Dali::DevelWindow::SetPositionSize(mMainWindow, mWindowPositionSize);

  if(mWindowData)
  {
    // Apply the real application's window configuration to the pre-initialized window.
    mMainWindow.SetTransparency(mWindowData->GetTransparency());
    mMainWindow.SetType(mWindowData->GetWindowType());
    Dali::DevelWindow::SetFrontBufferRendering(mMainWindow, mWindowData->GetFrontBufferRendering());

    std::string screen = Dali::Integration::ToStdString(mWindowData->GetScreen());
    if(!screen.empty())
    {
      Dali::DevelWindow::SetScreen(mMainWindow, screen);
    }
  }

  if(mMainWindow.GetType() == Dali::WindowType::IME)
  {
    Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(mMainWindow);
    windowImpl.InitializeImeInfo();
  }

  mMainWindow.Show();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
