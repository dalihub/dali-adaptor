/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>

namespace Dali::Graphics::Vulkan
{
RenderPass::RenderPass(const Graphics::RenderPassCreateInfo& createInfo, VulkanGraphicsController& controller)
: mCreateInfo(createInfo),
  mController(controller)
{
  if(createInfo.attachments)
  {
    mAttachments.insert(mAttachments.end(), createInfo.attachments->begin(), createInfo.attachments->end());
    mCreateInfo.attachments = &mAttachments;
  }
}

RenderPass::~RenderPass()
{
  // Notify observers before destruction so they drop any cached raw pointer
  // to this object — otherwise FramebufferImpl::mRenderPasses would be left
  // with a dangling entry.
  mObserverNotifying = true;
  for(auto* observer : mLifecycleObservers)
  {
    observer->RenderPassInvalidated(this);
  }
  // No need to clear mObserverNotifying — we're being destroyed.
}

void RenderPass::Reinitialize(const Graphics::RenderPassCreateInfo& createInfo)
{
  mCreateInfo = createInfo;
  mAttachments.clear();
  if(createInfo.attachments)
  {
    mAttachments.insert(mAttachments.end(), createInfo.attachments->begin(), createInfo.attachments->end());
    mCreateInfo.attachments = &mAttachments;
  }
  else
  {
    mCreateInfo.attachments = nullptr;
  }

  // Load/store ops or attachment shape may have changed; cached backend
  // RenderPassImpl handles in observing Framebuffers no longer reflect this
  // object's state. Clear them so the next lookup re-matches or recreates.
  mObserverNotifying = true;
  for(auto* observer : mLifecycleObservers)
  {
    observer->RenderPassInvalidated(this);
  }
  mObserverNotifying = false;
}

void RenderPass::AddLifecycleObserver(LifecycleObserver& observer)
{
  DALI_ASSERT_ALWAYS(!mObserverNotifying && "Cannot add observer while notifying RenderPass::LifecycleObservers");
  mLifecycleObservers.insert(&observer);
}

void RenderPass::RemoveLifecycleObserver(LifecycleObserver& observer)
{
  DALI_ASSERT_ALWAYS(!mObserverNotifying && "Cannot remove observer while notifying RenderPass::LifecycleObservers");
  mLifecycleObservers.erase(&observer);
}

} // namespace Dali::Graphics::Vulkan
