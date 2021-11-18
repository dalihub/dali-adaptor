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
 */

#include <dali/integration-api/debug.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-cache-item.h>

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
const float FROM_266        = 1.0f / 64.0f;
const float POINTS_PER_INCH = 72.f;

FontFaceCacheItem::FontFaceCacheItem(FT_Library&        freeTypeLibrary,
                                     FT_Face            ftFace,
                                     const FontPath&    path,
                                     PointSize26Dot6    requestedPointSize,
                                     FaceIndex          face,
                                     const FontMetrics& metrics)
: mFreeTypeLibrary(freeTypeLibrary),
  mFreeTypeFace(ftFace),
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

      metrics.ascender           = metrics.ascender * scaleFactor;
      metrics.descender          = metrics.descender * scaleFactor;
      metrics.height             = metrics.height * scaleFactor;
      metrics.underlinePosition  = metrics.underlinePosition * scaleFactor;
      metrics.underlineThickness = metrics.underlineThickness * scaleFactor;
    }
  }
}

bool FontFaceCacheItem::GetGlyphMetrics(GlyphInfo& glyph, unsigned int dpiVertical, bool horizontal) const
{
  bool success(true);

  FT_Face ftFace = mFreeTypeFace;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if we should be loading a Fixed Size bitmap?
  if(mIsFixedSizeBitmap)
  {
    FT_Select_Size(ftFace, mFixedSizeIndex); ///< @todo: needs to be investigated why it's needed to select the size again.
    int error = FT_Load_Glyph(ftFace, glyph.index, FT_LOAD_COLOR);
    if(FT_Err_Ok == error)
    {
      glyph.width    = mFixedWidthPixels;
      glyph.height   = mFixedHeightPixels;
      glyph.advance  = mFixedWidthPixels;
      glyph.xBearing = 0.0f;
      if(horizontal)
      {
        glyph.yBearing += static_cast<float>(ftFace->glyph->metrics.horiBearingY) * FROM_266;
      }
      else
      {
        glyph.yBearing += static_cast<float>(ftFace->glyph->metrics.vertBearingY) * FROM_266;
      }

      // Adjust the metrics if the fixed-size font should be down-scaled
      const float desiredFixedSize = static_cast<float>(mRequestedPointSize) * FROM_266 / POINTS_PER_INCH * dpiVertical;

      if(desiredFixedSize > 0.f)
      {
        const float scaleFactor = desiredFixedSize / mFixedHeightPixels;

        glyph.width    = glyph.width * scaleFactor;
        glyph.height   = glyph.height * scaleFactor;
        glyph.advance  = glyph.advance * scaleFactor;
        glyph.xBearing = glyph.xBearing * scaleFactor;
        glyph.yBearing = glyph.yBearing * scaleFactor;

        glyph.scaleFactor = scaleFactor;
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
    int error = FT_Load_Glyph(ftFace, glyph.index, FT_LOAD_NO_AUTOHINT);

    // Keep the width of the glyph before doing the software emboldening.
    // It will be used to calculate a scale factor to be applied to the
    // advance as Harfbuzz doesn't apply any SW emboldening to calculate
    // the advance of the glyph.
    const float width = static_cast<float>(ftFace->glyph->metrics.width) * FROM_266;

    if(FT_Err_Ok == error)
    {
      const bool isEmboldeningRequired = glyph.isBoldRequired && !(ftFace->style_flags & FT_STYLE_FLAG_BOLD);
      if(isEmboldeningRequired)
      {
        // Does the software bold.
        FT_GlyphSlot_Embolden(ftFace->glyph);
      }

      glyph.width  = static_cast<float>(ftFace->glyph->metrics.width) * FROM_266;
      glyph.height = static_cast<float>(ftFace->glyph->metrics.height) * FROM_266;
      if(horizontal)
      {
        glyph.xBearing += static_cast<float>(ftFace->glyph->metrics.horiBearingX) * FROM_266;
        glyph.yBearing += static_cast<float>(ftFace->glyph->metrics.horiBearingY) * FROM_266;
      }
      else
      {
        glyph.xBearing += static_cast<float>(ftFace->glyph->metrics.vertBearingX) * FROM_266;
        glyph.yBearing += static_cast<float>(ftFace->glyph->metrics.vertBearingY) * FROM_266;
      }

      if(isEmboldeningRequired && !Dali::EqualsZero(width))
      {
        // If the glyph is emboldened by software, the advance is multiplied by a
        // scale factor to make it slightly bigger.
        glyph.advance *= (glyph.width / width);
      }

      // Use the bounding box of the bitmap to correct the metrics.
      // For some fonts i.e the SNum-3R the metrics need to be corrected,
      // otherwise the glyphs 'dance' up and down depending on the
      // font's point size.

      FT_Glyph ftGlyph;
      error = FT_Get_Glyph(ftFace->glyph, &ftGlyph);

      FT_BBox bbox;
      FT_Glyph_Get_CBox(ftGlyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);

      const float descender = glyph.height - glyph.yBearing;
      glyph.height          = (bbox.yMax - bbox.yMin) * FROM_266;
      glyph.yBearing        = glyph.height - round(descender);

      // Created FT_Glyph object must be released with FT_Done_Glyph
      FT_Done_Glyph(ftGlyph);
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
 * @param[in]  isItalicRequired  Whether the glyph requires italic style.
 * @param[in]  isBoldRequired    Whether the glyph requires bold style.
 * @param[out] data              The bitmap data.
 * @param[in]  outlineWidth      The width of the glyph outline in pixels.
 */
void FontFaceCacheItem::CreateBitmap(
  GlyphIndex glyphIndex, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth, bool isItalicRequired, bool isBoldRequired) const
{
  FT_Face  ftFace = mFreeTypeFace;
  FT_Error error;
  // For the software italics.
  bool isShearRequired = false;

#ifdef FREETYPE_BITMAP_SUPPORT
  // Check to see if this is fixed size bitmap
  if(mIsFixedSizeBitmap)
  {
    error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_COLOR);
  }
  else
#endif
  {
    // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
    // i.e. with the SNum-3R font.
    // @todo: add an option to use the FT_LOAD_DEFAULT if required?
    error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_NO_AUTOHINT);
  }
  if(FT_Err_Ok == error)
  {
    if(isBoldRequired && !(ftFace->style_flags & FT_STYLE_FLAG_BOLD))
    {
      // Does the software bold.
      FT_GlyphSlot_Embolden(ftFace->glyph);
    }

    if(isItalicRequired && !(ftFace->style_flags & FT_STYLE_FLAG_ITALIC))
    {
      // Will do the software italic.
      isShearRequired = true;
    }

    FT_Glyph glyph;
    error = FT_Get_Glyph(ftFace->glyph, &glyph);

    // Convert to bitmap if necessary
    if(FT_Err_Ok == error)
    {
      if(glyph->format != FT_GLYPH_FORMAT_BITMAP)
      {
        int  offsetX = 0, offsetY = 0;
        bool isOutlineGlyph = (glyph->format == FT_GLYPH_FORMAT_OUTLINE && outlineWidth > 0);

        // Create a bitmap for the outline
        if(isOutlineGlyph)
        {
          // Retrieve the horizontal and vertical distance from the current pen position to the
          // left and top border of the glyph bitmap for a normal glyph before applying the outline.
          if(FT_Err_Ok == error)
          {
            FT_Glyph normalGlyph;
            error = FT_Get_Glyph(ftFace->glyph, &normalGlyph);

            error = FT_Glyph_To_Bitmap(&normalGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
            if(FT_Err_Ok == error)
            {
              FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(normalGlyph);

              offsetX = bitmapGlyph->left;
              offsetY = bitmapGlyph->top;
            }

            // Created FT_Glyph object must be released with FT_Done_Glyph
            FT_Done_Glyph(normalGlyph);
          }

          // Now apply the outline

          // Set up a stroker
          FT_Stroker stroker;
          error = FT_Stroker_New(mFreeTypeLibrary, &stroker);

          if(FT_Err_Ok == error)
          {
            FT_Stroker_Set(stroker, outlineWidth * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            error = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

            if(FT_Err_Ok == error)
            {
              FT_Stroker_Done(stroker);
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

        error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        if(FT_Err_Ok == error)
        {
          FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

          if(isOutlineGlyph)
          {
            // Calculate the additional horizontal and vertical offsets needed for the position of the outline glyph
            data.outlineOffsetX = offsetX - bitmapGlyph->left - outlineWidth;
            data.outlineOffsetY = bitmapGlyph->top - offsetY - outlineWidth;
          }

          ConvertBitmap(data, bitmapGlyph->bitmap, isShearRequired);
        }
        else
        {
          DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. FT_Get_Glyph Failed with error: %d\n", error);
        }
      }
      else
      {
        ConvertBitmap(data, ftFace->glyph->bitmap, isShearRequired);
      }

      data.isColorEmoji = mIsFixedSizeBitmap;

      // Created FT_Glyph object must be released with FT_Done_Glyph
      FT_Done_Glyph(glyph);
    }
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
    error = FT_Load_Glyph(mFreeTypeFace, glyphIndex, FT_LOAD_COLOR);
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

} // namespace Dali::TextAbstraction::Internal
