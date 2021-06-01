#ifndef DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H
#define DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H

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
#include <dali/integration-api/adaptor-framework/egl-interface.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <dali/internal/graphics/gles/egl-context-helper-implementation.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>
#include <dali/internal/graphics/gles/gl-implementation.h>
#include <dali/internal/graphics/gles/gl-proxy-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EnvironmentOptions;
class ConfigurationManager;

class EglGraphics : public GraphicsInterface
{
public:
  /**
   * Constructor
   */
  EglGraphics(EnvironmentOptions& environmentOptions);

  /**
   * Destructor
   */
  virtual ~EglGraphics();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::Initialize()
   */
  void Initialize() override;

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::Initialize(bool,bool,bool,int)
   */
  void Initialize(bool depth, bool stencil, bool partialRendering, int msaa);

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::ConfigureSurface()
   */
  void ConfigureSurface(Dali::RenderSurfaceInterface* surface) override;

  /**
   * Set gles version
   * Default version is gles 3.0
   */
  void SetGlesVersion(const int32_t glesVersion);

  /**
   * Set whether the surfaceless context is supported
   * @param[in] isSupported Whether the surfaceless context is supported
   */
  void SetIsSurfacelessContextSupported(const bool isSupported);

  /**
   * Activate the resource context (shared surfaceless context)
   */
  void ActivateResourceContext() override;

  /**
   * Inform graphics interface that this is the first frame after a resume.
   * (For debug only)
   */
  void SetFirstFrameAfterResume() override;

  /**
   * Gets the GL abstraction
   * @return The GL abstraction
   */
  Integration::GlAbstraction& GetGlAbstraction() const;

  /**
   * Gets the implementation of EGL
   * @return The implementation of EGL
   */
  EglImplementation& GetEglImplementation() const;

  /**
   * Gets the graphics interface for EGL
   * @return The graphics interface for EGL
   */
  EglInterface& GetEglInterface() const;

  /**
   * @copydoc Dali::Integration::GlAbstraction& GetGlesInterface()
   */
  GlImplementation& GetGlesInterface();

  /**
   * Gets the implementation of GlSyncAbstraction for EGL.
   * @return The implementation of GlSyncAbstraction for EGL.
   */
  EglSyncImplementation& GetSyncImplementation();

  /**
   * Gets the implementation of GlContextHelperAbstraction for EGL.
   * @return The implementation of GlContextHelperAbstraction for EGL.
   */
  EglContextHelperImplementation& GetContextHelperImplementation();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable& GetDepthBufferRequired();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired();

  /**
   * Gets the EGL image extension
   * @return The EGL image extension
   */
  EglImageExtensions* GetImageExtensions();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::Shutdown()
   */
  void Shutdown() override;

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::Destroy()
   */
  void Destroy() override;

  Graphics::Controller& GetController() override;

  bool IsAdvancedBlendEquationSupported() override
  {
    return mGLES->IsAdvancedBlendEquationSupported();
  }

  /**
   * @return true if graphics subsystem is initialized
   */
  bool IsInitialized() override
  {
    return mEglImplementation && mEglImplementation->IsGlesInitialized();
  }

  bool IsResourceContextSupported() override
  {
    return mEglImplementation && mEglImplementation->IsSurfacelessContextSupported();
  }

  uint32_t GetMaxTextureSize() override
  {
    return mGLES->GetMaxTextureSize();
  }

  uint32_t GetShaderLanguageVersion() override
  {
    return mGLES->GetShadingLanguageVersion();
  }

  const char* GetEglImageExtensionString()
  {
    return mGLES->GetEglImageExtensionString();
  }

  void CacheConfigurations(ConfigurationManager& configurationManager) override;

private:
  // Eliminate copy and assigned operations
  EglGraphics(const EglGraphics& rhs) = delete;
  EglGraphics& operator=(const EglGraphics& rhs) = delete;

  /**
   * Initialize graphics subsystems
   */
  void EglInitialize();

private:
  Graphics::EglGraphicsController                 mGraphicsController; ///< Graphics Controller for Dali Core
  std::unique_ptr<GlImplementation>               mGLES;               ///< GL implementation
  std::unique_ptr<EglImplementation>              mEglImplementation;  ///< EGL implementation
  std::unique_ptr<EglImageExtensions>             mEglImageExtensions; ///< EGL image extension
  std::unique_ptr<EglSyncImplementation>          mEglSync;            ///< GlSyncAbstraction implementation for EGL
  std::unique_ptr<EglContextHelperImplementation> mEglContextHelper;   ///< GlContextHelperAbstraction implementation for EGL

  int mMultiSamplingLevel; ///< The multiple sampling level
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H
