/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#import <Cocoa/Cocoa.h>

#include "extern-definitions.h"

// CLASS HEADER
#include <dali/internal/adaptor/common/framework.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

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
  APP_CREATE,
  APP_TERMINATE,
  APP_PAUSE,
  APP_RESUME,
  APP_RESET,
  APP_LANGUAGE_CHANGE,
};

} // Unnamed namespace

/**
 * Impl to hide WindowsSystem data members
 */
struct Framework::Impl
{
  // Constructor

  Impl(void* data)
  : mAbortCallBack( nullptr ),
    mLanguage( "NOT_SUPPORTED" ),
    mRegion( "NOT_SUPPORTED" )
  {
  }

  ~Impl()
  {
    delete mAbortCallBack;
  }

  std::string GetLanguage() const
  {
    return mLanguage;
  }

  std::string GetRegion() const
  {
    return mRegion;
  }

  void SetAbortCallback( CallbackBase *base )
  {
    mAbortCallBack = base;
  }

  bool ExecuteCallback()
  {
    if( nullptr != mAbortCallBack )
    {
      CallbackBase::Execute( *mAbortCallBack );
      return true;
    }

    return false;
  }

private:
  // Undefined
  Impl( const Impl& impl ) = delete;

  // Undefined
  Impl& operator=( const Impl& impl ) = delete;

private:
  // Data
  CallbackBase* mAbortCallBack;
  std::string mLanguage;
  std::string mRegion;
};

Framework::Framework(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: mObserver(observer),
  mTaskObserver(taskObserver),
  mInitialised(false),
  mPaused(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler( MakeCallback( this, &Framework::AbortCallback ) ),
  mImpl(NULL)
{
    InitThreads();
    mImpl = new Impl(this);

    // ensures the NSApp global object is initialized
    [NSApplication sharedApplication];

    // this is needed for applications without a bundle
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // make sure we can become the key window
    [NSApp activateIgnoringOtherApps:YES];
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
  AppStatusHandler(APP_CREATE, nullptr);
  [NSApp run];
  mRunning = false;
}

void Framework::Quit()
{
  AppStatusHandler(APP_TERMINATE, nullptr);
}

bool Framework::IsMainLoopRunning()
{
  return mRunning;
}

void Framework::AddAbortCallback( CallbackBase* callback )
{
  mImpl->SetAbortCallback( callback );
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
  // "DALI_APPLICATION_PACKAGE" is used by macOS specifically to get the already configured Application package path.
  const char* macEnvironmentVariable = "DALI_APPLICATION_PACKAGE";
  char* value = getenv( macEnvironmentVariable );

  std::string resourcePath;
  if ( value != NULL )
  {
    resourcePath = value;
  }

  if( resourcePath.back() != '/' )
  {
    resourcePath+="/";
  }

  return resourcePath;
}

std::string Framework::GetDataPath()
{
  return app_get_data_path();
}

void Framework::SetBundleId(const std::string& id)
{
  mBundleId = id;
}

void Framework::AbortCallback( )
{
  // if an abort call back has been installed run it.
  if( false == mImpl->ExecuteCallback() )
  {
    Quit();
  }
}

bool Framework::AppStatusHandler(int type, void *)
{
  switch (type)
  {
    case APP_CREATE:
    {
      mInitialised = true;
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
