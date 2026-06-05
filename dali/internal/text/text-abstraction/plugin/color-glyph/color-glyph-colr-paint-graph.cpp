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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-paint-graph.h>

#if !DALI_ENABLE_COLR_V1_RENDERER
// empty compilation unit
#else

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-bounds.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-common.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-composite.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-cpal.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-gradient.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-outline.h>
#include <dali/integration-api/debug.h>

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <vector>

namespace Dali::TextAbstraction::Internal
{

namespace
{

#define COLR_TRACE(glyphIndex, ...)          \
  do                                         \
  {                                          \
    if(IsColrDebugTraceEnabled(glyphIndex))  \
    {                                        \
      DALI_LOG_RELEASE_INFO(__VA_ARGS__);    \
    }                                        \
  } while(false)

#define COLR_UNSUPPORTED(...) DALI_LOG_RELEASE_INFO(__VA_ARGS__)

// Forward declaration for fill helpers used before ApplyPaintToShape() is defined.
bool ApplyPaintToShape(FT_OpaquePaint opaquePaint, tvg::Shape* shape, PaintContext& ctx, uint32_t depth);

constexpr float PI = 3.14159265358979323846f;
constexpr float FROM_FT_FIXED = 1.0f / 65536.0f;

float FixedToFloat16Dot16(FT_Fixed value)
{
  return static_cast<float>(value) * FROM_FT_FIXED;
}

float ColrAngleToRadians(FT_Fixed angle)
{
  // FreeType stores COLRv1 Rotate/Skew angles as degrees / 180.0 in 16.16.
  // C++ trigonometric functions expect radians.
  return FixedToFloat16Dot16(angle) * PI;
}

float ColrSkewAngleToShear(FT_Fixed angle)
{
  // A skew matrix needs the shear factor, not the encoded angle value.
  // This renderer stores transforms in font-space, then y-flips outlines for
  // ThorVG. Use the sign that matches COLRv1 browser rendering after that
  // explicit y-flip. Keep this in sync with color-glyph-colr-bounds.cpp.
  return -std::tan(ColrAngleToRadians(angle));
}

// ---- Paint build result and bounded offscreen buffer contracts ----

/**
 * @brief Internal build status for the COLRv1 Scene-based paint builder.
 *
 * The builder evaluates a COLRv1 paint node into one renderable result instead
 * of immediately appending every leaf paint to a flat list.
 */
enum class PaintBuildStatus
{
  SUCCESS,
  UNSUPPORTED,
  FAILED,
  REQUIRES_OFFSCREEN
};

/**
 * @brief Internal result type for the COLRv1 Scene-based paint builder.
 *
 * The builder evaluates one COLRv1 paint node into either a single caller-owned
 * @c tvg::Paint* or an explicit unsupported/failure state. On
 * success, @c paint is owned by the caller until it is transferred to a
 * ThorVG Scene/Canvas or released with @c tvg::Paint::rel().
 */
struct PaintBuildResult
{
  PaintBuildStatus status{PaintBuildStatus::FAILED};
  tvg::Paint* paint{nullptr};
  FT_PaintFormat unsupportedFormat{static_cast<FT_PaintFormat>(0)};
  FT_Composite_Mode unsupportedCompositeMode{static_cast<FT_Composite_Mode>(0)};

  static PaintBuildResult Success(tvg::Paint* paint)
  {
    PaintBuildResult result;
    if(!paint)
    {
      result.status = PaintBuildStatus::FAILED;
      return result;
    }

    result.status = PaintBuildStatus::SUCCESS;
    result.paint = paint;
    return result;
  }

  static PaintBuildResult Unsupported(FT_PaintFormat format)
  {
    PaintBuildResult result;
    result.status = PaintBuildStatus::UNSUPPORTED;
    result.unsupportedFormat = format;
    return result;
  }

  static PaintBuildResult UnsupportedComposite(FT_Composite_Mode mode)
  {
    PaintBuildResult result;
    result.status = PaintBuildStatus::UNSUPPORTED;
    result.unsupportedFormat = FT_COLR_PAINTFORMAT_COMPOSITE;
    result.unsupportedCompositeMode = mode;
    return result;
  }

  static PaintBuildResult Failed()
  {
    PaintBuildResult result;
    result.status = PaintBuildStatus::FAILED;
    return result;
  }

  /**
   * @brief Marks an exact offscreen fallback boundary.
   *
   * This state means the paint subtree cannot currently be represented exactly
   * without bounded offscreen evaluation. It is not an immediate rendering
   * failure: an offscreen-capable builder route may turn it into an exact
   * bounded offscreen fallback. Other callers must fall back to an explicit
   * boundary or fail closed without synthesizing replacement paints.
   */
  static PaintBuildResult RequiresOffscreenResult()
  {
    PaintBuildResult result;
    result.status = PaintBuildStatus::REQUIRES_OFFSCREEN;
    return result;
  }

  bool IsSuccess() const
  {
    return status == PaintBuildStatus::SUCCESS && paint != nullptr;
  }

  bool IsUnsupported() const
  {
    return status == PaintBuildStatus::UNSUPPORTED;
  }

  bool IsFailed() const
  {
    return status == PaintBuildStatus::FAILED;
  }

  bool RequiresOffscreen() const
  {
    return status == PaintBuildStatus::REQUIRES_OFFSCREEN;
  }
};

constexpr uint32_t MAX_OFFSCREEN_WIDTH  = 1019u;
constexpr uint32_t MAX_OFFSCREEN_HEIGHT = 1019u;
constexpr uint64_t MAX_OFFSCREEN_PIXELS =
  static_cast<uint64_t>(MAX_OFFSCREEN_WIDTH) * static_cast<uint64_t>(MAX_OFFSCREEN_HEIGHT);
constexpr float OFFSCREEN_PADDING_PIXELS = 2.0f;

struct OffscreenBounds
{
  bool valid{false};
  bool capExceeded{false};
  bool overflow{false};
  ColrPaintBounds conservativeBounds{};
  int32_t pixelX{0};
  int32_t pixelY{0};
  uint32_t pixelWidth{0u};
  uint32_t pixelHeight{0u};
  uint64_t pixelCount{0u};
};

enum class OffscreenAlpha
{
  UNKNOWN,
  PREMULTIPLIED,
  STRAIGHT
};

struct OffscreenBuffer
{
  CompositeBuffer surface{};
  OffscreenBounds bounds{};
  OffscreenAlpha alpha{OffscreenAlpha::UNKNOWN};
  float scale{0.0f};
  float offsetX{0.0f};
  float offsetY{0.0f};

  OffscreenBuffer() = default;
  ~OffscreenBuffer()
  {
    Reset();
  }

  OffscreenBuffer(const OffscreenBuffer&) = delete;
  OffscreenBuffer& operator=(const OffscreenBuffer&) = delete;

  OffscreenBuffer(OffscreenBuffer&& other) noexcept
  {
    MoveFrom(other);
  }

  OffscreenBuffer& operator=(OffscreenBuffer&& other) noexcept
  {
    if(this != &other)
    {
      Reset();
      MoveFrom(other);
    }
    return *this;
  }

  bool IsValid() const
  {
    return surface.valid && surface.buffer != nullptr;
  }

  uint64_t ByteSize() const
  {
    return bounds.pixelCount * static_cast<uint64_t>(sizeof(uint32_t));
  }

  void Reset()
  {
    FreeCompositeBuffer(surface);
  }

