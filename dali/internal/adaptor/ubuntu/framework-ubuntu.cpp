/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/ubuntu/framework-ubuntu.h>

// EXTERNAL INCLUDES
#include <X11/Xlib.h>

// INTERNAL INCLUDES
#include <dali/internal/system/linux/dali-ecore.h>
#include <dali/internal/system/linux/dali-efreet.h>
#include <dali/internal/system/linux/dali-elementary.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
FrameworkUbuntu::FrameworkUbuntu(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread)
{
  InitThreads();
}

FrameworkUbuntu::~FrameworkUbuntu()
{
  if(mRunning)
  {
    Quit();
  }
}

void FrameworkUbuntu::Run()
{
  mRunning = true;

  efreet_cache_disable();
  elm_init(mArgc ? *mArgc : 0, mArgv ? *mArgv : nullptr);

  mObserver.OnInit();

  elm_run();

  mRunning = false;
}

void FrameworkUbuntu::Quit()
{
  mObserver.OnTerminate();

  elm_exit();
}

void FrameworkUbuntu::InitThreads()
{
  XInitThreads();
}

/**
 * Impl for Pre-Initailized using UI Thread.
 */
struct UIThreadLoader::Impl
{
  // Constructor
  Impl(void* data)
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
