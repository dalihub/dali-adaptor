#ifndef DALI_CANVAS_RENDERER_LINEAR_GRADIENT_H
#define DALI_CANVAS_RENDERER_LINEAR_GRADIENT_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-gradient.h>
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
class LinearGradient;
} // namespace Adaptor
} // namespace DALI_INTERNAL

/**
 * @brief A class representing the linear gradient fill of the Shape object.
 *
 * Besides the class inherited from the Gradient class, it enables setting and getting the linear gradient bounds.
 * The behavior outside the gradient bounds depends on the value specified in the spread API.
 */
class DALI_ADAPTOR_API CanvasRenderer::LinearGradient : public CanvasRenderer::Gradient
{
public:
  /**
   * @brief Creates an initialized handle to a new CanvasRenderer::LinearGradient.
   * @return A handle to a newly allocated LinearGradient
   */
  static LinearGradient New();

public:
  /**
   * @brief Creates an empty handle. Use CanvasRenderer::LinearGradient::New() to create an initialized object.
   */
  LinearGradient();

  /**
   * @brief Destructor.
   */
  ~LinearGradient();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  LinearGradient(const LinearGradient& handle) = default;

public:
  /**
   * @brief Sets the linear gradient bounds.
   * The bounds of the linear gradient are defined as a surface constrained by two parallel lines crossing
   * the given points (@p x1, @p y1) and (@p x2, @p y2), respectively. Both lines are perpendicular to the line linking
   * (@p x1, @p y1) and (@p x2, @p y2).
   * @param[in] firstPoint The first point used to determine the gradient bounds.
   * @param[in] secondPoint The second point used to determine the gradient bounds.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetBounds(Vector2 firstPoint, Vector2 secondPoint);

  /**
   * @brief Gets the linear gradient bounds.
   * @param[out] firstPoint The first point used to determine the gradient bounds.
   * @param[out] secondPoint The second point used to determine the gradient bounds.
   * @return Returns True when it's successful. False otherwise.
   */
  bool GetBounds(Vector2& firstPoint, Vector2& secondPoint) const;

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::LinearGradient
   */
  explicit DALI_INTERNAL LinearGradient(Internal::Adaptor::LinearGradient* impl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_LINEAR_GRADIENT_H
