#ifndef DALI_TEXT_ABSTRACTION_BITMAP_FONT_H
#define DALI_TEXT_ABSTRACTION_BITMAP_FONT_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>



namespace Dali
{

namespace TextAbstraction
{

/**
 * @brief Struct that stores the needed info to create a bitmap glyph.
 *
 * BitmapGlyph objects need to be added to a BitmapFont.
 */
struct DALI_ADAPTOR_API BitmapGlyph
{
  /**
   * @brief Default constructor.
   *
   * Initialize the members to its defaults.
   */
  BitmapGlyph();

  /**
   * @brief Constructor.
   *
   * Initialize the members with the given values.
   *
   * @param[in] url The url of the bitmap for that glyph.
   * @param[in] utf32 The utf32 codification of the glyph.
   * @param[in] ascender The ascender of the glyph.
   * @param[in] descender The descender of the glyph.
   */
  BitmapGlyph( const std::string& url, GlyphIndex utf32, float ascender, float descender );

  /**
   * @brief Default destructor.
   */
  ~BitmapGlyph();

  std::string url;  ///< The url of the glyph's bitmap.
  GlyphIndex utf32; ///< The id of the glyph encoded in utf32.
  float ascender;   ///< The ascender in pixels. The distance from the base line to the top of the glyph.
  float descender;  ///< The descender in pixels. The distance from the base line to the bottom of the glyph.
};

/**
 * @brief Struct that stores the needed info to create a bitmap font.
 *
 * A bitmap font can be created by calling FontClient::GetFontId( const BitmapFont& ).
 */
struct DALI_ADAPTOR_API BitmapFont
{
  /**
   * @brief Default constructor.
   *
   * Initialize the members to its defaults but the @e underlineThickness which is initilized to 1 pixel.
   */
  BitmapFont();

  /**
   * @brief Default destructor.
   */
  ~BitmapFont();

  std::vector<BitmapGlyph> glyphs; ///< The glyphs of the font.
  std::string name;                ///< The name of the font.
  float ascender;                  ///< The ascender in pixels. Maximum ascender of all the glyphs.
  float descender;                 ///< The descender in pixels. Minimum descender of all the glyphs.
  float underlinePosition;         ///< The position in pixels of the underline from the base line.
  float underlineThickness;        ///< The thickness in pixels of the underline.
  bool isColorFont:1;              ///< Whether the glyphs of this font have their own colors.
};

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TEXT_ABSTRACTION_BITMAP_FONT_H
