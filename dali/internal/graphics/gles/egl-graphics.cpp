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
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/window-system/common/display-utils.h> // For Utils::MakeUnique
#include <X11/Xlib.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gEglLogFilter;
#endif

namespace Dali::Internal::Adaptor
{

EglGraphics::EglGraphics(EnvironmentOptions& environmentOptions, GraphicsCreateInfo createInfo)
: GraphicsInterface(createInfo,
                    static_cast<Integration::DepthBufferAvailable>(environmentOptions.DepthBufferRequired()),
                    static_cast<Integration::StencilBufferAvailable>(environmentOptions.StencilBufferRequired())),
  mMultiSamplingLevel(environmentOptions.GetMultiSamplingLevel())
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
  DALI_LOG_TRACE_METHOD(gEglLogFilter);

  if(mEglImplementation && mEglImplementation->IsSurfacelessContextSupported())
  {
    // Make the shared surfaceless context as current before rendering
    mEglImplementation->MakeContextCurrent(EGL_NO_SURFACE, mEglImplementation->GetContext());
  }

  mGraphicsController.ActivateResourceContext();
}

void EglGraphics::ActivateSurfaceContext(Dali::RenderSurfaceInterface* surface)
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);

  if(surface)
  {
    surface->InitializeGraphics(*this);
    surface->MakeContextCurrent();
  }

  mGraphicsController.ActivateSurfaceContext(surface);
}

void EglGraphics::PostRender()
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
  ActivateResourceContext();

  if(mGraphicsController.GetCurrentContext())
  {
    mGraphicsController.GetCurrentContext()->InvalidateDepthStencilBuffers();
  }

  mGraphicsController.PostRender();
}

void EglGraphics::SetFirstFrameAfterResume()
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
  if(mEglImplementation)
  {
    mEglImplementation->SetFirstFrameAfterResume();
  }
}

void EglGraphics::Initialize(const Dali::DisplayConnection& displayConnection)
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
  EglInitialize();

  // Sync and context helper require EGL to be initialized first (can't execute in the constructor)
  mGraphicsController.Initialize(*this);
  InitializeGraphicsAPI(displayConnection);
}

void EglGraphics::Initialize(const Dali::DisplayConnection& displayConnection, bool depth, bool stencil, bool partialRendering, int msaa)
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
  mDepthBufferRequired   = static_cast<Integration::DepthBufferAvailable>(depth);
  mStencilBufferRequired = static_cast<Integration::StencilBufferAvailable>(stencil);
  mMultiSamplingLevel    = msaa;

  EglInitialize();
  InitializeGraphicsAPI(displayConnection);
}

void EglGraphics::InitializeGraphicsAPI(const Dali::DisplayConnection& displayConnection)
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
  // Bad name - it does call "eglInitialize"!!!! @todo Rename me!
  mEglImplementation->InitializeGles(displayConnection.GetDisplay().Get<::Display*>());
}

void EglGraphics::EglInitialize()
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
  mEglSync            = Utils::MakeUnique<EglSyncImplementation>();
  mEglImplementation  = Utils::MakeUnique<EglImplementation>(mMultiSamplingLevel, mDepthBufferRequired, mStencilBufferRequired);
  mEglImageExtensions = Utils::MakeUnique<EglImageExtensions>(mEglImplementation.get());

  mEglSync->Initialize(mEglImplementation.get()); // The sync impl needs the EglDisplay
}

void EglGraphics::ConfigureSurface(Dali::RenderSurfaceInterface* surface)
{
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
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

  RenderSurfaceInterface* currentSurface = nullptr;
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
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
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
  DALI_LOG_TRACE_METHOD(gEglLogFilter);
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
}

void EglGraphics::FrameStart()
{
  mGraphicsController.FrameStart();
}

void EglGraphics::LogMemoryPools()
{
  std::size_t graphicsCapacity = mGraphicsController.GetCapacity();
  DALI_LOG_RELEASE_INFO(
    "EglGraphics:\n"
    "  GraphicsController Capacity: %lu\n",
    graphicsCapacity);
}

} // namespace Dali::Internal::Adaptor
