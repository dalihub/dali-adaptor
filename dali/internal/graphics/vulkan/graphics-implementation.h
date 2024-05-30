#ifndef DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATION_H
#define DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATION_H

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

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/common/surface-factory.h>

// EXTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <memory>


namespace Dali::Graphics
{

/**
 * Graphics implementation class
 */
class VulkanGraphics final : public Dali::Graphics::GraphicsInterface
{
public:
  VulkanGraphics( const Dali::Graphics::GraphicsCreateInfo& info,
                  Integration::DepthBufferAvailable depthBufferAvailable,
                  Integration::StencilBufferAvailable stencilBufferRequired );

  ~VulkanGraphics();

  void Initialize() override;

  void ConfigureSurface(Dali::RenderSurfaceInterface* surface) override;
  Graphics::FramebufferId CreateSurface(Graphics::SurfaceFactory& factory) override;

  void Destroy() override;

  /**
   * Lifecycle event for pausing application
   */
  void Pause() override;

  /**
   * Lifecycle event for resuming application
   */
  void Resume() override;

  /**
   * Returns controller object
   * @return
   */
  Dali::Graphics::Controller& GetController() override;

  /*
   * Surface resized
   */
  void SurfaceResized( unsigned int width, unsigned int height ) override;

  /**
   * Get the vulkan device
   */
  Dali::Graphics::Vulkan::Device& GetDevice();

private:
  Vulkan::Device mGraphicsDevice;
  Vulkan::VulkanGraphicsController mGraphicsController;
};

} // Namespace Dali::Graphics


#endif // DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATION_H
