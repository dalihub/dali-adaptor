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
*
*/

// CLASS HEADER
#include <dali/internal/window-system/windows/platform-implement-win.h>

// EXTERNAL INCLUDES
#include <windows.h>
#include <map>

// INTERNAL INCLUDES
#include <dali/internal/window-system/windows/event-system-win.h>

namespace
{
constexpr float INCH = 25.4;
}

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
  WindowImpl::ProcWinMessage(reinterpret_cast<uint64_t>(hWnd), uMsg, wParam, lParam);

  LRESULT ret = DefWindowProc(hWnd, uMsg, wParam, lParam);
  return ret;
}

namespace
{
const char* DALI_WINDOW_CLASS_NAME = "DaliWindow";

uint32_t sNumWindows = 0;

void EnsureWindowClassRegistered()
{
  if(sNumWindows == 0)
  {
    WNDCLASS cs      = {0};
    cs.cbClsExtra    = 0;
    cs.cbWndExtra    = 0;
    cs.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    cs.hCursor       = NULL;
    cs.hIcon         = NULL;
    cs.hInstance     = GetModuleHandle(NULL);
    cs.lpfnWndProc   = (WNDPROC)WinProc;
    cs.lpszClassName = DALI_WINDOW_CLASS_NAME.c_str();
    cs.lpszMenuName  = NULL;
    cs.style         = CS_VREDRAW | CS_HREDRAW;
    RegisterClass(&cs);
  }
}

void EnsureWindowClassUnregistered()
{
  if(sNumWindows == 0)
  {
    UnregisterClass(DALI_WINDOW_CLASS_NAME.c_str(), GetModuleHandle(NULL));
  }
}

std::map<uint64_t, WindowImpl*> sHWndToListener;

void RemoveListener(uint64_t hWnd)
{
  std::map<uint64_t, WindowImpl*>::iterator x = sHWndToListener.find(hWnd);
  if(sHWndToListener.end() != x)
  {
    sHWndToListener.erase(x);
  }
}

} // namespace

const uint32_t WindowImpl::STYLE       = WS_OVERLAPPED;
const int32_t  WindowImpl::EDGE_WIDTH  = 8;
const int32_t  WindowImpl::EDGE_HEIGHT = 18;

WindowImpl::WindowImpl()
{
  colorDepth = -1;
  mHWnd      = 0;
  mHdc       = 0;
  listener   = NULL;
}

WindowImpl::~WindowImpl()
{
  RemoveListener(mHWnd);
}

void WindowImpl::ProcWinMessage(uint64_t hWnd, uint32_t uMsg, uint64_t wParam, uint64_t lParam)
{
  std::map<uint64_t, WindowImpl*>::iterator x = sHWndToListener.find(hWnd);

  if(sHWndToListener.end() != x)
  {
    CallbackBase* listener = x->second->listener;

    if(NULL != listener)
    {
      TWinEventInfo eventInfo(hWnd, uMsg, wParam, lParam);
      CallbackBase::Execute(*listener, &eventInfo);
    }
  }
}

void WindowImpl::GetDPI(float& xDpi, float& yDpi)
{
  HDC hdcScreen = GetDC(reinterpret_cast<HWND>(mHWnd));

  int32_t iX    = GetDeviceCaps(hdcScreen, HORZRES);  // pixel
  int32_t iY    = GetDeviceCaps(hdcScreen, VERTRES);  // pixel
  int32_t iPhsX = GetDeviceCaps(hdcScreen, HORZSIZE); // mm
  int32_t iPhsY = GetDeviceCaps(hdcScreen, VERTSIZE); // mm

  xDpi = static_cast<float>(iX) / static_cast<float>(iPhsX) * INCH;
  yDpi = static_cast<float>(iY) / static_cast<float>(iPhsY) * INCH;
}

int WindowImpl::GetColorDepth()
{
  DALI_ASSERT_DEBUG(colorDepth >= 0 && "HWND hasn't been created, no color depth");
  return colorDepth;
}

uint64_t WindowImpl::CreateHwnd(
  _In_opt_ const char* lpWindowName,
  _In_ int             X,
  _In_ int             Y,
  _In_ int             nWidth,
  _In_ int             nHeight,
  _In_opt_ uint64_t    parent)
{
  EnsureWindowClassRegistered();
  ++sNumWindows;

  HWND hWnd = CreateWindow(DALI_WINDOW_CLASS_NAME.c_str(), lpWindowName, STYLE, X, Y, nWidth + 2 * EDGE_WIDTH, nHeight + 2 * EDGE_HEIGHT, NULL, NULL, GetModuleHandle(NULL), NULL);
  ::ShowWindow(hWnd, SW_SHOW);

  return reinterpret_cast<uint64_t>(hWnd);
}

