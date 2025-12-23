#ifndef DALI_OFFSCREEN_WINDOW_H
#define DALI_OFFSCREEN_WINDOW_H

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
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class Actor;
class Layer;

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class OffscreenWindow;
} // namespace Adaptor
} //namespace Internal DALI_INTERNAL

/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief OffscreenWindow provides a window-like interface for offscreen rendering.
 *
 * IMPORTANT: This is NOT a real native window! OffscreenWindow is a rendering surface
 * that uses framebuffer objects (FBO) to render UI content offscreen without creating
 * an actual visible window on the screen.
 *
 * IMPORTANT: You must call SetNativeImage() to set the render target BEFORE calling
 * OffscreenApplication::Start(). The native image source determines where the
 * rendered content will be stored.
 *
 * Usage Example:
 * @code
 * auto window = OffscreenWindow.New();
 * window.SetBackgroundColor(Color::WHITE);
 *
 * // IMPORTANT: Set native image source BEFORE starting the application
 * NativeImageSourcePtr nativeImage = NativeImageSource::New(width, height);
 * window.SetNativeImage(nativeImage);
 *
 * // Now start the application
 * app.Start();
 *
 * // Add UI elements
 * Actor actor = Actor::New();
 * window.Add(actor);
 * @endcode
 */
class DALI_ADAPTOR_API OffscreenWindow : public Dali::BaseHandle
{
public:
  using WindowSize = Uint16Pair;

public:
  /**
   * @brief Creates an initialized handle to a new OffscreenWindow
   *
   * Creates a new offscreen rendering surface. This does NOT create a visible window
   * but instead sets up a framebuffer object for offscreen rendering.
   *
   * @note You should hold the returned handle. If you miss the handle, the OffscreenWindow will be released
   * @return A new OffscreenWindow instance ready for offscreen rendering
   */
  static OffscreenWindow New();

  /**
   * @brief Constructs an empty handle
   *
   * Creates an uninitialized OffscreenWindow handle. This is typically used
   * for variable declaration before assignment.
   */
  OffscreenWindow();

  /**
   * @brief Copy constructor
   *
   * Creates a new OffscreenWindow handle that references the same underlying
   * offscreen rendering surface as the source handle.
   *
   * @param [in] window A reference to the copied handle
   */
  OffscreenWindow(const OffscreenWindow& window);

  /**
   * @brief Assignment operator
   *
   * Assigns this handle to reference the same underlying offscreen rendering
   * surface as the source handle.
   *
   * @param [in] window A reference to the copied handle
   * @return A reference to this handle
   */
  OffscreenWindow& operator=(const OffscreenWindow& window);

  /**
   * @brief Move constructor
   *
   * Moves the ownership of the offscreen rendering surface from the source handle.
   *
   * @param [in] window A reference to the moved handle
   */
  OffscreenWindow(OffscreenWindow&& window) noexcept;

  /**
   * @brief Move assignment operator
   *
   * Moves the ownership of the offscreen rendering surface from the source handle.
   *
   * @param [in] window A reference to the moved handle
   * @return A reference to this handle
   */
  OffscreenWindow& operator=(OffscreenWindow&& window) noexcept;

  /**
   * @brief Destructor
   *
   * Cleans up the OffscreenWindow handle. The underlying offscreen rendering
   * surface is only destroyed when all handles to it are destroyed.
   */
  ~OffscreenWindow();

public:
  /**
   * @brief Sets the native image source for capturing rendered content
   *
   * This method allows you to specify a NativeImageSource where the offscreen
   * rendered content will be stored.
   *
   * IMPORTANT: This method MUST be called BEFORE OffscreenApplication::Start() to
   * ensure the render target is properly configured. The native image source
   * determines where the rendered content will be stored.
   *
   * @param [in] nativeImage Shared pointer to the native image source that will receive the rendered content
   */
  void SetNativeImage(NativeImageSourcePtr nativeImage);

  /**
   * @brief Adds a child Actor to the OffscreenWindow.
   *
   * @param[in] actor The child Actor to add to the offscreen window
   * @pre The actor has been initialized.
   */
  void Add(Actor actor);

  /**
   * @brief Removes a child Actor from the OffscreenWindow.
   *
   * Removes an Actor from the offscreen rendering hierarchy. The actor will no
   * longer be rendered to the offscreen framebuffer.
   *
   * @param[in] actor The child Actor to remove from the offscreen window
   */
  void Remove(Actor actor);

  /**
   * @brief Sets the background color of the OffscreenWindow.
   *
   * Sets the clear color that will be used to fill the offscreen framebuffer
   * before rendering the UI elements.
   *
   * @param[in] color The new background color (RGBA format)
   */
  void SetBackgroundColor(const Vector4& color);

  /**
   * @brief Gets the background color of the OffscreenWindow.
   *
   * Returns the current background color used for clearing the offscreen framebuffer.
   *
   * @return The background color (RGBA format)
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @brief Returns the root Layer of the OffscreenWindow.
   *
   * Every OffscreenWindow has a root layer that serves as the container for all
   * UI elements. This method returns access to that root layer for advanced
   * layer manipulation.
   *
   * @return The root layer of the offscreen window
   */
  Layer GetRootLayer() const;

  /**
   * @brief Returns the size of the OffscreenWindow in pixels.
   *
   * This size determines the dimensions of the offscreen framebuffer.
   *
   * @return The size of the OffscreenWindow as a Uint16Pair (width, height)
   */
  WindowSize GetSize() const;

  /**
   * @brief Adds a callback to be executed synchronously after each render frame
   *
   * This allows you to register a callback that will be called after the offscreen
   * rendering is complete for each frame. Useful for post-processing operations,
   * capturing rendered content, or triggering other actions based on render completion.
   *
   * IMPORTANT: This is a synchronous call where the engine's internal render thread
   * stops and waits for the callback execution to complete. This means the callback
   * should be kept as short and efficient as possible to avoid blocking the rendering
   * pipeline.
   *
   * Note: GPU rendering completion is guaranteed before the callback is invoked.
   * This ensures that all rendered content is fully available and accessible when
   * the callback executes.
   *
   * A callback of the following type should be used:
   * @code
   *   void MyFunction(OffscreenWindow window);
   * @endcode
   *
   * @param [in] callback Unique pointer to the callback base object to be executed post-render
   */
  void AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback);

  /**
   * @brief Adds a callback to be executed asynchronously after each render frame
   *
   * This allows you to register a callback that will be called asynchronously after the offscreen
   * rendering is complete for each frame. Unlike the synchronous version, this method does not block the render thread.
   *
   * The callback receives a fence file descriptor that can be used to verify GPU rendering completion.
   * The user is responsible for closing the file descriptor when it's no longer needed.
   *
   * A callback of the following type should be used:
   * @code
   *   void MyFunction(OffscreenWindow window, int32_t fenceFd);
   *   // fenceFd: File descriptor for the fence to verify GPU rendering completion
   *   // IMPORTANT: User must call close(fenceFd) when done with the fence
   * @endcode
   *
   * @param [in] callback Unique pointer to the callback base object to be executed post-render
   */
  void AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback);

public: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL OffscreenWindow(Internal::Adaptor::OffscreenWindow* window);
};

/**
 * @}
 */

} // namespace Dali

#endif // DALI_OFFSCREEN_WINDOW_H
