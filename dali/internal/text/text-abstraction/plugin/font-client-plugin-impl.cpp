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
 *
 */

// CLASS HEADER
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-list.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/platform-abstraction.h>
#include <dali/integration-api/trace.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/system/common/logging.h>
#include <dali/internal/text/text-abstraction/plugin/bitmap-font-cache-item.h>
#include <dali/internal/text/text-abstraction/plugin/embedded-item.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-cache-handler.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-cache-item.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>

// EXTERNAL INCLUDES
#include <fontconfig/fontconfig.h>
#include <algorithm>
#include <iterator>

// Use this macro only if need to log messages before the log function is set.
#define FONT_LOG_MESSAGE(level, format, ...)                                    \
  do                                                                            \
  {                                                                             \
    char buffer[256];                                                           \
    int  result = std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
    if(result >= static_cast<int>(sizeof(buffer)))                              \
    {                                                                           \
      std::string log("Font log message is too long to fit in the buffer.\n");  \
      Dali::TizenPlatform::LogMessage(Dali::Integration::Log::ERROR, log);      \
      break;                                                                    \
    }                                                                           \
    std::string log(buffer);                                                    \
    Dali::TizenPlatform::LogMessage(level, log);                                \
  } while(0)

#if defined(DEBUG_ENABLED)

// Note, to turn on trace and verbose logging, use "export LOG_FONT_CLIENT=3,true"
// Or re-define the following filter using Verbose,true instead of NoLogging,false,
// Or, add DALI_LOG_FILTER_ENABLE_TRACE(gFontClientLogFilter) in the code below.

Dali::Integration::Log::Filter* gFontClientLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_FONT_CLIENT");

#define FONT_LOG_DESCRIPTION(fontDescription, prefix)                                                                            \
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, #prefix "  description; family : [%s]\n", fontDescription.family.c_str()); \
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose,                                                                            \
                "                 path : [%s]\n"                                                                                 \
                "                width : [%s]\n"                                                                                 \
                "               weight : [%s]\n"                                                                                 \
                "                slant : [%s]\n\n",                                                                              \
                fontDescription.path.c_str(),                                                                                    \
                FontWidth::Name[fontDescription.width],                                                                          \
                FontWeight::Name[fontDescription.weight],                                                                        \
                FontSlant::Name[fontDescription.slant])

#define FONT_LOG_REQUEST(charcode, requestedPointSize, preferColor) \
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General,               \
                "           character : %p\n"                       \
                "  requestedPointSize : %d\n"                       \
                "         preferColor : %s\n",                      \
                charcode,                                           \
                requestedPointSize,                                 \
                (preferColor ? "true" : "false"))

#else

#define FONT_LOG_DESCRIPTION(fontDescription, prefix)
#define FONT_LOG_REQUEST(charcode, requestedPointSize, preferColor)

#endif

namespace
{
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_FONT_PERFORMANCE_MARKER, false);

/**
 * Conversion from Fractional26.6 to float
 */
const float FROM_266        = 1.0f / 64.0f;
const float POINTS_PER_INCH = 72.f;
const uint32_t FONT_AXIS_NAME_LEN = 4;
const uint32_t FROM_16DOT16 = (1 << 16);

const uint32_t ELLIPSIS_CHARACTER     = 0x2026;
const uint32_t CUSTOM_FONTS_MAX_COUNT = 10u;

} // namespace

using Dali::Vector;

namespace Dali::TextAbstraction::Internal
{
namespace
{
/**
 * @brief Check if @p ftFace and @p requestedPointSize produces block that fit into atlas block
 *
 * @param[in/out] ftFace Face type object.
 * @param[in] horizontalDpi The horizontal dpi.
 * @param[in] verticalDpi The vertical dpi.
 * @param[in] maxSizeFitInAtlas The maximum size of block to fit into atlas
 * @param[in] requestedPointSize The requested point-size.
 * @return whether the  ftFace's block can fit into atlas
 */
bool IsFitIntoAtlas(FT_Face& ftFace, int& error, const unsigned int& horizontalDpi, const unsigned int& verticalDpi, const Size& maxSizeFitInAtlas, const uint32_t& requestedPointSize)
{
  bool isFit = false;

  error = FT_Set_Char_Size(ftFace,
                           0,
                           FT_F26Dot6(requestedPointSize),
                           horizontalDpi,
                           verticalDpi);

  if(error == FT_Err_Ok)
  {
    //Check width and height of block for requestedPointSize
    //If the width or height is greater than the maximum-size then decrement by one unit of point-size.
    if(static_cast<float>(ftFace->size->metrics.height) * FROM_266 <= maxSizeFitInAtlas.height && (static_cast<float>(ftFace->size->metrics.ascender) - static_cast<float>(ftFace->size->metrics.descender)) * FROM_266 <= maxSizeFitInAtlas.width)
    {
      isFit = true;
    }
  }

  return isFit;
}

/**
 * @brief Convert Freetype-type tag to string.
 *
 * @param[in] tag The Freetype variable tag.
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

/**
 * @brief Search on proper @p requestedPointSize that produces block that fit into atlas block considering on @p ftFace, @p horizontalDpi, and @p verticalDpi
 *
 * @param[in/out] ftFace Face type object.
 * @param[in] horizontalDpi The horizontal dpi.
 * @param[in] verticalDpi The vertical dpi.
 * @param[in] maxSizeFitInAtlas The maximum size of block to fit into atlas
 * @param[in/out] requestedPointSize The requested point-size.
 * @return FreeType error code. 0 means success when requesting the nominal size (in points).
 */
int SearchOnProperPointSize(FT_Face& ftFace, const unsigned int& horizontalDpi, const unsigned int& verticalDpi, const Size& maxSizeFitInAtlas, uint32_t& requestedPointSize)
{
  //To improve performance of sequential search. This code is applying Exponential search then followed by Binary search.
  const uint32_t& pointSizePerOneUnit = TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  bool            canFitInAtlas;
  int             error; // FreeType error code.

  canFitInAtlas = IsFitIntoAtlas(ftFace, error, horizontalDpi, verticalDpi, maxSizeFitInAtlas, requestedPointSize);
  if(FT_Err_Ok != error)
  {
    return error;
  }

  if(!canFitInAtlas)
  {
    //Exponential search
    uint32_t exponentialDecrement = 1;

    while(!canFitInAtlas && requestedPointSize > pointSizePerOneUnit * exponentialDecrement)
    {
      requestedPointSize -= (pointSizePerOneUnit * exponentialDecrement);
      canFitInAtlas = IsFitIntoAtlas(ftFace, error, horizontalDpi, verticalDpi, maxSizeFitInAtlas, requestedPointSize);
      if(FT_Err_Ok != error)
      {
        return error;
      }

      exponentialDecrement *= 2;
    }

    //Binary search
    uint32_t minPointSize;
    uint32_t maxPointSize;

    if(canFitInAtlas)
    {
      exponentialDecrement /= 2;
      minPointSize = requestedPointSize;
      maxPointSize = requestedPointSize + (pointSizePerOneUnit * exponentialDecrement);
    }
    else
    {
      minPointSize = 0;
      maxPointSize = requestedPointSize;
    }

    while(minPointSize < maxPointSize)
    {
      requestedPointSize = ((maxPointSize / pointSizePerOneUnit - minPointSize / pointSizePerOneUnit) / 2) * pointSizePerOneUnit + minPointSize;
      canFitInAtlas      = IsFitIntoAtlas(ftFace, error, horizontalDpi, verticalDpi, maxSizeFitInAtlas, requestedPointSize);
      if(FT_Err_Ok != error)
      {
        return error;
      }

      if(canFitInAtlas)
      {
        if(minPointSize == requestedPointSize)
        {
          //Found targeted point-size
          return error;
        }

        minPointSize = requestedPointSize;
      }
      else
      {
        maxPointSize = requestedPointSize;
      }
    }
  }

  return error;
}

} // namespace

FontClient::Plugin::Plugin(unsigned int horizontalDpi,
                           unsigned int verticalDpi)
: mFreeTypeLibrary(nullptr),
  mDpiHorizontal(horizontalDpi),
  mDpiVertical(verticalDpi),
  mIsAtlasLimitationEnabled(TextAbstraction::FontClient::DEFAULT_ATLAS_LIMITATION_ENABLED),
  mCurrentMaximumBlockSizeFitInAtlas(TextAbstraction::FontClient::MAX_SIZE_FIT_IN_ATLAS),
  mVectorFontCache(nullptr),
  mCacheHandler(new CacheHandler())
{
  int error = FT_Init_FreeType(&mFreeTypeLibrary);
  if(FT_Err_Ok != error)
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FreeType Init error: %d\n", error);
  }

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  mVectorFontCache = new VectorFontCache(mFreeTypeLibrary);
#endif
}

