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
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/actors/actor-devel.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/addon-manager.h>
#include <dali/integration-api/context-notifier.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <dali/integration-api/input-options.h>
#include <dali/integration-api/processor-interface.h>
#include <dali/integration-api/profiling.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/object-registry.h>

#include <sys/stat.h>
#include <cerrno>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/text-abstraction/font-client.h>

#include <dali/internal/accessibility/common/tts-player-impl.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/addons/common/addon-manager-factory.h>
#include <dali/internal/addons/common/addon-manager-impl.h>
#include <dali/internal/imaging/common/image-loader-plugin-proxy.h>
#include <dali/internal/imaging/common/image-loader.h>
#include <dali/internal/system/common/callback-manager.h>
#include <dali/internal/system/common/configuration-manager.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/system/common/logging.h>
#include <dali/internal/system/common/object-profiler.h>
#include <dali/internal/system/common/performance-interface-factory.h>
#include <dali/internal/system/common/system-error-print.h>
#include <dali/internal/system/common/system-factory.h>
#include <dali/internal/system/common/thread-controller.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/display-utils.h> // For Utils::MakeUnique
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

using Dali::TextAbstraction::FontClient;

extern std::string GetSystemCachePath();
extern std::string GetProgramBinaryPath();
extern std::string GetInternalProgramBinaryCachePath();
extern std::string GetCustomProgramBinaryCachePath();

namespace Dali::Internal::Adaptor
{
namespace
{
thread_local Adaptor* gThreadLocalAdaptor = nullptr; // raw thread specific pointer to allow Adaptor::Get

DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_PERFORMANCE_MARKER, false);
} // unnamed namespace

Dali::Adaptor* Adaptor::New(Dali::Integration::SceneHolder window, Dali::Integration::RenderSurfaceInterface* surface, EnvironmentOptions* environmentOptions, ThreadMode threadMode)
{
  Dali::Adaptor* adaptor = new Dali::Adaptor;
  Adaptor*       impl    = new Adaptor(window, *adaptor, surface, environmentOptions, threadMode);
  adaptor->mImpl         = impl;

  AdaptorBuilder& adaptorBuilder  = AdaptorBuilder::Get(*(impl->mEnvironmentOptions));
  auto&           graphicsFactory = adaptorBuilder.GetGraphicsFactory();
  impl->Initialize(graphicsFactory);

  AdaptorBuilder::Finalize();

  return adaptor;
}

Dali::Adaptor* Adaptor::New(Dali::Integration::SceneHolder window, EnvironmentOptions* environmentOptions)
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation(window);
  Dali::Adaptor*                  adaptor    = New(window, windowImpl.GetSurface(), environmentOptions, ThreadMode::NORMAL);
  windowImpl.SetAdaptor(*adaptor);
  return adaptor;
}

Dali::Adaptor* Adaptor::New(GraphicsFactoryInterface& graphicsFactory, Dali::Integration::SceneHolder window, Dali::Integration::RenderSurfaceInterface* surface, EnvironmentOptions* environmentOptions, ThreadMode threadMode)
{
  Dali::Adaptor* adaptor = new Dali::Adaptor;                                                      // Public adaptor
  Adaptor*       impl    = new Adaptor(window, *adaptor, surface, environmentOptions, threadMode); // Impl adaptor
  adaptor->mImpl         = impl;

  impl->Initialize(graphicsFactory);

  return adaptor;
} // Called second

Dali::Adaptor* Adaptor::New(GraphicsFactoryInterface& graphicsFactory, Dali::Integration::SceneHolder window, EnvironmentOptions* environmentOptions)
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation(window);
  Dali::Adaptor*                  adaptor    = New(graphicsFactory, window, windowImpl.GetSurface(), environmentOptions, ThreadMode::NORMAL);
  windowImpl.SetAdaptor(*adaptor);
  return adaptor;
} // Called first

