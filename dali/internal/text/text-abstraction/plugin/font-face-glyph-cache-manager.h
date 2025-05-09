#ifndef DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_GLYPH_CACHE_MANAGER_H
#define DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_GLYPH_CACHE_MANAGER_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/glyph-buffer-data.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/internal/text/text-abstraction/plugin/lru-cache-container.h>

// EXTERNAL INCLUDES
#include <memory> // for std::shared_ptr
#include <map>
#include <fontconfig/fontconfig.h>

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
  GlyphCacheManager(std::size_t maxNumberOfGlyphCache);

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
    GlyphCacheData();
    ~GlyphCacheData();

    // Move operations
    GlyphCacheData(GlyphCacheData&& rhs) noexcept;
    GlyphCacheData& operator=(GlyphCacheData&& rhs) noexcept;

    union
    {
      FT_Glyph   mGlyph;
      FT_Bitmap* mBitmap;
    };
    FT_Glyph_Metrics_ mGlyphMetrics{}; // Get from FT_GlyphSlot
    FT_Int32          mStyleFlags{0};  // Get from FT_Face
    bool              mIsBitmap{false};

    TextAbstraction::GlyphBufferData* mRenderedBuffer{nullptr}; // Rendered glyph buffer. Cached only if system allow to cache and we rendered it before. Otherwise, just nullptr

  private:
    // Delete copy operations
    GlyphCacheData(const GlyphCacheData&) = delete;
    GlyphCacheData& operator=(const GlyphCacheData&) = delete;

    /**
     * @brief Release the memory of loaded mGlyph / mBitmap and mRenderedBuffer.
     */
    void ReleaseGlyphData();
  };

  using GlyphCacheDataPtr = std::shared_ptr<GlyphCacheData>;

  // Compression priority of rendered glyph buffer.
  enum class CompressionPolicyType
  {
    SPEED  = 0,
    MEMORY = 1,
  };

public:
  // Public API area.

  /**
   * @brief Load GlyphCacheData from face. The result will be cached.
   * @note Inputed glyph data pointer will be overwrited.
   *
   * @param[in] freeTypeFace The freetype face handle.
   * @param[in] requestedPointSize The requested point size.
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[in] variationsHash The hash of the variations to use key.
   * @param[out] glyphDataPtr Result of pointer of glyph load.
   * @param[out] error Error code during load glyph.
   * @return True if load successfully. False if something error occured.
   */
  bool GetGlyphCacheDataFromIndex(
    const FT_Face         freeTypeFace,
    const PointSize26Dot6 requestedPointSize,
    const GlyphIndex      index,
    const FT_Int32        flag,
    const bool            isBoldRequired,
    const std::size_t     variationsHash,
    GlyphCacheDataPtr&    glyphDataPtr,
    FT_Error&             error);

  /**
   * @brief Load GlyphCacheData from face. The result will not be cached.
   *
   * @param[in] freeTypeFace The freetype face handle.
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[in, out] glyphData Result of glyph load.
   * @param[out] error Error code during load glyph.
   * @return True if load successfully. False if something error occured.
   */
  bool LoadGlyphDataFromIndex(
    const FT_Face    freeTypeFace,
    const GlyphIndex index,
    const FT_Int32   flag,
    const bool       isBoldRequired,
    GlyphCacheData&  glyphData,
    FT_Error&        error);

  /**
   * @brief Resize bitmap glyph. The result will change cached glyph bitmap information.
   * If glyph is not bitmap glyph, nothing happened.
   *
   * @param[in] freeTypeFace The freetype face handle.
   * @param[in] requestedPointSize The requested point size.
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[in] variationsHash The hash of the variations to use key.
   * @param[in] desiredWidth Desired width of bitmap.
   * @param[in] desiredHeight Desired height of bitmap.
   */
  void ResizeBitmapGlyph(
    const FT_Face         freeTypeFace,
    const PointSize26Dot6 requestedPointSize,
    const GlyphIndex      index,
    const FT_Int32        flag,
    const bool            isBoldRequired,
    const std::size_t     variationsHash,
    const uint32_t        desiredWidth,
    const uint32_t        desiredHeight);

  /**
   * @brief Cache rendered glyph bitmap. The result will change cached glyph information.
   * If glyph is not single color glyph, or we already cached buffer before, nothing happened.
   *
   * @param[in] freeTypeFace The freetype face handle.
   * @param[in] requestedPointSize The requested point size.
   * @param[in] index Index of glyph in this face.
   * @param[in] flag Flag when we load the glyph.
   * @param[in] isBoldRequired True if we require some software bold.
   * @param[in] variationsHash The hash of the variations to use key.
   * @param[in] srcBitmap Rendered glyph bitmap.
   * @param[in] policy Compress behavior policy.
   */
  void CacheRenderedGlyphBuffer(
    const FT_Face               freeTypeFace,
    const PointSize26Dot6       requestedPointSize,
    const GlyphIndex            index,
    const FT_Int32              flag,
    const bool                  isBoldRequired,
    const std::size_t           variationsHash,
    const FT_Bitmap&            srcBitmap,
    const CompressionPolicyType policy);

  /**
   * @brief Clear all cached glyph informations which has inputed FreeTypeFace.
   *
   * @note This API iterate all cached glyph. Should be called rarely.
   * @param[in] freeTypeFace The freetype face handle.
   */
  void RemoveGlyphFromFace(const FT_Face freeTypeFace);

  /**
   * @brief Clear all cached glyph informations.
   *
   * @param[in] remainCount The number of remained cache items after call this API. Default is 0, clear all items.
   */
  void ClearCache(const std::size_t remainCount = 0u);

