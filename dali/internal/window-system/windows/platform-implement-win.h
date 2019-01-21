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

typedef bool Eina_Bool;

typedef unsigned int   winWindow;
typedef unsigned int   Win_Window_Handle;
typedef unsigned int   WinPixmap;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace WindowsPlatformImplement
{

void RunLoop();

void SetParent(long child, long parent);

int GetEdgeHeight();

long CreateHwnd(
    _In_opt_ const char *lpClassName,
    _In_opt_ const char *lpWindowName,
    _In_ int X,
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ long parent);

typedef void (*winEventCallback)(long hWnd, unsigned int uMsg, long wParam, long lParam);

void RemoveListener(int msgType);

void AddListener( winEventCallback callback );
void AddListener( int msgType, Dali::CallbackBase *callback );

bool PostWinMessage(
    _In_ unsigned int Msg,
    _In_ long wParam,
    _In_ long lParam,
    _In_ long hWnd = 0);

void ShowWindow(long hWnd);
void HideWindow(long hWnd);

long CreateWinSemaphore(_In_ long lInitialCount, _In_ long lMaximumCount);

unsigned long WaitForSingleObject(_In_ long hHandle, _In_ unsigned long dwMilliseconds);

bool ReleaseSemaphore(_In_ long hSemaphore, _In_ long lReleaseCount, _Out_opt_ long *lpPreviousCount);

typedef void(*timerCallback)(void *data);

int SetTimer(int interval, timerCallback callback, void *data);

void KillTimer(int id);

void GetDPI( float &xDpi, float &yDpi );

const char* GetKeyName( int keyCode );

long GetCurrentThreadId();

void GetNanoseconds( uint64_t& timeInNanoseconds );

unsigned int GetCurrentMilliSeconds( void );

} // namespace WindowsPlatformImplement

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // WIN32_WINDOWS_SYSTEM_INCLUDE
