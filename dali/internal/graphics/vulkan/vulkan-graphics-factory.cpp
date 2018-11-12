/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan/vulkan-graphics-factory.h>
#include <dali/integration-api/graphics/graphics.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

GraphicsFactory::GraphicsFactory( EnvironmentOptions& environmentOptions )
: mEnvironmentOptions( environmentOptions )
{
}

GraphicsFactory::~GraphicsFactory()
{
  /* Deleted by Adaptor destructor */
}

Integration::Graphics::GraphicsInterface& GraphicsFactory::Create(GraphicsFactory::PositionSize positionSize)
{
  uint32_t depthStencilMask = mEnvironmentOptions.StencilBufferRequired() ? 1 : 0;
  depthStencilMask |= mEnvironmentOptions.DepthBufferRequired() ? 1 << 1 : 0;

  Integration::Graphics::GraphicsCreateInfo info;
  info.surfaceWidth = uint32_t( positionSize.width );
  info.surfaceHeight = uint32_t( positionSize.height );
  info.depthStencilMode = std::function<Integration::Graphics::DepthStencilMode()>(
    [depthStencilMask]() {
      switch( depthStencilMask )
      {
        case 1:
        case 3:
          return Integration::Graphics::DepthStencilMode::DEPTH_STENCIL_OPTIMAL;
        case 2:
          return Integration::Graphics::DepthStencilMode::DEPTH_OPTIMAL;
        case 0:
          return Integration::Graphics::DepthStencilMode::NONE;
        default:
          return Integration::Graphics::DepthStencilMode::NONE;
      }
    }
  )();

  info.swapchainBufferingMode = Integration::Graphics::SwapchainBufferingMode::OPTIMAL;

  auto graphics = new Dali::Integration::Graphics::Graphics( info );
  return *graphics;
}

void GraphicsFactory::Destroy()
{
  /* Deleted by EglGraphics */
}

} // Adaptor
} // Internal
} // Dali
