#ifndef DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_MANAGER_H
#define DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_MANAGER_H

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
#include <dali/devel-api/text-abstraction/font-file-manager.h>
#include <dali/devel-api/text-abstraction/font-list.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/internal/text/text-abstraction/plugin/lru-cache-container.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/property-map.h>

#include <memory> // for std::shared_ptr
#include <unordered_map>

#include <fontconfig/fontconfig.h>
#include <harfbuzz/hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SIZES_H
#include FT_MULTIPLE_MASTERS_H

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief A manager for handling font faces and their associated sizes and variations.
 *
 * This class is responsible for managing font face data, including loading font faces, caching them,
 * handling size and variation settings, and managing the interaction with FreeType and HarfBuzz.
 */
class FontFaceManager
{
public:
  /**
   * @brief Constructor for FontFaceManager.
   *
   * @param[in] maxNumberOfFaceSizeCache The maximum number of face size entries to cache.
   */  FontFaceManager(std::size_t maxNumberOfFaceSizeCache);

  // Destructor
  ~FontFaceManager();

  FontFaceManager(const FontFaceManager& rhs) = delete; // Do not use copy construct
  FontFaceManager(FontFaceManager&& rhs)      = delete; // Do not use move construct

  /**
   * @brief Data structure for caching face-related data.
   */
  struct FaceCacheData
  {
    FaceCacheData()
    : mFreeTypeFace(nullptr),
      mReference(0)
    {
    }

    FaceCacheData(FT_Face freeTypeFace)
    : mFreeTypeFace(freeTypeFace),
      mReference(0)
    {
    }

    FT_Face mFreeTypeFace; ///< The FreeType face handle.
    int     mReference;    ///< The reference count for the face.

    void ReleaseData();
  };

  /**
   * @brief Data structure for caching face size-related data.
   *
   * This struct holds the FreeType size handle for a specific face size.
   */
  struct FaceSizeCacheData
  {
    FaceSizeCacheData();
    ~FaceSizeCacheData();

    // Move operations
    FaceSizeCacheData(FaceSizeCacheData&& rhs) noexcept;
    FaceSizeCacheData& operator=(FaceSizeCacheData&& rhs) noexcept;

    FT_Size mFreeTypeSize{};

  private:
    // Delete copy operations
    FaceSizeCacheData(const FaceSizeCacheData&) = delete;
    FaceSizeCacheData& operator=(const FaceSizeCacheData&) = delete;

    void ReleaseData();
  };

  using FaceSizeCacheDataPtr = std::shared_ptr<FaceSizeCacheData>;

  /**
   * @brief Key structure for identifying face size cache entries.
   */
  struct FaceSizeCacheKey
  {
    FaceSizeCacheKey()
    : mFreeTypeFace(nullptr),
      mRequestedPointSize(0u),
      mVariationsHash(0u)
    {
    }

    FaceSizeCacheKey(const FT_Face freeTypeFace, const PointSize26Dot6 requestedPointSize, const std::size_t variationsHash)
    : mFreeTypeFace(freeTypeFace),
      mRequestedPointSize(requestedPointSize),
      mVariationsHash(variationsHash)
    {
    }

    FT_Face         mFreeTypeFace;
    PointSize26Dot6 mRequestedPointSize;
    std::size_t     mVariationsHash;

    bool operator==(FaceSizeCacheKey const& rhs) const noexcept
    {
      return mFreeTypeFace == rhs.mFreeTypeFace && mRequestedPointSize == rhs.mRequestedPointSize && mVariationsHash == rhs.mVariationsHash;
    }
  };

  /**
   * @brief Hash function of FaceSizeCacheKey.
   */
  struct FaceSizeCacheKeyHash
  {
    std::size_t operator()(FaceSizeCacheKey const& key) const noexcept
    {
      return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(key.mFreeTypeFace)) ^
             static_cast<std::size_t>(key.mRequestedPointSize) ^
             key.mVariationsHash;
    }
  };

  /**
   * @brief Data structure for storing activated face size data.
   */
  struct ActivatedSizeData
  {
    ActivatedSizeData()
    : mRequestedPointSize(0u),
      mVariationsHash(0u)
    {
    }

    ActivatedSizeData(const PointSize26Dot6 requestedPointSize, const std::size_t variationsHash)
    : mRequestedPointSize(requestedPointSize),
      mVariationsHash(variationsHash)
    {
    }

    PointSize26Dot6 mRequestedPointSize;
    std::size_t     mVariationsHash;
  };

private:
  std::size_t mMaxNumberOfFaceSizeCache; ///< The maximum capacity of face size cache.

  using CacheContainer = LRUCacheContainer<FaceSizeCacheKey, FaceSizeCacheDataPtr, FaceSizeCacheKeyHash>;

  CacheContainer mLRUFaceSizeCache; ///< LRU Cache container of face size.

