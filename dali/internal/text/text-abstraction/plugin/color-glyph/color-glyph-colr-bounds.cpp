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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-bounds.h>
#include <dali/integration-api/debug.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <cmath>

namespace Dali::TextAbstraction::Internal
{

namespace
{

constexpr float PI = 3.14159265358979323846f;
constexpr float FROM_FT_FIXED = 1.0f / 65536.0f;

float FixedToFloat16Dot16(FT_Fixed value)
{
  return static_cast<float>(value) * FROM_FT_FIXED;
}

float ColrAngleToRadians(FT_Fixed angle)
{
  return FixedToFloat16Dot16(angle) * PI;
}

float ColrSkewAngleToShear(FT_Fixed angle)
{
  // This renderer stores transforms in font-space, then y-flips outlines for
  // ThorVG. Use the sign that matches COLRv1 browser rendering after that
  // explicit y-flip. Keep this in sync with color-glyph-colr-paint-graph.cpp.
  return -std::tan(ColrAngleToRadians(angle));
}

} // namespace

// Extend PaintBounds with union operations for internal use.
// These are free functions since PaintBounds is defined in the header without methods.
void PaintBoundsUnion(ColrPaintBounds& bounds, float x1, float y1, float x2, float y2)
{
  if(!bounds.valid)
  {
    bounds.minX = x1; bounds.minY = y1; bounds.maxX = x2; bounds.maxY = y2;
    bounds.valid = true;
  }
  else
  {
    if(x1 < bounds.minX) bounds.minX = x1;
    if(y1 < bounds.minY) bounds.minY = y1;
    if(x2 > bounds.maxX) bounds.maxX = x2;
    if(y2 > bounds.maxY) bounds.maxY = y2;
  }
}

void PaintBoundsUnionBBox(ColrPaintBounds& bounds, float bMinX, float bMinY, float bMaxX, float bMaxY,
                          const TransformState& transform)
{
  float corners[4][2] = {
    {bMinX, bMinY}, {bMaxX, bMinY},
    {bMinX, bMaxY}, {bMaxX, bMaxY}
  };
  for(int i = 0; i < 4; i++)
  {
    float x = corners[i][0];
    float y = corners[i][1];
    float tx = transform.matrix[0] * x + transform.matrix[1] * y + transform.matrix[4];
    float ty = transform.matrix[2] * x + transform.matrix[3] * y + transform.matrix[5];
    if(!bounds.valid)
    {
      bounds.minX = tx; bounds.minY = ty; bounds.maxX = tx; bounds.maxY = ty;
      bounds.valid = true;
    }
    else
    {
      if(tx < bounds.minX) bounds.minX = tx;
      if(ty < bounds.minY) bounds.minY = ty;
      if(tx > bounds.maxX) bounds.maxX = tx;
      if(ty > bounds.maxY) bounds.maxY = ty;
    }
  }
}

// Compute outline bbox for a single glyph (in font units, y-up, unscaled)
ColrPaintBounds ComputeGlyphOutlineBounds(FT_Face ftFace, uint32_t glyphIndex)
{
  ColrPaintBounds result;
  FT_Error error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
  if(error != FT_Err_Ok) return result;

  FT_Outline* outline = &ftFace->glyph->outline;
  if(outline->n_contours <= 0 || outline->n_points <= 0) return result;

  FT_BBox ftBBox;
  FT_Outline_Get_BBox(outline, &ftBBox);
  if(ftBBox.xMin >= ftBBox.xMax || ftBBox.yMin >= ftBBox.yMax) return result;

  result.minX = static_cast<float>(ftBBox.xMin);
  result.minY = static_cast<float>(ftBBox.yMin);
  result.maxX = static_cast<float>(ftBBox.xMax);
  result.maxY = static_cast<float>(ftBBox.yMax);
  result.valid = true;
  return result;
}

// Traverse paint graph to compute union bounds of all PaintGlyph outlines
void TraversePaintBounds(FT_Face ftFace, FT_OpaquePaint opaquePaint, ColrPaintBounds& bounds, TransformState& geometryTransform, uint32_t debugGlyph, uint32_t depth)
{
  if(depth > 64 || !opaquePaint.p) return;

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, opaquePaint, &paint)) return;

  switch(paint.format)
  {
    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    {
      FT_LayerIterator& layerIter = paint.u.colr_layers.layer_iterator;
      FT_OpaquePaint layerOpaque;
      while(FT_Get_Paint_Layers(ftFace, &layerIter, &layerOpaque))
      {
        TraversePaintBounds(ftFace, layerOpaque, bounds, geometryTransform, debugGlyph, depth + 1);
      }
      break;
    }

    case FT_COLR_PAINTFORMAT_GLYPH:
    {
      uint32_t glyphID = paint.u.glyph.glyphID;
      ColrPaintBounds glyphBounds = ComputeGlyphOutlineBounds(ftFace, glyphID);
      if(glyphBounds.valid)
      {
        if(geometryTransform.hasTransform)
        {
          PaintBoundsUnionBBox(bounds, glyphBounds.minX, glyphBounds.minY, glyphBounds.maxX, glyphBounds.maxY, geometryTransform);
        }
        else
        {
          PaintBoundsUnion(bounds, glyphBounds.minX, glyphBounds.minY, glyphBounds.maxX, glyphBounds.maxY);
        }
      }
      // Child paint is fill style only, no geometry contribution
      break;
    }

    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      TransformState prevTransform = geometryTransform;
      const auto& tf = paint.u.transform;
      float xx = static_cast<float>(tf.affine.xx) / 65536.0f;
      float xy = static_cast<float>(tf.affine.xy) / 65536.0f;
      float yx = static_cast<float>(tf.affine.yx) / 65536.0f;
      float yy = static_cast<float>(tf.affine.yy) / 65536.0f;
      float dx = static_cast<float>(tf.affine.dx) / 65536.0f;
      float dy = static_cast<float>(tf.affine.dy) / 65536.0f;
      geometryTransform.Apply(xx, xy, yx, yy, dx, dy);
      TraversePaintBounds(ftFace, tf.paint, bounds, geometryTransform, debugGlyph, depth + 1);
      geometryTransform = prevTransform;
      break;
    }

    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      TransformState prevTransform = geometryTransform;
      float dx = static_cast<float>(paint.u.translate.dx) / 65536.0f;
      float dy = static_cast<float>(paint.u.translate.dy) / 65536.0f;
      geometryTransform.Apply(1, 0, 0, 1, dx, dy);
      TraversePaintBounds(ftFace, paint.u.translate.paint, bounds, geometryTransform, debugGlyph, depth + 1);
      geometryTransform = prevTransform;
      break;
    }

    case FT_COLR_PAINTFORMAT_SCALE:
    {
      TransformState prevTransform = geometryTransform;
      float sx = static_cast<float>(paint.u.scale.scale_x) / 65536.0f;
      float sy = static_cast<float>(paint.u.scale.scale_y) / 65536.0f;
      float cx = static_cast<float>(paint.u.scale.center_x) / 65536.0f;
      float cy = static_cast<float>(paint.u.scale.center_y) / 65536.0f;
      geometryTransform.Apply(1, 0, 0, 1, cx, cy);
      geometryTransform.Apply(sx, 0, 0, sy, 0, 0);
      geometryTransform.Apply(1, 0, 0, 1, -cx, -cy);
      TraversePaintBounds(ftFace, paint.u.scale.paint, bounds, geometryTransform, debugGlyph, depth + 1);
      geometryTransform = prevTransform;
      break;
    }

    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      TransformState prevTransform = geometryTransform;
      float angle = ColrAngleToRadians(paint.u.rotate.angle);
      float cx = FixedToFloat16Dot16(paint.u.rotate.center_x);
      float cy = FixedToFloat16Dot16(paint.u.rotate.center_y);
      float cosA = std::cos(angle);
      float sinA = std::sin(angle);
      geometryTransform.Apply(1, 0, 0, 1, cx, cy);
      geometryTransform.Apply(cosA, -sinA, sinA, cosA, 0, 0);
      geometryTransform.Apply(1, 0, 0, 1, -cx, -cy);
      TraversePaintBounds(ftFace, paint.u.rotate.paint, bounds, geometryTransform, debugGlyph, depth + 1);
      geometryTransform = prevTransform;
      break;
    }

    case FT_COLR_PAINTFORMAT_SKEW:
    {
      TransformState prevTransform = geometryTransform;
      float skewX = ColrSkewAngleToShear(paint.u.skew.x_skew_angle);
      float skewY = ColrSkewAngleToShear(paint.u.skew.y_skew_angle);
      float cx = FixedToFloat16Dot16(paint.u.skew.center_x);
      float cy = FixedToFloat16Dot16(paint.u.skew.center_y);
      geometryTransform.Apply(1, 0, 0, 1, cx, cy);
      geometryTransform.Apply(1, skewX, skewY, 1, 0, 0);
      geometryTransform.Apply(1, 0, 0, 1, -cx, -cy);
      TraversePaintBounds(ftFace, paint.u.skew.paint, bounds, geometryTransform, debugGlyph, depth + 1);
      geometryTransform = prevTransform;
      break;
    }

    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    {
      // PaintColrGlyph: traverse the referenced glyph's paint graph for bounds.
      // This ensures component glyphs contribute to the overall paint bounds,
      // preventing clipping when the referenced glyph extends beyond the base glyph.
      const GlyphIndex referencedGlyphID = paint.u.colr_glyph.glyphID;

      FT_OpaquePaint referencedRootPaint;
      referencedRootPaint.p = nullptr;
      referencedRootPaint.insert_root_transform = false;

      const FT_Bool hasReferencedPaint = FT_Get_Color_Glyph_Paint(
        ftFace,
        referencedGlyphID,
        COLR_ROOT_TRANSFORM_OPTION,
        &referencedRootPaint);

      if(hasReferencedPaint && referencedRootPaint.p != nullptr)
      {
        TraversePaintBounds(ftFace, referencedRootPaint, bounds, geometryTransform, debugGlyph, depth + 1);
      }
      break;
    }

    case FT_COLR_PAINTFORMAT_COMPOSITE:
    {
      // PaintComposite bounds: union of backdrop and source bounds.
      //
      // Policy: Use union of backdrop and source bounds for all composite modes.
      // This is a conservative bound: the actual output bounds may be
      // smaller for certain modes (e.g., SrcIn only keeps the intersection area,
      // DstIn only keeps the backdrop area within the source shape). However,
      // using union is safe for atlas clipping prevention — it ensures the
      // composite result is never clipped. A more precise per-mode bounds
      // calculation can be added later if needed.
      ColrPaintBounds backdropBounds;
      ColrPaintBounds sourceBounds;

      // Traverse backdrop subgraph for bounds
      if(paint.u.composite.backdrop_paint.p != nullptr)
      {
        TraversePaintBounds(ftFace, paint.u.composite.backdrop_paint, backdropBounds, geometryTransform, debugGlyph, depth + 1);
      }

      // Traverse source subgraph for bounds
      if(paint.u.composite.source_paint.p != nullptr)
      {
        TraversePaintBounds(ftFace, paint.u.composite.source_paint, sourceBounds, geometryTransform, debugGlyph, depth + 1);
      }

      // Union backdrop and source bounds into the output
      if(backdropBounds.valid)
      {
        PaintBoundsUnion(bounds, backdropBounds.minX, backdropBounds.minY, backdropBounds.maxX, backdropBounds.maxY);
      }
      if(sourceBounds.valid)
      {
        PaintBoundsUnion(bounds, sourceBounds.minX, sourceBounds.minY, sourceBounds.maxX, sourceBounds.maxY);
      }

      if(IsColrDebugTraceEnabled(debugGlyph))
      {
        const bool hasBackdrop = (paint.u.composite.backdrop_paint.p != nullptr);
        const bool hasSource   = (paint.u.composite.source_paint.p != nullptr);

        FT_PaintFormat backdropFormat = static_cast<FT_PaintFormat>(-1);
        const char* backdropName = "(null)";
        if(hasBackdrop)
        {
          FT_COLR_Paint backdropPaint;
          if(FT_Get_Paint(ftFace, paint.u.composite.backdrop_paint, &backdropPaint))
          {
            backdropFormat = backdropPaint.format;
            backdropName = PaintFormatToString(backdropPaint.format);
          }
          else
          {
            backdropName = "(FT_Get_Paint-failed)";
          }
        }

        FT_PaintFormat sourceFormat = static_cast<FT_PaintFormat>(-1);
        const char* sourceName = "(null)";
        if(hasSource)
        {
          FT_COLR_Paint sourcePaint;
          if(FT_Get_Paint(ftFace, paint.u.composite.source_paint, &sourcePaint))
          {
            sourceFormat = sourcePaint.format;
            sourceName = PaintFormatToString(sourcePaint.format);
          }
          else
          {
            sourceName = "(FT_Get_Paint-failed)";
          }
        }

        DALI_LOG_RELEASE_INFO("COLOR_GLYPH_COLR_RENDER CompositeBounds depth:%u mode:%d(%s) backdrop:%s format:%d(%s) bounds:[%d,(%.1f,%.1f)-(%.1f,%.1f)] source:%s format:%d(%s) bounds:[%d,(%.1f,%.1f)-(%.1f,%.1f)] union:[%d,(%.1f,%.1f)-(%.1f,%.1f)]\n",
                       depth,
                       static_cast<int>(paint.u.composite.composite_mode), CompositeModeToString(paint.u.composite.composite_mode),
                       hasBackdrop ? "yes" : "no", static_cast<int>(backdropFormat), backdropName,
                       backdropBounds.valid, backdropBounds.minX, backdropBounds.minY, backdropBounds.maxX, backdropBounds.maxY,
                       hasSource ? "yes" : "no", static_cast<int>(sourceFormat), sourceName,
                       sourceBounds.valid, sourceBounds.minX, sourceBounds.minY, sourceBounds.maxX, sourceBounds.maxY,
                       bounds.valid, bounds.minX, bounds.minY, bounds.maxX, bounds.maxY);
      }
      break;
    }

    default:
      break;
  }
}

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
