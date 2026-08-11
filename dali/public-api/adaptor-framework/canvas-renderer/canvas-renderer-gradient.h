#ifndef DALI_CANVAS_RENDERER_GRADIENT_H
#define DALI_CANVAS_RENDERER_GRADIENT_H

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
#include <dali/public-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class CanvasRenderer;
class Gradient;
} // namespace Adaptor
} //namespace Internal DALI_INTERNAL

/**
 * @brief An abstract class representing the gradient fill of the Shape object.
 *
 * It contains the information about the gradient colors and their arrangement
 * inside the gradient bounds. The gradients bounds are defined in the LinearGradient
 * or RadialGradient class, depending on the type of the gradient to be used.
 * It specifies the gradient behavior in case the area defined by the gradient bounds
 * is smaller than the area to be filled.
 *
 * @SINCE_2_5.35
 */
class DALI_ADAPTOR_API CanvasRenderer::Gradient : public BaseHandle
{
public:
  /**
   * @brief Constructor
   *
   * @SINCE_2_5.35
   */
  Gradient();

  /**
   * @brief Destructor.
   *
   * @SINCE_2_5.35
   */
  ~Gradient();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_2_5.35
   * @param[in] handle A reference to the copied handle
   */
  Gradient(const Gradient& handle) = default;

public:
  /**
   * @brief Enumeration specifying how to fill the area outside the gradient bounds.
   *
   * @SINCE_2_5.35
   */
  enum class Spread
  {
    PAD = 0, ///< The remaining area is filled with the closest stop color. @SINCE_2_5.35
    REFLECT, ///< The gradient pattern is reflected outside the gradient area until the expected region is filled. @SINCE_2_5.35
    REPEAT   ///< The gradient pattern is repeated continuously beyond the gradient area until the expected region is filled. @SINCE_2_5.35
  };

public:
  /**
   * @brief Appends a color stop to the gradient.
   *
   * A color stop associates a color with a relative position inside the gradient bounds.
   * Stops are applied in the order they are added.
   *
   * @SINCE_2_5.35
   * @param[in] offset The relative position of the color, in the range [0.0, 1.0]
   * @param[in] color The color value (RGBA, each channel 0.0 ~ 1.0)
   * @return Returns True when it's successful. False otherwise.
   */
  bool AddColorStop(float offset, const Vector4& color);

  /**
   * @brief Removes all color stops added to the gradient.
   *
   * @SINCE_2_5.35
   * @return Returns True when it's successful. False otherwise.
   */
  bool ClearColorStops();

  /**
   * @brief Gets the number of color stops added to the gradient.
   *
   * @SINCE_2_5.35
   * @return Returns the number of color stops.
   */
  uint32_t GetColorStopCount() const;

  /**
   * @brief Gets the relative position of the color stop at the given index.
   *
   * @SINCE_2_5.35
   * @param[in] index The color stop index, in the range [0, GetColorStopCount())
   * @return Returns the relative position, or 0.0f if @p index is out of range.
   */
  float GetColorStopOffset(uint32_t index) const;

  /**
   * @brief Gets the color of the color stop at the given index.
   *
   * @SINCE_2_5.35
   * @param[in] index The color stop index, in the range [0, GetColorStopCount())
   * @return Returns the color value, or Vector4::ZERO if @p index is out of range.
   */
  Vector4 GetColorStopColor(uint32_t index) const;

  /**
   * @brief Set the spread.
   *
   * @SINCE_2_5.35
   * @param[in] spread The current spraed type of the shape.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetSpread(Spread spread);

  /**
   * @brief Get the spread type
   *
   * @SINCE_2_5.35
   * @return Returns the current spread type of the shape.
   */
  Spread GetSpread() const;

  /**
   * @brief Downcast a handle to Gradient handle.
   *
   * If handle points to an InputMethodContext the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @SINCE_2_5.35
   * @param[in] handle Handle to an object.
   * @return Handle to an Gradient or an uninitialized handle.
   */
  static Gradient DownCast(BaseHandle handle);

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   *
   * @SINCE_2_5.35
   * @note  Not intended for application developers.
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::Gradient
   */
  explicit DALI_INTERNAL Gradient(Internal::Adaptor::Gradient* pImpl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_GRADIENT_H
