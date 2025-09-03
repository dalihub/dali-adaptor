/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <Ecore_Wl2.h>
#include <dali/devel-api/adaptor-framework/screen-information.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>

#include <dlog.h>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/keyboard.h>
#include <dali/internal/adaptor/common/framework-factory.h>
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
static int32_t                              gScreenWidth     = 0;
static int32_t                              gScreenHeight    = 0;
static bool                                 gGeometryHittest = false;
static bool                                 gIsIntialized    = false;
static std::vector<Dali::ScreenInformation> gScreenList;
} // unnamed namespace

bool EcoreInitialize()
{
  if(gIsIntialized == false)
  {
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "ecore_wl2_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
    if(!ecore_wl2_init())
    {
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to ecore_wl2_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
      return false;
    }
    gIsIntialized = true;
  }
  return true;
}

void EcoreShutdown()
{
  if(gIsIntialized)
  {
    print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "ecore_wl2_shutdown()", DALI_LOG_FORMAT_PREFIX_ARGS);
    ecore_wl2_shutdown();
    gIsIntialized = false;
  }
}

void Initialize()
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    EcoreInitialize();
  }
}

void Shutdown()
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    EcoreShutdown();
  }
}

void GetScreenSize(int32_t& width, int32_t& height)
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    if(gScreenWidth == 0 || gScreenHeight == 0)
    {
      if(!EcoreInitialize())
      {
        width  = 0;
        height = 0;
        return;
      }

      Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
      if(display)
      {
        START_DURATION_CHECK();
        ecore_wl2_display_screen_size_get(display, &gScreenWidth, &gScreenHeight);
        FINISH_DURATION_CHECK("ecore_wl2_display_screen_size_get");

        // Since dali adaptor doesn't initialize this time, we should use dlog.
        print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() for display(%p) return %d x %d", DALI_LOG_FORMAT_PREFIX_ARGS, display, gScreenWidth, gScreenHeight);

        DALI_ASSERT_ALWAYS((gScreenWidth > 0) && "screen width is 0");
        DALI_ASSERT_ALWAYS((gScreenHeight > 0) && "screen height is 0");
      }
      else
      {
        // Since dali adaptor doesn't initialize this time, we should use dlog.
        print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() but display is null", DALI_LOG_FORMAT_PREFIX_ARGS);
      }
    }
  }
  width  = gScreenWidth;
  height = gScreenHeight;
}

std::vector<Dali::ScreenInformation> GetAvailableScreens()
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    if(!EcoreInitialize())
    {
      print_log(DLOG_ERROR, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to ecore_init()", DALI_LOG_FORMAT_PREFIX_ARGS);
      gScreenList.clear();
      return gScreenList;
    }

    Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
    if(!display)
    {
      print_log(DLOG_ERROR, "DALI", DALI_LOG_FORMAT_PREFIX "Fail to ecore_wl2_display_connect()", DALI_LOG_FORMAT_PREFIX_ARGS);
      gScreenList.clear();
      return gScreenList;
    }

    if(gScreenList.size() == 0)
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
          int   width, height;
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
          gScreenList.push_back(Dali::ScreenInformation{std::string(ecoreScreenName), width, height});
        }
      }
    }
  }
  print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "Update Screen List:%d", DALI_LOG_FORMAT_PREFIX_ARGS, gScreenList.size());
  return gScreenList;
}

void UpdateScreenSize()
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
    if(display)
    {
      START_DURATION_CHECK();
      ecore_wl2_display_screen_size_get(display, &gScreenWidth, &gScreenHeight);
      FINISH_DURATION_CHECK("ecore_wl2_display_screen_size_get");

      // Since dali adaptor doesn't initialize this time, we should use dlog.
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() for display(%p) return %d x %d", DALI_LOG_FORMAT_PREFIX_ARGS, display, gScreenWidth, gScreenHeight);
    }
    else
    {
      // Since dali adaptor doesn't initialize this time, we should use dlog.
      print_log(DLOG_INFO, "DALI", DALI_LOG_FORMAT_PREFIX "GetScreenSize() but display is null", DALI_LOG_FORMAT_PREFIX_ARGS);
    }
  }
}

bool SetKeyboardRepeatInfo(float rate, float delay)
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    return ecore_wl2_input_keyboard_repeat_set(input, static_cast<double>(rate), static_cast<double>(delay));
  }
  return false;
}

bool GetKeyboardRepeatInfo(float& rate, float& delay)
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    double           rateVal, delayVal;
    bool             ret = ecore_wl2_input_keyboard_repeat_get(input, &rateVal, &delayVal);
    rate                 = static_cast<float>(rateVal);
    delay                = static_cast<float>(delayVal);

    return ret;
  }
  return false;
}

bool SetKeyboardHorizontalRepeatInfo(float rate, float delay)
{
#ifdef OVER_TIZEN_VERSION_8
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    return ecore_wl2_input_keyboard_horizontal_way_repeat_set(input, static_cast<double>(rate), static_cast<double>(delay));
  }
  return false;
#else
  return SetKeyboardRepeatInfo(rate, delay);
#endif
}

bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay)
{
#ifdef OVER_TIZEN_VERSION_8
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    double           rateVal, delayVal;
    bool             ret = ecore_wl2_input_keyboard_horizontal_way_repeat_get(input, &rateVal, &delayVal);
    rate                 = static_cast<float>(rateVal);
    delay                = static_cast<float>(delayVal);

    return ret;
  }
  return false;
#else
  return GetKeyboardRepeatInfo(rate, delay);
#endif
}

bool SetKeyboardVerticalRepeatInfo(float rate, float delay)
{
#ifdef OVER_TIZEN_VERSION_8
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    return ecore_wl2_input_keyboard_vertical_way_repeat_set(input, static_cast<double>(rate), static_cast<double>(delay));
  }
  return false;
#else
  return SetKeyboardRepeatInfo(rate, delay);
#endif
}

bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay)
{
#ifdef OVER_TIZEN_VERSION_8
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory == nullptr || (frameworkFactory && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::DEFAULT))
  {
    Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(ecore_wl2_connected_display_get(NULL));
    double           rateVal, delayVal;
    bool             ret = ecore_wl2_input_keyboard_vertical_way_repeat_get(input, &rateVal, &delayVal);
    rate                 = static_cast<float>(rateVal);
    delay                = static_cast<float>(delayVal);

    return ret;
  }
  return false;
#else
  return GetKeyboardRepeatInfo(rate, delay);
#endif
}

void SetGeometryHittestEnabled(bool enable)
{
  DALI_LOG_RELEASE_INFO("GeometryHittest : %d \n", enable);
  if(gGeometryHittest != enable && Dali::Adaptor::IsAvailable())
  {
    Dali::SceneHolderList sceneHolders = Dali::Adaptor::Get().GetSceneHolders();
    for(auto iter = sceneHolders.begin(); iter != sceneHolders.end(); ++iter)
    {
      if(*iter)
      {
        (*iter).SetGeometryHittestEnabled(enable);
      }
    }
  }
  gGeometryHittest = enable;
}

bool IsGeometryHittestEnabled()
{
  return gGeometryHittest;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
