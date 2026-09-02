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
#include <atomic>
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
  if(uMsg == WM_NCHITTEST && GetPropW(hWnd, L"DALI_DRAG_AND_DROP_SHADOW") != nullptr)
  {
    return HTTRANSPARENT;
  }

  // Idle and trigger callbacks are posted as WIN_CALLBACK_EVENT window messages
  // (see PostWinCallback) so that a nested modal message loop (window move/resize)
  // still delivers them here instead of draining them without execution.
  if(uMsg == WIN_CALLBACK_EVENT)
  {
    ExecuteWinCallback(static_cast<WinCallbackToken>(wParam));
    return 0;
  }

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
const char* DALI_WINDOW_CLASS_NAME             = "DaliWindow";
const char* DALI_TRANSPARENT_WINDOW_CLASS_NAME = "DaliTransparentWindow";

bool                                sWindowClassRegistered            = false;
bool                                sWindowClassOwned                 = false;
bool                                sTransparentWindowClassRegistered = false;
bool                                sTransparentWindowClassOwned      = false;
std::unordered_set<WinWindowHandle> sOwnedWindows;

bool RegisterDaliWindowClass(const char* className, UINT classStyle, bool& registered, bool& owned)
{
  if(registered)
  {
    return true;
  }

  WNDCLASSA cs{};
  cs.cbClsExtra    = 0;
  cs.cbWndExtra    = 0;
  cs.hbrBackground = nullptr;
  cs.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  cs.hIcon         = NULL;
  cs.hInstance     = GetModuleHandleA(nullptr);
  cs.lpfnWndProc   = WinProc;
  cs.lpszClassName = className;
  cs.lpszMenuName  = NULL;
  cs.style         = classStyle;

  if(RegisterClassA(&cs) == 0)
  {
    const DWORD error = GetLastError();
    if(error != ERROR_CLASS_ALREADY_EXISTS)
    {
      DALI_LOG_ERROR("Failed to register DALi window class %s, error %lu\n", className, static_cast<unsigned long>(error));
      return false;
    }

    WNDCLASSA existingClass{};
    if(!GetClassInfoA(cs.hInstance, className, &existingClass) || existingClass.lpfnWndProc != WinProc)
    {
      DALI_LOG_ERROR("A different Win32 class is already registered as %s\n", className);
      return false;
    }
    owned = false;
  }
  else
  {
    owned = true;
  }

  registered = true;
  return true;
}

bool EnsureWindowClassRegistered(bool transparent)
{
  // Keep the established class for normal EGL windows and use a dedicated
  // class for the transparent DirectComposition presentation path.
  return RegisterDaliWindowClass(DALI_WINDOW_CLASS_NAME,
                                 CS_VREDRAW | CS_HREDRAW | CS_OWNDC,
                                 sWindowClassRegistered,
                                 sWindowClassOwned) &&
         (!transparent ||
          RegisterDaliWindowClass(DALI_TRANSPARENT_WINDOW_CLASS_NAME,
                                  CS_VREDRAW | CS_HREDRAW,
                                  sTransparentWindowClassRegistered,
                                  sTransparentWindowClassOwned));
}

