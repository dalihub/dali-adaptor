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
#include <dali/graphics/graphics-interface.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>
#include <dali/internal/graphics/common/surface-factory.h>

// EXTERNAL INCLUDES
#include <memory>

namespace Dali
{
namespace Graphics
{
class Controller;

namespace Vulkan
{
class Graphics; // Low level implementation. @todo rename to make less confusing
}


/**
 * Graphics implementation class
 */
class Graphics final : public Dali::Graphics::GraphicsInterface
{
public:
  Graphics( const Dali::Graphics::GraphicsCreateInfo& info,
            Integration::DepthBufferAvailable depthBufferAvailable,
            Integration::StencilBufferAvailable stencilBufferRequired );

  ~Graphics();

  void Initialize() override;

  /**
   * @param surfaceFactory
   * @return
   */
  std::unique_ptr<Dali::Graphics::Surface> CreateSurface( Dali::Graphics::SurfaceFactory& surfaceFactory ) override;

  /**
   * When creating Graphics at least one surfaceFactory must be supplied ( no headless mode )
   * @param surfaceFactory the surface factory.
   *
   * @note This should be called from the render thread
   */
  void Create() override;

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
   * Prerender
   */
  void PreRender() override;

  /*
   * Postrender
   */
  void PostRender() override;

  /**
   * Returns controller object
   * @return
   */
  Dali::Graphics::Controller& GetController() override;

  /*
   * Surface resized
   */
  void SurfaceResized( unsigned int width, unsigned int height ) override;

  // this function is used only by the standalone test
  template <class T>
  T& GetImplementation() const
  {
    return static_cast<T&>(*mGraphicsImpl.get());
  }

private:
  std::unique_ptr<Dali::Graphics::Vulkan::Graphics> mGraphicsImpl;
};


} // Namespace Graphics
} // Namespace Dali

#endif // DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATIONH
