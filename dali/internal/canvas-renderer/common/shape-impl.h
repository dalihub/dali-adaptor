#ifndef DALI_INTERNAL_SHAPE_IMPL_H
#define DALI_INTERNAL_SHAPE_IMPL_H

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
#ifdef THORVG_SUPPORT
#include <thorvg.h>
#endif
#include <dali/public-api/common/intrusive-ptr.h>
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
class Shape;
typedef IntrusivePtr<Shape> ShapePtr;

/**
 * Dali internal Shape.
 */
class Shape : public Internal::Adaptor::Drawable
{
public:
  /**
   * @brief Creates a Shape object.
   * @return A pointer to a newly allocated shape
   */
  static ShapePtr New();

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddRect()
   */
  bool AddRect(Rect<float> rect, Vector2 roundedCorner);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddCircle()
   */
  bool AddCircle(Vector2 center, Vector2 radius);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddArc()
   */
  bool AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddMoveTo()
   */
  bool AddMoveTo(Vector2 point);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddLineTo()
   */
  bool AddLineTo(Vector2 line);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddCubicTo()
   */
  bool AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::AddPath()
   */
  bool AddPath(Dali::CanvasRenderer::Shape::PathCommands& pathCommand);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::Close()
   */
  bool Close();

  /**
   * @copydoc Dali::CanvasRenderer::Shape::ResetPath()
   */
  bool ResetPath();

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillColor()
   */
  bool SetFillColor(Vector4 color);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillColor()
   */
  Vector4 GetFillColor() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillGradient()
   */
  bool SetFillGradient(Dali::CanvasRenderer::Gradient& gradient);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillGradient()
   */
  Dali::CanvasRenderer::Gradient GetFillGradient() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetFillRule()
   */
  bool SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetFillRule()
   */
  Dali::CanvasRenderer::Shape::FillRule GetFillRule() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeWidth()
   */
  bool SetStrokeWidth(float width);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeWidth()
   */
  float GetStrokeWidth() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeColor()
   */
  bool SetStrokeColor(Vector4 color);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeColor()
   */
  Vector4 GetStrokeColor() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeGradient()
   */
  bool SetStrokeGradient(Dali::CanvasRenderer::Gradient& gradient);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeGradient()
   */
  Dali::CanvasRenderer::Gradient GetStrokeGradient() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeDash()
   */
  bool SetStrokeDash(const Dali::Vector<float> dashPattern);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeDash()
   */
  Dali::Vector<float> GetStrokeDash() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeCap()
   */
  bool SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeCap()
   */
  Dali::CanvasRenderer::Shape::StrokeCap GetStrokeCap() const;

  /**
   * @copydoc Dali::CanvasRenderer::Shape::SetStrokeJoin()
   */
  bool SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join);

  /**
   * @copydoc Dali::CanvasRenderer::Shape::GetStrokeJoin()
   */
  Dali::CanvasRenderer::Shape::StrokeJoin GetStrokeJoin() const;

private:
  Shape(const Shape&)       = delete;
  Shape& operator=(Shape&)  = delete;
  Shape(Shape&&)            = delete;
  Shape& operator=(Shape&&) = delete;

  /**
   * @brief Constructor
   */
  Shape();

  /**
   * @brief Destructor.
   */
  virtual ~Shape() override;

private:
  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
  Dali::CanvasRenderer::Gradient mFillGradient;
  Dali::CanvasRenderer::Gradient mStrokeGradient;
#ifdef THORVG_SUPPORT
  tvg::Shape* mTvgShape;
#endif
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
