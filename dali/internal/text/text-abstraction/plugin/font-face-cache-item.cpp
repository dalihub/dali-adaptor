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

// EXTERNAL HEADERS
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-cache-item.h>
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
namespace
{
const float FROM_266        = 1.0f / 64.0f;
const float POINTS_PER_INCH = 72.f;

/**
 * @brief Maximum rate of bitmap glyph resize.
 * If scale factor is bigger than this value, we will not cache resized glyph.
 * Else, resize bitmap glyph itself and cache it.
 */
constexpr float MAXIMUM_RATE_OF_BITMAP_GLYPH_CACHE_RESIZE = 1.5f;

/**
 * @brief Behavior about cache the rendered glyph cache.
 */
constexpr bool DEFAULT_ENABLE_CACHE_RENDERED_GLYPH = true;

/**
 * @brief Get whether we allow to cache rendered glyph from environment.
 * If not settuped, default as true.
 * @note This value fixed when we call it first time.
 * @return True if we allow to cache rendered glyph.
 */
inline bool EnableCacheRenderedGlyph()
{
  static auto numberString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ENABLE_CACHE_RENDERED_GLYPH);
  static auto number       = numberString ? (std::strtoul(numberString, nullptr, 10) ? true : false) : DEFAULT_ENABLE_CACHE_RENDERED_GLYPH;
  return number;
}

/**
 * @brief Policy about compress the cached rendered glyph.
 * It will be used only if CacheRenderedGlyph is enabled
 */
constexpr auto DEFAULT_RENDERED_GLYPH_COMPRESS_POLICY =
#if !(defined(DALI_PROFILE_UBUNTU) || defined(ANDROID) || defined(WIN32) || defined(__APPLE__))
  GlyphCacheManager::CompressionPolicyType::MEMORY; // If tizen target
#else
  GlyphCacheManager::CompressionPolicyType::SPEED; // If not tizen target
#endif

/**
 * @brief Get whether we allow to cache rendered glyph from environment.
 * If not settuped, default value used, as defined above.
 * @note This value fixed when we call it first time.
 * @return SPEED if value start with 's' or 'S'. MEMORY if value start with 'm' or 'M'. otherwise, use default
 */
inline GlyphCacheManager::CompressionPolicyType GetRenderedGlyphCompressPolicy()
{
  static auto policyString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_RENDERED_GLYPH_COMPRESS_POLICY);

  static auto policy = policyString ? policyString[0] == 's' || policyString[0] == 'S'   ? GlyphCacheManager::CompressionPolicyType::SPEED
                                      : policyString[0] == 'm' || policyString[0] == 'M' ? GlyphCacheManager::CompressionPolicyType::MEMORY
                                                                                         : DEFAULT_RENDERED_GLYPH_COMPRESS_POLICY
                                    : DEFAULT_RENDERED_GLYPH_COMPRESS_POLICY;
  return policy;
}

#if DALI_ENABLE_COLR_V1_RENDERER
void SetGlyphBufferDataFromRenderedCache(const TextAbstraction::GlyphBufferData& cachedBuffer,
                                         TextAbstraction::GlyphBufferData&       data,
                                         bool                                    isColorEmoji)
{
  data.buffer          = cachedBuffer.buffer;
  data.width           = cachedBuffer.width;
  data.height          = cachedBuffer.height;
  data.format          = cachedBuffer.format;
  data.compressionType = cachedBuffer.compressionType;
  data.isBufferOwned   = false;
  data.isColorEmoji    = isColorEmoji;
  data.isColorBitmap   = false;
}
#endif // DALI_ENABLE_COLR_V1_RENDERER
} // namespace

FontFaceCacheItem::FontFaceCacheItem(const FT_Library&                  freeTypeLibrary,
                                     FT_Face                            ftFace,
                                     FontFaceManager*                   fontFaceManager,
                                     GlyphCacheManager*                 glyphCacheManager,
                                     ColorGlyphColrRasterizer*          colorGlyphColrRasterizer,
                                     const FontPath&                    path,
                                     PointSize26Dot6                    requestedPointSize,
                                     FaceIndex                          face,
                                     const FontMetrics&                 metrics,
                                     const std::size_t                  variationsHash,
                                     const std::vector<FT_Fixed>&       freeTypeCoords,
                                     const std::vector<hb_variation_t>& harfBuzzVariations,
                                     bool                               hasColorTables,
                                     const FontFaceManager::ColorFontInfo& colorFontInfo,
                                     FontFaceManager::ColorFontRenderability colorFontRenderability)
