#ifndef DALI_GRAPHICS_GRAPHICS_INTERFACE_H
#define DALI_GRAPHICS_GRAPHICS_INTERFACE_H

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
 */

#include <dali/public-api/dali-adaptor-common.h>
#include <dali/integration-api/core-enumerations.h>
#include <dali/graphics-api/graphics-api-controller.h>
#include <cstdint>

namespace Dali
{
namespace Graphics
{
namespace Internal
{
// Forward declarations
class GraphicsImpl;
}

class SurfaceFactory;

using FBID = int32_t;

/**
 * Structure to manager lifecycle of graphics surface.
 */
struct DALI_ADAPTOR_API Surface
{
  Surface( Internal::GraphicsImpl* graphicsImpl, Dali::Graphics::FBID framebufferId);
  ~Surface();

  Internal::GraphicsImpl* mGraphicsImpl;
  Dali::Graphics::FBID mFramebufferId;
};

enum class DepthStencilMode
{
  /**
   * No depth/stencil at all
   */
  NONE,

  /**
   * Optimal depth ( chosen by the implementation )
   */
  DEPTH_OPTIMAL,

  /**
   * Optimal depth and stencil ( chosen by the implementation )
   */
  DEPTH_STENCIL_OPTIMAL,

  /**
   * Depth and stencil with explicit format set in depthStencilFormat
   */
  DEPTH_STENCIL_EXPLICIT,
};

enum class SwapchainBufferingMode
{
  OPTIMAL = 0,

  DOUBLE_BUFFERING = 2,

  TRIPLE_BUFFERING = 3,
};


struct GraphicsCreateInfo
{
  uint32_t                    surfaceWidth;
  uint32_t                    surfaceHeight;
  DepthStencilMode            depthStencilMode;
  SwapchainBufferingMode      swapchainBufferingMode;
};

/**
 * Defines the Graphics interface that a graphics backend will implement (in Adaptor only)
 */
class GraphicsInterface
{
public:
  /**
   * Constructor
   */
  GraphicsInterface( const GraphicsCreateInfo& info,
                     Integration::DepthBufferAvailable depthBufferRequired,
                     Integration::StencilBufferAvailable stencilBufferRequired )
  : mCreateInfo(info),
    mDepthBufferRequired( depthBufferRequired ),
    mStencilBufferRequired( stencilBufferRequired )
  {
  };

  /**
   * Initialize the graphics interface
   */
  virtual void Initialize() = 0;

  /**
   * Create the  Graphics Factory implementation
   */
  virtual void Create() = 0;

  /**
   * Destroy the Graphics Factory implementation
   */
  virtual void Destroy() = 0;

  /**
   * Lifecycle event for pausing application
   */
  virtual void Pause() = 0;

  /**
   * Lifecycle event for resuming application
   */
  virtual void Resume() = 0;

  /**
   * Prerender
   */
  virtual void PreRender() = 0;

  /*
   * Postrender
   */
  virtual void PostRender() = 0;

  /**
   * Create a surface for the graphics implementation
   */
  virtual std::unique_ptr<Surface> CreateSurface( SurfaceFactory& surfaceFactory ) = 0;

  /**
   * Returns controller object
   * @return
   */
  virtual Dali::Graphics::Controller& GetController() = 0;

  /**
   * Surface was resized
   * @param[in] width The new width
   * @param[in] height The new height
   */
  virtual void SurfaceResized( unsigned int width, unsigned int height );

  /**
   * Get the create info used to instantiate the implementation
   */
  const GraphicsCreateInfo& GetCreateInfo() const
  {
    return mCreateInfo;
  }

  /**
   * Get whether the depth buffer is required
   * @return TRUE if the depth buffer is required
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() const
  {
    return mDepthBufferRequired;
  };

  /**
   * Get whether the stencil buffer is required
   * @return TRUE if the stencil buffer is required
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() const
  {
    return mStencilBufferRequired;
  };

protected:
  // No destruction through this interface
  ~GraphicsInterface() = default;

protected:
  const GraphicsCreateInfo& mCreateInfo;
  const Integration::DepthBufferAvailable mDepthBufferRequired;       ///< Whether the depth buffer is required
  const Integration::StencilBufferAvailable mStencilBufferRequired;   ///< Whether the stencil buffer is required
};

} // namespace Graphics
} // namespace Dali

#endif //DALI_GRAPHICS_GRAPHICS_INTERFACE_H
