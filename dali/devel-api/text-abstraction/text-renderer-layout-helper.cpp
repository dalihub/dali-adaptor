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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>

namespace Dali
{

namespace TextAbstraction
{

void TransformToArc( const CircularTextParameters& parameters, double& x, double& y )
{
  double yP = y;

  // Does the italic synthesization for circular layout.
  if( parameters.synthesizeItalic )
  {
    const double xP = -yP * sin( TextAbstraction::FontClient::DEFAULT_ITALIC_ANGLE );
    yP *= cos( TextAbstraction::FontClient::DEFAULT_ITALIC_ANGLE );

    x += xP;
  }

  double angle = 0.0;
  double radius = parameters.radius;

  // Transform to a circular layout.
  if( parameters.isClockwise )
  {
    angle = parameters.beginAngle - parameters.invRadius * x;
    radius -= yP;

    x = radius * cos( angle );
    y = -radius * sin( angle );
  }
  else
  {
    angle = parameters.beginAngle + parameters.invRadius * x;
    radius += yP;

    x = radius * cos( angle );
    y = radius * sin( -angle );
  }

  // Transforms to the text area coordinate system.
  x += parameters.centerX;
  y += parameters.centerY;
}

} // namespace TextAbstraction

} // namespace Dali
