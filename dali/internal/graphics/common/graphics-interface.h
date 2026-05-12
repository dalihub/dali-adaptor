#ifndef DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
#define DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-controller.h>
#include <dali/integration-api/core-enumerations.h>
#include <dali/public-api/math/int-pair.h>
#include <limits>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface-enum.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/window-base.h>

namespace Dali
{
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
    Integration::PartialUpdateAvailable partialUpdateRequired,
    int                                 multiSamplingLevel,
    Dali::Graphics::ContextPriority     contextPriority)
  : mCreateInfo(info),
    mDepthBufferRequired(depthBufferRequired),
    mStencilBufferRequired(stencilBufferRequired),
    mPartialUpdateRequired(partialUpdateRequired),
    mMultiSamplingLevel(multiSamplingLevel),
    mContextPriority(contextPriority)
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
   * @param[in] contextPriority Priority of context
   */
  virtual void Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa, Dali::Graphics::ContextPriority contextPriority) = 0;

  /**
   * Initialize the graphics API subsystem
   * @param displayConnection
   */
  virtual void InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection) = 0;

  /**
   * Get the graphics display handle. It will be a type in the specific graphics backend,
   * e.g. EGLDisplay.
   */
  virtual Dali::Any GetDisplay() const = 0;

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
    Graphics::SurfaceFactory*            factory,
    Dali::Internal::Adaptor::WindowBase* windowBase,
    ColorDepth                           colorDepth,
    int                                  width,
    int                                  height) = 0;

  /**
   * @brief Create the graphics surface with per-surface depth/stencil/MSAA configuration.
   *
   * This overload allows each window to specify its own depth buffer, stencil buffer,
   * and MSAA requirements rather than using the global defaults from GraphicsCreateInfo.
   *
   * @param[in] factory The factory used to generate a graphics surface for a window
   * @param[in] windowBase The base window to generate a graphics surface for
   * @param[in] colorDepth The color depth of the window
   * @param[in] width The width of the window (After rotation applied)
   * @param[in] height The height of the window (After rotation applied)
   * @param[in] depthBufferRequired Whether depth buffer is required for this surface
   * @param[in] stencilBufferRequired Whether stencil buffer is required for this surface
   * @param[in] multiSamplingLevel The MSAA level for this surface (0 = disabled)
   * @return the Id of the graphics surface/swapchain pair.
   */
  virtual Graphics::SurfaceId CreateSurface(
    Graphics::SurfaceFactory*            factory,
    Dali::Internal::Adaptor::WindowBase* windowBase,
    ColorDepth                           colorDepth,
    int                                  width,
    int                                  height,
    bool                                 depthBufferRequired,
    bool                                 stencilBufferRequired,
    int                                  multiSamplingLevel) = 0;

  /**
   * @brief Reconfigure an existing surface with new depth/stencil/MSAA settings.
   *
   * The new context shares resources with the existing resource context.
   *
   * @param[in] surfaceId The surface ID to reconfigure
   * @param[in] depthBufferRequired Whether depth buffer is required
   * @param[in] stencilBufferRequired Whether stencil buffer is required
   * @param[in] multiSamplingLevel The MSAA level (0 = disabled)
   * @return true if the surface was successfully reconfigured
   */
  virtual bool ReconfigureSurface(Graphics::SurfaceId surfaceId,
                                  bool                depthBufferRequired,
                                  bool                stencilBufferRequired,
                                  int                 multiSamplingLevel) = 0;

  /**
   * Reset the surface's gpu state cache and re-initializes gpu state to defaults.
   *
   * This must be called after creating a new context when the state
   * has been invalidated (e.g. after ReconfigureSurface).
   * The context must already be current when this is called.
   */
  virtual void ResetSurfaceState() = 0;

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
  virtual void ActivateSurfaceContext(Integration::RenderSurfaceInterface* surface) = 0;

  /**
   * Makes the context for the given surface ID the current context.
   *
   * @param surfaceId The ID of the surface whose context we want to make current.
   */
  virtual void MakeContextCurrent(Graphics::SurfaceId surfaceId) = 0;

  /**
   * Acquire the next image to draw onto.
   *
   * @param surface The surface to acquire the next image for.
   */
  virtual void AcquireNextImage(Integration::RenderSurfaceInterface* surface) = 0;

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
   * Sets the size of the surface.
   *
   * @param[in] surface The surface whose context to resize
   * @param[in] size The new size of the surface
   */
  virtual void Resize(Integration::RenderSurfaceInterface* surface, SurfaceSize size) = 0;

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
  virtual void SetDamageRegion(Graphics::SurfaceId surfaceId, std::vector<BoundsInteger>& damagedRegion) = 0;

  /**
   * Swap the surface's buffers. May be done by other mechanisms, depending on
   * the graphics backend.
   */
  virtual void SwapBuffers(Graphics::SurfaceId surfaceId)                                                = 0;
  virtual void SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<BoundsInteger>& damageRects) = 0;

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
   * Get level of anti-aliasing required (-1 = off)
   * @return The level of multi sampling
   */
  const int GetMultiSamplingLevel() const
  {
    return mMultiSamplingLevel;
  };

  /**
   * Get proirity of context
   * @return The priority of context
   */
  const Dali::Graphics::ContextPriority GetContextPriority() const
  {
    return mContextPriority;
  };

  /**
   * @brief Set the graphics requirements
   * @note We must call this API before Initialize()
   */
  void UpdateGraphicsRequired(Integration::DepthBufferAvailable depth, Integration::StencilBufferAvailable stencil, Integration::PartialUpdateAvailable partial, int multiSamplingLevel, Dali::Graphics::ContextPriority contextPriority)
  {
    mDepthBufferRequired   = depth;
    mStencilBufferRequired = stencil;
    mPartialUpdateRequired = partial;
    mMultiSamplingLevel    = multiSamplingLevel;
    mContextPriority       = contextPriority;
  }

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
   * Signal that PreRender has completed.
   */
  virtual void RenderStart() = 0;

  /**
   * @return true if rendering data need to be presented forcibly.
   */
  virtual bool ForcePresentRequired() = 0;

  /**
   * Get whether any rendering data was presented for the latest used RenderSurfaceInterface.
   * @note This will reset the presented flag internally, so it should be called only once per each surface.
   * @return true if any rendering data was presented for the latest used RenderSurfaceInterface.
   */
  virtual bool DidPresent() = 0;

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
  int                                 mMultiSamplingLevel;    ///< The multiple sampling level
  Dali::Graphics::ContextPriority     mContextPriority;       ///< The priority of current graphics system
};

} // namespace Graphics

} // namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
