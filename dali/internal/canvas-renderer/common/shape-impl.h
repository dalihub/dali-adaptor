#ifndef DALI_INTERNAL_SHAPE_IMPL_H
#define DALI_INTERNAL_SHAPE_IMPL_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-gradient.h>
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-shape.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Dali internal Shape.
 */
class Shape : public Internal::Adaptor::Drawable
{
public:
  /**
   * @brief Constructor
   */
  Shape();

  /**
   * @brief Destructor.
   */
  ~Shape() override;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddRect()
   */
  virtual bool AddRect(Rect<float> rect, Vector2 roundedCorner);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddCircle()
   */
  virtual bool AddCircle(Vector2 center, Vector2 radius);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddArc()
   */
  virtual bool AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddMoveTo()
   */
  virtual bool AddMoveTo(Vector2 point);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddLineTo()
   */
  virtual bool AddLineTo(Vector2 line);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddCubicTo()
   */
  virtual bool AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddPath()
   */
  virtual bool AddPath(Dali::CanvasRenderer::Shape::PathCommands& pathCommand);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::Close()
   */
  virtual bool Close();

  /**
   * @copydoc Dali::CanvasRenderer::Shape::ResetPath()
   */
  virtual bool ResetPath();

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillColor()
   */
  virtual bool SetFillColor(Vector4 color);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillColor()
   */
  virtual Vector4 GetFillColor() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillGradient()
   */
  virtual bool SetFillGradient(Dali::CanvasRenderer::Gradient& gradient);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillGradient()
   */
  virtual Dali::CanvasRenderer::Gradient GetFillGradient() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillRule()
   */
  virtual bool SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillRule()
   */
  virtual Dali::CanvasRenderer::Shape::FillRule GetFillRule() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeWidth()
   */
  virtual bool SetStrokeWidth(float width);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeWidth()
   */
  virtual float GetStrokeWidth() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeColor()
   */
  virtual bool SetStrokeColor(Vector4 color);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeColor()
   */
  virtual Vector4 GetStrokeColor() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeGradient()
   */
  virtual bool SetStrokeGradient(Dali::CanvasRenderer::Gradient& gradient);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeGradient()
   */
  virtual Dali::CanvasRenderer::Gradient GetStrokeGradient() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeDash()
   */
  virtual bool SetStrokeDash(const Dali::Vector<float> dashPattern);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeDash()
   */
  virtual Dali::Vector<float> GetStrokeDash() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeCap()
   */
  virtual bool SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeCap()
   */
  virtual Dali::CanvasRenderer::Shape::StrokeCap GetStrokeCap() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeJoin()
   */
  virtual bool SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeJoin()
   */
  virtual Dali::CanvasRenderer::Shape::StrokeJoin GetStrokeJoin() const;

  Shape(const Shape&) = delete;
  Shape& operator=(Shape&) = delete;
  Shape(Shape&&)           = delete;
  Shape& operator=(Shape&&) = delete;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::Shape& GetImplementation(Dali::CanvasRenderer::Shape& shape)
{
  DALI_ASSERT_ALWAYS(shape && "Shape handle is empty.");

  BaseObject& handle = shape.GetBaseObject();

  return static_cast<Internal::Adaptor::Shape&>(handle);
}

inline static const Internal::Adaptor::Shape& GetImplementation(const Dali::CanvasRenderer::Shape& shape)
{
  DALI_ASSERT_ALWAYS(shape && "Shape handle is empty.");

  const BaseObject& handle = shape.GetBaseObject();

  return static_cast<const Internal::Adaptor::Shape&>(handle);
}

} // namespace Dali

#endif // DALI_INTERNAL_SHAPE_IMPL_H
