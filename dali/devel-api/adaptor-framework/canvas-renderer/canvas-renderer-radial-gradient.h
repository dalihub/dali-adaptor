#ifndef DALI_CANVAS_RENDERER_RADIAL_GRADIENT_H
#define DALI_CANVAS_RENDERER_RADIAL_GRADIENT_H

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
class RadialGradient;
} // namespace Adaptor
} //namespace Internal DALI_INTERNAL

/**
 * @brief A class representing the radial gradient fill of the Shape object.
 */
class DALI_ADAPTOR_API CanvasRenderer::RadialGradient : public CanvasRenderer::Gradient
{
public:
  /**
   * @brief Creates an initialized handle to a new CanvasRenderer::RadialGradient.
   * @return A handle to a newly allocated RadialGradient
   */
  static RadialGradient New();

public:
  /**
   * @brief Creates an empty handle. Use CanvasRenderer::RadialGradient::New() to create an initialized object.
   */
  RadialGradient();

  /**
   * @brief Destructor.
   */
  ~RadialGradient();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  RadialGradient(const RadialGradient& handle) = default;

public:
  /**
   * @brief Sets the radial gradient bounds.
   * The radial gradient bounds are defined as a circle centered in a given point of a given radius.
   * @param[in] centerPoint The point of the center of the bounding circle.
   * @param[in] radius The radius of the bounding circle.
   * @return Returns True when it's successful. False otherwise.
   */
  bool SetBounds(Vector2 centerPoint, float radius);

  /**
   * @brief Gets the radial gradient bounds.
   * @param[out] centerPoint The point used to determine the gradient bounds.
   * @param[out] radius The radius of the bounding circle.
   * @return Returns True when it's successful. False otherwise.
   */
  bool GetBounds(Vector2& centerPoint, float& radius) const;

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated CanvasRenderer::RadialGradient
   */
  explicit DALI_INTERNAL RadialGradient(Internal::Adaptor::RadialGradient* impl);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_CANVAS_RENDERER_RADIAL_GRADIENT_H
