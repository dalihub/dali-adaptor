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
#include <dali/internal/graphics/common/graphics-library-handle.h>
#include <dali/internal/graphics/common/surface-factory.h>
#include <dali/internal/imaging/common/native-image-source-factory.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/window-system/common/native-image-surface-factory.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

namespace Dali::Internal::Adaptor::GraphicsLibrary
{
/**
 * @brief Returns the library loaded graphics backend currently.
 * @warning We MUST NOT call this API inside of graphics library functions. It might return invalid variables.
 * @return The library loaded graphics backend currently. Or DEFAULT it library not loaded yet.
 */
Dali::Graphics::Backend GetCurrentGraphicsLibraryBackend();

/**
 * @brief Resets the graphics library.
 * @warning We MUST NOT call this API inside of graphics library functions. It might return invalid variables.
 * @param[in] reload Re-load dynamic graphics library and functions as current graphics backend. If false, reset static graphics library and do nothing.
 */
void ResetGraphicsLibrary(bool reload);

/**
 * @brief Gets the handle to the loaded graphics library.
 * @warning We MUST NOT call this API inside of graphics library functions. It might return invalid variables.
 * @return A shared pointer to the loaded graphics library handle.
 */
GraphicsLibraryHandlePtr GetGraphicsLibraryHandle();

/**
 * @brief Creates a graphics factory with the given environment options.
 * @param[in] environmentOptions The environment options for graphics configuration.
 * @return A unique pointer to the graphics factory interface.
 */
std::unique_ptr<GraphicsFactoryInterface> CreateGraphicsFactory(EnvironmentOptions& environmentOptions);

/**
 * @brief Gets the render surface factory.
 * @return A unique pointer to the render surface factory.
 */
std::unique_ptr<RenderSurfaceFactory> GetRenderSurfaceFactory();

/**
 * @brief Gets the native image source factory.
 * @return A unique pointer to the native image source factory.
 */
std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory();

/**
 * @brief Creates a surface factory for the given native window.
 * @param[in] nativeWindow The native window interface to create surface factory for.
 * @return A unique pointer to the graphics surface factory.
 */
std::unique_ptr<Graphics::SurfaceFactory> CreateSurfaceFactory(Graphics::NativeWindowInterface& nativeWindow);

/**
 * @brief Creates a native image surface with the given queue.
 * @param[in] queue The native image source queue for the surface.
 * @return A unique pointer to the native image surface.
 */
std::unique_ptr<NativeImageSurface> CreateNativeImageSurface(NativeImageSourceQueuePtr queue);

/**
 * @brief Casts a void pointer to a native graphics type.
 * @param[in] pointer The void pointer to cast.
 * @return An Any object containing the native graphics type.
 */
Any CastToNativeGraphicsType(void*);

} // namespace Dali::Internal::Adaptor::GraphicsLibrary
