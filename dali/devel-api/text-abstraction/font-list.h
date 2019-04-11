#ifndef DALI_TEXT_ABSTRACTION_FONT_LIST_H
#define DALI_TEXT_ABSTRACTION_FONT_LIST_H

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
#include <string>
#include <iostream>
#include <dali/public-api/common/vector-wrapper.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace TextAbstraction
{
typedef std::string FontPath;
typedef std::string FontFamily;
typedef std::string FontStyle;

namespace FontWidth
{
  /**
   * @brief Enumeration type for the font's width
   */
  enum Type
  {
    NONE,            ///< Means not defined. Will use what is set as default, currently NORMAL.
    ULTRA_CONDENSED,
    EXTRA_CONDENSED,
    CONDENSED,
    SEMI_CONDENSED,
    NORMAL,
    SEMI_EXPANDED,
    EXPANDED,
    EXTRA_EXPANDED,
    ULTRA_EXPANDED
  };

  const char* const Name[] =
  {
    "NONE",
    "ULTRA_CONDENSED",
    "EXTRA_CONDENSED",
    "CONDENSED",
    "SEMI_CONDENSED",
    "NORMAL",
    "SEMI_EXPANDED",
    "EXPANDED",
    "EXTRA_EXPANDED",
    "ULTRA_EXPANDED"
  };
} // namespace FontWidth

namespace FontWeight
{
  /**
   * @brief Enumeration type for the font's weight
   */
  enum Type
  {
    NONE,                      ///< Means not defined. Will use what is set as default, currently NORMAL.
    THIN,
    ULTRA_LIGHT,
    EXTRA_LIGHT = ULTRA_LIGHT,
    LIGHT,
    DEMI_LIGHT,
    SEMI_LIGHT = DEMI_LIGHT,
    BOOK,
    NORMAL,
    REGULAR = NORMAL,
    MEDIUM,
    DEMI_BOLD,
    SEMI_BOLD = DEMI_BOLD,
    BOLD,
    ULTRA_BOLD,
    EXTRA_BOLD = ULTRA_BOLD,
    BLACK,
    HEAVY = BLACK,
    EXTRA_BLACK = BLACK
  };

  const char* const Name[] =
  {
    "NONE",
    "THIN",
    "ULTRA_LIGHT",
    "LIGHT",
    "DEMI_LIGHT",
    "BOOK",
    "NORMAL",
    "MEDIUM",
    "DEMI_BOLD",
    "BOLD",
    "ULTRA_BOLD",
    "BLACK"
  };
}

namespace FontSlant
{
  /**
   * @brief Enumeration type for the font's slant
   */
  enum Type
  {
    NONE,           ///< Means not defined. Will use what is set as default, currently NORMAL.
    NORMAL,
    ROMAN = NORMAL,
    ITALIC,
    OBLIQUE
  };

  const char* const Name[] =
  {
    "NONE",
    "NORMAL",
    "ITALIC",
    "OBLIQUE"
  };
} // namespace FontSlant

struct FontDescription
{
  enum Type
  {
    INVALID,     ///< Not valid font.
    FACE_FONT,   ///< A face font.
    BITMAP_FONT, ///< A bitmap font. Each glyph has a url with the bitmap.
  };

  FontDescription()
  : path(),
    family(),
    width( FontWidth::NONE ),
    weight( FontWeight::NONE ),
    slant( FontSlant::NONE ),
    type( INVALID )
  {}

  ~FontDescription()
  {}

  FontPath         path;   ///< The font's file name path.
  FontFamily       family; ///< The font's family name.
  FontWidth::Type  width;  ///< The font's width.
  FontWeight::Type weight; ///< The font's weight.
  FontSlant::Type  slant;  ///< The font's slant.
  Type             type;   ///< The type of font.
};

typedef std::vector<FontDescription> FontList;

DALI_ADAPTOR_API std::ostream& operator<<( std::ostream& o, const FontList& fontList );

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TEXT_ABSTRACTION_FONT_LIST_H