FontClient::Plugin::~Plugin()
{
  // Delete cache hanlder before remove mFreeTypeLibrary
  delete mCacheHandler;

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  delete mVectorFontCache;
#endif

  FT_Done_FreeType(mFreeTypeLibrary);
}

void FontClient::Plugin::ClearCache() const
{
  mCacheHandler->ClearCache();
}

void FontClient::Plugin::ClearCacheOnLocaleChanged() const
{
  mCacheHandler->ClearCacheOnLocaleChanged();
}

void FontClient::Plugin::SetDpi(unsigned int horizontalDpi,
                                unsigned int verticalDpi)
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical   = verticalDpi;
}

void FontClient::Plugin::ResetSystemDefaults() const
{
  mCacheHandler->ResetSystemDefaults();
}

void FontClient::Plugin::CacheFontDataFromFile(const std::string& fontPath) const
{
  if(fontPath.empty())
  {
    return;
  }

  if(mCacheHandler->FindFontData(fontPath))
  {
    // Font data is already cached, no need to reload
    return;
  }

  Dali::Vector<uint8_t> fontDataBuffer;
  std::streampos        dataSize = 0;
  if(!mCacheHandler->LoadFontDataFromFile(fontPath, fontDataBuffer, dataSize))
  {
    fontDataBuffer.Clear();
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "Failed to load font data : %s\n", fontPath.c_str());
    return;
  }

  // Cache font data
  mCacheHandler->CacheFontData(fontPath, std::move(fontDataBuffer), dataSize);
}

void FontClient::Plugin::CacheFontFaceFromFile(const std::string& fontPath) const
{
  if(fontPath.empty())
  {
    return;
  }

  if(mCacheHandler->FindFontFace(fontPath))
  {
    // Font face is already cached, no need to reload
    return;
  }

  FT_Face ftFace;
  int     error = FT_New_Face(mFreeTypeLibrary, fontPath.c_str(), 0, &ftFace);
  if(FT_Err_Ok != error)
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "Failed to load font face : %s\n", fontPath.c_str());
    return;
  }

  // Cache font face
  mCacheHandler->CacheFontFace(fontPath, ftFace);
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "PreLoad font new face : %s\n", fontPath.c_str());
}

void FontClient::Plugin::FontPreLoad(const FontPathList& fontPathList, const FontPathList& memoryFontPathList) const
{
  for(const auto& fontPath : fontPathList)
  {
    CacheFontFaceFromFile(fontPath);
  }

  for(const auto& memoryFontPath : memoryFontPathList)
  {
    CacheFontDataFromFile(memoryFontPath);
  }
}

void FontClient::Plugin::FontPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily) const
{
  mCacheHandler->InitDefaultFontDescription();

  FontFamilyList familyList;
  familyList.reserve(extraFamilyList.size() + 1);

  for(const auto& fallbackFont : fallbackFamilyList)
  {
    FontList*         fontList          = nullptr;
    CharacterSetList* characterSetList  = nullptr;
    FontDescriptionId fontDescriptionId = 0u;
    FontDescription   fontDescription;
    fontDescription.family = FontFamily(fallbackFont);
    fontDescription.weight = DefaultFontWeight();
    fontDescription.width  = DefaultFontWidth();
    fontDescription.slant  = DefaultFontSlant();

    if(!mCacheHandler->FindFallbackFontList(fontDescription, fontList, characterSetList))
    {
      FontDescription copiedFontDescription = fontDescription;
      mCacheHandler->CacheFallbackFontList(std::move(copiedFontDescription), fontList, characterSetList);
    }
    if(!mCacheHandler->FindValidatedFont(fontDescription, fontDescriptionId))
    {
      mCacheHandler->ValidateFont(fontDescription, fontDescriptionId);
    }

    if(extraFamilyList.empty() && localeFamily.empty())
    {
      continue;
    }

    familyList.clear();
    familyList.insert(familyList.end(), extraFamilyList.begin(), extraFamilyList.end());
    if(!localeFamily.empty())
    {
      familyList.push_back(localeFamily);
    }

    for(const auto& font : *fontList)
    {
      auto it = std::find(familyList.begin(), familyList.end(), font.family);
      if(it != familyList.end())
      {
        if(!mCacheHandler->FindValidatedFont(font, fontDescriptionId))
        {
          mCacheHandler->ValidateFont(font, fontDescriptionId);
        }
        familyList.erase(it);
      }
    }
  }
}

