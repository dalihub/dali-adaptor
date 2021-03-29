#ifndef DALI_CANVAS_RENDERER_DRAWABLE_H
#define DALI_CANVAS_RENDERER_DRAWABLE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer.h>
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
class Drawable;
class Shape;
} // namespace Adaptor
} // namespace DALI_INTERNAL

/**
 * @brief Drawable is a object class for drawing a vector primitive.
 */
class CanvasRenderer::Drawable : public BaseHandle
{
public:
  /**
   * @brief Constructor
   */
  Drawable();

  /**
   * @brief Destructor.
   */
  ~Drawable();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  Drawable(const Drawable& handle) = default;

public:
  /**
   * @brief Set the transparency value
   * @param[in] opacity The transparency level [0 ~ 1.0], 0 means totally transparent, while 1 means opaque.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetOpacity(float opacity);

  /**
   * @brief Get the transparency value
   * @return Returns the transparency level
   */
  float GetOpacity() const;

  /**
   * @brief Set the angle of rotation transformation.
   * @param[in] degree The degree value of angle.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Rotate(Degree degree);

  /**
   * @brief Set the scale value of scale transformation.
   * @param[in] factor The scale factor value.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Scale(float factor);

  /**
   * @brief Set the x, y movement value of translate transformation.
   * @param[in] translate The x and y-axis movement value.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Translate(Vector2 translate);

  /**
   * @brief Set the matrix value for affine transform.
   * @param[in] matrix The 3x3 matrix value.
   * @return Returns True when it's successful. False otherwise.
   */
  bool Transform(const Dali::Matrix3& matrix);

  /**
   * @brief Downcast a handle to Drawable handle.
   *
   * If handle points to an InputMethodContext the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object.
   * @return Handle to an Drawable or an uninitialized handle.
   */
  static Drawable DownCast(BaseHandle handle);

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::Drawable
   */
  explicit DALI_INTERNAL Drawable(Internal::Adaptor::Drawable* pImpl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_DRAWABLE_H