: mFreeTypeLibrary(freeTypeLibrary),
  mFreeTypeFace(ftFace),
  mFontFaceManager(fontFaceManager),
  mGlyphCacheManager(glyphCacheManager),
  mColorGlyphColrRasterizer(colorGlyphColrRasterizer),
  mHarfBuzzProxyFont(),
  mPath(path),
  mRequestedPointSize(requestedPointSize),
  mFaceIndex(face),
  mMetrics(metrics),
  mCharacterSet(nullptr),
  mFixedSizeIndex(0),
  mFixedWidthPixels(0.f),
  mFixedHeightPixels(0.f),
  mVectorFontId(0u),
  mFontId(0u),
  mIsFixedSizeBitmap(false),
  mHasColorTables(hasColorTables),
  mColorFontInfo(colorFontInfo),
  mColorFontRenderability(colorFontRenderability),
  mVariationsHash(variationsHash),
  mFreeTypeCoords(freeTypeCoords),
  mHarfBuzzVariations(harfBuzzVariations)
{
}

FontFaceCacheItem::FontFaceCacheItem(const FT_Library&  freeTypeLibrary,
                                     FT_Face            ftFace,
                                     FontFaceManager*   fontFaceManager,
                                     GlyphCacheManager* glyphCacheManager,
                                     ColorGlyphColrRasterizer* colorGlyphColrRasterizer,
                                     const FontPath&    path,
                                     PointSize26Dot6    requestedPointSize,
                                     FaceIndex          face,
                                     const FontMetrics& metrics,
                                     int                fixedSizeIndex,
                                     float              fixedWidth,
                                     float              fixedHeight,
                                     bool               hasColorTables,
                                     const FontFaceManager::ColorFontInfo& colorFontInfo,
                                     FontFaceManager::ColorFontRenderability colorFontRenderability)
: mFreeTypeLibrary(freeTypeLibrary),
  mFreeTypeFace(ftFace),
  mFontFaceManager(fontFaceManager),
  mGlyphCacheManager(glyphCacheManager),
  mColorGlyphColrRasterizer(colorGlyphColrRasterizer),
  mHarfBuzzProxyFont(),
  mPath(path),
  mRequestedPointSize(requestedPointSize),
  mFaceIndex(face),
  mMetrics(metrics),
  mCharacterSet(nullptr),
  mFixedSizeIndex(fixedSizeIndex),
  mFixedWidthPixels(fixedWidth),
  mFixedHeightPixels(fixedHeight),
  mVectorFontId(0u),
  mFontId(0u),
  mIsFixedSizeBitmap(true),
  mHasColorTables(hasColorTables),
  mColorFontInfo(colorFontInfo),
  mColorFontRenderability(colorFontRenderability),
  mVariationsHash(0u),
  mFreeTypeCoords(),
  mHarfBuzzVariations()
{
}

// Move constructor. font client plugin container may call this.
// Note that we make nullptr of some reference sensitive values here.
FontFaceCacheItem::FontFaceCacheItem(FontFaceCacheItem&& rhs) noexcept
: mFreeTypeLibrary(rhs.mFreeTypeLibrary)
{
  mFreeTypeFace            = rhs.mFreeTypeFace;
  mFontFaceManager        = rhs.mFontFaceManager;
  mGlyphCacheManager      = rhs.mGlyphCacheManager;
  mColorGlyphColrRasterizer = rhs.mColorGlyphColrRasterizer;
  mHarfBuzzProxyFont      = std::move(rhs.mHarfBuzzProxyFont);
  mPath               = std::move(rhs.mPath);
  mRequestedPointSize = rhs.mRequestedPointSize;
  mFaceIndex          = rhs.mFaceIndex;
  mMetrics            = rhs.mMetrics;
  mCharacterSet       = rhs.mCharacterSet;
  mFixedSizeIndex     = rhs.mFixedSizeIndex;
  mFixedWidthPixels   = rhs.mFixedWidthPixels;
  mFixedHeightPixels  = rhs.mFixedHeightPixels;
  mVectorFontId       = rhs.mVectorFontId;
  mFontId             = rhs.mFontId;
  mIsFixedSizeBitmap  = rhs.mIsFixedSizeBitmap;
  mHasColorTables     = rhs.mHasColorTables;
  mColorFontInfo      = rhs.mColorFontInfo;
  mColorFontRenderability = rhs.mColorFontRenderability;
  mVariationsHash     = rhs.mVariationsHash;
  mFreeTypeCoords     = std::move(rhs.mFreeTypeCoords);
  mHarfBuzzVariations = std::move(rhs.mHarfBuzzVariations);

  rhs.mFreeTypeFace      = nullptr;
  rhs.mFontFaceManager   = nullptr;
  rhs.mGlyphCacheManager = nullptr;
  rhs.mColorGlyphColrRasterizer = nullptr;
}

