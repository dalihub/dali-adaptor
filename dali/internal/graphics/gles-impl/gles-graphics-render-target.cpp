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
#include "gles-graphics-render-target.h"

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include "egl-graphics-controller.h"
#include "gles-graphics-framebuffer.h"

namespace Dali::Graphics::GLES
{
struct RenderTarget::Impl
{
  Impl(EglGraphicsController& controller)
  : controller(controller){};

  ~Impl() = default;

  EglGraphicsController& controller;
};

RenderTarget::RenderTarget(const Graphics::RenderTargetCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: RenderTargetResource(createInfo, controller)
{
  mImpl = std::make_unique<Impl>(controller);

  if(createInfo.surface)
  {
    controller.CreateSurfaceContext(static_cast<Dali::RenderSurfaceInterface*>(createInfo.surface));
  }
}

RenderTarget::~RenderTarget()
{
  if(mCreateInfo.surface)
  {
    mImpl->controller.DeleteSurfaceContext(static_cast<Dali::RenderSurfaceInterface*>(mCreateInfo.surface));
  }
}

GLES::Framebuffer* RenderTarget::GetFramebuffer() const
{
  return static_cast<GLES::Framebuffer*>(mCreateInfo.framebuffer);
}

Surface* RenderTarget::GetSurface() const
{
  return mCreateInfo.surface;
}

} // namespace Dali::Graphics::GLES