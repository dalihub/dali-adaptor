/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "adaptor-impl.h"

// EXTERNAL INCLUDES
#include <boost/thread/tss.hpp>
#include <dali/public-api/common/dali-common.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/profiling.h>
#include <dali/integration-api/input-options.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL INCLUDES
#include <base/update-render-controller.h>
#include <base/environment-variables.h>
#include <base/performance-logging/performance-interface-factory.h>
#include <base/lifecycle-observer.h>

#include <callback-manager.h>
#include <trigger-event.h>
#include <render-surface-impl.h>
#include <tts-player-impl.h>
#include <accessibility-manager-impl.h>
#include <timer-impl.h>
#include <events/gesture-manager.h>
#include <events/event-handler.h>
#include <feedback/feedback-controller.h>
#include <feedback/feedback-plugin-proxy.h>
#include <gl/gl-proxy-implementation.h>
#include <gl/gl-implementation.h>
#include <gl/egl-sync-implementation.h>
#include <gl/egl-image-extensions.h>
#include <gl/egl-factory.h>
#include <imf-manager-impl.h>
#include <clipboard-impl.h>
#include <vsync-monitor.h>
#include <object-profiler.h>

#include <slp-logging.h>



namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
boost::thread_specific_ptr<Adaptor> gThreadLocalAdaptor;

unsigned int GetIntegerEnvironmentVariable( const char* variable, unsigned int defaultValue )
{
  const char* variableParameter = std::getenv(variable);

  // if the parameter exists convert it to an integer, else return the default value
  unsigned int intValue = variableParameter ? atoi(variableParameter) : defaultValue;
  return intValue;
}

bool GetIntegerEnvironmentVariable( const char* variable, int& intValue )
{
  const char* variableParameter = std::getenv(variable);

  if( !variableParameter )
  {
    return false;
  }
  // if the parameter exists convert it to an integer, else return the default value
  intValue = atoi(variableParameter);
  return true;
}

bool GetBooleanEnvironmentVariable( const char* variable, bool& boolValue )
{
  const char* variableParameter = std::getenv(variable);

  boolValue = variableParameter ? true : false;
  return boolValue;
}

bool GetFloatEnvironmentVariable( const char* variable, float& floatValue )
{
  const char* variableParameter = std::getenv(variable);

  if( !variableParameter )
  {
    return false;
  }
  // if the parameter exists convert it to an integer, else return the default value
  floatValue = atof(variableParameter);
  return true;
}

} // unnamed namespace

Dali::Adaptor* Adaptor::New( RenderSurface *surface, const DeviceLayout& baseLayout )
{
  DALI_ASSERT_ALWAYS( surface->GetType() != Dali::RenderSurface::NO_SURFACE && "No surface for adaptor" );

  Dali::Adaptor* adaptor = new Dali::Adaptor;
  Adaptor* impl = new Adaptor( *adaptor, surface, baseLayout );
  adaptor->mImpl = impl;

  impl->Initialize();

  return adaptor;
}