FontFaceCacheItem::~FontFaceCacheItem()
{
  // delete cached glyph informations before free face.
  if(mGlyphCacheManager)
  {
    mGlyphCacheManager->RemoveGlyphFromFace(mFreeTypeFace);
  }

  // delete harfbuzz proxy font before free face.
  if(mHarfBuzzProxyFont)
  {
    mHarfBuzzProxyFont.reset();
  }

  if(mFontFaceManager)
  {
    mFontFaceManager->ReleaseFace(mPath);
  }

  mFreeTypeCoords.clear();
  mHarfBuzzVariations.clear();
}

void FontFaceCacheItem::GetFontMetrics(FontMetrics& metrics, unsigned int dpiVertical) const
{
  metrics = mMetrics;

  // Adjust the metrics if the fixed-size font should be down-scaled
  if(mIsFixedSizeBitmap)
  {
    const float desiredFixedSize = static_cast<float>(mRequestedPointSize) * FROM_266 / POINTS_PER_INCH * dpiVertical;

    if(desiredFixedSize > 0.f)
    {
      const float scaleFactor = desiredFixedSize / mFixedHeightPixels;

      metrics.ascender           = round(metrics.ascender * scaleFactor);
      metrics.descender          = round(metrics.descender * scaleFactor);
      metrics.height             = round(metrics.height * scaleFactor);
      metrics.underlinePosition  = metrics.underlinePosition * scaleFactor;
      metrics.underlineThickness = metrics.underlineThickness * scaleFactor;
    }
  }
}

bool FontFaceCacheItem::GetGlyphMetrics(GlyphInfo& glyphInfo, unsigned int dpiVertical, bool horizontal) const
{
  bool success(true);

  GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
  FT_Error                             error;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if we should be loading a Fixed Size bitmap?
  if(mIsFixedSizeBitmap)
  {
    error = mFontFaceManager->SelectFixedSize(mFreeTypeFace, mRequestedPointSize, mFixedSizeIndex);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetGlyphMetrics. SelectFixedSize fail\n");
    }

    mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphInfo.index, FT_LOAD_COLOR, glyphInfo.isBoldRequired, mVariationsHash, glyphDataPtr, error);

    if(FT_Err_Ok == error)
    {
      GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();

      glyphInfo.width    = mFixedWidthPixels;
      glyphInfo.height   = mFixedHeightPixels;
      glyphInfo.advance  = mFixedWidthPixels;
      glyphInfo.xBearing = 0.0f;

      const auto& metrics = glyphData.mGlyphMetrics;

      if(horizontal)
      {
        glyphInfo.yBearing += static_cast<float>(metrics.horiBearingY) * FROM_266;
      }
      else
      {
        glyphInfo.yBearing += static_cast<float>(metrics.vertBearingY) * FROM_266;
      }

      // Adjust the metrics if the fixed-size font should be down-scaled
      const float desiredFixedSize = static_cast<float>(mRequestedPointSize) * FROM_266 / POINTS_PER_INCH * dpiVertical;

      if(desiredFixedSize > 0.f)
      {
        const float scaleFactor = desiredFixedSize / mFixedHeightPixels;
        glyphInfo.width         = round(glyphInfo.width * scaleFactor);
        glyphInfo.height        = round(glyphInfo.height * scaleFactor);
        glyphInfo.advance       = round(glyphInfo.advance * scaleFactor);
        glyphInfo.xBearing      = round(glyphInfo.xBearing * scaleFactor);
        glyphInfo.yBearing      = round(glyphInfo.yBearing * scaleFactor);

        glyphInfo.scaleFactor = scaleFactor;

        if(scaleFactor < MAXIMUM_RATE_OF_BITMAP_GLYPH_CACHE_RESIZE)
        {
          // Resize bitmap glyph and cache it due to the performance issue.
          // If scaleFactor is too big, cached bitmap may hold too big memory.
          // So, we only hold small enough case.

          // TODO : If dpiVertical value changed, this resize feature will be break down.
          // Otherwise, this glyph will be resized only one times.
          mGlyphCacheManager->ResizeBitmapGlyph(mFreeTypeFace, mRequestedPointSize, glyphInfo.index, FT_LOAD_COLOR, glyphInfo.isBoldRequired, mVariationsHash, static_cast<uint32_t>(glyphInfo.width), static_cast<uint32_t>(glyphInfo.height));
        }
      }
    }
    else
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetBitmapMetrics. FreeType Bitmap Load_Glyph error %d\n", error);
      success = false;
    }
  }
  else
