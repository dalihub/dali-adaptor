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
 *
 */

// CLASS HEADER
#include <dali/internal/window-system/windows/platform-implement-win.h>

// EXTERNAL INCLUDES
#include <windows.h>
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/window-system/windows/event-system-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowsPlatform
{
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  const auto windowHandle       = reinterpret_cast<WinWindowHandle>(hWnd);
  const auto previousWindowProc = WindowImpl::GetPreviousWindowProc(windowHandle);
  const bool handled            = WindowImpl::ProcWinMessage(windowHandle, uMsg, static_cast<uintptr_t>(wParam), static_cast<intptr_t>(lParam));

  LRESULT result = 0;
  if(!handled)
  {
    if(previousWindowProc != 0)
    {
      result = CallWindowProc(reinterpret_cast<WNDPROC>(previousWindowProc), hWnd, uMsg, wParam, lParam);
    }
    else
    {
      result = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
  }

  if(uMsg == WM_NCDESTROY)
  {
    WindowImpl::RemoveWindow(windowHandle);
  }
  return result;
}

namespace
{
const char* DALI_WINDOW_CLASS_NAME = "DaliWindow";

bool                                sWindowClassRegistered = false;
bool                                sWindowClassOwned      = false;
std::unordered_set<WinWindowHandle> sOwnedWindows;

bool EnsureWindowClassRegistered()
{
  if(!sWindowClassRegistered)
  {
    WNDCLASSA cs{};
    cs.cbClsExtra    = 0;
    cs.cbWndExtra    = 0;
    cs.hbrBackground = nullptr;
    cs.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    cs.hIcon         = NULL;
    cs.hInstance     = GetModuleHandleA(nullptr);
    cs.lpfnWndProc   = WinProc;
    cs.lpszClassName = DALI_WINDOW_CLASS_NAME;
    cs.lpszMenuName  = NULL;
    cs.style         = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

    if(RegisterClassA(&cs) == 0)
    {
      const DWORD error = GetLastError();
      if(error != ERROR_CLASS_ALREADY_EXISTS)
      {
        DALI_LOG_ERROR("Failed to register DALi window class, error %lu\n", static_cast<unsigned long>(error));
        return false;
      }

      WNDCLASSA existingClass{};
      if(!GetClassInfoA(cs.hInstance, DALI_WINDOW_CLASS_NAME, &existingClass) || existingClass.lpfnWndProc != WinProc)
      {
        DALI_LOG_ERROR("A different Win32 class is already registered as %s\n", DALI_WINDOW_CLASS_NAME);
        return false;
      }
      sWindowClassOwned = false;
    }
    else
    {
      sWindowClassOwned = true;
    }
    sWindowClassRegistered = true;
  }
  return true;
}

void EnsureWindowClassUnregistered()
{
  if(sWindowClassRegistered && sWindowClassOwned && sOwnedWindows.empty())
  {
    if(UnregisterClassA(DALI_WINDOW_CLASS_NAME, GetModuleHandleA(nullptr)) == 0)
    {
      const DWORD error = GetLastError();
      if(error != ERROR_CLASS_HAS_WINDOWS)
      {
        DALI_LOG_ERROR("Failed to unregister DALi window class, error %lu\n", static_cast<unsigned long>(error));
      }
      return;
    }
    sWindowClassRegistered = false;
    sWindowClassOwned      = false;
  }
}

std::map<WinWindowHandle, WindowImpl*> sHWndToListener;
std::map<WinWindowHandle, intptr_t>    sDetachedWindowProcedures;

struct RegisteredCallback
{
  explicit RegisteredCallback(CallbackBase* callback)
  : mCallback(callback),
    mCancelled(false)
  {
  }

  // Kept locked while the callback executes. A recursive mutex permits a
  // callback to unregister itself while making destruction from another
  // thread wait until execution has returned.
  std::recursive_mutex          mExecutionMutex;
  std::unique_ptr<CallbackBase> mCallback;
  bool                          mCancelled;
};

struct CallbackRegistry
{
  std::mutex                                                                mMutex;
  std::unordered_map<WinCallbackToken, std::shared_ptr<RegisteredCallback>> mCallbacks;
  WinCallbackToken                                                          mNextToken{1u};
};

CallbackRegistry& GetCallbackRegistry()
{
  // Constructed on first registration, so owners created during static
  // initialization are destroyed before the registry at process shutdown.
  static CallbackRegistry registry;
  return registry;
}

void RemoveListener(WinWindowHandle hWnd)
{
  auto x = sHWndToListener.find(hWnd);
  if(sHWndToListener.end() != x)
  {
    sHWndToListener.erase(x);
  }
}

} // namespace

const uint32_t WindowImpl::STYLE       = WS_OVERLAPPEDWINDOW;
const int32_t  WindowImpl::EDGE_WIDTH  = 0;
const int32_t  WindowImpl::EDGE_HEIGHT = 0;

WindowImpl::WindowImpl()
: colorDepth(-1),
  mHWnd(0),
  mPreviousWindowProc(0),
  mIsExternalWindow(false),
  mListener()
{
}

WindowImpl::~WindowImpl()
{
  DetachWindow();
}

bool WindowImpl::ProcWinMessage(WinWindowHandle hWnd, uint32_t uMsg, uintptr_t wParam, intptr_t lParam)
{
  auto x = sHWndToListener.find(hWnd);

  if(sHWndToListener.end() != x)
  {
    WindowImpl* implementation = x->second;
    auto        listener       = implementation->mListener;
    const bool  handleClose    = (uMsg == WM_CLOSE && !implementation->mIsExternalWindow && listener != nullptr);

    if(listener)
    {
      TWinEventInfo eventInfo(hWnd, uMsg, wParam, lParam, static_cast<uint32_t>(GetMessageTime()));
      CallbackBase::Execute(*listener, &eventInfo);
    }
    return handleClose;
  }
  return false;
}

intptr_t WindowImpl::GetPreviousWindowProc(WinWindowHandle hWnd)
{
  auto iter = sHWndToListener.find(hWnd);
  if(iter != sHWndToListener.end())
  {
    return iter->second->mPreviousWindowProc;
  }

  auto detached = sDetachedWindowProcedures.find(hWnd);
  return detached == sDetachedWindowProcedures.end() ? 0 : detached->second;
}

void WindowImpl::RemoveWindow(WinWindowHandle hWnd)
{
  auto iter = sHWndToListener.find(hWnd);
  if(iter != sHWndToListener.end())
  {
    iter->second->mHWnd               = 0;
    iter->second->mPreviousWindowProc = 0;
    iter->second->mIsExternalWindow   = false;
    iter->second->colorDepth          = -1;
    sHWndToListener.erase(iter);
  }

  if(sOwnedWindows.erase(hWnd) != 0u)
  {
    EnsureWindowClassUnregistered();
  }
  sDetachedWindowProcedures.erase(hWnd);
}

void WindowImpl::GetDPI(float& xDpi, float& yDpi)
{
  using GetDpiForWindowFunction     = UINT(WINAPI*)(HWND);
  static const auto getDpiForWindow = []()
  {
    const HMODULE user32 = GetModuleHandleA("user32.dll");
    return user32 ? reinterpret_cast<GetDpiForWindowFunction>(GetProcAddress(user32, "GetDpiForWindow")) : nullptr;
  }();

  if(getDpiForWindow && mHWnd != 0)
  {
    const UINT dpi = getDpiForWindow(reinterpret_cast<HWND>(mHWnd));
    if(dpi != 0u)
    {
      xDpi = static_cast<float>(dpi);
      yDpi = static_cast<float>(dpi);
      return;
    }
  }

  HWND window = reinterpret_cast<HWND>(mHWnd);
  HDC  dc     = GetDC(window);
  if(dc)
  {
    xDpi = static_cast<float>(GetDeviceCaps(dc, LOGPIXELSX));
    yDpi = static_cast<float>(GetDeviceCaps(dc, LOGPIXELSY));
    ReleaseDC(window, dc);
  }
  else
  {
    xDpi = 96.0f;
    yDpi = 96.0f;
  }
}

int WindowImpl::GetColorDepth()
{
  DALI_ASSERT_DEBUG(colorDepth >= 0 && "HWND hasn't been created, no color depth");
  return colorDepth;
}

WinWindowHandle WindowImpl::CreateHwnd(
  const char*     lpWindowName,
  int             X,
  int             Y,
  int             nWidth,
  int             nHeight,
  WinWindowHandle parent)
{
  if(!EnsureWindowClassRegistered())
  {
    return 0;
  }

  const HWND  parentWindow = reinterpret_cast<HWND>(parent);
  const DWORD style        = parentWindow ? (WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) : STYLE;
  RECT        windowRect   = {0, 0, (std::max)(nWidth, 1), (std::max)(nHeight, 1)};
  if(!AdjustWindowRectEx(&windowRect, style, FALSE, 0))
  {
    DALI_LOG_ERROR("Failed to calculate DALi window frame, error %lu\n", static_cast<unsigned long>(GetLastError()));
  }

  HWND hWnd = CreateWindowExA(0,
                              DALI_WINDOW_CLASS_NAME,
                              lpWindowName ? lpWindowName : "DALi",
                              style,
                              X,
                              Y,
                              windowRect.right - windowRect.left,
                              windowRect.bottom - windowRect.top,
                              parentWindow,
                              nullptr,
                              GetModuleHandleA(nullptr),
                              nullptr);

  if(!hWnd)
  {
    DALI_LOG_ERROR("Failed to create DALi window, error %lu\n", static_cast<unsigned long>(GetLastError()));
    EnsureWindowClassUnregistered();
    return 0;
  }

  const auto handle = reinterpret_cast<WinWindowHandle>(hWnd);
  sOwnedWindows.insert(handle);
  return handle;
}

void WindowImpl::DestroyHWnd(WinWindowHandle hWnd)
{
  if(sOwnedWindows.find(hWnd) != sOwnedWindows.end())
  {
    auto listener = sHWndToListener.find(hWnd);
    if(listener != sHWndToListener.end())
    {
      listener->second->DetachWindow();
    }

    HWND       window    = reinterpret_cast<HWND>(hWnd);
    const bool destroyed = !IsWindow(window) || ::DestroyWindow(window) != FALSE;
    if(!destroyed)
    {
      DALI_LOG_ERROR("Failed to destroy DALi window, error %lu\n", static_cast<unsigned long>(GetLastError()));
      return;
    }
    // WM_NCDESTROY may already have erased the handle while DestroyWindow()
    // was on the stack, so erase by key rather than retaining an iterator.
    sOwnedWindows.erase(hWnd);
    EnsureWindowClassUnregistered();
  }
  else
  {
    // Also retries a class-unregister attempt deferred by WM_NCDESTROY.
    EnsureWindowClassUnregistered();
  }
}

void WindowImpl::SetListener(CallbackBase* callback)
{
  mListener.reset(callback);
}

bool WindowImpl::PostWinMessage(
  uint32_t  Msg,
  uintptr_t wParam,
  intptr_t  lParam)
{
  return PostMessage(reinterpret_cast<HWND>(mHWnd), Msg, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)) != FALSE;
}

