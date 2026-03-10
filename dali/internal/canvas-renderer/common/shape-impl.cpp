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
#include <dali/internal/canvas-renderer/common/shape-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
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
  return Dali::CanvasRenderer::Shape::New();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Shape), typeid(Dali::CanvasRenderer::Drawable), Create);

} // unnamed namespace

ShapePtr Shape::New()
{
  auto* shape = new Shape();
  shape->Initialize();
  return shape;
}

Shape::Shape()
: mFillGradient(),
  mStrokeGradient()
#ifdef THORVG_SUPPORT
  ,
  mTvgShape(nullptr)
#endif
{
}

Shape::~Shape()
{
}

void Shape::Initialize()
{
#ifdef THORVG_SUPPORT
#ifdef THORVG_VERSION_1
  mTvgShape = tvg::Shape::gen();
#else
  mTvgShape = tvg::Shape::gen().release();
#endif
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
  }

  Drawable::SetObject(static_cast<void*>(mTvgShape));
  Drawable::SetType(Drawable::Types::SHAPE);
#endif
}

bool Shape::AddRect(Rect<float> rect, Vector2 roundedCorner)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return false;
  }
  if(static_cast<tvg::Shape*>(mTvgShape)->appendRect(rect.x, rect.y, rect.width, rect.height, roundedCorner.x, roundedCorner.y) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Add Rect Fail [%p]\n", this);
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::AddCircle(Vector2 center, Vector2 radius)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return false;
  }
  if(static_cast<tvg::Shape*>(mTvgShape)->appendCircle(center.x, center.y, radius.x, radius.y) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Add Circle Fail [%p]\n", this);
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->appendArc(center.x, center.y, radius, startAngle, sweep, pie) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Add arc fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::AddMoveTo(Vector2 point)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->moveTo(point.x, point.y) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("AddMoveTo() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::AddLineTo(Vector2 line)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->lineTo(line.x, line.y) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("AddLineTo() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->cubicTo(controlPoint1.x, controlPoint1.y, controlPoint2.x, controlPoint2.y, endPoint.x, endPoint.y) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("cubicTo() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::AddPath(Dali::CanvasRenderer::Shape::PathCommands& pathCommand)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

#ifdef THORVG_VERSION_1
  tvg::PathCommand tvgPathCommands[pathCommand.mCommandCount];
  for (uint32_t i = 0; i < pathCommand.mCommandCount; i++)
  {
    tvgPathCommands[i] = static_cast<tvg::PathCommand>(static_cast<uint8_t>(pathCommand.mCommands[i]));
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->appendPath(tvgPathCommands, pathCommand.mCommandCount, static_cast<const tvg::Point*>(static_cast<void*>(pathCommand.mPoints)), pathCommand.mPointCount) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->appendPath(reinterpret_cast<const tvg::PathCommand*>(pathCommand.mCommands), pathCommand.mCommandCount, static_cast<const tvg::Point*>(static_cast<void*>(pathCommand.mPoints)), pathCommand.mPointCount) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("AddPath() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::Close()
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->close() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("close() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::ResetPath()
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->reset() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("reset() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Shape::SetFillColor(Vector4 color)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->fill((uint8_t)(color.r * 255.f), (uint8_t)(color.g * 255.f), (uint8_t)(color.b * 255.f), (uint8_t)(color.a * 255.f)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("SetFillColor fail [%p]\n", this);
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Vector4 Shape::GetFillColor() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return Vector4(0, 0, 0, 0);
  }
  uint8_t r, g, b, a;

#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->fill(&r, &g, &b, &a) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->fillColor(&r, &g, &b, &a) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("GetFillColor fail [%p]\n", this);
    return Vector4(0, 0, 0, 0);
  }
  return Vector4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
#else
  return Vector4::ZERO;
#endif
}

bool Shape::SetFillGradient(Dali::CanvasRenderer::Gradient& gradient)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return false;
  }
  mFillGradient = gradient;
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Gradient Shape::GetFillGradient() const
{
  return mFillGradient;
}

bool Shape::SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }
#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->fillRule(static_cast<tvg::FillRule>(rule)) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->fill(static_cast<tvg::FillRule>(rule)) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("SetFillRule fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Shape::FillRule Shape::GetFillRule() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Dali::CanvasRenderer::Shape::FillRule::WINDING;
  }

  tvg::FillRule rule = static_cast<tvg::Shape*>(mTvgShape)->fillRule();

  return static_cast<Dali::CanvasRenderer::Shape::FillRule>(rule);
#endif
  return Dali::CanvasRenderer::Shape::FillRule::WINDING;
}

