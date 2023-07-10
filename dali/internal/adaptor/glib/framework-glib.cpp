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
#include <dali/internal/adaptor/glib/framework-glib.h>

// EXTERNAL INCLUDES
#include <glib.h>
#include <cstdio>
#include <cstring>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
thread_local GMainContext* gContext{nullptr};

GMainContext* GetMainLoopContext()
{
  return gContext;
}

/**
 * Impl to hide GLib data members
 */
struct FrameworkGlib::Impl
{
  // Constructor
  Impl(void* data)
  {
    gContext = mContext = g_main_context_new();
    mMainLoop           = g_main_loop_new(mContext, false);
  }

  ~Impl()
  {
    g_main_loop_unref(mMainLoop);
    g_main_context_unref(mContext);

    gContext = nullptr;
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
  GMainLoop*    mMainLoop{nullptr};
  GMainContext* mContext{nullptr};

private:
  Impl(const Impl& impl) = delete;
  Impl& operator=(const Impl& impl) = delete;
};

FrameworkGlib::FrameworkGlib(Framework::Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread),
  mImpl(NULL)
{
  mImpl = new Impl(this);
}

FrameworkGlib::~FrameworkGlib()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
}

void FrameworkGlib::Run()
{
  mRunning = true;
  mObserver.OnInit();
  mImpl->Run();

  mRunning = false;
}

void FrameworkGlib::Quit()
{
  mObserver.OnTerminate();
  mImpl->Quit();
}

Any FrameworkGlib::GetMainLoopContext() const
{
  return mImpl->mContext;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
