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
 */

// EXTERNAL HEADERS
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-cache-item.h>

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
 * @brief Maximum size of glyph cache per each font face.
 */
constexpr std::size_t DEFAULT_GLYPH_CACHE_MAX         = 128;
constexpr std::size_t MINIMUM_SIZE_OF_GLYPH_CACHE_MAX = 3u;

constexpr auto MAX_NUMBER_OF_GLYPH_CACHE_ENV = "DALI_GLYPH_CACHE_MAX";

/**
 * @brief Get maximum size of glyph cache size from environment.
 * If not settuped, default as 128.
 * @note This value fixed when we call it first time.
 * @return The max size of glyph cache.
 */
inline const size_t GetMaxNumberOfGlyphCache()
{
  using Dali::EnvironmentVariable::GetEnvironmentVariable;
  static auto numberString = GetEnvironmentVariable(MAX_NUMBER_OF_GLYPH_CACHE_ENV);
  static auto number       = numberString ? std::strtoul(numberString, nullptr, 10) : DEFAULT_GLYPH_CACHE_MAX;
  return (number < MINIMUM_SIZE_OF_GLYPH_CACHE_MAX) ? MINIMUM_SIZE_OF_GLYPH_CACHE_MAX : number;
}

/**
 * @brief Behavior about cache the rendered glyph cache.
 */
constexpr bool DEFAULT_ENABLE_CACHE_RENDERED_GLYPH = true;
constexpr auto ENABLE_CACHE_RENDERED_GLYPH_ENV     = "DALI_ENABLE_CACHE_RENDERED_GLYPH";

/**
 * @brief Get whether we allow to cache rendered glyph from environment.
 * If not settuped, default as true.
 * @note This value fixed when we call it first time.
 * @return True if we allow to cache rendered glyph.
 */
inline const bool EnableCacheRenderedGlyph()
{
  using Dali::EnvironmentVariable::GetEnvironmentVariable;
  static auto numberString = GetEnvironmentVariable(ENABLE_CACHE_RENDERED_GLYPH_ENV);
  static auto number       = numberString ? (std::strtoul(numberString, nullptr, 10) ? true : false) : DEFAULT_ENABLE_CACHE_RENDERED_GLYPH;
  return number;
}

/**
 * @brief Policy about compress the cached rendered glyph.
 * It will be used only if CacheRenderedGlyph is enabled
 */
constexpr auto DEFAULT_RENDERED_GLYPH_COMPRESS_POLICY =
#if !(defined(DALI_PROFILE_UBUNTU) || defined(ANDROID) || defined(WIN32) || defined(__APPLE__))
  TextAbstraction::FontClient::GlyphBufferData::CompressPolicyType::MEMORY; // If tizen target
#else
  TextAbstraction::FontClient::GlyphBufferData::CompressPolicyType::SPEED; // If not tizen target
#endif
constexpr auto RENDERED_GLYPH_COMPRESS_POLICY_ENV = "DALI_RENDERED_GLYPH_COMPRESS_POLICY";

/**
 * @brief Get whether we allow to cache rendered glyph from environment.
 * If not settuped, default value used, as defined above.
 * @note This value fixed when we call it first time.
 * @return SPEED if value start with 's' or 'S'. MEMORY if value start with 'm' or 'M'. otherwise, use default
 */
inline const TextAbstraction::FontClient::GlyphBufferData::CompressPolicyType GetRenderedGlyphCompressPolicy()
{
  using Dali::EnvironmentVariable::GetEnvironmentVariable;
  static auto policyString = GetEnvironmentVariable(RENDERED_GLYPH_COMPRESS_POLICY_ENV);

  static auto policy = policyString ? policyString[0] == 's' || policyString[0] == 'S' ? TextAbstraction::FontClient::GlyphBufferData::CompressPolicyType::SPEED
                                                                                       : policyString[0] == 'm' || policyString[0] == 'M' ? TextAbstraction::FontClient::GlyphBufferData::CompressPolicyType::MEMORY
                                                                                                                                          : DEFAULT_RENDERED_GLYPH_COMPRESS_POLICY
                                    : DEFAULT_RENDERED_GLYPH_COMPRESS_POLICY;
  return policy;
}
} // namespace

FontFaceCacheItem::FontFaceCacheItem(FT_Library&        freeTypeLibrary,
                                     FT_Face            ftFace,
                                     const FontPath&    path,
                                     PointSize26Dot6    requestedPointSize,
                                     FaceIndex          face,
                                     const FontMetrics& metrics)
