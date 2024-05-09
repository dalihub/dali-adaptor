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
#include <dali/internal/adaptor/common/framework.h>

// EXTERNAL INCLUDES
#include <X11/Xlib.h>
#include <uv.h>
#include <cstdio>
#include <cstring>

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
thread_local uv_loop_t* gUVLoop{nullptr};
}

uv_loop_t* GetUVMainLoop()
{
  return gUVLoop;
}

/**
 * Impl to hide LibUV data members
 */
struct Framework::Impl
{
  // Constructor
  Impl(void* data)
  : mAbortCallBack(nullptr),
    mCallbackManager(nullptr),
    mLanguage("NOT_SUPPORTED"),
    mRegion("NOT_SUPPORTED")
  {
    mCallbackManager = CallbackManager::New();
    mMainLoop        = new uv_loop_t;
    gUVLoop          = mMainLoop; // "There can be only one!"

    uv_loop_init(mMainLoop);
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
    uv_run(mMainLoop, UV_RUN_DEFAULT);
    uv_loop_close(mMainLoop);
  }

  void Quit()
  {
    uv_stop(mMainLoop);
  }

  // Data

  CallbackBase*    mAbortCallBack;
  CallbackManager* mCallbackManager;
  uv_loop_t*       mMainLoop;

  std::string mLanguage;
  std::string mRegion;

private:
  Impl(const Impl& impl) = delete;
  Impl& operator=(const Impl& impl) = delete;
};

Framework::Framework(Framework::Observer& observer, int* argc, char*** argv, Type type)
: mObserver(observer),
  mInitialised(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler(MakeCallback(this, &Framework::AbortCallback)),
  mImpl(NULL)
{
  //InitThreads();
  mImpl = new Impl(this);
}

Framework::~Framework()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
}

void Framework::Run()
{
  mRunning = true;
  mObserver.OnInit();
  mImpl->Run();

  mRunning = false;
}

void Framework::Quit()
{
  mObserver.OnTerminate();
  mImpl->Quit();
}

bool Framework::IsMainLoopRunning()
{
  return mRunning;
}

void Framework::AddAbortCallback(CallbackBase* callback)
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

std::string Framework::GetResourcePath()
{
  // "DALI_APPLICATION_PACKAGE" is used by Ubuntu specifically to get the already configured Application package path.
  const char* ubuntuEnvironmentVariable = "DALI_APPLICATION_PACKAGE";
  char*       value                     = getenv(ubuntuEnvironmentVariable);
  std::string resourcePath;
  if(value != NULL)
  {
    resourcePath = value;
  }

  if(resourcePath.back() != '/')
  {
    resourcePath += "/";
  }

  return resourcePath;
}

void Framework::AbortCallback()
{
  // if an abort call back has been installed run it.
  if(mImpl->mAbortCallBack)
  {
    CallbackBase::Execute(*mImpl->mAbortCallBack);
  }
  else
  {
    Quit();
  }
}

bool Framework::AppStatusHandler(int type, void* bundleData)
{
  return true;
}

std::string Framework::GetLanguage() const
{
  return mImpl->mLanguage;
}

std::string Framework::GetRegion() const
{
  return mImpl->mRegion;
}

std::string Framework::GetDataPath()
{
  const char* ubuntuEnvironmentVariable = "DALI_APPLICATION_DATA_DIR";
  char*       value                     = getenv(ubuntuEnvironmentVariable);
  std::string dataPath;
  if(value != NULL)
  {
    dataPath = value;
  }

  return dataPath;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
