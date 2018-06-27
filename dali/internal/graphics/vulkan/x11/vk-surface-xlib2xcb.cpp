/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/integration-api/render-surface.h>
#include <dali/internal/graphics/vulkan/x11/vk-surface-xlib2xcb.h>
#include <dali/internal/window-system/ubuntu-x11/window-render-surface-ecore-x.h>
#include <X11/Xlib-xcb.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

VkSurfaceXlib2Xcb::VkSurfaceXlib2Xcb(Dali::RenderSurface& renderSurface)
: VkSurfaceFactory()
{
  auto ecoreSurface = dynamic_cast<Dali::Internal::Adaptor::WindowRenderSurfaceEcoreX*>(&renderSurface);
  assert( ecoreSurface != nullptr && "This is not ecore surface!");
  mConnection = XGetXCBConnection(XOpenDisplay(nullptr));
  mWindow = static_cast<decltype(mWindow)>(ecoreSurface->GetXWindow());
}

VkSurfaceXlib2Xcb::VkSurfaceXlib2Xcb(Display* display, Window window)
: VkSurfaceFactory()
{
  mConnection = XGetXCBConnection(display);
  mWindow = static_cast<decltype(mWindow)>(window);
}

vk::SurfaceKHR VkSurfaceXlib2Xcb::Create(vk::Instance instance, const vk::AllocationCallbacks* allocCallbacks,
                                     vk::PhysicalDevice physicalDevice) const
{
  vk::XcbSurfaceCreateInfoKHR info;
  info.setConnection(mConnection).setWindow(mWindow);
  auto retval = instance.createXcbSurfaceKHR(info, allocCallbacks).value;
  return retval;
}
}
}
}