#endif
  {
    error = mFontFaceManager->ActivateFace(mFreeTypeFace, mRequestedPointSize, mVariationsHash, mFreeTypeCoords);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetGlyphMetrics. ActivateFace fail\n");
    }

    // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
    // i.e. with the SNum-3R font.
    // @todo: add an option to use the FT_LOAD_DEFAULT if required?
    mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphInfo.index, FT_LOAD_NO_AUTOHINT, glyphInfo.isBoldRequired, mVariationsHash, glyphDataPtr, error);

    // Keep the width of the glyph before doing the software emboldening.
    // It will be used to calculate a scale factor to be applied to the
    // advance as Harfbuzz doesn't apply any SW emboldening to calculate
    // the advance of the glyph.

    if(FT_Err_Ok == error)
    {
      GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();

      const auto& metrics = glyphData.mGlyphMetrics;

      glyphInfo.width  = static_cast<float>(metrics.width) * FROM_266;
      glyphInfo.height = static_cast<float>(metrics.height) * FROM_266;
      if(horizontal)
      {
        glyphInfo.xBearing += static_cast<float>(metrics.horiBearingX) * FROM_266;
        glyphInfo.yBearing += static_cast<float>(metrics.horiBearingY) * FROM_266;
      }
      else
      {
        glyphInfo.xBearing += static_cast<float>(metrics.vertBearingX) * FROM_266;
        glyphInfo.yBearing += static_cast<float>(metrics.vertBearingY) * FROM_266;
      }

      if(!glyphInfo.isShaped)
      {
        glyphInfo.advance = static_cast<float>(metrics.horiAdvance) * FROM_266;
      }

      glyphInfo.advance = round(glyphInfo.advance);

      const bool isEmboldeningRequired = glyphInfo.isBoldRequired && !(glyphData.mStyleFlags & FT_STYLE_FLAG_BOLD);
      if(isEmboldeningRequired)
      {
        // Get dummy glyph data without embolden.
        GlyphCacheManager::GlyphCacheDataPtr dummyDataPtr;
        if(mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphInfo.index, FT_LOAD_NO_AUTOHINT, false, mVariationsHash, dummyDataPtr, error))
        {
          // If the glyph is emboldened by software, the advance is multiplied by a
          // scale factor to make it slightly bigger.
          const float width = static_cast<float>(dummyDataPtr->mGlyphMetrics.width) * FROM_266;
          if(!EqualsZero(width))
          {
            glyphInfo.advance *= (glyphInfo.width / width);
          }
        }
      }

      // Use the bounding box of the bitmap to correct the metrics.
      // For some fonts i.e the SNum-3R the metrics need to be corrected,
      // otherwise the glyphs 'dance' up and down depending on the
      // font's point size.
      FT_Glyph glyph = glyphData.mGlyph;

      FT_BBox bbox;
      FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);

      const float descender = glyphInfo.height - glyphInfo.yBearing;
      glyphInfo.height      = (bbox.yMax - bbox.yMin) * FROM_266;
      glyphInfo.yBearing    = glyphInfo.height - round(descender);

      // COLRv1 metric correction: use paint bounds for COLRv1 glyphs
      // COLRv1 glyphs may have zero or inaccurate outline metrics because
      // the actual ink is defined in the paint graph (PaintGlyph nodes),
      // not in the base glyph outline. Use GetGlyphPaintBounds() to get
      // the correct ink bounds and override width/height/xBearing/yBearing.
      // advance is NOT changed - it comes from shaping/FreeType and is correct.
      if(mColorFontRenderability == FontFaceManager::ColorFontRenderability::RenderableColrV1 &&
         IsRenderableColrV1Glyph(glyphInfo.index))
      {
        ColorGlyphColrRasterizer::PaintBounds paintBounds;
        if(mColorGlyphColrRasterizer &&
           mColorGlyphColrRasterizer->GetPaintBounds(mFreeTypeFace, glyphInfo.index, mVariationsHash, paintBounds) &&
           paintBounds.valid &&
           paintBounds.maxX > paintBounds.minX &&
           paintBounds.maxY > paintBounds.minY)
        {
          const float unitsPerEm = static_cast<float>(mFreeTypeFace->units_per_EM);
          if(unitsPerEm > 0.0f && mFreeTypeFace->size)
          {
            // DALi text font size is vertical-size based. Use uniform yScale
            // derived from y_ppem / units_per_EM, matching RenderGlyph().
            const float scale = static_cast<float>(mFreeTypeFace->size->metrics.y_ppem) / unitsPerEm;

            const float paintWidth  = paintBounds.maxX - paintBounds.minX;
            const float paintHeight = paintBounds.maxY - paintBounds.minY;

            // Save advance before overwriting metrics
            const float savedAdvance = glyphInfo.advance;

            glyphInfo.xBearing = std::round(paintBounds.minX * scale);
            glyphInfo.yBearing = std::round(paintBounds.maxY * scale);
            glyphInfo.width    = std::round(paintWidth * scale);
            glyphInfo.height   = std::round(paintHeight * scale);

            // Restore advance - layout advance from shaping must be preserved
            glyphInfo.advance = savedAdvance;
          }
        }
      }
    }
    else
    {
      success = false;
    }
  }
  return success;
}