void Adaptor::Initialize(GraphicsFactoryInterface& graphicsFactory)
{
  // all threads here (event, update, and render) will send their logs to TIZEN Platform's LogMessage handler.
  Dali::Integration::Log::LogFunction logFunction(Dali::TizenPlatform::LogMessage);
  mEnvironmentOptions->SetLogFunction(logFunction);
  mEnvironmentOptions->InstallLogFunction(); // install logging for main thread

  DALI_LOG_RELEASE_INFO("Adaptor::Initialize\n");

  mPlatformAbstraction = new TizenPlatform::TizenPlatformAbstraction;

  std::string path;
  GetDataStoragePath(path);
  mPlatformAbstraction->SetDataStoragePath(path);

  if(mEnvironmentOptions->PerformanceServerRequired())
  {
    mPerformanceInterface = PerformanceInterfaceFactory::CreateInterface(*this, *mEnvironmentOptions);
  }

  mEnvironmentOptions->CreateTraceManager(mPerformanceInterface);
  mEnvironmentOptions->InstallTraceFunction(); // install tracing for main thread

  mCallbackManager = Dali::Internal::Adaptor::GetSystemFactory()->CreateCallbackManager();

  Dali::Internal::Adaptor::SceneHolder* defaultWindow = mWindows.front();

  DALI_ASSERT_DEBUG(defaultWindow->GetSurface() && "Surface not initialized");

  mGraphics = std::unique_ptr<Graphics::GraphicsInterface>(&graphicsFactory.Create());

  // Create the AddOnManager
  mAddOnManager.reset(Dali::Internal::AddOnManagerFactory::CreateAddOnManager());

  Integration::CorePolicyFlags corePolicyFlags = Integration::CorePolicyFlags::DEFAULT;
  if(0u != mEnvironmentOptions->GetRenderToFboInterval())
  {
    corePolicyFlags |= Integration::CorePolicyFlags::RENDER_TO_FRAME_BUFFER;
  }
  if(Integration::DepthBufferAvailable::TRUE == mGraphics->GetDepthBufferRequired())
  {
    corePolicyFlags |= Integration::CorePolicyFlags::DEPTH_BUFFER_AVAILABLE;
  }
  if(Integration::StencilBufferAvailable::TRUE == mGraphics->GetStencilBufferRequired())
  {
    corePolicyFlags |= Integration::CorePolicyFlags::STENCIL_BUFFER_AVAILABLE;
  }
  if(Integration::PartialUpdateAvailable::TRUE == mGraphics->GetPartialUpdateRequired())
  {
    corePolicyFlags |= Integration::CorePolicyFlags::PARTIAL_UPDATE_AVAILABLE;
  }

  mCore = Integration::Core::New(*this,
                                 *mPlatformAbstraction,
                                 mGraphics->GetController(),
                                 corePolicyFlags);

  // Create TextureUploadManager after mCore created
  mTextureUploadManager = Dali::Devel::TextureUploadManager::Get();

  defaultWindow->SetAdaptor(Get());

  Dali::Integration::SceneHolder defaultSceneHolder(defaultWindow);

  mWindowCreatedSignal.Emit(defaultSceneHolder);

  const unsigned int timeInterval = mEnvironmentOptions->GetObjectProfilerInterval();
  if(0u < timeInterval)
  {
    mObjectProfiler = new ObjectProfiler(mCore->GetObjectRegistry(), timeInterval);
  }

  const uint32_t poolTimeInterval = mEnvironmentOptions->GetMemoryPoolInterval();
  if(0u < poolTimeInterval)
  {
    mMemoryPoolTimer = Dali::Timer::New(poolTimeInterval * 1000);
    mMemoryPoolTimer.TickSignal().Connect(mMemoryPoolTimerSlotDelegate, &Adaptor::MemoryPoolTimeout);
    mMemoryPoolTimer.Start();
  }

  mNotificationTrigger = TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &Adaptor::ProcessCoreEvents), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);
  DALI_LOG_DEBUG_INFO("mNotificationTrigger Trigger Id(%u)\n", mNotificationTrigger->GetId());

  mDisplayConnection = Dali::DisplayConnection::New(defaultWindow->GetSurface()->GetSurfaceType());

  mThreadController = new ThreadController(*this, *mEnvironmentOptions, mThreadMode);

  // Should be called after Core creation
  if(mEnvironmentOptions->GetPanGestureLoggingLevel())
  {
    Integration::EnableProfiling(Dali::Integration::PROFILING_TYPE_PAN_GESTURE);
  }
  if(mEnvironmentOptions->GetPanGesturePredictionMode() >= 0)
  {
    Integration::SetPanGesturePredictionMode(mEnvironmentOptions->GetPanGesturePredictionMode());
  }
  if(mEnvironmentOptions->GetPanGesturePredictionAmount() >= 0)
  {
    Integration::SetPanGesturePredictionAmount(mEnvironmentOptions->GetPanGesturePredictionAmount());
  }
  if(mEnvironmentOptions->GetPanGestureMaximumPredictionAmount() >= 0)
  {
    Integration::SetPanGestureMaximumPredictionAmount(mEnvironmentOptions->GetPanGestureMaximumPredictionAmount());
  }
  if(mEnvironmentOptions->GetPanGestureMinimumPredictionAmount() >= 0)
  {
    Integration::SetPanGestureMinimumPredictionAmount(mEnvironmentOptions->GetPanGestureMinimumPredictionAmount());
  }
  if(mEnvironmentOptions->GetPanGesturePredictionAmountAdjustment() >= 0)
  {
    Integration::SetPanGesturePredictionAmountAdjustment(mEnvironmentOptions->GetPanGesturePredictionAmountAdjustment());
  }
  if(mEnvironmentOptions->GetPanGestureSmoothingMode() >= 0)
  {
    Integration::SetPanGestureSmoothingMode(mEnvironmentOptions->GetPanGestureSmoothingMode());
  }
  if(mEnvironmentOptions->GetPanGestureSmoothingAmount() >= 0.0f)
  {
    Integration::SetPanGestureSmoothingAmount(mEnvironmentOptions->GetPanGestureSmoothingAmount());
  }
  if(mEnvironmentOptions->GetPanGestureUseActualTimes() >= 0)
  {
    Integration::SetPanGestureUseActualTimes(mEnvironmentOptions->GetPanGestureUseActualTimes() == 0 ? true : false);
  }
  if(mEnvironmentOptions->GetPanGestureInterpolationTimeRange() >= 0)
  {
    Integration::SetPanGestureInterpolationTimeRange(mEnvironmentOptions->GetPanGestureInterpolationTimeRange());
  }
  if(mEnvironmentOptions->GetPanGestureScalarOnlyPredictionEnabled() >= 0)
  {
    Integration::SetPanGestureScalarOnlyPredictionEnabled(mEnvironmentOptions->GetPanGestureScalarOnlyPredictionEnabled() == 0 ? true : false);
  }
  if(mEnvironmentOptions->GetPanGestureTwoPointPredictionEnabled() >= 0)
  {
    Integration::SetPanGestureTwoPointPredictionEnabled(mEnvironmentOptions->GetPanGestureTwoPointPredictionEnabled() == 0 ? true : false);
  }
  if(mEnvironmentOptions->GetPanGestureTwoPointInterpolatePastTime() >= 0)
  {
    Integration::SetPanGestureTwoPointInterpolatePastTime(mEnvironmentOptions->GetPanGestureTwoPointInterpolatePastTime());
  }
  if(mEnvironmentOptions->GetPanGestureTwoPointVelocityBias() >= 0.0f)
  {
    Integration::SetPanGestureTwoPointVelocityBias(mEnvironmentOptions->GetPanGestureTwoPointVelocityBias());
  }
  if(mEnvironmentOptions->GetPanGestureTwoPointAccelerationBias() >= 0.0f)
  {
    Integration::SetPanGestureTwoPointAccelerationBias(mEnvironmentOptions->GetPanGestureTwoPointAccelerationBias());
  }
  if(mEnvironmentOptions->GetPanGestureMultitapSmoothingRange() >= 0)
  {
    Integration::SetPanGestureMultitapSmoothingRange(mEnvironmentOptions->GetPanGestureMultitapSmoothingRange());
  }
  if(mEnvironmentOptions->GetMinimumPanDistance() >= 0)
  {
    Integration::SetPanGestureMinimumDistance(mEnvironmentOptions->GetMinimumPanDistance());
  }
  if(mEnvironmentOptions->GetMinimumPanEvents() >= 0)
  {
    Integration::SetPanGestureMinimumPanEvents(mEnvironmentOptions->GetMinimumPanEvents());
  }
  if(mEnvironmentOptions->GetMinimumPinchDistance() >= 0)
  {
    Integration::SetPinchGestureMinimumDistance(mEnvironmentOptions->GetMinimumPinchDistance());
  }
  if(mEnvironmentOptions->GetMinimumPinchTouchEvents() >= 0)
  {
    Integration::SetPinchGestureMinimumTouchEvents(mEnvironmentOptions->GetMinimumPinchTouchEvents());
  }
  if(mEnvironmentOptions->GetMinimumPinchTouchEventsAfterStart() >= 0)
  {
    Integration::SetPinchGestureMinimumTouchEventsAfterStart(mEnvironmentOptions->GetMinimumPinchTouchEventsAfterStart());
  }
  if(mEnvironmentOptions->GetMinimumRotationTouchEvents() >= 0)
  {
    Integration::SetRotationGestureMinimumTouchEvents(mEnvironmentOptions->GetMinimumRotationTouchEvents());
  }
  if(mEnvironmentOptions->GetMinimumRotationTouchEventsAfterStart() >= 0)
  {
    Integration::SetRotationGestureMinimumTouchEventsAfterStart(mEnvironmentOptions->GetMinimumRotationTouchEventsAfterStart());
  }
  if(mEnvironmentOptions->GetLongPressMinimumHoldingTime() >= 0)
  {
    Integration::SetLongPressMinimumHoldingTime(mEnvironmentOptions->GetLongPressMinimumHoldingTime());
  }
  if(mEnvironmentOptions->GetTapMaximumAllowedTime() > 0)
  {
    Integration::SetTapMaximumAllowedTime(mEnvironmentOptions->GetTapMaximumAllowedTime());
  }

  std::string systemCachePath = GetSystemCachePath();
  if(!systemCachePath.empty())
  {
    DALI_LOG_RELEASE_INFO("Check and create dali system cache directory: %s\n", systemCachePath.c_str());
    int dir_err = mkdir(systemCachePath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if(0 != dir_err && errno != EEXIST)
    {
      DALI_LOG_ERROR("Error creating system cache directory: %s!\n", systemCachePath.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }

    std::string shaderCachePath= GetProgramBinaryPath();
    dir_err = mkdir(shaderCachePath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if(0 != dir_err && errno != EEXIST)
    {
      DALI_LOG_ERROR("Error creating shader cache directory: %s!\n", shaderCachePath.c_str());
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }

    if(!shaderCachePath.empty())
    {
      std::string internalShaderCachePath = GetInternalProgramBinaryCachePath();
      DALI_LOG_RELEASE_INFO("Check and create dali internal shader cache directory: %s\n", internalShaderCachePath.c_str());
      dir_err = mkdir(internalShaderCachePath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      if(0 != dir_err && errno != EEXIST)
      {
        DALI_LOG_ERROR("Error creating dali internal shader directory: %s!\n", internalShaderCachePath.c_str());
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }

      std::string customShaderCachePath = GetCustomProgramBinaryCachePath();
      DALI_LOG_RELEASE_INFO("Check and create dali custom shader cache directory: %s\n", customShaderCachePath.c_str());
      dir_err = mkdir(customShaderCachePath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      if(0 != dir_err && errno != EEXIST)
      {
        DALI_LOG_ERROR("Error creating dali custom shader directory: %s!\n", customShaderCachePath.c_str());
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }
  }

  mConfigurationManager = Utils::MakeUnique<ConfigurationManager>(systemCachePath, mGraphics.get(), mThreadController);

  DALI_LOG_RELEASE_INFO("Adaptor::Initialize: Initialized\n");
}

Adaptor::~Adaptor()
{
  DALI_LOG_RELEASE_INFO("Adaptor::~Adaptor()\n");
  Accessibility::Bridge::GetCurrentBridge()->Terminate();

  // Ensure stop status
  Stop();

  // set to NULL first as we do not want any access to Adaptor as it is being destroyed.
  gThreadLocalAdaptor = NULL;

  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnDestroy();
  }

  // Clear out all the handles to Windows
  mWindows.clear();

  delete mThreadController; // this will shutdown render thread, which will call Core::ContextDestroyed before exit
  delete mObjectProfiler;

  delete mCore;

  delete mDisplayConnection;
  delete mPlatformAbstraction;

  mCallbackManager.reset();

  delete mPerformanceInterface;

  mGraphics->Destroy();

  // uninstall it on this thread (main actor thread)
  Dali::Integration::Log::UninstallLogFunction();

  // Delete environment options if we own it
  if(mEnvironmentOptionsOwned)
  {
    delete mEnvironmentOptions;
  }
}

void Adaptor::Start()
{
  // It doesn't support restart after stop at this moment to support restarting, need more testing
  if(READY != mState)
  {
    return;
  }

  mCore->Initialize();

  SetupSystemInformation();
  TextAbstraction::EnsureLocale();

  // Start the callback manager
  mCallbackManager->Start();

  // Initialize accessibility bridge after callback manager is started to use Idler callback
  auto appName = GetApplicationPackageName();
  auto bridge  = Accessibility::Bridge::GetCurrentBridge();
  bridge->SetApplicationName(appName);
  bridge->Initialize();

  Dali::Internal::Adaptor::SceneHolder* defaultWindow = mWindows.front();

  unsigned int dpiHor, dpiVer;
  dpiHor = dpiVer = 0;

  defaultWindow->GetSurface()->GetDpi(dpiHor, dpiVer);
  Dali::Internal::Adaptor::WindowSystem::SetDpi(dpiHor, dpiVer);

  // Initialize the thread controller
  mThreadController->Initialize();

  // Set max texture size
  if(mEnvironmentOptions->GetMaxTextureSize() > 0)
  {
    Dali::TizenPlatform::ImageLoader::SetMaxTextureSize(mEnvironmentOptions->GetMaxTextureSize());
  }
  else
  {
    unsigned int maxTextureSize = mConfigurationManager->GetMaxTextureSize();
    Dali::TizenPlatform::ImageLoader::SetMaxTextureSize(maxTextureSize);
  }

  // cache advanced blending and shader language version
  mGraphics->CacheConfigurations(*mConfigurationManager);

  ProcessCoreEvents(); // Ensure any startup messages are processed.

  // Initialize the image loader plugin
  auto enablePluginString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ENABLE_IMAGE_LOADER_PLUGIN);
  bool enablePlugin       = enablePluginString ? std::atoi(enablePluginString) : false;
  if(enablePlugin)
  {
    Internal::Adaptor::ImageLoaderPluginProxy::Initialize();
  }

  for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
  {
    (*iter)->OnStart();
  }

  if(mAddOnManager)
  {
    mAddOnManager->Start();
  }

  DALI_LOG_RELEASE_INFO("Adaptor::Start: Started\n");
}

// Dali::Internal::Adaptor::Adaptor::Pause
void Adaptor::Pause()
{
  // Only pause the adaptor if we're actually running.
  if(RUNNING == mState)
  {
    // Inform observers that we are about to be paused.
    for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
    {
      (*iter)->OnPause();
    }

    // Extensions
    if(mAddOnManager)
    {
      mAddOnManager->Pause();
    }

    // Pause all windows event handlers when adaptor paused
    for(auto window : mWindows)
    {
      window->Pause();
    }

    mThreadController->Pause();
    mState = PAUSED;

    // Ensure any messages queued during pause callbacks are processed by doing another update.
    RequestUpdateOnce();

    DALI_LOG_RELEASE_INFO("Adaptor::Pause: Paused\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Adaptor::Pause: Not paused [%d]\n", mState);
  }
}

// Dali::Internal::Adaptor::Adaptor::Resume
void Adaptor::Resume()
{
  // Only resume the adaptor if we are in the suspended state.
  if(PAUSED == mState)
  {
    mState = RUNNING;

    // Reset the event handlers when adaptor resumed
    for(auto window : mWindows)
    {
      window->Resume();
    }

    // Resume AddOnManager
    if(mAddOnManager)
    {
      mAddOnManager->Resume();
    }

    // Inform observers that we have resumed.
    for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
    {
      (*iter)->OnResume();
    }

    // Trigger processing of events queued up while paused
    mCore->ProcessEvents();

    // Do at end to ensure our first update/render after resumption includes the processed messages as well
    mThreadController->Resume();

    DALI_LOG_RELEASE_INFO("Adaptor::Resume: Resumed\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Adaptor::Resume: Not resumed [%d]\n", mState);
  }
}

void Adaptor::Stop()
{
  if(RUNNING == mState ||
     PAUSED == mState ||
     PAUSED_WHILE_HIDDEN == mState)
  {
    for(ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter)
    {
      (*iter)->OnStop();
    }

    if(mAddOnManager)
    {
      mAddOnManager->Stop();
    }

    mThreadController->Stop();

    // Delete the TTS player
    for(int i = 0; i < Dali::TtsPlayer::MODE_NUM; i++)
    {
      if(mTtsPlayers[i])
      {
        mTtsPlayers[i].Reset();
      }
    }

    // Destroy the image loader plugin
    auto enablePluginString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ENABLE_IMAGE_LOADER_PLUGIN);
    bool enablePlugin       = enablePluginString ? std::atoi(enablePluginString) : false;
    if(enablePlugin)
    {
      Internal::Adaptor::ImageLoaderPluginProxy::Destroy();
    }

    delete mNotificationTrigger;
    mNotificationTrigger = NULL;

    mCallbackManager->Stop();

    mCore->UnregisterProcessors();

    RemoveSystemInformation();

    // Note: Must change the state at end of function.
    mState = STOPPED;

    DALI_LOG_RELEASE_INFO("Adaptor::Stop\n");
  }
}

void Adaptor::ContextLost()
{
  mCore->GetContextNotifier()->NotifyContextLost(); // Inform stage
}

void Adaptor::ContextRegained()
{
  // Inform core, so that texture resources can be reloaded
  mCore->RecoverFromContextLoss();

  mCore->GetContextNotifier()->NotifyContextRegained(); // Inform stage
}

void Adaptor::FeedTouchPoint(TouchPoint& point, int timeStamp)
{
  Integration::Point convertedPoint(point);
  mWindows.front()->FeedTouchPoint(convertedPoint, timeStamp);
}

void Adaptor::FeedWheelEvent(Dali::WheelEvent& wheelEvent)
{
  Integration::WheelEvent event(static_cast<Integration::WheelEvent::Type>(wheelEvent.GetType()), wheelEvent.GetDirection(), wheelEvent.GetModifiers(), wheelEvent.GetPoint(), wheelEvent.GetDelta(), wheelEvent.GetTime());
  mWindows.front()->FeedWheelEvent(event);
}

void Adaptor::FeedKeyEvent(Dali::KeyEvent& keyEvent)
{
  Integration::KeyEvent convertedEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
  convertedEvent.receiveTime = keyEvent.GetReceiveTime();
  mWindows.front()->FeedKeyEvent(convertedEvent);
}

void Adaptor::ReplaceSurface(Dali::Integration::SceneHolder window, Dali::Integration::RenderSurfaceInterface& newSurface)
{
  Internal::Adaptor::SceneHolder* windowImpl = &Dali::GetImplementation(window);
  for(auto windowPtr : mWindows)
  {
    if(windowPtr == windowImpl) // the window is not deleted
    {
      mResizedSignal.Emit(mAdaptor);

      windowImpl->SetSurface(&newSurface);

      // Flush the event queue to give the update-render thread chance
      // to start processing messages for new camera setup etc as soon as possible
      ProcessCoreEvents();

      // This method blocks until the render thread has completed the replace.
      mThreadController->ReplaceSurface(&newSurface);
      break;
    }
  }
}

void Adaptor::DeleteSurface(Dali::Integration::RenderSurfaceInterface& surface)
{
  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  // This method blocks until the render thread has finished rendering the current surface.
  mThreadController->DeleteSurface(&surface);
}

Dali::Integration::RenderSurfaceInterface& Adaptor::GetSurface() const
{
  return *mWindows.front()->GetSurface();
}

void Adaptor::ReleaseSurfaceLock()
{
  mWindows.front()->GetSurface()->ReleaseLock();
}

Dali::TtsPlayer Adaptor::GetTtsPlayer(Dali::TtsPlayer::Mode mode)
{
  if(!mTtsPlayers[mode])
  {
    // Create the TTS player when it needed, because it can reduce launching time.
    mTtsPlayers[mode] = TtsPlayer::New(mode);
  }

  return mTtsPlayers[mode];
}

bool Adaptor::AddIdle(CallbackBase* callback, bool hasReturnValue)
{
  bool idleAdded(false);

  // We want to run the processes even when paused
  if(STOPPED != mState)
  {
    idleAdded = mCallbackManager->AddIdleCallback(callback, hasReturnValue);
  }

  if(!idleAdded)
  {
    // Delete callback
    delete callback;
  }

  return idleAdded;
}

void Adaptor::RemoveIdle(CallbackBase* callback)
{
  mCallbackManager->RemoveIdleCallback(callback);
}

void Adaptor::ProcessIdle()
{
  bool idleProcessed           = mCallbackManager->ProcessIdle();
  mNotificationOnIdleInstalled = mNotificationOnIdleInstalled && !idleProcessed;
}

void Adaptor::SetPreRenderCallback(CallbackBase* callback)
{
  mThreadController->SetPreRenderCallback(callback);
}

bool Adaptor::AddWindow(Dali::Integration::SceneHolder childWindow)
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation(childWindow);
  windowImpl.SetAdaptor(Get());

  // ChildWindow is set to the layout direction of the default window.
  windowImpl.GetRootLayer().SetProperty(Dali::Actor::Property::LAYOUT_DIRECTION, mRootLayoutDirection);

  // Add the new Window to the container - the order is not important
  {
    Dali::Mutex::ScopedLock lock(mMutex);
    mWindows.push_back(&windowImpl);
  }

  Dali::Integration::RenderSurfaceInterface* surface = windowImpl.GetSurface();

  mThreadController->AddSurface(surface);

  mWindowCreatedSignal.Emit(childWindow);

  return true;
}

bool Adaptor::RemoveWindow(Dali::Integration::SceneHolder* childWindow)
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation(*childWindow);
  for(WindowContainer::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter)
  {
    if(*iter == &windowImpl)
    {
      Dali::Mutex::ScopedLock lock(mMutex);
      mWindows.erase(iter);
      return true;
    }
  }

  return false;
}

