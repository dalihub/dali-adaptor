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
#include "framework.h"

// EXTERNAL INCLUDES
#include <app.h>
#include <bundle.h>
#include <Ecore.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <callback-manager.h>

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
  APP_CREATE,
  APP_TERMINATE,
  APP_PAUSE,
  APP_RESUME,
  APP_RESET,
  APP_LANGUAGE_CHANGE,
};

} // Unnamed namespace

/**
 * Impl to hide EFL data members
 */
struct Framework::Impl
{
  // Constructor

  Impl(void* data)
  : mAbortCallBack( NULL ),
    mCallbackManager( NULL )
  {
    mEventCallback.create = AppCreate;
    mEventCallback.terminate = AppTerminate;
    mEventCallback.pause = AppPause;
    mEventCallback.resume = AppResume;
#if defined(TIZEN_SDK_2_3)
    mEventCallback.app_control = AppControl;
#else
    mEventCallback.service = AppService;
#endif
    mEventCallback.low_memory = NULL;
    mEventCallback.low_battery = NULL;
    mEventCallback.device_orientation = DeviceRotated;
    mEventCallback.language_changed = AppLanguageChange;
    mEventCallback.region_format_changed = NULL;

    mCallbackManager = CallbackManager::New();
  }

  ~Impl()
  {
    delete mAbortCallBack;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;
  }

  // Data

  CallbackBase* mAbortCallBack;
  app_event_callback_s mEventCallback;
  CallbackManager *mCallbackManager;
  // Static methods

  /**
   * Called by AppCore on application creation.
   */
  static bool AppCreate(void *data)
  {
    return static_cast<Framework*>(data)->SlpAppStatusHandler(APP_CREATE);
  }

  /**
   * Called by AppCore when the application should terminate.
   */
  static void AppTerminate(void *data)
  {
    static_cast<Framework*>(data)->SlpAppStatusHandler(APP_TERMINATE);
  }

  /**
   * Called by AppCore when the application is paused.
   */
  static void AppPause(void *data)
  {
    static_cast<Framework*>(data)->SlpAppStatusHandler(APP_PAUSE);
  }

  /**
   * Called by AppCore when the application is resumed.
   */
  static void AppResume(void *data)
  {
    static_cast<Framework*>(data)->SlpAppStatusHandler(APP_RESUME);
  }

  /**
   * Called by AppCore when the application is launched from another module (e.g. homescreen).
   * @param[in] b the bundle data which the launcher module sent
   */

#if defined(TIZEN_SDK_2_3)
  static void AppControl(app_control_h app_control, void *data)
#else
  static void AppService(service_h service, void *data)
#endif
  {
    Framework* framework = static_cast<Framework*>(data);

    if(framework)
    {
      bundle *bundleData = NULL;
#if defined(TIZEN_SDK_2_3)
      app_control_to_bundle(app_control, &bundleData);
#else
      service_to_bundle(service, &bundleData);
#endif

      if(bundleData)
      {
        // get bundle name
        char* bundleName = const_cast<char*>(bundle_get_val(bundleData, "name"));
        if(bundleName != NULL)
        {
          framework->SetBundleName(bundleName);
        }

        // get bundle id
        char* bundleId = const_cast<char*>(bundle_get_val(bundleData, "id"));
        if(bundleId != NULL)
        {
          framework->SetBundleId(bundleId);
        }
      }
      framework->SlpAppStatusHandler(APP_RESET);
    }
  }

  /**
   * Called by AppCore when the language changes on the device.
   */
  static void AppLanguageChange(void* data)
  {
    static_cast<Framework*>(data)->SlpAppStatusHandler(APP_LANGUAGE_CHANGE);
  }

  static void DeviceRotated(app_device_orientation_e orientation, void *user_data)
  {
    switch(orientation)
    {
      case APP_DEVICE_ORIENTATION_0:
        break;
      case APP_DEVICE_ORIENTATION_90:
        break;
      case APP_DEVICE_ORIENTATION_180:
        break;
      case APP_DEVICE_ORIENTATION_270:
        break;
    }
  }

};

Framework::Framework(Framework::Observer& observer, int *argc, char ***argv, const std::string& name)
: mObserver(observer),
  mInitialised(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mName(name),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler( MakeCallback( this, &Framework::AbortCallback ) ),
  mImpl(NULL)
{
  InitThreads();
  mImpl = new Impl(this);
}

Framework::~Framework()
{
  if (mRunning)
  {
    Quit();
  }

  delete mImpl;
}

void Framework::Run()
{
  mRunning = true;

  app_efl_main(mArgc, mArgv, &mImpl->mEventCallback, this);

  mRunning = false;
}

void Framework::Quit()
{
  app_efl_exit();
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

bool Framework::SlpAppStatusHandler(int type)
{
  switch (type)
  {
    case APP_CREATE:
    {
      mInitialised = true;

      // Connect to abnormal exit signals
      mAbortHandler.AbortOnSignal( SIGINT );
      mAbortHandler.AbortOnSignal( SIGQUIT );
      mAbortHandler.AbortOnSignal( SIGKILL );

      mObserver.OnInit();
      break;
    }

    case APP_RESET:
      mObserver.OnReset();
      break;

    case APP_RESUME:
      mObserver.OnResume();
      break;

    case APP_TERMINATE:
     mObserver.OnTerminate();
      break;

    case APP_PAUSE:
      mObserver.OnPause();
      break;

    case APP_LANGUAGE_CHANGE:
      mObserver.OnLanguageChanged();
      break;

    default:
      break;
  }

  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
