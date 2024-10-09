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
  Integration::PartialUpdateAvailable partialUpdateRequired)
: GraphicsInterface(info, depthBufferRequired, stencilBufferRequired, partialUpdateRequired),
  mMultiSamplingLevel(info.multiSamplingLevel)
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

void EglGraphics::PostRender()
{
  ActivateResourceContext();

  if(mGraphicsController.GetCurrentContext())
  {
    mGraphicsController.GetCurrentContext()->InvalidateDepthStencilBuffers();
  }

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

void EglGraphics::Resize(Integration::RenderSurfaceInterface* surface, Uint16Pair size)
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

void EglGraphics::Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa)
{
  mDepthBufferRequired   = static_cast<Integration::DepthBufferAvailable>(depth);
  mStencilBufferRequired = static_cast<Integration::StencilBufferAvailable>(stencil);
  mPartialUpdateRequired = static_cast<Integration::PartialUpdateAvailable>(partialRendering);
  mMultiSamplingLevel    = msaa;

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
  mEglImplementation  = Utils::MakeUnique<EglImplementation>(mMultiSamplingLevel, mDepthBufferRequired, mStencilBufferRequired, mPartialUpdateRequired);
  mEglImageExtensions = Utils::MakeUnique<EglImageExtensions>(mEglImplementation.get());

  mEglSync->Initialize(mEglImplementation.get()); // The sync impl needs the EglDisplay
}

Graphics::SurfaceId EglGraphics::CreateSurface(Graphics::SurfaceFactory* surfaceFactory, WindowBase* windowBase, ColorDepth colorDepth, int width, int height)
{
  // Create the OpenGL context for this window
  mEglImplementation->ChooseConfig(true, colorDepth);
  EGLContext context = 0;
  mEglImplementation->CreateWindowContext(context);

  auto window     = windowBase->CreateWindow(width, height);
  auto eglWindow  = reinterpret_cast<EGLNativeWindowType>(window.Get<void*>());
  auto eglSurface = mEglImplementation->CreateSurfaceWindow(eglWindow, colorDepth);

  auto surfaceId         = ++mBaseSurfaceId;
  mSurfaceMap[surfaceId] = EglSurfaceContext{windowBase, eglSurface, context};

  return surfaceId;
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
  mGraphicsController.Destroy();
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
