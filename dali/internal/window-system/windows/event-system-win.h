#pragma once
#ifndef _WINDOWEVENTSYSTEM_H_
#define _WINDOWEVENTSYSTEM_H_

#define EVENT_MOUSE_BUTTON_DOWN   WM_LBUTTONDOWN
#define EVENT_MOUSE_BUTTON_UP     WM_LBUTTONUP
#define EVENT_MOUSE_MOVE          WM_MOUSEMOVE
#define EVENT_MOUSE_OUT           WM_MOUSELEAVE
#define EVENT_MOUSE_WHEEL         WM_MOUSEWHEEL

#define EVENT_KEY_DOWN            WM_KEYDOWN
#define EVENT_KEY_UP              WM_KEYUP

#define WIN_EVENT_WINDOW_FOCUS_IN   WM_SETFOCUS
#define WIN_EVENT_WINDOW_FOCUS_OUT  WM_KILLFOCUS

#define WIN_EVENT_WINDOW_DAMAGE     WM_PAINT

#define WIN_EVENT_XDND_ENTER        0
#define WIN_EVENT_XDND_POSITION     0
#define WIN_EVENT_XDND_LEAVE        0
#define WIN_EVENT_XDND_DROP         0

#define WIN_EVENT_XDND_FINISHED     0
#define WIN_EVENT_XDND_STATUS       0

#define WIN_EVENT_CLIENT_MESSAGE    0
#define WIN_EVENT_SELECTION_CLEAR   0
#define WIN_EVENT_SELECTION_NOTIFY  0

#define CALLBACK_PASS_ON true;

#define EVENT_MODIFIER_SHIFT      0x0001
#define EVENT_MODIFIER_CTRL       0x0002
#define EVENT_MODIFIER_ALT        0x0004
#define EVENT_MODIFIER_WIN        0x0008
#define EVENT_MODIFIER_SCROLL     0x0010
#define EVENT_MODIFIER_NUM        0x0020
#define EVENT_MODIFIER_CAPS       0x0040
#define EVENT_LOCK_SCROLL         0x0080
#define EVENT_LOCK_NUM            0x0100
#define EVENT_LOCK_CAPS           0x0200
#define EVENT_LOCK_SHIFT          0x0300
#define EVENT_MODIFIER_ALTGR      0x0400 /**< @since 1.7 */

#define DEVICE_MOUSE                    0

// INTERNAL INCLUDES
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

typedef enum
{
    IMF_KEYBOARD_MODIFIER_NONE = 0,      /**< No active modifiers */
    IMF_KEYBOARD_MODIFIER_CTRL = 1 << 0, /**< "Control" is pressed */
    IMF_KEYBOARD_MODIFIER_ALT = 1 << 1, /**< "Alt" is pressed */
    IMF_KEYBOARD_MODIFIER_SHIFT = 1 << 2, /**< "Shift" is pressed */
    IMF_KEYBOARD_MODIFIER_WIN = 1 << 3, /**< "Win" (between "Ctrl" and "Alt") is pressed */
    IMF_KEYBOARD_MODIFIER_ALTGR = 1 << 4  /**< "AltGr" is pressed @since 1.7 */
} IMF_Keyboard_Modifiers;

struct _Event_Key
{
    const char      *keyname; /**< The key name */
    const char      *key; /**< The key symbol */
    const char      *string;
    const char      *compose; /**< final string corresponding to the key symbol composed */
    winWindow       window; /**< The main window where event happened */
    winWindow       root_window; /**< The root window where event happened */
    winWindow       event_window; /**< The child window where event happened */

    unsigned int    timestamp; /**< Time when the event occurred */

    unsigned int    keycode; /**< Key scan code numeric value @since 1.10 */

    void            *data; /**< User data associated with an Event_Key @since 1.10 */
};

/**
* @struct _Event_Mouse_Button
* Contains information about an Ecore mouse button event.
*/

struct _Event_Mouse_Button
{
    winWindow     window; /**< The main window where event happened */

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
* @struct _Event_Mouse_Wheel
* Contains information about an Ecore mouse wheel event.
*/
struct _Event_Mouse_Wheel
{
    winWindow     window; /**< The main window where event happened */
    winWindow     root_window; /**< The root window where event happened */
    winWindow     event_window; /**< The child window where event happened */

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
* @struct _Event_Mouse_Move
* Contains information about an Ecore mouse move event.
*/
struct _Event_Mouse_Move
{
    winWindow     window; /**< The main window where event happened */
    winWindow     root_window; /**< The root window where event happened */
    winWindow     event_window; /**< The child window where event happened */

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

typedef enum _Win_Event_Mode
{
    WIN_EVENT_MODE_NORMAL,
    WIN_EVENT_MODE_WHILE_GRABBED,
    WIN_EVENT_MODE_GRAB,
    WIN_EVENT_MODE_UNGRAB
} Win_Event_Mode;

typedef enum _Win_Event_Detail
{
    WIN_EVENT_DETAIL_ANCESTOR,
    WIN_EVENT_DETAIL_VIRTUAL,
    WIN_EVENT_DETAIL_INFERIOR,
    WIN_EVENT_DETAIL_NON_LINEAR,
    WIN_EVENT_DETAIL_NON_LINEAR_VIRTUAL,
    WIN_EVENT_DETAIL_POINTER,
    WIN_EVENT_DETAIL_POINTER_ROOT,
    WIN_EVENT_DETAIL_DETAIL_NONE
} Win_Event_Detail;

typedef unsigned int Win_Time;

struct _Win_Event_Mouse_In
{
    int                  modifiers;
    int                  x, y;

    struct
    {
        int x, y;
    } root;
    winWindow       win;
    winWindow       event_win;
    winWindow       root_win;
    Win_Event_Mode   mode;
    Win_Event_Detail detail;
    Win_Time         time;
};

struct _Win_Event_Mouse_Out
{
    int                  modifiers;
    int                  x, y;

    struct
    {
        int x, y;
    } root;
    winWindow       win;
    winWindow       event_win;
    winWindow       root_win;
    Win_Event_Mode   mode;
    Win_Event_Detail detail;
    Win_Time         time;
};

struct _Win_Event_Window_Focus_In
{
    winWindow       win;
};

struct _Win_Event_Window_Focus_Out
{
    winWindow       win;
};

typedef struct _Event_Key          Event_Key;
typedef struct _Event_Mouse_Button Event_Mouse_Button;
typedef struct _Event_Mouse_Wheel  Event_Mouse_Wheel;
typedef struct _Event_Mouse_Move   Event_Mouse_Move;
typedef struct _Win_Event_Mouse_In                     Win_Event_Mouse_In;
typedef struct _Win_Event_Mouse_Out                    Win_Event_Mouse_Out;
typedef struct _Win_Event_Window_Focus_In              Win_Event_Window_Focus_In;
typedef struct _Win_Event_Window_Focus_Out             Win_Event_Window_Focus_Out;

typedef int Event_Handler;

struct TWinEventInfo
{
    TWinEventInfo(long hWnd, unsigned int uMsg, long wParam, long lParam)
    {
        this->mWindow = (winWindow)hWnd;
        this->uMsg = uMsg;
        this->wParam = wParam;
        this->lParam = lParam;
    }

    winWindow mWindow;
    unsigned int uMsg;
    long wParam;
    long lParam;
};

typedef bool(*EventCallback)(void* data, int type, TWinEventInfo* event);

Event_Handler event_handler_add(int eventType, EventCallback callback, void* handler);

void event_handler_del(Event_Handler handler);

void* GetEventHandler(int eventType);
EventCallback GetCallback(int eventType);

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif
