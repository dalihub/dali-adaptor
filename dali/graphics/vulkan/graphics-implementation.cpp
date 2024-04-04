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
#include <dali/graphics/vulkan/graphics-implementation.h>

// INTERNAL HEADERS
#include <dali/graphics/vulkan/api/vulkan-api-controller.h>
#include <dali/graphics/vulkan/internal/vulkan-types.h>
#include <dali/graphics/vulkan/internal/vulkan-surface.h>
#include <dali/graphics/vulkan/internal/vulkan-framebuffer.h>
#include <dali/graphics/vulkan/internal/vulkan-swapchain.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>

extern "C" std::vector<uint32_t> GraphicsGetBuiltinShader( const std::string& tag );

namespace Dali
{
namespace Graphics
{

/**
 * Implementation of Graphics::Surface
 */
Surface::Surface( Dali::Graphics::Controller& graphicsController, Dali::Graphics::FBID framebufferId )
: mGraphicsController( graphicsController ),
  mFramebufferId( framebufferId )
{
}

Surface::~Surface()
{
  auto vkGraphicsController = dynamic_cast< const Dali::Graphics::VulkanAPI::Controller* >( &mGraphicsController );
  if( vkGraphicsController )
  {
    vkGraphicsController->GetGraphics().DestroySurface( mFramebufferId );
  }
}

GraphicsInterface::GraphicsInterface( const GraphicsCreateInfo& info,
                                      Integration::DepthBufferAvailable depthBufferRequired,
                                      Integration::StencilBufferAvailable stencilBufferRequired )
: mCreateInfo(info),
  mDepthBufferRequired( depthBufferRequired ),
  mStencilBufferRequired( stencilBufferRequired )
{
}

GraphicsInterface::~GraphicsInterface() = default;


Graphics::Graphics( const Dali::Graphics::GraphicsCreateInfo& info,
                    Integration::DepthBufferAvailable depthBufferAvailable,
                    Integration::StencilBufferAvailable stencilBufferRequired )
: GraphicsInterface( info, depthBufferAvailable, stencilBufferRequired ),
  mGraphicsController()
{
}

Graphics::~Graphics() = default;


void Graphics::Initialize()
{
}

void Graphics::Create()
{
  mGraphicsController.Initialise();
}

std::unique_ptr<Dali::Graphics::Surface> Graphics::CreateSurface( Dali::Graphics::SurfaceFactory& surfaceFactory)
{
  auto& graphicsImpl = mGraphicsController.GetGraphics();

  // create surface ( also takes surface factory ownership )
  auto framebufferId = graphicsImpl.CreateSurface( surfaceFactory, mCreateInfo );

  // create swapchain from surface
  auto surface = graphicsImpl.GetSurface( framebufferId );

  // create swapchain
  graphicsImpl.CreateSwapchainForSurface( surface );

  return std::unique_ptr<Dali::Graphics::Surface>( new Dali::Graphics::Surface( mGraphicsController, framebufferId ) );
}

void Graphics::Destroy()
{
}

void Graphics::Pause()
{
  mGraphicsController.Pause();
}

void Graphics::Resume()
{
  mGraphicsController.Resume();
}


Dali::Graphics::Controller& Graphics::GetController()
{
  return mGraphicsController;
}



} // Namespace Graphics
} // Namespace Dali