void FontClient::Plugin::InitDefaultFontDescription() const
{
  mCacheHandler->InitDefaultFontDescription();
}

void FontClient::Plugin::GetDefaultPlatformFontDescription(FontDescription& fontDescription) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  mCacheHandler->InitDefaultFontDescription();
  fontDescription = mCacheHandler->mDefaultFontDescription;

  FONT_LOG_DESCRIPTION(fontDescription, "");
}

void FontClient::Plugin::GetDefaultFonts(FontList& defaultFonts) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  mCacheHandler->InitDefaultFonts();
  defaultFonts = mCacheHandler->mDefaultFonts;

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of default fonts : [%d]\n", mCacheHandler->mDefaultFonts.size());
}

void FontClient::Plugin::GetSystemFonts(FontList& systemFonts) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  mCacheHandler->InitSystemFonts();
  systemFonts = mCacheHandler->mSystemFonts;

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of system fonts : [%d]\n", mCacheHandler->mSystemFonts.size());
}

void FontClient::Plugin::GetDescription(FontId           fontId,
                                        FontDescription& fontDescription) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);

  if(mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
  {
    const auto& fontIdCacheItem = mCacheHandler->FindFontIdCacheItem(fontId - 1u);
    switch(fontIdCacheItem.type)
    {
      case FontDescription::FACE_FONT:
      {
        for(auto it = mCacheHandler->mFontDescriptionSizeCache.Begin();
        it != mCacheHandler->mFontDescriptionSizeCache.End();
        it++)
        {
          const auto& item = mCacheHandler->mFontDescriptionSizeCache.GetElement(it);
          if(item == fontIdCacheItem.index)
          {
            const auto& key = mCacheHandler->mFontDescriptionSizeCache.GetKey(it);
            fontDescription = *(mCacheHandler->mFontDescriptionCache.begin() + key.fontDescriptionId - 1u);

            FONT_LOG_DESCRIPTION(fontDescription, "");
            return;
          }
        }
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        fontDescription.type   = FontDescription::BITMAP_FONT;
        auto it = mCacheHandler->mBitmapFontCache.find(fontIdCacheItem.index);
        if(it != mCacheHandler->mBitmapFontCache.end())
        {
          fontDescription.family = it->second.font.name;
        }
        break;
      }
      default:
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  Invalid type of font\n");
        fontDescription.type = FontDescription::INVALID;
        fontDescription.family.clear();
      }
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  No description found for the font id %d\n", fontId);
}

PointSize26Dot6 FontClient::Plugin::GetPointSize(FontId fontId) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);

  PointSize26Dot6               pointSize     = TextAbstraction::FontClient::DEFAULT_POINT_SIZE;
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    pointSize = fontCacheItem->GetPointSize();
  }
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  point size : %d\n", pointSize);

  return pointSize;
}

bool FontClient::Plugin::IsCharacterSupportedByFont(FontId fontId, Character character) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "    font id : %d\n", fontId);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  character : %p\n", character);

  bool isSupported   = false;
  auto fontCacheItem = const_cast<FontCacheItemInterface*>(GetCachedFontItem(fontId));
  if(fontCacheItem != nullptr)
  {
    isSupported = fontCacheItem->IsCharacterSupported(mCacheHandler->mFontConfig, character); // May cache
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  is supported : %s\n", (isSupported ? "true" : "false"));
  return isSupported;
}

const FontCacheItemInterface* FontClient::Plugin::GetCachedFontItem(FontId fontId) const
{
  if(mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
  {
    const auto& fontIdCacheItem = mCacheHandler->FindFontIdCacheItem(fontId - 1u);
    switch(fontIdCacheItem.type)
    {
      case FontDescription::FACE_FONT:
      {
        return &mCacheHandler->FindFontFaceCacheItem(fontIdCacheItem.index);
      }
      case FontDescription::BITMAP_FONT:
      {
        return &mCacheHandler->FindBitmapFontCacheItem(fontIdCacheItem.index);
      }
      default:
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  Invalid type of font\n");
      }
    }
  }
  return nullptr;
}