  void MoveFrom(OffscreenBuffer& other)
  {
    surface = other.surface;
    bounds  = other.bounds;
    alpha   = other.alpha;
    scale   = other.scale;
    offsetX = other.offsetX;
    offsetY = other.offsetY;

    other.surface = CompositeBuffer();
  }
};

/**
 * @brief Converts conservative paint bounds into a guarded offscreen rectangle.
 *
 * Current offscreen bounds contract:
 *   fontX -> pixelX = fontX * scale + offsetX
 *   fontY -> pixelY = offsetY - fontY * scale
 * The signed pixelX/Y origin is preserved so the subtree can be rendered in
 * local offscreen coordinates and later placed back in parent pixel space.
 */
OffscreenBounds ValidateOffscreenBounds(
  const ColrPaintBounds& bounds,
  float scale,
  float offsetX,
  float offsetY)
{
  OffscreenBounds result;
  result.conservativeBounds = bounds;

  if(!bounds.valid)
  {
    return result;
  }

  if(!std::isfinite(bounds.minX) || !std::isfinite(bounds.minY) ||
     !std::isfinite(bounds.maxX) || !std::isfinite(bounds.maxY) ||
     !std::isfinite(offsetX) || !std::isfinite(offsetY))
  {
    return result;
  }

  if(!std::isfinite(scale) || scale <= 0.0f)
  {
    return result;
  }

  if(bounds.maxX <= bounds.minX || bounds.maxY <= bounds.minY)
  {
    return result;
  }

  const double padding = static_cast<double>(OFFSCREEN_PADDING_PIXELS);
  const double scaleD = static_cast<double>(scale);
  const double offsetXD = static_cast<double>(offsetX);
  const double offsetYD = static_cast<double>(offsetY);

  const double pixelMinX = std::floor(static_cast<double>(bounds.minX) * scaleD + offsetXD - padding);
  const double pixelMaxX = std::ceil(static_cast<double>(bounds.maxX) * scaleD + offsetXD + padding);
  const double pixelMinY = std::floor(offsetYD - static_cast<double>(bounds.maxY) * scaleD - padding);
  const double pixelMaxY = std::ceil(offsetYD - static_cast<double>(bounds.minY) * scaleD + padding);

  if(!std::isfinite(pixelMinX) || !std::isfinite(pixelMaxX) ||
     !std::isfinite(pixelMinY) || !std::isfinite(pixelMaxY))
  {
    result.overflow = true;
    return result;
  }

  const double int32Min = static_cast<double>(std::numeric_limits<int32_t>::min());
  const double int32Max = static_cast<double>(std::numeric_limits<int32_t>::max());
  if(pixelMinX < int32Min || pixelMinX > int32Max ||
     pixelMinY < int32Min || pixelMinY > int32Max ||
     pixelMaxX < int32Min || pixelMaxX > int32Max ||
     pixelMaxY < int32Min || pixelMaxY > int32Max)
  {
    result.overflow = true;
    return result;
  }

  const double widthD = pixelMaxX - pixelMinX;
  const double heightD = pixelMaxY - pixelMinY;
  if(widthD <= 0.0 || heightD <= 0.0)
  {
    return result;
  }

  if(widthD > static_cast<double>(MAX_OFFSCREEN_WIDTH))
  {
    result.capExceeded = true;
    return result;
  }

  if(heightD > static_cast<double>(MAX_OFFSCREEN_HEIGHT))
  {
    result.capExceeded = true;
    return result;
  }

  result.pixelX = static_cast<int32_t>(pixelMinX);
  result.pixelY = static_cast<int32_t>(pixelMinY);
  result.pixelWidth = static_cast<uint32_t>(widthD);
  result.pixelHeight = static_cast<uint32_t>(heightD);
  result.pixelCount = static_cast<uint64_t>(result.pixelWidth) * static_cast<uint64_t>(result.pixelHeight);

  if(result.pixelCount > MAX_OFFSCREEN_PIXELS)
  {
    result.capExceeded = true;
    return result;
  }

  result.valid = true;
  return result;
}

// Used by composite bounds before the ClipBox helper block below.
bool TryBuildClipBoxPaintBounds(FT_Face ftFace, const PaintContext& ctx, ColrPaintBounds& bounds);

OffscreenBounds BuildCompositeOffscreenBounds(
  FT_Face ftFace,
  const FT_PaintComposite& composite,
  const PaintContext& ctx,
  uint32_t depth)
{
  if(!ftFace || depth > 64u)
  {
    return OffscreenBounds();
  }

  ColrPaintBounds bounds;
  if(TryBuildClipBoxPaintBounds(ftFace, ctx, bounds))
  {
    return ValidateOffscreenBounds(bounds, ctx.scale, ctx.offsetX, ctx.offsetY);
  }

  if(composite.backdrop_paint.p)
  {
    TransformState backdropTransform = ctx.geometryTransform;
    TraversePaintBounds(ftFace, composite.backdrop_paint, bounds, backdropTransform, ctx.debugGlyph, depth + 1u);
  }

  if(composite.source_paint.p)
  {
    TransformState sourceTransform = ctx.geometryTransform;
    TraversePaintBounds(ftFace, composite.source_paint, bounds, sourceTransform, ctx.debugGlyph, depth + 1u);
  }

  return ValidateOffscreenBounds(bounds, ctx.scale, ctx.offsetX, ctx.offsetY);
}

OffscreenBuffer AllocateOffscreenBuffer(const OffscreenBounds& bounds)
{
  OffscreenBuffer result;
  result.bounds = bounds;

  if(!bounds.valid)
  {
    return result;
  }

  result.surface = AllocateCompositeBuffer(bounds.pixelWidth, bounds.pixelHeight);
  if(!result.surface.valid)
  {
    return result;
  }

  result.alpha = OffscreenAlpha::PREMULTIPLIED;
  return result;
}

PaintContext BuildOffscreenPaintContext(
  FT_Face ftFace,
  const PaintContext& ctx,
  const OffscreenBounds& bounds)
{
  PaintContext offscreenCtx;
  offscreenCtx.ftFace = ftFace;
  offscreenCtx.paletteIndex = ctx.paletteIndex;
  offscreenCtx.targetWidth = bounds.pixelWidth;
  offscreenCtx.targetHeight = bounds.pixelHeight;
  offscreenCtx.unitsPerEm = ctx.unitsPerEm;
  if(offscreenCtx.unitsPerEm <= 0.0f)
  {
    offscreenCtx.unitsPerEm = ftFace ? static_cast<float>(ftFace->units_per_EM) : 1000.0f;
    if(offscreenCtx.unitsPerEm <= 0.0f)
    {
      offscreenCtx.unitsPerEm = 1000.0f;
    }
  }
  offscreenCtx.scale = ctx.scale;
  // Render into local offscreen pixel space by subtracting the guarded pixel
  // origin. WrapOffscreenPicture() adds the same origin back as a translation.
  offscreenCtx.offsetX = ctx.offsetX - static_cast<float>(bounds.pixelX);
  offscreenCtx.offsetY = ctx.offsetY - static_cast<float>(bounds.pixelY);
  offscreenCtx.geometryTransform = ctx.geometryTransform;
  offscreenCtx.debugGlyph = ctx.debugGlyph;
  offscreenCtx.allowOffscreenCompositeResult = false;
  offscreenCtx.rasterizer = ctx.rasterizer;
  offscreenCtx.variationsHash = ctx.variationsHash;
  // Parent/root ClipBox only selects the already-created isolated surface.
  // Do not propagate it into nested Composite operands; they compute their own
  // isolation bounds unless a child explicitly opts into referenced glyph bounds.
  offscreenCtx.useClipBoxBounds = false;
  offscreenCtx.clipBoxBoundsGlyph = 0u;
  offscreenCtx.colrGlyphStack = ctx.colrGlyphStack;
  return offscreenCtx;
}

// ---- ThorVG paint ownership and offscreen draw helpers ----

void ReleasePaints(std::vector<tvg::Paint*>& paints)
{
  for(auto*& paint : paints)
  {
    if(paint)
    {
      tvg::Paint::rel(paint);
      paint = nullptr;
    }
  }
  paints.clear();
}

bool DrawPaintsToOffscreen(OffscreenBuffer& target, std::vector<tvg::Paint*>& paints)
{
  if(!target.IsValid())
  {
    ReleasePaints(paints);
    return false;
  }

  tvg::SwCanvas* canvas = tvg::SwCanvas::gen();
  if(!canvas)
  {
    ReleasePaints(paints);
    target.Reset();
    return false;
  }

  if(canvas->target(reinterpret_cast<uint32_t*>(target.surface.buffer),
                    target.surface.width,
                    target.surface.width,
                    target.surface.height,
                    tvg::ColorSpace::ARGB8888) != tvg::Result::Success)
  {
    ReleasePaints(paints);
    delete canvas;
    target.Reset();
    return false;
  }

  for(auto*& paint : paints)
  {
    if(paint)
    {
      if(canvas->add(paint) != tvg::Result::Success)
      {
        ReleasePaints(paints);
        delete canvas;
        target.Reset();
        return false;
      }
      paint = nullptr;
    }
  }
  paints.clear();

  if(canvas->draw(true) != tvg::Result::Success ||
     canvas->sync() != tvg::Result::Success)
  {
    delete canvas;
    target.Reset();
    return false;
  }

  // ThorVG ARGB8888 canvas output is premultiplied. CompositeBuffers() uses
  // straight/unassociated color channels in its W3C alpha math, and the final
  // root color glyph upload path also expects straight BGRA before it applies
  // its own premultiplication step.
  UnpremultiplyBgraBuffer(target.surface.buffer,
                          target.surface.width,
                          target.surface.height,
                          target.surface.stride);
  target.alpha = OffscreenAlpha::STRAIGHT;

  delete canvas;
  return true;
}

bool DrawBuiltPaintToOffscreen(OffscreenBuffer& target, PaintBuildResult& buildResult)
{
  if(!buildResult.IsSuccess())
  {
    return false;
  }

  std::vector<tvg::Paint*> paints;
  paints.push_back(buildResult.paint);
  buildResult.paint = nullptr;
  return DrawPaintsToOffscreen(target, paints);
}

bool IsStandaloneFillPaint(FT_PaintFormat format)
{
  return format == FT_COLR_PAINTFORMAT_SOLID ||
         format == FT_COLR_PAINTFORMAT_LINEAR_GRADIENT ||
         format == FT_COLR_PAINTFORMAT_RADIAL_GRADIENT;
}

bool IsStandaloneFillTreeForOffscreen(FT_Face ftFace, FT_OpaquePaint opaquePaint, uint32_t depth)
{
  if(!ftFace || !opaquePaint.p || depth > 64u)
  {
    return false;
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, opaquePaint, &paint))
  {
    return false;
  }

  if(IsStandaloneFillPaint(paint.format))
  {
    return true;
  }

  switch(paint.format)
  {
    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      return IsStandaloneFillTreeForOffscreen(ftFace, paint.u.transform.paint, depth + 1u);
    }
    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      return IsStandaloneFillTreeForOffscreen(ftFace, paint.u.translate.paint, depth + 1u);
    }
    case FT_COLR_PAINTFORMAT_SCALE:
    {
      return IsStandaloneFillTreeForOffscreen(ftFace, paint.u.scale.paint, depth + 1u);
    }
    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      return IsStandaloneFillTreeForOffscreen(ftFace, paint.u.rotate.paint, depth + 1u);
    }
    case FT_COLR_PAINTFORMAT_SKEW:
    {
      return IsStandaloneFillTreeForOffscreen(ftFace, paint.u.skew.paint, depth + 1u);
    }
    default:
    {
      return false;
    }
  }
}

bool RenderStandaloneFillToOffscreen(
  FT_OpaquePaint paint,
  OffscreenBuffer& target,
  PaintContext& offscreenCtx,
  uint32_t depth)
{
  if(!target.IsValid() || offscreenCtx.scale <= 0.0f)
  {
    target.Reset();
    return false;
  }

  const float fontMinX = (0.0f - offscreenCtx.offsetX) / offscreenCtx.scale;
  const float fontMaxX = (static_cast<float>(target.surface.width) - offscreenCtx.offsetX) / offscreenCtx.scale;
  const float pathMinY = -offscreenCtx.offsetY / offscreenCtx.scale;
  const float pathMaxY = (static_cast<float>(target.surface.height) - offscreenCtx.offsetY) / offscreenCtx.scale;

  tvg::Shape* rect = tvg::Shape::gen();
  if(!rect)
  {
    target.Reset();
    return false;
  }

  rect->moveTo(fontMinX, pathMinY);
  rect->lineTo(fontMaxX, pathMinY);
  rect->lineTo(fontMaxX, pathMaxY);
  rect->lineTo(fontMinX, pathMaxY);
  rect->close();
  rect->fillRule(tvg::FillRule::NonZero);

  if(!ApplyPaintToShape(paint, rect, offscreenCtx, depth + 1u))
  {
    tvg::Paint::rel(rect);
    target.Reset();
    return false;
  }

  if(offscreenCtx.geometryTransform.hasTransform)
  {
    tvg::Matrix geometryMatrix = BuildGeometryMatrix(offscreenCtx.geometryTransform, offscreenCtx.scale, offscreenCtx.offsetX, offscreenCtx.offsetY);
    rect->transform(geometryMatrix);
  }
  else
  {
    rect->scale(offscreenCtx.scale);
    rect->translate(offscreenCtx.offsetX, offscreenCtx.offsetY);
  }

  std::vector<tvg::Paint*> paints;
  paints.push_back(rect);
  return DrawPaintsToOffscreen(target, paints);
}

OffscreenBuffer RenderPaintToOffscreen(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  const OffscreenBounds& bounds,
  const PaintContext& ctx,
  uint32_t depth);

PaintBuildResult BuildPaintNodeForSceneBuilder(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  PaintContext& ctx,
  uint32_t depth);

void ReleasePaintIfNotNull(tvg::Paint*& paint);

OffscreenBuffer BuildCompositeOffscreenBuffer(
  FT_Face ftFace,
  const FT_PaintComposite& composite,
  const OffscreenBounds& bounds,
  const PaintContext& ctx,
  uint32_t depth);

OffscreenBuffer RenderPaintToOffscreen(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  const OffscreenBounds& bounds,
  const PaintContext& ctx,
  uint32_t depth)
{
  OffscreenBuffer target = AllocateOffscreenBuffer(bounds);
  if(!target.IsValid())
  {
    return target;
  }

  if(!ftFace || !opaquePaint.p || depth > 64u)
  {
    target.Reset();
    return target;
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, opaquePaint, &paint))
  {
    target.Reset();
    return target;
  }

  if(paint.format == FT_COLR_PAINTFORMAT_COMPOSITE)
  {
    target.Reset();
    return BuildCompositeOffscreenBuffer(ftFace, paint.u.composite, bounds, ctx, depth + 1u);
  }

  if(paint.format == FT_COLR_PAINTFORMAT_SWEEP_GRADIENT)
  {
    target.Reset();
    return target;
  }

  PaintContext offscreenCtx = BuildOffscreenPaintContext(ftFace, ctx, bounds);
  target.scale = offscreenCtx.scale;
  target.offsetX = offscreenCtx.offsetX;
  target.offsetY = offscreenCtx.offsetY;

  if(IsStandaloneFillTreeForOffscreen(ftFace, opaquePaint, depth))
  {
    RenderStandaloneFillToOffscreen(opaquePaint, target, offscreenCtx, depth);
    return target;
  }

  PaintBuildResult buildResult = BuildPaintNodeForSceneBuilder(ftFace, opaquePaint, offscreenCtx, depth + 1u);
  if(!buildResult.IsSuccess())
  {
    ReleasePaintIfNotNull(buildResult.paint);
    target.Reset();
    return target;
  }

  DrawBuiltPaintToOffscreen(target, buildResult);
  return target;
}

