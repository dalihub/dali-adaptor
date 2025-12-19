#ifndef DALI_OFFSCREEN_APPLICATION_H
#define DALI_OFFSCREEN_APPLICATION_H

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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/offscreen-window.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class OffscreenApplication;
} // namespace Adaptor
} //namespace Internal DALI_INTERNAL

/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief OffscreenApplication enables rendering UI to an offscreen buffer without creating a visible window.
 *
 * This class is designed for applications that need to render UI content in the background without displaying it on screen.
 *
 * The OffscreenApplication uses framebuffer objects (FBO) for rendering instead of
 * creating actual native windows, making it ideal for batch processing and automated tasks.
 *
 * IMPORTANT: Unlike Dali::Application, OffscreenApplication does NOT have its own event loop.
 * The application must create and run its own event loop to process events and rendering.
 * This gives developers full control over the application lifecycle and event processing.
 *
 * Usage Example:
 * @code
 * int argc = 0;
 * char* argv[] = { nullptr };
 * auto app = OffscreenApplication::New(&argc, &argv,
 *                                      OffscreenApplication::FrameworkBackend::ECORE,
 *                                      OffscreenApplication::RenderMode::AUTO);
 * app.Start();
 *
 * // Create and configure UI
 * auto window = app.GetWindow();
 * // ... Add UI elements to window ...
 *
 * // Run your own event loop
 * while (applicationRunning)
 * {
 *   // Process events
 *   // For AUTO mode, rendering happens automatically
 *   // For MANUAL mode, call app.RenderOnce() when needed
 *
 *   // Your custom application logic here
 * }
 *
 * app.Terminate();
 * @endcode
 */
class DALI_ADAPTOR_API OffscreenApplication : public Dali::BaseHandle
{
public:
  /**
   * @brief Enumeration for the framework backend to use
   *
   * Specifies the underlying event loop framework that will be used
   * for the offscreen application.
   */
  enum class FrameworkBackend
  {
    ECORE, ///< Use Ecore event loop
    GLIB   ///< Use GLib event loop
  };

  /**
   * @brief Enumeration for the render mode
   *
   * Controls how the scene is rendered in the offscreen buffer.
   */
  enum class RenderMode
  {
    AUTO,  ///< Scene is rendered automatically in a continuous loop
    MANUAL ///< Scene is rendered only when RenderOnce() is called explicitly
  };

public:
  /**
   * @brief Creates a new OffscreenApplication instance
   *
   * This is the main constructor for creating an offscreen application that renders
   * to a framebuffer object instead of a visible window.
   *
   * @param [in] argc Pointer to argument count (typically from main())
   * @param [in] argv Array of argument strings (typically from main())
   * @param [in] framework The backend framework to use for the event loop
   * @param [in] renderMode The rendering mode (AUTO for continuous, MANUAL for on-demand)
   * @return A new OffscreenApplication instance
   */
  static OffscreenApplication New(int* argc, char** argv[], FrameworkBackend framework, RenderMode renderMode);

  /**
   * @brief Constructs an empty handle
   *
   * Creates an uninitialized OffscreenApplication handle.
   */
  OffscreenApplication();

  /**
   * @brief Copy constructor
   *
   * @param [in] offscreenApplication A reference to the copied handle
   */
  OffscreenApplication(const OffscreenApplication& offscreenApplication);

  /**
   * @brief Assignment operator
   *
   * @param [in] offscreenApplication A reference to the copied handle
   * @return A reference to this handle
   */
  OffscreenApplication& operator=(const OffscreenApplication& offscreenApplication);

  /**
   * @brief Move constructor.
   *
   * @param[in] rhs A reference to the moved handle
   */
  OffscreenApplication(OffscreenApplication&& rhs) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  OffscreenApplication& operator=(OffscreenApplication&& rhs) noexcept;

  /**
   * @brief Destructor
   */
  ~OffscreenApplication();

public:
  /**
   * @brief Starts the offscreen application
   *
   * Initializes the offscreen rendering system and prepares it for rendering.
   * This must be called before any rendering can occur. In AUTO mode, rendering
   * will start automatically when events are processed. In MANUAL mode, you need
   * to call RenderOnce() to trigger rendering.
   *
   * Note: This method does NOT start an event loop. The application is responsible
   * for creating and running its own event loop to process rendering and events.
   */
  void Start();

  /**
   * @brief Terminates the offscreen application
   *
   * Stops the internal rendering operations and cleans up resources. After calling this method,
   * the application cannot be used for rendering anymore.
   *
   * Note: This stops the internal rendering mechanism, not the external event loop
   * that the application is responsible for managing.
   */
  void Terminate();

  /**
   * @brief Retrieves the main offscreen window
   *
   * Returns the OffscreenWindow associated with this application. This window
   * is not a real native window but a rendering surface that uses framebuffer
   * objects for offscreen rendering.
   *
   * @return The OffscreenWindow handle
   */
  OffscreenWindow GetWindow();

  /**
   * @brief Renders the scene once
   *
   * Forces a single frame render even in MANUAL mode or when the application
   * is paused. This is useful for on-demand rendering scenarios where you want
   * to control exactly when rendering occurs.
   * you need to capture specific frames for processing.
   */
  void RenderOnce();

public: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL OffscreenApplication(Internal::Adaptor::OffscreenApplication* offscreenApplication);
};

/**
 * @}
 */

} // namespace Dali

#endif // DALI_OFFSCREEN_APPLICATION_H
