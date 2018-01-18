#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_VECTOR_FONT_CACHE_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_VECTOR_FONT_CACHE_H__

/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/glyph-info.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

/**
 * A cache of vector-based font data.
 */
class VectorFontCache
{
public:

  /**
   * Constructor
   */
  VectorFontCache( FT_Library freeTypeLibrary );

  /**
   * Destructor
   */
  ~VectorFontCache();

  /**
   * @brief Get the font ID for a vector-based font.
   *
   * @param[in] url The path to the font file.
   * @return A valid font ID, or zero if the font does not exist.
   */
  FontId GetFontId( const std::string& url );

  /**
   * @brief Get the unscaled metrics for a glyph.
   *
   * @param[in] vectorFontId The font ID for a vector-based font.
   * @param[in,out] array A glyph-info structure with initialized FontId & GlyphIndex value.
   * On return the size, bearing and advance values will be set.
   */
  void GetGlyphMetrics( FontId vectorFontId, GlyphInfo& glyph_info );

  /**
   * @brief Get the vector representation of a glyph.
   *
   * @param[in] vectorFontId The font ID for a vector-based font.
   * @param[in] fontId The equivalent font ID.
   * @param[in] glyphIndex The index of a glyph within the specified font.
   * @param[out] blob A blob of data; this is owned by VectorFontCache and should be copied by the caller of GetVectorBlob().
   * @param[out] blobLength The length of the blob data, or zero if the blob creation failed.
   * @param[out] nominalWidth The width of the blob.
   * @param[out] nominalHeight The height of the blob.
   */
  void GetVectorBlob( FontId vectorFontId, FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight );

private:

  /**
   * @brief Get the font ID for a vector-based font.
   *
   * @param[in] url The path to the font file.
   * @param[out] fontId A valid font ID, or zero if the font does not exist.
   * @return True if the font was found.
   */
  bool FindFont( const std::string& url, FontId& fontId ) const;

  /**
   * @brief Add a new vector-based font to the cache.
   *
   * @param[in] url The path to the font file.
   * @return A valid font ID, or zero if the font does not exist.
   */
  FontId CreateFont( const std::string& url );

  // Undefined copy constructor.
  VectorFontCache( const VectorFontCache& );

  // Undefined assignment constructor.
  VectorFontCache& operator=( const VectorFontCache& );

private:

  struct Impl;
  Impl* mImpl;
};

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_VECTOR_FONT_CACHE_H__
