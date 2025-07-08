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

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-shape.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/shape-impl.h>

namespace Dali
{
CanvasRenderer::Shape CanvasRenderer::Shape::New()
{
  Internal::Adaptor::ShapePtr shape = Internal::Adaptor::Shape::New();
  return Shape(shape.Get());
}

CanvasRenderer::Shape::Shape()
{
}

CanvasRenderer::Shape::~Shape()
{
}

CanvasRenderer::Shape::Shape(Internal::Adaptor::Shape* impl)
: CanvasRenderer::Drawable(impl)
{
}

bool CanvasRenderer::Shape::AddRect(Rect<float> rect, Vector2 roundedCorner)
{
  return GetImplementation(*this).AddRect(rect, roundedCorner);
}

bool CanvasRenderer::Shape::AddCircle(Vector2 center, Vector2 radius)
{
  return GetImplementation(*this).AddCircle(center, radius);
}

bool CanvasRenderer::Shape::AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie)
{
  return GetImplementation(*this).AddArc(center, radius, startAngle, sweep, pie);
}

bool CanvasRenderer::Shape::AddMoveTo(Vector2 point)
{
  return GetImplementation(*this).AddMoveTo(point);
}

bool CanvasRenderer::Shape::AddLineTo(Vector2 line)
{
  return GetImplementation(*this).AddLineTo(line);
}

bool CanvasRenderer::Shape::AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint)
{
  return GetImplementation(*this).AddCubicTo(controlPoint1, controlPoint2, endPoint);
}

bool CanvasRenderer::Shape::AddPath(PathCommands& pathCommand)
{
  return GetImplementation(*this).AddPath(pathCommand);
}

bool CanvasRenderer::Shape::Close()
{
  return GetImplementation(*this).Close();
}

bool CanvasRenderer::Shape::ResetPath()
{
  return GetImplementation(*this).ResetPath();
}

bool CanvasRenderer::Shape::SetFillColor(Vector4 color)
{
  return GetImplementation(*this).SetFillColor(color);
}

Vector4 CanvasRenderer::Shape::GetFillColor() const
{
  return GetImplementation(*this).GetFillColor();
}

bool CanvasRenderer::Shape::SetFillGradient(CanvasRenderer::Gradient& gradient)
{
  return GetImplementation(*this).SetFillGradient(gradient);
}

CanvasRenderer::Gradient CanvasRenderer::Shape::GetFillGradient() const
{
  return GetImplementation(*this).GetFillGradient();
}

bool CanvasRenderer::Shape::SetFillRule(CanvasRenderer::Shape::FillRule rule)
{
  return GetImplementation(*this).SetFillRule(rule);
}

CanvasRenderer::Shape::FillRule CanvasRenderer::Shape::GetFillRule() const
{
  return GetImplementation(*this).GetFillRule();
}

bool CanvasRenderer::Shape::SetStrokeWidth(float width)
{
  return GetImplementation(*this).SetStrokeWidth(width);
}

float CanvasRenderer::Shape::GetStrokeWidth() const
{
  return GetImplementation(*this).GetStrokeWidth();
}

bool CanvasRenderer::Shape::SetStrokeColor(Vector4 color)
{
  return GetImplementation(*this).SetStrokeColor(color);
}

Vector4 CanvasRenderer::Shape::GetStrokeColor() const
{
  return GetImplementation(*this).GetStrokeColor();
}

bool CanvasRenderer::Shape::SetStrokeGradient(CanvasRenderer::Gradient& gradient)
{
  return GetImplementation(*this).SetStrokeGradient(gradient);
}

CanvasRenderer::Gradient CanvasRenderer::Shape::GetStrokeGradient() const
{
  return GetImplementation(*this).GetStrokeGradient();
}

bool CanvasRenderer::Shape::SetStrokeDash(const Dali::Vector<float>& dashPattern)
{
  return GetImplementation(*this).SetStrokeDash(dashPattern);
}

Dali::Vector<float> CanvasRenderer::Shape::GetStrokeDash() const
{
  return GetImplementation(*this).GetStrokeDash();
}

bool CanvasRenderer::Shape::SetStrokeCap(CanvasRenderer::Shape::StrokeCap cap)
{
  return GetImplementation(*this).SetStrokeCap(cap);
}

CanvasRenderer::Shape::StrokeCap CanvasRenderer::Shape::GetStrokeCap() const
{
  return GetImplementation(*this).GetStrokeCap();
}

bool CanvasRenderer::Shape::SetStrokeJoin(CanvasRenderer::Shape::StrokeJoin join)
{
  return GetImplementation(*this).SetStrokeJoin(join);
}

CanvasRenderer::Shape::StrokeJoin CanvasRenderer::Shape::GetStrokeJoin() const
{
  return GetImplementation(*this).GetStrokeJoin();
}

} // namespace Dali
