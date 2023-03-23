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
#include <dali/integration-api/debug.h>
#include <dali/internal/text/text-abstraction/plugin/harfbuzz-proxy-font.h>

// EXTERNAL INCLUDES
#include FT_GLYPH_H
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Helper class to create and destroy harfbuzz font, and hold data in harfbuzz callback
 * It also cache informations what harfbuzz font need to be created.
 */
struct HarfBuzzProxyFont::Impl
{
  /**
   * @brief Constructor.
   *
   * @param[in] freeTypeFace The FreeType face.
   * @param[in] glyphCacheManager Glyph caching system for this harfbuzz font. It will be used as harfbuzz callback data.
   */
  Impl(FT_Face freeTypeFace, GlyphCacheManager* glyphCacheManager)
  : mFreeTypeFace(freeTypeFace),
    mGlyphCacheManager(glyphCacheManager),
    mHarfBuzzFont(nullptr)
  {
  }

  // Destructor
  ~Impl()
  {
    if(mHarfBuzzFont)
    {
      // It will reduce reference of freetype face automatically.
      hb_font_destroy(mHarfBuzzFont);
    }
  }

public:
  /**
   * @brief Create new harfbuzz font.
   *
   * @param[in] requestedPointSize The requiested point size of font.
   * @param[in] horizontalDpi Horizontal DPI.
   * @param[in] verticalDpi Vertical DPI.
   */
  void CreateHarfBuzzFont(const PointSize26Dot6& requestedPointSize, const uint32_t& horizontalDpi, const uint32_t& verticalDpi);

private:
  /**
   * @brief Register harfbuzz callback functions into current harfbuzz font.
   */
  void SetHarfBuzzFunctions();

public:
  FT_Face            mFreeTypeFace;      ///< The FreeType face. Owned from font-face-cache-item.
  GlyphCacheManager* mGlyphCacheManager; ///< Glyph caching system for this harfbuzz font. Owned from font-client-plugin-cache-handler.

  hb_font_t* mHarfBuzzFont; ///< Harfbuzz font handle integrated with FT_Face.
};

HarfBuzzProxyFont::HarfBuzzProxyFont(FT_Face freeTypeFace, const PointSize26Dot6& requestedPointSize, const uint32_t& horizontalDpi, const uint32_t& verticalDpi, GlyphCacheManager* glyphCacheManager)
: mHorizontalDpi(horizontalDpi),
  mVerticalDpi(verticalDpi),
  mImpl(new Impl(freeTypeFace, glyphCacheManager))
{
  mImpl->CreateHarfBuzzFont(requestedPointSize, mHorizontalDpi, mVerticalDpi);
}

HarfBuzzProxyFont::~HarfBuzzProxyFont()
{
  if(mImpl)
  {
    delete mImpl;
  }
}

HarfBuzzFontHandle HarfBuzzProxyFont::GetHarfBuzzFont() const
{
  if(mImpl)
  {
    return static_cast<HarfBuzzFontHandle>(mImpl->mHarfBuzzFont);
  }
  return nullptr;
}

