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

#include <adaptors/surface/vk-surface-xcb.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

VkSurfaceXcb::VkSurfaceXcb(xcb_connection_t* connection, xcb_window_t window)
: VkSurfaceFactory{}, mConnection(connection), mWindow(window)
{
}

vk::SurfaceKHR VkSurfaceXcb::Create(vk::Instance instance, vk::AllocationCallbacks* allocCallbacks,
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