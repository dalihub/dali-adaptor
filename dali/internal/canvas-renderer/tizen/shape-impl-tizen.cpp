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
: mTvgShape(nullptr)
{
  Initialize();
}

ShapeTizen::~ShapeTizen()
{
}

void ShapeTizen::Initialize()
{
  mTvgShape = tvg::Shape::gen().release();
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null [%p]\n", this);
  }

  Drawable::Create();
  Drawable::SetObject(static_cast<void*>(mTvgShape));
}

bool ShapeTizen::AddRect(Rect<float> rect, Vector2 roundedCorner)
{
  if(!mTvgShape)
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
}

bool ShapeTizen::AddCircle(Vector2 center, Vector2 radius)
{
  if(!mTvgShape)
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
}

bool ShapeTizen::AddArc(Vector2 center, float radius, float startAngle, float sweep, bool pie)
{
  if(!mTvgShape)
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
}

bool ShapeTizen::AddMoveTo(Vector2 point)
{
  if(!mTvgShape)
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
}

bool ShapeTizen::AddLineTo(Vector2 line)
{
  if(!mTvgShape)
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
}

bool ShapeTizen::AddCubicTo(Vector2 controlPoint1, Vector2 controlPoint2, Vector2 endPoint)
{
  if(!mTvgShape)
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
}

bool ShapeTizen::Close()
{
  if(!mTvgShape)
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
}

bool ShapeTizen::ResetPath()
{
#ifdef THORVG_SUPPORT
  if(!mTvgShape)
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
  if(!mTvgShape)
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
}

Vector4 ShapeTizen::GetFillColor() const
{
  if(!mTvgShape)
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
}

bool ShapeTizen::SetFillRule(Dali::CanvasRenderer::Shape::FillRule rule)
{
  if(!mTvgShape)
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
}

Dali::CanvasRenderer::Shape::FillRule ShapeTizen::GetFillRule() const
{
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Dali::CanvasRenderer::Shape::FillRule::WINDING;
  }

  tvg::FillRule rule = static_cast<tvg::Shape*>(mTvgShape)->fillRule();

  return static_cast<Dali::CanvasRenderer::Shape::FillRule>(rule);
}

bool ShapeTizen::SetStrokeWidth(float width)
{
  if(!mTvgShape)
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
}

float ShapeTizen::GetStrokeWidth() const
{
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return false;
  }
  return static_cast<tvg::Shape*>(mTvgShape)->strokeWidth();
}

bool ShapeTizen::SetStrokeColor(Vector4 color)
{
  if(!mTvgShape)
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
}

Vector4 ShapeTizen::GetStrokeColor() const
{
  if(!mTvgShape)
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
}

bool ShapeTizen::SetStrokeDash(const Dali::Vector<float> dashPattern)
{
  if(!mTvgShape)
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
}

Dali::Vector<float> ShapeTizen::GetStrokeDash() const
{
  if(!mTvgShape)
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
}

bool ShapeTizen::SetStrokeCap(Dali::CanvasRenderer::Shape::StrokeCap cap)
{
  if(!mTvgShape)
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
}

Dali::CanvasRenderer::Shape::StrokeCap ShapeTizen::GetStrokeCap() const
{
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Dali::CanvasRenderer::Shape::StrokeCap::SQUARE;
  }

  tvg::StrokeCap cap = static_cast<tvg::Shape*>(mTvgShape)->strokeCap();

  return static_cast<Dali::CanvasRenderer::Shape::StrokeCap>(cap);
}

bool ShapeTizen::SetStrokeJoin(Dali::CanvasRenderer::Shape::StrokeJoin join)
{
  if(!mTvgShape)
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
}

Dali::CanvasRenderer::Shape::StrokeJoin ShapeTizen::GetStrokeJoin() const
{
  if(!mTvgShape)
  {
    DALI_LOG_ERROR("Shape is null\n");
    return Dali::CanvasRenderer::Shape::StrokeJoin::BEVEL;
  }

  tvg::StrokeJoin join = static_cast<tvg::Shape*>(mTvgShape)->strokeJoin();

  return static_cast<Dali::CanvasRenderer::Shape::StrokeJoin>(join);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