void WindowImpl::SetHWND(WinWindowHandle inHWnd)
{
  if(mHWnd != inHWnd)
  {
    // SetHWND can be used to switch between host-provided HWNDs. Restore an
    // existing subclass before replacing the handle while retaining DALi's
    // listener for the new one.
    auto listener = std::move(mListener);
    DetachWindow();
    mListener = std::move(listener);

    mHWnd = inHWnd;
    if(mHWnd == 0)
    {
      return;
    }

    HWND window = reinterpret_cast<HWND>(mHWnd);
    HDC  dc     = GetDC(window);
    if(dc)
    {
      colorDepth = GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES);
      ReleaseDC(window, dc);
    }
    else
    {
      colorDepth = -1;
      DALI_LOG_ERROR("Failed to acquire DALi window DC, error %lu\n", static_cast<unsigned long>(GetLastError()));
    }

    auto x = sHWndToListener.find(mHWnd);
    if(sHWndToListener.end() == x)
    {
      sHWndToListener.insert(std::make_pair(mHWnd, this));
    }
    else
    {
      x->second = this;
    }
  }
}

bool WindowImpl::SetWinProc()
{
  if(mHWnd == 0 || mIsExternalWindow)
  {
    return mHWnd != 0;
  }

  auto detached = sDetachedWindowProcedures.find(mHWnd);
  if(detached != sDetachedWindowProcedures.end())
  {
    // DALi's WinProc is already present deeper in another subclass chain.
    // Reattach the listener without inserting the same procedure twice.
    mPreviousWindowProc = detached->second;
    mIsExternalWindow   = true;
    sDetachedWindowProcedures.erase(detached);
    return true;
  }

  // Sets the WinProc function.
  SetLastError(ERROR_SUCCESS);
  LONG_PTR previousWindowProc = SetWindowLongPtr(reinterpret_cast<HWND>(mHWnd),
                                                 GWLP_WNDPROC,
                                                 reinterpret_cast<LONG_PTR>(&WinProc));

  if(previousWindowProc == 0 && GetLastError() != ERROR_SUCCESS)
  {
    DALI_LOG_ERROR("Failed to subclass external window, error %lu\n", static_cast<unsigned long>(GetLastError()));
    return false;
  }

  mPreviousWindowProc = static_cast<intptr_t>(previousWindowProc);
  mIsExternalWindow   = true;
  return true;
}

