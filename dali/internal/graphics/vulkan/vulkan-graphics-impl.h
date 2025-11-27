#ifndef DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATION_H
#define DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATION_H

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
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

// EXTERNAL INCLUDES
#include <memory>

namespace Dali::Internal::Adaptor
{
class ConfigurationManager;
}

namespace Dali::Graphics
{
/**
 * Graphics implementation class
 */
class VulkanGraphics final : public Dali::Graphics::GraphicsInterface
{
public:
  VulkanGraphics(const Dali::Graphics::GraphicsCreateInfo& info,
                 Integration::DepthBufferAvailable         depthBufferAvailable,
                 Integration::StencilBufferAvailable       stencilBufferRequired,
                 Integration::PartialUpdateAvailable       partialUpdateRequired,
                 int                                       multiSamplingLevel);

  ~VulkanGraphics();

  void Initialize(const Dali::DisplayConnection& displayConnection) override;
  void Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa) override;

  void InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection) override;

  Dali::Any GetDisplay() const override;

  void ConfigureSurface(Dali::Integration::RenderSurfaceInterface* surface) override;

  Graphics::SurfaceId CreateSurface(
    Graphics::SurfaceFactory*      surfaceFactory,
    Internal::Adaptor::WindowBase* windowBase,
    ColorDepth                     colorDepth,
    int                            width,
    int                            height) override;

  void DestroySurface(Graphics::SurfaceId) override;

  bool ReplaceSurface(Graphics::SurfaceId surface, int width, int height) override;

  /**
   * Activate the resource context, if there is one.
   */
  void ActivateResourceContext() override;

  /**
   * Activate the surface context
   *
   * @param[in] surface The surface whose context to be switched to.
   */
  void ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface) override;

  /**
   * @copydoc Graphics::GraphicsInterface::MakeContextCurrent()
   */
  void MakeContextCurrent(Graphics::SurfaceId surfaceId) override;

  /**
   * @copydoc Graphics::GraphicsInterface::AcquireNextImage()
   */
  void AcquireNextImage(Integration::RenderSurfaceInterface* surface) override;

  /**
   * Inform graphics interface that all the surfaces have been rendered.
   *
   * @note This should not be called if uploading resource only without rendering any surface.
   */
  void PostRender() override;

  /**
   * Shut down the graphics implementation
   */
  void Shutdown() override;

  void Destroy() override;

  /**
   * Lifecycle event for pausing application
   */
  void Pause() override;

  /**
   * Lifecycle event for resuming application
   * Inform graphics interface that this is the first frame after a resume.
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

  /**
   * @copydoc Graphics::GraphicsInterface::SwapBuffers
   */
  void SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>&) override;

public: // @todo Consider refactoring these methods out to GraphicsConfig
  /**
   * @return true if advanced blending options are supported
   */
  bool IsAdvancedBlendEquationSupported() override;

  /**
   * @return true if multisampled render to texture is supported
   */
  bool IsMultisampledRenderToTextureSupported() override;

  /**
   * @return true if graphics subsystem is initialized
   */
  bool IsInitialized() override;

  /**
   * @return true if a separate resource context is supported
   */
  bool IsResourceContextSupported() override;

  /**
   * @return the maximum texture size
   */
  uint32_t GetMaxTextureSize() override;

  /**
   * @return the maximum number of combined texture units
   */
  uint32_t GetMaxCombinedTextureUnits() override;

  /**
   * @return the maximum texture samples when we use multisampled texture
   */
  uint8_t GetMaxTextureSamples() override;

  /**
   * @return the version number of the shader language
   */
  uint32_t GetShaderLanguageVersion() override;

  /**
   * Store cached configurations
   */
  void CacheConfigurations(Dali::Internal::Adaptor::ConfigurationManager& configurationManager) override;

public: // Present APIs
  /**
   * @copydoc Dali::Graphics::GraphicsInterface::FrameStart()
   */
  void FrameStart() override;
  /**
   * @copydoc Dali::Graphics::GraphicsInterface::RenderStart()
   */
  void RenderStart() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::ForcePresentRequired()
   */
  bool ForcePresentRequired() override;

  /**
   * @copydoc Dali::Graphics::GraphicsInterface::DidPresent()
   */
  bool DidPresent() override;

public: // Debug APIs
  /**
   * Log frame statistics
   */
  void PostRenderDebug() override;

  /**
   * Log total capacity of memory pools during this frame
   */
  void LogMemoryPools() override;

public:
  /**
   * Returns controller object
   * @return
   */
  Dali::Graphics::Controller& GetController() override;

  /**
   * Get the vulkan device
   */
  Dali::Graphics::Vulkan::Device& GetDevice();

private:
  Vulkan::Device                                 mGraphicsDevice;
  Vulkan::VulkanGraphicsController               mGraphicsController;
  Dali::Internal::Adaptor::ConfigurationManager* mConfigurationManager{nullptr};
};

} // Namespace Dali::Graphics

#endif // DALI_GRAPHICS_VULKAN_GRAPHICS_IMPLEMENTATION_H