void Adaptor::ParseEnvironmentOptions()
{
  // get logging options
  unsigned int logFrameRateFrequency = GetIntegerEnvironmentVariable( DALI_ENV_FPS_TRACKING, 0 );
  unsigned int logupdateStatusFrequency = GetIntegerEnvironmentVariable( DALI_ENV_UPDATE_STATUS_INTERVAL, 0 );
  unsigned int logPerformanceLevel = GetIntegerEnvironmentVariable( DALI_ENV_LOG_PERFORMANCE, 0 );
  unsigned int logPanGesture = GetIntegerEnvironmentVariable( DALI_ENV_LOG_PAN_GESTURE, 0 );

  // all threads here (event, update, and render) will send their logs to SLP Platform's LogMessage handler.
  Dali::Integration::Log::LogFunction  logFunction(Dali::SlpPlatform::LogMessage);

  mEnvironmentOptions.SetLogOptions( logFunction, logFrameRateFrequency, logupdateStatusFrequency, logPerformanceLevel, logPanGesture );

  int predictionMode;
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_PREDICTION_MODE, predictionMode) )
  {
    mEnvironmentOptions.SetPanGesturePredictionMode(predictionMode);
  }
  int predictionAmount = -1;
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_PREDICTION_AMOUNT, predictionAmount) )
  {
    if( predictionAmount < 0 )
    {
      // do not support times in the past
      predictionAmount = 0;
    }
    mEnvironmentOptions.SetPanGesturePredictionAmount(predictionAmount);
  }
  int smoothingMode;
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_SMOOTHING_MODE, smoothingMode) )
  {
    mEnvironmentOptions.SetPanGestureSmoothingMode(smoothingMode);
  }
  float smoothingAmount = 1.0f;
  if( GetFloatEnvironmentVariable(DALI_ENV_PAN_SMOOTHING_AMOUNT, smoothingAmount) )
  {
    smoothingAmount = Clamp(smoothingAmount, 0.0f, 1.0f);
    mEnvironmentOptions.SetPanGestureSmoothingAmount(smoothingAmount);
  }

  int minimumDistance(-1);
  if ( GetIntegerEnvironmentVariable(DALI_ENV_PAN_MINIMUM_DISTANCE, minimumDistance ))
  {
    mEnvironmentOptions.SetMinimumPanDistance( minimumDistance );
  }

  int minimumEvents(-1);
  if ( GetIntegerEnvironmentVariable(DALI_ENV_PAN_MINIMUM_EVENTS, minimumEvents ))
  {
    mEnvironmentOptions.SetMinimumPanEvents( minimumEvents );
  }

  int glesCallTime(0);
  if ( GetIntegerEnvironmentVariable(DALI_GLES_CALL_TIME, glesCallTime ))
  {
    mEnvironmentOptions.SetGlesCallTime( glesCallTime );
  }

  mEnvironmentOptions.InstallLogFunction();
}

void Adaptor::Initialize()
{
  ParseEnvironmentOptions();

  mPlatformAbstraction = new SlpPlatform::SlpPlatformAbstraction;

  if( mEnvironmentOptions.GetPerformanceLoggingLevel() > 0 )
  {
    mPerformanceInterface = PerformanceInterfaceFactory::CreateInterface( *this, mEnvironmentOptions );
  }

  mCallbackManager = CallbackManager::New();

  PositionSize size = mSurface->GetPositionSize();

  mGestureManager = new GestureManager(*this, Vector2(size.width, size.height), mCallbackManager, mEnvironmentOptions);

  if( mEnvironmentOptions.GetGlesCallTime() > 0 )
  {
    mGLES = new GlProxyImplementation( mEnvironmentOptions );
  }
  else
  {
    mGLES = new GlImplementation();
  }

  mEglFactory = new EglFactory();

  EglSyncImplementation* eglSyncImpl = mEglFactory->GetSyncImplementation();

  mCore = Integration::Core::New( *this, *mPlatformAbstraction, *mGLES, *eglSyncImpl, *mGestureManager );

  mObjectProfiler = new ObjectProfiler();

  mNotificationTrigger = new TriggerEvent( boost::bind(&Adaptor::ProcessCoreEvents, this) );

  mVSyncMonitor = new VSyncMonitor;

  mUpdateRenderController = new UpdateRenderController( *this, mEnvironmentOptions );

  mDaliFeedbackPlugin = new FeedbackPluginProxy( FeedbackPluginProxy::DEFAULT_OBJECT_NAME );

  // Should be called after Core creation
  if( mEnvironmentOptions.GetPanGestureLoggingLevel() )
  {
    Integration::EnableProfiling( Dali::Integration::PROFILING_TYPE_PAN_GESTURE );
  }
  if( mEnvironmentOptions.GetPanGesturePredictionMode() >= 0 )
  {
    Integration::SetPanGesturePredictionMode(mEnvironmentOptions.GetPanGesturePredictionMode());
  }
  if( mEnvironmentOptions.GetPanGesturePredictionAmount() >= 0.0f )
  {
    Integration::SetPanGesturePredictionAmount(mEnvironmentOptions.GetPanGesturePredictionAmount());
  }
  if( mEnvironmentOptions.GetPanGestureSmoothingMode() >= 0 )
  {
    Integration::SetPanGestureSmoothingMode(mEnvironmentOptions.GetPanGestureSmoothingMode());
  }
  if( mEnvironmentOptions.GetPanGestureSmoothingAmount() >= 0.0f )
  {
    Integration::SetPanGestureSmoothingAmount(mEnvironmentOptions.GetPanGestureSmoothingAmount());
  }
}

