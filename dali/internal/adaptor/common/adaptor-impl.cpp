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
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <dali/integration-api/processor-interface.h>

#include <fstream>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/system/common/thread-controller.h>
#include <dali/internal/system/common/performance-interface-factory.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>

#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#include <dali/internal/graphics/gles/egl-graphics.h> // Temporary until Core is abstracted

#include <dali/devel-api/text-abstraction/font-client.h>

#include <dali/internal/system/common/callback-manager.h>
#include <dali/internal/accessibility/common/tts-player-impl.h>
#include <dali/internal/accessibility/common/accessibility-adaptor-impl.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/graphics/gles/gl-proxy-implementation.h>
#include <dali/internal/graphics/gles/gl-implementation.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/system/common/object-profiler.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#include <dali/internal/system/common/logging.h>

#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/imaging/common/image-loader-plugin-proxy.h>
#include <dali/internal/imaging/common/image-loader.h>

#include <dali/devel-api/adaptor-framework/file-stream.h>

using Dali::TextAbstraction::FontClient;

extern std::string GetSystemCachePath();

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

Dali::Adaptor* Adaptor::New( Dali::Integration::SceneHolder window, Dali::RenderSurfaceInterface *surface, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Dali::Adaptor* adaptor = new Dali::Adaptor;
  Adaptor* impl = new Adaptor( window, *adaptor, surface, environmentOptions );
  adaptor->mImpl = impl;

  Dali::Internal::Adaptor::AdaptorBuilder* mAdaptorBuilder = new AdaptorBuilder();
  auto graphicsFactory = mAdaptorBuilder->GetGraphicsFactory();

  impl->Initialize( graphicsFactory, configuration );
  delete mAdaptorBuilder; // Not needed anymore as the graphics interface has now been created

  return adaptor;
}

Dali::Adaptor* Adaptor::New( Dali::Integration::SceneHolder window, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation( window );
  Dali::Adaptor* adaptor = New( window, windowImpl.GetSurface(), configuration, environmentOptions );
  windowImpl.SetAdaptor( *adaptor );
  return adaptor;
}

Dali::Adaptor* Adaptor::New( GraphicsFactory& graphicsFactory, Dali::Integration::SceneHolder window, Dali::RenderSurfaceInterface *surface, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Dali::Adaptor* adaptor = new Dali::Adaptor; // Public adaptor
  Adaptor* impl = new Adaptor( window, *adaptor, surface, environmentOptions ); // Impl adaptor
  adaptor->mImpl = impl;

  impl->Initialize( graphicsFactory, configuration );

  return adaptor;
} // Called second

