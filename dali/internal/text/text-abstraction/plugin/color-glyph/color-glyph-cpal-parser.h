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

#ifndef DALI_INTERNAL_TEXT_TEXT_ABSTRACTION_PLUGIN_COLOR_GLYPH_CPAL_PARSER_H
#define DALI_INTERNAL_TEXT_TEXT_ABSTRACTION_PLUGIN_COLOR_GLYPH_CPAL_PARSER_H

// EXTERNAL HEADERS
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cstdint>

namespace Dali::TextAbstraction::Internal
{

/**
 * @brief Palette info from CPAL table.
 */
struct PaletteInfo
{
  uint16_t paletteIndex{0};      ///< Selected palette index.
  uint16_t colorCount{0};        ///< Number of colors in this palette.
  uint16_t firstColorIndex{0};   ///< First color record index for this palette.
  uint16_t numPalettes{0};       ///< Total number of palettes in the CPAL table.
  uint32_t colorRecordOffset{0}; ///< Offset to color records from the start of the CPAL table.
  bool     valid{false};         ///< Whether palette selection succeeded.
};

/**
 * @brief Resolved color from CPAL palette.
 */
struct CpalColor
{
  uint8_t r{0}, g{0}, b{0}, a{255}; ///< RGBA color values.
  bool    valid{false};              ///< Whether color resolution succeeded.
};

/**
 * @brief Parser for CPAL (Color Palette) table.
 *
 * CPAL provides palette entries used by COLR color glyphs. The foreground
 * palette index 0xFFFF is not resolved here; callers must fail closed or
 * provide the actual text foreground color.
 */
class ColorGlyphCpalParser
{
public:
  /**
   * @brief Checks whether the font face has a CPAL table.
   *
   * @param[in] ftFace The FreeType face handle.
   * @return @e true if a CPAL table exists.
   */
  static bool HasCpalTable(FT_Face ftFace);

  /**
   * @brief Select a palette from the CPAL table.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] paletteIndex The palette index to select.
   * @param[out] outInfo The selected palette information.
   */
  static void SelectPalette(FT_Face ftFace, uint16_t paletteIndex, PaletteInfo& outInfo);

  /**
   * @brief Resolve a color from the CPAL palette.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] paletteIndex The palette index.
   * @param[in] colorIndex The color record index within the palette.
   * @param[out] outColor The resolved color.
   */
  static void ResolveColor(FT_Face ftFace, uint16_t paletteIndex, uint16_t colorIndex, CpalColor& outColor);
};

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_INTERNAL_TEXT_TEXT_ABSTRACTION_PLUGIN_COLOR_GLYPH_CPAL_PARSER_H