/**
 * @brief Create a bitmap representation of a glyph from a face font
 *
 * @param[in]  glyphIndex        The index of a glyph within the specified font.
 * @param[out] data              The bitmap data.
 * @param[in]  outlineWidth      The width of the glyph outline in pixels.
 * @param[in]  isItalicRequired  Whether the glyph requires italic style.
 * @param[in]  isBoldRequired    Whether the glyph requires bold style.
 */
void FontFaceCacheItem::CreateBitmap(
  GlyphIndex glyphIndex, Dali::TextAbstraction::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const
{
  GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
  FT_Error                             error;
  FT_Int32                             loadFlag;
  // For the software italics.
  bool isShearRequired = false;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if this is fixed size bitmap
  if(mIsFixedSizeBitmap)
  {
    error = mFontFaceManager->SelectFixedSize(mFreeTypeFace, mRequestedPointSize, mFixedSizeIndex);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. SelectFixedSize fail\n");
    }

    loadFlag = FT_LOAD_COLOR;
  }
  else
#endif
  {
    error = mFontFaceManager->ActivateFace(mFreeTypeFace, mRequestedPointSize, mVariationsHash, mFreeTypeCoords);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. ActivateFace fail\n");
    }

    // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
    // i.e. with the SNum-3R font.
    // @todo: add an option to use the FT_LOAD_DEFAULT if required?
    loadFlag = FT_LOAD_NO_AUTOHINT;
  }
  mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphIndex, loadFlag, isBoldRequired, mVariationsHash, glyphDataPtr, error);

  if(FT_Err_Ok == error)
  {
    GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();
    if(isItalicRequired && !(glyphData.mStyleFlags & FT_STYLE_FLAG_ITALIC))
    {
      // Will do the software italic.
      isShearRequired = true;
    }

    if(!glyphData.mIsBitmap)
    {
#if DALI_ENABLE_COLR_V1_RENDERER
      // COLRv1 color glyph path for scalable fonts.
      // Glyphs without COLRv1 root paint fall through to standard outline rendering.
      const bool isRenderableColrV1Glyph =
        mColorFontRenderability == FontFaceManager::ColorFontRenderability::RenderableColrV1 &&
        IsRenderableColrV1Glyph(glyphIndex);
      if(isRenderableColrV1Glyph)
      {
        const uint32_t yPpem = mFreeTypeFace->size ? mFreeTypeFace->size->metrics.y_ppem : 64u;
        const uint32_t targetSize = (yPpem < 16u) ? 16u : (yPpem > 128u) ? 128u : yPpem;

        if(glyphData.mRenderedBuffer)
        {
          SetGlyphBufferDataFromRenderedCache(*glyphData.mRenderedBuffer, data, true);
          return; // Skip COLRv1 rasterization and normal outline rendering.
        }

        ColorGlyphColrRasterizer::RenderResult renderResult;

        if(mColorGlyphColrRasterizer)
        {
          renderResult = mColorGlyphColrRasterizer->Rasterize(
            mFreeTypeFace,
            glyphIndex,
            mVariationsHash,
            targetSize,
            targetSize,
            0u);
        }
        if(renderResult.success && renderResult.buffer)
        {
          const bool cached = mGlyphCacheManager->CacheExternalGlyphBuffer(
            mFreeTypeFace, mRequestedPointSize, glyphIndex, loadFlag, isBoldRequired, mVariationsHash,
            renderResult.buffer, renderResult.width, renderResult.height, renderResult.stride, renderResult.format,
            false);

          mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace,
                                                         mRequestedPointSize,
                                                         glyphIndex,
                                                         loadFlag,
                                                         isBoldRequired,
                                                         mVariationsHash,
                                                         glyphDataPtr,
                                                         error);
          if(FT_Err_Ok == error && glyphDataPtr->mRenderedBuffer)
          {
            SetGlyphBufferDataFromRenderedCache(*glyphDataPtr->mRenderedBuffer, data, true);
            return; // Skip normal outline rendering.
          }

          if(!cached)
          {
            DALI_LOG_ERROR("COLOR_GLYPH_COLR COLRv1 rasterization cache failed idx:%u\n", glyphIndex);
          }
          else
          {
            DALI_LOG_ERROR("COLOR_GLYPH_COLR COLRv1 rasterization re-fetch failed idx:%u\n", glyphIndex);
          }
        }
        else
        {
          DALI_LOG_ERROR("COLOR_GLYPH_COLR COLRv1 rasterization failed idx:%u\n", glyphIndex);
        }
      }
#endif // DALI_ENABLE_COLR_V1_RENDERER

      // Convert to bitmap if necessary
      FT_Glyph glyph = glyphData.mGlyph;

      DALI_ASSERT_ALWAYS(glyph->format != FT_GLYPH_FORMAT_BITMAP && "Something wrong with cashing. Some bitmap glyph cached failed.");

      int  offsetX = 0, offsetY = 0;
      bool isOutlineGlyph       = (glyph->format == FT_GLYPH_FORMAT_OUTLINE && outlineWidth > 0);
      bool isStrokeGlyphSuccess = false;

      // Create a bitmap for the outline
      if(isOutlineGlyph)
      {
        // Retrieve the horizontal and vertical distance from the current pen position to the
        // left and top border of the glyph bitmap for a normal glyph before applying the outline.
        if(FT_Err_Ok == error)
        {
          // Copy new glyph, and keep original cached glyph.
          error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 0);
          if(FT_Err_Ok == error)
          {
            FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

            offsetX = bitmapGlyph->left;
            offsetY = bitmapGlyph->top;

            // Copied FT_Glyph object must be released with FT_Done_Glyph
            FT_Done_Glyph(glyph);
          }

          // Replace as original glyph
          glyph = glyphData.mGlyph;
        }

        // Now apply the outline

        // Set up a stroker
        FT_Stroker stroker;
        error = FT_Stroker_New(mFreeTypeLibrary, &stroker);

        if(FT_Err_Ok == error)
        {
          // Copy glyph pointer for release memory.
          FT_Stroker_Set(stroker, outlineWidth * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
          error = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 0);

          if(FT_Err_Ok == error)
          {
            FT_Stroker_Done(stroker);
            isStrokeGlyphSuccess = true;
          }
          else
          {
            DALI_LOG_ERROR("FT_Glyph_StrokeBorder Failed with error: %d\n", error);
          }
        }
        else
        {
          DALI_LOG_ERROR("FT_Stroker_New Failed with error: %d\n", error);
        }
      }

      const bool ableUseCachedRenderedGlyph = EnableCacheRenderedGlyph() && !isOutlineGlyph && !isShearRequired;

      // If we cache rendered glyph, and if we can use it, use cached thing first.
      if(ableUseCachedRenderedGlyph && glyphData.mRenderedBuffer)
      {
        data.buffer          = glyphData.mRenderedBuffer->buffer;
        data.width           = glyphData.mRenderedBuffer->width;
        data.height          = glyphData.mRenderedBuffer->height;
        data.format          = glyphData.mRenderedBuffer->format;
        data.compressionType = glyphData.mRenderedBuffer->compressionType;
        data.isBufferOwned   = false;
      }
      else
      {
        // Copy new glyph, and keep original cached glyph.
        // If we already copy new glyph by stroke, just re-use that.
        error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, isStrokeGlyphSuccess);
        if(FT_Err_Ok == error)
        {
          FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

          if(isOutlineGlyph)
          {
            // Calculate the additional horizontal and vertical offsets needed for the position of the outline glyph
            data.outlineOffsetX = offsetX - bitmapGlyph->left - outlineWidth;
            data.outlineOffsetY = bitmapGlyph->top - offsetY - outlineWidth;
          }

          // If we can cache this bitmapGlyph, store it.
          // Note : We will call this API once per each glyph.
          if(ableUseCachedRenderedGlyph)
          {
            mGlyphCacheManager->CacheRenderedGlyphBuffer(mFreeTypeFace, mRequestedPointSize, glyphIndex, loadFlag, isBoldRequired, mVariationsHash, bitmapGlyph->bitmap, GetRenderedGlyphCompressPolicy());

            GlyphCacheManager::GlyphCacheDataPtr dummyDataPtr;
            mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphIndex, loadFlag, isBoldRequired, mVariationsHash, dummyDataPtr, error);

            if(DALI_LIKELY(FT_Err_Ok == error && dummyDataPtr->mRenderedBuffer))
            {
              data.buffer          = dummyDataPtr->mRenderedBuffer->buffer;
              data.width           = dummyDataPtr->mRenderedBuffer->width;
              data.height          = dummyDataPtr->mRenderedBuffer->height;
              data.format          = dummyDataPtr->mRenderedBuffer->format;
              data.compressionType = dummyDataPtr->mRenderedBuffer->compressionType;
              data.isBufferOwned   = false;
            }
            else
            {
              // Something problem during cache or get rendered glyph buffer.
              // Move bitmap buffer into data.buffer
              ConvertBitmap(data, bitmapGlyph->bitmap, isShearRequired, true);
            }
          }
          else
          {
            // Move bitmap buffer into data.buffer
            ConvertBitmap(data, bitmapGlyph->bitmap, isShearRequired, true);
          }

          // Copied FT_Glyph object must be released with FT_Done_Glyph
          FT_Done_Glyph(glyph);
        }
        else
        {
          DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. FT_Get_Glyph Failed with error: %d\n", error);
        }
      }
    }
    else
    {
      ConvertBitmap(data, *glyphData.mBitmap, isShearRequired);
    }

    data.isColorEmoji = mIsFixedSizeBitmap;
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. FT_Load_Glyph Failed with error: %d\n", error);
  }
}

