#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_RASTERIZER_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_RASTERIZER_H

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

#include <cstddef>
#include <cstdint>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/internal/text/text-abstraction/plugin/lru-cache-container.h>
#include <dali/public-api/images/pixel.h>

#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * @brief Whether the build exposes the ThorVG API required by the COLRv1 renderer.
 */
#if defined(THORVG_SUPPORT) && defined(THORVG_VERSION_1)
#define DALI_HAS_THORVG_COLR_RENDER_API 1
#else
#define DALI_HAS_THORVG_COLR_RENDER_API 0
#endif

/**
 * @brief Whether the build exposes the FreeType COLRv1 API.
 */
#if (FREETYPE_MAJOR > 2) || (FREETYPE_MAJOR == 2 && FREETYPE_MINOR >= 13)
#define DALI_HAS_FREETYPE_COLR_V1_API 1
#else
#define DALI_HAS_FREETYPE_COLR_V1_API 0
#endif

/**
 * @brief Enables COLRv1 rasterization when both required backends are available.
 */
#if DALI_HAS_FREETYPE_COLR_V1_API && DALI_HAS_THORVG_COLR_RENDER_API
#define DALI_ENABLE_COLR_V1_RENDERER 1
#else
#define DALI_ENABLE_COLR_V1_RENDERER 0
#endif

namespace Dali::TextAbstraction::Internal
{

/**
 * @brief Owns COLRv1 glyph rasterization support state.
 *
 * Owns the COLRv1 ClipBox and fallback paint-graph bounds LRU caches and provides the
 * rasterization entry point for COLRv1 color glyphs.
 *
 * @see https://freetype.org/freetype2/docs/reference/ft2-layer_management.html
 * @see https://github.com/googlefonts/colr-gradients-spec
 */
class ColorGlyphColrRasterizer
{
public:
  /**
   * @brief Paint bounds of a COLRv1 glyph in font units (y-up, unscaled).
   *
   * Represents the COLRv1 ClipBox when present, otherwise the conservative union
   * bounding box of all PaintGlyph outlines in the COLRv1 paint graph.
   *
   * This is exposed as a font-unit metric. ClipBox lookup is cached separately
   * from conservative paint-graph fallback bounds. Valid ClipBox-sourced bounds
   * do not access the paint-graph bounds cache.
   * Used by GetPaintBounds() and can be used by GetGlyphMetrics()
   * to provide correct metrics for COLRv1 glyphs.
   */
  struct PaintBounds
  {
    enum class Source : uint8_t
    {
      NONE,
      CLIP_BOX,
      PAINT_GRAPH
    };

    bool  valid{false};   ///< Whether bounds were successfully computed.
    float minX{0.0f};     ///< Minimum X in font units.
    float minY{0.0f};     ///< Minimum Y in font units.
    float maxX{0.0f};     ///< Maximum X in font units.
    float maxY{0.0f};     ///< Maximum Y in font units.
    Source source{Source::NONE}; ///< Source used to produce these bounds.
  };

  enum class ClipBoxCacheStatus : uint8_t
  {
    UNKNOWN,
    OK,
    INVALID_INPUT,
    MISSING,
    INVALID_BOUNDS
  };

  /**
   * @brief Raw COLRv1 ClipBox lookup result in FreeType face-scaled 26.6 coordinates.
   *
   * Converted canvas/pixel coordinates are intentionally not cached because they
   * depend on the current raster context offset and local offscreen origin.
   */
  struct RawClipBox
  {
    ClipBoxCacheStatus status{ClipBoxCacheStatus::UNKNOWN};
    FT_Pos minX{0};
    FT_Pos minY{0};
    FT_Pos maxX{0};
    FT_Pos maxY{0};

    bool IsValid() const
    {
      return status == ClipBoxCacheStatus::OK;
    }
  };

  /**
   * @brief Result of a COLR glyph render attempt.
   *
   * On success, @c buffer is a caller-owned straight BGRA8888 bitmap suitable
   * for the DALi color glyph cache.
   */
  struct RenderResult
  {
    bool        success{false};     ///< Whether rendering succeeded.
    uint8_t*    buffer{nullptr};    ///< BGRA8888 pixel buffer; caller owns it on success.
    uint32_t    width{0};           ///< Buffer width in pixels.
    uint32_t    height{0};          ///< Buffer height in pixels.
    uint32_t    stride{0};          ///< Stride in bytes.
    Pixel::Format format{Pixel::BGRA8888}; ///< Pixel format

    int32_t     horizontalOffset{0}; ///< Horizontal offset from glyph origin to bitmap left edge in pixels.
    int32_t     verticalOffset{0};   ///< Vertical offset from baseline to bitmap top edge in pixels.

    bool        hasPaintBounds{false}; ///< Whether paint bounds were computed.
    float       paintMinX{0.0f};       ///< Paint bounds minimum X in font units.
    float       paintMinY{0.0f};       ///< Paint bounds minimum Y in font units.
    float       paintMaxX{0.0f};       ///< Paint bounds maximum X in font units.
    float       paintMaxY{0.0f};       ///< Paint bounds maximum Y in font units.
  };

