/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/screen-information.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>

#include <dlog.h>
#include <tizen_core_wl.h>

#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/tizen/tcore/tizen-core-wl-display-util.h>

#define START_DURATION_CHECK()                               \
  uint32_t durationMilliSeconds = static_cast<uint32_t>(-1); \
  uint32_t startTime, endTime;                               \
  startTime = TimeService::GetMilliSeconds();

// Since dali adaptor doesn't initialize this time, we should use dlog.
#define FINISH_DURATION_CHECK(functionName)                                                                                                           \
  endTime              = TimeService::GetMilliSeconds();                                                                                              \
  durationMilliSeconds = endTime - startTime;                                                                                                         \
  if(durationMilliSeconds > 0)                                                                                                                        \
  {                                                                                                                                                   \
    print_log(DLOG_DEBUG, "DALI", DALI_LOG_FORMAT_PREFIX "%s : duration [%u ms]\n", DALI_LOG_FORMAT_PREFIX_ARGS, functionName, durationMilliSeconds); \
  }

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
namespace
{
/**
 * @brief Connects to the tizen-core-wl display and retrieves the preferred screen size.
 *
 * Creates a temporary display connection, queries the preferred screen geometry via
 * tizen_core_wl_display_get_preferred_screen() and then cleans up the connection.
 *
 * @param[out] width  The preferred screen width (unchanged on failure).
 * @param[out] height The preferred screen height (unchanged on failure).
 * @return true if a valid (w > 0, h > 0) geometry was obtained, false otherwise.
 */
static bool FetchPreferredScreenSize(int32_t& width, int32_t& height)
{
  tizen_core_wl_display_h display     = nullptr;
  bool                    ownsDisplay = false;

  // tizen-core-wl allows only one connection per process. If AppCoreUiBase
  // (or any other component) already connected to the compositor, display_connect()
  // reuses the cached connection instead of creating a new Wayland connection.
  if(!TcoreWlAcquireDisplay(&display))
  {
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "FetchPreferredScreenSize() but display connect failed", DALI_LOG_FORMAT_PREFIX_ARGS);
    return false;
  }
  ownsDisplay = true;

  bool result = false;
  START_DURATION_CHECK();

  tizen_core_wl_screen_h screen = nullptr;
  if(tizen_core_wl_display_get_preferred_screen(display, &screen) == TIZEN_CORE_WL_ERROR_NONE)
  {
    if(screen)
    {
      int x = 0, y = 0, w = 0, h = 0;
      if(tizen_core_wl_screen_get_geometry(screen, &x, &y, &w, &h) == TIZEN_CORE_WL_ERROR_NONE && w > 0 && h > 0)
      {
        width  = w;
        height = h;
        result = true;
        DALI_LOG_RELEASE_INFO("Get Default Screen() screen size %d x %d", w, h);
      }
    }
    else
    {
      DALI_LOG_RELEASE_INFO("Get Default Screen() but screen is null", DALI_LOG_FORMAT_PREFIX_ARGS);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Get Default Screen() but display is null", DALI_LOG_FORMAT_PREFIX_ARGS);

    GList* list = NULL;
    if(tizen_core_wl_display_get_output_device_list(display, &list) == TIZEN_CORE_WL_ERROR_NONE && list)
    {
      GList* l;
      int    idx = 0;
      for(l = list; l != NULL; l = l->next)
      {
        tizen_core_wl_output_h output = (tizen_core_wl_output_h)l->data;
        int                    w = 0, h = 0;
        if(tizen_core_wl_output_device_get_geometry(output, &w, &h) == TIZEN_CORE_WL_ERROR_NONE)
        {
          DALI_LOG_RELEASE_INFO("tizen_core_wl_output_device_get_geometry %d x %d\n", w, h);
          width  = w;
          height = h;
          break;
        }
        else
        {
          DALI_LOG_RELEASE_INFO("Output %d: Failed to get output information\n", idx);
        }
        idx++;
      }
      g_list_free(list);
    }
  }

  FINISH_DURATION_CHECK("tizen_core_wl_screen_get_geometry");

  print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "FetchPreferredScreenSize() for display(%p) return %d x %d", DALI_LOG_FORMAT_PREFIX_ARGS, static_cast<void*>(display), width, height);