bool Adaptor::RemoveWindow(std::string childWindowName)
{
  for(WindowContainer::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter)
  {
    if((*iter)->GetName() == childWindowName)
    {
      Dali::Mutex::ScopedLock lock(mMutex);
      mWindows.erase(iter);
      return true;
    }
  }

  return false;
}

bool Adaptor::RemoveWindow(Internal::Adaptor::SceneHolder* childWindow)
{
  for(WindowContainer::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter)
  {
    if((*iter)->GetId() == childWindow->GetId())
    {
      Dali::Mutex::ScopedLock lock(mMutex);
      mWindows.erase(iter);
      return true;
    }
  }

  return false;
}

Dali::Adaptor& Adaptor::Get()
{
  DALI_ASSERT_ALWAYS((gThreadLocalAdaptor != NULL) && "Adaptor not instantiated");
  return gThreadLocalAdaptor->mAdaptor;
}

bool Adaptor::IsAvailable()
{
  return gThreadLocalAdaptor != NULL && (gThreadLocalAdaptor->mState != Adaptor::State::STOPPED);
}

void Adaptor::SceneCreated()
{
  mCore->SceneCreated();
}

Dali::Integration::Core& Adaptor::GetCore()
{
  return *mCore;
}

void Adaptor::SetRenderRefreshRate(unsigned int numberOfVSyncsPerRender)
{
  mThreadController->SetRenderRefreshRate(numberOfVSyncsPerRender);
}

