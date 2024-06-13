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
#include <limits>
#include "dali/internal/window-system/common/display-connection.h"

namespace Dali
{
enum ColorDepth
{
  COLOR_DEPTH_24 = 24,
  COLOR_DEPTH_32 = 32
};

namespace Integration
{
class RenderSurfaceInterface;
}
namespace Internal::Adaptor
{
class ConfigurationManager;
class WindowBase;
} // namespace Internal::Adaptor

namespace Graphics
{
class SurfaceFactory;

/**
 * @brief Surface identifier
 *
 * The surface id is used as the index for windows in the vulkan implementation
 */
using SurfaceId = uint32_t;

const SurfaceId INVALID_SURFACE_ID = std::numeric_limits<uint32_t>::max();

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
  uint32_t               surfaceWidth;
  uint32_t               surfaceHeight;
  DepthStencilMode       depthStencilMode;
  SwapchainBufferingMode swapchainBufferingMode;
  int                    multiSamplingLevel;
};

class ConfigurationManager;

/**
 * Factory interface for creating Graphics Factory implementation
 */
class GraphicsInterface
{
public:
  /**
   * Constructor
   */
  GraphicsInterface(
    const GraphicsCreateInfo&           info,
    Integration::DepthBufferAvailable   depthBufferRequired,
    Integration::StencilBufferAvailable stencilBufferRequired,
    Integration::PartialUpdateAvailable partialUpdateRequired)
  : mCreateInfo(info),
    mDepthBufferRequired(depthBufferRequired),
    mStencilBufferRequired(stencilBufferRequired),
    mPartialUpdateRequired(partialUpdateRequired)
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
  virtual void Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa) = 0;

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
  virtual void ConfigureSurface(Dali::Integration::RenderSurfaceInterface* surface) = 0;

  /**
   * Create the graphics surface and swapchain.
   * @param[in] factory The factory used to generate a graphics surface for a window
   * @param[in] windowBase The base window to generate a graphics surface for.
   * @param[in] colorDepth The color depth of the window
   * @param[in] width The width of the window (After rotation applied)
   * @param[in] height The height of the window (After rotation applied)
   * @return the Id of the graphics surface/swapchain pair.
   */
  virtual Graphics::SurfaceId CreateSurface(
    Graphics::SurfaceFactory*      factory,
    Internal::Adaptor::WindowBase* windowBase,
    ColorDepth                     colorDepth,
    int                            width,
    int                            height) = 0;

  /**
   * Destroy the graphics surface and it's resources.
   * @param[in] surfaceId The surface ID to destroy
   */
  virtual void DestroySurface(Graphics::SurfaceId surfaceId) = 0;

  /**
   * Replace the graphics surface for the given surface id.
   * @param[in] surfaceId The surface ID to replace
   * @param[in] width The new surface width
   * @param[in] height The new surface height
   * @return true if the surface was successfully replaced
   */
  virtual bool ReplaceSurface(Graphics::SurfaceId surfaceId, int width, int height) = 0;

  /**
   * Activate the resource context
   */
  virtual void ActivateResourceContext() = 0;

  /**
   * Activate the surface context
   *
   * @param[in] surface The surface whose context to be switched to.
   */
  virtual void ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface) = 0;

  virtual void MakeContextCurrent(Graphics::SurfaceId surfaceId) = 0;

  /**
   * Inform graphics interface that all the surfaces have been rendered.
   *
   * @note This should not be called if uploading resource only without rendering any surface.
   */
  virtual void PostRender() = 0;

  /**
   * Shut down the graphics implementation
   */
  virtual void Shutdown() = 0;

  /**
   * Destroy the Graphics implementation
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
   * Get the buffer age of the surface. 0 means that the back buffer
   * is invalid and needs a full swap.
   */
  virtual int GetBufferAge(Graphics::SurfaceId surfaceId) = 0;

  /**
   * Set damage regions onto the surface
   * @param[in] surfaceId The surface to define damage regions for
   * @param[in] damagedRegion The damage regions
   */
  virtual void SetDamageRegion(Graphics::SurfaceId surfaceId, std::vector<Rect<int>>& damagedRegion) = 0;

  /**
   * Swap the surface's buffers. May be done by other mechanisms, depending on
   * the graphics backend.
   */
  virtual void SwapBuffers(Graphics::SurfaceId surfaceId)                                            = 0;
  virtual void SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>& damageRects) = 0;

  /**
   * Get whether the depth buffer is required
   * @return TRUE if the depth buffer is required
   */
  const Integration::DepthBufferAvailable GetDepthBufferRequired() const
  {
    return mDepthBufferRequired;
  };

  /**
   * Get whether the stencil buffer is required
   * @return TRUE if the stencil buffer is required
   */
  const Integration::StencilBufferAvailable GetStencilBufferRequired() const
  {
    return mStencilBufferRequired;
  };

  /**
   * Get whether partial update is required
   * @return TRUE if partial update is required
   */
  const Integration::PartialUpdateAvailable GetPartialUpdateRequired() const
  {
    return mPartialUpdateRequired;
  };

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
  virtual void CacheConfigurations(Dali::Internal::Adaptor::ConfigurationManager& configurationManager) = 0;

  /**
   * Initialize data for logging frame info
   */
  virtual void FrameStart() = 0;

  /**
   * Log any collected statistics
   */
  virtual void PostRenderDebug() = 0;

  /**
   * Log total capacity of memory pools during this frame
   */
  virtual void LogMemoryPools() = 0;

protected:
  GraphicsCreateInfo                  mCreateInfo;            ///< the surface creation info
  Integration::DepthBufferAvailable   mDepthBufferRequired;   ///< Whether the depth buffer is required
  Integration::StencilBufferAvailable mStencilBufferRequired; ///< Whether the stencil buffer is required
  Integration::PartialUpdateAvailable mPartialUpdateRequired; ///< Whether the partial update is required
};

} // namespace Graphics

} // namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