Dali::Adaptor* Adaptor::New( GraphicsFactory& graphicsFactory, Dali::Integration::SceneHolder window, Dali::Configuration::ContextLoss configuration, EnvironmentOptions* environmentOptions )
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation( window );
  Dali::Adaptor* adaptor = New( graphicsFactory, window, windowImpl.GetSurface(), configuration, environmentOptions );
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

  Dali::Internal::Adaptor::SceneHolder* defaultWindow = mWindows.front();

  DALI_ASSERT_DEBUG( defaultWindow->GetSurface() && "Surface not initialized" );

  mGraphics = &( graphicsFactory.Create() );
  mGraphics->Initialize( mEnvironmentOptions );

  auto eglGraphics = static_cast<EglGraphics *>( mGraphics ); // This interface is temporary until Core has been updated to match

  // This will only be created once
  eglGraphics->Create();

  GlImplementation& mGLES = eglGraphics->GetGlesInterface();
  EglSyncImplementation& eglSyncImpl = eglGraphics->GetSyncImplementation();
  EglContextHelperImplementation& eglContextHelperImpl = eglGraphics->GetContextHelperImplementation();

  mCore = Integration::Core::New( *this,
                                  *mPlatformAbstraction,
                                  mGLES,
                                  eglSyncImpl,
                                  eglContextHelperImpl,
                                  dataRetentionPolicy ,
                                  ( 0u != mEnvironmentOptions->GetRenderToFboInterval() ) ? Integration::RenderToFrameBuffer::TRUE : Integration::RenderToFrameBuffer::FALSE,
                                  mGraphics->GetDepthBufferRequired(),
                                  mGraphics->GetStencilBufferRequired() );

  defaultWindow->SetAdaptor( Get() );

  Dali::Integration::SceneHolder defaultSceneHolder( defaultWindow );

  mWindowCreatedSignal.Emit( defaultSceneHolder );

  const unsigned int timeInterval = mEnvironmentOptions->GetObjectProfilerInterval();
  if( 0u < timeInterval )
  {
    mObjectProfiler = new ObjectProfiler( timeInterval );
  }

  mNotificationTrigger = mTriggerEventFactory.CreateTriggerEvent( MakeCallback( this, &Adaptor::ProcessCoreEvents ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);

  mDisplayConnection = Dali::DisplayConnection::New( *mGraphics, defaultWindow->GetSurface()->GetSurfaceType() );

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
  if( mEnvironmentOptions->GetMinimumPanDistance() >= 0 )
  {
    Integration::SetPanGestureMinimumDistance( mEnvironmentOptions->GetMinimumPanDistance() );
  }
  if( mEnvironmentOptions->GetMinimumPanEvents() >= 0 )
  {
    Integration::SetPanGestureMinimumPanEvents( mEnvironmentOptions->GetMinimumPanEvents() );
  }
  if( mEnvironmentOptions->GetMinimumPinchDistance() >= 0 )
  {
    Integration::SetPinchGestureMinimumDistance( mEnvironmentOptions->GetMinimumPinchDistance() );
  }

  // Set max texture size
  if( mEnvironmentOptions->GetMaxTextureSize() > 0 )
  {
    Dali::TizenPlatform::ImageLoader::SetMaxTextureSize( mEnvironmentOptions->GetMaxTextureSize() );
  }

  std::string systemCachePath = GetSystemCachePath();
  if ( systemCachePath.c_str() != NULL )
  {
    Dali::FileStream fileStream( systemCachePath + "gpu-environment.conf", Dali::FileStream::READ | Dali::FileStream::TEXT );
    std::fstream& stream = dynamic_cast<std::fstream&>( fileStream.GetStream() );
    if( stream.is_open() )
    {
      std::string line;
      while( std::getline( stream, line ) )
      {
        line.erase( line.find_last_not_of( " \t\r\n" ) + 1 );
        line.erase( 0, line.find_first_not_of( " \t\r\n" ) );
        if( '#' == *( line.cbegin() ) || line == "" )
        {
          continue;
        }

        std::istringstream stream( line );
        std::string environmentVariableName, environmentVariableValue;
        std::getline(stream, environmentVariableName, ' ');
        if( environmentVariableName == "DALI_ENV_MAX_TEXTURE_SIZE" && mEnvironmentOptions->GetMaxTextureSize() == 0 )
        {
          std::getline(stream, environmentVariableValue);
          setenv( environmentVariableName.c_str() , environmentVariableValue.c_str(), 1 );
          Dali::TizenPlatform::ImageLoader::SetMaxTextureSize( std::atoi( environmentVariableValue.c_str() ) );
        }
      }
    }
    else
    {
      DALI_LOG_ERROR( "Fail to open file : %s\n", ( systemCachePath + "gpu-environment.conf" ).c_str() );
    }
  }
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

  // Clear out all the handles to Windows
  mWindows.clear();

  delete mThreadController; // this will shutdown render thread, which will call Core::ContextDestroyed before exit
  delete mObjectProfiler;

  delete mCore;

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

  mCore->Initialize();

  SetupSystemInformation();

  // Start the callback manager
  mCallbackManager->Start();

  Dali::Internal::Adaptor::SceneHolder* defaultWindow = mWindows.front();

  unsigned int dpiHor, dpiVer;
  dpiHor = dpiVer = 0;

  defaultWindow->GetSurface()->GetDpi( dpiHor, dpiVer );

  // set the DPI value for font rendering
  FontClient fontClient = FontClient::Get();
  fontClient.SetDpi( dpiHor, dpiVer );

  // Initialize the thread controller
  mThreadController->Initialize();

  if( !Dali::TizenPlatform::ImageLoader::MaxTextureSizeUpdated() )
  {
    auto eglGraphics = static_cast<EglGraphics *>( mGraphics );
    GlImplementation& mGLES = eglGraphics->GetGlesInterface();
    Dali::TizenPlatform::ImageLoader::SetMaxTextureSize( mGLES.GetMaxTextureSize() );

    std::string systemCachePath = GetSystemCachePath();
    if( systemCachePath.c_str() != NULL )
    {
      const int dir_err = system( std::string( "mkdir " + systemCachePath ).c_str() );
      if (-1 == dir_err)
      {
          printf("Error creating directory!n");
          exit(1);
      }

      Dali::FileStream fileStream( systemCachePath + "gpu-environment.conf", Dali::FileStream::WRITE | Dali::FileStream::TEXT );
      std::fstream& configFile = dynamic_cast<std::fstream&>( fileStream.GetStream() );
      if( configFile.is_open() )
      {
        configFile << "DALI_ENV_MAX_TEXTURE_SIZE " << mGLES.GetMaxTextureSize() << std::endl;
      }
    }
  }

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

    // Pause all windows event handlers when adaptor paused
    for( auto window : mWindows )
    {
      window->Pause();
    }

    mThreadController->Pause();
    mState = PAUSED;

    // Ensure any messages queued during pause callbacks are processed by doing another update.
    RequestUpdateOnce();

    DALI_LOG_RELEASE_INFO( "Adaptor::Pause: Paused\n" );
  }
  else
  {
    DALI_LOG_RELEASE_INFO( "Adaptor::Pause: Not paused [%d]\n", mState );
  }
}

