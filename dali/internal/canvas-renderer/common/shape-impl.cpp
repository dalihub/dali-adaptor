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
#include <dali/internal/canvas-renderer/common/shape-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Shape::Shape() = default;

Shape::~Shape() = default;

bool Shape::AddRect(Rect<float> rect, Vector2 roundedCorner)
{
  return false;
}

bool Shape::AddCircle(Vector2 center, Vector2 radius)
{
  return false;
}

bool Shape::AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie)
{
  return false;
}

bool Shape::AddMoveTo(Vector2 point)
{
  return false;
}

bool Shape::AddLineTo(Vector2 line)
{
  return false;
}

bool Shape::AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint)
{
  return false;
}

bool Shape::Close()
{
  return false;
}

bool Shape::ResetPath()
{
  return false;
}

bool Shape::SetFillColor(Vector4 color)
{
  return false;
}

Vector4 Shape::GetFillColor() const
{
  return Vector4(0, 0, 0, 0);
}

bool Shape::SetFillGradient(Dali::CanvasRenderer::Gradient& gradient)
{
  return false;
}

Dali::CanvasRenderer::Gradient Shape::GetFillGradient() const
{
  return Dali::CanvasRenderer::Gradient();
}

bool Shape::SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule)
{
  return false;
}

Dali::CanvasRenderer::Shape::FillRule Shape::GetFillRule() const
{
  return Dali::CanvasRenderer::Shape::FillRule::WINDING;
}

bool Shape::SetStrokeWidth(float width)
{
  return false;
}

float Shape::GetStrokeWidth() const
{
  return false;
}

bool Shape::SetStrokeColor(Vector4 color)
{
  return false;
}

Vector4 Shape::GetStrokeColor() const
{
  return Vector4(0, 0, 0, 0);
}

bool Shape::SetStrokeGradient(Dali::CanvasRenderer::Gradient& gradient)
{
  return false;
}

Dali::CanvasRenderer::Gradient Shape::GetStrokeGradient() const
{
  return Dali::CanvasRenderer::Gradient();
}

bool Shape::SetStrokeDash(const Dali::Vector<float> dashPattern)
{
  return false;
}

Dali::Vector<float> Shape::GetStrokeDash() const
{
  return Vector<float>();
}

bool Shape::SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap)
{
  return false;
}

Dali::CanvasRenderer::Shape::StrokeCap Shape::GetStrokeCap() const
{
  return Dali::CanvasRenderer::Shape::StrokeCap::SQUARE;
}

bool Shape::SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join)
{
  return false;
}

Dali::CanvasRenderer::Shape::StrokeJoin Shape::GetStrokeJoin() const
{
  return Dali::CanvasRenderer::Shape::StrokeJoin::BEVEL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
