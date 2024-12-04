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

// CLASS HEADER
#include <dali/internal/window-system/tizen-wayland/display-connection-impl-ecore-wl.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>
#include <tbm_bufmgr.h>
#include <tbm_dummy_display.h>

#ifdef ECORE_WAYLAND2
#include <Ecore_Wl2.h>
#else
#include <Ecore_Wayland.h>
#endif

#if !defined(VULKAN_ENABLED)
#include <dali/internal/graphics/common/egl-include.h>
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
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
    mDisplay(nullptr)
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
    if(mBufMgr)
    {
      mDisplay = reinterpret_cast<NativeDisplayType>(tbm_dummy_display_create());
    }
  }
  void Destroy()
  {
#ifdef VULKAN_ENABLED
    if(!mDisplay.Empty())
    {
      // TODO: Fix this call for Vulkan
      //tbm_dummy_display_destroy(mDisplay.Get<tbm_dummy_display>());
    }
#else
    if(mDisplay)
    {
      tbm_dummy_display_destroy(reinterpret_cast<tbm_dummy_display*>(mDisplay));
    }
#endif
    if(mBufMgr)
    {
      tbm_bufmgr_deinit(mBufMgr);
    }
  }

  tbm_bufmgr mBufMgr; ///< For creating tbm_dummy_display

  NativeDisplayType mDisplay;
};

static NativeDisplayType GetUniqueTbmDummyDisplay()
{
  static NativeRenderSurfaceDisplayHolder sNativeRenderSurfaceDisplayHolder;
  if(sNativeRenderSurfaceDisplayHolder.mBufMgr == nullptr)
  {
    // Retry to initialize tbm bufmgr
    sNativeRenderSurfaceDisplayHolder.Destroy();
    sNativeRenderSurfaceDisplayHolder.Initialize();
    if(sNativeRenderSurfaceDisplayHolder.mBufMgr == nullptr)
    {
      DALI_LOG_ERROR("Fail to init tbm buf mgr\n");
      return nullptr;
    }
  }
  return sNativeRenderSurfaceDisplayHolder.mDisplay;
}
} // namespace

DisplayConnection* DisplayConnectionEcoreWl::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionEcoreWl());

  return pDisplayConnection;
}

DisplayConnectionEcoreWl::DisplayConnectionEcoreWl()
: mDisplay(NULL),
  mSurfaceType(Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
{
}

DisplayConnectionEcoreWl::~DisplayConnectionEcoreWl()
{
  if(mSurfaceType == Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
  {
    ReleaseNativeDisplay();
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
    mDisplay = GetNativeDisplay();
  }
  else
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    mDisplay                   = NativeDisplayType(ecore_wl2_display_get(display));
#else
    mDisplay = reinterpret_cast<NativeDisplayType>(ecore_wl_display_get());
#endif
  }
}

Any DisplayConnectionEcoreWl::GetNativeGraphicsDisplay()
{
  return Any(mDisplay);
}

NativeDisplayType DisplayConnectionEcoreWl::GetNativeDisplay()
{
  return GetUniqueTbmDummyDisplay();
}

void DisplayConnectionEcoreWl::ReleaseNativeDisplay()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