// Collection of harfbuzz custom callback functions.
// Reference : https://github.com/harfbuzz/harfbuzz/blob/main/src/hb-ft.cc
namespace
{
/**
 * @brief Get glyph informations by dali glyph cache system.
 *
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex Index of glyph.
 * @param[out] glyphDataPtr The result of cached glyph data pointer.
 * @return True if we success to get some glyph data. False otherwise.
 */
static bool GetGlyphCacheData(void* font_data, const GlyphIndex& glyphIndex, GlyphCacheManager::GlyphCacheDataPtr& glyphDataPtr)
{
  HarfBuzzProxyFont::Impl* impl = reinterpret_cast<HarfBuzzProxyFont::Impl*>(font_data);

  // Note : HarfBuzz used only FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING internally.
  if(DALI_LIKELY(impl && impl->mGlyphCacheManager))
  {
    FT_Error error;
    return impl->mGlyphCacheManager->GetGlyphCacheDataFromIndex(impl->mFreeTypeFace, glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING, false, glyphDataPtr, error);
  }
  return false;
}

/**
 * @brief Calculate font extents value both in vertical and horizontal.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[out] extents Extents value of font. (scale as 26.6)
 * @param[in] user_data Registered user data.
 * @return True if we success to get font extents. False otherwise.
 */
static hb_bool_t FontExtentsFunc(hb_font_t* font, void* font_data, hb_font_extents_t* extents, void* user_data)
{
  HarfBuzzProxyFont::Impl* impl = reinterpret_cast<HarfBuzzProxyFont::Impl*>(font_data);

  if(DALI_LIKELY(impl && impl->mFreeTypeFace))
  {
    FT_Size_Metrics& ftMetrics = impl->mFreeTypeFace->size->metrics;

    extents->ascender  = ftMetrics.ascender;
    extents->descender = ftMetrics.descender;
    extents->line_gap  = ftMetrics.height - (extents->ascender - extents->descender);

    return true;
  }
  return false;
}

/**
 * @brief Convert from character into index of glyph.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] character The value of character what we want to get index.
 * @param[out] glyphIndex Index of glyph that current font face used.
 * @param[in] user_data Registered user data.
 * @return True if we success to convert.
 */
static hb_bool_t GlyphNormalIndexConvertFunc(hb_font_t* font, void* font_data, hb_codepoint_t character, hb_codepoint_t* glyphIndex, void* user_data)
{
  HarfBuzzProxyFont::Impl* impl = reinterpret_cast<HarfBuzzProxyFont::Impl*>(font_data);

  if(DALI_LIKELY(impl && impl->mFreeTypeFace))
  {
    *glyphIndex = FT_Get_Char_Index(impl->mFreeTypeFace, character);
    return *glyphIndex != 0;
  }
  return false;
}

/**
 * @brief Convert from character and variant selector into index of glyph.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] character The value of character what we want to get index.
 * @param[in] variantSelector Variant selector.
 * @param[out] glyphIndex Index of glyph that current font face used.
 * @param[in] user_data Registered user data.
 * @return True if we success to convert.
 */
static hb_bool_t GlyphVariantIndexConvertFunc(hb_font_t* font, void* font_data, hb_codepoint_t character, hb_codepoint_t variantSelector, hb_codepoint_t* glyphIndex, void* user_data)
{
  HarfBuzzProxyFont::Impl* impl = reinterpret_cast<HarfBuzzProxyFont::Impl*>(font_data);

  if(DALI_LIKELY(impl && impl->mFreeTypeFace))
  {
    *glyphIndex = FT_Face_GetCharVariantIndex(impl->mFreeTypeFace, character, variantSelector);
    return *glyphIndex != 0;
  }
  return false;
}

/**
 * @brief Calculate glyph advance value in horizontal.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex Index of glyph.
 * @param[in] user_data Registered user data.
 * @return Horizontal advance value of glyphIndex. (scale as 26.6)
 */
static hb_position_t GlyphHorizontalAdvanceFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex, void* user_data)
{
  // Output data stored here.
  GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
  if(GetGlyphCacheData(font_data, static_cast<GlyphIndex>(glyphIndex), glyphDataPtr))
  {
    GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();

    // Note : It may return invalid value for fixed size bitmap glyph.
    // But, Harfbuzz library also return Undefined advanced value if it is fixed size font.
    // So we'll also ignore that case.
    return static_cast<hb_position_t>(glyphData.mGlyphMetrics.horiAdvance);
  }
  return 0;
}
/**
 * @brief Calculate glyph advance value in vertical.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex Index of glyph.
 * @param[in] user_data Registered user data.
 * @return Vertical advance value of glyphIndex. (scale as 26.6)
 */
static hb_position_t GlyphVerticalAdvanceFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex, void* user_data)
{
  // Output data stored here.
  GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
  if(GetGlyphCacheData(font_data, static_cast<GlyphIndex>(glyphIndex), glyphDataPtr))
  {
    GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();

    // Note : It may return invalid value for fixed size bitmap glyph.
    // But, Harfbuzz library also return Undefined advanced value if it is fixed size font.
    // So we'll also ignore that case.
    return static_cast<hb_position_t>(glyphData.mGlyphMetrics.vertAdvance);
  }
  return 0;
}

