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

// CLASS HEADER
#include <dali/internal/text/text-abstraction/plugin/font-face-manager.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>

// COLRv1 renderer gating macros - needed for renderability check
#include <dali/internal/text/text-abstraction/plugin/color-glyph/color-glyph-colr-rasterizer.h>

#if DALI_ENABLE_COLR_V1_RENDERER
#include FT_COLOR_H
#endif

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
namespace
{
const uint32_t FONT_AXIS_NAME_LEN = 4;
const uint32_t FROM_16DOT16       = (1 << 16);

constexpr std::size_t MAX_COLR_V1_GLYPH_PAINT_CACHE_SIZE = 512u; ///< Maximum COLRv1 glyph paint cache entries.

/**
 * @brief Convert FreeType-type tag to string.
 *
 * @param[in] tag The FreeType variable tag.
 * @param[out] buffer The converted string tag.
 */
void ConvertTagToString(FT_ULong tag, char buffer[5])
{
  // the tag is same format as used in Harfbuzz.
  buffer[0] = (tag >> 24) & 0xFF;
  buffer[1] = (tag >> 16) & 0xFF;
  buffer[2] = (tag >> 8) & 0xFF;
  buffer[3] = tag & 0xFF;
  buffer[4] = 0;
}

FT_Error ApplyVariationCoordinates(FT_Face ftFace, const std::vector<FT_Fixed>& freeTypeCoords)
{
  if(!ftFace || !FT_HAS_MULTIPLE_MASTERS(ftFace))
  {
    return FT_Err_Ok;
  }

  if(freeTypeCoords.empty())
  {
    return FT_Set_Var_Design_Coordinates(ftFace, 0u, nullptr);
  }

  return FT_Set_Var_Design_Coordinates(ftFace, freeTypeCoords.size(), const_cast<FT_Fixed*>(freeTypeCoords.data()));
}
} // namespace

FontFaceManager::FontFaceManager(std::size_t maxNumberOfFaceSizeCache)
: mMaxNumberOfFaceSizeCache(maxNumberOfFaceSizeCache),
  mLRUFaceSizeCache(mMaxNumberOfFaceSizeCache),
  mFontFileManager(),
  mFreeTypeFaces(),
  mActivatedSizes(),
  mSelectedIndices(),
  mDpiHorizontal(0u),
  mDpiVertical(0u),
  mColrV1GlyphPaintCache(MAX_COLR_V1_GLYPH_PAINT_CACHE_SIZE)
{
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::FontFaceManager Create with maximum size : %d\n", static_cast<int>(mMaxNumberOfFaceSizeCache));
}

FontFaceManager::~FontFaceManager()
{
  ClearCache();
}

void FontFaceManager::SetFontFileManager(TextAbstraction::FontFileManager fontFileManager)
{
  mFontFileManager = fontFileManager;
}

void FontFaceManager::SetDpi(const uint32_t dpiHorizontal, const uint32_t dpiVertical)
{
  mDpiHorizontal = dpiHorizontal;
  mDpiVertical   = dpiVertical;
}

FT_Error FontFaceManager::LoadFace(const FT_Library& freeTypeLibrary, const FontPath& fontPath, const FaceIndex faceIndex, FT_Face& ftFace)
{
  FT_Error error;

  auto iter = mFreeTypeFaces.find(fontPath);
  if(iter != mFreeTypeFaces.end())
  {
    ftFace = iter->second.mFreeTypeFace;
    error  = FT_Err_Ok;
  }
  else
  {
    Dali::Any      fontFilePtr   = nullptr;
    std::streampos fileSize      = 0;
    bool           fontFileFound = mFontFileManager.FindFontFile(fontPath, fontFilePtr, fileSize);

    if(fontFileFound)
    {
      error = FT_New_Memory_Face(freeTypeLibrary, reinterpret_cast<FT_Byte*>(AnyCast<uint8_t*>(fontFilePtr)), static_cast<FT_Long>(fileSize), static_cast<FT_Long>(faceIndex), &ftFace);
      DALI_LOG_DEBUG_INFO("FontFaceManager, FT_New_Memory_Face : %s\n", fontPath.c_str());
    }
    else
    {
      error = FT_New_Face(freeTypeLibrary, fontPath.c_str(), static_cast<FT_Long>(faceIndex), &ftFace);
      DALI_LOG_DEBUG_INFO("FontFaceManager, FT_New_Face : %s\n", fontPath.c_str());
    }

    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_ERROR("Load freetype face fail, error code:0x%02X, memory face:%d\n", error, fontFileFound);
    }
    else
    {
      // Compute color font metadata once at face creation time.
      // This avoids repeated FT_Load_Sfnt_Table() calls when the same face
      // is used with different point sizes.
      ColorFontInfo          colorInfo;
      ColorFontRenderability renderability = ColorFontRenderability::NotColorFont;
      if(IsColorFontCandidate(ftFace))
      {
        colorInfo     = DetectColorFontTables(ftFace);
        renderability = GetColorFontRenderability(ftFace, colorInfo);
      }
      mFreeTypeFaces[fontPath] = FaceCacheData(ftFace, colorInfo, renderability);
    }
  }
  return error;
}

