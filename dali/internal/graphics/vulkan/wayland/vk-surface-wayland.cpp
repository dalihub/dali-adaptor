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

// Dali Core math structs need hardening
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include <dali/internal/graphics/vulkan/wayland/vk-surface-wayland.h>
#include <dali/internal/window-system/common/window-render-surface.h>

// Ecore system headers need hardening!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wcast-qual"

#include <Ecore_Wl2.h>

#pragma GCC diagnostic pop

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
VkSurfaceWayland::VkSurfaceWayland(NativeWindowInterface& nativeWindow)
: SurfaceFactory()
{
  Ecore_Wl2_Window* ecoreWl2Window = AnyCast<Ecore_Wl2_Window*>(nativeWindow.GetNativeWindow());
  w_surface                        = ecore_wl2_window_surface_get(ecoreWl2Window);

  Ecore_Wl2_Display* wl2_display = ecore_wl2_window_display_get(ecoreWl2Window);
  w_display                      = ecore_wl2_display_get(wl2_display);
}

VkSurfaceWayland::VkSurfaceWayland(::wl_display* display, ::wl_surface* surface)
: SurfaceFactory()
{
  w_display = display;
  w_surface = surface;
}

vk::SurfaceKHR VkSurfaceWayland::Create(
  vk::Instance                   instance,
  const vk::AllocationCallbacks* allocCallbacks) const
{
  vk::WaylandSurfaceCreateInfoKHR info;

  info.setDisplay(w_display);
  info.setSurface(w_surface);

  auto retval = instance.createWaylandSurfaceKHR(info, allocCallbacks).value;

  return retval;
}

} // namespace Vulkan

std::unique_ptr<SurfaceFactory> SurfaceFactory::New(NativeWindowInterface& nativeWindow)
{
  auto surfaceFactory = std::unique_ptr<Graphics::Vulkan::VkSurfaceWayland>(new Graphics::Vulkan::VkSurfaceWayland(nativeWindow));
  return surfaceFactory;
}

} // namespace Graphics
} // namespace Dali

#pragma GCC diagnostic pop