Adaptor::~Adaptor()
{
  // Ensure stop status
  Stop();

  // Release first as we do not want any access to Adaptor as it is being destroyed.
  gThreadLocalAdaptor.release();

  for ( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
  {
    (*iter)->OnDestroy();
  }

  delete mUpdateRenderController; // this will shutdown render thread, which will call Core::ContextDestroyed before exit
  delete mVSyncMonitor;
  delete mEventHandler;
  delete mObjectProfiler;

  delete mCore;
  delete mEglFactory;
  // Delete feedback controller before feedback plugin & style monitor dependencies
  delete mFeedbackController;
  delete mDaliFeedbackPlugin;
  delete mGLES;
  delete mGestureManager;
  delete mPlatformAbstraction;
  delete mCallbackManager;
  delete mPerformanceInterface;

  // uninstall it on this thread (main actor thread)
  Dali::Integration::Log::UninstallLogFunction();
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

  // guarantee map the surface before starting render-thread.
  mSurface->Map();

  // NOTE: dpi must be set before starting the render thread
  // use default or command line settings if not run on device
#ifdef __arm__
  // set the DPI value for font rendering
  unsigned int dpiHor, dpiVer;
  dpiHor = dpiVer = 0;
  mSurface->GetDpi(dpiHor, dpiVer);

  // tell core about the value
  mCore->SetDpi(dpiHor, dpiVer);
#else
  mCore->SetDpi(mHDpi, mVDpi);
#endif

  // Tell the core the size of the surface just before we start the render-thread
  PositionSize size = mSurface->GetPositionSize();
  mCore->SurfaceResized( size.width, size.height );

  // Start the update & render threads
  mUpdateRenderController->Start();

  mState = RUNNING;

  ProcessCoreEvents(); // Ensure any startup messages are processed.

  if ( !mFeedbackController )
  {
    // Start sound & haptic feedback
    mFeedbackController = new FeedbackController( *mDaliFeedbackPlugin );
  }

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
      mEventHandler->Reset();
    }

    mUpdateRenderController->Pause();
    mCore->Suspend();
    mState = PAUSED;
  }
}

