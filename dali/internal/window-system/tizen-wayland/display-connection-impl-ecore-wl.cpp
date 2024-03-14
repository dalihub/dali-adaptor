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
#include <tbm_dummy_display.h>

#ifdef ECORE_WAYLAND2
#include <Ecore_Wl2.h>
#else
#include <Ecore_Wayland.h>
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
DisplayConnection* DisplayConnectionEcoreWl::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionEcoreWl());

  return pDisplayConnection;
}

DisplayConnectionEcoreWl::DisplayConnectionEcoreWl()
: mDisplay(NULL),
  mSurfaceType(RenderSurfaceInterface::WINDOW_RENDER_SURFACE),
  mBufMgr(nullptr)
{
}

DisplayConnectionEcoreWl::~DisplayConnectionEcoreWl()
{
  if(mSurfaceType == RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
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

void DisplayConnectionEcoreWl::SetSurfaceType(Dali::RenderSurfaceInterface::Type type)
{
  mSurfaceType = type;

  if(mSurfaceType == Dali::RenderSurfaceInterface::NATIVE_RENDER_SURFACE)
  {
    mDisplay = GetNativeDisplay();
  }
  else
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
    mDisplay                   = reinterpret_cast<EGLNativeDisplayType>(ecore_wl2_display_get(display));
#else
    mDisplay = reinterpret_cast<EGLNativeDisplayType>(ecore_wl_display_get());
#endif
  }
}

EGLNativeDisplayType DisplayConnectionEcoreWl::GetNativeDisplay()
{
  mBufMgr = tbm_bufmgr_init(-1); // -1 is meaningless. The parameter in this function is deprecated.
  if(mBufMgr == nullptr)
  {
    DALI_LOG_ERROR("Fail to init tbm buf mgr\n");
    return nullptr;
  }
  return reinterpret_cast<EGLNativeDisplayType>(tbm_dummy_display_create());
}

void DisplayConnectionEcoreWl::ReleaseNativeDisplay()
{
  if(mDisplay)
  {
    tbm_dummy_display_destroy(reinterpret_cast<tbm_dummy_display*>(mDisplay));
  }

  if(mBufMgr != nullptr)
  {
    tbm_bufmgr_deinit(mBufMgr);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
