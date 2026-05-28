/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-outline.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <ft2build.h>
#include FT_OUTLINE_H

namespace Dali::TextAbstraction::Internal
{

namespace
{

struct OutlineDecomposeContext
{
  tvg::Shape* shape{nullptr};
  float currentX{0.0f};
  float currentY{0.0f};
  bool hasOpenContour{false};
};

float ToFloat(FT_Pos value)
{
  return static_cast<float>(value);
}

int MoveToCallback(const FT_Vector* to, void* user)
{
  auto* ctx = static_cast<OutlineDecomposeContext*>(user);
  if(ctx->hasOpenContour)
  {
    ctx->shape->close();
  }

  ctx->currentX = ToFloat(to->x);
  ctx->currentY = ToFloat(to->y);
  ctx->shape->moveTo(ctx->currentX, -ctx->currentY);
  ctx->hasOpenContour = true;
  return 0;
}

int LineToCallback(const FT_Vector* to, void* user)
{
  auto* ctx = static_cast<OutlineDecomposeContext*>(user);
  ctx->currentX = ToFloat(to->x);
  ctx->currentY = ToFloat(to->y);
  ctx->shape->lineTo(ctx->currentX, -ctx->currentY);
  return 0;
}

int ConicToCallback(const FT_Vector* control, const FT_Vector* to, void* user)
{
  auto* ctx = static_cast<OutlineDecomposeContext*>(user);
  const float p0x = ctx->currentX;
  const float p0y = ctx->currentY;
  const float qx  = ToFloat(control->x);
  const float qy  = ToFloat(control->y);
  const float p2x = ToFloat(to->x);
  const float p2y = ToFloat(to->y);

  const float cp1x = p0x + (2.0f / 3.0f) * (qx - p0x);
  const float cp1y = p0y + (2.0f / 3.0f) * (qy - p0y);
  const float cp2x = p2x + (2.0f / 3.0f) * (qx - p2x);
  const float cp2y = p2y + (2.0f / 3.0f) * (qy - p2y);

  ctx->shape->cubicTo(cp1x, -cp1y, cp2x, -cp2y, p2x, -p2y);
  ctx->currentX = p2x;
  ctx->currentY = p2y;
  return 0;
}

int CubicToCallback(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
  auto* ctx = static_cast<OutlineDecomposeContext*>(user);
  ctx->currentX = ToFloat(to->x);
  ctx->currentY = ToFloat(to->y);
  ctx->shape->cubicTo(
    ToFloat(control1->x), -ToFloat(control1->y),
    ToFloat(control2->x), -ToFloat(control2->y),
    ctx->currentX, -ctx->currentY);
  return 0;
}

} // namespace

// ---- Convert FreeType FT_Outline to ThorVG Shape path ----
bool FtOutlineToTvgShape(FT_Outline* outline, tvg::Shape* shape)
{
  if(!outline || !shape || outline->n_contours <= 0)
  {
    return false;
  }

  OutlineDecomposeContext context;
  context.shape = shape;

  FT_Outline_Funcs funcs{};
  funcs.move_to  = MoveToCallback;
  funcs.line_to  = LineToCallback;
  funcs.conic_to = ConicToCallback;
  funcs.cubic_to = CubicToCallback;
  funcs.shift    = 0;
  funcs.delta    = 0;

  if(FT_Outline_Decompose(outline, &funcs, &context) != 0)
  {
    return false;
  }

  if(context.hasOpenContour)
  {
    shape->close();
  }

  return true;
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