  /**
   * @brief Creates a COLRv1 rasterizer.
   *
   * @param[in] maxPaintBoundsCacheSize The maximum number of cached paint bounds entries.
   */
  explicit ColorGlyphColrRasterizer(std::size_t maxPaintBoundsCacheSize = 512u);
  ~ColorGlyphColrRasterizer();

  ColorGlyphColrRasterizer(const ColorGlyphColrRasterizer&) = delete;
  ColorGlyphColrRasterizer& operator=(const ColorGlyphColrRasterizer&) = delete;

  ColorGlyphColrRasterizer(ColorGlyphColrRasterizer&&) = delete;
  ColorGlyphColrRasterizer& operator=(ColorGlyphColrRasterizer&&) = delete;

  /**
   * @brief Get COLRv1 paint bounds for a glyph, using LRU cache.
   *
   * Checks the ClipBox cache first. If a valid ClipBox exists, returns
   * ClipBox-sourced bounds immediately without touching the paint-graph bounds
   * cache. Missing/invalid ClipBox states are cached separately, then the
   * conservative paint graph fallback cache is used.
   *
   * @param[in]  ftFace        The FreeType face handle.
   * @param[in]  glyphIndex    The glyph index.
   * @param[in]  variationsHash The hash of the font variations.
   * @param[out] outBounds     Receives the paint bounds on success.
   * @return true if paint bounds are valid, false otherwise.
   */
  bool GetPaintBounds(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    std::size_t variationsHash,
    PaintBounds& outBounds);

  /**
   * @brief Get a COLRv1 ClipBox for a glyph using the rasterizer LRU cache.
   *
   * The cached value is the raw FreeType face-scaled 26.6 ClipBox. Missing and
   * invalid results are cached as lookup states to avoid repeated table walks.
   *
   * @param[in]  ftFace         The FreeType face handle.
   * @param[in]  glyphIndex     The glyph index.
   * @param[in]  variationsHash The hash of the font variations.
   * @param[out] outClipBox     Receives the raw ClipBox lookup result.
   * @return The lookup status.
   */
  ClipBoxCacheStatus GetClipBox(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    std::size_t variationsHash,
    RawClipBox& outClipBox);

  /**
   * @brief Rasterize a COLRv1 glyph to a BGRA8888 buffer.
   *
   * Queries ClipBox-first paint bounds and performs rasterization
   * with a precomputed paint bounds hint when available, avoiding redundant
   * paint graph traversal for bounds computation.
   *
   * @param[in]  ftFace        The FreeType face handle.
   * @param[in]  glyphIndex    The glyph index to render.
   * @param[in]  variationsHash The hash of the font variations.
   * @param[in]  targetWidth   Target buffer width in pixels.
   * @param[in]  targetHeight  Target buffer height in pixels.
   * @param[in]  paletteIndex  CPAL palette index (typically 0).
   * @return RenderResult with buffer on success (caller takes ownership).
   */
  RenderResult Rasterize(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    std::size_t variationsHash,
    uint32_t targetWidth,
    uint32_t targetHeight,
    uint16_t paletteIndex);

  /**
   * @brief Clear internal caches.
   */
  void ClearCache();

private:
  /**
   * @brief Try to produce font-unit bounds from the ClipBox cache.
   */
  bool TryGetClipBoxPaintBounds(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    std::size_t variationsHash,
    PaintBounds& outBounds);

  /**
   * @brief Compute conservative paint graph bounds for fallback use (no cache).
   */
  bool ComputePaintGraphBounds(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    PaintBounds& outBounds);

  /**
   * @brief Get conservative paint graph bounds through the fallback-only cache.
   */
  bool GetFallbackPaintGraphBounds(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    std::size_t variationsHash,
    PaintBounds& outBounds);

  /**
   * @brief Compute raw COLRv1 ClipBox lookup result without consulting the cache.
   */
  RawClipBox ComputeClipBox(
    FT_Face ftFace,
    GlyphIndex glyphIndex) const;

  /**
   * @brief Internal rasterization implementation.
   *
   * Called by Rasterize() after paint bounds lookup.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] glyphIndex The glyph index to render.
   * @param[in] targetWidth Target buffer width in pixels.
   * @param[in] targetHeight Target buffer height in pixels.
   * @param[in] paletteIndex The CPAL palette index.
   * @param[in] variationsHash The hash of the font variations.
   * @param[in] paintBoundsHint Optional paint bounds in font units.
   * @return RenderResult with a caller-owned buffer on success.
   */
  RenderResult RasterizeInternal(
    FT_Face ftFace,
    GlyphIndex glyphIndex,
    uint32_t targetWidth,
    uint32_t targetHeight,
    uint16_t paletteIndex,
    std::size_t variationsHash,
    const PaintBounds* paintBoundsHint);

