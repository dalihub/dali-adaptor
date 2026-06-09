#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_COMPOSITE_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_COMPOSITE_H

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

#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>

#include <cstdint>

#if DALI_ENABLE_COLR_V1_RENDERER
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_COLOR_H
#endif

namespace Dali::TextAbstraction::Internal
{

#if DALI_ENABLE_COLR_V1_RENDERER

/**
 * @brief Owns a tightly packed BGRA8888 buffer for COLRv1 composition.
 *
 * The buffer is caller-owned and must be released with FreeCompositeBuffer().
 * Pixel channels use BGRA byte order. CompositeBuffers() expects straight
 * alpha input and produces straight alpha output.
 */
struct CompositeBuffer
{
  uint8_t* buffer{nullptr}; ///< BGRA8888 pixel buffer.
  uint32_t width{0};        ///< Width in pixels.
  uint32_t height{0};       ///< Height in pixels.
  uint32_t stride{0};       ///< Row stride in bytes.
  bool valid{false};        ///< Whether the buffer allocation is valid.
};

/**
 * @brief Releases a CompositeBuffer and resets its fields.
 *
 * @param[in,out] buf The buffer to release.
 */
void FreeCompositeBuffer(CompositeBuffer& buf);

/**
 * @brief Allocates a tightly packed BGRA8888 CompositeBuffer.
 *
 * @param[in] width The width in pixels.
 * @param[in] height The height in pixels.
 * @return The allocated buffer, or an invalid buffer on failure.
 */
CompositeBuffer AllocateCompositeBuffer(uint32_t width, uint32_t height);

/**
 * @brief Converts premultiplied BGRA pixels to straight BGRA pixels in place.
 *
 * Alpha-zero pixels are normalized to transparent black.
 *
 * @param[in,out] buffer The BGRA8888 pixel buffer.
 * @param[in] width The width in pixels.
 * @param[in] height The height in pixels.
 * @param[in] stride The row stride in bytes.
 */
void UnpremultiplyBgraBuffer(uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride);

/**
 * @brief Checks whether the COLRv1 composite mode has a pixel implementation.
 *
 * Unsupported modes must fail closed rather than selecting another visual mode.
 *
 * @param[in] mode The FreeType COLRv1 composite mode.
 * @return @e true if CompositeBuffers() supports the mode.
 */
bool IsCompositeModeSupported(FT_Composite_Mode mode);

/**
 * @brief Composites source and backdrop buffers into an output buffer.
 *
 * All buffers must have matching dimensions and tightly packed BGRA8888 rows.
 * Input and output pixels use straight alpha. The source/backdrop order follows
 * the COLRv1 PaintComposite source and backdrop operands.
 *
 * @param[in] mode The FreeType COLRv1 composite mode.
 * @param[in] backdrop The backdrop operand buffer.
 * @param[in] source The source operand buffer.
 * @param[in,out] output The destination buffer.
 * @param[in] debugGlyph The root glyph index used for optional diagnostics.
 * @return @e true if composition succeeds.
 */
bool CompositeBuffers(
  FT_Composite_Mode mode,
  const CompositeBuffer& backdrop,
  const CompositeBuffer& source,
  CompositeBuffer& output,
  uint32_t debugGlyph);

#endif // DALI_ENABLE_COLR_V1_RENDERER

} // namespace Dali::TextAbstraction::Internal

#endif // DALI_TEXT_ABSTRACTION_INTERNAL_COLOR_GLYPH_COLR_COMPOSITE_H
