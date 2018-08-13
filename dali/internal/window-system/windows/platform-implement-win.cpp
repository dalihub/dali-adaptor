// CLASS HEADER
#include <dali/internal/window-system/windows/platform-implement-win.h>

// EXTERNAL_HEADERS
#include <windows.h>
#include <map>
#include <set>

using namespace std;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace WindowsPlatformImplement
{

HWND roothWnd = NULL;

void RunLoop()
{
  MSG nMsg = { 0 };

  while( GetMessage( &nMsg, 0, NULL, NULL ) )
  {
    TranslateMessage( &nMsg );
    DispatchMessage( &nMsg );

#ifdef _WIN32
    //::Sleep( 20 );
#endif

    if( WM_CLOSE == nMsg.message )
    {
      break;
    }
  }
}

void SetParent(long child, long parent)
{
    if (roothWnd == (HWND)child && NULL != (HWND)parent)
    {
        roothWnd = (HWND)parent;
    }

    ::SetParent((HWND)child, (HWND)parent);
}

#define INCH 25.4

void GetDPI(float &xDpi, float &yDpi)
{
  HDC hdcScreen = GetDC(roothWnd);

  int iX = GetDeviceCaps( hdcScreen, HORZRES );    // pixel
  int iY = GetDeviceCaps( hdcScreen, VERTRES );    // pixel
  int iPhsX = GetDeviceCaps( hdcScreen, HORZSIZE );    // mm
  int iPhsY = GetDeviceCaps( hdcScreen, VERTSIZE );    // mm

  xDpi = (float)iX / (float)iPhsX * INCH;
  yDpi = (float)iY / (float)iPhsY * INCH;
}

map<int, set<Dali::CallbackBase*>> callbackMap;
set<winEventCallback> listenerSet;

LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  for( set<winEventCallback>::iterator x = listenerSet.begin(); x != listenerSet.end(); ++x )
  {
    ( *x )( (long)hWnd, (long)uMsg, (long)wParam, (long)lParam );
  }

  {
  map<int, set<Dali::CallbackBase*>>::iterator x = callbackMap.find( uMsg );

  if( callbackMap.end() != x )
  {
    set<Dali::CallbackBase*> &cbSet = x->second;
    for( set<Dali::CallbackBase*>::iterator y = cbSet.begin(); cbSet.end() != y; ++y )
    {
      Dali::CallbackBase::Execute(**y);
    }
  }
  }

  LRESULT ret = DefWindowProc( hWnd, uMsg, wParam, lParam );
  return ret;
}

DWORD windowStyle = WS_OVERLAPPED;

int GetEdgeWidth()
{
  switch( windowStyle )
  {
  case WS_OVERLAPPED:
    return 8;
    break;

  default:
    return 0;
    break;
  }
}

int GetEdgeHeight()
{
  switch( windowStyle )
  {
  case WS_OVERLAPPED:
    return 18;
    break;

  default:
    return 0;
    break;
  }
}

long CreateHwnd(
    _In_opt_ const char *lpClassName,
    _In_opt_ const char *lpWindowName,
    _In_ int X,
    _In_ int Y,
    _In_ int nWidth,
    _In_ int nHeight,
    _In_opt_ long parent)
{
  WNDCLASS cs = { 0 };
  cs.cbClsExtra = 0;
  cs.cbWndExtra = 0;
  cs.hbrBackground = (HBRUSH)( COLOR_WINDOW + 2 );
  cs.hCursor = NULL;
  cs.hIcon = NULL;
  cs.hInstance = GetModuleHandle( NULL );
  cs.lpfnWndProc = (WNDPROC)WinProc;
  cs.lpszClassName = lpClassName;
  cs.lpszMenuName = NULL;
  cs.style = CS_VREDRAW | CS_HREDRAW;
  RegisterClass( &cs );//ÊµÏÖ×¢²á´°¿Ú

  HWND hWnd = CreateWindow( lpClassName, lpWindowName, windowStyle, X, Y, nWidth + 2 * GetEdgeWidth(), nHeight + 2 * GetEdgeHeight(), NULL, NULL, cs.hInstance, NULL );
  ShowWindow( hWnd, SW_SHOW );

  if( hWnd != roothWnd )
  {
    roothWnd = hWnd;
  }

  return (long)hWnd;
}

void AddListener(winEventCallback callback)
{
  listenerSet.insert(callback);
}

void AddListener( int msgType, Dali::CallbackBase *callback )
{
  map<int, set<Dali::CallbackBase*>>::iterator x = callbackMap.find( msgType );
  if( callbackMap.end() == x )
  {
    set<Dali::CallbackBase*> callbackSet;
    callbackSet.insert( callback );

    callbackMap.insert( make_pair( msgType, callbackSet ) );
  }
  else
  {
    set<Dali::CallbackBase*> &callbackSet = x->second;
    set<Dali::CallbackBase*>::iterator y = callbackSet.find( callback );
    if( callbackSet.end() == y )
    {
      callbackSet.insert( callback );
    }
  }
}