// Dali::Internal::Adaptor::Adaptor::Resume
void Adaptor::Resume()
{
  // Only resume the adaptor if we are in the suspended state.
  if( PAUSED == mState )
  {
    // We put ResumeFrameTime first, as this was originally called at the start of mCore->Resume()
    // If there were events pending, mCore->Resume() will call
    //   RenderController->RequestUpdate()
    //     UpdateRenderController->RequestUpdate()
    //       UpdateRenderSynchronization->RequestUpdate()
    // and we should have reset the frame timers before allowing Core->Update() to be called.
    //@todo Should we call UpdateRenderController->Resume before mCore->Resume()?

    mUpdateRenderController->ResumeFrameTime();
    mCore->Resume();
    mUpdateRenderController->Resume();

    mState = RUNNING;

    // Reset the event handler when adaptor resumed
    if( mEventHandler )
    {
      mEventHandler->Reset();
    }

    // Inform observers that we have resumed.
    for( ObserverContainer::iterator iter = mObservers.begin(), endIter = mObservers.end(); iter != endIter; ++iter )
    {
      (*iter)->OnResume();
    }

    ProcessCoreEvents(); // Ensure any outstanding messages are processed
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

    mUpdateRenderController->Stop();
    mCore->Suspend();

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

void Adaptor::FeedWheelEvent( MouseWheelEvent& wheelEvent )
{
  mEventHandler->FeedWheelEvent( wheelEvent );
}

void Adaptor::FeedKeyEvent( KeyEvent& keyEvent )
{
  mEventHandler->FeedKeyEvent( keyEvent );
}

bool Adaptor::MoveResize( const PositionSize& positionSize )
{
  PositionSize old = mSurface->GetPositionSize();

  // just resize the surface. The driver should automatically resize the egl Surface (untested)
  // EGL Spec says : EGL window surfaces need to be resized when their corresponding native window
  // is resized. Implementations typically use hooks into the OS and native window
  // system to perform this resizing on demand, transparently to the client.
  mSurface->MoveResize( positionSize );

  if(old.width != positionSize.width || old.height != positionSize.height)
  {
    SurfaceSizeChanged(positionSize);
  }

  return true;
}

void Adaptor::SurfaceResized( const PositionSize& positionSize )
{
  PositionSize old = mSurface->GetPositionSize();

  // Called by an application, when it has resized a window outside of Dali.
  // The EGL driver automatically detects X Window resize calls, and resizes
  // the EGL surface for us.
  mSurface->MoveResize( positionSize );

  if(old.width != positionSize.width || old.height != positionSize.height)
  {
    SurfaceSizeChanged(positionSize);
  }
}

void Adaptor::ReplaceSurface( Dali::RenderSurface& surface )
{
  // adaptor implementation needs the implementation of
  RenderSurface* internalSurface = dynamic_cast<Internal::Adaptor::RenderSurface*>( &surface );
  DALI_ASSERT_ALWAYS( internalSurface && "Incorrect surface" );

  mSurface = internalSurface;

  SurfaceSizeChanged( internalSurface->GetPositionSize() );

  // flush the event queue to give update and render threads chance
  // to start processing messages for new camera setup etc as soon as possible
  ProcessCoreEvents();

  // this method is synchronous
  mUpdateRenderController->ReplaceSurface(internalSurface);
}

void Adaptor::RenderSync()
{
  mUpdateRenderController->RenderSync();
}

Dali::RenderSurface& Adaptor::GetSurface() const
{
  return *mSurface;
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

bool Adaptor::AddIdle(boost::function<void(void)> callBack)
{
  bool idleAdded(false);

  // Only add an idle if the Adaptor is actually running
  if( RUNNING == mState )
  {
    idleAdded = mCallbackManager->AddCallback(callBack, CallbackManager::IDLE_PRIORITY);
  }

  return idleAdded;
}

bool Adaptor::CallFromMainLoop(boost::function<void(void)> callBack)
{
  bool callAdded(false);

  // Only allow the callback if the Adaptor is actually running
  if ( RUNNING == mState )
  {
    callAdded = mCallbackManager->AddCallback(callBack, CallbackManager::DEFAULT_PRIORITY);
  }

  return callAdded;
}

Dali::Adaptor& Adaptor::Get()
{
  DALI_ASSERT_ALWAYS( gThreadLocalAdaptor.get() != NULL && "Adaptor not instantiated" );
  return gThreadLocalAdaptor->mAdaptor;
}

bool Adaptor::IsAvailable()
{
  return gThreadLocalAdaptor.get() != NULL;
}

Dali::Integration::Core& Adaptor::GetCore()
{
  return *mCore;
}

void Adaptor::DisableVSync()
{
  mUpdateRenderController->DisableVSync();
}

void Adaptor::SetDpi(size_t hDpi, size_t vDpi)
{
  mHDpi = hDpi;
  mVDpi = vDpi;
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

Integration::GlAbstraction& Adaptor::GetGlAbstraction() const
{
  DALI_ASSERT_DEBUG( mGLES && "GLImplementation not created" );
  return *mGLES;
}

Dali::Integration::PlatformAbstraction& Adaptor::GetPlatformAbstractionInterface()
{
  return *mPlatformAbstraction;
}

Dali::Integration::GlAbstraction& Adaptor::GetGlesInterface()
{
  return *mGLES;
}

TriggerEventInterface& Adaptor::GetTriggerEventInterface()
{
  return *mNotificationTrigger;
}
TriggerEventFactoryInterface& Adaptor::GetTriggerEventFactoryInterface()
{
  return mTriggerEventFactory;
}
RenderSurface* Adaptor::GetRenderSurfaceInterface()
{
  return mSurface;
}
VSyncMonitorInterface* Adaptor::GetVSyncMonitorInterface()
{
  return mVSyncMonitor;
}

KernelTraceInterface& Adaptor::GetKernelTraceInterface()
{
  return mKernelTracer;
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
      mPerformanceInterface->AddMarker( PerformanceMarker::PROCESS_EVENTS_START );
    }

    mCore->ProcessEvents();

    if( mPerformanceInterface )
    {
      mPerformanceInterface->AddMarker( PerformanceMarker::PROCESS_EVENTS_END );
    }
  }
}

void Adaptor::RequestUpdate()
{
  // When Dali applications are partially visible behind the lock-screen,
  // the indicator must be updated (therefore allow updates in the PAUSED state)
  if ( PAUSED  == mState ||
       RUNNING == mState )
  {
    mUpdateRenderController->RequestUpdate();
  }
}

void Adaptor::RequestProcessEventsOnIdle()
{
  // Only request a notification if the Adaptor is actually running
  if ( RUNNING == mState )
  {
    boost::unique_lock<boost::mutex> lock( mIdleInstaller );

    // check if the idle handle is already installed
    if( mNotificationOnIdleInstalled )
    {
      return;
    }
    mNotificationOnIdleInstalled = AddIdle( boost::bind( &Adaptor::ProcessCoreEventsFromIdle, this ) );
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
  if ( STOPPED != mState )
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
  RequestUpdate();
}

void Adaptor::SurfaceSizeChanged(const PositionSize& positionSize)
{
  // let the core know the surface size has changed
  mCore->SurfaceResized(positionSize.width, positionSize.height);

  mResizedSignalV2.Emit( mAdaptor );
}

void Adaptor::NotifyLanguageChanged()
{
  mLanguageChangedSignalV2.Emit( mAdaptor );
}

void Adaptor::RequestUpdateOnce()
{
  if( PAUSED_WHILE_HIDDEN != mState )
  {
    if( mUpdateRenderController )
    {
      mUpdateRenderController->RequestUpdateOnce();
    }
  }
}

void Adaptor::ProcessCoreEventsFromIdle()
{
  ProcessCoreEvents();

  // the idle handle automatically un-installs itself
  mNotificationOnIdleInstalled = false;
}

void Adaptor::RegisterSingleton(const std::type_info& info, BaseHandle singleton)
{
  if(singleton)
  {
    mSingletonContainer.insert(SingletonPair(info.name(), singleton));
  }
}

BaseHandle Adaptor::GetSingleton(const std::type_info& info) const
{
  BaseHandle object = Dali::BaseHandle();

  SingletonConstIter iter = mSingletonContainer.find(info.name());
  if(iter != mSingletonContainer.end())
  {
    object = (*iter).second;
  }

  return object;
}

Adaptor::Adaptor(Dali::Adaptor& adaptor, RenderSurface* surface, const DeviceLayout& baseLayout)
: mAdaptor(adaptor),
  mState(READY),
  mCore(NULL),
  mUpdateRenderController(NULL),
  mVSyncMonitor(NULL),
  mGLES( NULL ),
  mEglFactory( NULL ),
  mSurface( surface ),
  mPlatformAbstraction( NULL ),
  mEventHandler( NULL ),
  mCallbackManager( NULL ),
  mNotificationOnIdleInstalled( false ),
  mNotificationTrigger(NULL),
  mGestureManager(NULL),
  mHDpi( 0 ),
  mVDpi( 0 ),
  mDaliFeedbackPlugin(NULL),
  mFeedbackController(NULL),
  mObservers(),
  mDragAndDropDetector(),
  mDeferredRotationObserver(NULL),
  mBaseLayout(baseLayout),
  mEnvironmentOptions(),
  mPerformanceInterface(NULL),
  mObjectProfiler(NULL)
{
  DALI_ASSERT_ALWAYS( gThreadLocalAdaptor.get() == NULL && "Cannot create more than one Adaptor per thread" );
  gThreadLocalAdaptor.reset(this);
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

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
