#ifndef DALI_TEST_GRAPHICS_APPLICATION_H
#define DALI_TEST_GRAPHICS_APPLICATION_H

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
#include <dali/integration-api/core.h>
#include <dali/integration-api/resource-policies.h>
#include <dali/integration-api/scene.h>
#include <dali/integration-api/trace.h>

#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <dali/public-api/common/dali-common.h>

#include <test-gl-abstraction.h>
#include <test-graphics-sync-impl.h>
#include <test-platform-abstraction.h>
#include <test-render-controller.h>

namespace Dali
{
namespace Internal::Adaptor
{
class ConfigurationManager;
}

class TestGraphicsImpl : public Graphics::GraphicsInterface
{
public:
  TestGraphicsImpl()
  : GraphicsInterface(Graphics::GraphicsCreateInfo{}, Integration::DepthBufferAvailable::TRUE, Integration::StencilBufferAvailable::TRUE, Integration::PartialUpdateAvailable::TRUE)
  {
  }
  virtual ~TestGraphicsImpl() = default;

  Dali::Graphics::Controller& GetController() override
  {
    Dali::Graphics::Controller* controller{nullptr};
    return *controller;
  }

  /**
   * Initialize the graphics subsystem, configured from environment
   */
  void Initialize(const Dali::DisplayConnection& dc) override
  {
    mCallstack.PushCall("Initialize()", "");
  }

  /**
   * Initialize the graphics subsystem, providing explicit parameters.
   *
   * @param[in] depth True if depth buffer is required
   * @param[in] stencil True if stencil buffer is required
   * @param[in] partialRendering True if partial rendering is required
   * @param[in] msaa level of anti-aliasing required (-1 = off)
   */
  void Initialize(const Dali::DisplayConnection& dc, bool depth, bool stencil, bool partialRendering, int msaa) override
  {
    TraceCallStack::NamedParams namedParams;
    namedParams["depth"] << depth;
    namedParams["stencil"] << stencil;
    namedParams["partialRendering"] << partialRendering;
    namedParams["msaa"] << msaa;
    mCallstack.PushCall("Initialize()", "");
  }

  void InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection) override
  {
    mCallstack.PushCall("InitializeGraphicsAPI()", "");
  }

  Dali::Any GetDisplay() const override
  {
    return {};
  }

  /**
   * Configure the graphics surface
   *
   * @param[in] surface The surface to configure, or NULL if not present
   */
  void ConfigureSurface(Dali::Integration::RenderSurfaceInterface* surface) override
  {
    mCallstack.PushCall("ConfigureSurface()", "");
  }

  Graphics::SurfaceId CreateSurface(
    Graphics::SurfaceFactory*      factory,
    Internal::Adaptor::WindowBase* windowBase,
    ColorDepth                     colorDepth,
    int                            width,
    int                            height) override
  {
    mCallstack.PushCall("CreateSurface()", "");
    return 0;
  }

  void DestroySurface(Graphics::SurfaceId surfaceId) override
  {
    mCallstack.PushCall("DestroySurface()", "");
  }

  bool ReplaceSurface(Graphics::SurfaceId surfaceId, int width, int height) override
  {
    mCallstack.PushCall("ReplaceSurface()", "");
    return true;
  }
  /**
   * Activate the resource context
   */
  void ActivateResourceContext() override
  {
    mCallstack.PushCall("ActivateResourceContext()", "");
  }

  /**
   * Activate the resource context
   *
   * @param[in] surface The surface whose context to be switched to.
   */
  void ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface) override
  {
    TraceCallStack::NamedParams namedParams;
    namedParams["surface"] << std::hex << surface;
    mCallstack.PushCall("ActivateResourceContext()", namedParams.str(), namedParams);
  }

  void MakeContextCurrent(Graphics::SurfaceId surfaceId) override
  {
    mCallstack.PushCall("MakeContextCurrent()", "");
  }

  void PostRender() override
  {
    mCallstack.PushCall("PostRender()", "");
  }

  /**
   * Shut down the graphics implementation
   */
  void Shutdown() override
  {
    mCallstack.PushCall("Shutdown()", "");
  }

  /**
   * Destroy the Graphics implementation
   */
  void Destroy() override
  {
    mCallstack.PushCall("Destroy()", "");
  }

  /**
   * Lifecycle event for pausing application
   */
  void Pause() override
  {
    mCallstack.PushCall("()", "");
  }

  /**
   * Lifecycle event for resuming application
   */
  void Resume() override
  {
    mCallstack.PushCall("Resume()", "");
  }

  /**
   * Get the buffer age of the surface. 0 means that the back buffer
   * is invalid and needs a full swap.
   */
  int GetBufferAge(Graphics::SurfaceId surfaceId) override
  {
    mCallstack.PushCall("GetBufferAge()", "");
    return 0;
  }

  /**
   * Set damage regions onto the surface
   * @param[in] surfaceId The surface to define damage regions for
   * @param[in] damagedRegion The damage regions
   */
  void SetDamageRegion(Graphics::SurfaceId surfaceId, std::vector<Rect<int>>& damagedRegion) override
  {
    mCallstack.PushCall("SetDamageRegion()", "");
  }

  /**
   * Swap the surface's buffers. May be done by other mechanisms, depending on
   * the graphics backend.
   */
  void SwapBuffers(Graphics::SurfaceId surfaceId) override
  {
    mCallstack.PushCall("SwapBuffers()", "");
  }
  void SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>& damageRects) override
  {
    mCallstack.PushCall("SwapBuffers()", "");
  }
  /**
   * @return true if advanced blending options are supported
   */
  bool IsAdvancedBlendEquationSupported() override
  {
    mCallstack.PushCall("IsAdvancedBlendEquationSupported()", "");
    return true;
  }

  /**
   * @return true if multisampled render to texture is supported
   */
  bool IsMultisampledRenderToTextureSupported() override
  {
    mCallstack.PushCall("IsMultisampledRenderToTextureSupported()", "");
    return true;
  }

  /**
   * @return true if graphics subsystem is initialized
   */
  bool IsInitialized() override
  {
    return true;
  }

  /**
   * @return true if a separate resource context is supported
   */
  bool IsResourceContextSupported() override
  {
    return true;
  }

  /**
   * @return the maximum texture size
   */
  uint32_t GetMaxTextureSize() override
  {
    return 32768u;
  }

  uint32_t GetMaxCombinedTextureUnits() override
  {
    return 96;
  }

  /**
   * @return the maximum texture samples when we use multisampled texture
   */
  uint8_t GetMaxTextureSamples() override
  {
    return 8u;
  }

  /**
   * @return the version number of the shader language
   */
  uint32_t GetShaderLanguageVersion() override
  {
    return 320;
  }

  void FrameStart() override
  {
  }

  void PostRenderDebug() override
  {
  }

  void LogMemoryPools() override
  {
  }

  /**
   * Store cached configurations
   */
  void CacheConfigurations(Internal::Adaptor::ConfigurationManager& configurationManager) override
  {
  }