void FontFaceManager::ReferenceFace(const FontPath& fontPath)
{
  auto iter = mFreeTypeFaces.find(fontPath);
  if(iter != mFreeTypeFaces.end())
  {
    iter->second.mReference++;
  }
}

void FontFaceManager::ReleaseFace(const FontPath& fontPath)
{
  if(fontPath.empty())
  {
    return;
  }

  auto iter = mFreeTypeFaces.find(fontPath);
  if(iter != mFreeTypeFaces.end())
  {
    iter->second.mReference--;
    if(iter->second.mReference <= 0)
    {
      // TODO:
      // Currently FontFaceCacheItem can be removed from LRU Cache regardless of whether the text control is referenced or not.
      // This will cause a crash when the freetype face is destroyed at this time.
      // We should remove this comment after fixing the FontFaceCacheItem issue.
      // EraseColrV1GlyphPaintCacheForFace(iter->second.mFreeTypeFace);
      // iter->second.ReleaseData();
      // mFreeTypeFaces.erase(iter);
    }
  }
}

void FontFaceManager::BuildVariations(FT_Face ftFace, const Property::Map* variationsMapPtr, std::vector<FT_Fixed>& freeTypeCoords, std::vector<hb_variation_t>& harfBuzzVariations)
{
  if(variationsMapPtr)
  {
    FT_MM_Var* mm_var;
    FT_Error   error = FT_Get_MM_Var(ftFace, &mm_var);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_ERROR("FT_Get_MM_Var fail, error code:0x%02X\n", error);
      return;
    }

    const Property::Map& variationsMap = *variationsMapPtr;

    freeTypeCoords.resize(mm_var->num_axis);
    harfBuzzVariations.resize(mm_var->num_axis);

    for(uint32_t axisIndex = 0; axisIndex < mm_var->num_axis; axisIndex++)
    {
      char stringTag[FONT_AXIS_NAME_LEN + 1];
      ConvertTagToString(mm_var->axis[axisIndex].tag, stringTag);
      auto  valuePtr = variationsMap.Find(stringTag);
      float value    = 0.0f;

      if(valuePtr != nullptr && valuePtr->Get(value))
      {
        freeTypeCoords[axisIndex] = static_cast<FT_Fixed>(value * FROM_16DOT16);
      }
      else
      {
        freeTypeCoords[axisIndex] = mm_var->axis[axisIndex].def;
        value                     = static_cast<float>(freeTypeCoords[axisIndex]) / static_cast<float>(FROM_16DOT16);
      }

      hb_variation_t harfBuzzVariation;
      harfBuzzVariation.tag         = mm_var->axis[axisIndex].tag;
      harfBuzzVariation.value       = value;
      harfBuzzVariations[axisIndex] = harfBuzzVariation;
    }
  }
}

