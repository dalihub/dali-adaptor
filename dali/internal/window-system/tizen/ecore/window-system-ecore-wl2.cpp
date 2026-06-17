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

#include <Ecore_Wl2.h>
#include <dlog.h>

#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/window-system/common/window-system.h>

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
class WindowSystemEcoreWl2 : public WindowSystemBase
{
public:
  WindowSystemEcoreWl2()
  {
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "ecore_wl2_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
    if(!ecore_wl2_init())
    {
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to ecore_wl2_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
    }
    mKeyboardRepeatEventHandler = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED, OnKeyboardRepeatChanged, this);
  }

  ~WindowSystemEcoreWl2()
  {
    if(mKeyboardRepeatEventHandler)
    {
      ecore_event_handler_del(mKeyboardRepeatEventHandler);
    }
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "ecore_wl2_shutdown()", DALI_LOG_FORMAT_PREFIX_ARGS);
    ecore_wl2_shutdown();
  }

  void GetScreenSize(int32_t& width, int32_t& height) override
  {
    if(mScreenWidth == 0 || mScreenHeight == 0)
    {
      Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
      if(display)
      {
        START_DURATION_CHECK();
        ecore_wl2_display_screen_size_get(display, &mScreenWidth, &mScreenHeight);
        FINISH_DURATION_CHECK("ecore_wl2_display_screen_size_get");

        print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() for display(%p) return %d x %d", DALI_LOG_FORMAT_PREFIX_ARGS, display, mScreenWidth, mScreenHeight);

        DALI_ASSERT_ALWAYS((mScreenWidth > 0) && "screen width is 0");
        DALI_ASSERT_ALWAYS((mScreenHeight > 0) && "screen height is 0");
      }
      else
      {
        print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() but display is null", DALI_LOG_FORMAT_PREFIX_ARGS);
      }
    }
    width  = mScreenWidth;
    height = mScreenHeight;
  }

  void UpdateScreenSize() override
  {
    Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
    if(display)
    {
      START_DURATION_CHECK();
      ecore_wl2_display_screen_size_get(display, &mScreenWidth, &mScreenHeight);
      FINISH_DURATION_CHECK("ecore_wl2_display_screen_size_get");

      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() for display(%p) return %d x %d", DALI_LOG_FORMAT_PREFIX_ARGS, display, mScreenWidth, mScreenHeight);
    }
    else
    {
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() but display is null", DALI_LOG_FORMAT_PREFIX_ARGS);
    }
  }

  std::vector<Dali::ScreenInformation> GetAvailableScreens() override
  {
#ifdef OVER_TIZEN_VERSION_10
    Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
    if(!display)
    {
      print_log(DLOG_ERROR, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to ecore_wl2_display_connect()", DALI_LOG_FORMAT_PREFIX_ARGS);
      mScreenList.clear();
      return mScreenList;
    }

    if(mScreenList.size() == 0)
    {
      Eina_List* ecoreScreenList = nullptr;
      Eina_List* l               = nullptr;
      void*      screen          = nullptr;

      START_DURATION_CHECK();
      ecoreScreenList = ecore_wl2_display_screens_get(display);
      FINISH_DURATION_CHECK("ecore_wl2_display_screens_get");
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "try to get Screens Information: %p", DALI_LOG_FORMAT_PREFIX_ARGS, ecoreScreenList);
      if(ecoreScreenList)
      {
        EINA_LIST_FOREACH(ecoreScreenList, l, (screen))
        {
          int         width, height;
          const char* ecoreScreenName = ecore_wl2_screen_name_get(static_cast<Ecore_Wl2_Screen*>(screen));
          if(!ecoreScreenName)
          {
            print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "screen(%p) name is empty", DALI_LOG_FORMAT_PREFIX_ARGS, screen);
            continue;
          }
          print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Get screen(%p) name: %s", DALI_LOG_FORMAT_PREFIX_ARGS, screen, ecoreScreenName);

          ecore_wl2_screen_size_get(static_cast<Ecore_Wl2_Screen*>(screen), &width, &height);
          if(width == 0 || height == 0)
          {
            print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "screen(%p) size 0, width(%d), height(%d) ", DALI_LOG_FORMAT_PREFIX_ARGS, screen, width, height);
            continue;
          }

          print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Get screen(%p) size(%d x %d)", DALI_LOG_FORMAT_PREFIX_ARGS, screen, width, height);
          mScreenList.push_back(Dali::ScreenInformation{std::string(ecoreScreenName), width, height});
        }
      }
    }
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Update Screen List:%zu", DALI_LOG_FORMAT_PREFIX_ARGS, mScreenList.size());
#else
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Not supported platform version", DALI_LOG_FORMAT_PREFIX_ARGS);
#endif
    return mScreenList;
  }

  bool SetKeyboardRepeatInfo(float rate, float delay) override
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    return ecore_wl2_input_keyboard_repeat_set(input, static_cast<double>(rate), static_cast<double>(delay));
  }

  bool GetKeyboardRepeatInfo(float& rate, float& delay) override
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    double           rateVal, delayVal;
    bool             ret = ecore_wl2_input_keyboard_repeat_get(input, &rateVal, &delayVal);
    rate                 = static_cast<float>(rateVal);
    delay                = static_cast<float>(delayVal);
    return ret;
  }

  bool SetKeyboardHorizontalRepeatInfo(float rate, float delay) override
  {
#ifdef OVER_TIZEN_VERSION_8
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    return ecore_wl2_input_keyboard_horizontal_way_repeat_set(input, static_cast<double>(rate), static_cast<double>(delay));
#else
    return SetKeyboardRepeatInfo(rate, delay);
#endif
  }

  bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay) override
  {
#ifdef OVER_TIZEN_VERSION_8
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    double           rateVal, delayVal;
    bool             ret = ecore_wl2_input_keyboard_horizontal_way_repeat_get(input, &rateVal, &delayVal);
    rate                 = static_cast<float>(rateVal);
    delay                = static_cast<float>(delayVal);
    return ret;
#else
    return GetKeyboardRepeatInfo(rate, delay);
#endif
  }

  bool SetKeyboardVerticalRepeatInfo(float rate, float delay) override
  {
#ifdef OVER_TIZEN_VERSION_8
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    return ecore_wl2_input_keyboard_vertical_way_repeat_set(input, static_cast<double>(rate), static_cast<double>(delay));
#else
    return SetKeyboardRepeatInfo(rate, delay);
#endif
  }

  bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay) override
  {
#ifdef OVER_TIZEN_VERSION_8
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    double           rateVal, delayVal;
    bool             ret = ecore_wl2_input_keyboard_vertical_way_repeat_get(input, &rateVal, &delayVal);
    rate                 = static_cast<float>(rateVal);
    delay                = static_cast<float>(delayVal);
    return ret;
#else
    return GetKeyboardRepeatInfo(rate, delay);
#endif
  }

private:
  static Eina_Bool OnKeyboardRepeatChanged(void* data, int type, void* event)
  {
    if(Dali::Adaptor::IsAvailable())
    {
      static_cast<WindowSystemEcoreWl2*>(data)->mKeyboardRepeatSettingsChangedSignal.Emit();
    }
    return ECORE_CALLBACK_RENEW;
  }

  int32_t                              mScreenWidth{0};
  int32_t                              mScreenHeight{0};
  std::vector<Dali::ScreenInformation> mScreenList;
  Ecore_Event_Handler*                 mKeyboardRepeatEventHandler{nullptr};
};

std::unique_ptr<WindowSystemEcoreWl2> gWindowSystem;

WindowSystemEcoreWl2& GetImpl()
{
  if(!gWindowSystem)
  {
    gWindowSystem = std::make_unique<WindowSystemEcoreWl2>();
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
