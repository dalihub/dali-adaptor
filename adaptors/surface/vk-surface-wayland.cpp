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

//Testing
#include <adaptors/surface/vk-surface-wayland.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

VkSurfaceWayland::VkSurfaceWayland(Dali::RenderSurface& renderSurface)
: VkSurfaceFactory()
{
		
	auto ecoreWlSurface = dynamic_cast<Dali::ECore::WindowRenderSurface*>(&renderSurface);
	assert( ecoreWlSurface != nullptr && "This is not ecore wayland surface!");

	w_surface = ecore_wl_window_surface_get(ecoreWlSurface->GetWlWindow());
	w_display = ecore_wl_display_get();
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