FT_Error FontFaceManager::ActivateFace(FT_Face ftFace, const PointSize26Dot6 requestedPointSize, const std::size_t variationsHash, const std::vector<FT_Fixed>& freeTypeCoords)
{
  auto sizeIter = mActivatedSizes.find(ftFace);
  if(sizeIter != mActivatedSizes.end())
  {
    if(sizeIter->second.mRequestedPointSize == requestedPointSize && variationsHash == sizeIter->second.mVariationsHash)
    {
      // Already activated face.
      return FT_Err_Ok;
    }
  }

  const FaceSizeCacheKey key  = FaceSizeCacheKey(ftFace, requestedPointSize, variationsHash);
  auto                   iter = mLRUFaceSizeCache.Find(key);
  FaceSizeCacheDataPtr   faceSizePtr;
  FT_Error               error;

  if(iter != mLRUFaceSizeCache.End())
  {
    error = ApplyVariationCoordinates(ftFace, freeTypeCoords);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_ERROR("FT_Set_Var_Design_Coordinates fail, error code:0x%02X\n", error);
      return error;
    }

    mActivatedSizes[ftFace] = ActivatedSizeData(requestedPointSize, variationsHash);
    faceSizePtr             = mLRUFaceSizeCache.GetElement(iter);
    error                   = FT_Activate_Size(faceSizePtr->mFreeTypeSize);
    if(DALI_UNLIKELY(error != FT_Err_Ok))
    {
      DALI_LOG_ERROR("FT_Activate_Size fail, error code:0x%02X\n", error);
    }
    return error;
  }

  if(mLRUFaceSizeCache.IsFull())
  {
    auto removedData = mLRUFaceSizeCache.PopWithKey();
    if(removedData.first.mRequestedPointSize == mActivatedSizes[removedData.first.mFreeTypeFace].mRequestedPointSize &&
       removedData.first.mVariationsHash == mActivatedSizes[removedData.first.mFreeTypeFace].mVariationsHash)
    {
      DALI_LOG_DEBUG_INFO("FontClient::Plugin::FontFaceManager::ActivateFace, cache size : %zu, erase : %p\n", mLRUFaceSizeCache.Count(), removedData.second->mFreeTypeSize);
      mActivatedSizes.erase(removedData.first.mFreeTypeFace);
    }
  }

  error = ApplyVariationCoordinates(ftFace, freeTypeCoords);
  if(DALI_UNLIKELY(error != FT_Err_Ok))
  {
    DALI_LOG_ERROR("FT_Set_Var_Design_Coordinates fail, error code:0x%02X\n", error);
    return error;
  }

  FT_Size ftSize;
  error = FT_New_Size(ftFace, &ftSize);
  if(DALI_UNLIKELY(error != FT_Err_Ok))
  {
    DALI_LOG_ERROR("FT_New_Size fail, error code:0x%02X\n", error);
    return error;
  }

  error = FT_Activate_Size(ftSize);
  if(DALI_UNLIKELY(error != FT_Err_Ok))
  {
    FT_Done_Size(ftSize);
    DALI_LOG_ERROR("FT_Activate_Size fail, error code:0x%02X\n", error);
    return error;
  }

  error = FT_Set_Char_Size(ftFace, 0u, FT_F26Dot6(requestedPointSize), mDpiHorizontal, mDpiVertical);
  if(DALI_UNLIKELY(error != FT_Err_Ok))
  {
    FT_Done_Size(ftSize);
    DALI_LOG_ERROR("FT_Set_Char_Size fail, error code:0x%02X\n", error);
    return error;
  }

  mActivatedSizes[ftFace]    = ActivatedSizeData(requestedPointSize, variationsHash);
  faceSizePtr                = std::make_shared<FaceSizeCacheData>();
  faceSizePtr->mFreeTypeSize = std::move(ftSize);
  mLRUFaceSizeCache.Push(key, faceSizePtr);

  return error;
}

bool FontFaceManager::IsBitmapFont(FT_Face ftFace) const
{
  const bool isScalable           = (0 != (ftFace->face_flags & FT_FACE_FLAG_SCALABLE));
  const bool hasFixedSizedBitmaps = (0 != (ftFace->face_flags & FT_FACE_FLAG_FIXED_SIZES)) && (0 != ftFace->num_fixed_sizes);
  return !isScalable && hasFixedSizedBitmaps;
}

bool FontFaceManager::IsColorFontCandidate(FT_Face ftFace)
{
  if(!ftFace)
  {
    return false;
  }

  if(FT_HAS_COLOR(ftFace))
  {
    return true;
  }

#ifdef FT_HAS_SVG
  if(FT_HAS_SVG(ftFace))
  {
    return true;
  }
#endif

#ifdef FT_HAS_SBIX
  if(FT_HAS_SBIX(ftFace))
  {
    return true;
  }
#endif

  return false;
}