OffscreenBuffer CompositeOffscreenBuffers(
  FT_Composite_Mode mode,
  const OffscreenBuffer& backdrop,
  const OffscreenBuffer& source,
  uint32_t debugGlyph)
{
  OffscreenBuffer output;
  output.bounds = backdrop.bounds;
  output.scale = backdrop.scale;
  output.offsetX = backdrop.offsetX;
  output.offsetY = backdrop.offsetY;

  if(!backdrop.IsValid() || !source.IsValid())
  {
    return output;
  }

  CompositeBuffer combined;
  if(!CompositeBuffers(mode, backdrop.surface, source.surface, combined, debugGlyph))
  {
    return output;
  }

  output.surface = combined;
  // RenderPaintToOffscreen() normalizes operands to straight BGRA, and
  // CompositeBuffers() writes straight BGRA output. Picture wrapping therefore
  // uses ARGB8888S, not ARGB8888.
  output.alpha = OffscreenAlpha::STRAIGHT;
  return output;
}

OffscreenBuffer BuildCompositeOffscreenBuffer(
  FT_Face ftFace,
  const FT_PaintComposite& composite,
  const OffscreenBounds& bounds,
  const PaintContext& ctx,
  uint32_t depth)
{
  OffscreenBuffer result;
  result.bounds = bounds;

  if(depth > 64u)
  {
    return result;
  }

  if(!IsCompositeModeSupported(composite.composite_mode))
  {
    return result;
  }

  if(!bounds.valid)
  {
    return result;
  }

  if(!composite.backdrop_paint.p || !composite.source_paint.p)
  {
    return result;
  }

  OffscreenBuffer backdrop = RenderPaintToOffscreen(ftFace, composite.backdrop_paint, bounds, ctx, depth + 1u);
  if(!backdrop.IsValid())
  {
    return result;
  }

  OffscreenBuffer source = RenderPaintToOffscreen(ftFace, composite.source_paint, bounds, ctx, depth + 1u);
  if(!source.IsValid())
  {
    return result;
  }

  return CompositeOffscreenBuffers(composite.composite_mode, backdrop, source, ctx.debugGlyph);
}

CompositeBuffer TakeOffscreenSurface(OffscreenBuffer& buffer)
{
  CompositeBuffer surface = buffer.surface;
  buffer.surface = CompositeBuffer();
  return surface;
}

tvg::Picture* WrapOffscreenPicture(const OffscreenBuffer& buffer)
{
  if(!buffer.IsValid() ||
     !buffer.bounds.valid ||
     buffer.alpha != OffscreenAlpha::STRAIGHT ||
     buffer.surface.width == 0u ||
     buffer.surface.height == 0u ||
     buffer.surface.width != buffer.bounds.pixelWidth ||
     buffer.surface.height != buffer.bounds.pixelHeight ||
     buffer.surface.stride != buffer.surface.width * static_cast<uint32_t>(sizeof(uint32_t)))
  {
    return nullptr;
  }

  tvg::Picture* picture = tvg::Picture::gen();
  if(!picture)
  {
    return nullptr;
  }

  const uint32_t* pixels = reinterpret_cast<const uint32_t*>(buffer.surface.buffer);
  if(picture->load(pixels,
                   buffer.surface.width,
                   buffer.surface.height,
                   tvg::ColorSpace::ARGB8888S,
                   true) != tvg::Result::Success)
  {
    tvg::Paint::rel(picture);
    return nullptr;
  }

  // Source/backdrop operands were rasterized in local offscreen coordinates by
  // subtracting pixelX/Y from ctx.offsetX/Y. Put the resulting picture back at
  // that guarded pixel-space origin before handing it to a parent scene.
  if(picture->translate(static_cast<float>(buffer.bounds.pixelX),
                        static_cast<float>(buffer.bounds.pixelY)) != tvg::Result::Success)
  {
    tvg::Paint::rel(picture);
    return nullptr;
  }

  return picture;
}

/**
 * @brief Rasterizes and composites PaintComposite operands into a bounded buffer.
 *
 * The source/backdrop/output buffers are real, local BGRA8888 allocations. This
 * helper returns Success only when the current builder route explicitly allows
 * bounded offscreen Composite results.
 */
PaintBuildResult BuildCompositeOffscreen(
  FT_Face ftFace,
  const FT_PaintComposite& composite,
  PaintContext& ctx,
  uint32_t depth)
{
  if(!IsCompositeModeSupported(composite.composite_mode))
  {
    return PaintBuildResult::RequiresOffscreenResult();
  }

  const OffscreenBounds bounds = BuildCompositeOffscreenBounds(ftFace, composite, ctx, depth);
  OffscreenBuffer output = BuildCompositeOffscreenBuffer(ftFace, composite, bounds, ctx, depth);
  if(!output.IsValid())
  {
    return PaintBuildResult::RequiresOffscreenResult();
  }

  tvg::Picture* picture = WrapOffscreenPicture(output);
  if(!picture)
  {
    return PaintBuildResult::RequiresOffscreenResult();
  }

  if(ctx.allowOffscreenCompositeResult)
  {
    return PaintBuildResult::Success(picture);
  }

  tvg::Paint::rel(picture);
  return PaintBuildResult::RequiresOffscreenResult();
}

/**
 * @brief Releases a caller-owned ThorVG paint and clears the pointer.
 *
 * Use this only for paints that have not been transferred to a ThorVG
 * Scene/Canvas. Calling this on a paint already owned by a Scene/Canvas would
 * violate ThorVG ownership rules.
 */
void ReleasePaintIfNotNull(tvg::Paint*& paint)
{
  if(paint)
  {
    tvg::Paint::rel(paint);
    paint = nullptr;
  }
}

/**
 * @brief Releases a caller-owned ThorVG Scene and clears the pointer.
 *
 * Use this only before the Scene has been transferred to another ThorVG parent.
 * Releasing the Scene also releases any child paints already transferred into it.
 */
void ReleaseSceneIfNotNull(tvg::Scene*& scene)
{
  if(scene)
  {
    tvg::Paint* scenePaint = scene;
    ReleasePaintIfNotNull(scenePaint);
    scene = nullptr;
  }
}

enum class PaintGlyphChildKind
{
  SIMPLE_FILL,
  BUILD_AND_CLIP,
  BOUNDARY
};

/**
 * @brief Classification result for PaintGlyph child handling in the Scene builder.
 *
 * A simple fill can safely use the current Shape fill fast path. A boundary
 * result must be propagated by the PaintGlyph builder without calling
 * ApplyPaintToShape(), because formats that require whole-result clipping or
 * exact unsupported handling must not be forced through the shape-fill path.
 */
struct GlyphChildInfo
{
  static GlyphChildInfo SimpleFill(FT_PaintFormat format)
  {
    GlyphChildInfo result;
    result.kind = PaintGlyphChildKind::SIMPLE_FILL;
    result.format = format;
    return result;
  }

  static GlyphChildInfo BuildAndClip(FT_PaintFormat format)
  {
    GlyphChildInfo result;
    result.kind = PaintGlyphChildKind::BUILD_AND_CLIP;
    result.format = format;
    return result;
  }

  static GlyphChildInfo Boundary(PaintBuildResult buildResult, FT_PaintFormat format)
  {
    GlyphChildInfo result;
    result.kind = PaintGlyphChildKind::BOUNDARY;
    result.boundaryResult = buildResult;
    result.format = format;
    return result;
  }

  bool IsSimpleFill() const
  {
    return kind == PaintGlyphChildKind::SIMPLE_FILL;
  }

  bool ShouldBuildAndClip() const
  {
    return kind == PaintGlyphChildKind::BUILD_AND_CLIP;
  }

  PaintGlyphChildKind kind{PaintGlyphChildKind::BOUNDARY};
  PaintBuildResult boundaryResult{PaintBuildResult::Failed()};
  FT_PaintFormat format{static_cast<FT_PaintFormat>(0)};
};

/**
 * @brief Classifies a PaintGlyph child for the Scene builder.
 *
 * Solid, LinearGradient, RadialGradient, and transform wrappers around those
 * leaves can use the existing Shape fill fast path. ColrLayers, ColrGlyph, and
 * nested Glyph children are build-and-clip candidates. Composite remains an
 * isolated/offscreen boundary when it appears under a glyph outline. Unsupported
 * formats are returned as explicit boundaries instead of being passed to the
 * shape-fill path.
 */
GlyphChildInfo ClassifyGlyphChild(
  FT_Face ftFace,
  FT_OpaquePaint childPaint,
  uint32_t depth)
{
  if(depth > 64)
  {
    return GlyphChildInfo::Boundary(
      PaintBuildResult::Failed(),
      static_cast<FT_PaintFormat>(0));
  }

  if(!ftFace || !childPaint.p)
  {
    return GlyphChildInfo::Boundary(
      PaintBuildResult::Failed(),
      static_cast<FT_PaintFormat>(0));
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, childPaint, &paint))
  {
    return GlyphChildInfo::Boundary(
      PaintBuildResult::Failed(),
      static_cast<FT_PaintFormat>(0));
  }

  switch(paint.format)
  {
    case FT_COLR_PAINTFORMAT_SOLID:
    case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
    case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
    {
      return GlyphChildInfo::SimpleFill(paint.format);
    }

    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      return ClassifyGlyphChild(ftFace, paint.u.transform.paint, depth + 1);
    }

    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      return ClassifyGlyphChild(ftFace, paint.u.translate.paint, depth + 1);
    }

    case FT_COLR_PAINTFORMAT_SCALE:
    {
      return ClassifyGlyphChild(ftFace, paint.u.scale.paint, depth + 1);
    }

    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      return ClassifyGlyphChild(ftFace, paint.u.rotate.paint, depth + 1);
    }

    case FT_COLR_PAINTFORMAT_SKEW:
    {
      return ClassifyGlyphChild(ftFace, paint.u.skew.paint, depth + 1);
    }

    case FT_COLR_PAINTFORMAT_COMPOSITE:
    {
      return GlyphChildInfo::Boundary(
        PaintBuildResult::RequiresOffscreenResult(),
        paint.format);
    }

    case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
    {
      return GlyphChildInfo::Boundary(
        PaintBuildResult::Unsupported(paint.format),
        paint.format);
    }

    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    case FT_COLR_PAINTFORMAT_GLYPH:
    {
      return GlyphChildInfo::BuildAndClip(paint.format);
    }

    default:
    {
      return GlyphChildInfo::Boundary(
        PaintBuildResult::Unsupported(paint.format),
        paint.format);
    }
  }
}

/**
 * @brief Builds the glyph outline shape used by the PaintGlyph Scene builder.
 *
 * The returned shape is caller-owned and has no placement transform applied yet.
 * The caller can use it either as the final simple-fill shape or as a clipper for
 * a complex child result.
 */
