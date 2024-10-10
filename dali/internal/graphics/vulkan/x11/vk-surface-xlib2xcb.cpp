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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include <X11/Xlib-xcb.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/graphics/vulkan/x11/vk-surface-xlib2xcb.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
VkSurfaceXlib2Xcb::VkSurfaceXlib2Xcb(NativeWindowInterface& nativeWindow)
: SurfaceFactory()
{
  mConnection = XGetXCBConnection(XOpenDisplay(nullptr));
  mWindow     = static_cast<decltype(mWindow)>(nativeWindow.GetNativeWindowId());
}

VkSurfaceXlib2Xcb::VkSurfaceXlib2Xcb(::Display* display, ::Window window)
: SurfaceFactory()
{
  mConnection = XGetXCBConnection(display);
  mWindow     = static_cast<decltype(mWindow)>(window);
}

vk::SurfaceKHR VkSurfaceXlib2Xcb::Create(vk::Instance instance, const vk::AllocationCallbacks* allocCallbacks) const
{
  vk::XcbSurfaceCreateInfoKHR info;
  info.setConnection(mConnection).setWindow(mWindow);
  auto retval = instance.createXcbSurfaceKHR(info, allocCallbacks).value;
  return retval;
}

} // namespace Vulkan

std::unique_ptr<SurfaceFactory> SurfaceFactory::New(NativeWindowInterface& nativeWindow)
{
  auto surfaceFactory = std::unique_ptr<Graphics::Vulkan::VkSurfaceXlib2Xcb>(new Graphics::Vulkan::VkSurfaceXlib2Xcb(nativeWindow));
  return surfaceFactory;
}

} // namespace Graphics
} // namespace Dali

#pragma GCC diagnostic pop
