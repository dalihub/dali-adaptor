#ifndef DALI_TEST_ABSTRACTION_INTERNAL_FONT_CLIENT_UTILS_H
#define DALI_TEST_ABSTRACTION_INTERNAL_FONT_CLIENT_UTILS_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>

// EXTERNAL INCLUDES
#include <fontconfig/fontconfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H

namespace Dali::TextAbstraction::Internal
{
void ConvertBitmap(TextAbstraction::FontClient::GlyphBufferData& data,
                   unsigned int                                  srcWidth,
                   unsigned int                                  srcHeight,
                   const unsigned char* const                    srcBuffer,
                   const Pixel::Format                           srcFormat);

void ConvertBitmap(TextAbstraction::FontClient::GlyphBufferData& data,
                   FT_Bitmap&                                    srcBitmap,
                   bool                                          isShearRequired,
                   bool                                          moveBuffer = false);

/**
 * @brief Creates a font family pattern used to match fonts.
 *
 * @note Need to call FcPatternDestroy to free the resources.
 *
 * @param[in] fontDescription The font to cache.
 *
 * @return The pattern.
 */
FcPattern* CreateFontFamilyPattern(const FontDescription& fontDescription);

/**
 * @brief Creates a character set from a given font's @p description.
 *
 * @note Need to call FcCharSetDestroy to free the resources.
 *
 * @param[in] description The font's description.
 *
 * @return A character set.
 */
FcCharSet* CreateCharacterSetFromDescription(const FontDescription& description);

constexpr int ValueToIndex(int value, const int* const table, unsigned int maxIndex)
{
  if(nullptr == table)
  {
    // Return an invalid index if there is no table.
    return -1;
  }

  if(value <= table[0])
  {
    return 0;
  }

  if(value >= table[maxIndex])
  {
    return maxIndex;
  }

  for(unsigned int index = 0u; index < maxIndex; ++index)
  {
    const int          v1        = table[index];
    const unsigned int indexPlus = index + 1u;
    const int          v2        = table[indexPlus];
    if((v1 < value) && (value <= v2))
    {
      const int result = ((v1 > 0) && ((value - v1) < (v2 - value))) ? index : indexPlus;
      return result;
    }
  }
  return 0;
}

/**
 * @brief Returns the FontWidth's enum index for the given width value.
 *
 * @param[in] width The width value.
 *
 * @return The FontWidth's enum index.
 */
const FontWidth::Type IntToWidthType(int width);

/**
 * @brief Returns the FontWeight's enum index for the given weight value.
 *
 * @param[in] weight The weight value.
 *
 * @return The FontWeight's enum index.
 */
const FontWeight::Type IntToWeightType(int weight);

/**
 * @brief Returns the FontSlant's enum index for the given slant value.
 *
 * @param[in] slant The slant value.
 *
 * @return The FontSlant's enum index.
 */
const FontSlant::Type IntToSlantType(int slant);

const FontWidth::Type  DefaultFontWidth();
const FontWeight::Type DefaultFontWeight();
const FontSlant::Type  DefaultFontSlant();

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEST_ABSTRACTION_INTERNAL_FONT_CLIENT_UTILS_H