PaintBuildResult BuildGlyphOutlineShape(
  FT_PaintGlyph& paintGlyph,
  PaintContext& ctx)
{
  FT_Error error = FT_Load_Glyph(ctx.ftFace, paintGlyph.glyphID,
                                 FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_SCALE);
  if(error != FT_Err_Ok)
  {
    return PaintBuildResult::Failed();
  }

  FT_Outline* outline = &ctx.ftFace->glyph->outline;
  if(outline->n_contours <= 0 || outline->n_points <= 0)
  {
    return PaintBuildResult::Failed();
  }

  tvg::Shape* shape = tvg::Shape::gen();
  if(!shape)
  {
    return PaintBuildResult::Failed();
  }

  if(!FtOutlineToTvgShape(outline, shape))
  {
    tvg::Paint* shapePaint = shape;
    ReleasePaintIfNotNull(shapePaint);
    return PaintBuildResult::Failed();
  }

  shape->fillRule(tvg::FillRule::NonZero);
  return PaintBuildResult::Success(shape);
}

// ---- Glyph outline placement and whole-result clip wrapper helpers ----

void ApplyGlyphPlacement(tvg::Shape* shape, const PaintContext& ctx)
{
  if(ctx.geometryTransform.hasTransform)
  {
    tvg::Matrix geometryMatrix = BuildGeometryMatrix(ctx.geometryTransform, ctx.scale, ctx.offsetX, ctx.offsetY);
    shape->transform(geometryMatrix);
  }
  else
  {
    shape->scale(ctx.scale);
    shape->translate(ctx.offsetX, ctx.offsetY);
  }
}

void ReleaseClipperReference(tvg::Shape*& clipper)
{
  tvg::Paint* clipperPaint = clipper;
  ReleasePaintIfNotNull(clipperPaint);
  clipper = nullptr;
}

/**
 * @brief Wraps a paint in a Scene so an additional outer clip can be kept.
 *
 * ThorVG Paint::clip() stores only one clipper on the target paint. If a nested
 * subtree already produced a clipped result, applying another clip directly
 * would replace that inner clipper. Wrapping the paint in a local Scene lets
 * the outer clip apply to the group while preserving the inner clip.
 */
PaintBuildResult WrapPaintForOuterClip(
  PaintBuildResult& childResult)
{
  tvg::Scene* scene = tvg::Scene::gen();
  if(!scene)
  {
    ReleasePaintIfNotNull(childResult.paint);
    return PaintBuildResult::Failed();
  }

  if(scene->add(childResult.paint) != tvg::Result::Success)
  {
    ReleasePaintIfNotNull(childResult.paint);
    ReleaseSceneIfNotNull(scene);
    return PaintBuildResult::Failed();
  }

  childResult.paint = nullptr;
  return PaintBuildResult::Success(scene);
}

// ---- ClipBox status, coordinate conversion, and rectangular clip helpers ----

float Clip26Dot6ToFloat(FT_Pos value)
{
  return static_cast<float>(value) / 64.0f;
}

using ClipBoxStatus = ColorGlyphColrRasterizer::ClipBoxCacheStatus;

const char* ClipBoxStatusToString(ClipBoxStatus status)
{
  switch(status)
  {
    case ClipBoxStatus::OK:
      return "ok";
    case ClipBoxStatus::UNKNOWN:
      return "unknown";
    case ClipBoxStatus::INVALID_INPUT:
      return "invalid-input";
    case ClipBoxStatus::MISSING:
      return "missing";
    case ClipBoxStatus::INVALID_BOUNDS:
      return "invalid";
  }

  return "invalid";
}

ColorGlyphColrRasterizer::RawClipBox LookupGlyphClipBox(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  const PaintContext& ctx)
{
  ColorGlyphColrRasterizer::RawClipBox rawClipBox;
  if(ctx.rasterizer)
  {
    ctx.rasterizer->GetClipBox(ftFace, glyphIndex, ctx.variationsHash, rawClipBox);
    return rawClipBox;
  }

  // Product root/offscreen paths provide the rasterizer so ClipBox lookup goes
  // through the shared cache. Without it, fail closed instead of bypassing the
  // cache with a direct FreeType lookup from paint-graph helpers.
  rawClipBox.status = ClipBoxStatus::INVALID_INPUT;
  return rawClipBox;
}

struct GlyphClipBoxCanvasBounds
{
  ClipBoxStatus status{ClipBoxStatus::INVALID_BOUNDS};
  float clipMinX{0.0f};
  float clipMinY{0.0f};
  float clipMaxX{0.0f};
  float clipMaxY{0.0f};
  float canvasMinX{0.0f};
  float canvasMinY{0.0f};
  float canvasMaxX{0.0f};
  float canvasMaxY{0.0f};

  bool IsValid() const
  {
    return status == ClipBoxStatus::OK;
  }
};

tvg::Shape* BuildClipBoxRectClipper(float minX, float minY, float maxX, float maxY)
{
  // COLRv1 ClipBox is rectangular. PaintGlyph whole-result clipping uses the
  // glyph outline shape and must not be routed through this rectangle helper.
  tvg::Shape* clipper = tvg::Shape::gen();
  if(!clipper)
  {
    return nullptr;
  }

  clipper->moveTo(minX, minY);
  clipper->lineTo(maxX, minY);
  clipper->lineTo(maxX, maxY);
  clipper->lineTo(minX, maxY);
  clipper->close();
  clipper->fillRule(tvg::FillRule::NonZero);
  return clipper;
}

GlyphClipBoxCanvasBounds BuildGlyphClipBoxCanvasBounds(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  const PaintContext& ctx)
{
  GlyphClipBoxCanvasBounds result;

  if(!ftFace || ctx.scale <= 0.0f)
  {
    result.status = ClipBoxStatus::INVALID_INPUT;
    return result;
  }

  const ColorGlyphColrRasterizer::RawClipBox rawClipBox = LookupGlyphClipBox(ftFace, glyphIndex, ctx);
  result.status = rawClipBox.status;
  if(!rawClipBox.IsValid())
  {
    return result;
  }

  result.clipMinX = Clip26Dot6ToFloat(rawClipBox.minX);
  result.clipMinY = Clip26Dot6ToFloat(rawClipBox.minY);
  result.clipMaxX = Clip26Dot6ToFloat(rawClipBox.maxX);
  result.clipMaxY = Clip26Dot6ToFloat(rawClipBox.maxY);

  const float clipWidth = result.clipMaxX - result.clipMinX;
  const float clipHeight = result.clipMaxY - result.clipMinY;

  constexpr float MAX_ROOT_CLIPBOX_PIXELS = 1019.0f;
  if(!std::isfinite(clipWidth) || !std::isfinite(clipHeight) ||
     clipWidth <= 0.0f || clipHeight <= 0.0f ||
     clipWidth > MAX_ROOT_CLIPBOX_PIXELS || clipHeight > MAX_ROOT_CLIPBOX_PIXELS)
  {
    result.status = ClipBoxStatus::INVALID_BOUNDS;
    return result;
  }

  result.canvasMinX = ctx.offsetX + result.clipMinX;
  result.canvasMaxX = ctx.offsetX + result.clipMaxX;
  result.canvasMinY = ctx.offsetY - result.clipMaxY;
  result.canvasMaxY = ctx.offsetY - result.clipMinY;
  result.status = ClipBoxStatus::OK;
  return result;
}

bool TryBuildClipBoxPaintBounds(FT_Face ftFace, const PaintContext& ctx, ColrPaintBounds& bounds)
{
  bounds = ColrPaintBounds();
  if(!ctx.useClipBoxBounds || ctx.scale <= 0.0f)
  {
    return false;
  }

  const GlyphClipBoxCanvasBounds clip = BuildGlyphClipBoxCanvasBounds(ftFace, ctx.clipBoxBoundsGlyph, ctx);
  if(!clip.IsValid())
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER ClipBoxBounds glyph:%u boundsGlyph:%u source:fallback reason:%s\n",
               ctx.debugGlyph,
               ctx.clipBoxBoundsGlyph,
               ClipBoxStatusToString(clip.status));
    return false;
  }

  const float invScale = 1.0f / ctx.scale;
  bounds.valid = true;
  bounds.minX = clip.clipMinX * invScale;
  bounds.minY = clip.clipMinY * invScale;
  bounds.maxX = clip.clipMaxX * invScale;
  bounds.maxY = clip.clipMaxY * invScale;

  COLR_TRACE(ctx.debugGlyph,
             "COLOR_GLYPH_COLR_RENDER ClipBoxBounds glyph:%u boundsGlyph:%u source:clipbox bounds:(%.1f,%.1f)-(%.1f,%.1f)\n",
             ctx.debugGlyph,
             ctx.clipBoxBoundsGlyph,
             bounds.minX,
             bounds.minY,
             bounds.maxX,
             bounds.maxY);
  return true;
}

// ---- Root ClipBox vector clip and referenced PaintColrGlyph ClipBox helpers ----

tvg::Shape* BuildRootClipBoxClipper(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  const PaintContext& ctx)
{
  const GlyphClipBoxCanvasBounds clip = BuildGlyphClipBoxCanvasBounds(ftFace, glyphIndex, ctx);
  if(!clip.IsValid())
  {
    if(clip.status == ClipBoxStatus::INVALID_BOUNDS)
    {
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER RootClipBoxClip glyph:%u applied:0 reason:%s pixelAxis:(%.2f,%.2f)-(%.2f,%.2f) size:(%.2f,%.2f)\n",
                 glyphIndex,
                 ClipBoxStatusToString(clip.status),
                 clip.clipMinX,
                 clip.clipMinY,
                 clip.clipMaxX,
                 clip.clipMaxY,
                 clip.clipMaxX - clip.clipMinX,
                 clip.clipMaxY - clip.clipMinY);
    }
    else
    {
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER RootClipBoxClip glyph:%u applied:0 reason:%s\n",
                 glyphIndex,
                 ClipBoxStatusToString(clip.status));
    }
    return nullptr;
  }

  tvg::Shape* clipper = BuildClipBoxRectClipper(clip.canvasMinX, clip.canvasMinY, clip.canvasMaxX, clip.canvasMaxY);
  if(!clipper)
  {
    return nullptr;
  }

  COLR_TRACE(ctx.debugGlyph,
             "COLOR_GLYPH_COLR_RENDER RootClipBoxClip glyph:%u applied:1 canvas:(%.2f,%.2f)-(%.2f,%.2f) source:face-scaled-26.6\n",
             glyphIndex, clip.canvasMinX, clip.canvasMinY, clip.canvasMaxX, clip.canvasMaxY);

  return clipper;
}

