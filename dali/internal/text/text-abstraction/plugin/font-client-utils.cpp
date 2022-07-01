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

#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>

#include <dali/integration-api/debug.h>
#include <dali/internal/imaging/common/image-operations.h>

#include <memory>

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
namespace
{
// http://www.freedesktop.org/software/fontconfig/fontconfig-user.html

// NONE            -1  --> DEFAULT_FONT_WIDTH (NORMAL) will be used.
// ULTRA_CONDENSED 50
// EXTRA_CONDENSED 63
// CONDENSED       75
// SEMI_CONDENSED  87
// NORMAL         100
// SEMI_EXPANDED  113
// EXPANDED       125
// EXTRA_EXPANDED 150
// ULTRA_EXPANDED 200
const int          FONT_WIDTH_TYPE_TO_INT[] = {-1, 50, 63, 75, 87, 100, 113, 125, 150, 200};
const unsigned int NUM_FONT_WIDTH_TYPE      = sizeof(FONT_WIDTH_TYPE_TO_INT) / sizeof(int);

// NONE                       -1  --> DEFAULT_FONT_WEIGHT (NORMAL) will be used.
// THIN                        0
// ULTRA_LIGHT, EXTRA_LIGHT   40
// LIGHT                      50
// DEMI_LIGHT, SEMI_LIGHT     55
// BOOK                       75
// NORMAL, REGULAR            80
// MEDIUM                    100
// DEMI_BOLD, SEMI_BOLD      180
// BOLD                      200
// ULTRA_BOLD, EXTRA_BOLD    205
// BLACK, HEAVY, EXTRA_BLACK 210
const int          FONT_WEIGHT_TYPE_TO_INT[] = {-1, 0, 40, 50, 55, 75, 80, 100, 180, 200, 205, 210};
const unsigned int NUM_FONT_WEIGHT_TYPE      = sizeof(FONT_WEIGHT_TYPE_TO_INT) / sizeof(int);

// NONE             -1 --> DEFAULT_FONT_SLANT (NORMAL) will be used.
// NORMAL, ROMAN     0
// ITALIC          100
// OBLIQUE         110
const int          FONT_SLANT_TYPE_TO_INT[] = {-1, 0, 100, 110};
const unsigned int NUM_FONT_SLANT_TYPE      = sizeof(FONT_SLANT_TYPE_TO_INT) / sizeof(int);

} // namespace

/**
 * @brief Returns the FontWidth's enum index for the given width value.
 *
 * @param[in] width The width value.
 *
 * @return The FontWidth's enum index.
 */
const FontWidth::Type IntToWidthType(int width)
{
  return static_cast<FontWidth::Type>(ValueToIndex(width, FONT_WIDTH_TYPE_TO_INT, NUM_FONT_WIDTH_TYPE - 1u));
}

/**
 * @brief Returns the FontWeight's enum index for the given weight value.
 *
 * @param[in] weight The weight value.
 *
 * @return The FontWeight's enum index.
 */
const FontWeight::Type IntToWeightType(int weight)
{
  return static_cast<FontWeight::Type>(ValueToIndex(weight, FONT_WEIGHT_TYPE_TO_INT, NUM_FONT_WEIGHT_TYPE - 1u));
}

/**
 * @brief Returns the FontSlant's enum index for the given slant value.
 *
 * @param[in] slant The slant value.
 *
 * @return The FontSlant's enum index.
 */
const FontSlant::Type IntToSlantType(int slant)
{
  return static_cast<FontSlant::Type>(ValueToIndex(slant, FONT_SLANT_TYPE_TO_INT, NUM_FONT_SLANT_TYPE - 1u));
}

const int DEFAULT_FONT_WIDTH(100);
const int DEFAULT_FONT_WEIGHT(80);
const int DEFAULT_FONT_SLANT(0);

const FontWidth::Type DefaultFontWidth()
{
  return IntToWidthType(DEFAULT_FONT_WIDTH);
}
const FontWeight::Type DefaultFontWeight()
{
  return IntToWeightType(DEFAULT_FONT_WEIGHT);
}
const FontSlant::Type DefaultFontSlant()
{
  return IntToSlantType(DEFAULT_FONT_SLANT);
}

/**
 * @brief Copy the color bitmap given in @p srcBuffer to @p data.
 *
 * @param[out] data The bitmap data.
 * @param[in] srcWidth The width of the bitmap.
 * @param[in] srcHeight The height of the bitmap.
 * @param[in] srcBuffer The buffer of the bitmap.
 */
