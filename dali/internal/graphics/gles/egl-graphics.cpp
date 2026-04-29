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

// CLASS HEADER
#include <dali/internal/graphics/gles/egl-graphics.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/internal/system/common/configuration-manager.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/window-system/common/display-utils.h> // For Utils::MakeUnique
#include <dali/internal/window-system/common/window-base.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
EglGraphics::EglGraphics(
  EnvironmentOptions&                 environmentOptions,
  const Graphics::GraphicsCreateInfo& info,
  Integration::DepthBufferAvailable   depthBufferRequired,
  Integration::StencilBufferAvailable stencilBufferRequired,
  Integration::PartialUpdateAvailable partialUpdateRequired,
  int                                 multiSamplingLevel,
  Dali::Graphics::ContextPriority     contextPriority)
: GraphicsInterface(info, depthBufferRequired, stencilBufferRequired, partialUpdateRequired, multiSamplingLevel, contextPriority),
  mForcePresentRequired(false)
{
  if(environmentOptions.GetGlesCallTime() > 0)
  {
    mGLES = Utils::MakeUnique<GlProxyImplementation>(environmentOptions);
  }
  else
  {
    mGLES.reset(new GlImplementation());
  }

  mGraphicsController.InitializeGLES(*mGLES.get());
}

EglGraphics::~EglGraphics()
{
}

void EglGraphics::SetIsSurfacelessContextSupported(const bool isSupported)
{
  mGLES->SetIsSurfacelessContextSupported(isSupported);
}

void EglGraphics::ActivateResourceContext()
{
  if(mEglImplementation && mEglImplementation->IsSurfacelessContextSupported())
  {
    // Make the shared surfaceless context as current before rendering
    mEglImplementation->MakeContextCurrent(EGL_NO_SURFACE, mEglImplementation->GetContext());
  }

  mGraphicsController.ActivateResourceContext();
}

void EglGraphics::ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface)
{
  if(surface)
  {
    surface->InitializeGraphics();
    surface->MakeContextCurrent();
  }

  mGraphicsController.ActivateSurfaceContext(surface);
}

void EglGraphics::MakeContextCurrent(Graphics::SurfaceId surfaceId)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return;
  }

  mEglImplementation->MakeContextCurrent(search->second.surface, search->second.context);
}

void EglGraphics::AcquireNextImage(Integration::RenderSurfaceInterface* surface)
{
  // Nothing to do here (SwapBuffers does this under the hood)
}

void EglGraphics::PostRender()
{
  // Restore to resource context for mGraphicsController.PostRender()
  ActivateResourceContext();

  mGraphicsController.PostRender();
}

void EglGraphics::Pause()
{
}

void EglGraphics::Resume()
{
  if(mEglImplementation)
  {
    mEglImplementation->SetFirstFrameAfterResume();
  }
}

void EglGraphics::Resize(Integration::RenderSurfaceInterface* surface, SurfaceSize size)
{
  if(surface)
  {
    surface->Resize(size);
  }
}

int EglGraphics::GetBufferAge(Graphics::SurfaceId surfaceId)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return 0;
  }

  mForcePresentRequired = true; // Force present required after eglQuerySurface
  return mEglImplementation->GetBufferAge(search->second.surface);
}

void EglGraphics::SetDamageRegion(Graphics::SurfaceId surfaceId, std::vector<Rect<int>>& damagedRegion)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return;
  }

  mEglImplementation->SetDamageRegion(search->second.surface, damagedRegion);
}

void EglGraphics::SwapBuffers(Graphics::SurfaceId surfaceId)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return;
  }

  mEglImplementation->SwapBuffers(search->second.surface);
}

void EglGraphics::SwapBuffers(Graphics::SurfaceId surfaceId, const std::vector<Rect<int>>& damagedRegion)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return;
  }

  mEglImplementation->SwapBuffers(search->second.surface, damagedRegion);
}

void EglGraphics::Initialize(const Dali::DisplayConnection& displayConnection)
{
  EglInitialize();

  // Sync and context helper require EGL to be initialized first (can't execute in the constructor)
  mGraphicsController.Initialize(*mEglSync.get(), *this);
  InitializeGraphicsAPI(displayConnection);
}

