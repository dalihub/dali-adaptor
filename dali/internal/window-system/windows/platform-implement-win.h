#ifndef PLATFORM_IMPLEMENT_WIN_INCLUDE
#define PLATFORM_IMPLEMENT_WIN_INCLUDE

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

// EXTERNAL_HEADERS
#include <dali/public-api/signals/callback.h>
#include <stdint.h>
#include <memory>
#include <string>

typedef uintptr_t WinWindowHandle;
typedef uint64_t  WinPixmap;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowsPlatform
{
using WinCallbackToken = uintptr_t;

bool PostWinThreadMessage(
  uint32_t  Msg,
  uintptr_t wParam,
  intptr_t  lParam,
  uint32_t  threadID = 0u);

/** Register a callback and transfer its ownership to the Win32 dispatcher. */
WinCallbackToken RegisterWinCallback(CallbackBase* callback);

/** Cancel a callback token. Already queued messages for it become harmless. */
void UnregisterWinCallback(WinCallbackToken token);

/** Post a registered callback token to a Win32 message queue. */
bool PostWinCallback(WinCallbackToken token, uint32_t threadID = 0u);

/** Execute a token if it is still registered. Called by FrameworkWin. */
void ExecuteWinCallback(WinCallbackToken token);

using timerCallback = bool (*)(void* data);

intptr_t SetTimer(uint32_t interval, timerCallback callback, void* data);

void KillTimer(intptr_t id);

/**
 * Return an XKB-compatible physical key name for a Win32 virtual key.
 * lParam is used to distinguish extended and left/right keys.
 */
std::string GetKeyName(int keyCode, intptr_t lParam = 0);

uint32_t GetCurrentThreadId();

void GetNanoseconds(uint64_t& timeInNanoseconds);

unsigned int GetCurrentMilliSeconds(void);

class WindowImpl
{
public:
  static const uint32_t STYLE;
  static const int32_t  EDGE_WIDTH;
  static const int32_t  EDGE_HEIGHT;

  WindowImpl();

  virtual ~WindowImpl();

  static bool ProcWinMessage(WinWindowHandle hWnd, uint32_t uMsg, uintptr_t wParam, intptr_t lParam);

  static intptr_t GetPreviousWindowProc(WinWindowHandle hWnd);

  static void RemoveWindow(WinWindowHandle hWnd);

  static WinWindowHandle CreateHwnd(
    const char*     lpWindowName,
    int             X,
    int             Y,
    int             nWidth,
    int             nHeight,
    WinWindowHandle parent);

  static void DestroyHWnd(WinWindowHandle hWnd);

  void GetDPI(float& xDpi, float& yDpi);

  int GetColorDepth();

  void SetListener(CallbackBase* callback);

  bool PostWinMessage(
    uint32_t  Msg,
    uintptr_t wParam,
    intptr_t  lParam);

  void SetHWND(WinWindowHandle inHWnd);
  bool SetWinProc();
  void DetachWindow();

private:
  int             colorDepth;
  WinWindowHandle mHWnd; // no ownership, managed outside
  intptr_t        mPreviousWindowProc;
  bool            mIsExternalWindow;

  std::shared_ptr<CallbackBase> mListener;
};

} // namespace WindowsPlatform

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // PLATFORM_IMPLEMENT_WIN_INCLUDE
