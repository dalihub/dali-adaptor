#ifndef DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
#define DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

namespace Dali
{
class RenderSurfaceInterface;

namespace Internal
{
namespace Adaptor
{
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
  GraphicsInterface()
  : mDepthBufferRequired(Integration::DepthBufferAvailable::FALSE),
    mStencilBufferRequired(Integration::StencilBufferAvailable::FALSE),
    mPartialUpdateRequired(Integration::PartialUpdateAvailable::FALSE){};

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
  virtual void Initialize() = 0;

  /**
   * Initialize the graphics subsystem, providing explicit parameters.
   *
   * @param[in] depth True if depth buffer is required
   * @param[in] stencil True if stencil buffer is required
   * @param[in] partialRendering True if partial rendering is required
   * @param[in] msaa level of anti-aliasing required (-1 = off)
   */
  virtual void Initialize(bool depth, bool stencil, bool partialRendering, int msaa) = 0;

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

  /**
   * Get whether the stencil buffer is required
   * @return TRUE if the stencil buffer is required
   */
  Integration::PartialUpdateAvailable GetPartialUpdateRequired()
  {
    return mPartialUpdateRequired;
  };

  /**
   * @return true if advanced blending options are supported
   */
  virtual bool IsAdvancedBlendEquationSupported() = 0;

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
   * @return the version number of the shader language
   */
  virtual uint32_t GetShaderLanguageVersion() = 0;

  /**
   * Store cached configurations
   */
  virtual void CacheConfigurations(ConfigurationManager& configurationManager) = 0;

protected:
  Integration::DepthBufferAvailable   mDepthBufferRequired;   ///< Whether the depth buffer is required
  Integration::StencilBufferAvailable mStencilBufferRequired; ///< Whether the stencil buffer is required
  Integration::PartialUpdateAvailable mPartialUpdateRequired; ///< Whether the partial update is required
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_INTERFACE_H