FontFaceManager::ColorFontInfo FontFaceManager::DetectColorFontTables(FT_Face ftFace)
{
  ColorFontInfo info;

  if(!ftFace)
  {
    return info;
  }

  auto checkTable = [](FT_Face face, FT_ULong tag) -> bool {
    FT_ULong length = 0;
    FT_Error error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &length);
    return (FT_Err_Ok == error && length > 0);
  };

  info.hasCOLR = checkTable(ftFace, FT_MAKE_TAG('C', 'O', 'L', 'R'));
  info.hasCPAL = checkTable(ftFace, FT_MAKE_TAG('C', 'P', 'A', 'L'));
  info.hasSVG  = checkTable(ftFace, FT_MAKE_TAG('S', 'V', 'G', ' '));
  info.hasCBDT = checkTable(ftFace, FT_MAKE_TAG('C', 'B', 'D', 'T'));
  info.hasCBLC = checkTable(ftFace, FT_MAKE_TAG('C', 'B', 'L', 'C'));
  info.hasSbix = checkTable(ftFace, FT_MAKE_TAG('s', 'b', 'i', 'x'));

  return info;
}

FontFaceManager::ColorFontRenderability FontFaceManager::GetColorFontRenderability(FT_Face ftFace, const ColorFontInfo& info)
{
  if(!ftFace)
  {
    return ColorFontRenderability::NotColorFont;
  }

  // No color tables at all
  if(!info.hasCOLR && !info.hasCPAL && !info.hasSVG && !info.hasCBDT && !info.hasCBLC && !info.hasSbix)
  {
    return ColorFontRenderability::NotColorFont;
  }

  // Legacy bitmap color font: CBDT+CBLC → renderable via FT_LOAD_COLOR
  if(info.hasCBDT && info.hasCBLC)
  {
    return ColorFontRenderability::RenderableBitmap;
  }

  // COLRv1 renderer: requires COLR+CPAL and DALI_ENABLE_COLR_V1_RENDERER
  if(info.hasCOLR && info.hasCPAL && DALI_ENABLE_COLR_V1_RENDERER)
  {
    return ColorFontRenderability::RenderableColrV1;
  }

  // Has color tables but no renderer available in this build
  // This includes:
  // - COLR+CPAL without DALI_ENABLE_COLR_V1_RENDERER (FreeType < 2.13 or no ThorVG 1.0)
  // - SVG-only fonts (SVG renderer disabled in current work direction)
  // - sbix-only fonts (not supported)
  return ColorFontRenderability::NotRenderableColorFont;
}

bool FontFaceManager::HasRenderableColrV1GlyphPaint(FT_Face ftFace, GlyphIndex glyphIndex)
{
#if DALI_ENABLE_COLR_V1_RENDERER
  if(!ftFace)
  {
    return false;
  }

  const ColrV1GlyphPaintKey key(ftFace, glyphIndex);

  // Check LRU cache. Use Get() on hit to mark as recently used.
  const auto iter = mColrV1GlyphPaintCache.Find(key);
  if(iter != mColrV1GlyphPaintCache.End())
  {
    return mColrV1GlyphPaintCache.Get(key);
  }

  // Cache miss: call FT_Get_Color_Glyph_Paint()
  FT_OpaquePaint rootPaint;
  rootPaint.p                     = nullptr;
  rootPaint.insert_root_transform = false;

  const FT_Bool hasPaint = FT_Get_Color_Glyph_Paint(
    ftFace,
    glyphIndex,
    FT_COLOR_NO_ROOT_TRANSFORM,
    &rootPaint);

  const bool result = (hasPaint && rootPaint.p != nullptr);

  // Store result (both positive and negative) in bounded LRU cache
  mColrV1GlyphPaintCache.Push(key, result);

  return result;
#else
  return false;
#endif
}