/**
 * @brief Calculate glyph origin position value in horizontal.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl that register this callback as void* type.
 * @param[in] glyphIndex Index of glyph.
 * @param[out] x Origin position x (scale as 26.6)
 * @param[out] y Origin position y (scale as 26.6)
 * @param[in] user_data Registered user data.
 * @return True if we get data successfully. False if some error occured.
 */
static hb_bool_t GlyphHorizontalOriginFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex, hb_position_t* x, hb_position_t* y, void* user_data)
{
  // Nothing to do
  return true;
}
/**
 * @brief Calculate glyph origin position value in vertical.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex Index of glyph.
 * @param[out] x Origin position x (scale as 26.6)
 * @param[out] y Origin position y (scale as 26.6)
 * @param[in] user_data Registered user data.
 * @return True if we get data successfully. False if some error occured.
 */
static hb_bool_t GlyphVerticalOriginFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex, hb_position_t* x, hb_position_t* y, void* user_data)
{
  // Output data stored here.
  GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
  if(GetGlyphCacheData(font_data, static_cast<GlyphIndex>(glyphIndex), glyphDataPtr))
  {
    GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();

    *x = glyphData.mGlyphMetrics.horiBearingX - glyphData.mGlyphMetrics.vertBearingX;
    *y = glyphData.mGlyphMetrics.horiBearingY + glyphData.mGlyphMetrics.vertBearingY;
    return true;
  }
  return false;
}

/**
 * @brief Calculate glyph kerning value in horizontal.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex1 First index of glyph to get kerning.
 * @param[in] glyphIndex2 Second index of glyph to get kerning.
 * @param[in] user_data Registered user data.
 * @return Horizontal kerning position. (scale as 26.6)
 */
static hb_position_t GlyphHorizontalKerningFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex1, hb_codepoint_t glyphIndex2, void* user_data)
{
  HarfBuzzProxyFont::Impl* impl = reinterpret_cast<HarfBuzzProxyFont::Impl*>(font_data);

  if(DALI_LIKELY(impl && impl->mFreeTypeFace))
  {
    FT_Error  error;
    FT_Vector kerning;

    error = FT_Get_Kerning(impl->mFreeTypeFace, glyphIndex1, glyphIndex2, FT_KERNING_UNSCALED, &kerning);
    if(error == FT_Err_Ok)
    {
      return kerning.x;
    }
  }
  return 0;
}
/**
 * @brief Calculate glyph kerning value in vertical.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex1 First index of glyph to get kerning.
 * @param[in] glyphIndex2 Second index of glyph to get kerning.
 * @param[in] user_data Registered user data.
 * @return Vertical kerning position. (scale as 26.6)
 */
static hb_position_t GlyphVerticalKerningFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex1, hb_codepoint_t glyphIndex2, void* user_data)
{
  // FreeType doesn't support vertical kerning
  return 0;
}

/**
 * @brief Calculate glyph extents.
 *
 * @param[in] font Current harfbuzz font data.
 * @param[in] font_data HarfBuzzProxyFont::Impl pointer that register this callback as void* type.
 * @param[in] glyphIndex Index of glyph.
 * @param[out] extents Extents value of glyph. (scale as 26.6)
 * @param[in] user_data Registered user data.
 * @return True if we get data successfully. False if some error occured.
 */
static hb_bool_t GlyphExtentsFunc(hb_font_t* font, void* font_data, hb_codepoint_t glyphIndex, hb_glyph_extents_t* extents, void* user_data)
{
  // Output data stored here.
  GlyphCacheManager::GlyphCacheDataPtr glyphDataPtr;
  if(!GetGlyphCacheData(font_data, static_cast<GlyphIndex>(glyphIndex), glyphDataPtr))
  {
    GlyphCacheManager::GlyphCacheData& glyphData = *glyphDataPtr.get();

    extents->x_bearing = glyphData.mGlyphMetrics.horiBearingX;
    extents->y_bearing = glyphData.mGlyphMetrics.horiBearingY;
    extents->width     = glyphData.mGlyphMetrics.width;
    extents->height    = glyphData.mGlyphMetrics.height;
    return true;
  }
  return false;
}

} // namespace

