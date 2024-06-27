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
#include <dali/internal/graphics/vulkan/vulkan-graphics-impl.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/system/common/configuration-manager.h>
#include <vector>

extern "C" std::vector<uint32_t> GraphicsGetBuiltinShader(const std::string& tag);

namespace Dali
{
namespace Graphics
{
VulkanGraphics::VulkanGraphics(const Dali::Graphics::GraphicsCreateInfo& info,
                               Integration::DepthBufferAvailable         depthBufferAvailable,
                               Integration::StencilBufferAvailable       stencilBufferRequired,
                               Integration::PartialUpdateAvailable       partialUpdateRequired)
: GraphicsInterface(info, depthBufferAvailable, stencilBufferRequired, partialUpdateRequired),
  mGraphicsController()
{
}

VulkanGraphics::~VulkanGraphics() = default;

void VulkanGraphics::Initialize(const Dali::DisplayConnection& displayConnection)
{
  // Pass down depth/stencil req, partial rendering & msaa level
  mGraphicsDevice.Create();
  mGraphicsDevice.CreateDevice();

  // Create DescriptorSetAllocator

  mGraphicsController.Initialize(*this, mGraphicsDevice);
  InitializeGraphicsAPI(displayConnection);
}

void VulkanGraphics::Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa)
{
  mDepthBufferRequired   = static_cast<Integration::DepthBufferAvailable>(depth);
  mStencilBufferRequired = static_cast<Integration::StencilBufferAvailable>(stencil);
  mPartialUpdateRequired = static_cast<Integration::PartialUpdateAvailable>(partialRendering);
  mMultiSamplingLevel    = msaa;
  Initialize(displayConnection);
}

void VulkanGraphics::InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection)
{
  // Pass native X11/Wayland/Mac/Android display to vulkan device?
  // Not needed for xlib-xcb surface creation...
}

void VulkanGraphics::ConfigureSurface(Dali::Integration::RenderSurfaceInterface* surface)
{
  surface->InitializeGraphics(); // Calls CreateSurface below
}

Graphics::SurfaceId VulkanGraphics::CreateSurface(
  Graphics::SurfaceFactory*      surfaceFactory,
  Internal::Adaptor::WindowBase* windowBase,
  ColorDepth                     colorDepth,
  int                            width,
  int                            height)
{
  // create surface ( also takes surface factory ownership )
  auto createInfo          = mCreateInfo;
  createInfo.surfaceWidth  = width;
  createInfo.surfaceHeight = height;
  createInfo.colorDepth    = colorDepth;
  auto surfaceId           = mGraphicsDevice.CreateSurface(*surfaceFactory, createInfo);

  // create swapchain for surface
  auto surface = mGraphicsDevice.GetSurface(surfaceId);
  mGraphicsDevice.CreateSwapchainForSurface(surface);

  return surfaceId;
}

void VulkanGraphics::DestroySurface(Graphics::SurfaceId)
{
  // @todo Destroy swapchain.
}

bool VulkanGraphics::ReplaceSurface(Graphics::SurfaceId surface, int width, int height)
{
  return true;
}

void VulkanGraphics::ActivateResourceContext()
{
  // Do nothing.
}
void VulkanGraphics::ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface)
{
  // Do nothing.
}

void VulkanGraphics::MakeContextCurrent(Graphics::SurfaceId surfaceId)
{
}

void VulkanGraphics::PostRender()
{
}

void VulkanGraphics::Shutdown()
{
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

int VulkanGraphics::GetBufferAge(Graphics::SurfaceId surfaceId)
{
  return 0;
}

void VulkanGraphics::SetDamageRegion(Graphics::SurfaceId, std::vector<Rect<int>>& damagedRegion)
{
}

void VulkanGraphics::SwapBuffers(Graphics::SurfaceId surfaceId)
{
  // Swapchain update comes from a different place in Vulkan backend
}

void VulkanGraphics::SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>& damageRects)
{
  // Swapchain update comes from a different place in Vulkan backend
}

Dali::Graphics::Controller& VulkanGraphics::GetController()
{
  return mGraphicsController;
}

Vulkan::Device& VulkanGraphics::GetDevice()
{
  return mGraphicsDevice;
}

bool VulkanGraphics::IsAdvancedBlendEquationSupported()
{
  if(mConfigurationManager)
  {
    return mConfigurationManager->IsAdvancedBlendEquationSupported();
  }
  return false;
}

bool VulkanGraphics::IsMultisampledRenderToTextureSupported()
{
  if(mConfigurationManager)
  {
    return mConfigurationManager->IsMultisampledRenderToTextureSupported();
  }
  return false;
}

bool VulkanGraphics::IsInitialized()
{
  return true;
}

bool VulkanGraphics::IsResourceContextSupported()
{
  return false;
}
uint32_t VulkanGraphics::GetMaxTextureSize()
{
  if(mConfigurationManager)
  {
    return mConfigurationManager->GetMaxTextureSize();
  }
  return 0;
}

uint32_t VulkanGraphics::GetMaxCombinedTextureUnits()
{
  if(mConfigurationManager)
  {
    return mConfigurationManager->GetMaxCombinedTextureUnits();
  }
  return 8;
}

uint8_t VulkanGraphics::GetMaxTextureSamples()
{
  return 8;
}

uint32_t VulkanGraphics::GetShaderLanguageVersion()
{
  return 4;
}

void VulkanGraphics::CacheConfigurations(Dali::Internal::Adaptor::ConfigurationManager& configurationManager)
{
  mConfigurationManager = &configurationManager;
}

void VulkanGraphics::FrameStart()
{
  mGraphicsController.FrameStart();
}

void VulkanGraphics::PostRenderDebug()
{
  // Do nothing for now.
}

void VulkanGraphics::LogMemoryPools()
{
  std::size_t graphicsCapacity = mGraphicsController.GetCapacity();
  DALI_LOG_RELEASE_INFO(
    "EglGraphics:\n"
    "  GraphicsController Capacity: %lu\n",
    graphicsCapacity);
}

} // Namespace Graphics
} // Namespace Dali