FontId FontClient::Plugin::FindFontForCharacter(const FontList&         fontList,
                                                const CharacterSetList& characterSetList,
                                                Character               character,
                                                PointSize26Dot6         requestedPointSize,
                                                bool                    preferColor) const
{
  DALI_ASSERT_DEBUG((fontList.size() == characterSetList.Count()) && "FontClient::Plugin::FindFontForCharacter. Different number of fonts and character sets.");
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "           character : %p\n", character);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "         preferColor : %s\n", (preferColor ? "true" : "false"));

  FontId fontId     = 0u;
  bool   foundColor = false;

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of fonts : %d\n", fontList.size());

  // Traverse the list of fonts.
  // Check for each font if supports the character.
  for(unsigned int index = 0u, numberOfFonts = fontList.size(); index < numberOfFonts; ++index)
  {
    const FontDescription& description  = fontList[index];
    const FcCharSet* const characterSet = characterSetList[index];

    FONT_LOG_DESCRIPTION(description, "");

    bool foundInRanges = false;
    if(nullptr != characterSet)
    {
      foundInRanges = FcCharSetHasChar(characterSet, character);
    }

    if(foundInRanges)
    {
      fontId = GetFontId(description, requestedPointSize);

      if(fontId > 0u)
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "     font id : %d\n", fontId);

        if(preferColor)
        {
          if(mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
          {
            auto index = mCacheHandler->FindFontIdCacheItem(fontId - 1u).index;
            const FontFaceCacheItem& item = mCacheHandler->FindFontFaceCacheItem(index);
            foundColor = item.mHasColorTables;
          }

          DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  foundColor : %s\n", (foundColor ? "true" : "false"));
        }

        // Keep going unless we prefer a different (color) font.
        if(!preferColor || foundColor)
        {
          break;
        }
      }
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::FindDefaultFont(Character       charcode,
                                           PointSize26Dot6 requestedPointSize,
                                           bool            preferColor) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_REQUEST(charcode, requestedPointSize, preferColor);

  FontId fontId(0);

  // Create the list of default fonts if it has not been created.
  mCacheHandler->InitDefaultFonts();
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of default fonts : %d\n", mCacheHandler->mDefaultFonts.size());

  // Traverse the list of default fonts.
  // Check for each default font if supports the character.
  fontId = FindFontForCharacter(mCacheHandler->mDefaultFonts, mCacheHandler->mDefaultFontCharacterSets, charcode, requestedPointSize, preferColor);

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::FindFallbackFont(Character              charcode,
                                            const FontDescription& preferredFontDescription,
                                            PointSize26Dot6        requestedPointSize,
                                            bool                   preferColor) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_REQUEST(charcode, requestedPointSize, preferColor);

  DALI_TRACE_SCOPE(gTraceFilter, "DALI_TEXT_FIND_FALLBACKFONT");

  // The font id to be returned.
  FontId fontId = 0u;

  FontDescription fontDescription;

  // Fill the font description with the preferred font description and complete with the defaults.
  fontDescription.family = preferredFontDescription.family.empty() ? DefaultFontFamily() : preferredFontDescription.family;
  fontDescription.weight = ((FontWeight::NONE == preferredFontDescription.weight) ? DefaultFontWeight() : preferredFontDescription.weight);
  fontDescription.width  = ((FontWidth::NONE == preferredFontDescription.width) ? DefaultFontWidth() : preferredFontDescription.width);
  fontDescription.slant  = ((FontSlant::NONE == preferredFontDescription.slant) ? DefaultFontSlant() : preferredFontDescription.slant);

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  preferredFontDescription --> fontDescription\n");
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  [%s] --> [%s]\n", preferredFontDescription.family.c_str(), fontDescription.family.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontWeight::Name[preferredFontDescription.weight], FontWeight::Name[fontDescription.weight]);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontWidth::Name[preferredFontDescription.width], FontWidth::Name[fontDescription.width]);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontSlant::Name[preferredFontDescription.slant], FontSlant::Name[fontDescription.slant]);

#if defined(TRACE_ENABLED)
  if(gTraceFilter && gTraceFilter->IsTraceEnabled())
  {
    DALI_LOG_DEBUG_INFO("DALI_TEXT_FIND_FALLBACKFONT : %s -> %s\n", preferredFontDescription.family.c_str(), fontDescription.family.c_str());
  }
#endif

  // Check first if the font's description has been queried before.
  FontList*         fontList         = nullptr;
  CharacterSetList* characterSetList = nullptr;

  if(!mCacheHandler->FindFallbackFontList(fontDescription, fontList, characterSetList))
  {
    mCacheHandler->CacheFallbackFontList(std::move(fontDescription), fontList, characterSetList);
  }

  if(fontList && characterSetList)
  {
    fontId = FindFontForCharacter(*fontList, *characterSetList, charcode, requestedPointSize, preferColor);
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::GetFontIdByPath(const FontPath& path,
                                           PointSize26Dot6 requestedPointSize,
                                           FaceIndex       faceIndex,
                                           bool            cacheDescription,
                                           Property::Map*  variationsMapPtr) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "                path : [%s]\n", path.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);

  FontId id = 0u;

  if(nullptr != mFreeTypeLibrary)
  {
    FontId foundId = 0u;
    if(mCacheHandler->FindFontByPath(path, requestedPointSize, faceIndex, variationsMapPtr, foundId))
    {
      id = foundId;
    }
    else
    {
      id = CreateFont(path, requestedPointSize, faceIndex, cacheDescription, variationsMapPtr);
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", id);
  return id;
}

FontId FontClient::Plugin::GetFontId(const FontDescription& fontDescription,
                                     PointSize26Dot6        requestedPointSize,
                                     FaceIndex              faceIndex,
                                     Property::Map*         variationsMapPtr) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_DESCRIPTION(fontDescription, "");

  // Special case when font Description don't have family information.
  // In this case, we just use default description family and path.
  const FontDescription& realFontDescription = fontDescription.family.empty() ? FontDescription(mCacheHandler->mDefaultFontDescription.path,
                                                                                                mCacheHandler->mDefaultFontDescription.family,
                                                                                                fontDescription.width,
                                                                                                fontDescription.weight,
                                                                                                fontDescription.slant,
                                                                                                fontDescription.type)
                                                                              : fontDescription;

  FONT_LOG_DESCRIPTION(realFontDescription, "");
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "   requestedPointSize : %d\n", requestedPointSize);

  // This method uses three vectors which caches:
  // * The bitmap font cache
  // * Pairs of non validated font descriptions and an index to a vector with paths to font file names.
  // * The path to font file names.
  // * The font ids of pairs 'font point size, index to the vector with paths to font file names'.

  // 1) Checks if the font description matches with a previously loaded bitmap font.
  //    Returns if a font is found.
  // 2) Checks in the cache if the font's description has been validated before.
  //    If it was it gets an index to the vector with paths to font file names. Otherwise,
  //    retrieves using font config a path to a font file name which matches with the
  //    font's description. The path is stored in the cache.
  //
  // 3) Checks in the cache if the pair 'font point size, index to the vector with paths to
  //    font file names' exists. If exists, it gets the font id. If it doesn't it calls
  //    the GetFontId() method with the path to the font file name and the point size to
  //    get the font id.

  // The font id to be returned.
  FontId fontId = 0u;

  // Check first if the font description matches with a previously loaded bitmap font.
  if(mCacheHandler->FindBitmapFont(realFontDescription.family, fontId))
  {
    return fontId;
  }

  // Check if the font's description have been validated before.
  FontDescriptionId fontDescriptionId = 0u;

  if(!mCacheHandler->FindValidatedFont(realFontDescription, fontDescriptionId))
  {
    // Use font config to validate the font's description.
    mCacheHandler->ValidateFont(realFontDescription, fontDescriptionId);
  }

  using FontCacheIndex          = CacheHandler::FontCacheIndex;
  FontCacheIndex fontCacheIndex = 0u;
  // Check if exists a pair 'fontDescriptionId, requestedPointSize' in the cache.

  if(!mCacheHandler->FindFont(fontDescriptionId, requestedPointSize, fontCacheIndex, variationsMapPtr)
  || !mCacheHandler->IsFontFaceCacheItemExist(fontCacheIndex))
  {
    if(fontDescriptionId > 0u && fontDescriptionId <= mCacheHandler->mCharacterSetCache.Count())
    {
      // Retrieve the font file name path.
      const FontDescription& description = *(mCacheHandler->mFontDescriptionCache.begin() + fontDescriptionId - 1u);

      // Retrieve the font id. Do not cache the description as it has been already cached.
      // Note : CacheFontPath() API call ValidateFont() + setup CharacterSet + cache the font description.
      // So set cacheDescription=false, that we don't call CacheFontPath().
      fontId = GetFontIdByPath(description.path, requestedPointSize, faceIndex, false, variationsMapPtr);

      if(fontId > 0u && mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
      {
        fontCacheIndex = mCacheHandler->FindFontIdCacheItem(fontId - 1u).index;
        mCacheHandler->FindFontFaceCacheItem(fontCacheIndex).mCharacterSet = FcCharSetCopy(mCacheHandler->mCharacterSetCache[fontDescriptionId - 1u]);
      }

      // Cache the pair 'fontDescriptionId, requestedPointSize' to improve the following queries.
      mCacheHandler->CacheFontDescriptionSize(fontDescriptionId, requestedPointSize, variationsMapPtr, fontCacheIndex);
    }
  }
  else
  {
    fontId = mCacheHandler->FindFontFaceCacheItem(fontCacheIndex).mFontId + 1u;
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::GetFontId(const BitmapFont& bitmapFont) const
{
  // The font id to be returned.
  FontId fontId = 0u;
  if(!mCacheHandler->FindBitmapFont(bitmapFont.name, fontId))
  {
    BitmapFontCacheItem bitmapFontCacheItem(bitmapFont);

    fontId = mCacheHandler->CacheBitmapFontCacheItem(std::move(bitmapFontCacheItem));
  }
  return fontId;
}

void FontClient::Plugin::GetFontMetrics(FontId       fontId,
                                        FontMetrics& metrics) const
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    fontCacheItem->GetFontMetrics(metrics, mDpiVertical);
  }
}

GlyphIndex FontClient::Plugin::GetGlyphIndex(FontId    fontId,
                                             Character charcode) const
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    return fontCacheItem->GetGlyphIndex(charcode);
  }

  return 0u;
}

