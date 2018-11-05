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
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/stage.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/object/any.h>
#include <dali/devel-api/actors/actor-devel.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/context-notifier.h>
#include <dali/integration-api/profiling.h>
#include <dali/integration-api/input-options.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/system/common/thread-controller.h>
#include <dali/internal/system/common/performance-interface-factory.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>

#include <dali/internal/graphics/gles20/egl-graphics-factory.h>
#include <dali/internal/graphics/gles20/egl-graphics.h> // Temporary until Core is abstracted

#include <dali/devel-api/text-abstraction/font-client.h>

#include <dali/internal/system/common/callback-manager.h>
#include <dali/internal/accessibility/common/tts-player-impl.h>
#include <dali/internal/accessibility/common/accessibility-adaptor-impl.h>
#include <dali/internal/input/common/gesture-manager.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/graphics/gles20/gl-proxy-implementation.h>
#include <dali/internal/graphics/gles20/gl-implementation.h>
#include <dali/internal/graphics/gles20/egl-sync-implementation.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/graphics/common/vsync-monitor.h>
#include <dali/internal/system/common/object-profiler.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#include <dali/internal/system/common/logging.h>

#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/imaging/common/image-loader-plugin-proxy.h>
#include <dali/internal/imaging/common/image-loader.h>


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

  Dali::Internal::Adaptor::AdaptorBuilder* mAdaptorBuilder = new AdaptorBuilder();
  auto graphicsFactory = mAdaptorBuilder->GetGraphicsFactory();

  impl->Initialize( graphicsFactory, configuration );
  delete mAdaptorBuilder; // Not needed anymore as the graphics interface has now been created

  return adaptor;
}

Dali::Adaptor* Adaptor::New( Dali::Window window, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Any winId = window.GetNativeHandle();

  Window& windowImpl = Dali::GetImplementation( window );
  Dali::Adaptor* adaptor = New( winId, windowImpl.GetSurface(), configuration, environmentOptions );
  windowImpl.SetAdaptor( *adaptor );
  return adaptor;
}

Dali::Adaptor* Adaptor::New( GraphicsFactory& graphicsFactory, Any nativeWindow, RenderSurface *surface, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Dali::Adaptor* adaptor = new Dali::Adaptor; // Public adaptor
  Adaptor* impl = new Adaptor( nativeWindow, *adaptor, surface, environmentOptions ); // Impl adaptor
  adaptor->mImpl = impl;

  impl->Initialize( graphicsFactory, configuration );

  return adaptor;
} // Called second

Dali::Adaptor* Adaptor::New( GraphicsFactory& graphicsFactory, Dali::Window window, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Any winId = window.GetNativeHandle();

  Window& windowImpl = Dali::GetImplementation( window );
  Dali::Adaptor* adaptor = New( graphicsFactory, winId, windowImpl.GetSurface(), configuration, environmentOptions );
  windowImpl.SetAdaptor( *adaptor );
  return adaptor;
} // Called first

