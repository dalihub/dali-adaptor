//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include "adaptor-impl.h"

// EXTERNAL INCLUDES
#include <boost/thread/tss.hpp>
#include <dali/public-api/common/dali-common.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/notification-event.h>

// INTERNAL INCLUDES
#include <base/update-render-controller.h>
#include <base/environment-variables.h>
#include <base/performance-logging/performance-interface-factory.h>
#include <base/lifecycle-observer.h>

#include <internal/common/callback-manager.h>
#include <internal/common/trigger-event.h>
#include <internal/common/render-surface-impl.h>
#include <internal/common/tts-player-impl.h>
#include <internal/common/accessibility-manager-impl.h>
#include <internal/common/timer-impl.h>
#include <internal/common/events/gesture-manager.h>
#include <internal/common/events/event-handler.h>
#include <internal/common/feedback/feedback-controller.h>
#include <internal/common/feedback/feedback-plugin-proxy.h>
#include <internal/common/gl/gl-implementation.h>
#include <internal/common/gl/egl-sync-implementation.h>
#include <internal/common/gl/egl-image-extensions.h>
#include <internal/common/gl/egl-factory.h>
#include <internal/common/imf-manager-impl.h>
#include <internal/common/clipboard-impl.h>
#include <internal/common/vsync-monitor.h>

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

void Adaptor::ParseLogOptions()
{
  const char* resourceLogOption = std::getenv(DALI_ENV_ENABLE_LOG);
  unsigned int logOpts = Integration::Log::ParseLogOptions(resourceLogOption);

  // get logging options
  unsigned int logFrameRateFrequency = GetIntegerEnvironmentVariable( DALI_ENV_FPS_TRACKING, 0 ); ;
  unsigned int logupdateStatusFrequency = GetIntegerEnvironmentVariable( DALI_ENV_UPDATE_STATUS_INTERVAL, 0 );
  unsigned int logPerformanceLevel = GetIntegerEnvironmentVariable( DALI_ENV_LOG_PERFORMANCE, 0 );

  Dali::Integration::Log::LogFunction  logFunction(Dali::SlpPlatform::LogMessage);

  mLogOptions.SetOptions( logFunction, logOpts, logFrameRateFrequency, logupdateStatusFrequency, logPerformanceLevel );

  // all threads here (event, update, and render) will send their logs to SLP Platform's LogMessage handler.
  // Dali::Integration::Log::LogFunction logFunction(Dali::SlpPlatform::LogMessage);
  if( mLogOptions.IsFilterEnabled(  Debug::LogEventThread ))
  {
    mLogOptions.InstallLogFunction();
  }

}
void Adaptor::Initialize()
{
  ParseLogOptions();

  mPlatformAbstraction = new SlpPlatform::SlpPlatformAbstraction;

  if( mLogOptions.GetPerformanceLoggingLevel() > 0 )
  {
    mPerformanceInterface = PerformanceInterfaceFactory::CreateInterface( *mPlatformAbstraction, mLogOptions );
  }

  mCallbackManager = CallbackManager::New();

  PositionSize size = mSurface->GetPositionSize();

  mGestureManager = new GestureManager(*this, Vector2(size.width, size.height), mCallbackManager);

  mGLES = new GlImplementation;

  mEglFactory = new EglFactory();

  EglSyncImplementation* eglSyncImpl = mEglFactory->GetSyncImplementation();

  mCore = Integration::Core::New( *this, *mPlatformAbstraction, *mGLES, *eglSyncImpl, *mGestureManager );

  mNotificationTrigger = new TriggerEvent( boost::bind(&Adaptor::SendNotificationEvent, this) );

  mVSyncMonitor = new VSyncMonitor;

  mUpdateRenderController = new UpdateRenderController( *this, mLogOptions );

  mDaliFeedbackPlugin = new FeedbackPluginProxy( FeedbackPluginProxy::DEFAULT_OBJECT_NAME );
  DALI_LOG_RESOURCE("[INIT] Resource log start\n");
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

  if (mUpdateRenderController)
  {
    delete mUpdateRenderController;
    mUpdateRenderController = NULL;
  }

  if( mVSyncMonitor )
  {
    delete mVSyncMonitor;
    mVSyncMonitor = NULL;
  }

  if (mEventHandler)
  {
    delete mEventHandler;
    mEventHandler = NULL;
  }

  if (mCore)
  {
    delete mCore;
    mCore = NULL;
  }

  // Delete EGL factory after Core, otherwise we may have a crash from GL resource destructors
  if ( mEglFactory )
  {
    delete mEglFactory;
    mEglFactory = NULL;
  }

  // Delete feedback controller before feedback plugin & style monitor dependencies
  delete mFeedbackController;

  if (mDaliFeedbackPlugin)
  {
    delete mDaliFeedbackPlugin;
    mDaliFeedbackPlugin = NULL;
  }

  if (mGLES)
  {
    delete mGLES;
    mGLES = NULL;
  }

  if (mGestureManager)
  {
    delete mGestureManager;
    mGestureManager = NULL;
  }

  if (mPlatformAbstraction)
  {
    delete mPlatformAbstraction;
    mPlatformAbstraction = NULL;
  }

  if (mCallbackManager)
  {
    delete mCallbackManager;
    mCallbackManager = NULL;
  }

  if( mPerformanceInterface )
  {
    delete mPerformanceInterface;
    mPerformanceInterface = NULL;
  }

  // uninstall it on this thread (main actor thread)
  Dali::Integration::Log::UninstallLogFunction();

  DALI_LOG_RESOURCE("[FIN] Resource log end\n");
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

  SendNotificationEvent(); // Ensure any startup messages are processed.

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

    SendNotificationEvent(); // Ensure any outstanding messages are processed
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
  SendNotificationEvent();

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
RenderSurface* Adaptor::GetRenderSurfaceInterface()
{
  return mSurface;
}
VSyncMonitorInterface* Adaptor::GetVSyncMonitorInterface()
{
  return mVSyncMonitor;
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

void Adaptor::RequestNotificationEventOnIdle()
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
    mNotificationOnIdleInstalled = AddIdle( boost::bind( &Adaptor::SendNotificationEventFromIdle, this ) );
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

void Adaptor::SendNotificationEvent()
{
  // Notification events are sent in order to process messages queued (internally) during rendering.
  Integration::NotificationEvent event;
  QueueCoreEvent(event);
  ProcessCoreEvents();
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

void Adaptor::SendNotificationEventFromIdle()
{
  SendNotificationEvent();

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
  mLogOptions(),
  mPerformanceInterface(NULL)
{
  DALI_ASSERT_ALWAYS( gThreadLocalAdaptor.get() == NULL && "Cannot create more than one Adaptor per thread" );
  gThreadLocalAdaptor.reset(this);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