void WindowImpl::DetachWindow()
{
  if(mHWnd == 0)
  {
    mListener.reset();
    return;
  }

  HWND window                     = reinterpret_cast<HWND>(mHWnd);
  bool restoredPreviousWindowProc = false;
  if(mPreviousWindowProc != 0 && IsWindow(window))
  {
    const LONG_PTR currentWindowProc = GetWindowLongPtr(window, GWLP_WNDPROC);
    if(currentWindowProc == reinterpret_cast<LONG_PTR>(&WinProc))
    {
      SetLastError(ERROR_SUCCESS);
      const LONG_PTR result = SetWindowLongPtr(window, GWLP_WNDPROC, static_cast<LONG_PTR>(mPreviousWindowProc));
      if(result == 0 && GetLastError() != ERROR_SUCCESS)
      {
        DALI_LOG_ERROR("Failed to restore external window procedure, error %lu\n", static_cast<unsigned long>(GetLastError()));
      }
      else
      {
        restoredPreviousWindowProc = true;
      }
    }

    // If another component subclassed the HWND after DALi, our WinProc can
    // remain deeper in its call chain. Keep just the previous-proc forwarding
    // record (never the WindowBase pointer) until WM_NCDESTROY.
    if(!restoredPreviousWindowProc)
    {
      sDetachedWindowProcedures[mHWnd] = mPreviousWindowProc;
    }
  }

  RemoveListener(mHWnd);
  mListener.reset();
  mHWnd               = 0;
  mPreviousWindowProc = 0;
  mIsExternalWindow   = false;
  colorDepth          = -1;
}

