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
#include <dali/internal/canvas-renderer/tizen/shape-impl-tizen.h>

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
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Shape), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

ShapeTizen* ShapeTizen::New()
{
  return new ShapeTizen();
}

ShapeTizen::ShapeTizen()
#ifdef THORVG_SUPPORT
: mTvgShape(nullptr)
#endif
{
  Initialize();
}

ShapeTizen::~ShapeTizen()
{
}

void ShapeTizen::Initialize()
{
#ifdef THORVG_SUPPORT
  mTvgShape = tvg::Shape::gen().release();
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
  }

  Drawable::Create();
  Drawable::SetObject(static_cast<void*>(mTvgShape));
  Drawable::SetDrawableType(Drawable::DrawableTypes::SHAPE);
#endif
}

bool ShapeTizen::AddRect(Rect<float> rect, Vector2 roundedCorner)
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

bool ShapeTizen::AddCircle(Vector2 center, Vector2 radius)
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

bool ShapeTizen::AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie)
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

bool ShapeTizen::AddMoveTo(Vector2 point)
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

bool ShapeTizen::AddLineTo(Vector2 line)
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

bool ShapeTizen::AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint)
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

bool ShapeTizen::Close()
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

bool ShapeTizen::ResetPath()
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

bool ShapeTizen::SetFillColor(Vector4 color)
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

Vector4 ShapeTizen::GetFillColor() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
    return Vector4(0, 0, 0, 0);
  }
  uint8_t r, g, b, a;

  if(static_cast<tvg::Shape*>(mTvgShape)->fillColor(&r, &g, &b, &a) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("GetFillColor fail [%p]\n", this);
    return Vector4(0, 0, 0, 0);
  }
  return Vector4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
#else
  return Vector4::ZERO;
#endif
}

bool ShapeTizen::SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }
  if(static_cast<tvg::Shape*>(mTvgShape)->fill(static_cast<tvg::FillRule>(rule)) != tvg::Result::Success)
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

Dali::CanvasRenderer::Shape::FillRule ShapeTizen::GetFillRule() const
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

bool ShapeTizen::SetStrokeWidth(float width)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(width) != tvg::Result::Success)
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

float ShapeTizen::GetStrokeWidth() const
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

bool ShapeTizen::SetStrokeColor(Vector4 color)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f) != tvg::Result::Success)
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

Vector4 ShapeTizen::GetStrokeColor() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Vector4(0, 0, 0, 0);
  }

  uint8_t r, g, b, a;

  if(static_cast<tvg::Shape*>(mTvgShape)->strokeColor(&r, &g, &b, &a) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("GetStrokeColor fail.\n");
    return Vector4(0, 0, 0, 0);
  }
  return Vector4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
#else
  return Vector4(0, 0, 0, 0);
#endif
}

bool ShapeTizen::SetStrokeDash(const Dali::Vector<float> dashPattern)
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

  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(tvgDashPattern, dashPattern.Count()) != tvg::Result::Success)
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

Dali::Vector<float> ShapeTizen::GetStrokeDash() const
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

bool ShapeTizen::SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }
  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(static_cast<tvg::StrokeCap>(cap)) != tvg::Result::Success)
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

Dali::CanvasRenderer::Shape::StrokeCap ShapeTizen::GetStrokeCap() const
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

bool ShapeTizen::SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }

  if(static_cast<tvg::Shape*>(mTvgShape)->stroke(static_cast<tvg::StrokeJoin>(join)) != tvg::Result::Success)
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

Dali::CanvasRenderer::Shape::StrokeJoin ShapeTizen::GetStrokeJoin() const
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