bool FontFaceCacheItem::IsColorGlyph(GlyphIndex glyphIndex) const
{
  // Policy: IsColorGlyph() returns true only when this glyph can actually
  // produce a color bitmap (BGRA/RGBA) in the current DALi build configuration.
  // Having color font tables is NOT sufficient - the renderer must be available.

  // NotRenderableColorFont: has color tables but no renderer → always false
  if(mColorFontRenderability == FontFaceManager::ColorFontRenderability::NotRenderableColorFont)
  {
    return false;
  }

  // NotColorFont: no color tables at all → always false
  if(mColorFontRenderability == FontFaceManager::ColorFontRenderability::NotColorFont)
  {
    return false;
  }

  // RenderableBitmap (CBDT/CBLC): use existing FT_LOAD_COLOR cache lookup
  if(mColorFontRenderability == FontFaceManager::ColorFontRenderability::RenderableBitmap)
  {
    FT_Error error = -1;
#ifdef FREETYPE_BITMAP_SUPPORT
    GlyphCacheManager::GlyphCacheDataPtr dummyDataPtr;
    mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphIndex, FT_LOAD_COLOR, false, mVariationsHash, dummyDataPtr, error);
#endif
    return FT_Err_Ok == error;
  }

  // RenderableColrV1: true only if COLRv1 renderer has already produced a rendered buffer
  // for this glyph (i.e., CreateBitmap() was called first and succeeded).
  // We do NOT trigger COLRv1 rasterization here - only check cache state.
  if(mColorFontRenderability == FontFaceManager::ColorFontRenderability::RenderableColrV1)
  {
    FT_Error error = -1;
    GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
    mGlyphCacheManager->GetGlyphCacheDataFromIndex(mFreeTypeFace, mRequestedPointSize, glyphIndex, FT_LOAD_COLOR, false, mVariationsHash, glyphDataPtr, error);
    const bool hasRenderedBuffer = (FT_Err_Ok == error && glyphDataPtr && glyphDataPtr->mRenderedBuffer != nullptr);
    return hasRenderedBuffer;
  }

  // For COLRv1, IsColorGlyph() intentionally reports only an already-rendered
  // color buffer. Renderable COLRv1 candidate checks must use
  // IsRenderableColrV1Glyph() instead.

  return false;
}

