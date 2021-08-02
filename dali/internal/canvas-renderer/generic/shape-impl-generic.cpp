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

// CLASS HEADER
#include <dali/internal/canvas-renderer/generic/shape-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
// Type Registration
Dali::BaseHandle Create()
{
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Shape), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

ShapeGeneric* ShapeGeneric::New()
{
  return new ShapeGeneric();
}

ShapeGeneric::ShapeGeneric()
{
}

ShapeGeneric::~ShapeGeneric()
{
}

bool ShapeGeneric::AddRect(Rect<float> rect, Vector2 roundedCorner)
{
  return false;
}

bool ShapeGeneric::AddCircle(Vector2 center, Vector2 radius)
{
  return false;
}

bool ShapeGeneric::AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie)
{
  return false;
}

bool ShapeGeneric::AddMoveTo(Vector2 point)
{
  return false;
}

bool ShapeGeneric::AddLineTo(Vector2 line)
{
  return false;
}

bool ShapeGeneric::AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint)
{
  return false;
}

bool ShapeGeneric::AddPath(Dali::CanvasRenderer::Shape::PathCommands& pathCommand)
{
  return false;
}

bool ShapeGeneric::Close()
{
  return false;
}

bool ShapeGeneric::SetFillColor(Vector4 color)
{
  return false;
}

Vector4 ShapeGeneric::GetFillColor() const
{
  return Vector4(0, 0, 0, 0);
}

bool ShapeGeneric::SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule)
{
  return false;
}

Dali::CanvasRenderer::Shape::FillRule ShapeGeneric::GetFillRule() const
{
  return Dali::CanvasRenderer::Shape::FillRule::WINDING;
}

bool ShapeGeneric::SetStrokeWidth(float width)
{
  return false;
}

float ShapeGeneric::GetStrokeWidth() const
{
  return false;
}

bool ShapeGeneric::SetStrokeColor(Vector4 color)
{
  return false;
}

Vector4 ShapeGeneric::GetStrokeColor() const
{
  return Vector4(0, 0, 0, 0);
}

bool ShapeGeneric::SetStrokeDash(const Dali::Vector<float> dashPattern)
{
  return false;
}

Dali::Vector<float> ShapeGeneric::GetStrokeDash() const
{
  return Vector<float>();
}

bool ShapeGeneric::SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap)
{
  return false;
}

Dali::CanvasRenderer::Shape::StrokeCap ShapeGeneric::GetStrokeCap() const
{
  return Dali::CanvasRenderer::Shape::StrokeCap::SQUARE;
}

bool ShapeGeneric::SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join)
{
  return false;
}

Dali::CanvasRenderer::Shape::StrokeJoin ShapeGeneric::GetStrokeJoin() const
{
  return Dali::CanvasRenderer::Shape::StrokeJoin::BEVEL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