Dali::DisplayConnection& Adaptor::GetDisplayConnectionInterface()
{
  DALI_ASSERT_DEBUG(mDisplayConnection && "Display connection not created");
  return *mDisplayConnection;
}

Dali::Graphics::GraphicsInterface& Adaptor::GetGraphicsInterface()
{
  DALI_ASSERT_DEBUG(mGraphics && "Graphics interface not created");
  return *mGraphics;
}

Dali::Integration::PlatformAbstraction& Adaptor::GetPlatformAbstractionInterface()
{
  return *mPlatformAbstraction;
}

TriggerEventInterface& Adaptor::GetProcessCoreEventsTrigger()
{
  return *mNotificationTrigger;
}

SocketFactoryInterface& Adaptor::GetSocketFactoryInterface()
{
  return mSocketFactory;
}

Dali::Integration::RenderSurfaceInterface* Adaptor::GetRenderSurfaceInterface()
{
  if(!mWindows.empty())
  {
    return mWindows.front()->GetSurface();
  }

  return nullptr;
}

TraceInterface& Adaptor::GetKernelTraceInterface()
{
  return mKernelTracer;
}

TraceInterface& Adaptor::GetSystemTraceInterface()
{
  return mSystemTracer;
}

PerformanceInterface* Adaptor::GetPerformanceInterface()
{
  return mPerformanceInterface;
}

