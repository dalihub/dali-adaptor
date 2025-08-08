#ifndef DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H
#define DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H

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

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/egl-interface.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
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
class WindowBase;

// Needs exporting as required by the GlWindow library
class DALI_ADAPTOR_API EglGraphics : public Graphics::GraphicsInterface
{
public:
  /**
   * Constructor
   */
  EglGraphics(
    EnvironmentOptions&                 environmentOptions,
    const Graphics::GraphicsCreateInfo& info,
    Integration::DepthBufferAvailable   depthBufferRequired,
    Integration::StencilBufferAvailable stencilBufferRequired,
    Integration::PartialUpdateAvailable partialUpdateRequired,
    int                                 multiSamplingLevel);

  /**
   * Destructor
   */
  virtual ~EglGraphics();

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::Initialize()
   */
  void Initialize(const Dali::DisplayConnection& displayConnection) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::Initialize(bool,bool,bool,int)
   */
  void Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::InitializeGraphicsAPI()
   */
  void InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::GetDisplay()
   */
  Dali::Any GetDisplay() const override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::CreateSurface()
   */
  Graphics::SurfaceId CreateSurface(Graphics::SurfaceFactory* surfaceFactory, WindowBase* windowBase, ColorDepth colorDepth, int width, int height) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::DestroySurface()
   */
  void DestroySurface(Graphics::SurfaceId surfaceId) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::ReplaceSurface()
   */
  bool ReplaceSurface(Graphics::SurfaceId surfaceId, int width, int height) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::ConfigureSurface()
   */
  void ConfigureSurface(Dali::Integration::RenderSurfaceInterface* surface) override;

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
   * Activate the surface context
   *
   * @param[in] surface The surface whose context to be switched to.
   */
  void ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface) override;

  /**
   * Make the surface context current
   * @todo Same as ActivateSurfaceContext?!
   */
  void MakeContextCurrent(Graphics::SurfaceId surfaceId) override;

  /**
   * @copydoc Graphics::GraphicsInterface::AcquireNextImage()
   */
  void AcquireNextImage(Integration::RenderSurfaceInterface* surface) override;

  /**
   * This is called after all the surfaces have been rendered.
   *
   * @note This should not be called if uploading resource only without rendering any surface.
   */
  void PostRender() override;

  /**
   * Generate frame statistics and post to debug log.
   */
  void PostRenderDebug() override;

  /**
   * @copydoc Graphics::GraphicsInterface::Pause()
   */
  void Pause() override;

  /**
   * @copydoc Graphics::GraphicsInterface::Resume()
   * Inform graphics interface that this is the first frame after a resume.
   * (For debug only)
   */
  void Resume() override;

  /**
   * @copydoc Graphics::GraphicsInterface::Resize()
   */
  void Resize(Integration::RenderSurfaceInterface* surface, Uint16Pair positionSize) override;

  /**
   * @copydoc Graphics::GraphicsInterface::GetBufferAge()
   */
  int GetBufferAge(Graphics::SurfaceId surfaceId) override;

  /**
   * @copydoc Graphics::GraphicsInterface::SetDamageRegion()
   */
  void SetDamageRegion(Graphics::SurfaceId, std::vector<Rect<int>>& damagedRegion) override;

  /**
   * @copydoc Graphics::GraphicsInterface::SwapBuffers
   */
  void SwapBuffers(Graphics::SurfaceId surfaceId) override;
  void SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>& damagedRegion) override;

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
   * Gets the implementation of GlSyncAbstraction for EGL.
   * @return The implementation of GlSyncAbstraction for EGL.
   */
  EglSyncImplementation& GetSyncImplementation();

  /**
   * Gets the EGL image extension
   * @return The EGL image extension
   */
  EglImageExtensions* GetImageExtensions();

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::Shutdown()
   */
  void Shutdown() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::Destroy()
   */
  void Destroy() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::GetController()
   */
  Graphics::Controller& GetController() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::IsAdvancedBlendEquationSupported()
   */
  bool IsAdvancedBlendEquationSupported() override
  {
    return mGLES->IsAdvancedBlendEquationSupported();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::IsMultisampledRenderToTextureSupported
   */
  bool IsMultisampledRenderToTextureSupported() override
  {
    return mGLES->IsMultisampledRenderToTextureSupported();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::IsInitialized
   */
  bool IsInitialized() override
  {
    return mEglImplementation && mEglImplementation->IsGlesInitialized();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::IsResourceContextSupported
   */
  bool IsResourceContextSupported() override
  {
    return mEglImplementation && mEglImplementation->IsSurfacelessContextSupported();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::GetMaxTextureSize
   */
  uint32_t GetMaxTextureSize() override
  {
    return mGLES->GetMaxTextureSize();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::GetMaxCombinedTextureUnits
   */
  uint32_t GetMaxCombinedTextureUnits() override
  {
    return mGLES->GetMaxCombinedTextureUnits();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::GetMaxTextureSamples
   */
  uint8_t GetMaxTextureSamples() override
  {
    return mGLES->GetMaxTextureSamples();
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::GetShaderLanguageVersion
   */
  uint32_t GetShaderLanguageVersion() override
  {
    return mGLES->GetShadingLanguageVersion();
  }

  bool ApplyNativeFragmentShader(std::string& shader, const char* customSamplerType)
  {
    return mGLES->ApplyNativeFragmentShader(shader, customSamplerType);
  }

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::
   */
  void CacheConfigurations(ConfigurationManager& configurationManager) override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::FrameStart()
   */
  void FrameStart() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::ForcePresentRequired()
   */
  bool ForcePresentRequired() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::DidPresent()
   */
  bool DidPresent() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::LogMemoryPools()
   */
  void LogMemoryPools() override;

public:
  // Eliminate copy and assigned operations
  EglGraphics(const EglGraphics& rhs)            = delete;
  EglGraphics& operator=(const EglGraphics& rhs) = delete;

private:
  /**
   * Initialize graphics subsystems
   */
  void EglInitialize();

private:
  struct EglSurfaceContext
  {
    WindowBase* windowBase;
    EGLSurface  surface;
    EGLContext  context;
  };

  ///<@todo Should SurfaceId be sequential from here, or just a hash of the surface ptr?
  std::unordered_map<Graphics::SurfaceId, EglSurfaceContext> mSurfaceMap;
  Graphics::SurfaceId                                        mBaseSurfaceId{0u};
  Graphics::EglGraphicsController                            mGraphicsController; ///< Graphics Controller for Dali Core
  std::unique_ptr<GlImplementation>                          mGLES;               ///< GL implementation
  std::unique_ptr<EglImplementation>                         mEglImplementation;  ///< EGL implementation
  std::unique_ptr<EglImageExtensions>                        mEglImageExtensions; ///< EGL image extension
  std::unique_ptr<EglSyncImplementation>                     mEglSync;            ///< GlSyncAbstraction implementation for EGL

  bool mForcePresentRequired; ///< Flag to force present
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H
