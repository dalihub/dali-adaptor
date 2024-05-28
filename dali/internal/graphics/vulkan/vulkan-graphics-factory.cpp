/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/integration-api/core-enumerations.h>
#include <dali/internal/graphics/vulkan/graphics-implementation.h>

namespace Dali::Internal::Adaptor
{

GraphicsFactory::GraphicsFactory( EnvironmentOptions& environmentOptions )
: mEnvironmentOptions( environmentOptions )
{
}

GraphicsFactory::~GraphicsFactory() = default;


Graphics::GraphicsInterface& GraphicsFactory::Create(GraphicsFactory::PositionSize positionSize)
{
  auto depthBufferRequired = (mEnvironmentOptions.DepthBufferRequired() ?
                              Integration::DepthBufferAvailable::TRUE :
                              Integration::DepthBufferAvailable::FALSE );

  auto stencilBufferRequired = (mEnvironmentOptions.StencilBufferRequired() ?
                                Integration::StencilBufferAvailable::TRUE :
                                Integration::StencilBufferAvailable::FALSE );

  uint32_t depthStencilMask = mEnvironmentOptions.StencilBufferRequired() ? 1u : 0u;
  depthStencilMask |= uint32_t(mEnvironmentOptions.DepthBufferRequired()) ? 1u << 1u : 0u;

  Graphics::GraphicsCreateInfo info{};
  info.surfaceWidth = uint32_t( positionSize.width );
  info.surfaceHeight = uint32_t( positionSize.height );
  info.depthStencilMode = std::function<Graphics::DepthStencilMode()>(
    [depthStencilMask]() {
      switch( depthStencilMask )
      {
        case 1:
        case 3:
          return Graphics::DepthStencilMode::DEPTH_STENCIL_OPTIMAL;
        case 2:
          return Graphics::DepthStencilMode::DEPTH_OPTIMAL;
        case 0:
        default:
          return Graphics::DepthStencilMode::NONE;
      }
    }
  )();

  info.swapchainBufferingMode = Graphics::SwapchainBufferingMode::OPTIMAL;

  auto graphics = new Dali::Graphics::VulkanGraphics( info, depthBufferRequired, stencilBufferRequired );
  return static_cast<Dali::Graphics::GraphicsInterface&>(*graphics);
}

void GraphicsFactory::Destroy()
{
}

} // Dali::Internal::Adaptor