void ConvertBitmap(TextAbstraction::FontClient::GlyphBufferData& data, unsigned int srcWidth, unsigned int srcHeight, const unsigned char* const srcBuffer, const Pixel::Format srcFormat)
{
  // Set the input dimensions.
  const ImageDimensions inputDimensions(srcWidth, srcHeight);

  // Set the output dimensions.
  // If the output dimension is not given, the input dimension is set
  // and won't be downscaling.
  data.width  = (data.width == 0) ? srcWidth : data.width;
  data.height = (data.height == 0) ? srcHeight : data.height;
  const ImageDimensions desiredDimensions(data.width, data.height);

  data.format = srcFormat;

  // Note we don't compress here
  data.compressType = TextAbstraction::FontClient::GlyphBufferData::CompressType::NO_COMPRESS;

  const uint32_t bytePerPixel = Dali::Pixel::GetBytesPerPixel(srcFormat);

  // Creates the output buffer
  const uint32_t bufferSize = data.width * data.height * bytePerPixel;

  if(inputDimensions == desiredDimensions)
  {
    // There isn't downscaling.
    data.isBufferOwned = false;
    data.buffer        = const_cast<uint8_t*>(srcBuffer);
  }
  else
  {
    data.isBufferOwned = true;
    data.buffer        = (uint8_t*)malloc(bufferSize); // @note The caller is responsible for deallocating the bitmap data using free.
    Dali::Internal::Platform::LanczosSample(srcBuffer,
                                            inputDimensions,
                                            srcWidth,
                                            srcFormat,
                                            data.buffer,
                                            desiredDimensions);
  }
}

/**
 * @brief Copy the FreeType bitmap to the given buffer.
 *
 * @param[out] data The bitmap data.
 * @param[in,out] srcBitmap The FreeType bitmap.
 * @param[in] isShearRequired Whether the bitmap needs a shear transform (for software italics).
 * @param[in] moveBuffer Whether the bitmap buffer move. True if just copy buffer pointer. False if we use memcpy. (Default is false.)
 * @note If you set moveBuffer=true, the bitmap's buffer moved frome srcBitmap to data. So srcBitmap buffer changed as nullptr.
 */
void ConvertBitmap(TextAbstraction::FontClient::GlyphBufferData& data, FT_Bitmap& srcBitmap, bool isShearRequired, bool moveBuffer)
{
  data.buffer = nullptr;
  if(srcBitmap.width * srcBitmap.rows > 0)
  {
    switch(srcBitmap.pixel_mode)
    {
      case FT_PIXEL_MODE_GRAY:
      {
        if(srcBitmap.pitch == static_cast<int>(srcBitmap.width))
        {
          uint8_t*     pixelsIn = srcBitmap.buffer;
          unsigned int width    = srcBitmap.width;
          unsigned     height   = srcBitmap.rows;

          uint8_t* releaseRequiredPixelPtr = nullptr;

          if(isShearRequired)
          {
            /**
             * Glyphs' bitmaps with no slant retrieved from FreeType:
             * __________     ____
             * |XXXXXXXX|     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * ----------     ----
             *
             * Expected glyphs' bitmaps with italic slant:
             * ____________   ______
             * |  XXXXXXXX|   |  XX|
             * |     XX   |   |  XX|
             * |    XX    |   | XX |
             * |    XX    |   | XX |
             * |   XX     |   |XX  |
             * |   XX     |   |XX  |
             * ------------   ------
             *
             * Glyphs' bitmaps with software italic slant retrieved from FreeType:
             * __________     ______
             * |XXXXXXXX|     |  XX|
             * |   XX   |     |  XX|
             * |  XX    |     | XX |
             * |  XX    |     | XX |
             * | XX     |     |XX  |
             * | XX     |     |XX  |
             * ----------     ------
             *
             * This difference in some bitmaps' width causes an overlap of some glyphs. This is the reason why a shear operation is done here instead of relying on the experimental FT_GlyphSlot_Oblique() implementation.
             */
            unsigned int widthOut  = 0u;
            unsigned int heightOut = 0u;
            uint8_t*     pixelsOut = nullptr;

            Dali::Internal::Platform::HorizontalShear(pixelsIn,
                                                      width,
                                                      height,
                                                      width,
                                                      1u,
                                                      -TextAbstraction::FontClient::DEFAULT_ITALIC_ANGLE,
                                                      pixelsOut,
                                                      widthOut,
                                                      heightOut);

            if(DALI_LIKELY(pixelsOut))
            {
              width  = widthOut;
              height = heightOut;

              if(moveBuffer)
              {
                releaseRequiredPixelPtr = pixelsIn;
              }
              else
              {
                releaseRequiredPixelPtr = pixelsOut;
              }

              // Change input buffer ptr.
              pixelsIn = pixelsOut;
            }
            else
            {
              DALI_LOG_ERROR("ERROR! software italic slant failed!\n");
            }
          }

          data.width  = width;
          data.height = height;
          data.format = Pixel::L8; // Sets the pixel format.

          // Note we don't compress here
          data.compressType = TextAbstraction::FontClient::GlyphBufferData::CompressType::NO_COMPRESS;

          if(moveBuffer)
          {
            data.isBufferOwned = true;
            data.buffer        = pixelsIn;

            // Happy trick for copyless convert bitmap!
            srcBitmap.buffer = nullptr;
          }
          else
          {
            data.isBufferOwned = false;
            data.buffer        = pixelsIn;
          }

          if(releaseRequiredPixelPtr)
          {
            free(releaseRequiredPixelPtr);
          }
        }
        break;
      }

#ifdef FREETYPE_BITMAP_SUPPORT
      case FT_PIXEL_MODE_BGRA:
      {
        if(srcBitmap.pitch == static_cast<int>(srcBitmap.width << 2u))
        {
          // Color glyph doesn't support copyless convert bitmap. Just memcpy
          ConvertBitmap(data, srcBitmap.width, srcBitmap.rows, srcBitmap.buffer, Pixel::BGRA8888);
        }
        break;
      }
#endif
      default:
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::ConvertBitmap. FontClient Unable to create Bitmap of this PixelType\n");
        break;
      }
    }
  }
}

