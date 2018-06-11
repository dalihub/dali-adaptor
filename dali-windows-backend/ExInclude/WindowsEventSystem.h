#pragma once
#ifndef _WINDOWEVENTSYSTEM_H_
#define _WINDOWEVENTSYSTEM_H_

#include <Win32WindowSystem.h>

#define ECORE_EVENT_MOUSE_BUTTON_DOWN   WM_LBUTTONDOWN
#define ECORE_EVENT_MOUSE_BUTTON_UP     WM_LBUTTONUP
#define ECORE_EVENT_MOUSE_MOVE          WM_MOUSEMOVE
#define ECORE_EVENT_MOUSE_OUT           WM_MOUSELEAVE
#define ECORE_EVENT_MOUSE_WHEEL         WM_MOUSEWHEEL

#define ECORE_EVENT_KEY_DOWN            WM_KEYDOWN
#define ECORE_EVENT_KEY_UP              WM_KEYUP

#define ECORE_WIN_EVENT_WINDOW_FOCUS_IN   WM_SETFOCUS
#define ECORE_WIN_EVENT_WINDOW_FOCUS_OUT  WM_KILLFOCUS

#define ECORE_WIN_EVENT_WINDOW_DAMAGE     WM_PAINT

#define ECORE_WIN_EVENT_XDND_ENTER        0
#define ECORE_WIN_EVENT_XDND_POSITION     0
#define ECORE_WIN_EVENT_XDND_LEAVE        0
#define ECORE_WIN_EVENT_XDND_DROP         0

#define ECORE_WIN_EVENT_XDND_FINISHED     0
#define ECORE_WIN_EVENT_XDND_STATUS       0

#define ECORE_WIN_EVENT_CLIENT_MESSAGE    0
#define ECORE_WIN_EVENT_SELECTION_CLEAR   0
#define ECORE_WIN_EVENT_SELECTION_NOTIFY  0

#define ECORE_CALLBACK_PASS_ON true;

#define ECORE_EVENT_MODIFIER_SHIFT      0x0001
#define ECORE_EVENT_MODIFIER_CTRL       0x0002
#define ECORE_EVENT_MODIFIER_ALT        0x0004
#define ECORE_EVENT_MODIFIER_WIN        0x0008
#define ECORE_EVENT_MODIFIER_SCROLL     0x0010
#define ECORE_EVENT_MODIFIER_NUM        0x0020
#define ECORE_EVENT_MODIFIER_CAPS       0x0040
#define ECORE_EVENT_LOCK_SCROLL         0x0080
#define ECORE_EVENT_LOCK_NUM            0x0100
#define ECORE_EVENT_LOCK_CAPS           0x0200
#define ECORE_EVENT_LOCK_SHIFT          0x0300
#define ECORE_EVENT_MODIFIER_ALTGR      0x0400 /**< @since 1.7 */

#define DEVICE_MOUSE                    0

namespace Dali
{

namespace Internal
{

namespace Win32System
{
typedef enum
{
    ECORE_IMF_KEYBOARD_MODIFIER_NONE = 0,      /**< No active modifiers */
    ECORE_IMF_KEYBOARD_MODIFIER_CTRL = 1 << 0, /**< "Control" is pressed */
    ECORE_IMF_KEYBOARD_MODIFIER_ALT = 1 << 1, /**< "Alt" is pressed */
    ECORE_IMF_KEYBOARD_MODIFIER_SHIFT = 1 << 2, /**< "Shift" is pressed */
    ECORE_IMF_KEYBOARD_MODIFIER_WIN = 1 << 3, /**< "Win" (between "Ctrl" and "Alt") is pressed */
    ECORE_IMF_KEYBOARD_MODIFIER_ALTGR = 1 << 4  /**< "AltGr" is pressed @since 1.7 */
} Ecore_IMF_Keyboard_Modifiers;

struct _Ecore_Event_Key
{
    const char      *keyname; /**< The key name */
    const char      *key; /**< The key symbol */
    const char      *string;
    const char      *compose; /**< final string corresponding to the key symbol composed */
    Ecore_Window     window; /**< The main window where event happened */
    Ecore_Window     root_window; /**< The root window where event happened */
    Ecore_Window     event_window; /**< The child window where event happened */