void WindowImpl::DestroyHWnd(uint64_t hWnd)
{
  if(hWnd != 0)
  {
    ::DestroyWindow(reinterpret_cast<HWND>(hWnd));

    --sNumWindows;
    EnsureWindowClassUnregistered();
  }
}

void WindowImpl::SetListener(CallbackBase* callback)
{
  listener = callback;
}

bool WindowImpl::PostWinMessage(
  _In_ uint32_t Msg,
  _In_ uint64_t wParam,
  _In_ uint64_t lParam)
{
  return (bool)PostMessage(reinterpret_cast<HWND>(mHWnd), Msg, wParam, lParam);
}

void WindowImpl::SetHWND(uint64_t inHWnd)
{
  if(mHWnd != inHWnd)
  {
    RemoveListener(mHWnd);

    mHWnd      = inHWnd;
    mHdc       = reinterpret_cast<uint64_t>(GetDC(reinterpret_cast<HWND>(mHWnd)));
    colorDepth = GetDeviceCaps(reinterpret_cast<HDC>(mHdc), BITSPIXEL) * GetDeviceCaps(reinterpret_cast<HDC>(mHdc), PLANES);

    std::map<uint64_t, WindowImpl*>::iterator x = sHWndToListener.find(mHWnd);
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

void WindowImpl::SetWinProc()
{
  // Sets the WinProc function.
  LONG_PTR ret = SetWindowLongPtr((HWND)mHWnd,
                                  GWLP_WNDPROC,
                                  reinterpret_cast<LONG_PTR>(&WinProc));

  if(0 == ret)
  {
    DWORD error = GetLastError();
    return;
  }

  HMODULE module = GetModuleHandle(nullptr);
  ret            = SetWindowLongPtr((HWND)mHWnd,
                         GWLP_HINSTANCE,
                         reinterpret_cast<LONG_PTR>(&module));
}

bool PostWinThreadMessage(
  _In_ uint32_t Msg,
  _In_ uint64_t wParam,
  _In_ uint64_t lParam,
  _In_ uint64_t threadID /* = -1*/)
{
  if(-1 == threadID)
  {
    threadID = GetCurrentThreadId();
  }

  return (bool)PostThreadMessage(threadID, Msg, wParam, lParam);
}

struct TTimerCallbackInfo
{
  void*         data;
  timerCallback callback;
  HWND          hWnd;
};

void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT_PTR nTimerid, DWORD dwTime)
{
  TTimerCallbackInfo* info = (TTimerCallbackInfo*)nTimerid;
  info->callback(info->data);
}

intptr_t SetTimer(int interval, timerCallback callback, void* data)
{
  HWND hwnd = GetActiveWindow();
  if(!hwnd)
  {
    hwnd = FindWindow(DALI_WINDOW_CLASS_NAME.c_str(), nullptr);
  }

  if(!hwnd)
  {
    return -1;
  }

  TTimerCallbackInfo* callbackInfo = new TTimerCallbackInfo;
  callbackInfo->data               = data;
  callbackInfo->callback           = callback;
  callbackInfo->hWnd               = hwnd;

  INT_PTR timerID = (INT_PTR)callbackInfo;
  ::SetTimer(hwnd, timerID, interval, TimerProc);

  return timerID;
}

void KillTimer(intptr_t id)
{
  TTimerCallbackInfo* info = (TTimerCallbackInfo*)id;
  ::KillTimer(info->hWnd, id);
  delete info;
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

static LARGE_INTEGER  cpuFrequency;
static LARGE_INTEGER* pCpuFrequency = NULL;

uint64_t GetCurrentThreadId()
{
  return ::GetCurrentThreadId();
}

void GetNanoseconds(uint64_t& timeInNanoseconds)
{
  if(NULL == pCpuFrequency)
  {
    pCpuFrequency = &cpuFrequency;
    QueryPerformanceFrequency(pCpuFrequency);
  }

  LARGE_INTEGER curTime;
  QueryPerformanceCounter(&curTime);

  timeInNanoseconds = static_cast<double>(curTime.QuadPart) / static_cast<double>(pCpuFrequency->QuadPart) * 1000000000;
}

unsigned int GetCurrentMilliSeconds(void)
{
  if(NULL == pCpuFrequency)
  {
    pCpuFrequency = &cpuFrequency;
    QueryPerformanceFrequency(pCpuFrequency);
  }

  LARGE_INTEGER curTime;
  QueryPerformanceCounter(&curTime);

  return curTime.QuadPart * 1000 / pCpuFrequency->QuadPart;
}

} // namespace WindowsPlatform

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