bool PostWinThreadMessage(
  uint32_t  Msg,
  uintptr_t wParam,
  intptr_t  lParam,
  uint32_t  threadID /* = 0u */)
{
  if(threadID == 0u)
  {
    threadID = GetCurrentThreadId();
  }

  return PostThreadMessage(threadID, Msg, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam)) != FALSE;
}

WinCallbackToken RegisterWinCallback(CallbackBase* callback)
{
  if(!callback)
  {
    return 0u;
  }

  auto&                       registry           = GetCallbackRegistry();
  auto                        registeredCallback = std::make_shared<RegisteredCallback>(callback);
  std::lock_guard<std::mutex> lock(registry.mMutex);

  WinCallbackToken token{0u};
  do
  {
    token = registry.mNextToken++;
    if(registry.mNextToken == 0u)
    {
      registry.mNextToken = 1u;
    }
  } while(token == 0u || registry.mCallbacks.find(token) != registry.mCallbacks.end());

  registry.mCallbacks.emplace(token, std::move(registeredCallback));
  return token;
}

void UnregisterWinCallback(WinCallbackToken token)
{
  if(token == 0u)
  {
    return;
  }

  auto&                               registry = GetCallbackRegistry();
  std::shared_ptr<RegisteredCallback> registeredCallback;
  {
    std::lock_guard<std::mutex> lock(registry.mMutex);
    auto                        iter = registry.mCallbacks.find(token);
    if(iter == registry.mCallbacks.end())
    {
      return;
    }
    registeredCallback = std::move(iter->second);
    registry.mCallbacks.erase(iter);
  }

  // If another thread is executing the callback this waits for it. If the
  // callback unregisters itself, recursive locking lets cancellation finish
  // and the shared entry keeps the callback wrapper alive until Execute exits.
  std::lock_guard<std::recursive_mutex> executionLock(registeredCallback->mExecutionMutex);
  registeredCallback->mCancelled = true;
}

bool PostWinCallback(WinCallbackToken token, uint32_t threadID)
{
  return token != 0u && PostWinThreadMessage(WIN_CALLBACK_EVENT, token, 0, threadID);
}

void ExecuteWinCallback(WinCallbackToken token)
{
  if(token == 0u)
  {
    return;
  }

  auto&                               registry = GetCallbackRegistry();
  std::shared_ptr<RegisteredCallback> registeredCallback;
  {
    std::lock_guard<std::mutex> lock(registry.mMutex);
    auto                        iter = registry.mCallbacks.find(token);
    if(iter == registry.mCallbacks.end())
    {
      return; // Expected for a callback cancelled after its message was queued.
    }
    registeredCallback = iter->second;
  }

  std::lock_guard<std::recursive_mutex> executionLock(registeredCallback->mExecutionMutex);
  if(!registeredCallback->mCancelled && registeredCallback->mCallback)
  {
    CallbackBase::Execute(*registeredCallback->mCallback);
  }
}