void Adaptor::Initialize( GraphicsFactory& graphicsFactory, Dali::Configuration::ContextLoss configuration )
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

  // Note, Tizen does not use DALI_RETAINS_ALL_DATA, as it can reload images from files automatically.

  if( mEnvironmentOptions->PerformanceServerRequired() )
  {
    mPerformanceInterface = PerformanceInterfaceFactory::CreateInterface( *this, *mEnvironmentOptions );
  }

  mEnvironmentOptions->CreateTraceManager( mPerformanceInterface );
  mEnvironmentOptions->InstallTraceFunction(); // install tracing for main thread

  mCallbackManager = CallbackManager::New();

  WindowPane defaultWindow = mWindowFrame.front();
  PositionSize size = defaultWindow.surface->GetPositionSize();

  mGestureManager = new GestureManager(*this, Vector2(size.width, size.height), mCallbackManager, *mEnvironmentOptions);

  mGraphics = &( graphicsFactory.Create() );
  mGraphics->Initialize( mEnvironmentOptions );

  auto eglGraphics = static_cast<EglGraphics *>( mGraphics ); // This interface is temporary until Core has been updated to match

  GlImplementation& mGLES = eglGraphics->GetGlesInterface();
  EglSyncImplementation& eglSyncImpl = eglGraphics->GetSyncImplementation();

  mCore = Integration::Core::New( *this,
                                  *mPlatformAbstraction,
                                  mGLES,
                                  eglSyncImpl,
                                  *mGestureManager,
                                  dataRetentionPolicy ,
                                  ( 0u != mEnvironmentOptions->GetRenderToFboInterval() ) ? Integration::RenderToFrameBuffer::TRUE : Integration::RenderToFrameBuffer::FALSE,
                                  mGraphics->GetDepthBufferRequired(),
                                  mGraphics->GetStencilBufferRequired() );

  const unsigned int timeInterval = mEnvironmentOptions->GetObjectProfilerInterval();
  if( 0u < timeInterval )
  {
    mObjectProfiler = new ObjectProfiler( timeInterval );
  }

  mNotificationTrigger = mTriggerEventFactory.CreateTriggerEvent( MakeCallback( this, &Adaptor::ProcessCoreEvents ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);

  mVSyncMonitor = new VSyncMonitor;

  if( defaultWindow.surface )
  {
    mDisplayConnection = Dali::DisplayConnection::New( *mGraphics, defaultWindow.surface->GetSurfaceType() );
  }
  else
  {
    mDisplayConnection = Dali::DisplayConnection::New( *mGraphics );
  }

  mThreadController = new ThreadController( *this, *mEnvironmentOptions );

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
    Dali::TizenPlatform::ImageLoader::SetMaxTextureSize( mEnvironmentOptions->GetMaxTextureSize() );
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

  delete mGestureManager;
  delete mDisplayConnection;
  delete mPlatformAbstraction;
  delete mCallbackManager;
  delete mPerformanceInterface;

  mGraphics->Destroy();

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
  // It doesn't support restart after stop at this moment to support restarting, need more testing
  if( READY != mState )
  {
    return;
  }

  // Start the callback manager
  mCallbackManager->Start();

  WindowPane defaultWindow = mWindowFrame.front();

  // Create event handler
  mEventHandler = new EventHandler( defaultWindow.surface, *this, *mGestureManager, *this, mDragAndDropDetector );

  if( mDeferredRotationObserver != NULL )
  {
    mEventHandler->SetRotationObserver(mDeferredRotationObserver);
    mDeferredRotationObserver = NULL;
  }

  unsigned int dpiHor, dpiVer;
  dpiHor = dpiVer = 0;

  defaultWindow.surface->GetDpi( dpiHor, dpiVer );

  // tell core about the DPI value
  mCore->SetDpi(dpiHor, dpiVer);

  // set the DPI value for font rendering
  FontClient fontClient = FontClient::Get();
  fontClient.SetDpi( dpiHor, dpiVer );

  // Tell the core the size of the surface just before we start the render-thread
  PositionSize size = defaultWindow.surface->GetPositionSize();

  mCore->SurfaceResized( size.width, size.height );

  // Initialize the thread controller
  mThreadController->Initialize();

  ProcessCoreEvents(); // Ensure any startup messages are processed.

  // Initialize the image loader plugin
  Internal::Adaptor::ImageLoaderPluginProxy::Initialize();

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

    // Trigger processing of events queued up while paused
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

    // Clear out all the handles to Windows
    mWindowFrame.clear();

    // Delete the TTS player
    for(int i =0; i < Dali::TtsPlayer::MODE_NUM; i++)
    {
      if(mTtsPlayers[i])
      {
        mTtsPlayers[i].Reset();
      }
    }

    // Destroy the image loader plugin
    Internal::Adaptor::ImageLoaderPluginProxy::Destroy();

    delete mEventHandler;
    mEventHandler = NULL;

    delete mNotificationTrigger;
    mNotificationTrigger = NULL;

    mCallbackManager->Stop();

    mState = STOPPED;
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

void Adaptor::ReplaceSurface( Any nativeWindow, RenderSurface& newSurface )
{
  PositionSize positionSize = newSurface.GetPositionSize();

  // Let the core know the surface size has changed
  mCore->SurfaceResized( positionSize.width, positionSize.height );

  mResizedSignal.Emit( mAdaptor );

  WindowPane newDefaultWindow;
  newDefaultWindow.nativeWindow = nativeWindow;
  newDefaultWindow.surface = &newSurface;

  // Must delete the old Window first before replacing it with the new one
  WindowPane oldDefaultWindow = mWindowFrame.front();
  oldDefaultWindow.surface->DestroySurface();

  // Update WindowFrame
  std::vector<WindowPane>::iterator iter = mWindowFrame.begin();
  iter = mWindowFrame.insert( iter, newDefaultWindow );

  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  // This method blocks until the render thread has completed the replace.
  mThreadController->ReplaceSurface( newDefaultWindow.surface );
}

RenderSurface& Adaptor::GetSurface() const
{
  WindowPane defaultWindow = mWindowFrame.front();
  return *(defaultWindow.surface);
}

void Adaptor::ReleaseSurfaceLock()
{
  WindowPane defaultWindow = mWindowFrame.front();
  defaultWindow.surface->ReleaseLock();
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

bool Adaptor::AddWindow( Dali::Window* childWindow, const std::string& childWindowName, const std::string& childWindowClassName, const bool& childWindowMode )
{
  // This is any Window that is not the main (default) one
  WindowPane additionalWindow;
  additionalWindow.instance = childWindow;
  additionalWindow.window_name = childWindowName;
  additionalWindow.class_name = childWindowClassName;
  additionalWindow.window_mode = childWindowMode;

  // Add the new Window to the Frame - the order is not important
  mWindowFrame.push_back( additionalWindow );

  Window& windowImpl = Dali::GetImplementation( *childWindow );
  windowImpl.SetAdaptor( Get() );

  return true;
}

bool Adaptor::RemoveWindow( Dali::Window* childWindow )
{
  for ( WindowFrames::iterator iter = mWindowFrame.begin(); iter != mWindowFrame.end(); ++iter )
  {
    if( iter->instance == childWindow )
    {
      mWindowFrame.erase( iter );
      return true;
    }
  }

  return false;
}

bool Adaptor::RemoveWindow( std::string childWindowName )
{
  for ( WindowFrames::iterator iter = mWindowFrame.begin(); iter != mWindowFrame.end(); ++iter )
  {
    if( iter->window_name == childWindowName )
    {
      mWindowFrame.erase( iter );
      return true;
    }
  }

  return false;
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

Dali::DisplayConnection& Adaptor::GetDisplayConnectionInterface()
{
  DALI_ASSERT_DEBUG( mDisplayConnection && "Display connection not created" );
  return *mDisplayConnection;
}

GraphicsInterface& Adaptor::GetGraphicsInterface()
{
  DALI_ASSERT_DEBUG( mGraphics && "Graphics interface not created" );
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
  if( !mWindowFrame.empty())
  {
    WindowPane defaultWindow = mWindowFrame.front();
    return defaultWindow.surface;
  }
  else
  {
    return nullptr;
  }
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
  WindowPane defaultWindow = mWindowFrame.front();
  return defaultWindow.nativeWindow;
}

Any Adaptor::GetGraphicsDisplay()
{
  Any display;

  if (mGraphics)
  {
    auto eglGraphics = static_cast<EglGraphics *>( mGraphics ); // This interface is temporary until Core has been updated to match

    EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    display = eglImpl.GetDisplay();
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
  // Let the core know the surface size has changed
  mCore->SurfaceResized( surfaceSize.GetWidth(), surfaceSize.GetHeight() );

  mResizedSignal.Emit( mAdaptor );
}

void Adaptor::SurfaceResizeComplete( SurfaceSize surfaceSize )
{
  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  mThreadController->ResizeSurface();
}

void Adaptor::NotifySceneCreated()
{
  GetCore().SceneCreated();

  // Start thread controller after the scene has been created
  mThreadController->Start();

  // Process after surface is created (registering to remote surface provider if required)
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
  // Let the core know the indicator height is changed
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
  mCore( nullptr ),
  mThreadController( nullptr ),
  mVSyncMonitor( nullptr ),
  mDisplayConnection( nullptr ),
  mPlatformAbstraction( nullptr ),
  mEventHandler( nullptr ),
  mCallbackManager( nullptr ),
  mNotificationOnIdleInstalled( false ),
  mNotificationTrigger( nullptr ),
  mGestureManager( nullptr ),
  mDaliFeedbackPlugin(),
  mFeedbackController( nullptr ),
  mTtsPlayers(),
  mObservers(),
  mDragAndDropDetector(),
  mDeferredRotationObserver( nullptr ),
  mEnvironmentOptions( environmentOptions ? environmentOptions : new EnvironmentOptions /* Create the options if not provided */),
  mPerformanceInterface( nullptr ),
  mKernelTracer(),
  mSystemTracer(),
  mTriggerEventFactory(),
  mObjectProfiler( nullptr ),
  mSocketFactory(),
  mEnvironmentOptionsOwned( environmentOptions ? false : true /* If not provided then we own the object */ ),
  mUseRemoteSurface( false )
{
  DALI_ASSERT_ALWAYS( !IsAvailable() && "Cannot create more than one Adaptor per thread" );

  WindowPane defaultWindow;
  defaultWindow.nativeWindow = nativeWindow;
  defaultWindow.surface = surface;

  std::vector<WindowPane>::iterator iter = mWindowFrame.begin();
  iter = mWindowFrame.insert( iter, defaultWindow );

  gThreadLocalAdaptor = this;
}

// Stereoscopy

void Adaptor::SetViewMode( ViewMode viewMode )
{
  WindowPane defaultWindow = mWindowFrame.front();
  defaultWindow.surface->SetViewMode( viewMode );

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