public:
  TraceCallStack mCallstack{true, "GraphicsImpl"};
};

class DALI_CORE_API TestGraphicsApplication : public ConnectionTracker
{
public:
  // Default values derived from H2 device.
  static const uint32_t DEFAULT_SURFACE_WIDTH  = 480;
  static const uint32_t DEFAULT_SURFACE_HEIGHT = 800;

  static constexpr uint32_t DEFAULT_HORIZONTAL_DPI = 220;
  static constexpr uint32_t DEFAULT_VERTICAL_DPI   = 217;

  static const uint32_t DEFAULT_RENDER_INTERVAL = 1;

  static const uint32_t RENDER_FRAME_INTERVAL = 16;

  TestGraphicsApplication(uint32_t surfaceWidth        = DEFAULT_SURFACE_WIDTH,
                          uint32_t surfaceHeight       = DEFAULT_SURFACE_HEIGHT,
                          uint32_t horizontalDpi       = DEFAULT_HORIZONTAL_DPI,
                          uint32_t verticalDpi         = DEFAULT_VERTICAL_DPI,
                          bool     initialize          = true,
                          bool     enablePartialUpdate = false);

  void Initialize();
  void CreateCore();
  void CreateScene();
  void InitializeCore();
  ~TestGraphicsApplication() override;
  static void              LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message);
  static void              LogContext(bool start, const char* tag, const char* message);
  Dali::Integration::Core& GetCore();
  TestPlatformAbstraction& GetPlatform();
  TestRenderController&    GetRenderController();
  Graphics::Controller&    GetGraphicsController();

  TestGlAbstraction& GetGlAbstraction();

  void        ProcessEvent(const Integration::Event& event);
  void        SendNotification();
  bool        Render(uint32_t intervalMilliseconds = DEFAULT_RENDER_INTERVAL, const char* location = NULL);
  bool        PreRenderWithPartialUpdate(uint32_t intervalMilliseconds, const char* location, std::vector<Rect<int>>& damagedRects);
  bool        RenderWithPartialUpdate(std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect);
  uint32_t    GetUpdateStatus();
  bool        UpdateOnly(uint32_t intervalMilliseconds = DEFAULT_RENDER_INTERVAL);
  bool        RenderOnly();
  void        ResetContext();
  bool        GetRenderNeedsUpdate();
  bool        GetRenderNeedsPostRender();
  uint32_t    Wait(uint32_t durationToWait);
  static void EnableLogging(bool enabled)
  {
    mLoggingEnabled = enabled;
  }

  Integration::Scene GetScene() const
  {
    return mScene;
  }

private:
  void DoUpdate(uint32_t intervalMilliseconds, const char* location = NULL);

protected:
  TestPlatformAbstraction                     mPlatformAbstraction;
  TestRenderController                        mRenderController;
  Graphics::EglGraphicsController             mGraphicsController; // Use real controller in Adaptor
  TestGlAbstraction                           mGlAbstraction;
  TestGraphicsSyncImplementation              mGraphicsSyncImplementation;
  TestGraphicsImpl                            mGraphics;
  Graphics::UniquePtr<Graphics::RenderTarget> mRenderTarget{nullptr};
  Dali::DisplayConnection*                    mDisplayConnection{nullptr};

  Integration::UpdateStatus mStatus;
  Integration::RenderStatus mRenderStatus;

  Integration::Core*       mCore;
  Dali::Integration::Scene mScene;

  uint32_t mSurfaceWidth;
  uint32_t mSurfaceHeight;
  uint32_t mFrame;

  struct
  {
    uint32_t x;
    uint32_t y;
  } mDpi;
  uint32_t    mLastVSyncTime;
  bool        mPartialUpdateEnabled;
  static bool mLoggingEnabled;
};

} // namespace Dali

#endif // DALI_TEST_GRAPHICS_APPLICATION_H
