#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_HARFBUZZ_PROXY_FONT_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_HARFBUZZ_PROXY_FONT_H

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
 */

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/internal/text/text-abstraction/font-client-impl.h> // for HarfBuzzFontHandle
#include <dali/internal/text/text-abstraction/plugin/font-face-glyph-cache-manager.h>

// EXTERNAL INCLUDES
#include <ft2build.h>
#include <harfbuzz/hb-ft.h>
#include FT_FREETYPE_H

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Helper class to shape of FT_Face by harfbuzz library.
 * @note Current class only be used for font face cache item.
 */
class HarfBuzzProxyFont
{
public:
  /**
   * @brief Constructor harfbuzz font data integrated with FreeType face and our font face cache item.
   *
   * @param[in] freeTypeFace The FreeType face.
   * @param[in] requestedPointSize The requiested point size of font.
   * @param[in] horizontalDpi Horizontal DPI.
   * @param[in] verticalDpi Vertical DPI.
   * @param[in] glyphCacheManager Glyph caching system for this harfbuzz font. It will be used as harfbuzz callback data.
   */
  HarfBuzzProxyFont(FT_Face freeTypeFace, const PointSize26Dot6& requestedPointSize, const std::size_t variationsHash, const std::vector<hb_variation_t>& harfBuzzVariations, const uint32_t& horizontalDpi, const uint32_t& verticalDpi, GlyphCacheManager* glyphCacheManager);

  // Destructor
  ~HarfBuzzProxyFont();

public:
  // Public API area.

  /**
   * @brief Get the created harfbuzz font data integrated with FreeType face and our font face cache item.
   *
   * @return Created harfbuzz font data. or nullptr if there is something error.
   */
  HarfBuzzFontHandle GetHarfBuzzFont() const;

  /**
   * Refreshes the state of font when the underlying FT_Face has changed.
   * This function should be called after changing the size or variation-axis settings on the FT_Face.
   */
  void FontChanged() const;

private:
  // Private API area.
  HarfBuzzProxyFont()                             = delete; // Do not use default construct
  HarfBuzzProxyFont(const HarfBuzzProxyFont& rhs) = delete; // Do not use copy construct
  HarfBuzzProxyFont(HarfBuzzProxyFont&& rhs)      = delete; // Do not use move construct
public:
  struct Impl;             // Harfbuzz callback can access this struct.
  uint32_t mHorizontalDpi; ///< Horizontal DPI.
  uint32_t mVerticalDpi;   ///< VerticalDPI.

private:
  // Private member value area.
  Impl* mImpl;
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEXT_ABSTRACTION_INTERNAL_HARFBUZZ_PROXY_FONT_H