tvg::Shape* BuildGlyphClipBoxClipper(
  FT_Face ftFace,
  GlyphIndex glyphIndex,
  const PaintContext& ctx)
{
  const GlyphClipBoxCanvasBounds clip = BuildGlyphClipBoxCanvasBounds(ftFace, glyphIndex, ctx);
  if(!clip.IsValid())
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER ColrGlyphClipBox glyph:%u refGlyph:%u applied:0 reason:%s\n",
               ctx.debugGlyph,
               glyphIndex,
               ClipBoxStatusToString(clip.status));
    return nullptr;
  }

  if(ctx.scale <= 0.0f)
  {
    return nullptr;
  }

  const float invScale = 1.0f / ctx.scale;
  const float minX = clip.clipMinX * invScale;
  const float minY = clip.clipMinY * invScale;
  const float maxX = clip.clipMaxX * invScale;
  const float maxY = clip.clipMaxY * invScale;

  tvg::Shape* clipper = BuildClipBoxRectClipper(minX, -maxY, maxX, -minY);
  if(!clipper)
  {
    return nullptr;
  }
  ApplyGlyphPlacement(clipper, ctx);

  COLR_TRACE(ctx.debugGlyph,
             "COLOR_GLYPH_COLR_RENDER ColrGlyphClipBox glyph:%u refGlyph:%u applied:1 clipPixels:(%.2f,%.2f)-(%.2f,%.2f) canvas:(%.2f,%.2f)-(%.2f,%.2f)\n",
             ctx.debugGlyph,
             glyphIndex,
             clip.clipMinX,
             clip.clipMinY,
             clip.clipMaxX,
             clip.clipMaxY,
             clip.canvasMinX,
             clip.canvasMinY,
             clip.canvasMaxX,
             clip.canvasMaxY);

  return clipper;
}

void ApplyPaintColrGlyphClipBoxIfAvailable(
  FT_Face ftFace,
  GlyphIndex referencedGlyphID,
  PaintContext& ctx,
  PaintBuildResult& buildResult)
{
  if(!buildResult.IsSuccess() || !buildResult.paint)
  {
    return;
  }

  tvg::Shape* clipper = BuildGlyphClipBoxClipper(ftFace, referencedGlyphID, ctx);
  if(!clipper)
  {
    return;
  }

  if(buildResult.paint->clip() != nullptr)
  {
    PaintBuildResult wrappedResult = WrapPaintForOuterClip(buildResult);
    if(!wrappedResult.IsSuccess())
    {
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER ColrGlyphClipBox glyph:%u refGlyph:%u applied:0 reason:wrapper-failed\n",
                 ctx.debugGlyph,
                 referencedGlyphID);
      ReleaseClipperReference(clipper);
      return;
    }
    buildResult = wrappedResult;
  }

  if(buildResult.paint->clip(clipper) != tvg::Result::Success)
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER ColrGlyphClipBox glyph:%u refGlyph:%u applied:0 reason:clip-failed\n",
               ctx.debugGlyph,
               referencedGlyphID);
    ReleaseClipperReference(clipper);
    return;
  }

  ReleaseClipperReference(clipper);
}

// Root direct paint uses ThorVG's vector clipper above. Root Composite returns a
// raw BGRA buffer, so this root-only path applies the same root ClipBox as a
// hard pixel crop by clearing pixels outside the box to transparent BGRA.
// It intentionally preserves allocation size, buffer origin, and RenderResult
// placement metadata; it is not an antialiased clip path.
void ClearOutsideRootClipBoxInCompositeBuffer(
  FT_Face ftFace,
  const PaintContext& ctx,
  OffscreenBuffer& buffer)
{
  if(!buffer.IsValid() ||
     buffer.surface.stride < buffer.surface.width * static_cast<uint32_t>(sizeof(uint32_t)))
  {
    return;
  }

  const GlyphClipBoxCanvasBounds clip = BuildGlyphClipBoxCanvasBounds(ftFace, ctx.debugGlyph, ctx);
  if(!clip.IsValid())
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER RootCompositeClipBox glyph:%u applied:0 reason:%s\n",
               ctx.debugGlyph,
               ClipBoxStatusToString(clip.status));
    return;
  }

  const int64_t clipMinX = static_cast<int64_t>(std::floor(static_cast<double>(clip.canvasMinX)));
  const int64_t clipMaxX = static_cast<int64_t>(std::ceil(static_cast<double>(clip.canvasMaxX)));
  const int64_t clipMinY = static_cast<int64_t>(std::floor(static_cast<double>(clip.canvasMinY)));
  const int64_t clipMaxY = static_cast<int64_t>(std::ceil(static_cast<double>(clip.canvasMaxY)));

  const int64_t bufferMinX = static_cast<int64_t>(buffer.bounds.pixelX);
  const int64_t bufferMinY = static_cast<int64_t>(buffer.bounds.pixelY);
  const int64_t bufferMaxX = bufferMinX + static_cast<int64_t>(buffer.surface.width);

  const int64_t keepMinX = std::max(bufferMinX, clipMinX) - bufferMinX;
  const int64_t keepMaxX = std::min(bufferMaxX, clipMaxX) - bufferMinX;
  const bool hasHorizontalOverlap = keepMinX < keepMaxX;

  for(uint32_t y = 0; y < buffer.surface.height; ++y)
  {
    uint8_t* const row = buffer.surface.buffer + static_cast<size_t>(y) * buffer.surface.stride;
    const int64_t globalY = bufferMinY + static_cast<int64_t>(y);
    if(globalY < clipMinY || globalY >= clipMaxY || !hasHorizontalOverlap)
    {
      std::memset(row, 0, static_cast<size_t>(buffer.surface.width) * sizeof(uint32_t));
      continue;
    }

    const uint32_t leftPixels = static_cast<uint32_t>(std::max<int64_t>(0, keepMinX));
    const uint32_t rightStart = static_cast<uint32_t>(std::min<int64_t>(buffer.surface.width, keepMaxX));
    if(leftPixels > 0u)
    {
      std::memset(row, 0, static_cast<size_t>(leftPixels) * sizeof(uint32_t));
    }
    if(rightStart < buffer.surface.width)
    {
      std::memset(row + static_cast<size_t>(rightStart) * sizeof(uint32_t),
                  0,
                  static_cast<size_t>(buffer.surface.width - rightStart) * sizeof(uint32_t));
    }
  }

  COLR_TRACE(ctx.debugGlyph,
             "COLOR_GLYPH_COLR_RENDER RootCompositeClipBox glyph:%u applied:1 clipCanvas:(%.2f,%.2f)-(%.2f,%.2f) clipPixels:(%lld,%lld)-(%lld,%lld) buffer:(%d,%d %ux%u)\n",
             ctx.debugGlyph,
             clip.canvasMinX,
             clip.canvasMinY,
             clip.canvasMaxX,
             clip.canvasMaxY,
             static_cast<long long>(clipMinX),
             static_cast<long long>(clipMinY),
             static_cast<long long>(clipMaxX),
             static_cast<long long>(clipMaxY),
             buffer.bounds.pixelX,
             buffer.bounds.pixelY,
             buffer.surface.width,
             buffer.surface.height);
}

bool WrapRootPaintForOuterClip(PaintBuildResult& buildResult)
{
  if(!buildResult.IsSuccess() || !buildResult.paint)
  {
    return false;
  }

  tvg::Scene* scene = tvg::Scene::gen();
  if(!scene)
  {
    return false;
  }

  tvg::Paint* child = buildResult.paint;
  if(scene->add(child) != tvg::Result::Success)
  {
    ReleaseSceneIfNotNull(scene);
    return false;
  }

  buildResult.paint = scene;
  return true;
}

void ApplyRootClipBoxIfAvailable(
  FT_Face ftFace,
  PaintContext& ctx,
  PaintBuildResult& buildResult)
{
  if(!buildResult.IsSuccess() || !buildResult.paint)
  {
    return;
  }

  tvg::Shape* clipper = BuildRootClipBoxClipper(ftFace, ctx.debugGlyph, ctx);
  if(!clipper)
  {
    return;
  }

  if(buildResult.paint->clip() != nullptr && !WrapRootPaintForOuterClip(buildResult))
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER RootClipBoxClip glyph:%u applied:0 reason:wrapper-failed\n",
               ctx.debugGlyph);
    ReleaseClipperReference(clipper);
    return;
  }

  if(buildResult.paint->clip(clipper) != tvg::Result::Success)
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER RootClipBoxClip glyph:%u applied:0 reason:clip-failed\n",
               ctx.debugGlyph);
    ReleaseClipperReference(clipper);
    return;
  }

  ReleaseClipperReference(clipper);
}

/**
 * @brief Builds a complex PaintGlyph child and clips the whole child result.
 *
 * This builder helper first builds the child paint subtree as a single result,
 * then creates the glyph outline as a clipper. ThorVG @c Paint::clip() stores a
 * reference to the clipper; after successful clipping the caller-owned clipper
 * reference is released, while the clipped child paint remains caller-owned and
 * is returned.
 */
PaintBuildResult BuildClippedGlyphChild(
  FT_PaintGlyph& paintGlyph,
  PaintContext& ctx,
  uint32_t depth)
{
  PaintBuildResult childResult = BuildPaintNodeForSceneBuilder(ctx.ftFace, paintGlyph.paint, ctx, depth + 1);
  if(!childResult.IsSuccess())
  {
    return childResult;
  }

  if(childResult.paint->clip() != nullptr)
  {
    childResult = WrapPaintForOuterClip(childResult);
    if(!childResult.IsSuccess())
    {
      return childResult;
    }
  }

  PaintBuildResult outlineResult = BuildGlyphOutlineShape(paintGlyph, ctx);
  if(!outlineResult.IsSuccess())
  {
    ReleasePaintIfNotNull(childResult.paint);
    return outlineResult;
  }

  tvg::Shape* clipper = static_cast<tvg::Shape*>(outlineResult.paint);
  outlineResult.paint = nullptr;
  ApplyGlyphPlacement(clipper, ctx);

  if(childResult.paint->clip(clipper) != tvg::Result::Success)
  {
    ReleasePaintIfNotNull(childResult.paint);
    ReleaseClipperReference(clipper);
    return PaintBuildResult::Failed();
  }

  // ThorVG clip() refs the clipper. Drop the caller-owned reference now; the
  // child paint will release the clipper when the child is destroyed or reset.
  ReleaseClipperReference(clipper);
  return childResult;
}

/**
 * @brief Builds a PaintGlyph as a caller-owned ThorVG Shape for the Scene builder path.
 *
 * This is the active PaintGlyph build path for the Scene/direct-root renderer.
 * On success, the returned @c tvg::Paint* is caller-owned until transferred to
 * a Scene/Canvas or released.
 *
 * Only simple Solid/Linear/Radial child fills, including transform wrappers
 * around those leaves, are passed to @c ApplyPaintToShape(). ColrLayers,
 * ColrGlyph, and nested Glyph child results are built and clipped as a whole.
 * Composite/offscreen and unsupported children still return explicit boundaries
 * and are not passed through the Shape fill path.
 */
PaintBuildResult BuildPaintGlyph(
  FT_PaintGlyph& paintGlyph,
  PaintContext& ctx,
  uint32_t depth)
{
  if(paintGlyph.paint.p != nullptr)
  {
    const GlyphChildInfo childClassification = ClassifyGlyphChild(ctx.ftFace, paintGlyph.paint, depth + 1);
    if(childClassification.ShouldBuildAndClip())
    {
      return BuildClippedGlyphChild(paintGlyph, ctx, depth);
    }

    if(!childClassification.IsSimpleFill())
    {
      return childClassification.boundaryResult;
    }
  }

  PaintBuildResult outlineResult = BuildGlyphOutlineShape(paintGlyph, ctx);
  if(!outlineResult.IsSuccess())
  {
    return outlineResult;
  }

  tvg::Shape* shape = static_cast<tvg::Shape*>(outlineResult.paint);
  outlineResult.paint = nullptr;

  if(paintGlyph.paint.p != nullptr)
  {
    // ApplyPaintToShape() is reused only after classification proves the child
    // tree is a simple fill. Preserve builder side-effect isolation by restoring
    // unsupportedCount after the call.
    const uint32_t previousUnsupportedCount = ctx.unsupportedCount;
    const bool fillApplied = ApplyPaintToShape(paintGlyph.paint, shape, ctx, depth + 1);
    ctx.unsupportedCount = previousUnsupportedCount;
    if(!fillApplied)
    {
      tvg::Paint* shapePaint = shape;
      ReleasePaintIfNotNull(shapePaint);
      return PaintBuildResult::Failed();
    }
  }
  else
  {
    shape->fill(0, 0, 0, 255);
  }

  ApplyGlyphPlacement(shape, ctx);

  return PaintBuildResult::Success(shape);
}

