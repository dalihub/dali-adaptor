/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include "gles-graphics-render-pass.h"

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
struct RenderPass::Impl
{
  Impl()  = default;
  ~Impl() = default;

  std::vector<AttachmentDescription> attachments{};
};

RenderPass::RenderPass(const Graphics::RenderPassCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: RenderPassResource(createInfo, controller)
{
  mImpl = std::make_unique<Impl>();

  // copy attachment description
  if(createInfo.attachments)
  {
    mImpl->attachments.insert(mImpl->attachments.end(), createInfo.attachments->begin(), createInfo.attachments->end());
    mCreateInfo.attachments = &mImpl->attachments;
  }
}

RenderPass::~RenderPass() = default;

void RenderPass::DiscardResource()
{
  mController.DiscardResource(this);
}

} // namespace Dali::Graphics::GLES
