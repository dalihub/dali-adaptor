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

// FILE HEADER
#include <dali/devel-api/text-abstraction/text-renderer-layout-helper.h>

// EXTERNAL INCLUDES
#include <cmath>

namespace Dali
{

namespace TextAbstraction
{

void TransformToArcClockwise( const CircularTextParameters& parameters, double& x, double& y )
{
  double radius = parameters.radius;
  double angle = parameters.beginAngle;

  angle -= parameters.invRadius * x;

  radius -= y;
  x =  radius * cos( angle );
  y = -radius * sin( angle );

  x += parameters.centerX;
  y += parameters.centerY;
}

void TransformToArcAntiClockwise( const CircularTextParameters& parameters, double& x, double& y )
{
  double radius = parameters.radius;
  double angle = parameters.beginAngle;

  angle += parameters.invRadius * x;

  radius += y;
  x = radius * cos( angle );
  y = radius * sin( -angle );

  x += parameters.centerX;
  y += parameters.centerY;
}

} // namespace TextAbstraction

} // namespace Dali