struct TTimerCallbackInfo
{
  void*         data;
  timerCallback callback;
};

thread_local std::unordered_map<UINT_PTR, std::shared_ptr<TTimerCallbackInfo>> gTimerCallbacks;

void CALLBACK TimerProc(HWND, UINT, UINT_PTR nTimerid, DWORD)
{
  auto timer = gTimerCallbacks.find(nTimerid);
  if(timer == gTimerCallbacks.end())
  {
    // KillTimer does not remove an already queued WM_TIMER message.  Looking
    // up the id instead of treating it as a pointer makes such stale messages
    // harmless.
    return;
  }

  // The callback is allowed to stop (and erase) its own timer.
  auto info = timer->second;
  if(info && info->callback)
  {
    info->callback(info->data);
  }
}

intptr_t SetTimer(uint32_t interval, timerCallback callback, void* data)
{
  if(!callback)
  {
    return -1;
  }

  auto callbackInfo      = std::make_shared<TTimerCallbackInfo>();
  callbackInfo->data     = data;
  callbackInfo->callback = callback;

  const UINT_PTR timerId = ::SetTimer(nullptr,
                                      0,
                                      static_cast<UINT>((std::max)(interval, 1u)),
                                      TimerProc);
  if(timerId == 0)
  {
    DALI_LOG_ERROR("Failed to create Windows timer, error %lu\n", static_cast<unsigned long>(GetLastError()));
    return -1;
  }

  gTimerCallbacks.emplace(timerId, std::move(callbackInfo));
  return static_cast<intptr_t>(timerId);
}

void KillTimer(intptr_t id)
{
  if(id < 0)
  {
    return;
  }

  const auto timerId = static_cast<UINT_PTR>(id);
  if(::KillTimer(nullptr, timerId) == 0)
  {
    DALI_LOG_ERROR("Failed to stop Windows timer %llu\n", static_cast<unsigned long long>(timerId));
  }
  gTimerCallbacks.erase(timerId);
}

std::string GetKeyName(int keyCode)
{
  switch(keyCode)
  {
    case VK_BACK:
    {
      return "Backspace";
    }
    case VK_TAB:
    {
      return "Tab";
    }
    case VK_RETURN:
    {
      return "Return";
    }
    case VK_ESCAPE:
    {
      return "Escape";
    }
    case VK_SPACE:
    {
      return "Space";
    }
    case VK_LEFT:
    {
      return "Left";
    }
    case VK_UP:
    {
      return "Up";
    }
    case VK_RIGHT:
    {
      return "Right";
    }
    case VK_DOWN:
    {
      return "Down";
    }
    case 48:
    {
      return "0";
    }
    case 49:
    {
      return "1";
    }
    case 50:
    {
      return "2";
    }
    case 51:
    {
      return "3";
    }
    case 52:
    {
      return "4";
    }
    case 53:
    {
      return "5";
    }
    case 54:
    {
      return "6";
    }
    case 55:
    {
      return "7";
    }
    case 56:
    {
      return "8";
    }
    case 57:
    {
      return "9";
    }
    default:
    {
      if(keyCode > 0 && keyCode < 128)
      {
        return std::string(1u, static_cast<char>(keyCode));
      }
      break;
    }
  }

  return "";
}

namespace
{
const LARGE_INTEGER& GetPerformanceCounterFrequency()
{
  static const LARGE_INTEGER frequency = []()
  {
    LARGE_INTEGER value{};
    if(!QueryPerformanceFrequency(&value) || value.QuadPart <= 0)
    {
      value.QuadPart = 1;
    }
    return value;
  }();
  return frequency;
}
} // unnamed namespace

uint32_t GetCurrentThreadId()
{
  return ::GetCurrentThreadId();
}

void GetNanoseconds(uint64_t& timeInNanoseconds)
{
  LARGE_INTEGER curTime{};
  QueryPerformanceCounter(&curTime);

  const auto& frequency = GetPerformanceCounterFrequency();
  timeInNanoseconds     = static_cast<uint64_t>(static_cast<double>(curTime.QuadPart) / static_cast<double>(frequency.QuadPart) * 1000000000.0);
}

unsigned int GetCurrentMilliSeconds(void)
{
  LARGE_INTEGER curTime{};
  QueryPerformanceCounter(&curTime);

  const auto& frequency = GetPerformanceCounterFrequency();
  return static_cast<unsigned int>(curTime.QuadPart * 1000 / frequency.QuadPart);
}

} // namespace WindowsPlatform

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
