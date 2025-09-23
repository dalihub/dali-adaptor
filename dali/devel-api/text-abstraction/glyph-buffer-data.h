#ifndef DALI_TEXT_ABSTRACTION_GLYPH_BUFFER_DATA_H
#define DALI_TEXT_ABSTRACTION_GLYPH_BUFFER_DATA_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/images/pixel.h>
#include <cstddef> /// for size_t

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace TextAbstraction
{
/**
 * @brief Struct used to retrieve the glyph's bitmap.
 */
struct DALI_ADAPTOR_API GlyphBufferData
{
  /**
   * @brief Constructor.
   *
   * Initializes struct members to their defaults.
   */
  GlyphBufferData();

  /**
   * @brief Destructor.
   */
  ~GlyphBufferData();

  /**
   * @brief Move constructor.
   *
   * @param[in] rhs moved data.
   */
  GlyphBufferData(GlyphBufferData&& rhs) noexcept;

  /**
   * @brief Move assign operator.
   *
   * @param[in] rhs moved data.
   * @return A reference to this.
   */
  GlyphBufferData& operator=(GlyphBufferData&& rhs) noexcept;

  // Compression method of buffer. Each buffer compressed line by line
  enum class CompressionType
  {
    NO_COMPRESSION = 0, // No compression
    BPP_4          = 1, // Compress as 4 bit. Color become value * 17 (0x00, 0x11, 0x22, ... 0xee, 0xff).
                        // Only works for Pixel::L8 format
    RLE_4 = 2,          // Compress as 4 bit, and Run-Length-Encode. For more high compress rate, we store difference between previous scanline.
                        // Only works for Pixel::L8 format
  };

  /**
   * @brief Helper static function to compress raw buffer from inBuffer to outBufferData.buffer.
   * outBufferData will have it's own buffer.
   *
   * @pre outBufferData must not have it's own buffer.
   * @param[in] inBuffer The input raw data.
   * @param[in, out] outBufferData The output glyph buffer data.
   * @return Size of compressed out buffer, Or 0 if compress failed.
   */
  static size_t Compress(const uint8_t* const inBuffer, GlyphBufferData& outBufferData);

  /**
   * @brief Helper static function to decompress raw buffer from inBuffer to outBufferPtr.
   * If outBuffer is nullptr, Do nothing.
   *
   * @pre outBuffer memory should be allocated.
   * @param[in] inBufferData The input glyph buffer data.
   * @param[in, out] outBuffer The output pointer of raw buffer data.
   */
  static void Decompress(const GlyphBufferData& inBufferData, uint8_t* outBuffer);

  /**
   * @brief Special Helper static function to decompress raw buffer from inBuffer to outBuffer one scanline.
   * After decompress one scanline successed, offset will be changed.
   *
   * @pre outBuffer memory should be allocated.
   * @pre if inBufferData's compression type is RLE4, outBuffer memory should store the previous scanline data.
   * @param[in] inBufferData The input glyph buffer data.
   * @param[in, out] outBuffer The output pointer of raw buffer data.
   * @param[in, out] offset The offset of input. It will be changed as next scanline's offset.
   */
  static void DecompressScanline(const GlyphBufferData& inBufferData, uint8_t* outBuffer, uint32_t& offset);

private:
  // Delete copy operation.
  GlyphBufferData(const GlyphBufferData& rhs)            = delete;
  GlyphBufferData& operator=(const GlyphBufferData& rhs) = delete;

public:
  uint8_t*        buffer;            ///< The glyph's bitmap buffer data.
  uint32_t        width;             ///< The width of the bitmap.
  uint32_t        height;            ///< The height of the bitmap.
  int             outlineOffsetX;    ///< The additional horizontal offset to be added for the glyph's position for outline.
  int             outlineOffsetY;    ///< The additional vertical offset to be added for the glyph's position for outline.
  Pixel::Format   format;            ///< The pixel's format of the bitmap.
  CompressionType compressionType;   ///< The type of buffer compression.
  bool            isColorEmoji : 1;  ///< Whether the glyph is an emoji.
  bool            isColorBitmap : 1; ///< Whether the glyph is a color bitmap.
  bool            isBufferOwned : 1; ///< Whether the glyph's bitmap buffer data owned by this class or not. Becareful when you use non-owned buffer data.
};

} // namespace TextAbstraction

} // namespace Dali

#endif //DALI_TEXT_ABSTRACTION_GLYPH_INFO_H
