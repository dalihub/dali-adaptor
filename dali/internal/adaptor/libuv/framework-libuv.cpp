/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/libuv/framework-libuv.h>

// EXTERNAL INCLUDES
#include <X11/Xlib.h>
#include <uv.h>
#include <cstdio>
#include <cstring>

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
struct FrameworkLibuv::Impl
{
  // Constructor
  Impl(void* data)
  {
    mMainLoop = new uv_loop_t;
    gUVLoop   = mMainLoop; // "There can be only one!"

    uv_loop_init(mMainLoop);
  }

  ~Impl()
  {
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
  uv_loop_t* mMainLoop;

private:
  Impl(const Impl& impl) = delete;
  Impl& operator=(const Impl& impl) = delete;
};

FrameworkLibuv::FrameworkLibuv(Framework::Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread),
  mImpl(NULL)
{
  mImpl = new Impl(this);
}

FrameworkLibuv::~FrameworkLibuv()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
}

void FrameworkLibuv::Run()
{
  mRunning = true;
  mObserver.OnInit();
  mImpl->Run();

  mRunning = false;
}

void FrameworkLibuv::Quit()
{
  mObserver.OnTerminate();
  mImpl->Quit();
}

/**
 * Impl for Pre-Initailized using UI Thread.
 */
struct UIThreadLoader::Impl
{
  // Constructor

  Impl(void *data)
  {
  }

  ~Impl()
  {
  }

  void Run(Runner runner)
  {
  }

private:
  // Undefined
  Impl(const Impl& impl);
  Impl& operator=(const Impl& impl);
};

/**
 * UI Thread loader to support Pre-Initailized using UI Thread.
 */
UIThreadLoader::UIThreadLoader(int* argc, char*** argv)
: mArgc(argc),
  mArgv(argv),
  mImpl(nullptr)
{
}

UIThreadLoader::~UIThreadLoader()
{
}

void UIThreadLoader::Run(Runner runner)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