: mFreeTypeLibrary(freeTypeLibrary),
  mFreeTypeFace(ftFace),
  mGlyphCacheManager(new GlyphCacheManager(mFreeTypeFace, GetMaxNumberOfGlyphCache())),
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
  mHasColorTables(false)
{
}

FontFaceCacheItem::FontFaceCacheItem(FT_Library&        freeTypeLibrary,
                                     FT_Face            ftFace,
                                     const FontPath&    path,
                                     PointSize26Dot6    requestedPointSize,
                                     FaceIndex          face,
                                     const FontMetrics& metrics,
                                     int                fixedSizeIndex,
                                     float              fixedWidth,
                                     float              fixedHeight,
                                     bool               hasColorTables)
: mFreeTypeLibrary(freeTypeLibrary),
  mFreeTypeFace(ftFace),
  mGlyphCacheManager(new GlyphCacheManager(mFreeTypeFace, GetMaxNumberOfGlyphCache())),
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
  mHasColorTables(hasColorTables)
{
}

// Move constructor. font client plugin container may call this.
// Note that we make nullptr of some reference sensitive values here.
FontFaceCacheItem::FontFaceCacheItem(FontFaceCacheItem&& rhs)
: mFreeTypeLibrary(rhs.mFreeTypeLibrary)
{
  mFreeTypeFace       = rhs.mFreeTypeFace;
  mGlyphCacheManager  = std::move(rhs.mGlyphCacheManager);
  mHarfBuzzProxyFont  = std::move(rhs.mHarfBuzzProxyFont);
  mPath               = std::move(rhs.mPath);
  mRequestedPointSize = rhs.mRequestedPointSize;
  mFaceIndex          = rhs.mFaceIndex;
  mMetrics            = rhs.mMetrics;
  mCharacterSet       = rhs.mCharacterSet;
  mFixedSizeIndex     = rhs.mFixedSizeIndex;
  mFixedWidthPixels   = rhs.mFixedWidthPixels;
  mFixedHeightPixels  = rhs.mFixedWidthPixels;
  mVectorFontId       = rhs.mVectorFontId;
  mFontId             = rhs.mFontId;
  mIsFixedSizeBitmap  = rhs.mIsFixedSizeBitmap;
  mHasColorTables     = rhs.mHasColorTables;

  rhs.mFreeTypeFace = nullptr;
}

