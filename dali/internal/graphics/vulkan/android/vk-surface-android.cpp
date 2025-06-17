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

#include <dali/internal/graphics/vulkan/android/vk-surface-android.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
VkSurfaceAndroid::VkSurfaceAndroid(NativeWindowInterface& nativeWindow)
: SurfaceFactory()
{
  mWindow = static_cast<ANativeWindow*>(AnyCast<void*>(nativeWindow.GetNativeWindow()));
}

VkSurfaceAndroid::VkSurfaceAndroid(ANativeWindow* window)
: SurfaceFactory()
{
  mWindow = window;
}

vk::SurfaceKHR VkSurfaceAndroid::Create(
  vk::Instance                   instance,
  const vk::AllocationCallbacks* allocCallbacks) const
{
  vk::AndroidSurfaceCreateInfoKHR info;
  info.window = mWindow;

  auto retval = instance.createAndroidSurfaceKHR(info, allocCallbacks).value;

  return retval;
}

} // namespace Vulkan

std::unique_ptr<SurfaceFactory> SurfaceFactory::New(NativeWindowInterface& nativeWindow)
{
  auto surfaceFactory = std::unique_ptr<Graphics::Vulkan::VkSurfaceAndroid>(new Graphics::Vulkan::VkSurfaceAndroid(nativeWindow));
  return surfaceFactory;
}

} // namespace Graphics
} // namespace Dali