/**
 * @brief Transfers a successful child build result into a ThorVG Scene.
 *
 * On @c Scene::add() success, ThorVG takes ownership of @c childResult.paint and
 * this helper clears the pointer to record that transfer. If adding fails,
 * ownership remains with the caller and the caller must clean up the paint. The
 * child paint must be unparented before this helper is called.
 */
bool AddBuiltPaintToScene(tvg::Scene* scene, PaintBuildResult& childResult)
{
  if(!scene || !childResult.IsSuccess() || !childResult.paint)
  {
    return false;
  }

  if(scene->add(childResult.paint) == tvg::Result::Success)
  {
    childResult.paint = nullptr;
    return true;
  }

  return false;
}

/**
 * @brief Gate for the full opt-in Scene-builder route.
 *
 * The default must remain false. Narrow root candidates use explicit helper
 * gates below; this broad gate is not a public API, build option, environment
 * variable, or runtime setting. A local parity experiment may temporarily flip
 * this to true, but the experiment must be reverted and this must never be
 * submitted enabled.
 */
bool IsSceneBuilderAdapterEnabledForColrV1()
{
  return false;
}

/**
 * @brief Limits which root paint formats can enter the Scene-builder route.
 *
 * This is deliberately narrow. Composite, SweepGradient, standalone leaves,
 * unverified transform wrappers, and unknown formats are excluded so the broad
 * gate cannot silently route unsupported COLRv1 semantics through this path.
 */
bool IsPaintFormatSafeForSceneBuilder(FT_PaintFormat format)
{
  switch(format)
  {
    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    case FT_COLR_PAINTFORMAT_GLYPH:
    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    {
      return true;
    }

    default:
    {
      return false;
    }
  }
}

constexpr uint32_t MAX_FEATURE_SCAN_DEPTH  = 64u;
constexpr uint32_t MAX_FEATURE_SCAN_PAINTS = 512u;

/**
 * @brief Root paint node predicate for the default Scene-builder subset.
 *
 * The default route is limited to root safe subsets, so this helper keeps that
 * root-only policy explicit.
 */
bool IsRootPaintNode(uint32_t depth)
{
  return depth == 0u;
}

/**
 * @brief Feature metadata collected without building ThorVG paints.
 *
 * This scan is a fail-closed precondition for the root PaintColrLayers default
 * route. It must not emit logs, update unsupported counters, or create
 * replacement paints.
 */
struct PaintFeatureFlags
{
  bool hasComposite{false};
  bool hasSweepGradient{false};
  bool hasColrGlyph{false};
  bool hasNestedGlyph{false};
  bool hasGlyphChildColrLayers{false};
  bool hasUnsupported{false};
  bool hasRequiresOffscreen{false};
  bool hasTransform{false};
  bool hasTranslate{false};
  bool hasScale{false};
  bool hasRotate{false};
  bool hasSkew{false};
  bool hasLinearGradient{false};
  bool hasRadialGradient{false};
  bool hasSolid{false};
  bool hasStandaloneLeafPaint{false};
  bool traversalFailed{false};
  bool paintCountExceeded{false};
  uint32_t visitedPaintCount{0u};
  uint32_t maxDepthReached{0u};
};

bool NoteVisitedPaint(
  PaintFeatureFlags& flags,
  uint32_t depth)
{
  if(depth > MAX_FEATURE_SCAN_DEPTH)
  {
    flags.traversalFailed = true;
    flags.maxDepthReached = depth;
    return false;
  }

  if(flags.visitedPaintCount >= MAX_FEATURE_SCAN_PAINTS)
  {
    flags.paintCountExceeded = true;
    flags.traversalFailed   = true;
    return false;
  }

  ++flags.visitedPaintCount;
  if(depth > flags.maxDepthReached)
  {
    flags.maxDepthReached = depth;
  }

  return true;
}

bool ScanPaintFeatures(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  PaintFeatureFlags& flags,
  uint32_t depth,
  bool insideGlyph,
  bool insideOffscreenOperand)
{
  if(!ftFace || !opaquePaint.p)
  {
    flags.traversalFailed = true;
    return false;
  }

  if(!NoteVisitedPaint(flags, depth))
  {
    return false;
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, opaquePaint, &paint))
  {
    flags.traversalFailed = true;
    return false;
  }

  switch(paint.format)
  {
    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    {
      if(insideGlyph)
      {
        flags.hasGlyphChildColrLayers = true;
        return false;
      }

      bool             hasLayer = false;
      FT_LayerIterator layerIter = paint.u.colr_layers.layer_iterator;
      FT_OpaquePaint   layerOpaque;
      while(FT_Get_Paint_Layers(ftFace, &layerIter, &layerOpaque))
      {
        hasLayer = true;
        if(!ScanPaintFeatures(ftFace, layerOpaque, flags, depth + 1, insideGlyph, insideOffscreenOperand))
        {
          return false;
        }
      }

      if(!hasLayer)
      {
        flags.hasUnsupported = true;
        return false;
      }

      return true;
    }

    case FT_COLR_PAINTFORMAT_GLYPH:
    {
      if(insideGlyph)
      {
        flags.hasNestedGlyph = true;
        return false;
      }

      return ScanPaintFeatures(ftFace, paint.u.glyph.paint, flags, depth + 1, true, insideOffscreenOperand);
    }

    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    {
      flags.hasColrGlyph = true;
      return false;
    }

    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      flags.hasTransform = true;
      return ScanPaintFeatures(ftFace, paint.u.transform.paint, flags, depth + 1, insideGlyph, insideOffscreenOperand);
    }

    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      flags.hasTranslate = true;
      return ScanPaintFeatures(ftFace, paint.u.translate.paint, flags, depth + 1, insideGlyph, insideOffscreenOperand);
    }

    case FT_COLR_PAINTFORMAT_SCALE:
    {
      flags.hasScale = true;
      return ScanPaintFeatures(ftFace, paint.u.scale.paint, flags, depth + 1, insideGlyph, insideOffscreenOperand);
    }

    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      flags.hasRotate = true;
      return false;
    }

    case FT_COLR_PAINTFORMAT_SKEW:
    {
      flags.hasSkew = true;
      return false;
    }

    case FT_COLR_PAINTFORMAT_COMPOSITE:
    {
      flags.hasComposite = true;
      if(insideGlyph ||
         insideOffscreenOperand ||
         !paint.u.composite.backdrop_paint.p ||
         !paint.u.composite.source_paint.p ||
         !IsCompositeModeSupported(paint.u.composite.composite_mode))
      {
        flags.hasRequiresOffscreen = true;
        return false;
      }

      if(!ScanPaintFeatures(ftFace, paint.u.composite.backdrop_paint, flags, depth + 1, false, true))
      {
        return false;
      }

      if(!ScanPaintFeatures(ftFace, paint.u.composite.source_paint, flags, depth + 1, false, true))
      {
        return false;
      }

      return true;
    }

    case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
    {
      flags.hasSweepGradient = true;
      flags.hasUnsupported   = true;
      return false;
    }

    case FT_COLR_PAINTFORMAT_SOLID:
    {
      flags.hasSolid = true;
      if(!insideGlyph && !insideOffscreenOperand)
      {
        flags.hasStandaloneLeafPaint = true;
        return false;
      }
      return true;
    }

    case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
    {
      flags.hasLinearGradient = true;
      if(!insideGlyph && !insideOffscreenOperand)
      {
        flags.hasStandaloneLeafPaint = true;
        return false;
      }
      return true;
    }

    case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
    {
      flags.hasRadialGradient = true;
      if(!insideGlyph && !insideOffscreenOperand)
      {
        flags.hasStandaloneLeafPaint = true;
        return false;
      }
      return true;
    }

    default:
    {
      flags.hasUnsupported = true;
      return false;
    }
  }
}

bool CollectPaintFeatures(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  PaintFeatureFlags& flags,
  uint32_t depth)
{
  return ScanPaintFeatures(ftFace, opaquePaint, flags, depth, false, false);
}

bool HasBlockedFeature(const PaintFeatureFlags& flags)
{
  return flags.hasSweepGradient ||
         flags.hasColrGlyph ||
         flags.hasNestedGlyph ||
         flags.hasGlyphChildColrLayers ||
         flags.hasUnsupported ||
         flags.hasRequiresOffscreen ||
         flags.hasRotate ||
         flags.hasSkew ||
         flags.hasStandaloneLeafPaint ||
         flags.traversalFailed ||
         flags.paintCountExceeded;
}

/**
 * @brief Tests whether a root PaintColrLayers graph is a default Scene-builder candidate.
 *
 * It accepts only root PaintColrLayers graphs that are free from known
 * unsupported features. Non-root PaintComposite is accepted only when the
 * bounded offscreen fallback can attempt it. Anything uncertain fails closed
 * without synthesizing replacement paints.
 */
bool IsRootColrLayersCandidate(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  const FT_COLR_Paint& paint,
  uint32_t depth)
{
  if(!IsRootPaintNode(depth) ||
     !ftFace ||
     !opaquePaint.p ||
     paint.format != FT_COLR_PAINTFORMAT_COLR_LAYERS)
  {
    return false;
  }

  PaintFeatureFlags flags;
  if(!CollectPaintFeatures(ftFace, opaquePaint, flags, depth))
  {
    return false;
  }

  return !HasBlockedFeature(flags);
}

/**
 * @brief Tests whether root Glyph or exact transform wrappers can use the default Scene-builder route.
 *
 * This intentionally excludes Composite. Root Composite already has a raw
 * RenderResult path through the shared offscreen core, while non-root Composite
 * success is limited to the root PaintColrLayers route. Rotate/Skew remain
 * excluded by the feature scan until bounds parity is fixture-verified.
 */
bool IsRootShapeCandidate(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  const FT_COLR_Paint& paint,
  uint32_t depth)
{
  if(!IsRootPaintNode(depth) ||
     !ftFace ||
     !opaquePaint.p ||
     (paint.format != FT_COLR_PAINTFORMAT_GLYPH &&
      paint.format != FT_COLR_PAINTFORMAT_TRANSFORM &&
      paint.format != FT_COLR_PAINTFORMAT_TRANSLATE &&
      paint.format != FT_COLR_PAINTFORMAT_SCALE))
  {
    return false;
  }

  PaintFeatureFlags flags;
  if(!CollectPaintFeatures(ftFace, opaquePaint, flags, depth))
  {
    return false;
  }

  return !flags.hasComposite &&
         !HasBlockedFeature(flags);
}

/**
 * @brief Selects the limited active Scene-builder route for this paint.
 *
 * The whole-route experiment gate remains disabled by default. When that gate is
 * false, only root PaintColrLayers plus root Glyph/Transform safe subsets may
 * enter the route. The returned @c allowOffscreenCompositeResult flag is
 * set only for the root PaintColrLayers subset, so non-root Composite success
 * cannot leak into root Glyph/Transform by accident.
 */