Integration::PlatformAbstraction& Adaptor::GetPlatformAbstraction() const
{
  DALI_ASSERT_DEBUG(mPlatformAbstraction && "PlatformAbstraction not created");
  return *mPlatformAbstraction;
}

void Adaptor::GetWindowContainerInterface(WindowContainer& windows)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  windows = mWindows;
}

Devel::TextureUploadManager& Adaptor::GetTextureUploadManager()
{
  return mTextureUploadManager;
}

void Adaptor::DestroyTtsPlayer(Dali::TtsPlayer::Mode mode)
{
  if(mTtsPlayers[mode])
  {
    mTtsPlayers[mode].Reset();
  }
}

Any Adaptor::GetNativeWindowHandle()
{
  return mWindows.front()->GetNativeHandle();
}

Any Adaptor::GetNativeWindowHandle(Dali::Actor actor)
{
  Any nativeWindowHandle;

  Dali::Integration::Scene scene = Dali::Integration::Scene::Get(actor);

  for(auto sceneHolder : mWindows)
  {
    if(scene == sceneHolder->GetScene())
    {
      nativeWindowHandle = sceneHolder->GetNativeHandle();
      break;
    }
  }

  return nativeWindowHandle;
}

Any Adaptor::GetGraphicsDisplay()
{
  Any display;

  if(mGraphics)
  {
    display = mGraphics->GetDisplay();
  }
  return display;
}