  // ---- ClipBox and paint graph bounds cache internal types ----

  static std::size_t CombineHash(std::size_t seed, std::size_t value) noexcept
  {
    return seed ^ (value + 0x9e3779b9u + (seed << 6u) + (seed >> 2u));
  }

  struct ClipBoxCacheKey
  {
    ClipBoxCacheKey()
    : mFreeTypeFace(nullptr),
      mFaceIndex(0),
      mGlyphIndex(0u),
      mVariationsHash(0u),
      mXPpem(0u),
      mYPpem(0u)
    {
    }

    ClipBoxCacheKey(const FT_Face freeTypeFace, const GlyphIndex glyphIndex, const std::size_t variationsHash)
    : mFreeTypeFace(freeTypeFace),
      mFaceIndex(freeTypeFace ? freeTypeFace->face_index : 0),
      mGlyphIndex(glyphIndex),
      mVariationsHash(variationsHash),
      mXPpem((freeTypeFace && freeTypeFace->size) ? freeTypeFace->size->metrics.x_ppem : 0u),
      mYPpem((freeTypeFace && freeTypeFace->size) ? freeTypeFace->size->metrics.y_ppem : 0u)
    {
    }

    FT_Face     mFreeTypeFace;
    FT_Long     mFaceIndex;
    GlyphIndex  mGlyphIndex;
    std::size_t mVariationsHash;
    uint32_t    mXPpem;
    uint32_t    mYPpem;

    bool operator==(ClipBoxCacheKey const& rhs) const noexcept
    {
      return mFreeTypeFace == rhs.mFreeTypeFace &&
             mFaceIndex == rhs.mFaceIndex &&
             mGlyphIndex == rhs.mGlyphIndex &&
             mVariationsHash == rhs.mVariationsHash &&
             mXPpem == rhs.mXPpem &&
             mYPpem == rhs.mYPpem;
    }
  };

  struct ClipBoxCacheKeyHash
  {
    std::size_t operator()(ClipBoxCacheKey const& key) const noexcept
    {
      std::size_t hash = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(key.mFreeTypeFace));
      hash = CombineHash(hash, static_cast<std::size_t>(key.mFaceIndex));
      hash = CombineHash(hash, static_cast<std::size_t>(key.mGlyphIndex));
      hash = CombineHash(hash, key.mVariationsHash);
      hash = CombineHash(hash, static_cast<std::size_t>(key.mXPpem));
      hash = CombineHash(hash, static_cast<std::size_t>(key.mYPpem));
      return hash;
    }
  };

  struct ClipBoxCacheData
  {
    RawClipBox clipBox{};
  };

  using ClipBoxCacheContainer =
    LRUCacheContainer<ClipBoxCacheKey,
                      ClipBoxCacheData,
                      ClipBoxCacheKeyHash>;

  struct PaintBoundsCacheKey
  {
    PaintBoundsCacheKey()
    : mFreeTypeFace(nullptr),
      mFaceIndex(0),
      mGlyphIndex(0u),
      mVariationsHash(0u)
    {
    }

    PaintBoundsCacheKey(const FT_Face freeTypeFace, const GlyphIndex glyphIndex, const std::size_t variationsHash)
    : mFreeTypeFace(freeTypeFace),
      mFaceIndex(freeTypeFace ? freeTypeFace->face_index : 0),
      mGlyphIndex(glyphIndex),
      mVariationsHash(variationsHash)
    {
    }

    FT_Face     mFreeTypeFace;
    FT_Long     mFaceIndex;
    GlyphIndex  mGlyphIndex;
    std::size_t mVariationsHash;

    bool operator==(PaintBoundsCacheKey const& rhs) const noexcept
    {
      return mFreeTypeFace == rhs.mFreeTypeFace &&
             mFaceIndex == rhs.mFaceIndex &&
             mGlyphIndex == rhs.mGlyphIndex &&
             mVariationsHash == rhs.mVariationsHash;
    }
  };

  struct PaintBoundsCacheKeyHash
  {
    std::size_t operator()(PaintBoundsCacheKey const& key) const noexcept
    {
      std::size_t hash = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(key.mFreeTypeFace));
      hash = CombineHash(hash, static_cast<std::size_t>(key.mFaceIndex));
      hash = CombineHash(hash, static_cast<std::size_t>(key.mGlyphIndex));
      hash = CombineHash(hash, key.mVariationsHash);
      return hash;
    }
  };

  struct PaintBoundsCacheData
  {
    bool       valid{false};
    PaintBounds bounds{};
  };

  using PaintBoundsCacheContainer =
    LRUCacheContainer<PaintBoundsCacheKey,
                      PaintBoundsCacheData,
                      PaintBoundsCacheKeyHash>;

  ClipBoxCacheContainer     mClipBoxCache;
  PaintBoundsCacheContainer mPaintBoundsCache;
#if DALI_ENABLE_COLR_V1_RENDERER
  bool mThorvgInitialized : 1;
#endif
};

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_RASTERIZER_H
