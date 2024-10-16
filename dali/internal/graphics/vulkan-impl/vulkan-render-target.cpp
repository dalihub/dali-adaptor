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
#include "vulkan-render-target.h"

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include "vulkan-framebuffer.h"
#include "vulkan-graphics-controller.h"

namespace Dali::Graphics::Vulkan
{
RenderTarget::RenderTarget(const Graphics::RenderTargetCreateInfo& createInfo, VulkanGraphicsController& controller)
: RenderTargetResource(createInfo, controller)
{
  if(createInfo.surface)
  {
    // Do creation stuff!
    //   Create Swapchain?!
  }
}

RenderTarget::~RenderTarget() = default;

void RenderTarget::DestroyResource()
{
}

void RenderTarget::DiscardResource()
{
  mController.DiscardResource(this);

  // The surface context should be deleted now
  if(mCreateInfo.surface)
  {
    //mController.DeleteSurfaceContext(static_cast<Dali::RenderSurfaceInterface*>(mCreateInfo.surface));
    mCreateInfo.surface = nullptr;
  }
}

Vulkan::Framebuffer* RenderTarget::GetFramebuffer() const
{
  return static_cast<Vulkan::Framebuffer*>(mCreateInfo.framebuffer);
}

Integration::RenderSurfaceInterface* RenderTarget::GetSurface() const
{
  return mCreateInfo.surface;
}

} // namespace Dali::Graphics::Vulkan
