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
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
GraphicsFactory::GraphicsFactory(EnvironmentOptions& environmentOptions)
: mEnvironmentOptions(environmentOptions)
{
}

GraphicsFactory::~GraphicsFactory()
{
  /* Deleted by Adaptor destructor */
}

Graphics::GraphicsInterface& GraphicsFactory::Create()
{
  Graphics::GraphicsCreateInfo info{};
  auto                         depthBufferRequired = (mEnvironmentOptions.DepthBufferRequired() ? Integration::DepthBufferAvailable::TRUE : Integration::DepthBufferAvailable::FALSE);

  auto stencilBufferRequired = (mEnvironmentOptions.StencilBufferRequired() ? Integration::StencilBufferAvailable::TRUE : Integration::StencilBufferAvailable::FALSE);

  auto partialUpdateRequired = (mEnvironmentOptions.PartialUpdateRequired() ? Integration::PartialUpdateAvailable::TRUE : Integration::PartialUpdateAvailable::FALSE);

  info.multiSamplingLevel = mEnvironmentOptions.GetMultiSamplingLevel();

  Graphics::GraphicsInterface* eglGraphicsInterface = new EglGraphics(mEnvironmentOptions, info, depthBufferRequired, stencilBufferRequired, partialUpdateRequired);
  return *eglGraphicsInterface;
}

void GraphicsFactory::Destroy()
{
  /* Deleted by EglGraphics */
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
