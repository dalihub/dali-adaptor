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

// INTERNAL HEADERS
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-cpal-parser.h>
#include <dali/integration-api/debug.h>

// FreeType headers for SFNT table access
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <cstdlib>

namespace Dali::TextAbstraction::Internal
{

namespace
{

/**
 * @brief Read big-endian uint16 from buffer.
 */
static uint16_t ReadU16(const uint8_t* p)
{
  return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

/**
 * @brief Read big-endian uint32 from buffer.
 */
static uint32_t ReadU32(const uint8_t* p)
{
  return (static_cast<uint32_t>(p[0]) << 24) |
         (static_cast<uint32_t>(p[1]) << 16) |
         (static_cast<uint32_t>(p[2]) << 8) |
         static_cast<uint32_t>(p[3]);
}

} // anonymous namespace

bool ColorGlyphCpalParser::HasCpalTable(FT_Face ftFace)
{
  if(!ftFace) return false;

  FT_ULong tag = FT_MAKE_TAG('C', 'P', 'A', 'L');
  FT_ULong length = 0;
  FT_Error error = FT_Load_Sfnt_Table(ftFace, tag, 0, nullptr, &length);
  return (error == FT_Err_Ok && length > 0);
}

void ColorGlyphCpalParser::SelectPalette(FT_Face ftFace, uint16_t paletteIndex, PaletteInfo& outInfo)
{
  outInfo = PaletteInfo();

  if(!ftFace) return;

  // Load CPAL table
  FT_ULong tag = FT_MAKE_TAG('C', 'P', 'A', 'L');
  FT_ULong length = 0;
  FT_Error error = FT_Load_Sfnt_Table(ftFace, tag, 0, nullptr, &length);
  if(error != FT_Err_Ok || length < 12) return;

  uint8_t* table = static_cast<uint8_t*>(malloc(length));
  if(!table) return;

  error = FT_Load_Sfnt_Table(ftFace, tag, 0, table, &length);
  if(error != FT_Err_Ok)
  {
    free(table);
    return;
  }

  // CPAL v0 header:
  // Offset  Size  Description
  // 0       2     version (0)
  // 2       2     numPaletteEntries
  // 4       2     numPalettes
  // 6       2     numColorRecords
  // 8       4     colorRecordsArrayOffset (Offset32)
  // 12      2*numPalettes  colorRecordIndices[numPalettes]
  //
  // Each colorRecordIndex is a uint16 giving the first color record index
  // for the corresponding palette.

  const uint16_t version = ReadU16(table + 0);
  const uint16_t numPaletteEntries = ReadU16(table + 2);
  const uint16_t numPalettes = ReadU16(table + 4);
  // const uint16_t numColorRecords = ReadU16(table + 6);  // not needed for selection
  const uint32_t colorRecordsArrayOffset = ReadU32(table + 8);

  if(version > 1 || paletteIndex >= numPalettes)
  {
    free(table);
    return;
  }

  // colorRecordIndices array starts at offset 12
  // Each entry is uint16 (2 bytes)
  const uint32_t indexOffset = 12 + paletteIndex * 2;
  if(indexOffset + 2 > length)
  {
    free(table);
    return;
  }

  const uint16_t firstColorIndex = ReadU16(table + indexOffset);

  outInfo.paletteIndex = paletteIndex;
  outInfo.colorCount = numPaletteEntries;
  outInfo.firstColorIndex = firstColorIndex;
  outInfo.numPalettes = numPalettes;
  outInfo.colorRecordOffset = colorRecordsArrayOffset;
  outInfo.valid = true;

  free(table);
}

void ColorGlyphCpalParser::ResolveColor(FT_Face ftFace, uint16_t paletteIndex, uint16_t colorIndex, CpalColor& outColor)
{
  outColor = CpalColor();

  if(!ftFace) return;

  // NOTE: palette_index == 0xFFFF in COLR means foreground color.
  // TODO: Resolve palette index 0xFFFF using the actual text foreground color.
  // The caller should handle this case before calling ResolveColor.
  if(colorIndex == 0xFFFF)
  {
    outColor.valid = false;
    return;
  }

  // Load CPAL table
  FT_ULong tag = FT_MAKE_TAG('C', 'P', 'A', 'L');
  FT_ULong length = 0;
  FT_Error error = FT_Load_Sfnt_Table(ftFace, tag, 0, nullptr, &length);
  if(error != FT_Err_Ok || length < 12) return;

  uint8_t* table = static_cast<uint8_t*>(malloc(length));
  if(!table) return;

  error = FT_Load_Sfnt_Table(ftFace, tag, 0, table, &length);
  if(error != FT_Err_Ok)
  {
    free(table);
    return;
  }

  // CPAL v0 header (same structure as SelectPalette)
  const uint16_t numPaletteEntries = ReadU16(table + 2);
  const uint16_t numPalettes = ReadU16(table + 4);
  const uint32_t colorRecordsArrayOffset = ReadU32(table + 8);

  if(paletteIndex >= numPalettes)
  {
    free(table);
    return;
  }

  // Range check: colorIndex must be within numPaletteEntries
  if(colorIndex >= numPaletteEntries)
  {
    free(table);
    return;
  }

  // Get firstColorIndex for this palette from colorRecordIndices array
  const uint32_t indexOffset = 12 + paletteIndex * 2;
  if(indexOffset + 2 > length)
  {
    free(table);
    return;
  }

  const uint16_t firstColorIndex = ReadU16(table + indexOffset);

  // Color record offset = colorRecordsArrayOffset + (firstColorIndex + colorIndex) * 4
  // Each color record is 4 bytes: B, G, R, A
  const uint32_t recordIndex = firstColorIndex + colorIndex;
  const uint32_t recordOffset = colorRecordsArrayOffset + recordIndex * 4;

  if(recordOffset + 4 > length)
  {
    free(table);
    return;
  }

  // CPAL color record: B, G, R, A (in CPAL binary order)
  outColor.b = table[recordOffset + 0];
  outColor.g = table[recordOffset + 1];
  outColor.r = table[recordOffset + 2];
  outColor.a = table[recordOffset + 3];
  outColor.valid = true;

  free(table);
}

} // namespace Dali::TextAbstraction::Internal
