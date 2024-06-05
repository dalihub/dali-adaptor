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
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <vector>

extern "C" std::vector<uint32_t> GraphicsGetBuiltinShader( const std::string& tag );

namespace Dali
{
namespace Graphics
{

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
  mGraphicsDevice.Create();
  mGraphicsDevice.CreateDevice();

  // Create DescriptorSetAllocator

  mGraphicsController.Initialize(*this, mGraphicsDevice);
}

void VulkanGraphics::ConfigureSurface(Dali::RenderSurfaceInterface* surface)
{
  surface->InitializeGraphics(*this); // Calls CreateSurface below
}

Graphics::SurfaceId VulkanGraphics::CreateSurface(Graphics::SurfaceFactory& surfaceFactory)
{
  // create surface ( also takes surface factory ownership )
  auto surfaceId = mGraphicsDevice.CreateSurface( surfaceFactory, mCreateInfo );

  // create swapchain for surface
  auto surface = mGraphicsDevice.GetSurface( surfaceId );
  mGraphicsDevice.CreateSwapchainForSurface( surface );

  return surfaceId;
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

Vulkan::Device& VulkanGraphics::GetDevice()
{
  return mGraphicsDevice;
}

void VulkanGraphics::SurfaceResized( unsigned int width, unsigned int height )
{
  mCreateInfo.surfaceWidth = width;
  mCreateInfo.surfaceHeight = height;

  mGraphicsDevice.SurfaceResized( width, height );
}

} // Namespace Graphics
} // Namespace Dali
