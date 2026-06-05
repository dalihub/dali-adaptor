#ifndef DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_MANAGER_H
#define DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_MANAGER_H

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

#include <cstdint> // for std::uintptr_t in hash function
#include <functional> // for std::hash
#include <memory> // for std::shared_ptr
#include <unordered_map>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <harfbuzz/hb-ft.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SIZES_H
#include FT_TRUETYPE_TABLES_H
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
   */
  FontFaceManager(std::size_t maxNumberOfFaceSizeCache);

  // Destructor
  ~FontFaceManager();

  FontFaceManager(const FontFaceManager& rhs) = delete; // Do not use copy construct
  FontFaceManager(FontFaceManager&& rhs)      = delete; // Do not use move construct

  /**
   * @brief Stores SFNT color table presence detected from a FreeType face.
   *
   * This records table presence only. It does not indicate whether DALi can
   * render the font as a color glyph font in the current build.
   */
  struct ColorFontInfo
  {
    bool hasCOLR{false};  ///< Whether the font has a COLR table.
    bool hasCPAL{false};  ///< Whether the font has a CPAL table.
    bool hasSVG{false};   ///< Whether the font has an SVG  table.
    bool hasCBDT{false};  ///< Whether the font has a CBDT table.
    bool hasCBLC{false};  ///< Whether the font has a CBLC table.
    bool hasSbix{false};  ///< Whether the font has an sbix table.
  };

  /**
   * @brief Color font renderability classification.
   *
   * Combines color table presence with renderer availability in the current
   * DALi build. Bitmap color fonts use the legacy FT_LOAD_COLOR path, while
   * renderable COLRv1 fonts use the COLRv1 renderer.
   */
  enum class ColorFontRenderability
  {
    NotColorFont,          ///< No color font tables detected
    RenderableBitmap,      ///< CBDT+CBLC: legacy bitmap color font (FT_LOAD_COLOR)
    RenderableColrV1,      ///< COLR+CPAL with DALi COLRv1 renderer available
    NotRenderableColorFont ///< Has color tables but no renderer available in this build
  };

  /**
   * @brief Data structure for caching face-related data.
   */
  struct FaceCacheData
  {
    FaceCacheData()
    : mFreeTypeFace(nullptr),
      mReference(0),
      mColorFontInfo{},
      mColorFontRenderability(ColorFontRenderability::NotColorFont)
    {
    }

    FaceCacheData(FT_Face freeTypeFace)
    : mFreeTypeFace(freeTypeFace),
      mReference(0),
      mColorFontInfo{},
      mColorFontRenderability(ColorFontRenderability::NotColorFont)
    {
    }

    FaceCacheData(FT_Face freeTypeFace, const ColorFontInfo& colorFontInfo, ColorFontRenderability colorFontRenderability)
    : mFreeTypeFace(freeTypeFace),
      mReference(0),
      mColorFontInfo(colorFontInfo),
      mColorFontRenderability(colorFontRenderability)
    {
    }

    FT_Face                mFreeTypeFace;          ///< The FreeType face handle.
    int                    mReference;             ///< The reference count for the face.
    ColorFontInfo          mColorFontInfo;         ///< Cached color table presence (face-level, independent of point size).
    ColorFontRenderability mColorFontRenderability; ///< Cached color font renderability (face-level, independent of point size).

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
    FaceSizeCacheData(const FaceSizeCacheData&)            = delete;
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
   * @brief Fast check whether a font face is a color font candidate.
   *
   * Uses FT_HAS_COLOR, FT_HAS_SVG, FT_HAS_SBIX macros for quick screening.
   * This is a fast candidate check; final determination requires DetectColorFontTables()
   * and GetColorFontRenderability().
   *
   * @param[in] ftFace The FreeType face handle.
   * @return true if the font may have color glyph support.
   */
  static bool IsColorFontCandidate(FT_Face ftFace);

  /**
   * @brief Detect color font SFNT tables in the given FreeType face.
   *
   * @param[in] ftFace The FreeType face handle.
   * @return Detected color table flags.
   */
  static ColorFontInfo DetectColorFontTables(FT_Face ftFace);

  /**
   * @brief Determine renderability using pre-computed ColorFontInfo.
   *
   * Avoids redundant DetectColorFontTables() calls when info is already available.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] info Pre-computed color font table info.
   * @return ColorFontRenderability classification.
   */
  static ColorFontRenderability GetColorFontRenderability(FT_Face ftFace, const ColorFontInfo& info);

  /**
   * @brief Check whether a glyph has a renderable COLRv1 root paint, with caching.
   *
   * Caches the result (both positive and negative) keyed by FT_Face + GlyphIndex
   * in a bounded LRU cache so that repeated queries for the same glyph on the
   * same face do not call FT_Get_Color_Glyph_Paint() again.
   *
   * @param[in] ftFace The FreeType face handle.
   * @param[in] glyphIndex The glyph index to query.
   * @return true if the glyph has a COLRv1 root paint, false otherwise.
   */
  bool HasRenderableColrV1GlyphPaint(FT_Face ftFace, GlyphIndex glyphIndex);

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
   * @brief Get cached color font metadata for a font path.
   *
   * Returns the ColorFontInfo and ColorFontRenderability cached in FaceCacheData
   * when the face was first loaded. If the path is not found, returns defaults.
   *
   * @param[in] fontPath The path to the font file.
   * @param[out] colorInfo The cached color table presence.
   * @param[out] renderability The cached color font renderability.
   */
  void GetColorFontInfo(const FontPath& fontPath, ColorFontInfo& colorInfo, ColorFontRenderability& renderability) const;

  /**
   * @brief Clear all cached face size informations.
   */
  void ClearCache();

private:
  // Data members - declaration order must match constructor initializer list order.
  std::size_t mMaxNumberOfFaceSizeCache; ///< The maximum capacity of face size cache.

  using CacheContainer = LRUCacheContainer<FaceSizeCacheKey, FaceSizeCacheDataPtr, FaceSizeCacheKeyHash>;

  CacheContainer mLRUFaceSizeCache; ///< LRU Cache container of face size.

  TextAbstraction::FontFileManager               mFontFileManager; ///< Handle to the font file manager.
  std::unordered_map<std::string, FaceCacheData>  mFreeTypeFaces;   ///< Cache of loaded FreeType faces.
  std::unordered_map<FT_Face, ActivatedSizeData>  mActivatedSizes;  ///< Cache of activated face sizes.
  std::unordered_map<FT_Face, PointSize26Dot6>    mSelectedIndices; ///< Cache of selected fixed size indices.

  uint32_t mDpiHorizontal; ///< Horizontal dpi.
  uint32_t mDpiVertical;   ///< Vertical dpi.

  /**
   * @brief Key for COLRv1 glyph root paint cache.
   *
   * Keyed by FT_Face + GlyphIndex only. Point size, DPI, and variation hash
   * are intentionally excluded: the question "does this face's glyph have a
   * COLRv1 root paint?" is independent of rendering parameters.
   */
  struct ColrV1GlyphPaintKey
  {
    ColrV1GlyphPaintKey()
    : mFreeTypeFace(nullptr),
      mGlyphIndex(0u)
    {
    }

    ColrV1GlyphPaintKey(FT_Face face, GlyphIndex glyphIndex)
    : mFreeTypeFace(face),
      mGlyphIndex(glyphIndex)
    {
    }

    FT_Face    mFreeTypeFace;
    GlyphIndex mGlyphIndex;

    bool operator==(ColrV1GlyphPaintKey const& rhs) const noexcept
    {
      return mFreeTypeFace == rhs.mFreeTypeFace && mGlyphIndex == rhs.mGlyphIndex;
    }
  };

  /**
   * @brief Hash function for ColrV1GlyphPaintKey.
   *
   * Uses boost::hash_combine-style mixing for better distribution than simple xor.
   */
  struct ColrV1GlyphPaintKeyHash
  {
    std::size_t operator()(ColrV1GlyphPaintKey const& key) const noexcept
    {
      const auto faceHash  = std::hash<std::uintptr_t>{}(reinterpret_cast<std::uintptr_t>(key.mFreeTypeFace));
      const auto glyphHash = std::hash<GlyphIndex>{}(key.mGlyphIndex);
      return faceHash ^ (glyphHash + 0x9e3779b9u + (faceHash << 6u) + (faceHash >> 2u));
    }
  };

  using ColrV1GlyphPaintCache = LRUCacheContainer<ColrV1GlyphPaintKey, bool, ColrV1GlyphPaintKeyHash>;

  /**
   * @brief Remove all cached COLRv1 paint entries for a given FT_Face.
   *
   * Should be called before the FT_Face is actually released to prevent
   * stale keys from remaining in the cache. Currently ReleaseFace() does
   * not actually release faces (commented out), but this helper is provided
   * for future-proofing.
   *
   * @param[in] ftFace The FT_Face whose entries should be removed.
   */
  void EraseColrV1GlyphPaintCacheForFace(FT_Face ftFace);

  ColrV1GlyphPaintCache mColrV1GlyphPaintCache; ///< Bounded LRU cache: (FT_Face, GlyphIndex) → has root paint.
};

} // namespace Dali::TextAbstraction::Internal

#endif //DALI_TEXT_ABSTRACTION_INTERNAL_FONT_FACE_MANAGER_H