  if(ownsDisplay)
  {
    tizen_core_wl_display_disconnect(display);
    tizen_core_wl_display_destroy(display);
  }
  return result;
}

static void ReleaseDisplay(tizen_core_wl_display_h display, bool ownsDisplay)
{
  if(ownsDisplay)
  {
    tizen_core_wl_display_disconnect(display);
    tizen_core_wl_display_destroy(display);
  }
}

static bool GetDefaultSeat(tizen_core_wl_display_h& display, tizen_core_wl_seat_h& seat, bool& ownsDisplay)
{
  display     = nullptr;
  seat        = nullptr;
  ownsDisplay = false;

  if(!TcoreWlAcquireDisplay(&display))
  {
    return false;
  }
  ownsDisplay = true;

  if(tizen_core_wl_display_get_default_seat(display, &seat) != TIZEN_CORE_WL_ERROR_NONE || !seat)
  {
    ReleaseDisplay(display, ownsDisplay);
    display     = nullptr;
    ownsDisplay = false;
    return false;
  }

  return true;
}

class WindowSystemTcoreWl : public WindowSystemBase
{
public:
  WindowSystemTcoreWl()
  {
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "tizen_core_wl_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
    if(tizen_core_wl_init() != TIZEN_CORE_WL_ERROR_NONE)
    {
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to tizen_core_wl_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
      return;
    }

    if(!TcoreWlAcquireDisplay(&mTcoreDisplay))
    {
      return;
    }

    tizen_core_event_h event = nullptr;
    if(tizen_core_wl_display_get_event(mTcoreDisplay, &event) == TIZEN_CORE_WL_ERROR_NONE && event)
    {
      tizen_core_wl_event_listener_h listener = nullptr;
      if(tizen_core_wl_event_add_listener(event, TIZEN_CORE_WL_EVENT_SEAT_KEYREPEAT_CHANGED, OnKeyboardRepeatChanged, this, &listener) == TIZEN_CORE_WL_ERROR_NONE && listener)
      {
        mKeyboardRepeatListener = listener;
        mTcoreEvent             = event;
      }
    }
  }

