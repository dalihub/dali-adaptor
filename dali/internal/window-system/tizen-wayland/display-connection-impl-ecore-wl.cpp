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

// CLASS HEADER
#include <dali/internal/window-system/tizen-wayland/display-connection-impl-ecore-wl.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <tbm_bufmgr.h>
#include <tbm_dummy_display.h>
#include <utility> ///< for std::pair

// INTERNAL HEADERS
#include <dali/internal/window-system/tizen-wayland/display-connection-native-types.h>

namespace Dali::Internal::Adaptor
{
namespace
{
/**
 * @brief Helper class to make we keep NativeDisplay for NativeRenderSurface as unique pointer
 * even if Application terminated and restart again.
 *
 * @note Follow by eglSpec, eglGetDipslay() creates new EGLDisplay per each input paramater,
 * and never be deleted until process terminated. But we can re-create DisplayConnecter multiple times
 * when we are using OffscreenApplication.
 * So, we need to keep dummy NativeDisplay pointer to avoid creating multiple EGLDisplay.
 */
struct NativeRenderSurfaceDisplayHolder
{
  NativeRenderSurfaceDisplayHolder()
  : mBufMgr(nullptr),
    mDisplayList()
  {
    Initialize();
  }

  ~NativeRenderSurfaceDisplayHolder()
  {
    Destroy();
  }

  void Initialize()
  {
    mBufMgr = tbm_bufmgr_init(-1); // -1 is meaningless. The parameter in this function is deprecated.
  }

  tbm_dummy_display* AcquireDisplay()
  {
    tbm_dummy_display* display = nullptr;
    if(mBufMgr)
    {
      for(auto& displayPair : mDisplayList)
      {
        if(!displayPair.second)
        {
          displayPair.second = true;
          DALI_LOG_DEBUG_INFO("Use created dummy display : %p (total dummy display : %zu)\n", displayPair.first, mDisplayList.size());
          return displayPair.first;
        }
      }

      // Create new dummy 1 display
      display = tbm_dummy_display_create();
      if(display)
      {
        mDisplayList.push_back({display, true});
        DALI_LOG_DEBUG_INFO("Create dummy display : %p (total dummy display : %zu)\n", display, mDisplayList.size());
      }
    }
    return display;
  }

  void ReleaseDisplay(tbm_dummy_display* display)
  {
    for(auto& displayPair : mDisplayList)
    {
      if(displayPair.first == display)
      {
        DALI_LOG_DEBUG_INFO("Release dummy display : %p (total dummy display : %zu)\n", display, mDisplayList.size());
        displayPair.second = false;
        break;
      }
    }
  }

  void Destroy()
  {
    for(auto& displayPair : mDisplayList)
    {
      tbm_dummy_display_destroy(displayPair.first);
    }
    mDisplayList.clear();

    if(mBufMgr)
    {
      tbm_bufmgr_deinit(mBufMgr);
      mBufMgr = nullptr;
    }
  }

  tbm_bufmgr mBufMgr; ///< For creating tbm_dummy_display

  std::vector<std::pair<tbm_dummy_display*, bool>> mDisplayList; ///< pair of {display, used}. Increase only.
};

static NativeRenderSurfaceDisplayHolder& GetDummyDisplayHolder()
{
  static NativeRenderSurfaceDisplayHolder gNativeRenderSurfaceDisplayHolder;
  return gNativeRenderSurfaceDisplayHolder;
}

static tbm_dummy_display* AcquireUniqueTbmDummyDisplay()
{
  auto& displayHolder = GetDummyDisplayHolder();
  if(displayHolder.mBufMgr == nullptr)
  {
    // Retry to initialize tbm bufmgr
    displayHolder.Destroy();
    displayHolder.Initialize();
    if(displayHolder.mBufMgr == nullptr)
    {
      DALI_LOG_ERROR("Fail to init tbm buf mgr\n");
      return nullptr;
    }
  }
  return displayHolder.AcquireDisplay();
}

static void ReleaseTbmDummyDisplay(tbm_dummy_display* display)
{
  auto& displayHolder = GetDummyDisplayHolder();
  if(displayHolder.mBufMgr == nullptr)
  {
    return;
  }
  return displayHolder.ReleaseDisplay(display);
}
} // namespace

DisplayConnectionEcoreWl::DisplayConnectionEcoreWl()
: mDisplay(nullptr),
  mSurfaceType(Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
{
}

DisplayConnectionEcoreWl::~DisplayConnectionEcoreWl()
{
  if(mSurfaceType == Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
  {
    ReleaseTbmDummyDisplay(mDisplay);
    mDisplay = nullptr;
  }
}

Any DisplayConnectionEcoreWl::GetDisplay()
{
  return Any(mDisplay);
}

void DisplayConnectionEcoreWl::ConsumeEvents()
{
}

void DisplayConnectionEcoreWl::SetSurfaceType(Integration::RenderSurfaceInterface::Type type)
{
  mSurfaceType = type;

  if(mSurfaceType == Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
  {
    mDisplay = static_cast<wl_display*>(AcquireUniqueTbmDummyDisplay());
  }
  else
  {
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(nullptr);
    mDisplay                   = ecore_wl2_display_get(display);
  }
}

Any DisplayConnectionEcoreWl::GetNativeGraphicsDisplay()
{
  return CastToNativeGraphicsType(mDisplay);
}

} // namespace Dali::Internal::Adaptor
