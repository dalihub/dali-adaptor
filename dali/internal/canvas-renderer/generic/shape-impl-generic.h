#ifndef DALI_INTERNAL_GENERIC_SHAPE_INTERFACE_GENERIC_H
#define DALI_INTERNAL_GENERIC_SHAPE_INTERFACE_GENERIC_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-shape.h>
#include <dali/internal/canvas-renderer/common/shape-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Shape.
 */
class ShapeGeneric : public Dali::Internal::Adaptor::Shape
{
public:
  /**
   * @brief Creates a Shape object.
   * @return A pointer to a newly allocated shape
   */
  static ShapeGeneric* New();

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddRect()
   */
  bool AddRect(Rect<float> rect, Vector2 roundedCorner) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddCircle()
   */
  bool AddCircle(Vector2 center, Vector2 radius) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddArc()
   */
  bool AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddMoveTo()
   */
  bool AddMoveTo(Vector2 point) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddLineTo()
   */
  bool AddLineTo(Vector2 line) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddCubicTo()
   */
  bool AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddPath()
   */
  bool AddPath(Dali::CanvasRenderer::Shape::PathCommands& pathCommand) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::Close()
   */
  bool Close() override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillColor()
   */
  bool SetFillColor(Vector4 color) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillColor()
   */
  Vector4 GetFillColor() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillRule()
   */
  bool SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillRule()
   */
  Dali::CanvasRenderer::Shape::FillRule GetFillRule() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeWidth()
   */
  bool SetStrokeWidth(float width) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeWidth()
   */
  float GetStrokeWidth() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeColor()
   */
  bool SetStrokeColor(Vector4 color) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeColor()
   */
  Vector4 GetStrokeColor() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeDash()
   */
  bool SetStrokeDash(const Dali::Vector<float> dashPattern) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeDash()
   */
  Dali::Vector<float> GetStrokeDash() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeCap()
   */
  bool SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeCap()
   */
  Dali::CanvasRenderer::Shape::StrokeCap GetStrokeCap() const override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeJoin()
   */
  bool SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join) override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeJoin()
   */
  Dali::CanvasRenderer::Shape::StrokeJoin GetStrokeJoin() const override;

private:
  ShapeGeneric(const ShapeGeneric&) = delete;
  ShapeGeneric& operator=(ShapeGeneric&) = delete;
  ShapeGeneric(ShapeGeneric&&)           = delete;
  ShapeGeneric& operator=(ShapeGeneric&&) = delete;

  /**
   * @brief Constructor
   */
  ShapeGeneric();

  /**
   * @brief Destructor.
   */
  virtual ~ShapeGeneric() override;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_GENERIC_SHAPE_INTERFACE_GENERIC_H