    unsigned int     timestamp; /**< Time when the event occurred */

    //int              same_screen; /**< same screen flag */

    unsigned int     keycode; /**< Key scan code numeric value @since 1.10 */

    void            *data; /**< User data associated with an Ecore_Event_Key @since 1.10 */
};

/**
* @struct _Ecore_Event_Mouse_Button
* Contains information about an Ecore mouse button event.
*/

struct _Ecore_Event_Mouse_Button
{
    Ecore_Window     window; /**< The main window where event happened */

    unsigned int     timestamp; /**< Time when the event occurred */
    unsigned int     buttons; /**< The button that was used */

    int              x; /**< x coordinate relative to window where event happened */
    int              y; /**< y coordinate relative to window where event happened */

    struct
    {
        int           device; /**< 0 if normal mouse, 1+ for other mouse-devices (eg multi-touch - other fingers) */
        double        radius, radius_x, radius_y; /**< radius of press point - radius_x and y if its an ellipse (radius is the average of the 2) */
        double        pressure; /**< pressure - 1.0 == normal, > 1.0 == more, 0.0 == none */
        double        angle; /**< angle relative to perpendicular (0.0 == perpendicular), in degrees */
        double        x, y; /**< same as x, y, but with sub-pixel precision, if available */
        struct
        {
            double     x, y;
        } root; /**< same as root.x, root.y, but with sub-pixel precision, if available */
    } multi;
};

/**
* @struct _Ecore_Event_Mouse_Wheel
* Contains information about an Ecore mouse wheel event.
*/
struct _Ecore_Event_Mouse_Wheel
{
    Ecore_Window     window; /**< The main window where event happened */
    Ecore_Window     root_window; /**< The root window where event happened */
    Ecore_Window     event_window; /**< The child window where event happened */

    unsigned int     timestamp; /**< Time when the event occurred */
    unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/

    int              direction; /**< Orientation of the wheel (horizontal/vertical) */
    int              z; /**< Value of the wheel event (+1/-1) */

    int              x; /**< x coordinate relative to window where event happened */
    int              y; /**< y coordinate relative to window where event happened */
    struct
    {
        int           x;
        int           y;
    } root; /**< Coordinates relative to root window */
};

/**
* @struct _Ecore_Event_Mouse_Move
* Contains information about an Ecore mouse move event.
*/
struct _Ecore_Event_Mouse_Move
{
    Ecore_Window     window; /**< The main window where event happened */
    Ecore_Window     root_window; /**< The root window where event happened */
    Ecore_Window     event_window; /**< The child window where event happened */

    unsigned int     timestamp; /**< Time when the event occurred */
    unsigned int     modifiers; /**< The combination of modifiers key (SHIFT,CTRL,ALT,..)*/

    int              x; /**< x coordinate relative to window where event happened */
    int              y; /**< y coordinate relative to window where event happened */
    struct
    {
        int           x;
        int           y;
    } root; /**< Coordinates relative to root window */

