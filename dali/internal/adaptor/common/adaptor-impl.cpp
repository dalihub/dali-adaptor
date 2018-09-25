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
#include <dali/internal/adaptor/common/adaptor-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/stage.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/object/any.h>
#include <dali/devel-api/actors/actor-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/profiling.h>
#include <dali/integration-api/input-options.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/graphics/graphics.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/system/common/thread-controller.h>
#include <dali/internal/system/common/performance-interface-factory.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>

#include <dali/devel-api/text-abstraction/font-client.h>

#include <dali/internal/system/common/callback-manager.h>
#include <dali/internal/accessibility/common/tts-player-impl.h>
#include <dali/internal/accessibility/common/accessibility-adaptor-impl.h>
#include <dali/internal/input/common/gesture-manager.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles20/egl-factory.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/graphics/common/vsync-monitor.h>
#include <dali/internal/system/common/object-profiler.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#ifdef VULKAN_WITH_WAYLAND
#include <dali/internal/graphics/vulkan/wayland/vk-surface-wayland.h>
#else
#include <dali/internal/graphics/vulkan/x11/vk-surface-xlib2xcb.h>
#endif

#include <dali/internal/system/common/logging.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>

#include <dali/internal/system/common/locale-utils.h>

using Dali::TextAbstraction::FontClient;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
thread_local Adaptor* gThreadLocalAdaptor = NULL; // raw thread specific pointer to allow Adaptor::Get
} // unnamed namespace

Dali::Adaptor* Adaptor::New( Any nativeWindow, RenderSurface *surface, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Dali::Adaptor* adaptor = new Dali::Adaptor;
  Adaptor* impl = new Adaptor( nativeWindow, *adaptor, surface, environmentOptions );
  adaptor->mImpl = impl;

  impl->Initialize(configuration);

  return adaptor;
}

Dali::Adaptor* Adaptor::New( Dali::Window window, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Any winId = window.GetNativeHandle();

  Window& windowImpl = Dali::GetImplementation(window);
  Dali::Adaptor* adaptor = New( winId, windowImpl.GetSurface(), configuration, environmentOptions );
  windowImpl.SetAdaptor(*adaptor);
  return adaptor;
}