public:
  /**
   * @brief Sets the font file manager.
   * @note For freetype memory face, not thread safe, read only.
   *
   * @param[in] fontFileManager The font file manager.
   */
  void SetFontFileManager(TextAbstraction::FontFileManager fontFileManager);

  /**
   * @brief Sets the DPI for horizontal and vertical dimensions.
   *
   * @param[in] dpiHorizontal The horizontal DPI.
   * @param[in] dpiVertical The vertical DPI.
   */
  void SetDpi(const uint32_t dpiHorizontal, const uint32_t dpiVertical);

  /**
   * @brief Loads a FreeType face from a font file.
   * @note The basic strategy of LoadFace is to create only one freetype face per font file.
   * If there is a face already created in the cache, it returns the cached face.
   * When creating a face, if there is a font file in the cache of the font file manager, it uses FT_New_Memory_Face.
   * Otherwise, it uses FT_New_Face.
   *
   * @param[in] freeTypeLibrary The FreeType library handle.
   * @param[in] fontPath The path to the font file.
   * @param[in] faceIndex The index of the face in the font.
   * @param[out] ftFace The FreeType face handle that will be loaded.
   * @return FT_Err_Ok on success, otherwise an error code.
   */
  FT_Error LoadFace(const FT_Library& freeTypeLibrary, const FontPath& fontPath, const FaceIndex faceIndex, FT_Face& ftFace);

  /**
   * @brief Increases the reference count for a given font face.
   *
   * @param[in] fontPath The path to the font file.
   */
  void ReferenceFace(const FontPath& fontPath);

  /**
   * @brief Decreases the reference count for a given font face and releases it if the count reaches zero.
   *
   * @param[in] fontPath The path to the font file.
   */
  void ReleaseFace(const FontPath& fontPath);

  /**
   * @brief Builds variations data for a font face based on a property map.
   * @note Face only.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] variationsMapPtr The variation map pointer.
   * @param[out] freeTypeCoords The FreeType coordinates for the variations.
   * @param[out] harfBuzzVariations The HarfBuzz variations data.
   */
  void BuildVariations(FT_Face ftFace, const Property::Map* variationsMapPtr, std::vector<FT_Fixed>& freeTypeCoords, std::vector<hb_variation_t>& harfBuzzVariations);

  /**
   * @brief Activates a face for rendering with a specific size and variations.
   * @note Face only.
   * If necessary, call FT_Set_Var_Design_Coordinates, FT_Activate_Size.
   * The key to this method is to minimize calls to FT_Set_Char_Size, which has overhead.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] requestedPointSize The requested point size.
   * @param[in] variationsHash The hash of the variations to use cache key.
   * @param[in] freeTypeCoords The FreeType coordinates for the variations.
   * @return FT_Err_Ok on success, otherwise an error code.
   */
  FT_Error ActivateFace(FT_Face ftFace, const PointSize26Dot6 requestedPointSize, const std::size_t variationsHash, const std::vector<FT_Fixed>& freeTypeCoords);

  /**
   * @brief Whether the freetype face is a bitmap font or not.
   *
   * @param[in] ftFace The FreeType face handle.
   * @return true if face is bitmap font, false otherwise.
   */
  bool IsBitmapFont(FT_Face ftFace) const;

  /**
   * @brief Find the proper fixed size the given freetype face and requested point size.
   * @note Bitmap only.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] requestedPointSize The requested point size.
   * @return The fixed size index for the given freetype face and requested point size.
   */
  int FindFixedSizeIndex(FT_Face ftFace, const PointSize26Dot6 requestedPointSize);

  /**
   * @brief Select the given fixed size.
   * @note Bitmap only.
   * This method minimizes calls to FT_Select_Size, but FT_Select_Size has almost no overhead.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] requestedPointSize The requested point size.
   * @param[in] fixedSizeIndex The fixed size index for freetype face.
   * @return FT_Err_Ok on success, otherwise an error code.
   */
  FT_Error SelectFixedSize(FT_Face ftFace, const PointSize26Dot6 requestedPointSize, const int fixedSizeIndex);

  /**
   * @brief Clear all cached face size informations.
   *
   * @param[in] remainCount The number of remained cache items after call this API. Default is 0, clear all items.
   */
  void ClearCache(const std::size_t remainCount = 0u);

private:
  TextAbstraction::FontFileManager               mFontFileManager; ///< Handle to the font file manager.
  std::unordered_map<std::string, FaceCacheData> mFreeTypeFaces;   //< Cache of loaded FreeType faces.
  std::unordered_map<FT_Face, ActivatedSizeData> mActivatedSizes;  ///< Cache of activated face sizes.
  std::unordered_map<FT_Face, PointSize26Dot6>   mSelectedIndices; ///< Cache of selected fixed size indices.

  uint32_t mDpiHorizontal; ///< Horizontal dpi.
  uint32_t mDpiVertical;   ///< Vertical dpi.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEST_ABSTRACTION_INTERNAL_FONT_FACE_MANAGER_H