    struct
    {
        int           device; /**< 0 if normal mouse, 1+ for other mouse-devices (eg multi-touch - other fingers) */
        double        radius, radius_x, radius_y; /**< radius of press point - radius_x and y if its an ellipse (radius is the average of the 2) */
        double        pressure; /**< pressure - 1.0 == normal, > 1.0 == more, 0.0 == none */
        double        angle; /**< angle relative to perpendicular (0.0 == perpendicular), in degrees */
        double        x, y; /**< same as x, y root.x, root.y, but with sub-pixel precision, if available */
        struct
        {
            double     x, y;
        } root;
    } multi;
};

typedef enum _Ecore_Win_Event_Mode
{
    ECORE_WIN_EVENT_MODE_NORMAL,
    ECORE_WIN_EVENT_MODE_WHILE_GRABBED,
    ECORE_WIN_EVENT_MODE_GRAB,
    ECORE_WIN_EVENT_MODE_UNGRAB
} Ecore_Win_Event_Mode;

typedef enum _Ecore_Win_Event_Detail
{
    ECORE_WIN_EVENT_DETAIL_ANCESTOR,
    ECORE_WIN_EVENT_DETAIL_VIRTUAL,
    ECORE_WIN_EVENT_DETAIL_INFERIOR,
    ECORE_WIN_EVENT_DETAIL_NON_LINEAR,
    ECORE_WIN_EVENT_DETAIL_NON_LINEAR_VIRTUAL,
    ECORE_WIN_EVENT_DETAIL_POINTER,
    ECORE_WIN_EVENT_DETAIL_POINTER_ROOT,
    ECORE_WIN_EVENT_DETAIL_DETAIL_NONE
} Ecore_Win_Event_Detail;

typedef unsigned int Ecore_Win_Time;

struct _Ecore_Win_Event_Mouse_In
{
    int                  modifiers;
    int                  x, y;

    struct
    {
        int x, y;
    } root;
    Ecore_Win_Window       win;
    Ecore_Win_Window       event_win;
    Ecore_Win_Window       root_win;
    Ecore_Win_Event_Mode   mode;
    Ecore_Win_Event_Detail detail;
    Ecore_Win_Time         time;
};

struct _Ecore_Win_Event_Mouse_Out
{
    int                  modifiers;
    int                  x, y;

    struct
    {
        int x, y;
    } root;
    Ecore_Win_Window       win;
    Ecore_Win_Window       event_win;
    Ecore_Win_Window       root_win;
    Ecore_Win_Event_Mode   mode;
    Ecore_Win_Event_Detail detail;
    Ecore_Win_Time         time;
};

struct _Ecore_Win_Event_Window_Focus_In
{
    Ecore_Win_Window       win;
};

struct _Ecore_Win_Event_Window_Focus_Out
{
    Ecore_Win_Window       win;
};

typedef struct _Ecore_Event_Key          Ecore_Event_Key;
typedef struct _Ecore_Event_Mouse_Button Ecore_Event_Mouse_Button;
typedef struct _Ecore_Event_Mouse_Wheel  Ecore_Event_Mouse_Wheel;
typedef struct _Ecore_Event_Mouse_Move   Ecore_Event_Mouse_Move;
typedef struct _Ecore_Win_Event_Mouse_In                     Ecore_Win_Event_Mouse_In;
typedef struct _Ecore_Win_Event_Mouse_Out                    Ecore_Win_Event_Mouse_Out;
typedef struct _Ecore_Win_Event_Window_Focus_In              Ecore_Win_Event_Window_Focus_In;
typedef struct _Ecore_Win_Event_Window_Focus_Out             Ecore_Win_Event_Window_Focus_Out;

typedef int Ecore_Event_Handler;

struct TWinEventInfo
{
    TWinEventInfo(long hWnd, unsigned int uMsg, long wParam, long lParam)
    {
        this->mWindow = (Ecore_Win_Window)hWnd;
        this->uMsg = uMsg;
        this->wParam = wParam;
        this->lParam = lParam;
    }

    Ecore_Win_Window mWindow;
    unsigned int uMsg;
    long wParam;
    long lParam;
};

typedef bool(*EventCallback)(void* data, int type, TWinEventInfo* event);

Ecore_Event_Handler ecore_event_handler_add(int eventType, EventCallback callback, void* handler);

void ecore_event_handler_del(Ecore_Event_Handler handler);

void* GetEventHandler(int eventType);
EventCallback GetCallback(int eventType);
}
}
}

#endif
