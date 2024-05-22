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
#include <dali/internal/graphics/vulkan/graphics-implementation.h>

// INTERNAL HEADERS
#if 0
#include <dali/internal/graphics/vulkan/api/vulkan-api-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-surface.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain.h>
#endif

#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <vector>

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
  auto vkGraphicsController = dynamic_cast< const Dali::Graphics::VulkanGraphicsController* >( &mGraphicsController );
  if( vkGraphicsController )
  {
    vkGraphicsController->GetGraphicsDevice().DestroySurface( mFramebufferId );
  }
}

GraphicsInterface::GraphicsInterface(const GraphicsCreateInfo& info,
                                     Integration::DepthBufferAvailable depthBufferRequired,
                                     Integration::StencilBufferAvailable stencilBufferRequired )
: mCreateInfo(info),
  mDepthBufferRequired( depthBufferRequired ),
  mStencilBufferRequired( stencilBufferRequired )
{
}

GraphicsInterface::~GraphicsInterface() = default;


VulkanGraphics::VulkanGraphics(const Dali::Graphics::GraphicsCreateInfo& info,
                   Integration::DepthBufferAvailable depthBufferAvailable,
                   Integration::StencilBufferAvailable stencilBufferRequired )
: GraphicsInterface( info, depthBufferAvailable, stencilBufferRequired ),
  mGraphicsController()
{
}

VulkanGraphics::~VulkanGraphics() = default;


void VulkanGraphics::Initialize()
{
  mGraphicsController.Initialize(*this);
}

void VulkanGraphics::ConfigureSurface(Dali::RenderSurfaceInterface* surface)
{
  surface->InitializeGraphics(*this); // Calls CreateSurface below
}

std::unique_ptr<Dali::Graphics::Surface> VulkanGraphics::CreateSurface( Dali::Graphics::SurfaceFactory& surfaceFactory)
{
  auto& graphicsDevice = mGraphicsController.GetGraphicsDevice();

  // create surface ( also takes surface factory ownership )
  auto framebufferId = graphicsDevice.CreateSurface( surfaceFactory, mCreateInfo );

  // create swapchain from surface
  auto surface = graphicsDevice.GetSurface( framebufferId );

  // create swapchain
  graphicsDevice.CreateSwapchainForSurface( surface );

  return std::unique_ptr<Dali::Graphics::Surface>( new Dali::Graphics::Surface( mGraphicsController, framebufferId ) );
}

void VulkanGraphics::Destroy()
{
}

void VulkanGraphics::Pause()
{
  mGraphicsController.Pause();
}

void VulkanGraphics::Resume()
{
  mGraphicsController.Resume();
}

Dali::Graphics::Controller& VulkanGraphics::GetController()
{
  return mGraphicsController;
}

void VulkanGraphics::SurfaceResized( unsigned int width, unsigned int height )
{
  mCreateInfo.surfaceWidth = width;
  mCreateInfo.surfaceHeight = height;

  //@todo Shouldn't this class own the graphicsDevice?!
  auto& graphicsDevice = mGraphicsController.GetGraphicsDevice();
  graphicsDevice.SurfaceResized( width, height );
}

} // Namespace Graphics
} // Namespace Dali