void EnsureWindowClassUnregistered()
{
  if(!sOwnedWindows.empty())
  {
    return;
  }

  if(sTransparentWindowClassRegistered && sTransparentWindowClassOwned)
  {
    if(UnregisterClassA(DALI_TRANSPARENT_WINDOW_CLASS_NAME, GetModuleHandleA(nullptr)) == 0)
    {
      const DWORD error = GetLastError();
      if(error != ERROR_CLASS_HAS_WINDOWS)
      {
        DALI_LOG_ERROR("Failed to unregister DALi transparent window class, error %lu\n", static_cast<unsigned long>(error));
      }
    }
    else
    {
      sTransparentWindowClassRegistered = false;
      sTransparentWindowClassOwned      = false;
    }
  }

  if(sWindowClassRegistered && sWindowClassOwned)
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

// Cached DALi window used to deliver WIN_CALLBACK_EVENT (idle/trigger callbacks).
// Posting them as window messages - rather than thread messages - lets nested
// modal message loops (window move/resize) dispatch them through WinProc instead
// of dropping them. Updated on the event thread whenever sHWndToListener changes;
// read from any thread that posts a callback, hence atomic.
std::atomic<WinWindowHandle> gDaliMessageWindow{0u};

void RefreshDaliMessageWindow()
{
  gDaliMessageWindow.store(sHWndToListener.empty() ? WinWindowHandle{0u} : sHWndToListener.begin()->first, std::memory_order_relaxed);
}
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
    RefreshDaliMessageWindow();

    // WIN_CALLBACK_EVENT messages can be queued to a short-lived DALi window
    // such as a drag shadow. Move them before the HWND is destroyed; otherwise
    // the callback manager still considers its wake-up registered although
    // Windows has discarded the only message that could execute it.
    MSG  message{};
    HWND window = reinterpret_cast<HWND>(hWnd);
    while(PeekMessageW(&message, window, WIN_CALLBACK_EVENT, WIN_CALLBACK_EVENT, PM_REMOVE) != FALSE)
    {
      if(!PostWinCallback(static_cast<WinCallbackToken>(message.wParam)))
      {
        DALI_LOG_ERROR("Failed to requeue a Windows callback while detaching HWND %p\n", window);
      }
    }
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
    RefreshDaliMessageWindow();
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
  WinWindowHandle parent,
  bool            transparent)
{
  if(!EnsureWindowClassRegistered(transparent && parent == 0u))
  {
    return 0;
  }

  const HWND  parentWindow = reinterpret_cast<HWND>(parent);
  const DWORD style        = parentWindow ? (WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) : STYLE;
  // DirectComposition owns the transparent window's visual. WS_EX_NOREDIRECTIONBITMAP
  // prevents the legacy redirected bitmap from placing an opaque surface behind it.
  //
  // Do not combine WS_EX_LAYERED with this path. Without the classic layered-window
  // APIs, that combination caused unstable caption and border painting during show
  // and reposition operations. DirectComposition provides per-pixel alpha by itself.
  const DWORD exStyle = transparent && !parentWindow ? WS_EX_NOREDIRECTIONBITMAP : 0u;
  const char* className    = transparent && !parentWindow ? DALI_TRANSPARENT_WINDOW_CLASS_NAME : DALI_WINDOW_CLASS_NAME;
  RECT        windowRect   = {0, 0, (std::max)(nWidth, 1), (std::max)(nHeight, 1)};
  if(!AdjustWindowRectEx(&windowRect, style, FALSE, exStyle))
  {
    DALI_LOG_ERROR("Failed to calculate DALi window frame, error %lu\n", static_cast<unsigned long>(GetLastError()));
  }

  HWND hWnd = CreateWindowExA(exStyle,
                              className,
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

bool WindowImpl::SetTransparency(bool transparent)
{
  // Host-owned HWNDs may have their own composition policy. The initial
  // implementation only changes windows created and owned by DALi.
  if(mHWnd == 0 || mIsExternalWindow)
  {
    return false;
  }

  HWND window = reinterpret_cast<HWND>(mHWnd);
  if(!IsWindow(window))
  {
    return false;
  }

  SetLastError(ERROR_SUCCESS);
  const LONG_PTR previousStyle = GetWindowLongPtr(window, GWL_EXSTYLE);
  if(previousStyle == 0 && GetLastError() != ERROR_SUCCESS)
  {
    DALI_LOG_ERROR("Failed to read DALi window style, error %lu\n", static_cast<unsigned long>(GetLastError()));
    return false;
  }

  const bool currentlyTransparent = (previousStyle & WS_EX_NOREDIRECTIONBITMAP) != 0;
  if(transparent == currentlyTransparent)
  {
    return true;
  }

  // The HWND class/style and EGL DirectComposition surface must be selected
  // together at creation time. Replacing them on a live window is deferred.
  DALI_LOG_RELEASE_INFO("Runtime transparency changes are not supported by the Windows backend\n");
  return false;
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
    RefreshDaliMessageWindow();
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
  if(token == 0u)
  {
    return false;
  }

  // Prefer posting to a DALi window: a WIN_CALLBACK_EVENT window message is still
  // dispatched to WinProc by a nested modal message loop (window move/resize),
  // whereas a thread message is drained by that loop without ever reaching
  // FrameworkWin::Run. Fall back to a thread message before any window exists.
  const WinWindowHandle messageWindow = gDaliMessageWindow.load(std::memory_order_relaxed);
  if(messageWindow != 0u &&
     PostMessage(reinterpret_cast<HWND>(messageWindow), WIN_CALLBACK_EVENT, static_cast<WPARAM>(token), 0) != FALSE)
  {
    return true;
  }

  return PostWinThreadMessage(WIN_CALLBACK_EVENT, token, 0, threadID);
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

std::string GetKeyName(int keyCode, intptr_t lParam)
{
  const auto nativeData = static_cast<uintptr_t>(lParam);
  const bool extended   = (nativeData & (1u << 24u)) != 0u;
  const UINT scanCode   = static_cast<UINT>((nativeData >> 16u) & 0xffu);

  // VK_HANGUL and VK_KANA share a value. DALi's public key table exposes the
  // language key as Hangul, which also matches the existing Linux backends.
  if(keyCode == VK_HANGUL)
  {
    return "Hangul";
  }

  if(keyCode >= '0' && keyCode <= '9')
  {
    return std::string(1u, static_cast<char>(keyCode));
  }
  if(keyCode >= 'A' && keyCode <= 'Z')
  {
    return std::string(1u, static_cast<char>('a' + keyCode - 'A'));
  }
  if(keyCode >= VK_NUMPAD0 && keyCode <= VK_NUMPAD9)
  {
    return "KP_" + std::to_string(keyCode - VK_NUMPAD0);
  }
  if(keyCode >= VK_F1 && keyCode <= VK_F24)
  {
    return "F" + std::to_string(keyCode - VK_F1 + 1);
  }

  switch(keyCode)
  {
    case VK_BACK:
    {
      return "BackSpace";
    }
    case VK_TAB:
    {
      return "Tab";
    }
    case VK_RETURN:
    {
      return extended ? "KP_Enter" : "Return";
    }
    case VK_SHIFT:
    {
      const UINT mappedKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
      return mappedKey == VK_RSHIFT ? "Shift_R" : "Shift_L";
    }
    case VK_LSHIFT:
    {
      return "Shift_L";
    }
    case VK_RSHIFT:
    {
      return "Shift_R";
    }
    case VK_CONTROL:
    {
      return extended ? "Control_R" : "Control_L";
    }
    case VK_LCONTROL:
    {
      return "Control_L";
    }
    case VK_RCONTROL:
    {
      return "Control_R";
    }
    case VK_MENU:
    {
      return extended ? "Alt_R" : "Alt_L";
    }
    case VK_LMENU:
    {
      return "Alt_L";
    }
    case VK_RMENU:
    {
      return "Alt_R";
    }
    case VK_PAUSE:
    {
      return "Pause";
    }
    case VK_CAPITAL:
    {
      return "Caps_Lock";
    }
    case VK_ESCAPE:
    {
      return "Escape";
    }
    case VK_SPACE:
    {
      return "Space";
    }
    case VK_PRIOR:
    {
      return extended ? "Prior" : "KP_Prior";
    }
    case VK_NEXT:
    {
      return extended ? "Next" : "KP_Next";
    }
    case VK_END:
    {
      return extended ? "End" : "KP_End";
    }
    case VK_HOME:
    {
      return extended ? "Home" : "KP_Home";
    }
    case VK_LEFT:
    {
      return extended ? "Left" : "KP_Left";
    }
    case VK_UP:
    {
      return extended ? "Up" : "KP_Up";
    }
    case VK_RIGHT:
    {
      return extended ? "Right" : "KP_Right";
    }
    case VK_DOWN:
    {
      return extended ? "Down" : "KP_Down";
    }
    case VK_CLEAR:
    {
      return "KP_Begin";
    }
    case VK_SELECT:
    {
      return "Select";
    }
    case VK_PRINT:
    {
      return "Print";
    }
    case VK_EXECUTE:
    {
      return "Execute";
    }
    case VK_SNAPSHOT:
    {
      return "Print";
    }
    case VK_INSERT:
    {
      return extended ? "Insert" : "KP_Insert";
    }
    case VK_DELETE:
    {
      return extended ? "Delete" : "KP_Delete";
    }
    case VK_HELP:
    {
      return "Help";
    }
    case VK_LWIN:
    {
      return "Super_L";
    }
    case VK_RWIN:
    {
      return "Super_R";
    }
    case VK_APPS:
    {
      return "Menu";
    }
    case VK_SLEEP:
    {
      return "XF86Standby";
    }
    case VK_MULTIPLY:
    {
      return "KP_Multiply";
    }
    case VK_ADD:
    {
      return "KP_Add";
    }
    case VK_SEPARATOR:
    {
      return "KP_Separator";
    }
    case VK_SUBTRACT:
    {
      return "KP_Subtract";
    }
    case VK_DECIMAL:
    {
      return "KP_Decimal";
    }
    case VK_DIVIDE:
    {
      return "KP_Divide";
    }
    case VK_NUMLOCK:
    {
      return "Num_Lock";
    }
    case VK_SCROLL:
    {
      return "Scroll_Lock";
    }
    case VK_BROWSER_BACK:
    {
      return "XF86Back";
    }
    case VK_BROWSER_FORWARD:
    {
      return "XF86Forward";
    }
    case VK_BROWSER_REFRESH:
    {
      return "XF86Refresh";
    }
    case VK_BROWSER_STOP:
    {
      return "XF86Stop";
    }
    case VK_BROWSER_SEARCH:
    {
      return "XF86Search";
    }
    case VK_BROWSER_FAVORITES:
    {
      return "XF86Favorites";
    }
    case VK_BROWSER_HOME:
    {
      return "XF86HomePage";
    }
    case VK_VOLUME_MUTE:
    {
      return "XF86AudioMute";
    }
    case VK_VOLUME_DOWN:
    {
      return "XF86AudioLowerVolume";
    }
    case VK_VOLUME_UP:
    {
      return "XF86AudioRaiseVolume";
    }
    case VK_MEDIA_NEXT_TRACK:
    {
      return "XF86AudioNext";
    }
    case VK_MEDIA_PREV_TRACK:
    {
      return "XF86AudioPrev";
    }
    case VK_MEDIA_STOP:
    {
      return "XF86AudioStop";
    }
    case VK_MEDIA_PLAY_PAUSE:
    {
      return "XF86AudioPlayPause";
    }
    case VK_LAUNCH_MAIL:
    {
      return "XF86Mail";
    }
    case VK_LAUNCH_MEDIA_SELECT:
    {
      return "XF86AudioMedia";
    }
    case VK_LAUNCH_APP1:
    {
      return "XF86Launch1";
    }
    case VK_LAUNCH_APP2:
    {
      return "XF86Launch2";
    }
    case VK_OEM_1:
    {
      return "semicolon";
    }
    case VK_OEM_PLUS:
    {
      return "equal";
    }
    case VK_OEM_COMMA:
    {
      return "comma";
    }
    case VK_OEM_MINUS:
    {
      return "minus";
    }
    case VK_OEM_PERIOD:
    {
      return "period";
    }
    case VK_OEM_2:
    {
      return "slash";
    }
    case VK_OEM_3:
    {
      return "grave";
    }
    case VK_OEM_4:
    {
      return "bracketleft";
    }
    case VK_OEM_5:
    {
      return "backslash";
    }
    case VK_OEM_6:
    {
      return "bracketright";
    }
    case VK_OEM_7:
    {
      return "apostrophe";
    }
    case VK_OEM_102:
    {
      return "less";
    }
    case VK_PROCESSKEY:
    {
      return "Process";
    }
    case VK_PACKET:
    {
      return "Packet";
    }
    default:
    {
      break;
    }
  }

  return "Keycode-" + std::to_string(keyCode);
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
