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

#include <dali/internal/graphics/vulkan/wayland/vk-surface-wayland.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#ifdef ECORE_WAYLAND2
#include <Ecore_Wl2.h>
#else
#include <Ecore_Wayland.h>
#endif

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

VkSurfaceWayland::VkSurfaceWayland(Dali::RenderSurface& renderSurface)
: VkSurfaceFactory()
{
  auto ecoreWlSurface = dynamic_cast<Dali::Internal::Adaptor::WindowRenderSurface*>(&renderSurface);
  assert( ecoreWlSurface != nullptr && "This is not ecore wayland surface!");
#ifdef ECORE_WAYLAND2
  Ecore_Wl2_Window *ecoreWl2Window = AnyCast< Ecore_Wl2_Window* >(ecoreWlSurface->GetNativeWindow());
  w_surface = ecore_wl2_window_surface_get(ecoreWl2Window);

  Ecore_Wl2_Display* wl2_display = ecore_wl2_window_display_get(ecoreWl2Window);
  w_display = ecore_wl2_display_get(wl2_display);
#else
  Ecore_Wl_Window* ecoreWlWindow = AnyCast< Ecore_Wl_Window* >(ecoreWlSurface->GetNativeWindow());

  w_surface = ecore_wl_window_surface_get(ecoreWlWindow);
  w_display = ecore_wl_display_get();
#endif
}

VkSurfaceWayland::VkSurfaceWayland(::wl_display* display, ::wl_surface* surface)
: VkSurfaceFactory()
{
  w_display = display;
  w_surface = surface;
}


vk::SurfaceKHR VkSurfaceWayland::Create(vk::Instance instance, vk::AllocationCallbacks* allocCallbacks,
                                    vk::PhysicalDevice physicalDevice) const
{
  vk::WaylandSurfaceCreateInfoKHR info;

  info.setDisplay(w_display);
  info.setSurface(w_surface);

  auto retval = instance.createWaylandSurfaceKHR(info, allocCallbacks).value;

  return retval;
}
}
}
}
