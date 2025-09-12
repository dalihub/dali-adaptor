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
#include <dali/internal/graphics/common/graphics-factory.h>
#include <dali/internal/graphics/common/surface-factory.h>
#include <dali/internal/imaging/common/native-image-source-factory.h>
#include <dali/internal/window-system/common/native-image-surface-factory.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/public-api/dali-adaptor-common.h>

extern "C" {
using namespace Dali::Internal::Adaptor;

DALI_ADAPTOR_API std::unique_ptr<GraphicsFactoryInterface> CreateGraphicsFactory(EnvironmentOptions& environmentOptions)
{
  return Dali::Internal::Adaptor::CreateGraphicsFactory(environmentOptions);
}

DALI_ADAPTOR_API std::unique_ptr<RenderSurfaceFactory> GetRenderSurfaceFactory()
{
  return Dali::Internal::Adaptor::GetRenderSurfaceFactory();
}

DALI_ADAPTOR_API std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory()
{
  return Dali::Internal::Adaptor::GetNativeImageSourceFactory();
}

DALI_ADAPTOR_API std::unique_ptr<NativeImageSurface> CreateNativeImageSurface(Dali::NativeImageSourceQueuePtr queue)
{
  return Dali::Internal::Adaptor::NativeImageSurfaceFactory::CreateNativeImageSurface(queue);
}

DALI_ADAPTOR_API std::unique_ptr<Dali::Graphics::SurfaceFactory> CreateSurfaceFactory(Dali::Graphics::NativeWindowInterface& nativeWindow)
{
  return Dali::Graphics::SurfaceFactory::New(nativeWindow);
}
} // extern "C"