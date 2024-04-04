#ifndef DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
#define DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H

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

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-controller.h>
#include <dali/integration-api/core-enumerations.h>
#include <dali/internal/system/common/environment-options.h>
#include "dali/internal/window-system/common/display-connection.h"

namespace Dali
{
class RenderSurfaceInterface;

namespace Internal::Adaptor
{
class ConfigurationManager;

class SurfaceFactory;

using FBID = int32_t;

/**
 * Structure to manager lifecycle of graphics surface.
 */
struct Surface
{
  Surface( Dali::Graphics::Controller& graphicsController, FBID framebufferId)
  : mGraphicsController(graphicsController),
    mFramebufferId(framebufferId)
  {
  }

  ~Surface()=default;

  const Dali::Graphics::Controller& mGraphicsController;
  FBID mFramebufferId;
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
  bool                        partialRendering;
};

/**
 * Interface for creating Graphics implementation through the GraphicsFactory.
 */
class GraphicsInterface
{
public:
  /**
   * Constructor
   */
  GraphicsInterface(GraphicsCreateInfo& info,
                    Integration::DepthBufferAvailable depthBufferRequired,
                    Integration::StencilBufferAvailable stencilBufferRequired )
  : mCreateInfo(info),
    mDepthBufferRequired(depthBufferRequired),
    mStencilBufferRequired(stencilBufferRequired)
  {
  }
  /**
   * Destructor
   */
  virtual ~GraphicsInterface() = default;

  /**
   * Returns controller object
   * @return
   */
  virtual Dali::Graphics::Controller& GetController() = 0;

  /**
   * Initialize the graphics subsystem, configured from environment
   */
  virtual void Initialize(const Dali::DisplayConnection& displayConnection) = 0;

  /**
   * Initialize the graphics subsystem, providing explicit parameters.
   *
   * @param[in] displayConnection The connection to the display
   * @param[in] depth True if depth buffer is required
   * @param[in] stencil True if stencil buffer is required
   * @param[in] partialRendering True if partial rendering is required
   * @param[in] msaa level of anti-aliasing required (-1 = off)
   */
  virtual void Initialize(const Dali::DisplayConnection& displayConnection,
                          bool depth, bool stencil, bool partialRendering, int msaa) = 0;

  /**
   * Initialize the graphics API subsystem
   * @param displayConnection
   */
  virtual void InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection) = 0;

  /**
   * Configure the graphics surface
   *
   * @param[in] surface The surface to configure, or NULL if not present
   */
  virtual void ConfigureSurface(Dali::RenderSurfaceInterface* surface) = 0;

  /**
   * Activate the resource context
   */
  virtual void ActivateResourceContext() = 0;

  /**
   * Activate the surface context
   *
   * @param[in] surface The surface whose context to be switched to.
   */
  virtual void ActivateSurfaceContext(Dali::RenderSurfaceInterface* surface) = 0;

  /**
   * Inform graphics interface that all the surfaces have been rendered.
   *
   * @note This should not be called if uploading resource only without rendering any surface.
   */
  virtual void PostRender() = 0;

  /**
   * Inform graphics interface that this is the first frame after a resume.
   */
  virtual void SetFirstFrameAfterResume() = 0;

  /**
   * Shut down the graphics implementation
   */
  virtual void Shutdown() = 0;

  /**
   * Destroy the Graphics implementation
   */
  virtual void Destroy() = 0;

  /**
   * Get whether the depth buffer is required
   * @return TRUE if the depth buffer is required
   */
  Integration::DepthBufferAvailable& GetDepthBufferRequired()
  {
    return mDepthBufferRequired;
  };

  /**
   * Get whether the stencil buffer is required
   * @return TRUE if the stencil buffer is required
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired()
  {
    return mStencilBufferRequired;
  };

  // Lifecycle:
  virtual void Pause()=0;
  virtual void Resume()=0;
  /**
   * @return true if advanced blending options are supported
   */
  virtual bool IsAdvancedBlendEquationSupported() = 0;

  /**
   * @return true if multisampled render to texture is supported
   */
  virtual bool IsMultisampledRenderToTextureSupported() = 0;

  /**
   * @return true if graphics subsystem is initialized
   */
  virtual bool IsInitialized() = 0;

  /**
   * @return true if a separate resource context is supported
   */
  virtual bool IsResourceContextSupported() = 0;

  /**
   * @return the maximum texture size
   */
  virtual uint32_t GetMaxTextureSize() = 0;

  /**
   * @return the maximum number of combined texture units
   */
  virtual uint32_t GetMaxCombinedTextureUnits() = 0;

  /**
   * @return the maximum texture samples when we use multisampled texture
   */
  virtual uint8_t GetMaxTextureSamples() = 0;

  /**
   * @return the version number of the shader language
   */
  virtual uint32_t GetShaderLanguageVersion() = 0;

  /**
   * Store cached configurations
   */
  virtual void CacheConfigurations(ConfigurationManager& configurationManager) = 0;

  /**
   * Initialize data for logging frame info
   */
  virtual void FrameStart() = 0;

  /**
   * Log total capacity of memory pools during this frame
   */
  virtual void LogMemoryPools() = 0;

protected:
  GraphicsCreateInfo mCreateInfo;
  Integration::DepthBufferAvailable   mDepthBufferRequired;   ///< Whether the depth buffer is required
  Integration::StencilBufferAvailable mStencilBufferRequired; ///< Whether the stencil buffer is required
};

} // namespace Internal::Adaptor



} // namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