bool ShouldUseSceneBuilder(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  const FT_COLR_Paint& paint,
  uint32_t depth,
  bool& allowOffscreenCompositeResult)
{
  allowOffscreenCompositeResult = false;

  if(IsSceneBuilderAdapterEnabledForColrV1())
  {
    return IsPaintFormatSafeForSceneBuilder(paint.format);
  }

  const bool isColrLayersCandidate = IsRootColrLayersCandidate(ftFace, opaquePaint, paint, depth);
  if(isColrLayersCandidate)
  {
    allowOffscreenCompositeResult = true;
    return true;
  }

  return IsRootShapeCandidate(ftFace, opaquePaint, paint, depth);
}

struct OffscreenCompositeResultGuard
{
  OffscreenCompositeResultGuard(PaintContext& context, bool allow)
  : ctx(context),
    previous(ctx.allowOffscreenCompositeResult)
  {
    ctx.allowOffscreenCompositeResult = allow;
  }

  ~OffscreenCompositeResultGuard()
  {
    ctx.allowOffscreenCompositeResult = previous;
  }

  PaintContext& ctx;
  bool previous{false};
};

/**
 * @brief Restores the PaintColrGlyph recursion stack on all builder return paths.
 *
 * The Scene builder uses @c PaintContext::colrGlyphStack as the active
 * PaintColrGlyph cycle guard.
 */
struct ColrGlyphBuildGuard
{
  ColrGlyphBuildGuard(PaintContext& context, GlyphIndex glyphIndex)
  : ctx(context)
  {
    ctx.colrGlyphStack.push_back(glyphIndex);
    pushed = true;
  }

  ~ColrGlyphBuildGuard()
  {
    if(pushed)
    {
      ctx.colrGlyphStack.pop_back();
    }
  }

  ColrGlyphBuildGuard(const ColrGlyphBuildGuard&) = delete;
  ColrGlyphBuildGuard& operator=(const ColrGlyphBuildGuard&) = delete;

  PaintContext& ctx;
  bool pushed{false};
};

/**
 * @brief Builds PaintColrLayers as a caller-owned ThorVG Scene for the Scene builder path.
 *
 * COLRv1 layer iteration order is preserved and each successfully built child
 * paint is added to a @c tvg::Scene, representing the layer group as a single
 * paint result. It mutates only the local Scene it creates. On success, the
 * returned Scene is caller-owned. If any child is unsupported, failed, or
 * requires offscreen handling, this helper releases the local Scene and any
 * untransferred child paint, then propagates that child result without creating
 * a replacement paint.
 */
PaintBuildResult BuildColrLayersScene(
  FT_Face ftFace,
  FT_PaintColrLayers& colrLayers,
  PaintContext& ctx,
  uint32_t depth)
{
  tvg::Scene* scene = tvg::Scene::gen();
  if(!scene)
  {
    return PaintBuildResult::Failed();
  }

  uint32_t layerCount = 0u;
  FT_LayerIterator& layerIter = colrLayers.layer_iterator;
  FT_OpaquePaint layerOpaque;

  while(FT_Get_Paint_Layers(ftFace, &layerIter, &layerOpaque))
  {
    PaintBuildResult childResult = BuildPaintNodeForSceneBuilder(ftFace, layerOpaque, ctx, depth + 1);
    if(!childResult.IsSuccess())
    {
      ReleasePaintIfNotNull(childResult.paint);
      ReleaseSceneIfNotNull(scene);
      return childResult;
    }

    if(!AddBuiltPaintToScene(scene, childResult))
    {
      ReleasePaintIfNotNull(childResult.paint);
      ReleaseSceneIfNotNull(scene);
      return PaintBuildResult::Failed();
    }

    ++layerCount;
  }

  if(layerCount == 0u)
  {
    ReleaseSceneIfNotNull(scene);
    return PaintBuildResult::Failed();
  }

  return PaintBuildResult::Success(scene);
}

/**
 * @brief Builds a PaintColrGlyph component reference for the Scene builder.
 *
 * It obtains the referenced glyph's COLRv1 root paint with
 * @c FT_Get_Color_Glyph_Paint(), then recursively evaluates that root through
 * @c BuildPaintNodeForSceneBuilder(). The returned child result is propagated
 * unchanged: a success remains
 * caller-owned by the original caller, while unsupported, failed, and
 * requires-offscreen states are propagated unchanged.
 *
 * Cycle detection reuses @c PaintContext::colrGlyphStack. A cycle is treated as
 * a failed graph evaluation because the builder cannot produce an exact finite
 * subtree. No fallback paint, log, or unsupported counter update is created here.
 */
PaintBuildResult BuildPaintColrGlyph(
  FT_Face ftFace,
  FT_PaintColrGlyph& colrGlyph,
  PaintContext& ctx,
  uint32_t depth)
{
  if(depth > 64)
  {
    return PaintBuildResult::Failed();
  }

  if(!ftFace)
  {
    return PaintBuildResult::Failed();
  }

  const GlyphIndex referencedGlyphID = colrGlyph.glyphID;
  for(const auto& visited : ctx.colrGlyphStack)
  {
    if(visited == referencedGlyphID)
    {
      return PaintBuildResult::Failed();
    }
  }

  FT_OpaquePaint referencedRootPaint;
  referencedRootPaint.p = nullptr;
  referencedRootPaint.insert_root_transform = false;

  const FT_Bool hasReferencedPaint = FT_Get_Color_Glyph_Paint(
    ftFace,
    referencedGlyphID,
    COLR_ROOT_TRANSFORM_OPTION,
    &referencedRootPaint);

  if(!hasReferencedPaint || referencedRootPaint.p == nullptr)
  {
    return PaintBuildResult::Failed();
  }

  ColrGlyphBuildGuard guard(ctx, referencedGlyphID);

  const bool previousUseClipBoxBounds = ctx.useClipBoxBounds;
  const GlyphIndex previousClipBoxBoundsGlyph = ctx.clipBoxBoundsGlyph;
  ctx.useClipBoxBounds = true;
  ctx.clipBoxBoundsGlyph = referencedGlyphID;

  PaintBuildResult result = BuildPaintNodeForSceneBuilder(ftFace, referencedRootPaint, ctx, depth + 1);
  ApplyPaintColrGlyphClipBoxIfAvailable(ftFace, referencedGlyphID, ctx, result);

  ctx.useClipBoxBounds = previousUseClipBoxBounds;
  ctx.clipBoxBoundsGlyph = previousClipBoxBoundsGlyph;
  return result;
}

/**
 * @brief Builds selected COLRv1 paint nodes into caller-owned ThorVG paints.
 *
 * This internal Scene-builder entry point is used by direct root rasterization
 * and the bounded offscreen Composite path for verified safe subsets. The broad
 * route gate remains false by default. Unsupported features do not create
 * replacement paints here.
 */
PaintBuildResult BuildPaintNodeForSceneBuilder(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  PaintContext& ctx,
  uint32_t depth)
{
  if(depth > 64)
  {
    return PaintBuildResult::Failed();
  }

  if(!ftFace || !opaquePaint.p)
  {
    return PaintBuildResult::Failed();
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, opaquePaint, &paint))
  {
    return PaintBuildResult::Failed();
  }

  switch(paint.format)
  {
    case FT_COLR_PAINTFORMAT_GLYPH:
    {
      return BuildPaintGlyph(paint.u.glyph, ctx, depth);
    }

    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    {
      return BuildColrLayersScene(ftFace, paint.u.colr_layers, ctx, depth);
    }

    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      TransformState prevTransform = ctx.geometryTransform;
      const auto& tf = paint.u.transform;
      const float xx = static_cast<float>(tf.affine.xx) / 65536.0f;
      const float xy = static_cast<float>(tf.affine.xy) / 65536.0f;
      const float yx = static_cast<float>(tf.affine.yx) / 65536.0f;
      const float yy = static_cast<float>(tf.affine.yy) / 65536.0f;
      const float dx = static_cast<float>(tf.affine.dx) / 65536.0f;
      const float dy = static_cast<float>(tf.affine.dy) / 65536.0f;
      ctx.geometryTransform.Apply(xx, xy, yx, yy, dx, dy);
      PaintBuildResult result = BuildPaintNodeForSceneBuilder(ftFace, tf.paint, ctx, depth + 1);
      ctx.geometryTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      TransformState prevTransform = ctx.geometryTransform;
      const float dx = static_cast<float>(paint.u.translate.dx) / 65536.0f;
      const float dy = static_cast<float>(paint.u.translate.dy) / 65536.0f;
      ctx.geometryTransform.Apply(1, 0, 0, 1, dx, dy);
      PaintBuildResult result = BuildPaintNodeForSceneBuilder(ftFace, paint.u.translate.paint, ctx, depth + 1);
      ctx.geometryTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_SCALE:
    {
      TransformState prevTransform = ctx.geometryTransform;
      const float sx = static_cast<float>(paint.u.scale.scale_x) / 65536.0f;
      const float sy = static_cast<float>(paint.u.scale.scale_y) / 65536.0f;
      const float cx = static_cast<float>(paint.u.scale.center_x) / 65536.0f;
      const float cy = static_cast<float>(paint.u.scale.center_y) / 65536.0f;
      ctx.geometryTransform.Apply(1, 0, 0, 1, cx, cy);
      ctx.geometryTransform.Apply(sx, 0, 0, sy, 0, 0);
      ctx.geometryTransform.Apply(1, 0, 0, 1, -cx, -cy);
      PaintBuildResult result = BuildPaintNodeForSceneBuilder(ftFace, paint.u.scale.paint, ctx, depth + 1);
      ctx.geometryTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      TransformState prevTransform = ctx.geometryTransform;
      const float angleRadians = ColrAngleToRadians(paint.u.rotate.angle);
      const float cx = FixedToFloat16Dot16(paint.u.rotate.center_x);
      const float cy = FixedToFloat16Dot16(paint.u.rotate.center_y);
      const float cosA = std::cos(angleRadians);
      const float sinA = std::sin(angleRadians);
      ctx.geometryTransform.Apply(1, 0, 0, 1, cx, cy);
      ctx.geometryTransform.Apply(cosA, -sinA, sinA, cosA, 0, 0);
      ctx.geometryTransform.Apply(1, 0, 0, 1, -cx, -cy);
      PaintBuildResult result = BuildPaintNodeForSceneBuilder(ftFace, paint.u.rotate.paint, ctx, depth + 1);
      ctx.geometryTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_SKEW:
    {
      TransformState prevTransform = ctx.geometryTransform;
      const float skewX = ColrSkewAngleToShear(paint.u.skew.x_skew_angle);
      const float skewY = ColrSkewAngleToShear(paint.u.skew.y_skew_angle);
      const float cx = FixedToFloat16Dot16(paint.u.skew.center_x);
      const float cy = FixedToFloat16Dot16(paint.u.skew.center_y);
      ctx.geometryTransform.Apply(1, 0, 0, 1, cx, cy);
      ctx.geometryTransform.Apply(1, skewX, skewY, 1, 0, 0);
      ctx.geometryTransform.Apply(1, 0, 0, 1, -cx, -cy);
      PaintBuildResult result = BuildPaintNodeForSceneBuilder(ftFace, paint.u.skew.paint, ctx, depth + 1);
      ctx.geometryTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_COMPOSITE:
    {
      return BuildCompositeOffscreen(ftFace, paint.u.composite, ctx, depth);
    }

    case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
    {
      return PaintBuildResult::Unsupported(paint.format);
    }

    case FT_COLR_PAINTFORMAT_SOLID:
    case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
    case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
    {
      return PaintBuildResult::Unsupported(paint.format);
    }

    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    {
      return BuildPaintColrGlyph(ftFace, paint.u.colr_glyph, ctx, depth);
    }

    default:
    {
      return PaintBuildResult::Unsupported(paint.format);
    }
  }
}

} // anonymous namespace

