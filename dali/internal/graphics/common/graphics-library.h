#pragma once

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-factory-interface.h>
#include <dali/internal/graphics/common/surface-factory.h>
#include <dali/internal/imaging/common/native-image-source-factory.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/window-system/common/native-image-surface-factory.h>
#include <dali/internal/window-system/common/render-surface-factory.h>

namespace Dali::Internal::Adaptor::GraphicsLibrary
{
std::unique_ptr<GraphicsFactoryInterface> CreateGraphicsFactory(EnvironmentOptions& environmentOptions);

std::unique_ptr<RenderSurfaceFactory> GetRenderSurfaceFactory();

std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory();

std::unique_ptr<Graphics::SurfaceFactory> CreateSurfaceFactory(Graphics::NativeWindowInterface& nativeWindow);

std::unique_ptr<NativeImageSurface> CreateNativeImageSurface(NativeImageSourceQueuePtr queue);

Any CastToNativeGraphicsType(void*);

} // namespace Dali::Internal::Adaptor::GraphicsLibrary