void Adaptor::Initialize( Dali::Configuration::ContextLoss configuration )
{
  // all threads here (event, update, and render) will send their logs to TIZEN Platform's LogMessage handler.
  Dali::Integration::Log::LogFunction logFunction( Dali::TizenPlatform::LogMessage );
  mEnvironmentOptions->SetLogFunction( logFunction );
  mEnvironmentOptions->InstallLogFunction(); // install logging for main thread

  mPlatformAbstraction = new TizenPlatform::TizenPlatformAbstraction;

  std::string path;
  GetDataStoragePath( path );
  mPlatformAbstraction->SetDataStoragePath( path );

  ResourcePolicy::DataRetention dataRetentionPolicy = ResourcePolicy::DALI_DISCARDS_ALL_DATA;
  if( configuration == Dali::Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS )
  {
    dataRetentionPolicy = ResourcePolicy::DALI_DISCARDS_ALL_DATA;
  }
  // Note, Tizen does not use DALI_RETAINS_ALL_DATA, as it can reload images from
  // files automatically.

  if( mEnvironmentOptions->PerformanceServerRequired() )
  {
    mPerformanceInterface = PerformanceInterfaceFactory::CreateInterface( *this, *mEnvironmentOptions );
  }

  mEnvironmentOptions->CreateTraceManager( mPerformanceInterface );
  mEnvironmentOptions->InstallTraceFunction(); // install tracing for main thread

  mCallbackManager = CallbackManager::New();

  PositionSize size = mSurface->GetPositionSize();

  mGestureManager = new GestureManager(*this, Vector2(size.width, size.height), mCallbackManager, *mEnvironmentOptions);

  const Integration::DepthBufferAvailable depthBufferAvailable = static_cast< Integration::DepthBufferAvailable >( mEnvironmentOptions->DepthBufferRequired() );
  const Integration::StencilBufferAvailable stencilBufferAvailable = static_cast< Integration::StencilBufferAvailable >( mEnvironmentOptions->StencilBufferRequired() );

  mEglFactory = new EglFactory( mEnvironmentOptions->GetMultiSamplingLevel(), depthBufferAvailable, stencilBufferAvailable );

  // @todo Refactor following into GraphicsFactory

  uint32_t depthStencilMask = mEnvironmentOptions->StencilBufferRequired() ? 1 : 0;
  depthStencilMask |= mEnvironmentOptions->DepthBufferRequired() ? 1 << 1 : 0;

  Integration::Graphics::GraphicsCreateInfo info;
  info.surfaceWidth = uint32_t( size.width );
  info.surfaceHeight = uint32_t( size.height );
  info.depthStencilMode = std::function<Integration::Graphics::DepthStencilMode()>(
    [depthStencilMask]() {
      switch( depthStencilMask )
      {
        case 1:
        case 3:
          return Integration::Graphics::DepthStencilMode::DEPTH_STENCIL_OPTIMAL;
        case 2:
          return Integration::Graphics::DepthStencilMode::DEPTH_OPTIMAL;
        case 0:
          return Integration::Graphics::DepthStencilMode::NONE;
        default:
          return Integration::Graphics::DepthStencilMode::NONE;
      }
    }
  )();

  info.swapchainBufferingMode = Integration::Graphics::SwapchainBufferingMode::OPTIMAL;

  mGraphics = std::unique_ptr<Dali::Integration::Graphics::Graphics>(
    new Dali::Integration::Graphics::Graphics( info )
  );

  mCore = Integration::Core::New( *this,
                                  *mPlatformAbstraction,
                                  *mGraphics,
                                  *mGestureManager,
                                  dataRetentionPolicy,
                                  ( 0u != mEnvironmentOptions->GetRenderToFboInterval() ) ? Integration::RenderToFrameBuffer::TRUE : Integration::RenderToFrameBuffer::FALSE,
                                  depthBufferAvailable,
                                  stencilBufferAvailable );

  const unsigned int timeInterval = mEnvironmentOptions->GetObjectProfilerInterval();
  if( 0u < timeInterval )
  {
    mObjectProfiler = new ObjectProfiler( timeInterval );
  }

  mNotificationTrigger = mTriggerEventFactory.CreateTriggerEvent( MakeCallback( this, &Adaptor::ProcessCoreEvents ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);

  mVSyncMonitor = new VSyncMonitor;

  mThreadController = new ThreadController( *this, *mGraphics, *mEnvironmentOptions );

  // Should be called after Core creation
  if( mEnvironmentOptions->GetPanGestureLoggingLevel() )
  {
    Integration::EnableProfiling( Dali::Integration::PROFILING_TYPE_PAN_GESTURE );
  }
  if( mEnvironmentOptions->GetPanGesturePredictionMode() >= 0 )
  {
    Integration::SetPanGesturePredictionMode(mEnvironmentOptions->GetPanGesturePredictionMode());
  }
  if( mEnvironmentOptions->GetPanGesturePredictionAmount() >= 0 )
  {
    Integration::SetPanGesturePredictionAmount(mEnvironmentOptions->GetPanGesturePredictionAmount());
  }
  if( mEnvironmentOptions->GetPanGestureMaximumPredictionAmount() >= 0 )
  {
    Integration::SetPanGestureMaximumPredictionAmount(mEnvironmentOptions->GetPanGestureMaximumPredictionAmount());
  }
  if( mEnvironmentOptions->GetPanGestureMinimumPredictionAmount() >= 0 )
  {
    Integration::SetPanGestureMinimumPredictionAmount(mEnvironmentOptions->GetPanGestureMinimumPredictionAmount());
  }
  if( mEnvironmentOptions->GetPanGesturePredictionAmountAdjustment() >= 0 )
  {
    Integration::SetPanGesturePredictionAmountAdjustment(mEnvironmentOptions->GetPanGesturePredictionAmountAdjustment());
  }
  if( mEnvironmentOptions->GetPanGestureSmoothingMode() >= 0 )
  {
    Integration::SetPanGestureSmoothingMode(mEnvironmentOptions->GetPanGestureSmoothingMode());
  }
  if( mEnvironmentOptions->GetPanGestureSmoothingAmount() >= 0.0f )
  {
    Integration::SetPanGestureSmoothingAmount(mEnvironmentOptions->GetPanGestureSmoothingAmount());
  }
  if( mEnvironmentOptions->GetPanGestureUseActualTimes() >= 0 )
  {
    Integration::SetPanGestureUseActualTimes( mEnvironmentOptions->GetPanGestureUseActualTimes() == 0 ? true : false );
  }
  if( mEnvironmentOptions->GetPanGestureInterpolationTimeRange() >= 0 )
  {
    Integration::SetPanGestureInterpolationTimeRange( mEnvironmentOptions->GetPanGestureInterpolationTimeRange() );
  }
  if( mEnvironmentOptions->GetPanGestureScalarOnlyPredictionEnabled() >= 0 )
  {
    Integration::SetPanGestureScalarOnlyPredictionEnabled( mEnvironmentOptions->GetPanGestureScalarOnlyPredictionEnabled() == 0 ? true : false  );
  }
  if( mEnvironmentOptions->GetPanGestureTwoPointPredictionEnabled() >= 0 )
  {
    Integration::SetPanGestureTwoPointPredictionEnabled( mEnvironmentOptions->GetPanGestureTwoPointPredictionEnabled() == 0 ? true : false  );
  }
  if( mEnvironmentOptions->GetPanGestureTwoPointInterpolatePastTime() >= 0 )
  {
    Integration::SetPanGestureTwoPointInterpolatePastTime( mEnvironmentOptions->GetPanGestureTwoPointInterpolatePastTime() );
  }
  if( mEnvironmentOptions->GetPanGestureTwoPointVelocityBias() >= 0.0f )
  {
    Integration::SetPanGestureTwoPointVelocityBias( mEnvironmentOptions->GetPanGestureTwoPointVelocityBias() );
  }
  if( mEnvironmentOptions->GetPanGestureTwoPointAccelerationBias() >= 0.0f )
  {
    Integration::SetPanGestureTwoPointAccelerationBias( mEnvironmentOptions->GetPanGestureTwoPointAccelerationBias() );
  }
  if( mEnvironmentOptions->GetPanGestureMultitapSmoothingRange() >= 0 )
  {
    Integration::SetPanGestureMultitapSmoothingRange( mEnvironmentOptions->GetPanGestureMultitapSmoothingRange() );
  }

  // Set max texture size
  if( mEnvironmentOptions->GetMaxTextureSize() > 0 )
  {
    Dali::SetMaxTextureSize( mEnvironmentOptions->GetMaxTextureSize() );
  }

  SetupSystemInformation();
}

Adaptor::~Adaptor()
{
  // Ensure stop status
  Stop();

  // set to NULL first as we do not want any access to Adaptor as it is being destroyed.
  gThreadLocalAdaptor = NULL;

  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnDestroy();
  }

  delete mThreadController; // this will shutdown render thread, which will call Core::ContextDestroyed before exit
  delete mVSyncMonitor;
  delete mEventHandler;
  delete mObjectProfiler;

  delete mCore;
  delete mEglFactory;
  delete mGestureManager;
  delete mPlatformAbstraction;
  delete mCallbackManager;
  delete mPerformanceInterface;

  // uninstall it on this thread (main actor thread)
  Dali::Integration::Log::UninstallLogFunction();

  // Delete environment options if we own it
  if( mEnvironmentOptionsOwned )
  {
    delete mEnvironmentOptions;
  }
}