GlyphIndex FontClient::Plugin::GetGlyphIndex(FontId    fontId,
                                             Character charcode,
                                             Character variantSelector) const
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    return fontCacheItem->GetGlyphIndex(charcode, variantSelector);
  }

  return 0u;
}

bool FontClient::Plugin::GetGlyphMetrics(GlyphInfo* array,
                                         uint32_t   size,
                                         GlyphType  type,
                                         bool       horizontal) const
{
  if(VECTOR_GLYPH == type)
  {
    return GetVectorMetrics(array, size, horizontal);
  }

  return GetBitmapMetrics(array, size, horizontal);
}

bool FontClient::Plugin::GetBitmapMetrics(GlyphInfo* array,
                                          uint32_t   size,
                                          bool       horizontal) const
{
  bool success(size > 0u);

  for(unsigned int i = 0; i < size; ++i)
  {
    GlyphInfo& glyph = array[i];

    const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(glyph.fontId);
    if(fontCacheItem != nullptr)
    {
      success &= fontCacheItem->GetGlyphMetrics(glyph, mDpiVertical, horizontal);
    }
    // Check if it's an embedded image.
    else if((0u == glyph.fontId) && (0u != glyph.index) && (glyph.index <= mCacheHandler->mEmbeddedItemCache.size()))
    {
      mCacheHandler->mEmbeddedItemCache[glyph.index - 1u].GetGlyphMetrics(glyph);
    }
    else
    {
      success = false;
    }
  }

  return success;
}

bool FontClient::Plugin::GetVectorMetrics(GlyphInfo* array,
                                          uint32_t   size,
                                          bool       horizontal) const
{
#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  bool success(true);

  for(unsigned int i = 0u; i < size; ++i)
  {
    FontId fontId = array[i].fontId;
    if(mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
    {
      auto index = mCacheHandler->FindFontIdCacheItem(fontId - 1u).index;
      FontFaceCacheItem& font = mCacheHandler->FindFontFaceCacheItem(index);

      if(!font.mVectorFontId)
      {
        font.mVectorFontId = mVectorFontCache->GetFontId(font.mPath);
      }

      mVectorFontCache->GetGlyphMetrics(font.mVectorFontId, array[i]);

      // Vector metrics are in EMs, convert to pixels
      const float scale = (static_cast<float>(font.mRequestedPointSize) * FROM_266) * static_cast<float>(mDpiVertical) / POINTS_PER_INCH;
      array[i].width *= scale;
      array[i].height *= scale;
      array[i].xBearing *= scale;
      array[i].yBearing *= scale;
      array[i].advance *= scale;
    }
    else
    {
      success = false;
    }
  }

  return success;
#else
  return false;
#endif
}

void FontClient::Plugin::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::GlyphBufferData& data, int outlineWidth) const
{
  data.isColorBitmap                          = false;
  data.isColorEmoji                           = false;
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    fontCacheItem->CreateBitmap(glyphIndex, data, outlineWidth, isItalicRequired, isBoldRequired);
  }
  else if((0u != glyphIndex) && (glyphIndex <= mCacheHandler->mEmbeddedItemCache.size()))
  {
    // It's an embedded item.
    mCacheHandler->mEmbeddedItemCache[glyphIndex - 1u].CreateBitmap(mCacheHandler->mPixelBufferCache, data);
  }
}

PixelData FontClient::Plugin::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth) const
{
  TextAbstraction::GlyphBufferData data;

  CreateBitmap(fontId, glyphIndex, false, false, data, outlineWidth);

  // If data is compressed or not owned buffer, copy this.
  if(!data.isBufferOwned || data.compressionType != TextAbstraction::GlyphBufferData::CompressionType::NO_COMPRESSION)
  {
    uint8_t* newBuffer = (uint8_t*)malloc(data.width * data.height * Pixel::GetBytesPerPixel(data.format));
    if(DALI_UNLIKELY(!newBuffer))
    {
      DALI_LOG_ERROR("malloc is failed. request malloc size : %u x %u x %u\n", data.width, data.height, Pixel::GetBytesPerPixel(data.format));
      return Dali::PixelData();
    }

    TextAbstraction::GlyphBufferData::Decompress(data, newBuffer);
    if(data.isBufferOwned)
    {
      free(data.buffer);
    }

    data.buffer          = newBuffer;
    data.isBufferOwned   = true;
    data.compressionType = TextAbstraction::GlyphBufferData::CompressionType::NO_COMPRESSION;
  }

  return PixelData::New(data.buffer,
                        data.width * data.height * Pixel::GetBytesPerPixel(data.format),
                        data.width,
                        data.height,
                        data.format,
                        PixelData::FREE);
}