bool Shape::SetStrokeWidth(float width)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeWidth(width) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(width) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("SetStrokeWidth fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

float Shape::GetStrokeWidth() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }
  return static_cast<tvg::Shape*>(mTvgShape)->strokeWidth();
#else
  return false;
#endif
}

bool Shape::SetStrokeColor(Vector4 color)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeFill(color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("SetStrokeColor fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Vector4 Shape::GetStrokeColor() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Vector4(0, 0, 0, 0);
  }

  uint8_t r, g, b, a;

#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeFill(&r, &g, &b, &a) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeColor(&r, &g, &b, &a) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("GetStrokeColor fail.\n");
    return Vector4(0, 0, 0, 0);
  }
  return Vector4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
#else
  return Vector4(0, 0, 0, 0);
#endif
}

bool Shape::SetStrokeGradient(Dali::CanvasRenderer::Gradient& gradient)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return false;
  }
  mStrokeGradient = gradient;
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Gradient Shape::GetStrokeGradient() const
{
  return mStrokeGradient;
}

bool Shape::SetStrokeDash(const Dali::Vector<float> dashPattern)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  float* tvgDashPattern = (float*)alloca(sizeof(float) * dashPattern.Count());

  for(unsigned int i = 0u; i < dashPattern.Count(); ++i)
  {
    tvgDashPattern[i] = dashPattern[i];
  }

#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeDash(tvgDashPattern, dashPattern.Count()) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(tvgDashPattern, dashPattern.Count()) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("SetStrokeDash fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Dali::Vector<float> Shape::GetStrokeDash() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Vector<float>();
  }
  const float* tvgDashPattern = nullptr;
  uint32_t     count          = 0;

  count = static_cast<tvg::Shape*>(mTvgShape)->strokeDash(&tvgDashPattern);
  if(!tvgDashPattern || count <= 0)
  {
    DALI_LOG_ERROR("GetStrokeDash() fail.\n");
    return Vector<float>();
  }

  Dali::Vector<float> dashPattern;

  dashPattern.Reserve(count);

  for(unsigned int i = 0u; i < count; ++i)
  {
    dashPattern.PushBack(tvgDashPattern[i]);
  }
  return dashPattern;
#else
  return Vector<float>();
#endif
}

bool Shape::SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }
#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeCap(static_cast<tvg::StrokeCap>(cap)) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(static_cast<tvg::StrokeCap>(cap)) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("SetStrokeCap fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Shape::StrokeCap Shape::GetStrokeCap() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Dali::CanvasRenderer::Shape::StrokeCap::SQUARE;
  }

  tvg::StrokeCap cap = static_cast<tvg::Shape*>(mTvgShape)->strokeCap();

  return static_cast<Dali::CanvasRenderer::Shape::StrokeCap>(cap);
#endif
  return Dali::CanvasRenderer::Shape::StrokeCap::SQUARE;
}

bool Shape::SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

#ifdef THORVG_VERSION_1
  if(static_cast<tvg::Shape*>(mTvgShape)->strokeJoin(static_cast<tvg::StrokeJoin>(join)) != tvg::Result::Success)
#else
  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(static_cast<tvg::StrokeJoin>(join)) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("SetStrokejoin fail.\n");
    return false;
  }
  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Shape::StrokeJoin Shape::GetStrokeJoin() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Dali::CanvasRenderer::Shape::StrokeJoin::BEVEL;
  }

  tvg::StrokeJoin join = static_cast<tvg::Shape*>(mTvgShape)->strokeJoin();

  return static_cast<Dali::CanvasRenderer::Shape::StrokeJoin>(join);
#endif
  return Dali::CanvasRenderer::Shape::StrokeJoin::BEVEL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
