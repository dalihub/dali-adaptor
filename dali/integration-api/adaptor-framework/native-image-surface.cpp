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

// CLASS HEADER
#include <dali/integration-api/adaptor-framework/native-image-surface.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/native-image-surface-factory.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>

namespace Dali
{
NativeImageSurfacePtr NativeImageSurface::New(Dali::NativeImageSourceQueuePtr queue)
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

NativeImageSurface::NativeImageSurface(Dali::NativeImageSourceQueuePtr queue)
{
  mImpl = Dali::Internal::Adaptor::NativeImageSurfaceFactory::CreateNativeImageSurface(queue);
}

NativeImageSurface::~NativeImageSurface()
{
}

} // namespace Dali