void FontClient::Plugin::CreateVectorBlob(FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight) const
{
  blob       = nullptr;
  blobLength = 0;

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING

  if(mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
  {
    using FontCacheIndex                = CacheHandler::FontCacheIndex;
    const FontCacheIndex fontCacheIndex = mCacheHandler->FindFontIdCacheItem(fontId - 1u).index;
    FontFaceCacheItem&   font           = mCacheHandler->FindFontFaceCacheItem(fontCacheIndex);
    if(!font.mVectorFontId)
    {
      font.mVectorFontId = mVectorFontCache->GetFontId(font.mPath);
    }

    mVectorFontCache->GetVectorBlob(font.mVectorFontId, fontCacheIndex, glyphIndex, blob, blobLength, nominalWidth, nominalHeight);
  }
#endif
}

const GlyphInfo& FontClient::Plugin::GetEllipsisGlyph(PointSize26Dot6 requestedPointSize) const
{
  using EllipsisCacheIndex              = CacheHandler::EllipsisCacheIndex;
  using EllipsisItem                    = CacheHandler::EllipsisItem;
  EllipsisCacheIndex ellipsisCacheIndex = 0u;

  if(!mCacheHandler->FindEllipsis(requestedPointSize, ellipsisCacheIndex))
  {
    // No glyph has been found. Create one.
    EllipsisItem item;

    item.requestedPointSize = requestedPointSize;

    // Find a font for the ellipsis glyph.
    item.glyph.fontId = FindDefaultFont(ELLIPSIS_CHARACTER,
                                        requestedPointSize,
                                        false);

    // Set the character index to access the glyph inside the font.
    item.glyph.index = GetGlyphIndex(item.glyph.fontId, ELLIPSIS_CHARACTER);

    // Get glyph informations.
    GetBitmapMetrics(&item.glyph, 1u, true);

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  glyph id %d found in the cache.\n", item.glyph.index);
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "      font %d.\n", item.glyph.fontId);

    // EllipsisCacheIndex is stored in item.index.
    ellipsisCacheIndex = mCacheHandler->CacheEllipsis(std::move(item));
    if(!mCacheHandler->mEllipsisCache.empty())
    {
      mCacheHandler->mEllipsisCache.back().index = ellipsisCacheIndex;
    }
  }
  return mCacheHandler->mEllipsisCache[ellipsisCacheIndex].glyph;
}

bool FontClient::Plugin::IsColorGlyph(FontId fontId, GlyphIndex glyphIndex) const
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  return fontCacheItem && fontCacheItem->IsColorGlyph(glyphIndex);
}

FT_FaceRec_* FontClient::Plugin::GetFreetypeFace(FontId fontId) const
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    return fontCacheItem->GetTypeface();
  }
  return nullptr;
}

FontDescription::Type FontClient::Plugin::GetFontType(FontId fontId) const
{
  if(mCacheHandler->IsFontIdCacheItemExist(fontId - 1u))
  {
    return mCacheHandler->FindFontIdCacheItem(fontId - 1u).type;
  }
  return FontDescription::INVALID;
}

bool FontClient::Plugin::AddCustomFontDirectory(const FontPath& path)
{
  mCacheHandler->mCustomFontDirectories.push_back(path);
  if(!mCacheHandler->mFontConfig)
  {
    return false;
  }
  return FcConfigAppFontAddDir(mCacheHandler->mFontConfig, reinterpret_cast<const FcChar8*>(path.c_str()));
}

const FontPathList& FontClient::Plugin::GetCustomFontDirectories()
{
  return mCacheHandler->mCustomFontDirectories;
}

HarfBuzzFontHandle FontClient::Plugin::GetHarfBuzzFont(FontId fontId) const
{
  FontCacheItemInterface* fontCacheItem = const_cast<FontCacheItemInterface*>(GetCachedFontItem(fontId));
  if(fontCacheItem != nullptr)
  {
    return fontCacheItem->GetHarfBuzzFont(mDpiHorizontal, mDpiVertical); // May cache
  }
  return nullptr;
}

GlyphIndex FontClient::Plugin::CreateEmbeddedItem(const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat) const
{
  EmbeddedItem embeddedItem;

  embeddedItem.pixelBufferId = 0u;
  embeddedItem.width         = description.width;
  embeddedItem.height        = description.height;

  pixelFormat = Pixel::A8;

  if(!description.url.empty())
  {
    // Check if the url is in the cache.
    Devel::PixelBuffer pixelBuffer;
    if(!mCacheHandler->FindEmbeddedPixelBufferId(description.url, embeddedItem.pixelBufferId))
    {
      // The pixel buffer is not in the cache. Create one and cache it.
      embeddedItem.pixelBufferId = mCacheHandler->CacheEmbeddedPixelBuffer(description.url);
    }

    if((embeddedItem.pixelBufferId > 0u) && (embeddedItem.pixelBufferId - 1u) < mCacheHandler->mPixelBufferCache.size())
    {
      // Retrieve the pixel buffer from the cache to set the pixel format.
      pixelBuffer = mCacheHandler->mPixelBufferCache[embeddedItem.pixelBufferId - 1u].pixelBuffer;
    }

    if(pixelBuffer)
    {
      // Set the size of the embedded item if it has not been set.
      if(0u == embeddedItem.width)
      {
        embeddedItem.width = static_cast<unsigned int>(pixelBuffer.GetWidth());
      }

      if(0u == embeddedItem.height)
      {
        embeddedItem.height = static_cast<unsigned int>(pixelBuffer.GetHeight());
      }

      // Set the pixel format.
      pixelFormat = pixelBuffer.GetPixelFormat();
    }
  }

  // Find if the same embeddedItem has already been created.
  GlyphIndex index = 0u;
  if(!mCacheHandler->FindEmbeddedItem(embeddedItem.pixelBufferId, embeddedItem.width, embeddedItem.height, index))
  {
    index = mCacheHandler->CacheEmbeddedItem(std::move(embeddedItem));
  }
  return index;
}

void FontClient::Plugin::EnableAtlasLimitation(bool enabled)
{
  mIsAtlasLimitationEnabled = enabled;
}

bool FontClient::Plugin::IsAtlasLimitationEnabled() const
{
  return mIsAtlasLimitationEnabled;
}

