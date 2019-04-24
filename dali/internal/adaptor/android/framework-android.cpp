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
#include <dali/internal/adaptor/common/framework.h>

// EXTERNAL INCLUDES
#include <unistd.h>
#include <queue>
#include <android_native_app_glue.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/events/key-event.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

/// Application Status Enum
enum
{
  APP_WINDOW_CREATED,
  APP_WINDOW_DESTROYED,
  APP_PAUSE,
  APP_RESUME,
  APP_RESET,
  APP_LANGUAGE_CHANGE,
  APP_DESTROYED,
};

struct ApplicationContext
{
  struct android_app* androidApplication;
  Framework* framework;
};

struct ApplicationContext applicationContext;

// Copied from x server
static unsigned int GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec tp;
  static clockid_t clockid;

  if (!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
      (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if (clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

} // Unnamed namespace

/**
 * Impl to hide android data members
 */
struct Framework::Impl
{

  struct IdleCallback
  {
    int timestamp;
    int timeout;
    int id;
    void* data;
    bool ( *callback )( void *data );

    IdleCallback( int timeout, int id, void* data, bool ( *callback )( void *data ) )
    : timestamp( GetCurrentMilliSeconds() + timeout ),
      timeout( timeout ),
      id ( id ),
      data( data ),
      callback( callback )
    {
    }

    bool operator()()
    {
      return callback( data );
    }

    bool operator<( const IdleCallback& rhs ) const
    {
      return rhs.timestamp > timestamp;
    }
  };

  // Constructor

  Impl(void* data)
  : mAbortCallBack( NULL ),
    mCallbackManager( CallbackManager::New() ),
    mLanguage( "NOT_SUPPORTED" ),
    mRegion( "NOT_SUPPORTED" ),
    mDestroyRequested( false ),
    mIdleReadPipe( -1 ),
    mIdleWritePipe( -1 ),
    mIdleId( 0 )

  {
    applicationContext.framework = static_cast<Framework*>( data );
    int idlepipe[2];
    if( pipe( idlepipe ) )
    {
      DALI_LOG_ERROR( "Failed to open idle pipe\n" );
    }
    else
    {
      mIdleReadPipe = idlepipe[0];
      mIdleWritePipe = idlepipe[1];
    }
  }

  ~Impl()
  {
    close( mIdleReadPipe );
    close( mIdleWritePipe );

    delete mAbortCallBack;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;
  }

  std::string GetLanguage() const
  {
    return mLanguage;
  }

  std::string GetRegion() const
  {
    return mRegion;
  }

  void OnIdle()
  {
    int8_t msg;
    read( mIdleReadPipe, &msg, sizeof( int8_t ) );

    unsigned int ts = GetCurrentMilliSeconds();

    while ( !mIdleCallbacks.empty() )
    {
      IdleCallback callback = mIdleCallbacks.top();
      if( callback.timestamp > ts )
        break;

      mIdleCallbacks.pop();

      auto i = mRemovedIdleCallbacks.begin();
      auto last = mRemovedIdleCallbacks.end();
      while( i != last )
      {
        if ( *i == callback.id )
        {
          mRemovedIdleCallbacks.erase( i );
          break;
        }
      }

      if ( i == last && callback() ) // keep the callback
      {
        AddIdle( callback.timeout, callback.data, callback.callback );
      }
    }

    if ( mIdleCallbacks.empty() )
    {
      mRemovedIdleCallbacks.clear();
    }
  }

  unsigned int AddIdle( int timeout, void* data, bool ( *callback )( void *data ) )
  {
    int8_t msg = 1;
    if( write( mIdleWritePipe, &msg, sizeof( msg ) ) == sizeof( msg ) )
    {
      ++mIdleId;
      if( mIdleId == 0)
      {
        ++mIdleId;
      }
      mIdleCallbacks.push( IdleCallback( timeout, mIdleId, data, callback ) );
    }

    return mIdleId;
  }

  void RemoveIdle( unsigned int id )
  {
    if( id != 0 )
    {
      mRemovedIdleCallbacks.push_back( id );
    }
  }

  int GetIdleTimeout()
  {
    int timeout = -1;
    unsigned int timestamp = GetCurrentMilliSeconds();

    if ( !mIdleCallbacks.empty() )
    {
      IdleCallback idleTimeout = mIdleCallbacks.top();
      timeout = idleTimeout.timestamp - timestamp;
      if( timeout < 0 )
      {
         timeout = 0;
      }
    }

    return timeout;
  }

  // Data
  CallbackBase* mAbortCallBack;
  CallbackManager* mCallbackManager;
  std::string mLanguage;
  std::string mRegion;
  bool mDestroyRequested;

  int mIdleReadPipe;
  int mIdleWritePipe;
  unsigned int mIdleId;
  std::priority_queue<IdleCallback> mIdleCallbacks;
  std::list<int> mRemovedIdleCallbacks;

  // Static methods

  /**
   * Called by AppCore when the application window is createds.
   */
  static bool NativeWindowCreated( Framework* framework, ANativeWindow* window )
  {
    return framework->AppStatusHandler( APP_WINDOW_CREATED, window );
  }

  /**
   * Called by AppCore when the application window is destroyed.
   */
  static void NativeWindowDestroyed( Framework* framework, ANativeWindow* window )
  {
    framework->AppStatusHandler( APP_WINDOW_DESTROYED, window );
  }

  /**
   * Called by AppCore when the application is paused.
   */
  static void NativeAppPaused( Framework* framework )
  {
    framework->AppStatusHandler( APP_PAUSE, NULL );
  }

  /**
   * Called by AppCore when the application is resumed.
   */
  static void NativeAppResumed( Framework* framework )
  {
    framework->AppStatusHandler( APP_RESUME, NULL );
  }

  static void NativeAppTouchEvent( Framework* framework, Dali::TouchPoint& touchPoint, int64_t timeStamp )
  {
    framework->mObserver.OnTouchEvent( touchPoint, timeStamp );
  }

  static void NativeAppKeyEvent( Framework* framework, Dali::KeyEvent& keyEvent )
  {
    framework->mObserver.OnKeyEvent( keyEvent );
  }

  /**
   * Called by AppCore when the language changes on the device.
   */
  static void AppLanguageChange( Framework* framework )
  {
    framework->AppStatusHandler( APP_LANGUAGE_CHANGE, NULL );
  }

  /**
   * Called by AppCore when the application is destroyed.
   */
  static void NativeAppDestroyed( Framework* framework )
  {
    framework->AppStatusHandler( APP_DESTROYED, NULL );
  }
/*
  Start
  Resume
  InitWindow
  GainedFocus

  Pause
  LostFocus
  Stop
  SaveState

  Start
  Resume
  TermWindow
  InitWindow
*/
  static void HandleAppCmd(struct android_app* app, int32_t cmd)
  {
    struct ApplicationContext* context = static_cast< struct ApplicationContext* >( app->userData );
    int8_t tmp;
    switch (cmd)
    {
      case APP_CMD_SAVE_STATE:
        // The system has asked us to save our current state.  Do so.
        // TODO: replace surface doesn't work without major refactoring, postpone for now, just kill the app
        context->framework->mImpl->mDestroyRequested = true;
        tmp = APP_CMD_DESTROY;
        write( app->msgwrite, &tmp, sizeof( tmp ) );
        break;
      case APP_CMD_START:
        break;
      case APP_CMD_STOP:
        break;
      case APP_CMD_RESUME:
        Dali::Internal::Adaptor::Framework::Impl::NativeAppResumed( context->framework );
        break;
      case APP_CMD_PAUSE:
        Dali::Internal::Adaptor::Framework::Impl::NativeAppPaused( context->framework );
        break;
      case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        Dali::Internal::Adaptor::Framework::Impl::NativeWindowCreated( context->framework, app->window );
        break;
      case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        Dali::Internal::Adaptor::Framework::Impl::NativeWindowDestroyed( context->framework, app->window );
        break;
      case APP_CMD_GAINED_FOCUS:
        break;
      case APP_CMD_LOST_FOCUS:
        break;
      case APP_CMD_DESTROY:
        if( context->framework->mImpl->mDestroyRequested ) // not requested by system
        {
          context->framework->mImpl->mDestroyRequested = false;
          app->destroyRequested = 0;

          sleep(1); // wait for the state to be saved

          context->framework->Quit();
        }
        else
        {
          Dali::Internal::Adaptor::Framework::Impl::NativeAppDestroyed( context->framework );
        }
        break;
    }
  }

  static int32_t HandleAppInput(struct android_app* app, AInputEvent* event)
  {
    struct ApplicationContext* context = static_cast< struct ApplicationContext* >( app->userData );

    if( AInputEvent_getType( event ) == AINPUT_EVENT_TYPE_MOTION )
    {
      int32_t deviceId = AInputEvent_getDeviceId( event );
      float x = AMotionEvent_getX( event, 0 );
      float y = AMotionEvent_getY( event, 0 );
      TouchPoint::State state = TouchPoint::Down;
      int32_t action = AMotionEvent_getAction( event );
      int64_t timeStamp = AMotionEvent_getEventTime( event );
      switch ( action & AMOTION_EVENT_ACTION_MASK )
      {
      case AMOTION_EVENT_ACTION_DOWN:
        break;
      case AMOTION_EVENT_ACTION_UP:
        state = TouchPoint::Up;
        break;
      case AMOTION_EVENT_ACTION_MOVE:
        state = TouchPoint::Motion;
        break;
      case AMOTION_EVENT_ACTION_CANCEL:
        state = TouchPoint::Interrupted;
        break;
      case AMOTION_EVENT_ACTION_OUTSIDE:
        state = TouchPoint::Leave;
        break;
      }

      Dali::TouchPoint point( deviceId, state, x, y);
      Dali::Internal::Adaptor::Framework::Impl::NativeAppTouchEvent( context->framework, point, timeStamp );
      return 1;
    }
    else if ( AInputEvent_getType( event ) == AINPUT_EVENT_TYPE_KEY )
    {
      int32_t deviceId = AInputEvent_getDeviceId( event );
      int32_t keyCode = AKeyEvent_getKeyCode( event );
      int32_t action = AKeyEvent_getAction( event );
      int64_t timeStamp = AKeyEvent_getEventTime( event );
      KeyEvent::State state = KeyEvent::Down;
      switch ( action )
      {
      case AKEY_EVENT_ACTION_DOWN:
        break;
      case AKEY_EVENT_ACTION_UP:
        state = KeyEvent::Up;
        break;
      }

      Dali::KeyEvent keyEvent( "", "", keyCode, 0, timeStamp, state );
      Dali::Internal::Adaptor::Framework::Impl::NativeAppKeyEvent( context->framework, keyEvent );
      return 1;
    }

    return 0;
  }

  static void HandleAppIdle(struct android_app* app, struct android_poll_source* source) {
    struct ApplicationContext* context = static_cast< struct ApplicationContext* >( app->userData );
    if( context )
    {
      context->framework->mImpl->OnIdle();
    }
  }

};

Framework::Framework( Framework::Observer& observer, int *argc, char ***argv, Type type )
: mObserver( observer ),
  mInitialised( false ),
  mResume( false ),
  mRunning( false ),
  mArgc( argc ),
  mArgv( argv ),
  mBundleName( "" ),
  mBundleId( "" ),
  mAbortHandler( MakeCallback( this, &Framework::AbortCallback ) ),
  mImpl( NULL )
{
  InitThreads();
  mImpl = new Impl( this );
}

Framework::~Framework()
{
  if( mRunning )
  {
    Quit();

    // Wait for app destroyed command
    Run();
  }

  delete mImpl;
  mImpl = nullptr;
}

void Framework::Run()
{
  mRunning = true;

  while( 1 ) {
    // Read all pending events.
    int id;
    int events;
    struct android_poll_source* source;
    struct android_poll_source idlePollSource;
    idlePollSource.id = LOOPER_ID_USER;
    idlePollSource.app = applicationContext.androidApplication;
    idlePollSource.process = Impl::HandleAppIdle;

    ALooper_addFd(applicationContext.androidApplication->looper,
        mImpl->mIdleReadPipe, LOOPER_ID_USER, ALOOPER_EVENT_INPUT, NULL, &idlePollSource);

    int idleTimeout = -1;
    while( ( id = ALooper_pollAll( idleTimeout, NULL, &events, (void**)&source) ) >= 0 )
    {
      // Process this event.
      if( source != NULL )
      {
        source->process( applicationContext.androidApplication, source );
      }

      if( id == LOOPER_ID_USER )
      {
        idleTimeout = mImpl->GetIdleTimeout();
      }

      // Check if we are exiting.
      if( applicationContext.androidApplication->destroyRequested != 0 )
      {
        mRunning = false;
        return;
      }
    }
  }
}

unsigned int Framework::AddIdle( int timeout, void* data, bool ( *callback )( void *data ) )
{
  if( mImpl )
  {
    return mImpl->AddIdle( timeout, data, callback );
  }

  return -1;
}
void Framework::RemoveIdle( unsigned int id )
{
  if( mImpl )
  {
    mImpl->RemoveIdle( id );
  }
}

void Framework::Quit()
{
  if( applicationContext.androidApplication )
  {
    ANativeActivity_finish( applicationContext.androidApplication->activity );
  }
}

bool Framework::IsMainLoopRunning()
{
  return mRunning;
}

void Framework::AddAbortCallback( CallbackBase* callback )
{
  mImpl->mAbortCallBack = callback;
}

std::string Framework::GetBundleName() const
{
  return mBundleName;
}

void Framework::SetBundleName(const std::string& name)
{
  mBundleName = name;
}

std::string Framework::GetBundleId() const
{
  return mBundleId;
}

std::string Framework::GetResourcePath()
{
  return "";
}

std::string Framework::GetDataPath()
{
  return "";
}

void Framework::SetApplicationContext(void* context)
{
  memset( &applicationContext, 0, sizeof( ApplicationContext ) );
  struct android_app* app = static_cast<android_app*>( context );
  app->userData = &applicationContext;
  app->onAppCmd = Framework::Impl::HandleAppCmd;
  app->onInputEvent = Framework::Impl::HandleAppInput;
  applicationContext.androidApplication = app;
}

void* Framework::GetApplicationContext()
{
  if( applicationContext.androidApplication == nullptr )
  {
    DALI_ASSERT_ALWAYS( 0 && "Failed to get Android context" );
  }
  return applicationContext.androidApplication;
}

Framework* Framework::GetApplicationFramework()
{
  if( applicationContext.framework == nullptr )
  {
    DALI_ASSERT_ALWAYS( 0 && "Failed to get Android framework" );
  }
  return applicationContext.framework;
}

void Framework::SetBundleId(const std::string& id)
{
  mBundleId = id;
}

void Framework::AbortCallback( )
{
  // if an abort call back has been installed run it.
  if (mImpl->mAbortCallBack)
  {
    CallbackBase::Execute( *mImpl->mAbortCallBack );
  }
  else
  {
    Quit();
  }
}

bool Framework::AppStatusHandler(int type, void* data)
{
  switch (type)
  {
    case APP_WINDOW_CREATED:
    {
      if( !mInitialised )
      {
        mObserver.OnInit();
        mInitialised = true;
        mResume = false;
        break;
      }

      mObserver.OnSurfaceCreated( data );

      if( mResume )
      {
        mObserver.OnResume();
        mResume = false;
      }
      break;
    }

    case APP_RESET:
      mObserver.OnReset();
      break;

    case APP_RESUME:
    {
      mResume = true;
      break;
    }

    case APP_WINDOW_DESTROYED:
    {
      mObserver.OnSurfaceDestroyed( data );
      break;
    }

    case APP_PAUSE:
    {
      if( mInitialised )
      {
        mObserver.OnPause();
      }
      break;
    }

    case APP_LANGUAGE_CHANGE:
      mObserver.OnLanguageChanged();
      break;

    case APP_DESTROYED:
    {
       mObserver.OnTerminate();
       mInitialised = false;
       break;
    }

    default:
      break;
  }

  return true;
}

void Framework::InitThreads()
{
}

std::string Framework::GetLanguage() const
{
  return mImpl->GetLanguage();
}

std::string Framework::GetRegion() const
{
  return mImpl->GetRegion();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
