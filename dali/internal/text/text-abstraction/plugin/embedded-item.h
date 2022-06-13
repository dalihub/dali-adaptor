#ifndef DALI_INTERNAL_TEXT_ABSTRACTION_PLUGIN_EMBEDDED_ITEM_H
#define DALI_INTERNAL_TEXT_ABSTRACTION_PLUGIN_EMBEDDED_ITEM_H

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

#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/text-abstraction/glyph-info.h>
#include <dali/internal/text/text-abstraction/plugin/pixel-buffer-cache-item.h>

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Caches embedded items.
 */
struct EmbeddedItem
{
  // Default constructor.
  EmbeddedItem() = default;

  // Default destructor.
  ~EmbeddedItem() = default;

  /**
   * Get metrics for glyph from image
   * @param[in] item The embedded image
   * @param[in,out] glyph The glyph to get metrics for
   */
  void GetGlyphMetrics(GlyphInfo& glyph);

  /**
   * @brief Create a glyph bitmap from an embedded item if present in the cache
   *
   * @param[in]  pixelBufferCache The pixel buffer cache
   * @param[out] data The bitmap data.
   */
  void CreateBitmap(const std::vector<PixelBufferCacheItem>&            pixelBufferCache,
                    Dali::TextAbstraction::FontClient::GlyphBufferData& data);

  PixelBufferId pixelBufferId{0u}; ///< Index to the vector of pixel buffers
  uint32_t      width{0u};         ///< The desired width.
  uint32_t      height{0u};        ///< The desired height.
  GlyphIndex    index{0u};         ///< The glyph index of this embedded items. Should be bigger than 0.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_INTERNAL_TEXT_ABSTRACTION_PLUGIN_EMBEDDED_ITEM_H
