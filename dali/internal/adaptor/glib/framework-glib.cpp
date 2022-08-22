/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
 */

// CLASS HEADER
#include <dali/internal/adaptor/common/framework.h>

// EXTERNAL INCLUDES
#include <glib.h>
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
thread_local GMainLoop* gMainLoop{nullptr};

GMainContext* GetMainLoopContext()
{
  if(gMainLoop != nullptr)
  {
    return g_main_loop_get_context(gMainLoop);
  }
  return nullptr;
}

/**
 * Impl to hide GLib data members
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

    // In future, may need to change this to have own context or use Tizen context
    gMainLoop = mMainLoop = g_main_loop_new(nullptr, false);
  }

  ~Impl()
  {
    delete mAbortCallBack;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;

    g_main_loop_unref(mMainLoop);
    gMainLoop = nullptr;
  }

  void Run()
  {
    g_main_loop_run(mMainLoop);
  }

  void Quit()
  {
    g_main_loop_quit(mMainLoop);
  }

  // Data
  CallbackBase*    mAbortCallBack;
  CallbackManager* mCallbackManager;
  GMainLoop*       mMainLoop{nullptr};
  GMainContext*    mContext{nullptr};

  std::string mLanguage;
  std::string mRegion;

private:
  Impl(const Impl& impl) = delete;
  Impl& operator=(const Impl& impl) = delete;
};

Framework::Framework(Framework::Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: mObserver(observer),
  mTaskObserver(taskObserver),
  mInitialised(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler(MakeCallback(this, &Framework::AbortCallback)),
  mImpl(NULL)
{
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

void Framework::SetBundleId(const std::string& id)
{
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
