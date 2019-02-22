#ifndef DALI_TEXT_ABSTRACTION_GLYPH_INFO_H
#define DALI_TEXT_ABSTRACTION_GLYPH_INFO_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

namespace Dali
{

namespace TextAbstraction
{

/**
 * The information describing a glyph (font ID, index, metrics)
 * The metrics are in pixels.
 */
struct DALI_ADAPTOR_API GlyphInfo
{
  /**
   * @brief Default constructor.
   */
  GlyphInfo();

  /**
   * @brief Creates the GlyphInfo without metrics.
   */
  GlyphInfo( FontId font, GlyphIndex i );

  FontId fontId;           ///< Identifies the font containing the glyph
  GlyphIndex index;        ///< Uniquely identifies a glyph for a given FontId
  float width;             ///< The width of the glyph
  float height;            ///< The height of the glyph
  float xBearing;          ///< The distance from the cursor position to the leftmost border of the glyph
  float yBearing;          ///< The distance from the baseline to the topmost border of the glyph
  float advance;           ///< The distance to move the cursor for this glyph
  float scaleFactor;       ///< The scaling applied (fixed-size fonts only)
  bool isItalicRequired:1; ///< Whether the italic style is required.
  bool isBoldRequired:1;   ///< Whether the bold style is required.
};

} // Dali

} // TextAbstraction

#endif //DALI_TEXT_ABSTRACTION_GLYPH_INFO_H
