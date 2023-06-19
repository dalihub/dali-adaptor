#ifndef DALI_OFFSCREEN_WINDOW_H
#define DALI_OFFSCREEN_WINDOW_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

class Layer;

namespace Internal
{
class OffscreenWindow;
}

class DALI_ADAPTOR_API OffscreenWindow : public Dali::BaseHandle
{
public:
  using WindowSize = Uint16Pair;

public:
  /**
   * @brief Creates an initialized handle to a new OffscreenWindow
   * @note You should hold the returned handle. If you missed the handle, the OffscreenWindow will be released
   *
   * @param[in] width The initial width of the OffscreenWindow
   * @param[in] height The initial height of the OffscreenWindow
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  static OffscreenWindow New(uint16_t width, uint16_t height, bool isTranslucent);

  /**
   * @brief Creates an initialized handle to a new OffscreenWindow
   * @note You should hold the returned handle. If you missed the handle, the OffscreenWindow will be released
   *
   * @param[in] surface The native surface handle of your platform
   */
  static OffscreenWindow New(Any surface);

  /**
   * @brief Constructs an empty handle
   */
  OffscreenWindow();

  /**
   * @brief Copy constructor
   *
   * @param [in] window A reference to the copied handle
   */
  OffscreenWindow(const OffscreenWindow& window);

  /**
   * @brief Assignment operator
   *
   * @param [in] window A reference to the copied handle
   * @return A reference to this
   */
  OffscreenWindow& operator=(const OffscreenWindow& window);

  /**
   * @brief Move constructor
   *
   * @param [in] window A reference to the moved handle
   */
  OffscreenWindow(OffscreenWindow&& window);

  /**
   * @brief Move assignment operator
   *
   * @param [in] window A reference to the moved handle
   * @return A reference to this
   */
  OffscreenWindow& operator=(OffscreenWindow&& window);

  /**
   * @brief Destructor
   */
  ~OffscreenWindow();

public:
  /**
   * @brief Adds a child Actor to the OffscreenWindow.
   *
   * The child will be referenced.
   *
   * @param[in] actor The child
   * @pre The actor has been initialized.
   * @pre The actor does not have a parent.
   */
  void Add(Actor actor);

  /**
   * @brief Removes a child Actor from the OffscreenWindow.
   *
   * The child will be unreferenced.
   *
   * @param[in] actor The child
   * @pre The actor has been added to the OffscreenWindow.
   */
  void Remove(Actor actor);

  /**
   * @brief Sets the background color of the OffscreenWindow.
   *
   * @param[in] color The new background color
   */
  void SetBackgroundColor(const Vector4& color);

  /**
   * @brief Gets the background color of the OffscreenWindow.
   *
   * @return The background color
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @brief Returns the root Layer of the OffscreenWindow.
   *
   * @return The root layer
   */
  Layer GetRootLayer() const;

  /**
   * @brief Queries the number of on-scene layers.
   *
   * Note that a default layer is always provided (count >= 1).
   *
   * @return The number of layers
   */
  uint32_t GetLayerCount() const;

  /**
   * @brief Retrieves the layer at a specified depth in the OffscreenWindow.
   *
   * @param[in] depth The depth
   * @return The layer found at the given depth
   * @pre Depth is less than layer count; see GetLayerCount().
   */
  Layer GetLayer(uint32_t depth) const;

  /**
   * @brief Returns the size of the OffscreenWindow in pixels as a Vector.
   *
   * The x component will be the width of the OffscreenWindow in pixels.
   * The y component will be the height of the OffscreenWindow in pixels.
   *
   * @return The size of the OffscreenWindow as a Vector
   */
  WindowSize GetSize() const;

  /**
   * @brief Gets the native handle.
   * @note When users call this function, it wraps the actual type used by the underlying system.
   * @return The native handle or an empty handle
   */
  Any GetNativeHandle() const;

  /**
   * @brief Retrieves the DPI of the window.
   *
   * @return The DPI of the window
   */
  Uint16Pair GetDpi() const;

  /**
   * @brief Sets the PostRenderCallback of the OffscreenWindow.
   *
   * @param[in] callback The PostRenderCallback function
   * @code
   *   void MyFunction( OffscreenWindow window, Any nativeSurface );
   * @endcode
   *
   * @note Ownership of the callback is passed onto this class.
   *
   */
  void SetPostRenderCallback(CallbackBase* callback);

  /**
   * @brief Sets a callback that is called when the frame rendering is done by the graphics driver.
   *
   * @param[in] callback The function to call
   *
   * @note A callback of the following type may be used:
   * @code
   *   void MyFunction();
   * @endcode
   *
   * @note Ownership of the callback is passed onto this class.
   */
  void SetFrameRenderedCallback(CallbackBase* callback);

public: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL OffscreenWindow(Internal::OffscreenWindow* window);
};

/**
 * @}
 */

} // namespace Dali

#endif // DALI_OFFSCREEN_WINDOW_H
