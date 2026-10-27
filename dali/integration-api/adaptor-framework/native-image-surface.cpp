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
#include <dali/integration-api/adaptor-framework/native-image-surface.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-factory.h> ///< For Dali::Internal::Adaptor::GetGraphicsBackendModule
#include <dali/internal/window-system/common/native-image-surface-factory.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>

namespace DALI_NAMESPACE
{
NativeImageSurfacePtr NativeImageSurface::New(Dali::NativeImageQueuePtr queue)
{
  NativeImageSurfacePtr surface = new NativeImageSurface(queue);
  if(surface->mImpl)
  {
    return surface;
  }
  return nullptr;
}

Any NativeImageSurface::GetNativeRenderable()
{
  return mImpl->GetNativeRenderable();
}

void NativeImageSurface::InitializeGraphics()
{
  mImpl->InitializeGraphics();
}

void NativeImageSurface::TerminateGraphics()
{
  mImpl->TerminateGraphics();
}

void NativeImageSurface::PreRender()
{
  mImpl->PreRender();
}

void NativeImageSurface::PostRender()
{
  mImpl->PostRender();
}

bool NativeImageSurface::CanRender()
{
  return mImpl->CanRender();
}

bool NativeImageSurface::SetGraphicsConfig(bool depth, bool stencil, int msaa, int version)
{
  return mImpl->SetGraphicsConfig(depth, stencil, msaa, version);
}

NativeImageSurface::NativeImageSurface(Dali::NativeImageQueuePtr queue)
{
  mImpl = Dali::Internal::Adaptor::NativeImageSurfaceFactory::CreateNativeImageSurface(queue);

  if(mImpl)
  {
    // For the dynamic backend case, mImpl's code lives in the backend library; keep it loaded
    // while mImpl is alive, even if this object outlives the Adaptor. The module must be
    // fetched here, as the backend library binds to its own copy of the accessor.
    mImpl->SetGraphicsLibrary(Dali::Internal::Adaptor::GetGraphicsBackendModule());
  }
}

NativeImageSurface::~NativeImageSurface()
{
  // Take a copy so the backend library, where mImpl's code lives, is unloaded only after
  // the reset below has completed.
  std::shared_ptr<void> graphicsLibrary = mImpl ? mImpl->GetGraphicsLibrary() : nullptr;
  mImpl.reset();
}

} //namespace DALI_NAMESPACE