FcPattern* CreateFontFamilyPattern(const FontDescription& fontDescription)
{
  // create the cached font family lookup pattern
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern* fontFamilyPattern = FcPatternCreate(); // FcPatternCreate creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

  if(!fontFamilyPattern)
  {
    return nullptr;
  }

  // add a property to the pattern for the font family
  FcPatternAddString(fontFamilyPattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(fontDescription.family.c_str()));

  // add a property to the pattern for local setting.
  const char* locale = setlocale(LC_MESSAGES, nullptr);
  if(locale != nullptr)
  {
    FcPatternAddString(fontFamilyPattern, FC_LANG, reinterpret_cast<const FcChar8*>(locale));
  }

  int width = FONT_WIDTH_TYPE_TO_INT[fontDescription.width];
  if(width < 0)
  {
    // Use default.
    width = DEFAULT_FONT_WIDTH;
  }

  int weight = FONT_WEIGHT_TYPE_TO_INT[fontDescription.weight];
  if(weight < 0)
  {
    // Use default.
    weight = DEFAULT_FONT_WEIGHT;
  }

  int slant = FONT_SLANT_TYPE_TO_INT[fontDescription.slant];
  if(slant < 0)
  {
    // Use default.
    slant = DEFAULT_FONT_SLANT;
  }

  FcPatternAddInteger(fontFamilyPattern, FC_WIDTH, width);
  FcPatternAddInteger(fontFamilyPattern, FC_WEIGHT, weight);
  FcPatternAddInteger(fontFamilyPattern, FC_SLANT, slant);

  // modify the config, with the mFontFamilyPatterm
  FcConfigSubstitute(nullptr /* use default configure */, fontFamilyPattern, FcMatchPattern);

  // provide default values for unspecified properties in the font pattern
  // e.g. patterns without a specified style or weight are set to Medium
  FcDefaultSubstitute(fontFamilyPattern);

  return fontFamilyPattern;
}

FcCharSet* CreateCharacterSetFromDescription(const FontDescription& description)
{
  FcCharSet* characterSet = nullptr;

  FcPattern* pattern = CreateFontFamilyPattern(description); // Creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

  if(nullptr != pattern)
  {
    FcResult   result = FcResultMatch;
    FcPattern* match  = FcFontMatch(nullptr, pattern, &result); // FcFontMatch creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcPatternGetCharSet(match, FC_CHARSET, 0u, &characterSet);

    // Destroys the created patterns.
    FcPatternDestroy(match);
    FcPatternDestroy(pattern);
  }

  return characterSet;
}

} // namespace Dali::TextAbstraction::Internal