// Dali::Internal::Adaptor::Adaptor::Resume
void Adaptor::Resume()
{
  // Only resume the adaptor if we are in the suspended state.
  if( PAUSED == mState )
  {
    mState = RUNNING;

    // Reset the event handlers when adaptor resumed
    for( auto window : mWindows )
    {
      window->Resume();
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

    DALI_LOG_RELEASE_INFO( "Adaptor::Resume: Resumed\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO( "Adaptor::Resume: Not resumed [%d]\n", mState );
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
    for( int i =0; i < Dali::TtsPlayer::MODE_NUM; i++ )
    {
      if( mTtsPlayers[i] )
      {
        mTtsPlayers[i].Reset();
      }
    }

    // Destroy the image loader plugin
    Internal::Adaptor::ImageLoaderPluginProxy::Destroy();

    delete mNotificationTrigger;
    mNotificationTrigger = NULL;

    mCallbackManager->Stop();

    mState = STOPPED;

    DALI_LOG_RELEASE_INFO( "Adaptor::Stop\n" );
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
  Integration::Point convertedPoint( point );
  mWindows.front()->FeedTouchPoint( convertedPoint, timeStamp );
}

void Adaptor::FeedWheelEvent( WheelEvent& wheelEvent )
{
  Integration::WheelEvent event( static_cast< Integration::WheelEvent::Type >(wheelEvent.type), wheelEvent.direction, wheelEvent.modifiers, wheelEvent.point, wheelEvent.z, wheelEvent.timeStamp );
  mWindows.front()->FeedWheelEvent( event );
}

void Adaptor::FeedKeyEvent( KeyEvent& keyEvent )
{
  Integration::KeyEvent convertedEvent( keyEvent );
  mWindows.front()->FeedKeyEvent( convertedEvent );
}

void Adaptor::ReplaceSurface( Dali::Integration::SceneHolder window, Dali::RenderSurfaceInterface& newSurface )
{
  Internal::Adaptor::SceneHolder* windowImpl = &Dali::GetImplementation( window );
  for( auto windowPtr : mWindows )
  {
    if( windowPtr == windowImpl ) // the window is not deleted
    {
      mResizedSignal.Emit( mAdaptor );

      windowImpl->SetSurface( &newSurface );

      // Flush the event queue to give the update-render thread chance
      // to start processing messages for new camera setup etc as soon as possible
      ProcessCoreEvents();

      // This method blocks until the render thread has completed the replace.
      mThreadController->ReplaceSurface( &newSurface );
      break;
    }
  }
}

void Adaptor::DeleteSurface( Dali::RenderSurfaceInterface& surface )
{
  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  // This method blocks until the render thread has finished rendering the current surface.
  mThreadController->DeleteSurface( &surface );
}

Dali::RenderSurfaceInterface& Adaptor::GetSurface() const
{
  return *mWindows.front()->GetSurface();
}

void Adaptor::ReleaseSurfaceLock()
{
  mWindows.front()->GetSurface()->ReleaseLock();
}

Dali::TtsPlayer Adaptor::GetTtsPlayer(Dali::TtsPlayer::Mode mode)
{
  if( !mTtsPlayers[mode] )
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

bool Adaptor::AddWindow( Dali::Integration::SceneHolder childWindow, const std::string& childWindowName, const std::string& childWindowClassName, bool childWindowMode )
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation( childWindow );
  windowImpl.SetAdaptor( Get() );

  // Add the new Window to the container - the order is not important
  mWindows.push_back( &windowImpl );

  mWindowCreatedSignal.Emit( childWindow );

  return true;
}

bool Adaptor::RemoveWindow( Dali::Integration::SceneHolder* childWindow )
{
  Internal::Adaptor::SceneHolder& windowImpl = Dali::GetImplementation( *childWindow );
  for ( WindowContainer::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter )
  {
    if( *iter == &windowImpl )
    {
      mWindows.erase( iter );
      return true;
    }
  }

  return false;
}

bool Adaptor::RemoveWindow( std::string childWindowName )
{
  for ( WindowContainer::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter )
  {
    if( ( *iter )->GetName() == childWindowName )
    {
      mWindows.erase( iter );
      return true;
    }
  }

  return false;
}

bool Adaptor::RemoveWindow( Internal::Adaptor::SceneHolder* childWindow )
{
  for ( WindowContainer::iterator iter = mWindows.begin(); iter != mWindows.end(); ++iter )
  {
    if( ( *iter )->GetId() == childWindow->GetId() )
    {
      mWindows.erase( iter );
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

Dali::RenderSurfaceInterface* Adaptor::GetRenderSurfaceInterface()
{
  if( !mWindows.empty() )
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
  DALI_ASSERT_DEBUG( mPlatformAbstraction && "PlatformAbstraction not created" );
  return *mPlatformAbstraction;
}

void Adaptor::DestroyTtsPlayer(Dali::TtsPlayer::Mode mode)
{
  if( mTtsPlayers[mode] )
  {
    mTtsPlayers[mode].Reset();
  }
}

Any Adaptor::GetNativeWindowHandle()
{
  return mWindows.front()->GetNativeHandle();
}

Any Adaptor::GetNativeWindowHandle( Dali::Actor actor )
{
  Any nativeWindowHandle;

  Dali::Integration::Scene scene = Dali::Integration::Scene::Get( actor );

  for( auto sceneHolder : mWindows )
  {
    if ( scene == sceneHolder->GetScene() )
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
  if( PAUSED_WHILE_HIDDEN == mState )
  {
    // Adaptor can now be resumed
    mState = PAUSED;

    Resume();

    // Force a render task
    RequestUpdateOnce();
  }
  else if( RUNNING == mState )
  {
    // Force a render task
    RequestUpdateOnce();

    DALI_LOG_RELEASE_INFO( "Adaptor::OnWindowShown: Update requested.\n" );
  }
  else
  {
    DALI_LOG_RELEASE_INFO( "Adaptor::OnWindowShown: Adaptor is not paused state.[%d]\n", mState );
  }
}

void Adaptor::OnWindowHidden()
{
  if( RUNNING == mState || READY == mState )
  {
    bool allWindowsHidden = true;

    for( auto window : mWindows )
    {
      if ( window->IsVisible() )
      {
        allWindowsHidden = false;
        break;
      }
    }

    // Only pause the adaptor when all the windows are hidden
    if( allWindowsHidden )
    {
      if( mState == RUNNING )
      {
        Pause();

        // Adaptor cannot be resumed until any window is shown
        mState = PAUSED_WHILE_HIDDEN;
      }
      else  // mState is READY
      {
        // Pause the adaptor after the state gets RUNNING
        mState = PAUSED_WHILE_INITIALIZING;
      }
    }
    else
    {
      DALI_LOG_RELEASE_INFO( "Adaptor::OnWindowHidden: Some windows are shown. Don't pause adaptor.\n" );
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO( "Adaptor::OnWindowHidden: Adaptor is not running state.[%d]\n", mState );
  }
}

// Dali::Internal::Adaptor::Adaptor::OnDamaged
void Adaptor::OnDamaged( const DamageArea& area )
{
  // This is needed for the case where Dali window is partially obscured
  RequestUpdate( false );
}

void Adaptor::SurfaceResizePrepare( Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize )
{
  mResizedSignal.Emit( mAdaptor );
}

void Adaptor::SurfaceResizeComplete( Dali::RenderSurfaceInterface* surface, SurfaceSize surfaceSize )
{
  // Flush the event queue to give the update-render thread chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  mThreadController->ResizeSurface();
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

  if( mState != PAUSED_WHILE_INITIALIZING )
  {
    mState = RUNNING;

    DALI_LOG_RELEASE_INFO( "Adaptor::NotifySceneCreated: Adaptor is running\n" );
  }
  else
  {
    mState = RUNNING;

    Pause();

    mState = PAUSED_WHILE_HIDDEN;

    DALI_LOG_RELEASE_INFO( "Adaptor::NotifySceneCreated: Adaptor is paused\n" );
  }
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

void Adaptor::RegisterProcessor( Integration::Processor& processor )
{
  GetCore().RegisterProcessor(processor);
}

void Adaptor::UnregisterProcessor( Integration::Processor& processor )
{
  GetCore().UnregisterProcessor(processor);
}

void Adaptor::RequestUpdateOnce()
{
  if( mThreadController )
  {
    mThreadController->RequestUpdateOnce();
  }
}

bool Adaptor::ProcessCoreEventsFromIdle()
{
  ProcessCoreEvents();

  // the idle handle automatically un-installs itself
  mNotificationOnIdleInstalled = false;

  return false;
}

Dali::Internal::Adaptor::SceneHolder* Adaptor::GetWindow( Dali::Actor& actor )
{
  Dali::Integration::Scene scene = Dali::Integration::Scene::Get( actor );

  for( auto window : mWindows )
  {
    if ( scene == window->GetScene() )
    {
      return window;
    }
  }

  return nullptr;
}

Dali::WindowContainer Adaptor::GetWindows() const
{
  Dali::WindowContainer windows;

  for ( auto iter = mWindows.begin(); iter != mWindows.end(); ++iter )
  {
    // Downcast to Dali::Window
    Dali::Window window( dynamic_cast<Dali::Internal::Adaptor::Window*>( *iter ) );
    if ( window )
    {
      windows.push_back( window );
    }
  }

  return windows;
}

Dali::SceneHolderList Adaptor::GetSceneHolders() const
{
  Dali::SceneHolderList sceneHolderList;

  for( auto iter = mWindows.begin(); iter != mWindows.end(); ++iter )
  {
    sceneHolderList.push_back( Dali::Integration::SceneHolder( *iter ) );
  }

  return sceneHolderList;
}

Adaptor::Adaptor(Dali::Integration::SceneHolder window, Dali::Adaptor& adaptor, Dali::RenderSurfaceInterface* surface, EnvironmentOptions* environmentOptions)
: mResizedSignal(),
  mLanguageChangedSignal(),
  mWindowCreatedSignal(),
  mAdaptor( adaptor ),
  mState( READY ),
  mCore( nullptr ),
  mThreadController( nullptr ),
  mGraphics( nullptr ),
  mDisplayConnection( nullptr ),
  mWindows(),
  mPlatformAbstraction( nullptr ),
  mCallbackManager( nullptr ),
  mNotificationOnIdleInstalled( false ),
  mNotificationTrigger( nullptr ),
  mDaliFeedbackPlugin(),
  mFeedbackController( nullptr ),
  mTtsPlayers(),
  mObservers(),
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
  mWindows.insert( mWindows.begin(), &Dali::GetImplementation( window ) );

  gThreadLocalAdaptor = this;
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