void Adaptor::SetUseRemoteSurface(bool useRemoteSurface)
{
  mUseRemoteSurface = useRemoteSurface;
}

void Adaptor::AddObserver(LifeCycleObserver& observer)
{
  ObserverContainer::iterator match(find(mObservers.begin(), mObservers.end(), &observer));

  if(match == mObservers.end())
  {
    mObservers.push_back(&observer);
  }
}

void Adaptor::RemoveObserver(LifeCycleObserver& observer)
{
  ObserverContainer::iterator match(find(mObservers.begin(), mObservers.end(), &observer));

  if(match != mObservers.end())
  {
    mObservers.erase(match);
  }
}

void Adaptor::QueueCoreEvent(const Dali::Integration::Event& event)
{
  if(mCore)
  {
    mCore->QueueEvent(event);
  }
}

void Adaptor::FlushUpdateMessages()
{
  if(mCore)
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_FLUSH_UPDATE_MESSAGES");

    mCore->ForceRelayout();
  }
}

void Adaptor::ProcessCoreEvents()
{
  if(mCore)
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_PROCESS_CORE_EVENTS");

    if(mPerformanceInterface)
    {
      mPerformanceInterface->AddMarker(PerformanceInterface::PROCESS_EVENTS_START);
    }

    mCore->ProcessEvents();

    if(mPerformanceInterface)
    {
      mPerformanceInterface->AddMarker(PerformanceInterface::PROCESS_EVENTS_END);
    }
  }
}

void Adaptor::RequestUpdate()
{
  switch(mState)
  {
    case RUNNING:
    {
      mThreadController->RequestUpdate();
      break;
    }
    case PAUSED:
    case PAUSED_WHILE_HIDDEN:
    {
      // Update (and resource upload) without rendering
      mThreadController->RequestUpdateOnce(UpdateMode::SKIP_RENDER);
      break;
    }
    default:
    {
      // Do nothing
      break;
    }
  }
}