void EglGraphics::Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa, Dali::Graphics::ContextPriority contextPriority)
{
  GraphicsInterface::UpdateGraphicsRequired(static_cast<Integration::DepthBufferAvailable>(depth),
                                            static_cast<Integration::StencilBufferAvailable>(stencil),
                                            static_cast<Integration::PartialUpdateAvailable>(partialRendering),
                                            msaa,
                                            contextPriority);

  // Do not initialize graphics controller for this case.
  EglInitialize();
  InitializeGraphicsAPI(displayConnection);
}

void EglGraphics::InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection)
{
  auto display    = displayConnection.GetNativeGraphicsDisplay();
  auto eglDisplay = display.Get<EGLNativeDisplayType>();
  mEglImplementation->InitializeGles(eglDisplay);
}

Dali::Any EglGraphics::GetDisplay() const
{
  return {mEglImplementation->GetDisplay()};
}

void EglGraphics::EglInitialize()
{
  mEglSync            = Utils::MakeUnique<EglSyncImplementation>();
  mEglImplementation  = Utils::MakeUnique<EglImplementation>(mMultiSamplingLevel, mDepthBufferRequired, mStencilBufferRequired, mPartialUpdateRequired, mContextPriority);
  mEglImageExtensions = Utils::MakeUnique<EglImageExtensions>(mEglImplementation.get());

  mEglSync->Initialize(mEglImplementation.get()); // The sync impl needs the EglDisplay
}

Graphics::SurfaceId EglGraphics::CreateSurface(Graphics::SurfaceFactory* surfaceFactory, WindowBase* windowBase, ColorDepth colorDepth, int width, int height)
{
  // Use global defaults for depth/stencil/MSAA
  return CreateSurface(surfaceFactory, windowBase, colorDepth, width, height,
                       mDepthBufferRequired == Integration::DepthBufferAvailable::TRUE,
                       mStencilBufferRequired == Integration::StencilBufferAvailable::TRUE,
                       mMultiSamplingLevel);
}

Graphics::SurfaceId EglGraphics::CreateSurface(Graphics::SurfaceFactory* surfaceFactory, WindowBase* windowBase, ColorDepth colorDepth, int width, int height, bool depthBufferRequired, bool stencilBufferRequired, int multiSamplingLevel)
{
  // Choose config with per-surface depth/stencil/MSAA settings
  mEglImplementation->ChooseConfig(true, colorDepth, depthBufferRequired, stencilBufferRequired, multiSamplingLevel);
  EGLConfig config = mEglImplementation->GetConfig();

  // Create the OpenGL context for this window using the chosen config
  EGLContext context = 0;
  mEglImplementation->CreateWindowContext(context, config);

  auto window     = windowBase->CreateWindow(width, height);
  auto eglWindow  = reinterpret_cast<EGLNativeWindowType>(window.Get<void*>());
  auto eglSurface = mEglImplementation->CreateSurfaceWindow(eglWindow, colorDepth, config);

  auto surfaceId         = ++mBaseSurfaceId;
  mSurfaceMap[surfaceId] = EglSurfaceContext{windowBase, eglSurface, context, config, eglWindow, depthBufferRequired, stencilBufferRequired, multiSamplingLevel};

  return surfaceId;
}

