/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <uv.h>


#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <callback-manager.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{


/**
 * Impl to hide LibUV data members
 */
struct Framework::Impl
{
  // Constructor

  Impl(void* data)
  : mAbortCallBack( NULL ),
    mCallbackManager( NULL )
  {
     mCallbackManager = CallbackManager::New();
     mMainLoop = new uv_loop_t;
     uv_loop_init( mMainLoop );

  }

  ~Impl()
  {
    delete mAbortCallBack;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;

    delete mMainLoop;
  }

  void Run()
  {
    uv_run( mMainLoop , UV_RUN_DEFAULT);

    uv_loop_close( mMainLoop );

  }

  void Quit()
  {
    uv_stop( mMainLoop );
  }

  // Data

  CallbackBase* mAbortCallBack;
  CallbackManager *mCallbackManager;
  uv_loop_t* mMainLoop;


private:
  // Undefined
  Impl( const Impl& impl );

  // Undefined
  Impl& operator=( const Impl& impl );
};

Framework::Framework( Framework::Observer& observer, int *argc, char ***argv )
: mObserver(observer),
  mInitialised(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler( MakeCallback( this, &Framework::AbortCallback ) ),
  mImpl(NULL)
{

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

  mImpl->Run();


  mRunning = false;
}

void Framework::Quit()
{
  mImpl->Quit();
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
}

std::string Framework::GetBundleId() const
{
  return "";
}

void Framework::SetBundleId(const std::string& id)
{
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

bool Framework::AppStatusHandler(int type, void *bundleData)
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