void Adaptor::RequestProcessEventsOnIdle()
{
  // We want to run the processes even when paused
  if(STOPPED != mState)
  {
    if(!mNotificationOnIdleInstalled)
    {
      // If we haven't installed the idle notification, install it idle enterer.
      mNotificationOnIdleInstalled = AddIdleEnterer(MakeCallback(this, &Adaptor::ProcessCoreEventsFromIdle));
    }
    else
    {
      // Request comes during ProcessCoreEventsFromIdle running.
      // Mark as we need to call ProcessEvents in next idle events.
      mRequiredIdleRepeat = true;
    }
  }
}

void Adaptor::OnWindowShown()
{
  if(PAUSED_WHILE_HIDDEN == mState)
  {
    // Adaptor can now be resumed
    mState = PAUSED;

    Resume();

    // Force a render task
    RequestUpdateOnce();
  }
  else if(RUNNING == mState)
  {
    // Force a render task
    RequestUpdateOnce();

    DALI_LOG_RELEASE_INFO("Adaptor::OnWindowShown: Update requested.\n");
  }
  else if(PAUSED_WHILE_INITIALIZING == mState)
  {
    // Change the state to READY again. It will be changed to RUNNING after the adaptor is started.
    mState = READY;
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Adaptor::OnWindowShown: Adaptor is not paused state.[%d]\n", mState);
  }
}

