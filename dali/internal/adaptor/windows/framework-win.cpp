/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <atomic>

// WinDef.h defines ERROR as a macro after debug.h has already sanitized it.
#ifdef ERROR
#undef ERROR
#endif

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
  : mThreadId(0u),
    mQuitRequested(false)
  {
  }

  ~Impl()
  {
  }

  void PrepareMessageQueue()
  {
    // PostThreadMessage() fails until the target thread owns a message queue.
    MSG message{};
    PeekMessage(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    // Publish the id only after the queue exists, so a concurrent Quit()
    // cannot race between these two operations and lose its WM_QUIT.
    const DWORD threadId = GetCurrentThreadId();
    mThreadId.store(threadId, std::memory_order_release);
    if(mQuitRequested.exchange(false, std::memory_order_acq_rel))
    {
      // This is the target thread and its queue has already been created.
      PostQuitMessage(0);
    }
  }

  void Run()
  {
    MSG nMsg{};

    while(true)
    {
      const BOOL result = GetMessage(&nMsg, nullptr, 0, 0);
      if(result == 0)
      {
        break; // WM_QUIT
      }
      if(result == -1)
      {
        DALI_LOG_ERROR("Windows message loop failed, error %lu\n", static_cast<unsigned long>(GetLastError()));
        break;
      }

      if(WIN_CALLBACK_EVENT == nMsg.message)
      {
        WindowsPlatform::ExecuteWinCallback(static_cast<WindowsPlatform::WinCallbackToken>(nMsg.wParam));
        continue;
      }

      TranslateMessage(&nMsg);
      DispatchMessage(&nMsg);
    }

    mThreadId.store(0u, std::memory_order_release);
  }

  void Quit()
  {
    mQuitRequested.store(true, std::memory_order_release);
    const DWORD threadId = mThreadId.load(std::memory_order_acquire);
    if(threadId != 0u && PostThreadMessage(threadId, WM_QUIT, 0, 0))
    {
      mQuitRequested.store(false, std::memory_order_release);
    }
    else if(threadId != 0u)
    {
      DALI_LOG_ERROR("Failed to quit Windows message loop, error %lu\n", static_cast<unsigned long>(GetLastError()));
    }
  }

private:
  std::atomic<DWORD> mThreadId;
  std::atomic_bool   mQuitRequested;

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
  mImpl->PrepareMessageQueue();
  mRunning = true;
  mObserver.OnInit();
  mImpl->Run();

  mObserver.OnTerminate();

  mRunning = false;
}

void FrameworkWin::Quit()
{
  mImpl->Quit();
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
