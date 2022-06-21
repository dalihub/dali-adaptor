#ifndef DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_GLYPH_CACHE_MANAGER_H
#define DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_GLYPH_CACHE_MANAGER_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h> // For GlyphBufferData
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/internal/text/text-abstraction/plugin/lru-cache-container.h>

// EXTERNAL INCLUDES
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Helper class to load and cache some glyphs of FT_Face.
 */
class GlyphCacheManager
{
public:
  // Constructor
  GlyphCacheManager(FT_Face ftFace, std::size_t maxNumberOfGlyphCache);

  // Destructor
  ~GlyphCacheManager();

  GlyphCacheManager(const GlyphCacheManager& rhs) = delete; // Do not use copy construct
  GlyphCacheManager(GlyphCacheManager&& rhs)      = delete; // Do not use move construct

public:
  // Public struct area.

  /**
   * @brief Result informations of glyph. It can be whether FT_Glyph or FT_Bitmap type.
   * @note FontFaceCacheItem could use this.
   */
  struct GlyphCacheData
  {
    GlyphCacheData()
    : mGlyph{nullptr}
    {
    }

    union
    {
      FT_Glyph   mGlyph;
      FT_Bitmap* mBitmap;
    };
    FT_Glyph_Metrics_ mGlyphMetrics{}; // Get from FT_GlyphSlot
    FT_Int32          mStyleFlags{0};  // Get from FT_Face
    bool              mIsBitmap{false};

    TextAbstraction::FontClient::GlyphBufferData* mRenderedBuffer{nullptr}; // Rendered glyph buffer. Cached only if system allow to cache and we rendered it before. Otherwise, just nullptr

    /**
     * @brief Release the memory of loaded mGlyph / mBitmap.
     */
    void ReleaseGlyphData();
  };

public:
  // Public API area.

  /**
   * @brief Load GlyphCacheData from face. The result will be cached.
   *
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[out] data Result of glyph load.
   * @param[out] error Error code during load glyph.
   * @return True if load successfully. False if something error occured.
   */
  bool GetGlyphCacheDataFromIndex(
    const GlyphIndex index,
    const FT_Int32   flag,
    const bool       isBoldRequired,
    GlyphCacheData&  data,
    FT_Error&        error);

  /**
   * @brief Load GlyphCacheData from face. The result will not be cached.
   * @note If we call this API, We should release GlyphCacheData manually.
   *
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[out] data Result of glyph load.
   * @param[out] error Error code during load glyph.
   * @return True if load successfully. False if something error occured.
   */
  bool LoadGlyphDataFromIndex(
    const GlyphIndex index,
    const FT_Int32   flag,
    const bool       isBoldRequired,
    GlyphCacheData&  data,
    FT_Error&        error);

  /**
   * @brief Resize bitmap glyph. The result will change cached glyph bitmap information.
   * If glyph is not bitmap glyph, nothing happened.
   *
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[in] desiredWidth Desired width of bitmap.
   * @param[in] desiredHeight Desired height of bitmap.
   */
  void ResizeBitmapGlyph(
    const GlyphIndex index,
    const FT_Int32   flag,
    const bool       isBoldRequired,
    const uint32_t   desiredWidth,
    const uint32_t   desiredHeight);

  /**
   * @brief Cache rendered glyph bitmap. The result will change cached glyph information.
   * If glyph is not single color glyph, or we already cached buffer before, nothing happened.
   *
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[in] srcBitmap Rendered glyph bitmap.
   * @param[in] policy Compress behavior policy. default is MEMORY
   */
  void CacheRenderedGlyphBuffer(
    const GlyphIndex                                                       index,
    const FT_Int32                                                         flag,
    const bool                                                             isBoldRequired,
    const FT_Bitmap&                                                       srcBitmap,
    const TextAbstraction::FontClient::GlyphBufferData::CompressPolicyType policy);

private:
  // Private struct area.
  /**
   * @brief Key of cached glyph.
   */
  struct GlyphCacheKey
  {
    GlyphCacheKey()
    : mIndex(0u),
      mFlag(0),
      mIsBoldRequired(false)
    {
    }

    GlyphCacheKey(const GlyphIndex index, const FT_Int32 flag, const bool boldRequired)
    : mIndex(index),
      mFlag(flag),
      mIsBoldRequired(boldRequired)
    {
    }
    GlyphIndex mIndex;
    FT_Int32   mFlag;
    bool       mIsBoldRequired : 1;

    bool operator==(GlyphCacheKey const& rhs) const noexcept
    {
      return mIndex == rhs.mIndex && mFlag == rhs.mFlag && mIsBoldRequired == rhs.mIsBoldRequired;
    }
  };

  /**
   * @brief Hash function of GlyphCacheKey.
   */
  struct GlyphCacheKeyHash
  {
    std::size_t operator()(GlyphCacheKey const& key) const noexcept
    {
      return static_cast<std::size_t>(key.mIndex) ^ static_cast<std::size_t>(key.mFlag) ^ (static_cast<std::size_t>(key.mIsBoldRequired) << 29);
    }
  };

private:
  // Private member value area.
  FT_Face mFreeTypeFace; ///< The FreeType face. Owned from font-face-cache-item

  std::size_t mGlyphCacheMaxSize; ///< The maximum capacity of glyph cache.

  using CacheContainer = LRUCacheContainer<GlyphCacheKey, GlyphCacheData, GlyphCacheKeyHash>;

  CacheContainer mLRUGlyphCache; ///< LRU Cache container of glyph
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_GLYPH_CACHE_MANAGER_H