Size FontClient::Plugin::GetMaximumTextAtlasSize() const
{
  return TextAbstraction::FontClient::MAX_TEXT_ATLAS_SIZE;
}

Size FontClient::Plugin::GetDefaultTextAtlasSize() const
{
  return TextAbstraction::FontClient::DEFAULT_TEXT_ATLAS_SIZE;
}

Size FontClient::Plugin::GetCurrentMaximumBlockSizeFitInAtlas() const
{
  return mCurrentMaximumBlockSizeFitInAtlas;
}

bool FontClient::Plugin::SetCurrentMaximumBlockSizeFitInAtlas(const Size& currentMaximumBlockSizeFitInAtlas)
{
  bool            isChanged        = false;
  const Size&     maxTextAtlasSize = TextAbstraction::FontClient::MAX_TEXT_ATLAS_SIZE;
  const uint16_t& padding          = TextAbstraction::FontClient::PADDING_TEXT_ATLAS_BLOCK;

  if(currentMaximumBlockSizeFitInAtlas.width <= maxTextAtlasSize.width - padding && currentMaximumBlockSizeFitInAtlas.height <= maxTextAtlasSize.height - padding)
  {
    mCurrentMaximumBlockSizeFitInAtlas = currentMaximumBlockSizeFitInAtlas;
    isChanged                          = true;
  }

  return isChanged;
}

uint32_t FontClient::Plugin::GetNumberOfPointsPerOneUnitOfPointSize() const
{
  return TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
}

FontId FontClient::Plugin::CreateFont(const FontPath& path,
                                      PointSize26Dot6 requestedPointSize,
                                      FaceIndex       faceIndex,
                                      bool            cacheDescription,
                                      Property::Map*  variationsMapPtr) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "                path : [%s]\n", path.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);

  DALI_TRACE_SCOPE(gTraceFilter, "DALI_TEXT_CREATE_FONT");

  FontId   fontId = 0u;
  FT_Face  ftFace;
  FT_Error error;

  uint8_t*       fontDataPtr   = nullptr;
  std::streampos dataSize      = 0;
  bool           fontDataFound = mCacheHandler->FindFontData(path, fontDataPtr, dataSize);

  if(fontDataFound)
  {
    // Create & cache new font face from pre-loaded font
    error = FT_New_Memory_Face(mFreeTypeLibrary, reinterpret_cast<FT_Byte*>(fontDataPtr), static_cast<FT_Long>(dataSize), 0, &ftFace);
#if defined(TRACE_ENABLED)
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("DALI_TEXT_CREATE_FONT : FT_New_Memory_Face : %s\n", path.c_str());
    }
#endif
  }
  else
  {
    // Create & cache new font face
    error = FT_New_Face(mFreeTypeLibrary, path.c_str(), 0, &ftFace);
#if defined(TRACE_ENABLED)
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("DALI_TEXT_CREATE_FONT : FT_New_Face : %s\n", path.c_str());
    }
#endif
  }

  if(FT_Err_Ok == error)
  {
    // Check if a font is scalable.
    const bool isScalable           = (0 != (ftFace->face_flags & FT_FACE_FLAG_SCALABLE));
    const bool hasFixedSizedBitmaps = (0 != (ftFace->face_flags & FT_FACE_FLAG_FIXED_SIZES)) && (0 != ftFace->num_fixed_sizes);
    const bool hasColorTables       = (0 != (ftFace->face_flags & FT_FACE_FLAG_COLOR));

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "            isScalable : [%s]\n", (isScalable ? "true" : "false"));
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  hasFixedSizedBitmaps : [%s]\n", (hasFixedSizedBitmaps ? "true" : "false"));
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "        hasColorTables : [%s]\n", (hasColorTables ? "true" : "false"));

    // Set Variable axes if applicable.
    if(variationsMapPtr)
    {
      Property::Map& variationsMap = *variationsMapPtr;

      FT_MM_Var* mm_var;
      error = FT_Get_MM_Var(ftFace, &mm_var);
      if(FT_Err_Ok == error)
      {
        FT_Fixed *coordinates = new FT_Fixed[mm_var->num_axis];
        for(uint32_t axisIndex = 0; axisIndex < mm_var->num_axis; axisIndex++)
        {
          char stringTag[FONT_AXIS_NAME_LEN + 1];
          ConvertTagToString(mm_var->axis[axisIndex].tag, stringTag);
          auto valuePtr = variationsMap.Find(stringTag);
          float value   = 0.0f;

          if(valuePtr != nullptr && valuePtr->Get(value))
          {
            coordinates[axisIndex] = static_cast<FT_Fixed>(value * FROM_16DOT16);
          }
          else
          {
            // Set to default.
            coordinates[axisIndex] = static_cast<FT_Fixed>(mm_var->axis[axisIndex].def);
          }
        }

        FT_Set_Var_Design_Coordinates(ftFace, mm_var->num_axis, coordinates);
        delete[] coordinates;
      }
    }

    // Check to see if the font contains fixed sizes?
    if(!isScalable && hasFixedSizedBitmaps)
    {
      PointSize26Dot6 actualPointSize = 0u;
      int             fixedSizeIndex  = 0;
      for(; fixedSizeIndex < ftFace->num_fixed_sizes; ++fixedSizeIndex)
      {
        const PointSize26Dot6 fixedSize = static_cast<PointSize26Dot6>(ftFace->available_sizes[fixedSizeIndex].size);
        DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  size index : %d, size : %d\n", fixedSizeIndex, fixedSize);

        if(fixedSize >= requestedPointSize)
        {
          actualPointSize = fixedSize;
          break;
        }
      }

      if(0u == actualPointSize)
      {
        // The requested point size is bigger than the bigest fixed size.
        fixedSizeIndex  = ftFace->num_fixed_sizes - 1;
        actualPointSize = static_cast<PointSize26Dot6>(ftFace->available_sizes[fixedSizeIndex].size);
      }

      DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  size index : %d, actual size : %d\n", fixedSizeIndex, actualPointSize);

      // Tell Freetype to use this size
      error = FT_Select_Size(ftFace, fixedSizeIndex);
      if(FT_Err_Ok != error)
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FreeType Select_Size error: %d\n", error);
      }
      else
      {
        FT_Size_Metrics& ftMetrics = ftFace->size->metrics;

        FontMetrics metrics(static_cast<float>(ftMetrics.ascender) * FROM_266,
                            static_cast<float>(ftMetrics.descender) * FROM_266,
                            static_cast<float>(ftMetrics.height) * FROM_266,
                            static_cast<float>(ftFace->underline_position) * FROM_266,
                            static_cast<float>(ftFace->underline_thickness) * FROM_266);

        const float fixedWidth  = static_cast<float>(ftFace->available_sizes[fixedSizeIndex].width);
        const float fixedHeight = static_cast<float>(ftFace->available_sizes[fixedSizeIndex].height);

        // Create the FreeType font face item to cache.
        FontFaceCacheItem fontFaceCacheItem(mFreeTypeLibrary, ftFace, mCacheHandler->GetGlyphCacheManager(), path, requestedPointSize, faceIndex, metrics, fixedSizeIndex, fixedWidth, fixedHeight, hasColorTables, (variationsMapPtr ? variationsMapPtr->GetHash() : 0u));

        fontId = mCacheHandler->CacheFontFaceCacheItem(std::move(fontFaceCacheItem));
      }
    }
    else
    {
      if(mIsAtlasLimitationEnabled)
      {
        //There is limitation on block size to fit in predefined atlas size.
        //If the block size cannot fit into atlas size, then the system cannot draw block.
        //This is workaround to avoid issue in advance
        //Decrementing point-size until arriving to maximum allowed block size.
        auto        requestedPointSizeBackup = requestedPointSize;
        const Size& maxSizeFitInAtlas        = GetCurrentMaximumBlockSizeFitInAtlas();
        error                                = SearchOnProperPointSize(ftFace, mDpiHorizontal, mDpiVertical, maxSizeFitInAtlas, requestedPointSize);

        if(requestedPointSize != requestedPointSizeBackup)
        {
          DALI_LOG_WARNING(" The requested-point-size : %d, is reduced to point-size : %d\n", requestedPointSizeBackup, requestedPointSize);
        }
      }
      else
      {
        error = FT_Set_Char_Size(ftFace,
                                 0,
                                 FT_F26Dot6(requestedPointSize),
                                 mDpiHorizontal,
                                 mDpiVertical);
      }

      if(FT_Err_Ok == error)
      {
        FT_Size_Metrics& ftMetrics = ftFace->size->metrics;

        FontMetrics metrics(static_cast<float>(ftMetrics.ascender) * FROM_266,
                            static_cast<float>(ftMetrics.descender) * FROM_266,
                            static_cast<float>(ftMetrics.height) * FROM_266,
                            static_cast<float>(ftFace->underline_position) * FROM_266,
                            static_cast<float>(ftFace->underline_thickness) * FROM_266);

        // Create the FreeType font face item to cache.
        FontFaceCacheItem fontFaceCacheItem(mFreeTypeLibrary, ftFace, mCacheHandler->GetGlyphCacheManager(), path, requestedPointSize, faceIndex, metrics);

        fontId = mCacheHandler->CacheFontFaceCacheItem(std::move(fontFaceCacheItem));
      }
      else
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  FreeType Set_Char_Size error: %d for pointSize %d\n", error, requestedPointSize);
      }
    }

    if(0u != fontId)
    {
      if(cacheDescription)
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  Cache Font Path at font id : %d [%s]\n", fontId, path.c_str());
        mCacheHandler->CacheFontPath(ftFace, fontId, requestedPointSize, variationsMapPtr, path);
      }
    }
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  FreeType New_Face error: %d for [%s]\n", error, path.c_str());
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