void FontFaceManager::EraseColrV1GlyphPaintCacheForFace(FT_Face ftFace)
{
  if(!ftFace)
  {
    return;
  }

  // Erase() returns the next valid iterator, so we can continue iterating safely.
  for(auto it = mColrV1GlyphPaintCache.Begin(); it != mColrV1GlyphPaintCache.End(); )
  {
    if(mColrV1GlyphPaintCache.GetKey(it).mFreeTypeFace == ftFace)
    {
      it = mColrV1GlyphPaintCache.Erase(it);
    }
    else
    {
      ++it;
    }
  }
}

int FontFaceManager::FindFixedSizeIndex(FT_Face ftFace, const PointSize26Dot6 requestedPointSize)
{
  PointSize26Dot6 actualPointSize = 0u;
  int             fixedSizeIndex  = 0;
  for(; fixedSizeIndex < ftFace->num_fixed_sizes; ++fixedSizeIndex)
  {
    const PointSize26Dot6 fixedSize = static_cast<PointSize26Dot6>(ftFace->available_sizes[fixedSizeIndex].size);
    if(fixedSize >= requestedPointSize)
    {
      actualPointSize = fixedSize;
      break;
    }
  }

  if(0u == actualPointSize)
  {
    // The requested point size is bigger than the bigest fixed size.
    fixedSizeIndex = ftFace->num_fixed_sizes - 1;
  }
  return fixedSizeIndex;
}

FT_Error FontFaceManager::SelectFixedSize(FT_Face ftFace, const PointSize26Dot6 requestedPointSize, const int fixedSizeIndex)
{
  auto sizeIter = mSelectedIndices.find(ftFace);
  if(sizeIter != mSelectedIndices.end())
  {
    if(sizeIter->second == requestedPointSize)
    {
      // Already selected fixed size.
      return FT_Err_Ok;
    }
  }

  FT_Error error = FT_Select_Size(ftFace, fixedSizeIndex);
  if(DALI_UNLIKELY(error != FT_Err_Ok))
  {
    DALI_LOG_ERROR("FT_Select_Size fail, error code:0x%02X\n", error);
  }
  else
  {
    mSelectedIndices[ftFace] = requestedPointSize;
  }
  return error;
}

void FontFaceManager::GetColorFontInfo(const FontPath& fontPath, ColorFontInfo& colorInfo, ColorFontRenderability& renderability) const
{
  colorInfo     = ColorFontInfo{};
  renderability = ColorFontRenderability::NotColorFont;

  auto iter = mFreeTypeFaces.find(fontPath);
  if(iter != mFreeTypeFaces.end())
  {
    colorInfo     = iter->second.mColorFontInfo;
    renderability = iter->second.mColorFontRenderability;
  }
}

void FontFaceManager::ClearCache()
{
  // Clear FT_Face-keyed caches before releasing faces
  mColrV1GlyphPaintCache.Clear();
  mLRUFaceSizeCache.Clear();

  for(auto& item : mFreeTypeFaces)
  {
    item.second.ReleaseData();
  }

  mFreeTypeFaces.clear();
  mActivatedSizes.clear();
  mSelectedIndices.clear();
}

// FontFaceManager::FaceCacheData
void FontFaceManager::FaceCacheData::ReleaseData()
{
  FT_Done_Face(mFreeTypeFace);
  mFreeTypeFace = nullptr;
}

// FontFaceManager::FaceSizeCacheData
void FontFaceManager::FaceSizeCacheData::ReleaseData()
{
  FT_Done_Size(mFreeTypeSize);
}

FontFaceManager::FaceSizeCacheData::FaceSizeCacheData()
: mFreeTypeSize{}
{
}

FontFaceManager::FaceSizeCacheData::~FaceSizeCacheData()
{
  ReleaseData();
}

FontFaceManager::FaceSizeCacheData::FaceSizeCacheData(FaceSizeCacheData&& rhs) noexcept
: mFreeTypeSize{}
{
  *this = std::move(rhs);
}

FontFaceManager::FaceSizeCacheData& FontFaceManager::FaceSizeCacheData::operator=(FaceSizeCacheData&& rhs) noexcept
{
  // Self-assignment detection
  if(this == &rhs)
  {
    return *this;
  }

  ReleaseData();

  mFreeTypeSize     = rhs.mFreeTypeSize;
  rhs.mFreeTypeSize = nullptr;

  return *this;
}

} // namespace Dali::TextAbstraction::Internal