bool TryBuildRootPaintForSceneBuilder(
  FT_Face ftFace,
  FT_OpaquePaint opaquePaint,
  PaintContext& ctx,
  tvg::Paint*& outPaint)
{
  outPaint = nullptr;

  if(!ftFace || !opaquePaint.p)
  {
    return false;
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ftFace, opaquePaint, &paint))
  {
    return false;
  }

  bool allowOffscreenCompositeResult = false;
  if(!ShouldUseSceneBuilder(ftFace, opaquePaint, paint, 0u, allowOffscreenCompositeResult))
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER RootSceneBuilder candidate rejected glyph:%u rootFormat:%d(%s)\n",
               ctx.debugGlyph, static_cast<int>(paint.format), PaintFormatToString(paint.format));
    return false;
  }

  OffscreenCompositeResultGuard offscreenCompositeResultGuard(ctx, allowOffscreenCompositeResult);
  PaintBuildResult buildResult = BuildPaintNodeForSceneBuilder(ftFace, opaquePaint, ctx, 0u);
  if(!buildResult.IsSuccess())
  {
    COLR_TRACE(ctx.debugGlyph,
               "COLOR_GLYPH_COLR_RENDER RootSceneBuilder build failed glyph:%u status:%d unsupportedFormat:%d(%s) unsupportedCompositeMode:%d\n",
               ctx.debugGlyph,
               static_cast<int>(buildResult.status),
               static_cast<int>(buildResult.unsupportedFormat),
               PaintFormatToString(buildResult.unsupportedFormat),
               static_cast<int>(buildResult.unsupportedCompositeMode));
    ReleasePaintIfNotNull(buildResult.paint);
    return false;
  }

  ApplyRootClipBoxIfAvailable(ftFace, ctx, buildResult);

  outPaint = buildResult.paint;
  buildResult.paint = nullptr;
  return true;
}

RootCompositeOffscreenResult BuildCompositeOffscreenRootBuffer(
  FT_Face ftFace,
  const FT_PaintComposite& composite,
  const PaintContext& ctx,
  uint32_t depth)
{
  RootCompositeOffscreenResult result;

  const OffscreenBounds bounds = BuildCompositeOffscreenBounds(ftFace, composite, ctx, depth);
  if(!bounds.valid)
  {
    return result;
  }

  OffscreenBuffer output = BuildCompositeOffscreenBuffer(ftFace, composite, bounds, ctx, depth);
  if(!output.IsValid())
  {
    return result;
  }

  ClearOutsideRootClipBoxInCompositeBuffer(ftFace, ctx, output);

  result.pixelX = output.bounds.pixelX;
  result.pixelY = output.bounds.pixelY;
  result.surface = TakeOffscreenSurface(output);
  return result;
}

namespace
{

// ---- Apply a paint (fill style + wrapper transforms) to a shape ----
// This recursively processes the paint tree for a PaintGlyph's child paint.
// Wrapper transforms (Scale, Translate, Rotate, Skew, etc.) are accumulated
// and the inner leaf paint (Solid, Gradient) is applied to the shape.
// Returns true if an exact simple fill was applied.
bool ApplyPaintToShape(FT_OpaquePaint opaquePaint, tvg::Shape* shape, PaintContext& ctx, uint32_t depth)
{
  if(depth > 64 || !opaquePaint.p)
  {
    return false;
  }

  FT_COLR_Paint paint;
  if(!FT_Get_Paint(ctx.ftFace, opaquePaint, &paint))
  {
    return false;
  }

  COLR_TRACE(ctx.debugGlyph,
             "COLOR_GLYPH_COLR_RENDER ApplyPaintToShape depth:%u format:%d name:%s\n",
             depth, static_cast<int>(paint.format), PaintFormatToString(paint.format));

  switch(paint.format)
  {
    case FT_COLR_PAINTFORMAT_SOLID:
    {
      return HandlePaintSolid(paint.u.solid, shape, ctx);
    }

    case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
    {
      return HandlePaintRadialGradient(paint.u.radial_gradient, shape, ctx);
    }

    case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
    {
      return HandlePaintLinearGradient(paint.u.linear_gradient, shape, ctx);
    }

    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      TransformState prevTransform = ctx.paintTransform;
      const auto& tf = paint.u.transform;
      float xx = static_cast<float>(tf.affine.xx) / 65536.0f;
      float xy = static_cast<float>(tf.affine.xy) / 65536.0f;
      float yx = static_cast<float>(tf.affine.yx) / 65536.0f;
      float yy = static_cast<float>(tf.affine.yy) / 65536.0f;
      float dx = static_cast<float>(tf.affine.dx) / 65536.0f;
      float dy = static_cast<float>(tf.affine.dy) / 65536.0f;
      ctx.paintTransform.Apply(xx, xy, yx, yy, dx, dy);
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER ApplyPaintToShape PaintTransform TransformWrapper [%.2f,%.2f,%.2f,%.2f,%.1f,%.1f] accumulated:[%.2f,%.2f,%.2f,%.2f,%.1f,%.1f]\n",
                 xx, xy, yx, yy, dx, dy,
                 ctx.paintTransform.matrix[0], ctx.paintTransform.matrix[1],
                 ctx.paintTransform.matrix[2], ctx.paintTransform.matrix[3],
                 ctx.paintTransform.matrix[4], ctx.paintTransform.matrix[5]);
      bool result = ApplyPaintToShape(tf.paint, shape, ctx, depth + 1);
      ctx.paintTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      TransformState prevTransform = ctx.paintTransform;
      float dx = static_cast<float>(paint.u.translate.dx) / 65536.0f;
      float dy = static_cast<float>(paint.u.translate.dy) / 65536.0f;
      ctx.paintTransform.Apply(1, 0, 0, 1, dx, dy);
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER ApplyPaintToShape PaintTransform TranslateWrapper dx:%.1f dy:%.1f\n",
                 dx, dy);
      bool result = ApplyPaintToShape(paint.u.translate.paint, shape, ctx, depth + 1);
      ctx.paintTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_SCALE:
    {
      TransformState prevTransform = ctx.paintTransform;
      float sx = static_cast<float>(paint.u.scale.scale_x) / 65536.0f;
      float sy = static_cast<float>(paint.u.scale.scale_y) / 65536.0f;
      float cx = static_cast<float>(paint.u.scale.center_x) / 65536.0f;
      float cy = static_cast<float>(paint.u.scale.center_y) / 65536.0f;
      // TransformState::Apply() right-composes transforms, so center-based
      // wrappers are listed as T(c) * operation * T(-c).
      ctx.paintTransform.Apply(1, 0, 0, 1, cx, cy);
      ctx.paintTransform.Apply(sx, 0, 0, sy, 0, 0);
      ctx.paintTransform.Apply(1, 0, 0, 1, -cx, -cy);
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER ApplyPaintToShape PaintTransform Scale sx:%.2f sy:%.2f center:(%.1f,%.1f) accumulated:[%.2f,%.2f,%.2f,%.2f,%.1f,%.1f]\n",
                 sx, sy, cx, cy,
                 ctx.paintTransform.matrix[0], ctx.paintTransform.matrix[1],
                 ctx.paintTransform.matrix[2], ctx.paintTransform.matrix[3],
                 ctx.paintTransform.matrix[4], ctx.paintTransform.matrix[5]);
      bool result = ApplyPaintToShape(paint.u.scale.paint, shape, ctx, depth + 1);
      ctx.paintTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      TransformState prevTransform = ctx.paintTransform;
      const float angleRadians = ColrAngleToRadians(paint.u.rotate.angle);
      const float cx = FixedToFloat16Dot16(paint.u.rotate.center_x);
      const float cy = FixedToFloat16Dot16(paint.u.rotate.center_y);
      const float cosA = std::cos(angleRadians);
      const float sinA = std::sin(angleRadians);
      ctx.paintTransform.Apply(1, 0, 0, 1, cx, cy);
      ctx.paintTransform.Apply(cosA, -sinA, sinA, cosA, 0, 0);
      ctx.paintTransform.Apply(1, 0, 0, 1, -cx, -cy);
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER ApplyPaintToShape PaintTransform Rotate angle:%.1f center:(%.1f,%.1f)\n",
                 angleRadians, cx, cy);
      bool result = ApplyPaintToShape(paint.u.rotate.paint, shape, ctx, depth + 1);
      ctx.paintTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_SKEW:
    {
      TransformState prevTransform = ctx.paintTransform;
      const float skewX = ColrSkewAngleToShear(paint.u.skew.x_skew_angle);
      const float skewY = ColrSkewAngleToShear(paint.u.skew.y_skew_angle);
      const float cx = FixedToFloat16Dot16(paint.u.skew.center_x);
      const float cy = FixedToFloat16Dot16(paint.u.skew.center_y);
      ctx.paintTransform.Apply(1, 0, 0, 1, cx, cy);
      ctx.paintTransform.Apply(1, skewX, skewY, 1, 0, 0);
      ctx.paintTransform.Apply(1, 0, 0, 1, -cx, -cy);
      COLR_TRACE(ctx.debugGlyph,
                 "COLOR_GLYPH_COLR_RENDER ApplyPaintToShape SkewWrapper sx:%.2f sy:%.2f center:(%.1f,%.1f)\n",
                 skewX, skewY, cx, cy);
      bool result = ApplyPaintToShape(paint.u.skew.paint, shape, ctx, depth + 1);
      ctx.paintTransform = prevTransform;
      return result;
    }

    case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
    {
      COLR_UNSUPPORTED("COLOR_GLYPH_COLR_RENDER ApplyPaintToShape SweepGradient unsupported in shape-fill path\n");
      ctx.unsupportedCount++;
      return false;
    }

    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    case FT_COLR_PAINTFORMAT_GLYPH:
    {
      COLR_UNSUPPORTED("COLOR_GLYPH_COLR_RENDER ApplyPaintToShape whole-result child format:%d name:%s unsupported in shape-fill path\n",
                       static_cast<int>(paint.format), PaintFormatToString(paint.format));
      ctx.unsupportedCount++;
      return false;
    }

    case FT_COLR_PAINTFORMAT_COMPOSITE:
    {
      COLR_UNSUPPORTED("COLOR_GLYPH_COLR_RENDER ApplyPaintToShape PaintGlyph child Composite requires Scene/offscreen path\n");
      ctx.unsupportedCount++;
      return false;
    }

    default:
    {
      COLR_UNSUPPORTED("COLOR_GLYPH_COLR_RENDER ApplyPaintToShape unknown format:%d name:%s unsupported in shape-fill path\n",
                       static_cast<int>(paint.format), PaintFormatToString(paint.format));
      ctx.unsupportedCount++;
      return false;
    }
  }
}

} // anonymous namespace

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_ENABLE_COLR_V1_RENDERER