bool FontFaceCacheItem::IsColorFont() const
{
  // Preserve existing semantics: true means the face has color font tables,
  // not necessarily that DALi can render them in this build.
  return mHasColorTables;
}

bool FontFaceCacheItem::IsRenderableColrV1Font() const
{
  // RenderableBitmap (CBDT/CBLC) returns false - it uses FT_LOAD_COLOR, not COLRv1.
  return mColorFontRenderability == FontFaceManager::ColorFontRenderability::RenderableColrV1;
}

bool FontFaceCacheItem::IsRenderableColrV1Glyph(GlyphIndex glyphIndex) const
{
#if DALI_ENABLE_COLR_V1_RENDERER
  if(mColorFontRenderability != FontFaceManager::ColorFontRenderability::RenderableColrV1)
  {
    return false;
  }

  if(!mFreeTypeFace || !mFontFaceManager)
  {
    return false;
  }

  return mFontFaceManager->HasRenderableColrV1GlyphPaint(mFreeTypeFace, glyphIndex);
#else
  return false;
#endif
}

/**
 * Check if the character is supported by this font
 * @param[in] fontConfig A handle to a FontConfig library instance.
 * @param[in] character The character to test
 */
bool FontFaceCacheItem::IsCharacterSupported(FcConfig* fontConfig, Character character)
{
  if(nullptr == mCharacterSet)
  {
    // Create again the character set.
    // It can be null if the ResetSystemDefaults() method has been called.
    FontDescription description;
    description.path   = mPath;
    description.family = FontFamily(mFreeTypeFace->family_name);
    description.weight = FontWeight::NONE;
    description.width  = FontWidth::NONE;
    description.slant  = FontSlant::NONE;

    // Note FreeType doesn't give too much info to build a proper font style.
    if(mFreeTypeFace->style_flags & FT_STYLE_FLAG_ITALIC)
    {
      description.slant = FontSlant::ITALIC;
    }
    if(mFreeTypeFace->style_flags & FT_STYLE_FLAG_BOLD)
    {
      description.weight = FontWeight::BOLD;
    }

    mCharacterSet = FcCharSetCopy(CreateCharacterSetFromDescription(fontConfig, description));
  }

  return FcCharSetHasChar(mCharacterSet, character);
}

