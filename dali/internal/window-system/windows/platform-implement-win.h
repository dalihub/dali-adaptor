#ifndef PLATFORM_IMPLEMENT_WIN_INCLUDE
#define PLATFORM_IMPLEMENT_WIN_INCLUDE

/*
* Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <stdint.h>
#include <dali/public-api/signals/callback.h>

typedef uint64_t   WinWindowHandle;
typedef uint64_t   WinPixmap;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace WindowsPlatformImplementation
{

bool PostWinThreadMessage(
    _In_ uint32_t Msg,
    _In_ uint64_t wParam,
    _In_ uint64_t lParam,
    _In_ uint64_t threadID = -1 );

using timerCallback = bool(*)(void *data);

int SetTimer(int interval, timerCallback callback, void *data);

void KillTimer(int id);

const char* GetKeyName( int keyCode );

uint64_t GetCurrentThreadId();

void GetNanoseconds( uint64_t& timeInNanoseconds );

unsigned int GetCurrentMilliSeconds( void );

class WindowImpl
{
public:
  WindowImpl();

  virtual ~WindowImpl();

  static void ProcWinMessage( uint64_t hWnd, uint32_t uMsg, uint64_t wParam, uint64_t lParam );

  void GetDPI( float &xDpi, float &yDpi );

  int GetColorDepth();

  uint64_t CreateHwnd(
    _In_opt_ const char *lpClassName,
    _In_opt_ const char *lpWindowName,
    _In_ int X,
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ uint64_t parent );

  void SetListener( CallbackBase *callback );

  bool PostWinMessage(
    _In_ uint32_t Msg,
    _In_ uint64_t wParam,
    _In_ uint64_t lParam );

  int32_t GetEdgeWidth();

  int32_t GetEdgeHeight();

  void SetHWND(uint64_t inHWnd);
  void SetWinProc();

protected:

private:

  unsigned long windowStyle;

  int colorDepth;
  uint64_t mHWnd;
  uint64_t mHdc;

  CallbackBase *listener;
};

} // namespace WindowsPlatformImplement

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // WIN32_WINDOWS_SYSTEM_INCLUDE
