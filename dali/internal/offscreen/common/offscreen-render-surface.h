#ifndef DALI_INTERNAL_OFFSCREEN_COMMON_OFFSCREEN_RENDER_SURFACE_H
#define DALI_INTERNAL_OFFSCREEN_COMMON_OFFSCREEN_RENDER_SURFACE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/render-tasks/render-task.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/public-api/images/native-image-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Internal interface for offscreen render surface implementation
 *
 * This is an abstract base class that defines the interface for offscreen rendering
 * surfaces. Unlike regular render surfaces that create visible windows,
 * OffscreenRenderSurface implementations use graphics API-specific
 * render targets (such as OpenGL framebuffer objects or Vulkan render passes)
 * to render content to offscreen buffers.
 *
 * Key characteristics:
 * - Does NOT create actual native window surfaces
 * - Uses graphics API-specific render targets for all rendering operations
 *   (OpenGL FBOs, Vulkan render passes, etc.)
 * - Provides a bridge between the high-level offscreen window API and low-level rendering
 *
 * Implementation classes must provide concrete implementations of all virtual methods
 * to handle platform-specific and graphics API-specific offscreen rendering details.
 */
class DALI_ADAPTOR_API OffscreenRenderSurface : public Dali::Integration::RenderSurfaceInterface
{
public:
  /**
   * @brief Default constructor
   */
  OffscreenRenderSurface() = default;

  /**
   * @brief Destructor
   */
  virtual ~OffscreenRenderSurface() = default;

public: // API
  /**
   * @brief Called when the adaptor is set for this render surface
   *
   * This method is invoked by the framework when the render surface is associated
   * with a render task. Implementations should use this opportunity to initialize
   * any adaptor-dependent resources and establish the connection between the
   * render task and the offscreen render target.
   *
   * @param [in] renderTask The default render task that will use this offscreen surface
   */
  virtual void OnAdaptorSet(Dali::RenderTask renderTask) = 0;

  /**
   * @brief Sets the native image interface to be rendered into
   *
   * Configures the offscreen surface to render into the provided native image interface.
   * This allows the rendered content to be accessed as pixel data for further
   * processing, saving to files, or other programmatic uses.
   *
   * The implementation should configure the graphics API render target to render into
   * the native image's underlying texture or buffer.
   *
   * @param [in] nativeImage Shared pointer to the native image interface that will receive rendered pixels
   */
  virtual void SetNativeImage(Dali::NativeImageInterfacePtr nativeImage) = 0;

  /**
   * @brief Gets the currently set native image interface
   *
   * Returns the native image interface that is currently set as the render target
   * for this offscreen surface.
   *
   * @return Shared pointer to the currently set native image interface
   */
  virtual Dali::NativeImageInterfacePtr GetNativeImage() const = 0;

  /**
   * @brief Adds a callback to be executed synchronously after each render frame completes
   *
   * Registers a post-render callback that will be invoked after the offscreen
   * rendering for a frame is complete.
   *
   * @param [in] callback Unique pointer to the callback object to be executed post-render
   */
  virtual void AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback) = 0;

  /**
   * @brief Adds a callback to be executed asynchronously after each render frame completes
   *
   * Registers a post-render callback that will be invoked asynchronously after the offscreen
   * rendering for a frame is complete. Unlike the synchronous version, this method does not block the render thread.
   *
   * The callback function signature should be: void MyFunction(int fenceFd)
   * Where fenceFd is the file descriptor for the fence that the user can use to verify GPU rendering completion.
   * The user is responsible for closing the file descriptor when done.
   *
   * @param [in] callback Unique pointer to the callback object to be executed post-render
   */
  virtual void AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback) = 0;

protected:
  // Undefined
  OffscreenRenderSurface(const OffscreenRenderSurface&) = delete;

  // Undefined
  OffscreenRenderSurface& operator=(const OffscreenRenderSurface& rhs) = delete;
}; // class OffscreenRenderSurface

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_COMMON_OFFSCREEN_RENDER_SURFACE_H
