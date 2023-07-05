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
#include <dali/internal/adaptor/windows/framework-win.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <windows.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Impl to hide WindowsSystem data members
 */
struct FrameworkWin::Impl
{
  // Constructor
  Impl(void* data)
  {
  }

  ~Impl()
  {
  }

  void Run()
  {
    MSG nMsg = {0};

    while(GetMessage(&nMsg, 0, NULL, NULL))
    {
      if(WIN_CALLBACK_EVENT == nMsg.message)
      {
        Dali::CallbackBase* callback = (Dali::CallbackBase*)nMsg.wParam;
        Dali::CallbackBase::Execute(*callback);
      }

      TranslateMessage(&nMsg);
      DispatchMessage(&nMsg);

      if(WM_CLOSE == nMsg.message)
      {
        break;
      }
    }
  }

private:
  // Undefined
  Impl(const Impl& impl) = delete;

  // Undefined
  Impl& operator=(const Impl& impl) = delete;
};

FrameworkWin::FrameworkWin(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread),
  mImpl(NULL)
{
  mImpl = new Impl(this);
}

FrameworkWin::~FrameworkWin()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
}

void FrameworkWin::Run()
{
  mRunning = true;

  mObserver.OnInit();
  mImpl->Run();
  mRunning = false;
}

void FrameworkWin::Quit()
{
  mObserver.OnTerminate();
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

  /**
   * Runs the main loop of framework
   */
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