FontFaceCacheItem::~FontFaceCacheItem()
{
  // delete glyph cache manager before free face.
  if(mGlyphCacheManager)
  {
    mGlyphCacheManager.reset();
  }

  if(mHarfBuzzProxyFont)
  {
    mHarfBuzzProxyFont.reset();
  }

  // Free face.
  if(mFreeTypeFace)
  {
    FT_Done_Face(mFreeTypeFace);
  }
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

  GlyphCacheManager::GlyphCacheData glyphData;
  FT_Error                          error;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if we should be loading a Fixed Size bitmap?
  if(mIsFixedSizeBitmap)
  {
    FT_Select_Size(mFreeTypeFace, mFixedSizeIndex); ///< @todo: needs to be investigated why it's needed to select the size again.
    mGlyphCacheManager->GetGlyphCacheDataFromIndex(glyphInfo.index, FT_LOAD_COLOR, glyphInfo.isBoldRequired, glyphData, error);

    if(FT_Err_Ok == error)
    {
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
          mGlyphCacheManager->ResizeBitmapGlyph(glyphInfo.index, FT_LOAD_COLOR, glyphInfo.isBoldRequired, static_cast<uint32_t>(glyphInfo.width), static_cast<uint32_t>(glyphInfo.height));
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
    // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
    // i.e. with the SNum-3R font.
    // @todo: add an option to use the FT_LOAD_DEFAULT if required?
    mGlyphCacheManager->GetGlyphCacheDataFromIndex(glyphInfo.index, FT_LOAD_NO_AUTOHINT, glyphInfo.isBoldRequired, glyphData, error);

    // Keep the width of the glyph before doing the software emboldening.
    // It will be used to calculate a scale factor to be applied to the
    // advance as Harfbuzz doesn't apply any SW emboldening to calculate
    // the advance of the glyph.

    if(FT_Err_Ok == error)
    {
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

      const bool isEmboldeningRequired = glyphInfo.isBoldRequired && !(glyphData.mStyleFlags & FT_STYLE_FLAG_BOLD);
      if(isEmboldeningRequired)
      {
        // Get dummy glyph data without embolden.
        GlyphCacheManager::GlyphCacheData dummyData;
        if(mGlyphCacheManager->GetGlyphCacheDataFromIndex(glyphInfo.index, FT_LOAD_NO_AUTOHINT, false, dummyData, error))
        {
          // If the glyph is emboldened by software, the advance is multiplied by a
          // scale factor to make it slightly bigger.
          const float width = static_cast<float>(dummyData.mGlyphMetrics.width) * FROM_266;
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
  GlyphIndex glyphIndex, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const
{
  GlyphCacheManager::GlyphCacheData glyphData;
  FT_Error                          error;
  FT_Int32                          loadFlag;
  // For the software italics.
  bool isShearRequired = false;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if this is fixed size bitmap
  if(mIsFixedSizeBitmap)
  {
    loadFlag = FT_LOAD_COLOR;
  }
  else
#endif
  {
    // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
    // i.e. with the SNum-3R font.
    // @todo: add an option to use the FT_LOAD_DEFAULT if required?
    loadFlag = FT_LOAD_NO_AUTOHINT;
  }
  mGlyphCacheManager->GetGlyphCacheDataFromIndex(glyphIndex, loadFlag, isBoldRequired, glyphData, error);

  if(FT_Err_Ok == error)
  {
    if(isItalicRequired && !(glyphData.mStyleFlags & FT_STYLE_FLAG_ITALIC))
    {
      // Will do the software italic.
      isShearRequired = true;
    }

    if(!glyphData.mIsBitmap)
    {
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
        data.buffer        = glyphData.mRenderedBuffer->buffer;
        data.width         = glyphData.mRenderedBuffer->width;
        data.height        = glyphData.mRenderedBuffer->height;
        data.format        = glyphData.mRenderedBuffer->format;
        data.compressType  = glyphData.mRenderedBuffer->compressType;
        data.isBufferOwned = false;
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
            mGlyphCacheManager->CacheRenderedGlyphBuffer(glyphIndex, loadFlag, isBoldRequired, bitmapGlyph->bitmap, GetRenderedGlyphCompressPolicy());

            GlyphCacheManager::GlyphCacheData dummyData;
            mGlyphCacheManager->GetGlyphCacheDataFromIndex(glyphIndex, loadFlag, isBoldRequired, dummyData, error);

            if(DALI_LIKELY(FT_Err_Ok == error && dummyData.mRenderedBuffer))
            {
              data.buffer        = dummyData.mRenderedBuffer->buffer;
              data.width         = dummyData.mRenderedBuffer->width;
              data.height        = dummyData.mRenderedBuffer->height;
              data.format        = dummyData.mRenderedBuffer->format;
              data.compressType  = dummyData.mRenderedBuffer->compressType;
              data.isBufferOwned = false;
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
  FT_Error error = -1;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if this is fixed size bitmap
  if(mHasColorTables)
  {
    GlyphCacheManager::GlyphCacheData dummyData;
    mGlyphCacheManager->GetGlyphCacheDataFromIndex(glyphIndex, FT_LOAD_COLOR, false, dummyData, error);
  }
#endif
  return FT_Err_Ok == error;
}

/**
 * Check if the character is supported by this font
 * @param[in] character The character to test
 */
bool FontFaceCacheItem::IsCharacterSupported(Character character)
{
  if(nullptr == mCharacterSet)
  {
    // Create again the character set.
    // It can be null if the ResetSystemDefaults() method has been called.

    FontDescription description;
    description.path   = mPath;
    description.family = std::move(FontFamily(mFreeTypeFace->family_name));
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

    mCharacterSet = FcCharSetCopy(CreateCharacterSetFromDescription(description));
  }

  return FcCharSetHasChar(mCharacterSet, character);
}

GlyphIndex FontFaceCacheItem::GetGlyphIndex(Character character) const
{
  return FT_Get_Char_Index(mFreeTypeFace, character);
}

GlyphIndex FontFaceCacheItem::GetGlyphIndex(Character character, Character variantSelector) const
{
  return FT_Face_GetCharVariantIndex(mFreeTypeFace, character, variantSelector);
}

HarfBuzzFontHandle FontFaceCacheItem::GetHarfBuzzFont(const uint32_t& horizontalDpi, const uint32_t& verticalDpi)
{
  // Create new harfbuzz font only first time or DPI changed.
  if(DALI_UNLIKELY(!mHarfBuzzProxyFont || mHarfBuzzProxyFont->mHorizontalDpi != horizontalDpi || mHarfBuzzProxyFont->mVerticalDpi != verticalDpi))
  {
    mHarfBuzzProxyFont.reset(new HarfBuzzProxyFont(mFreeTypeFace, mRequestedPointSize, horizontalDpi, verticalDpi, mGlyphCacheManager.get()));
  }
  return mHarfBuzzProxyFont->GetHarfBuzzFont();
}

} // namespace Dali::TextAbstraction::Internal