void HarfBuzzProxyFont::Impl::CreateHarfBuzzFont(const PointSize26Dot6& requestedPointSize, const uint32_t& horizontalDpi, const uint32_t& verticalDpi)
{
  // Destroy previous hb_font_t if exist.
  if(mHarfBuzzFont)
  {
    // It will reduce reference of freetype face automatically.
    hb_font_destroy(mHarfBuzzFont);
    mHarfBuzzFont = nullptr;
  }

  if(mFreeTypeFace)
  {
    // Before create hb_font_t, we must set FT_Char_Size
    FT_Set_Char_Size(mFreeTypeFace,
                     0u,
                     requestedPointSize,
                     horizontalDpi,
                     verticalDpi);

    // Create font face with increase font face's reference.
    mHarfBuzzFont = hb_ft_font_create_referenced(mFreeTypeFace);

    SetHarfBuzzFunctions();

    if(mHarfBuzzFont)
    {
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::HarfBuzzManager::GetHarfBuzzFont. Create new harfbuzz font : %p freetype face : %p. Requested point size : %u, dpi : horizon %u vertial %u\n", mHarfBuzzFont, mFreeTypeFace, requestedPointSize, horizontalDpi, verticalDpi);
    }
    else
    {
      DALI_LOG_ERROR("ERROR! failed to create harfbuzz font.");
    }
  }
  else
  {
    DALI_LOG_ERROR("ERROR! freetype face is null! something unknown problem occured.");
  }
}

void HarfBuzzProxyFont::Impl::SetHarfBuzzFunctions()
{
  if(mHarfBuzzFont)
  {
    hb_font_funcs_t* customFunctions = hb_font_funcs_create();

    if(customFunctions)
    {
      // Bind custom functions here
      hb_font_funcs_set_font_h_extents_func(customFunctions, FontExtentsFunc, 0, 0);
      hb_font_funcs_set_font_v_extents_func(customFunctions, FontExtentsFunc, 0, 0);

      hb_font_funcs_set_nominal_glyph_func(customFunctions, GlyphNormalIndexConvertFunc, 0, 0);
      hb_font_funcs_set_variation_glyph_func(customFunctions, GlyphVariantIndexConvertFunc, 0, 0);

      hb_font_funcs_set_glyph_h_advance_func(customFunctions, GlyphHorizontalAdvanceFunc, 0, 0);
      hb_font_funcs_set_glyph_v_advance_func(customFunctions, GlyphVerticalAdvanceFunc, 0, 0);
      hb_font_funcs_set_glyph_extents_func(customFunctions, GlyphExtentsFunc, 0, 0);

      hb_font_funcs_set_glyph_h_origin_func(customFunctions, GlyphHorizontalOriginFunc, 0, 0);
      hb_font_funcs_set_glyph_v_origin_func(customFunctions, GlyphVerticalOriginFunc, 0, 0);
      hb_font_funcs_set_glyph_h_kerning_func(customFunctions, GlyphHorizontalKerningFunc, 0, 0);
      hb_font_funcs_set_glyph_v_kerning_func(customFunctions, GlyphVerticalKerningFunc, 0, 0);

      // Set custom functions into our own harfbuzz font
      hb_font_set_funcs(mHarfBuzzFont, customFunctions, this, 0);

      // We must release functions type what we create.
      hb_font_funcs_destroy(customFunctions);
    }
    else
    {
      DALI_LOG_ERROR("ERROR! Fail to create custom harfbuzz functions.");

      // Something wrong while create harfbuzz font. Destory it.
      // It will reduce reference of freetype face automatically.
      hb_font_destroy(mHarfBuzzFont);
      mHarfBuzzFont = nullptr;
    }
  }
}

} // namespace Dali::TextAbstraction::Internal
