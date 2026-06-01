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

// CLASS HEADER
#include <dali/internal/window-system/tizen/tcore/display-connection-impl-tcore-wl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/vector-wrapper.h>
#include <dali/integration-api/debug.h>
#include <tbm_bufmgr.h>
#include <tbm_dummy_display.h>
#include <tizen-core-wl/tizen_core_wl_internal.h>
#include <tizen_core_wl.h>
#include <utility> ///< for std::pair

// INTERNAL INCLUDES
#include <dali/internal/window-system/tizen/display-connection-native-types.h>
#include <dali/internal/window-system/tizen/tcore/tizen-core-wl-display-util.h>

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

DisplayConnectionTcoreWl::DisplayConnectionTcoreWl()
: mDisplay(nullptr),
  mSurfaceType(Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
{
}

DisplayConnectionTcoreWl::~DisplayConnectionTcoreWl()
{
  if(mSurfaceType == Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
  {
    ReleaseTbmDummyDisplay(mDisplay);
    mDisplay = nullptr;
  }
}

Any DisplayConnectionTcoreWl::GetDisplay()
{
  return Any(mDisplay);
}

void DisplayConnectionTcoreWl::ConsumeEvents()
{
}

void DisplayConnectionTcoreWl::SetSurfaceType(Integration::RenderSurfaceInterface::Type type)
{
  mSurfaceType = type;

  if(mSurfaceType == Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
  {
    mDisplay = static_cast<wl_display*>(AcquireUniqueTbmDummyDisplay());
  }
  else
  {
    tizen_core_wl_display_h display = nullptr;
    if(TcoreWlAcquireDisplay(&display))
    {
      struct wl_display* wl = nullptr;
      if(tizen_core_wl_display_private_get_wl_display(display, &wl) == TIZEN_CORE_WL_ERROR_NONE)
      {
        mDisplay = wl;
      }
      else
      {
        DALI_LOG_ERROR("tizen_core_wl_display_private_get_wl_display failed\n");
      }
      TcoreWlReleaseDisplay(display);
    }
  }
}

Any DisplayConnectionTcoreWl::GetNativeGraphicsDisplay()
{
  std::unique_ptr<Any> nativeGraphicsDisplay = CastToNativeGraphicsType(mDisplay);
  if(!nativeGraphicsDisplay)
  {
    DALI_LOG_ERROR("Failed to cast native graphics display\n");
    return Any();
  }

  // Move out of the heap Any: copying can fail if the graphics Any holds non-copyable state (e.g. unique_ptr).
  return std::move(*nativeGraphicsDisplay);
}

} // namespace Dali::Internal::Adaptor
