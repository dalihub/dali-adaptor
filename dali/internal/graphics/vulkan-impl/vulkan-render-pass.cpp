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
 */


#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>


namespace Dali::Graphics::Vulkan
{
RenderPass::RenderPass(const Graphics::RenderPassCreateInfo& createInfo, VulkanGraphicsController& controller)
: RenderPassResource(createInfo, controller),
  mRenderPassImpl(nullptr)
{
}

RenderPass::~RenderPass() = default;

bool RenderPass::InitializeResource()
{
  auto renderTarget = static_cast<RenderTarget*>(mCreateInfo.renderTarget);
  auto framebuffer = renderTarget->GetFramebuffer();
  DALI_ASSERT_ALWAYS(framebuffer);
  auto vkFramebuffer = framebuffer->GetImpl();

  std::vector<FramebufferAttachment*> colorAttachments = vkFramebuffer->GetAttachments(AttachmentType::COLOR);;
  std::vector<FramebufferAttachment*> depthAttachment = vkFramebuffer->GetAttachments(AttachmentType::DEPTH_STENCIL);;

  mRenderPassImpl = new RenderPassImpl(mCreateInfo, mController, colorAttachments, depthAttachment[0]);
  return true;
}

void RenderPass::DestroyResource()
{
}

void RenderPass::DiscardResource()
{
}

RenderPassImpl* RenderPass::GetImpl()
{
  return mRenderPassImpl;
}

}