GlyphIndex FontFaceCacheItem::GetGlyphIndex(Character character) const
{
  const GlyphIndex glyphIndex = FT_Get_Char_Index(mFreeTypeFace, character);
  return glyphIndex;
}

GlyphIndex FontFaceCacheItem::GetGlyphIndex(Character character, Character variantSelector) const
{
  return FT_Face_GetCharVariantIndex(mFreeTypeFace, character, variantSelector);
}

HarfBuzzFontHandle FontFaceCacheItem::GetHarfBuzzFont(const uint32_t& horizontalDpi, const uint32_t& verticalDpi)
{
  if(mIsFixedSizeBitmap)
  {
    FT_Error error = mFontFaceManager->SelectFixedSize(mFreeTypeFace, mRequestedPointSize, mFixedSizeIndex);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetHarfBuzzFont. SelectFixedSize fail\n");
    }
  }
  else
  {
    FT_Error error = mFontFaceManager->ActivateFace(mFreeTypeFace, mRequestedPointSize, mVariationsHash, mFreeTypeCoords);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetHarfBuzzFont. ActivateFace fail\n");
    }
  }

  // Create new harfbuzz font only first time or DPI changed.
  if(DALI_UNLIKELY(!mHarfBuzzProxyFont || mHarfBuzzProxyFont->mHorizontalDpi != horizontalDpi || mHarfBuzzProxyFont->mVerticalDpi != verticalDpi))
  {
    mHarfBuzzProxyFont.reset(new HarfBuzzProxyFont(mFreeTypeFace, mRequestedPointSize, mVariationsHash, mHarfBuzzVariations, horizontalDpi, verticalDpi, mGlyphCacheManager));
  }

  // Should be called after changing the size or variation-axis settings on the FT_Face. (ActivateFace/SelectFixedSize)
  mHarfBuzzProxyFont->FontChanged();

  return mHarfBuzzProxyFont->GetHarfBuzzFont();
}

} // namespace Dali::TextAbstraction::Internal