bool FontClient::Plugin::IsScalable(const FontPath& path) const
{
  bool isScalable = false;

  FT_Face ftFace = nullptr;
  int     error  = FT_New_Face(mFreeTypeLibrary,
                          path.c_str(),
                          0,
                          &ftFace);
  if(FT_Err_Ok != error)
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::IsScalable. FreeType Cannot check font: %s\n", path.c_str());
  }
  else
  {
    isScalable = ftFace->face_flags & FT_FACE_FLAG_SCALABLE;
  }

  if(ftFace)
  {
    FT_Done_Face(ftFace);
  }

  return isScalable;
}

bool FontClient::Plugin::IsScalable(const FontDescription& fontDescription) const
{
  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern(mCacheHandler->mFontConfig, fontDescription); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // match the pattern
  FcPattern* match      = FcFontMatch(mCacheHandler->mFontConfig, fontFamilyPattern, &result); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.
  bool       isScalable = false;

  if(match)
  {
    // Get the path to the font file name.
    FontPath path;
    GetFcString(match, FC_FILE, path);
    isScalable = IsScalable(path);
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::IsScalable. FreeType Cannot check font: [%s]\n", fontDescription.family.c_str());
  }

  // Destroys the created patterns.
  FcPatternDestroy(match);
  FcPatternDestroy(fontFamilyPattern);

  return isScalable;
}

void FontClient::Plugin::GetFixedSizes(const FontPath& path, Vector<PointSize26Dot6>& sizes) const
{
  // Empty the caller container
  sizes.Clear();

  FT_Face ftFace = nullptr;
  int     error  = FT_New_Face(mFreeTypeLibrary,
                          path.c_str(),
                          0,
                          &ftFace);
  if(FT_Err_Ok != error)
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetFixedSizes. FreeType Cannot check font path : [%s]\n", path.c_str());
  }

  if(ftFace)
  {
    // Fetch the number of fixed sizes available
    if(ftFace->num_fixed_sizes && ftFace->available_sizes)
    {
      for(int i = 0; i < ftFace->num_fixed_sizes; ++i)
      {
        sizes.PushBack(ftFace->available_sizes[i].size);
      }
    }

    FT_Done_Face(ftFace);
  }
}

void FontClient::Plugin::GetFixedSizes(const FontDescription&   fontDescription,
                                       Vector<PointSize26Dot6>& sizes) const
{
  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern(mCacheHandler->mFontConfig, fontDescription); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // match the pattern
  FcPattern* match = FcFontMatch(mCacheHandler->mFontConfig, fontFamilyPattern, &result); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  if(match)
  {
    // Get the path to the font file name.
    FontPath path;
    GetFcString(match, FC_FILE, path);
    GetFixedSizes(path, sizes);
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetFixedSizes. FreeType Cannot check font: [%s]\n", fontDescription.family.c_str());
  }

  // Destroys the created patterns.
  FcPatternDestroy(match);
  FcPatternDestroy(fontFamilyPattern);
}

bool FontClient::Plugin::HasItalicStyle(FontId fontId) const
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    return fontCacheItem->HasItalicStyle();
  }
  return false;
}

} // namespace Dali::TextAbstraction::Internal
