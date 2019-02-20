#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_TEXT_RENDERER_LAYOUT_HELPER_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_TEXT_RENDERER_LAYOUT_HELPER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace TextAbstraction
{

/**
 * @brief Parameters used to transform the vertices of the glyphs to wrap a circular path.
 */
struct DALI_ADAPTOR_API CircularTextParameters
{
  CircularTextParameters()
  : centerX{ 0.0 },
    centerY{ 0.0 },
    radius{ 0.0 },
    invRadius{ 0.0 },
    beginAngle{ 0.0 },
    isClockwise{ true }
  {}

  double centerX;     ///< The 'x' center of the circular path.
  double centerY;     ///< The 'y' center of the circular path.
  double radius;      ///< The radius in pixels.
  double invRadius;   ///< 1.0 / radius.
  double beginAngle;  ///< The angle in radians where the circular text begins.
  bool isClockwise:1; ///< Whether the circular text layout is clockwise.
};

/**
 * @brief Transforms a vertex to wrap a clockwise circular path.
 *
 * @param[in] parameters The parameters of the circular path.
 * @param[in,out] x The 'x' coordinate of the vertex.
 * @param[in,out] y The 'y' coordinate of the vertex.
 */
DALI_ADAPTOR_API void TransformToArcClockwise( const CircularTextParameters& parameters, double& x, double& y );

/**
 * @brief Transforms a vertex to wrap an anti clockwise circular path.
 *
 * @param[in] parameters The parameters of the circular path.
 * @param[in,out] x The 'x' coordinate of the vertex.
 * @param[in,out] y The 'y' coordinate of the vertex.
 */
DALI_ADAPTOR_API void TransformToArcAntiClockwise( const CircularTextParameters& parameters, double& x, double& y );

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_TEXT_RENDERER_LAYOUT_HELPER_H