  ~WindowSystemTcoreWl()
  {
    if(mKeyboardRepeatListener && mTcoreEvent)
    {
      tizen_core_wl_event_remove_listener(mTcoreEvent, mKeyboardRepeatListener);
    }
    if(mTcoreDisplay)
    {
      TcoreWlReleaseDisplay(mTcoreDisplay);
    }
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "tizen_core_wl_shutdown()", DALI_LOG_FORMAT_PREFIX_ARGS);
    tizen_core_wl_shutdown();
  }

  void GetScreenSize(int32_t& width, int32_t& height) override
  {
    if(mScreenWidth == 0 || mScreenHeight == 0)
    {
      FetchPreferredScreenSize(mScreenWidth, mScreenHeight);
      DALI_ASSERT_ALWAYS((mScreenWidth > 0) && "screen width is 0");
      DALI_ASSERT_ALWAYS((mScreenHeight > 0) && "screen height is 0");
    }
    width  = mScreenWidth;
    height = mScreenHeight;
  }

  void UpdateScreenSize() override
  {
    FetchPreferredScreenSize(mScreenWidth, mScreenHeight);
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "UpdateScreenSize() return %d x %d", DALI_LOG_FORMAT_PREFIX_ARGS, mScreenWidth, mScreenHeight);
  }

  std::vector<Dali::ScreenInformation> GetAvailableScreens() override
  {
    if(mScreenList.empty())
    {
      tizen_core_wl_display_h display     = nullptr;
      bool                    ownsDisplay = false;

      if(!TcoreWlAcquireDisplay(&display))
      {
        print_log(DLOG_ERROR, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to connect tizen-core-wl display", DALI_LOG_FORMAT_PREFIX_ARGS);
        return mScreenList;
      }
      ownsDisplay = true;

      START_DURATION_CHECK();
      tizen_core_wl_screen_h* screens  = nullptr;
      int                     num_list = 0;
      tizen_core_wl_error_e   err      = tizen_core_wl_display_get_screen_list(display, &screens, &num_list);
      FINISH_DURATION_CHECK("tizen_core_wl_display_get_screen_list");
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "try to get Screens Information: %p, num=%d", DALI_LOG_FORMAT_PREFIX_ARGS, static_cast<void*>(screens), num_list);
      if(err == TIZEN_CORE_WL_ERROR_NONE && screens && num_list > 0)
      {
        for(int i = 0; i < num_list; ++i)
        {
          int x = 0, y = 0, w = 0, h = 0;
          if(tizen_core_wl_screen_get_geometry(screens[i], &x, &y, &w, &h) != TIZEN_CORE_WL_ERROR_NONE)
          {
            continue;
          }
          if(w == 0 || h == 0)
          {
            print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "screen[%d] size 0, width(%d), height(%d)", DALI_LOG_FORMAT_PREFIX_ARGS, i, w, h);
            continue;
          }
          char screenName[32];
          snprintf(screenName, sizeof(screenName), "Screen-%d", i);
          print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Get screen[%d] name: %s size(%d x %d)", DALI_LOG_FORMAT_PREFIX_ARGS, i, screenName, w, h);
          mScreenList.push_back(Dali::ScreenInformation{std::string(screenName), w, h});
        }
        free(screens);
      }
      if(ownsDisplay)
      {
        TcoreWlReleaseDisplay(display);
      }
    }
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Update Screen List:%zu", DALI_LOG_FORMAT_PREFIX_ARGS, mScreenList.size());
    return mScreenList;
  }

  bool SetKeyboardRepeatInfo(float rate, float delay) override
  {
    tizen_core_wl_display_h display     = nullptr;
    tizen_core_wl_seat_h    seat        = nullptr;
    bool                    ownsDisplay = false;
    if(!GetDefaultSeat(display, seat, ownsDisplay))
    {
      return false;
    }
    bool ret = tizen_core_wl_seat_set_keyboard_repeat(seat, TIZEN_CORE_WL_KEYBOARD_REPEAT_DEFAULT, static_cast<double>(rate), static_cast<double>(delay)) == TIZEN_CORE_WL_ERROR_NONE;
    ReleaseDisplay(display, ownsDisplay);
    return ret;
  }

  bool GetKeyboardRepeatInfo(float& rate, float& delay) override
  {
    tizen_core_wl_display_h display     = nullptr;
    tizen_core_wl_seat_h    seat        = nullptr;
    bool                    ownsDisplay = false;
    if(!GetDefaultSeat(display, seat, ownsDisplay))
    {
      return false;
    }
    double rateVal = 0.0, delayVal = 0.0;
    bool   ret = tizen_core_wl_seat_get_keyboard_repeat(seat, TIZEN_CORE_WL_KEYBOARD_REPEAT_DEFAULT, &rateVal, &delayVal) == TIZEN_CORE_WL_ERROR_NONE;
    ReleaseDisplay(display, ownsDisplay);
    rate  = static_cast<float>(rateVal);
    delay = static_cast<float>(delayVal);
    return ret;
  }

  bool SetKeyboardHorizontalRepeatInfo(float rate, float delay) override
  {
    tizen_core_wl_display_h display     = nullptr;
    tizen_core_wl_seat_h    seat        = nullptr;
    bool                    ownsDisplay = false;
    if(!GetDefaultSeat(display, seat, ownsDisplay))
    {
      return false;
    }
    bool ret = tizen_core_wl_seat_set_keyboard_repeat(seat, TIZEN_CORE_WL_KEYBOARD_REPEAT_HORIZONTAL, static_cast<double>(rate), static_cast<double>(delay)) == TIZEN_CORE_WL_ERROR_NONE;
    ReleaseDisplay(display, ownsDisplay);
    return ret;
  }

  bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay) override
  {
    tizen_core_wl_display_h display     = nullptr;
    tizen_core_wl_seat_h    seat        = nullptr;
    bool                    ownsDisplay = false;
    if(!GetDefaultSeat(display, seat, ownsDisplay))
    {
      return false;
    }
    double rateVal = 0.0, delayVal = 0.0;
    bool   ret = tizen_core_wl_seat_get_keyboard_repeat(seat, TIZEN_CORE_WL_KEYBOARD_REPEAT_HORIZONTAL, &rateVal, &delayVal) == TIZEN_CORE_WL_ERROR_NONE;
    ReleaseDisplay(display, ownsDisplay);
    rate  = static_cast<float>(rateVal);
    delay = static_cast<float>(delayVal);
    return ret;
  }

  bool SetKeyboardVerticalRepeatInfo(float rate, float delay) override
  {
    tizen_core_wl_display_h display     = nullptr;
    tizen_core_wl_seat_h    seat        = nullptr;
    bool                    ownsDisplay = false;
    if(!GetDefaultSeat(display, seat, ownsDisplay))
    {
      return false;
    }
    bool ret = tizen_core_wl_seat_set_keyboard_repeat(seat, TIZEN_CORE_WL_KEYBOARD_REPEAT_VERTICAL, static_cast<double>(rate), static_cast<double>(delay)) == TIZEN_CORE_WL_ERROR_NONE;
    ReleaseDisplay(display, ownsDisplay);
    return ret;
  }

  bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay) override
  {
    tizen_core_wl_display_h display     = nullptr;
    tizen_core_wl_seat_h    seat        = nullptr;
    bool                    ownsDisplay = false;
    if(!GetDefaultSeat(display, seat, ownsDisplay))
    {
      return false;
    }
    double rateVal = 0.0, delayVal = 0.0;
    bool   ret = tizen_core_wl_seat_get_keyboard_repeat(seat, TIZEN_CORE_WL_KEYBOARD_REPEAT_VERTICAL, &rateVal, &delayVal) == TIZEN_CORE_WL_ERROR_NONE;
    ReleaseDisplay(display, ownsDisplay);
    rate  = static_cast<float>(rateVal);
    delay = static_cast<float>(delayVal);
    return ret;
  }

private:
  static void OnKeyboardRepeatChanged(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
  {
    if(Dali::Adaptor::IsAvailable())
    {
      static_cast<WindowSystemTcoreWl*>(user_data)->mKeyboardRepeatSettingsChangedSignal.Emit();
    }
  }

  int32_t                              mScreenWidth{0};
  int32_t                              mScreenHeight{0};
  std::vector<Dali::ScreenInformation> mScreenList;
  tizen_core_wl_display_h              mTcoreDisplay{nullptr};
  tizen_core_event_h                   mTcoreEvent{nullptr};
  tizen_core_wl_event_listener_h       mKeyboardRepeatListener{nullptr};
};

std::unique_ptr<WindowSystemTcoreWl> gWindowSystem;

WindowSystemTcoreWl& GetImpl()
{
  if(!gWindowSystem)
  {
    gWindowSystem = std::make_unique<WindowSystemTcoreWl>();
  }
  return *gWindowSystem;
}

} // unnamed namespace

void Initialize()
{
  GetImpl(); // triggers singleton construction
}

void Shutdown()
{
  gWindowSystem.reset();
}

WindowSystemBase* GetWindowSystem()
{
  return &GetImpl();
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