bool EglGraphics::ReconfigureSurface(Graphics::SurfaceId surfaceId, bool depthBufferRequired, bool stencilBufferRequired, int multiSamplingLevel)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    return false;
  }

  auto& entry = search->second;

  // Check if config actually changed
  if(entry.depthBuffer == depthBufferRequired &&
     entry.stencilBuffer == stencilBufferRequired &&
     entry.msaaLevel == multiSamplingLevel)
  {
    return false; // No change needed
  }

  // Destroy old EGL surface and context only.
  // The native window is independent of the EGL config and can be reused.
  mEglImplementation->DestroySurface(entry.surface);
  mEglImplementation->DestroyContext(entry.context);

  // Choose new config with updated depth/stencil/MSAA settings
  if(!mEglImplementation->ChooseConfig(true, COLOR_DEPTH_32, depthBufferRequired, stencilBufferRequired, multiSamplingLevel))
  {
    DALI_LOG_ERROR("Failed to choose EGL config for reconfigured surface\n");
    return false;
  }
  EGLConfig newConfig = mEglImplementation->GetConfig();

  // Create new context with new config, sharing with the resource context
  EGLContext newContext = 0;
  mEglImplementation->CreateWindowContext(newContext, newConfig);

  // Reuse the cached native window handle from initial CreateSurface.
  // We must NOT call windowBase->GetNativeWindow() here because it may
  // return a different value than what CreateWindow() returned (e.g. on
  // X11 where GetNativeWindow returns the raw ecore window but CreateWindow
  // widens it to a 64-bit X11 Window handle).
  // We also must NOT call windowBase->CreateWindow() again because that
  // may allocate new resources (e.g. a wl_egl_window on Wayland). The
  // existing native window handle is still valid and can be reused with
  // a new EGL surface and context.
  auto newSurface = mEglImplementation->CreateSurfaceWindow(entry.nativeWindow, COLOR_DEPTH_32, newConfig);

  // Update map entry
  entry.surface       = newSurface;
  entry.context       = newContext;
  entry.config        = newConfig;
  entry.depthBuffer   = depthBufferRequired;
  entry.stencilBuffer = stencilBufferRequired;
  entry.msaaLevel     = multiSamplingLevel;

  return true;
}

void EglGraphics::ResetSurfaceState()
{
  // Called after ReconfigureSurface has destroyed and recreated the window's
  // EGL context. Shareable resources (textures, buffers, programs) survive via
  // the share group, but VAOs/FBOs/queries/program-pipelines do not — drop any
  // cached ids for those before issuing any further GL calls, otherwise the
  // next BindVertexArray will bind an id the driver no longer owns and draws
  // will silently produce no geometry.
  auto* currentContext = mGraphicsController.GetCurrentContext();
  if(currentContext)
  {
    currentContext->DiscardNonShareableCache();
    currentContext->ResetGLESState(true);
  }
}

void EglGraphics::DestroySurface(Graphics::SurfaceId surfaceId)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return;
  }

  mEglImplementation->DestroySurface(search->second.surface);
  mEglImplementation->DestroyContext(search->second.context);
  search->second.windowBase->DestroyWindow();
  mSurfaceMap.erase(search);
}

bool EglGraphics::ReplaceSurface(Graphics::SurfaceId surfaceId, int width, int height)
{
  auto search = mSurfaceMap.find(surfaceId);
  if(DALI_UNLIKELY(search == mSurfaceMap.end()))
  {
    DALI_LOG_ERROR("Invalid surface id [%u] used! Ignore\n", surfaceId);
    DALI_ASSERT_DEBUG(0 && "Invalid srufaceId");
    return false;
  }

  auto& windowBase = search->second.windowBase;

  // Destroy the old graphics window
  windowBase->DestroyWindow();

  // Create the EGL window
  Dali::Any window = windowBase->CreateWindow(width, height);

  EGLNativeWindowType eglWindow = window.Get<EGLNativeWindowType>();
  auto&               context   = search->second.context;
  auto&               surface   = search->second.surface;

  // Update cached native window handle since we destroyed and recreated it
  search->second.nativeWindow = eglWindow;

  return mEglImplementation->ReplaceSurfaceWindow(eglWindow, surface, context); // Should update the map.
}