void Adaptor::OnWindowHidden()
{
  if(RUNNING == mState || READY == mState)
  {
    bool allWindowsHidden = true;

    for(auto window : mWindows)
    {
      if(window->IsVisible())
      {
        allWindowsHidden = false;
        break;
      }
    }

    // Only pause the adaptor when all the windows are hidden
    if(allWindowsHidden)
    {
      if(mState == RUNNING)
      {
        Pause();

        // Adaptor cannot be resumed until any window is shown
        mState = PAUSED_WHILE_HIDDEN;
      }
      else // mState is READY
      {
        // Pause the adaptor after the state gets RUNNING
        mState = PAUSED_WHILE_INITIALIZING;
      }
    }
    else
    {
      DALI_LOG_RELEASE_INFO("Adaptor::OnWindowHidden: Some windows are shown. Don't pause adaptor.\n");
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Adaptor::OnWindowHidden: Adaptor is not running state.[%d]\n", mState);
  }
}

// Dali::Internal::Adaptor::Adaptor::OnDamaged
void Adaptor::OnDamaged(const DamageArea& area)
{
  // This is needed for the case where Dali window is partially obscured
  RequestUpdate();
}

void Adaptor::SurfaceResizePrepare(Dali::Integration::RenderSurfaceInterface* surface, SurfaceSize surfaceSize)
{
  mResizedSignal.Emit(mAdaptor);
  // @todo inform graphics device...
}

void Adaptor::SurfaceResizeComplete(Dali::Integration::RenderSurfaceInterface* surface, SurfaceSize surfaceSize)
{
  // Nofify surface resizing before flushing event queue
  mThreadController->ResizeSurface();

  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();
}

void Adaptor::IncreaseSurfaceResizeCounter()
{
  // Nofify surface resizing before flushing event queue
  if(mThreadController)
  {
    mThreadController->ResizeSurface();
  }
}

void Adaptor::NotifySceneCreated()
{
  GetCore().SceneCreated();

  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  // Start thread controller after the scene has been created
  mThreadController->Start();

  // Process after surface is created (registering to remote surface provider if required)
  SurfaceInitialized();

  if(mState != PAUSED_WHILE_INITIALIZING)
  {
    mState = RUNNING;

    DALI_LOG_RELEASE_INFO("Adaptor::NotifySceneCreated: Adaptor is running\n");
  }
  else
  {
    mState = RUNNING;

    Pause();

    mState = PAUSED_WHILE_HIDDEN;

    DALI_LOG_RELEASE_INFO("Adaptor::NotifySceneCreated: Adaptor is paused\n");
  }
}

void Adaptor::NotifyLanguageChanged()
{
  mLanguageChangedSignal.Emit(mAdaptor);
}

void Adaptor::RenderOnce()
{
  if(mThreadController)
  {
    UpdateMode updateMode;
    if(mThreadMode == ThreadMode::NORMAL)
    {
      updateMode = UpdateMode::NORMAL;
    }
    else
    {
      updateMode = UpdateMode::FORCE_RENDER;

      ProcessCoreEvents();
    }

    // Force rendering
    for(auto&& iter : mWindows)
    {
      iter->GetSurface()->SetFullSwapNextFrame();
    }

    mThreadController->RequestUpdateOnce(updateMode);
  }
}

const LogFactoryInterface& Adaptor::GetLogFactory()
{
  return *mEnvironmentOptions;
}

const TraceFactoryInterface& Adaptor::GetTraceFactory()
{
  return *mEnvironmentOptions;
}

void Adaptor::RegisterProcessor(Integration::Processor& processor, bool postProcessor)
{
  GetCore().RegisterProcessor(processor, postProcessor);
}

void Adaptor::UnregisterProcessor(Integration::Processor& processor, bool postProcessor)
{
  GetCore().UnregisterProcessor(processor, postProcessor);
}

void Adaptor::RegisterProcessorOnce(Integration::Processor& processor, bool postProcessor)
{
  GetCore().RegisterProcessorOnce(processor, postProcessor);
}

void Adaptor::UnregisterProcessorOnce(Integration::Processor& processor, bool postProcessor)
{
  GetCore().UnregisterProcessorOnce(processor, postProcessor);
}

bool Adaptor::IsMultipleWindowSupported() const
{
  return mConfigurationManager->IsMultipleWindowSupported();
}

int32_t Adaptor::GetRenderThreadId() const
{
  if(mThreadController)
  {
    return mThreadController->GetThreadId();
  }
  return 0;
}

void Adaptor::RequestUpdateOnce()
{
  if(mThreadController)
  {
    mThreadController->RequestUpdateOnce(UpdateMode::NORMAL);
  }
}

bool Adaptor::ProcessCoreEventsFromIdle()
{
  // Reset repeat idler flag.
  mRequiredIdleRepeat = false;
  ProcessCoreEvents();

  // If someone request ProcessCoreEvents during above ProcessCoreEvents call, we might need to run idle one more times.
  // Else, the idle handle automatically un-installs itself
  mNotificationOnIdleInstalled = mRequiredIdleRepeat;

  if(mRequiredIdleRepeat)
  {
    DALI_LOG_DEBUG_INFO("Required ProcessCoreEvents one more times\n");
  }

  return mRequiredIdleRepeat;
}

Dali::Internal::Adaptor::SceneHolder* Adaptor::GetWindow(Dali::Actor& actor)
{
  Dali::Integration::Scene scene = Dali::Integration::Scene::Get(actor);

  for(auto window : mWindows)
  {
    if(scene == window->GetScene())
    {
      return window;
    }
  }

  return nullptr;
}

Dali::WindowContainer Adaptor::GetWindows() const
{
  Dali::WindowContainer windows;

  for(auto iter = mWindows.begin(); iter != mWindows.end(); ++iter)
  {
    // Downcast to Dali::Window
    Dali::Window window(dynamic_cast<Dali::Internal::Adaptor::Window*>(*iter));
    if(window)
    {
      windows.push_back(window);
    }
  }

  return windows;
}

Dali::SceneHolderList Adaptor::GetSceneHolders() const
{
  Dali::SceneHolderList sceneHolderList;

  for(auto iter = mWindows.begin(); iter != mWindows.end(); ++iter)
  {
    sceneHolderList.push_back(Dali::Integration::SceneHolder(*iter));
  }

  return sceneHolderList;
}

Dali::ObjectRegistry Adaptor::GetObjectRegistry() const
{
  Dali::ObjectRegistry registry;
  if(mCore)
  {
    registry = mCore->GetObjectRegistry();
  }
  return registry;
}

Adaptor::Adaptor(Dali::Integration::SceneHolder window, Dali::Adaptor& adaptor, Dali::Integration::RenderSurfaceInterface* surface, EnvironmentOptions* environmentOptions, ThreadMode threadMode)
: mResizedSignal(),
  mLanguageChangedSignal(),
  mWindowCreatedSignal(),
  mAdaptor(adaptor),
  mState(READY),
  mCore(nullptr),
  mThreadController(nullptr),
  mGraphics(nullptr),
  mDisplayConnection(nullptr),
  mWindows(),
  mConfigurationManager(nullptr),
  mPlatformAbstraction(nullptr),
  mCallbackManager(nullptr),
  mNotificationOnIdleInstalled(false),
  mRequiredIdleRepeat(false),
  mNotificationTrigger(nullptr),
  mDaliFeedbackPlugin(),
  mFeedbackController(nullptr),
  mTtsPlayers(),
  mObservers(),
  mEnvironmentOptions(environmentOptions ? environmentOptions : new EnvironmentOptions /* Create the options if not provided */),
  mPerformanceInterface(nullptr),
  mKernelTracer(),
  mSystemTracer(),
  mTextureUploadManager(),
  mObjectProfiler(nullptr),
  mMemoryPoolTimerSlotDelegate(this),
  mSocketFactory(),
  mMutex(),
  mThreadMode(threadMode),
  mEnvironmentOptionsOwned(environmentOptions ? false : true /* If not provided then we own the object */),
  mUseRemoteSurface(false),
  mRootLayoutDirection(Dali::LayoutDirection::LEFT_TO_RIGHT)
{
  DALI_ASSERT_ALWAYS(!IsAvailable() && "Cannot create more than one Adaptor per thread");
  mWindows.insert(mWindows.begin(), &Dali::GetImplementation(window));

  gThreadLocalAdaptor = this;
}

void Adaptor::SetRootLayoutDirection(std::string locale)
{
  mRootLayoutDirection = static_cast<LayoutDirection::Type>(Internal::Adaptor::Locale::GetDirection(std::string(locale)));
  for(auto& window : mWindows)
  {
    Dali::Actor root = window->GetRootLayer();
    root.SetProperty(Dali::Actor::Property::LAYOUT_DIRECTION, mRootLayoutDirection);
  }
}

bool Adaptor::AddIdleEnterer(CallbackBase* callback)
{
  bool idleAdded(false);

  // We want to run the processes even when paused
  if(STOPPED != mState)
  {
    idleAdded = mCallbackManager->AddIdleEntererCallback(callback);
  }

  if(!idleAdded)
  {
    // Delete callback
    delete callback;
  }

  return idleAdded;
}

void Adaptor::RemoveIdleEnterer(CallbackBase* callback)
{
  mCallbackManager->RemoveIdleEntererCallback(callback);
}

bool Adaptor::MemoryPoolTimeout()
{
  mCore->LogMemoryPools();
  return true; // Keep logging forever
}

} // namespace Dali::Internal::Adaptor
