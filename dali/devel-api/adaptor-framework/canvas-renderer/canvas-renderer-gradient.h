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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
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
 */
class DALI_ADAPTOR_API CanvasRenderer::Gradient : public BaseHandle
{
public:
  /**
   * @brief Constructor
   */
  Gradient();

  /**
   * @brief Destructor.
   */
  ~Gradient();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  Gradient(const Gradient& handle) = default;

public:
  /**
   * @brief Enumeration specifying how to fill the area outside the gradient bounds.
   */
  enum class Spread
  {
    PAD = 0, ///< The remaining area is filled with the closest stop color.
    REFLECT, ///< The gradient pattern is reflected outside the gradient area until the expected region is filled.
    REPEAT   ///< The gradient pattern is repeated continuously beyond the gradient area until the expected region is filled.
  };

  /**
   * @brief A data structure storing the information about the color and its relative position inside the gradient bounds.
   */
  struct ColorStop
  {
    float   offset; /**< The relative position of the color. */
    Vector4 color;  /**< The color value. */
  };

  /// @brief List of Colorstop.
  using ColorStops = Dali::Vector<ColorStop>;

public:
  /**
   * @brief Sets the parameters of the colors of the gradient and their position.
   * @param[in] colorStops An array of ColorStop data structure.
   * @return Result::Success when succeed.
   */
  bool SetColorStops(ColorStops& colorStops);

  /**
   * @brief Gets the parameters of the colors of the gradient, their position and number.
   * @return Returns the colorstops list.
   */
  ColorStops GetColorStops() const;

  /**
   * @brief Set the spread.
   * @param[in] spread The current spraed type of the shape.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetSpread(Spread spread);

  /**
   * @brief Get the spread type
   * @return Returns the current spread type of the shape.
   */
  Spread GetSpread() const;

  /**
   * @brief Downcast a handle to Gradient handle.
   *
   * If handle points to an InputMethodContext the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object.
   * @return Handle to an Gradient or an uninitialized handle.
   */
  static Gradient DownCast(BaseHandle handle);

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
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
