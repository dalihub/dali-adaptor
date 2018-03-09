#ifndef _Win32WindowSystem_
#define _Win32WindowSystem_

#include <stdint.h>
#include <dali/public-api/signals/callback.h>

typedef bool Eina_Bool;

typedef unsigned int   Ecore_Window;

typedef unsigned int   Ecore_Win_Window;

typedef unsigned int   Ecore_Win_Drawable;

namespace Win32WindowSystem
{
void RunLoop();

void SetParent(long child, long parent);

int GetEdgeHeight();

long CreateWinWindow(
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

void GetNanoseconds(uint64_t& timeInNanoseconds);

unsigned int GetCurrentMilliSeconds(void);

long CreateWinSemaphore(_In_ long lInitialCount, _In_ long lMaximumCount);

unsigned long WaitForSingleObject(_In_ long hHandle, _In_ unsigned long dwMilliseconds);

bool ReleaseSemaphore(_In_ long hSemaphore, _In_ long lReleaseCount, _Out_opt_ long *lpPreviousCount);

void Sleep(uint64_t timeInNanoseconds);

typedef void(*timerCallback)(void *data);

int SetTimer(int interval, timerCallback callback, void *data);

void KillTimer(int id);

void GetDPI( float &xDpi, float &yDpi );

const char* GetKeyName( int keyCode );

long GetCurrentThreadId();
}

#endif
