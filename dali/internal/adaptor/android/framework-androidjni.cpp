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
#include <unordered_set>

#include <jni.h>
#include <sys/types.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/adaptor-framework/application.h>
#include <dali/devel-api/adaptor-framework/application-devel.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/application-impl.h>
#include <dali/internal/system/common/callback-manager.h>

using namespace Dali;

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
  APP_WINDOW_CREATED = 0,
  APP_WINDOW_DESTROYED,
  APP_PAUSE,
  APP_RESUME,
  APP_RESET,
  APP_LANGUAGE_CHANGE,
  APP_DESTROYED,
};

struct ApplicationContext
{
  JNIEnv* jniEnv;
  AAssetManager* assetManager;
  AConfiguration* config;
  ANativeWindow* window;
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
      return timestamp > rhs.timestamp;
    }
  };

  // Constructor

  Impl(void* data)
  : mAbortCallBack( nullptr ),
    mCallbackManager( CallbackManager::New() ),
    mLanguage( "NOT_SUPPORTED" ),
    mRegion( "NOT_SUPPORTED" ),
    mFinishRequested( false )
  {
    applicationContext.framework = static_cast<Framework*>( data );
  }

  ~Impl()
  {
    applicationContext.framework = nullptr;

    delete mAbortCallBack;
    mAbortCallBack = nullptr;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;
    mCallbackManager = nullptr;
  }

  std::string GetLanguage() const
  {
    return mLanguage;
  }

  std::string GetRegion() const
  {
    return mRegion;
  }

  CallbackBase* mAbortCallBack;
  CallbackManager* mCallbackManager;
  std::string mLanguage;
  std::string mRegion;
  bool mFinishRequested;
};

Framework::Framework( Framework::Observer& observer, int *argc, char ***argv, Type type )
: mObserver( observer ),
  mInitialised( false ),
  mResume( false ),
  mSurfaceCreated( false ),
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
  }

  delete mImpl;
  mImpl = nullptr;
}

void Framework::Run()
{
  mRunning = true;
}

unsigned int Framework::AddIdle( int timeout, void* data, bool ( *callback )( void *data ) )
{
  return -1;
}

void Framework::RemoveIdle( unsigned int id )
{
}

void Framework::Quit()
{
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
  return APPLICATION_RESOURCE_PATH;
}

std::string Framework::GetDataPath()
{
  return "";
}

void Framework::SetApplicationContext(void* context)
{
  memset( &applicationContext, 0, sizeof( ApplicationContext ) );
  applicationContext.jniEnv = static_cast<JNIEnv*>( context );
}

void* Framework::GetApplicationContext()
{
  DALI_ASSERT_ALWAYS( applicationContext.jniEnv && "Failed to get Android context" );
  return applicationContext.jniEnv;
}

Framework* Framework::GetApplicationFramework()
{
  DALI_ASSERT_ALWAYS( applicationContext.framework && "Failed to get Android framework" );
  return applicationContext.framework;
}

void* Framework::GetApplicationAssets()
{
  DALI_ASSERT_ALWAYS( applicationContext.assetManager && "Failed to get Android Asset manager" );
  return applicationContext.assetManager;
}

void Framework::SetApplicationAssets(void* assets)
{
  applicationContext.assetManager = static_cast<AAssetManager*>( assets );
}

void* Framework::GetApplicationConfiguration()
{
  DALI_ASSERT_ALWAYS( applicationContext.config && "Failed to get Android configuration" );
  return applicationContext.config;
}

void Framework::SetApplicationConfiguration(void* configuration)
{
  applicationContext.config = static_cast<AConfiguration*>( configuration );
}

void* Framework::GetApplicationWindow()
{
  return applicationContext.window;
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
        applicationContext.window = static_cast< ANativeWindow* >( data );
        mObserver.OnInit();
        mInitialised = true;
      }

      mObserver.OnReplaceSurface( data );
      mSurfaceCreated = true;

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
      if( mSurfaceCreated )
      {
        mObserver.OnResume();
        break;
      }
      mResume = true;
      break;
    }

    case APP_WINDOW_DESTROYED:
    {
      mSurfaceCreated = false;
      mObserver.OnPause();
      mResume = true;
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
