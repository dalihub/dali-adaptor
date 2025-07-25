/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include "dali/internal/window-system/common/window-render-surface.h"

extern "C" std::vector<uint32_t> GraphicsGetBuiltinShader(const std::string& tag);

namespace Dali
{
namespace Graphics
{
VulkanGraphics::VulkanGraphics(const Dali::Graphics::GraphicsCreateInfo& info,
                               Integration::DepthBufferAvailable         depthBufferAvailable,
                               Integration::StencilBufferAvailable       stencilBufferRequired,
                               Integration::PartialUpdateAvailable       partialUpdateRequired)
: GraphicsInterface(info, depthBufferAvailable, stencilBufferRequired, Integration::PartialUpdateAvailable::FALSE /*partialUpdateRequired*/),
  mGraphicsController(),
  mMultiSamplingLevel(-1) // No multisampling
{
}

VulkanGraphics::~VulkanGraphics() = default;

void VulkanGraphics::Initialize(const Dali::DisplayConnection& displayConnection)
{
  // Pass down depth/stencil req, partial rendering & msaa level
  mGraphicsDevice.Create();

  mGraphicsController.Initialize(*this, mGraphicsDevice);
  InitializeGraphicsAPI(displayConnection);
}

void VulkanGraphics::Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa)
{
  mDepthBufferRequired   = static_cast<Integration::DepthBufferAvailable>(depth);
  mStencilBufferRequired = static_cast<Integration::StencilBufferAvailable>(stencil);
  mPartialUpdateRequired = Integration::PartialUpdateAvailable::FALSE; // static_cast<Integration::PartialUpdateAvailable>(partialRendering);
  mMultiSamplingLevel    = msaa;
  Initialize(displayConnection);
}

void VulkanGraphics::InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection)
{
  // Pass native X11/Wayland/Mac/Android display to vulkan device?
  // Not needed for xlib-xcb surface creation...
}

Dali::Any VulkanGraphics::GetDisplay() const
{
  return {}; // We don't have a display equivalent for vulkan.
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
  // create surface ( also takes surface factory ownership ),
  // and find viable vulkan device.
  auto createInfo          = mCreateInfo;
  createInfo.surfaceWidth  = width;
  createInfo.surfaceHeight = height;
  createInfo.colorDepth    = colorDepth;
  auto surfaceId           = mGraphicsDevice.CreateSurface(*surfaceFactory, createInfo);

  // create swapchain for surface
  mGraphicsDevice.CreateSwapchainForSurface(surfaceId);

  return surfaceId;
}

void VulkanGraphics::DestroySurface(Graphics::SurfaceId surfaceId)
{
  mGraphicsDevice.DestroySurface(surfaceId);
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

void VulkanGraphics::AcquireNextImage(Integration::RenderSurfaceInterface* surface)
{
  auto surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
  mGraphicsDevice.AcquireNextImage(surfaceId);
}

void VulkanGraphics::PostRender()
{
}

void VulkanGraphics::Shutdown()
{
}

void VulkanGraphics::Destroy()
{
  mGraphicsController.RunGarbageCollector(0);
}

void VulkanGraphics::Pause()
{
  mGraphicsController.Pause();
}

void VulkanGraphics::Resume()
{
  mGraphicsController.Resume();
}

void VulkanGraphics::Resize(Integration::RenderSurfaceInterface* surface, Uint16Pair size)
{
  // TODO: Need to consider how to resize the surface for vulkan
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
}

void VulkanGraphics::SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>& damageRects)
{
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

bool VulkanGraphics::ForcePresentRequired()
{
  return false; // Vulkan does not have a force present requirement.
}

bool VulkanGraphics::DidPresent()
{
  const bool didPresent = mGraphicsController.DidPresent();
  mGraphicsController.ResetDidPresent();
  return didPresent;
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