void Adaptor::Start()
{
  // it doesn't support restart after stop at this moment
  // to support restarting, need more testing
  if( READY != mState )
  {
    return;
  }

  // Start the callback manager
  mCallbackManager->Start();

  // create event handler
  mEventHandler = new EventHandler( mSurface, *this, *mGestureManager, *this, mDragAndDropDetector );

  if( mDeferredRotationObserver != NULL )
  {
    mEventHandler->SetRotationObserver(mDeferredRotationObserver);
    mDeferredRotationObserver = NULL;
  }

  unsigned int dpiHor, dpiVer;
  dpiHor = dpiVer = 0;
  mSurface->GetDpi( dpiHor, dpiVer );

  // tell core about the DPI value
  mCore->SetDpi(dpiHor, dpiVer);

  // set the DPI value for font rendering
  FontClient fontClient = FontClient::Get();
  fontClient.SetDpi( dpiHor, dpiVer );

  // Tell the core the size of the surface just before we start the render-thread
  PositionSize size = mSurface->GetPositionSize();
  mCore->SurfaceResized( size.width, size.height );

  // Initialize the thread controller
  mThreadController->Initialize();

  ProcessCoreEvents(); // Ensure any startup messages are processed.

  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnStart();
  }
}

// Dali::Internal::Adaptor::Adaptor::Pause
void Adaptor::Pause()
{
  // Only pause the adaptor if we're actually running.
  if( RUNNING == mState )
  {
    // Inform observers that we are about to be paused.
    for( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
    {
      (*iter)->OnPause();
    }

    // Reset the event handler when adaptor paused
    if( mEventHandler )
    {
      mEventHandler->Pause();
    }

    mThreadController->Pause();
    mState = PAUSED;

    // Ensure any messages queued during pause callbacks are processed by doing another update.
    RequestUpdateOnce();
  }
}

// Dali::Internal::Adaptor::Adaptor::Resume
void Adaptor::Resume()
{
  // Only resume the adaptor if we are in the suspended state.
  if( PAUSED == mState )
  {
    mState = RUNNING;

    // Reset the event handler when adaptor resumed
    if( mEventHandler )
    {
      mEventHandler->Resume();
    }

    // Inform observers that we have resumed.
    for( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
    {
      (*iter)->OnResume();
    }

    // trigger processing of events queued up while paused
    mCore->ProcessEvents();

    // Do at end to ensure our first update/render after resumption includes the processed messages as well
    mThreadController->Resume();
  }
}

void Adaptor::Stop()
{
  if( RUNNING == mState ||
      PAUSED  == mState ||
      PAUSED_WHILE_HIDDEN == mState )
  {
    for( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
    {
      (*iter)->OnStop();
    }

    mThreadController->Stop();

    // Delete the TTS player
    for(int i =0; i < Dali::TtsPlayer::MODE_NUM; i++)
    {
      if(mTtsPlayers[i])
      {
        mTtsPlayers[i].Reset();
      }
    }

    delete mEventHandler;
    mEventHandler = NULL;

    delete mNotificationTrigger;
    mNotificationTrigger = NULL;

    mCallbackManager->Stop();

    mState = STOPPED;
  }
}

void Adaptor::FeedTouchPoint( TouchPoint& point, int timeStamp )
{
  mEventHandler->FeedTouchPoint( point, timeStamp );
}

void Adaptor::FeedWheelEvent( WheelEvent& wheelEvent )
{
  mEventHandler->FeedWheelEvent( wheelEvent );
}

void Adaptor::FeedKeyEvent( KeyEvent& keyEvent )
{
  mEventHandler->FeedKeyEvent( keyEvent );
}

void Adaptor::ReplaceSurface( Any nativeWindow, RenderSurface& surface )
{
  PositionSize positionSize = surface.GetPositionSize();

  // let the core know the surface size has changed
  mCore->SurfaceResized( positionSize.width, positionSize.height );

  mResizedSignal.Emit( mAdaptor );

  mNativeWindow = nativeWindow;
  mSurface = &surface;

  // flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  // this method blocks until the render thread has completed the replace.
  mThreadController->ReplaceSurface(mSurface);
}

RenderSurface& Adaptor::GetSurface() const
{
  return *mSurface;
}

void Adaptor::ReleaseSurfaceLock()
{
  mSurface->ReleaseLock();
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

bool Adaptor::AddIdle( CallbackBase* callback, bool hasReturnValue, bool forceAdd )
{
  bool idleAdded(false);

  // Only add an idle if the Adaptor is actually running
  if( RUNNING == mState || READY == mState || forceAdd )
  {
    idleAdded = mCallbackManager->AddIdleCallback( callback, hasReturnValue );
  }

  return idleAdded;
}

void Adaptor::RemoveIdle( CallbackBase* callback )
{
  mCallbackManager->RemoveIdleCallback( callback );
}

void Adaptor::SetPreRenderCallback( CallbackBase* callback )
{
  mThreadController->SetPreRenderCallback( callback );
}

Dali::Adaptor& Adaptor::Get()
{
  DALI_ASSERT_ALWAYS( IsAvailable() && "Adaptor not instantiated" );
  return gThreadLocalAdaptor->mAdaptor;
}

bool Adaptor::IsAvailable()
{
  return gThreadLocalAdaptor != NULL;
}

void Adaptor::SceneCreated()
{
  mCore->SceneCreated();
}

Dali::Integration::Core& Adaptor::GetCore()
{
  return *mCore;
}

void Adaptor::SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender )
{
  mThreadController->SetRenderRefreshRate( numberOfVSyncsPerRender );
}

void Adaptor::SetUseHardwareVSync( bool useHardware )
{
  mVSyncMonitor->SetUseHardwareVSync( useHardware );
}

EglFactory& Adaptor::GetEGLFactory() const
{
  DALI_ASSERT_DEBUG( mEglFactory && "EGL Factory not created" );
  return *mEglFactory;
}

EglFactoryInterface& Adaptor::GetEGLFactoryInterface() const
{
  return *mEglFactory;
}

Integration::Graphics::Graphics& Adaptor::GetGraphics() const
{
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

TriggerEventFactoryInterface& Adaptor::GetTriggerEventFactoryInterface()
{
  return mTriggerEventFactory;
}

SocketFactoryInterface& Adaptor::GetSocketFactoryInterface()
{
  return mSocketFactory;
}

RenderSurface* Adaptor::GetRenderSurfaceInterface()
{
  return mSurface;
}

VSyncMonitorInterface* Adaptor::GetVSyncMonitorInterface()
{
  return mVSyncMonitor;
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
  DALI_ASSERT_DEBUG( mPlatformAbstraction && "PlatformAbstraction not created" );
  return *mPlatformAbstraction;
}

void Adaptor::SetDragAndDropDetector( DragAndDropDetectorPtr detector )
{
  mDragAndDropDetector = detector;

  if ( mEventHandler )
  {
    mEventHandler->SetDragAndDropDetector( detector );
  }
}

void Adaptor::SetRotationObserver( RotationObserver* observer )
{
  if( mEventHandler )
  {
    mEventHandler->SetRotationObserver( observer );
  }
  else if( mState == READY )
  {
    // Set once event handler exists
    mDeferredRotationObserver = observer;
  }
}

void Adaptor::DestroyTtsPlayer(Dali::TtsPlayer::Mode mode)
{
  if(mTtsPlayers[mode])
  {
    mTtsPlayers[mode].Reset();
  }
}

void Adaptor::SetMinimumPinchDistance(float distance)
{
  if( mGestureManager )
  {
    mGestureManager->SetMinimumPinchDistance(distance);
  }
}

Any Adaptor::GetNativeWindowHandle()
{
  return mNativeWindow;
}

Any Adaptor::GetGraphicsDisplay()
{
  Any display;

  if( mEglFactory )
  {
    EglInterface* egl = mEglFactory->GetImplementation();
    if( egl )
    {
      auto eglImpl = static_cast<EglImplementation*>(egl);
      display = eglImpl->GetDisplay();
    }
  }
  return display;
}

void Adaptor::SetUseRemoteSurface(bool useRemoteSurface)
{
  mUseRemoteSurface = useRemoteSurface;
}

void Adaptor::AddObserver( LifeCycleObserver& observer )
{
  ObserverContainer::iterator match ( find(mObservers.begin(), mObservers.end(), &observer) );

  if ( match == mObservers.end() )
  {
    mObservers.push_back( &observer );
  }
}

void Adaptor::RemoveObserver( LifeCycleObserver& observer )
{
  ObserverContainer::iterator match ( find(mObservers.begin(), mObservers.end(), &observer) );

  if ( match != mObservers.end() )
  {
    mObservers.erase( match );
  }
}

void Adaptor::QueueCoreEvent(const Dali::Integration::Event& event)
{
  if( mCore )
  {
    mCore->QueueEvent(event);
  }
}

void Adaptor::ProcessCoreEvents()
{
  if( mCore )
  {
    if( mPerformanceInterface )
    {
      mPerformanceInterface->AddMarker( PerformanceInterface::PROCESS_EVENTS_START );
    }

    mCore->ProcessEvents();

    if( mPerformanceInterface )
    {
      mPerformanceInterface->AddMarker( PerformanceInterface::PROCESS_EVENTS_END );
    }
  }
}

void Adaptor::RequestUpdate( bool forceUpdate )
{
  switch( mState )
  {
    case RUNNING:
    {
      mThreadController->RequestUpdate();
      break;
    }
    case PAUSED:
    case PAUSED_WHILE_HIDDEN:
    {
      // When Dali applications are partially visible behind the lock-screen,
      // the indicator must be updated (therefore allow updates in the PAUSED state)
      if( forceUpdate )
      {
        mThreadController->RequestUpdateOnce();
      }
      break;
    }
    default:
    {
      // Do nothing
      break;
    }
  }
}

void Adaptor::RequestProcessEventsOnIdle( bool forceProcess )
{
  // Only request a notification if the Adaptor is actually running
  // and we haven't installed the idle notification
  if( ( ! mNotificationOnIdleInstalled ) && ( RUNNING == mState || READY == mState || forceProcess ) )
  {
    mNotificationOnIdleInstalled = AddIdleEnterer( MakeCallback( this, &Adaptor::ProcessCoreEventsFromIdle ), forceProcess );
  }
}

void Adaptor::OnWindowShown()
{
  if ( PAUSED_WHILE_HIDDEN == mState )
  {
    // Adaptor can now be resumed
    mState = PAUSED;

    Resume();

    // Force a render task
    RequestUpdateOnce();
  }
}

void Adaptor::OnWindowHidden()
{
  if ( RUNNING == mState )
  {
    Pause();

    // Adaptor cannot be resumed until the window is shown
    mState = PAUSED_WHILE_HIDDEN;
  }
}

// Dali::Internal::Adaptor::Adaptor::OnDamaged
void Adaptor::OnDamaged( const DamageArea& area )
{
  // This is needed for the case where Dali window is partially obscured
  RequestUpdate( false );
}

void Adaptor::SurfaceResizePrepare( SurfaceSize surfaceSize )
{
  // let the core know the surface size has changed
  mCore->SurfaceResized( surfaceSize.GetWidth(), surfaceSize.GetHeight() );

  mResizedSignal.Emit( mAdaptor );
}

void Adaptor::SurfaceResizeComplete( SurfaceSize surfaceSize )
{
  // flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  mThreadController->ResizeSurface();
}

void Adaptor::NotifySceneCreated()
{
  GetCore().SceneCreated();

  // Start thread controller after the scene has been created
  mThreadController->Start();

  // process after surface is created (registering to remote surface provider if required)
  SurfaceInitialized();

  mState = RUNNING;
}

void Adaptor::NotifyLanguageChanged()
{
  mLanguageChangedSignal.Emit( mAdaptor );
}

void Adaptor::RenderOnce()
{
  RequestUpdateOnce();
}

const LogFactoryInterface& Adaptor::GetLogFactory()
{
  return *mEnvironmentOptions;
}

void Adaptor::RequestUpdateOnce()
{
  if( mThreadController )
  {
    mThreadController->RequestUpdateOnce();
  }
}

void Adaptor::IndicatorSizeChanged(int height)
{
  // let the core know the indicator height is changed
  mCore->SetTopMargin(height);
}

bool Adaptor::ProcessCoreEventsFromIdle()
{
  ProcessCoreEvents();

  // the idle handle automatically un-installs itself
  mNotificationOnIdleInstalled = false;

  return false;
}

Adaptor::Adaptor(Any nativeWindow, Dali::Adaptor& adaptor, RenderSurface* surface, EnvironmentOptions* environmentOptions)
: mResizedSignal(),
  mLanguageChangedSignal(),
  mAdaptor( adaptor ),
  mState( READY ),
  mCore( NULL ),
  mThreadController( NULL ),
  mVSyncMonitor( NULL ),
  mEglFactory( NULL ),
  mNativeWindow( nativeWindow ),
  mSurface( surface ),
  mPlatformAbstraction( NULL ),
  mEventHandler( NULL ),
  mCallbackManager( NULL ),
  mNotificationOnIdleInstalled( false ),
  mNotificationTrigger( NULL ),
  mGestureManager( NULL ),
  mDaliFeedbackPlugin(),
  mFeedbackController( NULL ),
  mTtsPlayers(),
  mObservers(),
  mDragAndDropDetector(),
  mDeferredRotationObserver( NULL ),
  mEnvironmentOptions( environmentOptions ? environmentOptions : new EnvironmentOptions /* Create the options if not provided */),
  mPerformanceInterface( NULL ),
  mKernelTracer(),
  mSystemTracer(),
  mTriggerEventFactory(),
  mObjectProfiler( NULL ),
  mSocketFactory(),
  mEnvironmentOptionsOwned( environmentOptions ? false : true /* If not provided then we own the object */ ),
  mUseRemoteSurface( false )
{
  DALI_ASSERT_ALWAYS( !IsAvailable() && "Cannot create more than one Adaptor per thread" );
  gThreadLocalAdaptor = this;
}

// Stereoscopy

void Adaptor::SetViewMode( ViewMode viewMode )
{
  mSurface->SetViewMode( viewMode );
  mCore->SetViewMode( viewMode );
}

ViewMode Adaptor::GetViewMode() const
{
  return mCore->GetViewMode();
}

void Adaptor::SetStereoBase( float stereoBase )
{
  mCore->SetStereoBase( stereoBase );
}

float Adaptor::GetStereoBase() const
{
  return mCore->GetStereoBase();
}

void Adaptor::SetRootLayoutDirection( std::string locale )
{
  Dali::Stage stage = Dali::Stage::GetCurrent();

  stage.GetRootLayer().SetProperty( Dali::Actor::Property::LAYOUT_DIRECTION,
                                    static_cast< LayoutDirection::Type >( Internal::Adaptor::Locale::GetDirection( std::string( locale ) ) ) );
}

bool Adaptor::AddIdleEnterer( CallbackBase* callback, bool forceAdd )
{
  bool idleAdded( false );

  // Only add an idle if the Adaptor is actually running
  if( RUNNING == mState || READY == mState || forceAdd )
  {
    idleAdded = mCallbackManager->AddIdleEntererCallback( callback );
  }

  return idleAdded;
}

void Adaptor::RemoveIdleEnterer( CallbackBase* callback )
{
  mCallbackManager->RemoveIdleEntererCallback( callback );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