private:
  // Private struct area.
  /**
   * @brief Key of cached glyph.
   */
  struct GlyphCacheKey
  {
    GlyphCacheKey()
    : mFreeTypeFace(nullptr),
      mRequestedPointSize(0),
      mIndex(0u),
      mFlag(0),
      mIsBoldRequired(false),
      mVariationsHash(0u)
    {
    }

    GlyphCacheKey(const FT_Face freeTypeFace, const PointSize26Dot6 requestedPointSize, const GlyphIndex index, const FT_Int32 flag, const bool boldRequired, const std::size_t variationsHash)
    : mFreeTypeFace(freeTypeFace),
      mRequestedPointSize(requestedPointSize),
      mIndex(index),
      mFlag(flag),
      mIsBoldRequired(boldRequired),
      mVariationsHash(variationsHash)
    {
    }

    FT_Face         mFreeTypeFace;
    PointSize26Dot6 mRequestedPointSize;
    GlyphIndex      mIndex;
    FT_Int32        mFlag;
    bool            mIsBoldRequired : 1;
    std::size_t     mVariationsHash;

    bool operator==(GlyphCacheKey const& rhs) const noexcept
    {
      return mFreeTypeFace == rhs.mFreeTypeFace && mRequestedPointSize == rhs.mRequestedPointSize && mIndex == rhs.mIndex && mFlag == rhs.mFlag && mIsBoldRequired == rhs.mIsBoldRequired && mVariationsHash == rhs.mVariationsHash;
    }
  };

  /**
   * @brief Hash function of GlyphCacheKey.
   */
  struct GlyphCacheKeyHash
  {
    std::size_t operator()(GlyphCacheKey const& key) const noexcept
    {
      return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(key.mFreeTypeFace)) ^
             static_cast<std::size_t>(key.mRequestedPointSize) ^
             static_cast<std::size_t>(key.mIndex) ^
             static_cast<std::size_t>(key.mFlag) ^
             (static_cast<std::size_t>(key.mIsBoldRequired) << 29) ^
             key.mVariationsHash;
    }
  };

private:
  // Private member value area.
  std::size_t mGlyphCacheMaxSize; ///< The maximum capacity of glyph cache.

  using CacheContainer = LRUCacheContainer<GlyphCacheKey, GlyphCacheDataPtr, GlyphCacheKeyHash>;

  CacheContainer mLRUGlyphCache; ///< LRU Cache container of glyph
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_GLYPH_CACHE_MANAGER_H