void EglGraphics::ConfigureSurface(Dali::Integration::RenderSurfaceInterface* surface)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "EGLImplementation not created");

  // Try to use OpenGL es 3.0
  // ChooseConfig returns false here when the device only support gles 2.0.
  // Because eglChooseConfig with gles 3.0 setting fails when the device only support gles 2.0 and Our default setting is gles 3.0.
  if(!mEglImplementation->ChooseConfig(true, COLOR_DEPTH_32))
  {
    // Retry to use OpenGL es 2.0
    mEglImplementation->SetGlesVersion(20);

    // Mark gles that we will use gles 2.0 version.
    // After this call, we will not change mGLES version anymore.
    mGLES->SetGlesVersion(20);

    mEglImplementation->ChooseConfig(true, COLOR_DEPTH_32);
  }

  // Check whether surfaceless context is supported
  bool isSurfacelessContextSupported = mEglImplementation->IsSurfacelessContextSupported();
  SetIsSurfacelessContextSupported(isSurfacelessContextSupported);

  Integration::RenderSurfaceInterface* currentSurface = nullptr;
  if(isSurfacelessContextSupported)
  {
    // Create a surfaceless OpenGL context for shared resources
    mEglImplementation->CreateContext();
    ActivateResourceContext();
  }
  else
  {
    currentSurface = surface;
    if(currentSurface)
    {
      ActivateSurfaceContext(currentSurface);
    }
  }

  mGLES->ContextCreated(); // After this call, we can know exact gles version.
  auto glesVersion = mGLES->GetGlesVersion();

  // Set more detail GLES version to egl and graphics controller.
  // Note. usually we don't need EGL client's minor version. So don't need to choose config one more time.
  mEglImplementation->SetGlesVersion(glesVersion);
  mGraphicsController.SetGLESVersion(static_cast<Graphics::GLES::GLESVersion>(glesVersion));
}

void EglGraphics::Shutdown()
{
  if(mEglImplementation)
  {
    // Shutdown controller
    mGraphicsController.Shutdown();

    // Terminate GLES
    mEglImplementation->TerminateGles();
  }
}

void EglGraphics::Destroy()
{
  // Destroy only if Initialize called before.
  if(DALI_LIKELY(mEglImplementation))
  {
    mGraphicsController.Destroy();
  }
}

Integration::GlAbstraction& EglGraphics::GetGlAbstraction() const
{
  DALI_ASSERT_DEBUG(mGLES && "GLImplementation not created");
  return *mGLES;
}

EglImplementation& EglGraphics::GetEglImplementation() const
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "EGLImplementation not created");
  return *mEglImplementation;
}

EglInterface& EglGraphics::GetEglInterface() const
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "EGLImplementation not created");
  EglInterface* eglInterface = mEglImplementation.get();
  return *eglInterface;
}

EglSyncImplementation& EglGraphics::GetSyncImplementation()
{
  DALI_ASSERT_DEBUG(mEglSync && "EglSyncImplementation not created");
  return *mEglSync;
}

EglImageExtensions* EglGraphics::GetImageExtensions()
{
  DALI_ASSERT_DEBUG(mEglImageExtensions && "EglImageExtensions not created");
  return mEglImageExtensions.get();
}

Graphics::Controller& EglGraphics::GetController()
{
  return mGraphicsController;
}

void EglGraphics::CacheConfigurations(ConfigurationManager& configurationManager)
{
  mGLES->SetIsAdvancedBlendEquationSupported(configurationManager.IsAdvancedBlendEquationSupported());
  mGLES->SetIsMultisampledRenderToTextureSupported(configurationManager.IsMultisampledRenderToTextureSupported());
  mGLES->SetShadingLanguageVersion(configurationManager.GetShadingLanguageVersion());
}

void EglGraphics::FrameStart()
{
  mGraphicsController.FrameStart();
}

void EglGraphics::RenderStart()
{
  // Do nothing
}

bool EglGraphics::ForcePresentRequired()
{
  return mForcePresentRequired;
}

bool EglGraphics::DidPresent()
{
  const bool didPresent = mGraphicsController.DidPresent();
  mGraphicsController.ResetDidPresent();
  mForcePresentRequired = false;
  return didPresent;
}

void EglGraphics::PostRenderDebug()
{
  mGLES->PostRender();
}

void EglGraphics::LogMemoryPools()
{
  std::size_t graphicsCapacity = mGraphicsController.GetCapacity();
  DALI_LOG_RELEASE_INFO(
    "EglGraphics:\n"
    "  GraphicsController Capacity: %lu\n",
    graphicsCapacity);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