void RemoveListener(int msgType)
{
  callbackMap.erase(msgType);
}

bool PostWinMessage(
    _In_ unsigned int Msg,
    _In_ long wParam,
    _In_ long lParam,
    _In_ long hWnd/* = 0*/)
{
  if( 0 == hWnd )
  {
    return (bool)PostMessage( roothWnd, Msg, wParam, lParam );
  }
  else
  {
    return (bool)PostMessage( (HWND)hWnd, Msg, wParam, lParam );
  }
}

void ShowWindow(long hWnd)
{
  ::ShowWindow((HWND)hWnd, SW_SHOW);
}

void HideWindow(long hWnd)
{
  ::ShowWindow((HWND)hWnd, SW_HIDE);
}

long CreateWinSemaphore(_In_ long lInitialCount, _In_ long lMaximumCount)
{
  return (long)::CreateSemaphore(NULL, lInitialCount, lMaximumCount, NULL);
}

unsigned long WaitForSingleObject(_In_ long hHandle, _In_ unsigned long dwMilliseconds)
{
  return ::WaitForSingleObject((HWND)hHandle, dwMilliseconds);
}

bool ReleaseSemaphore(_In_ long hSemaphore, _In_ long lReleaseCount, _Out_opt_ long *lpPreviousCount)
{
  return ::ReleaseSemaphore((HWND)hSemaphore, lReleaseCount, lpPreviousCount);
}

struct TTimerCallbackInfo
{
  void *data;
  timerCallback callback;
};

map<int, TTimerCallbackInfo> mapTimerIDToData;

void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT_PTR nTimerid, DWORD dwTime)
{
  map<int, TTimerCallbackInfo>::iterator x = mapTimerIDToData.find( nTimerid );

  if( mapTimerIDToData.end() == x )
  {
    return;
  }

  TTimerCallbackInfo &info = x->second;

  info.callback( info.data );
}

int SetTimer(int interval, timerCallback callback, void *data)
{
  int timerID = 0;

  for( map<int, TTimerCallbackInfo>::iterator x = mapTimerIDToData.begin(); mapTimerIDToData.end() != x; ++x )
  {
    int id = x->first;

    if( timerID < id )
    {
      break;
    }
    else
    {
      timerID++;
    }
  }

  TTimerCallbackInfo callbackInfo;
  callbackInfo.data = data;
  callbackInfo.callback = callback;

  mapTimerIDToData.insert( make_pair( timerID, callbackInfo ) );

  ::SetTimer( roothWnd, timerID, interval, TimerProc );

  return timerID;
}

void KillTimer(int id)
{
  mapTimerIDToData.erase(id);
  ::KillTimer(roothWnd, id);
}

const char* GetKeyName( int keyCode )
{
  switch( keyCode )
  {
  case VK_BACK:
    return "Backspace";

  case VK_TAB:
    return "Tab";

  case VK_RETURN:
    return "Return";

  case VK_ESCAPE:
    return "Escape";

  case VK_SPACE:
    return "space";

  case VK_LEFT:
    return "Left";

  case VK_UP:
    return "Up";

  case VK_RIGHT:
    return "Right";

  case VK_DOWN:
    return "Down";

  case 48:
    return "0";

  case 49:
    return "1";

  case 50:
    return "2";

  case 51:
    return "3";

  case 52:
    return "4";

  case 53:
    return "5";

  case 54:
    return "6";

  case 55:
    return "7";

  case 56:
    return "8";

  case 57:
    return "9";

    break;
  }

  return "";
}

static LARGE_INTEGER _cpuFrequency;
static LARGE_INTEGER *_pCpuFrequency = NULL;

long GetCurrentThreadId()
{
  return ::GetCurrentThreadId();
}

void GetNanoseconds( uint64_t& timeInNanoseconds )
{
  if( NULL == _pCpuFrequency )
  {
    _pCpuFrequency = &_cpuFrequency;
    QueryPerformanceFrequency( _pCpuFrequency );
  }

  LARGE_INTEGER curTime;
  QueryPerformanceCounter( &curTime );

  timeInNanoseconds = (double)curTime.QuadPart / (double)_pCpuFrequency->QuadPart * 1000000000;
}

unsigned int GetCurrentMilliSeconds( void )
{
  if( NULL == _pCpuFrequency )
  {
    _pCpuFrequency = &_cpuFrequency;
    QueryPerformanceFrequency( _pCpuFrequency );
  }

  LARGE_INTEGER curTime;
  QueryPerformanceCounter( &curTime );

  return curTime.QuadPart * 1000 / _pCpuFrequency->QuadPart;
}

} // namespace WindowsPlatformImplement

} // namespace Adaptor

} // namespace internal

} // namespace Dali
