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
 *
 */

// CLASS HEADER
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-list.h>

#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/platform-abstraction.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/text/text-abstraction/plugin/bitmap-font-cache-item.h>
#include <dali/internal/text/text-abstraction/plugin/embedded-item.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-cache-item.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>

// EXTERNAL INCLUDES
#include <fontconfig/fontconfig.h>
#include <algorithm>
#include <iterator>

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
/**
 * Conversion from Fractional26.6 to float
 */
const float FROM_266        = 1.0f / 64.0f;
const float POINTS_PER_INCH = 72.f;

const std::string DEFAULT_FONT_FAMILY_NAME("Tizen");

const uint32_t ELLIPSIS_CHARACTER = 0x2026;

} // namespace

using Dali::Vector;
using namespace std;

namespace Dali::TextAbstraction::Internal
{
/**
 * @brief Free the resources allocated by the FcCharSet objects.
 *
 * @param[in] characterSets The vector of character sets.
 */
void DestroyCharacterSets(CharacterSetList& characterSets)
{
  for(auto& item : characterSets)
  {
    if(item)
    {
      FcCharSetDestroy(item);
    }
  }
}

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
                           requestedPointSize,
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

FontClient::Plugin::FallbackCacheItem::FallbackCacheItem(FontDescription&& font, FontList* fallbackFonts, CharacterSetList* characterSets)
: fontDescription{std::move(font)},
  fallbackFonts{fallbackFonts},
  characterSets{characterSets}
{
}

FontClient::Plugin::FontDescriptionCacheItem::FontDescriptionCacheItem(const FontDescription& fontDescription,
                                                                       FontDescriptionId      index)
: fontDescription{fontDescription},
  index{index}
{
}

FontClient::Plugin::FontDescriptionCacheItem::FontDescriptionCacheItem(FontDescription&& fontDescription,
                                                                       FontDescriptionId index)
: fontDescription{std::move(fontDescription)},
  index{index}
{
}

FontClient::Plugin::FontDescriptionSizeCacheKey::FontDescriptionSizeCacheKey(FontDescriptionId fontDescriptionId,
                                                                             PointSize26Dot6   requestedPointSize)
: fontDescriptionId(fontDescriptionId),
  requestedPointSize(requestedPointSize)
{
}

FontClient::Plugin::Plugin(unsigned int horizontalDpi,
                           unsigned int verticalDpi)
: mFreeTypeLibrary(nullptr),
  mDpiHorizontal(horizontalDpi),
  mDpiVertical(verticalDpi),
  mDefaultFontDescription(),
  mSystemFonts(),
  mDefaultFonts(),
  mFontIdCache(),
  mFontFaceCache(),
  mValidatedFontCache(),
  mFontDescriptionCache(),
  mCharacterSetCache(),
  mFontDescriptionSizeCache(),
  mVectorFontCache(nullptr),
  mEllipsisCache(),
  mEmbeddedItemCache(),
  mDefaultFontDescriptionCached(false),
  mIsAtlasLimitationEnabled(TextAbstraction::FontClient::DEFAULT_ATLAS_LIMITATION_ENABLED),
  mCurrentMaximumBlockSizeFitInAtlas(TextAbstraction::FontClient::MAX_SIZE_FIT_IN_ATLAS)

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
  ClearFallbackCache(mFallbackCache);

  // Free the resources allocated by the FcCharSet objects.
  DestroyCharacterSets(mDefaultFontCharacterSets);
  DestroyCharacterSets(mCharacterSetCache);
  ClearCharacterSetFromFontFaceCache();

  // Clear FontFaceCache here. Due to we sould deallocate FT_Faces before done freetype library
  mFontFaceCache.clear();

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  delete mVectorFontCache;
#endif
  FT_Done_FreeType(mFreeTypeLibrary);
}

void FontClient::Plugin::ClearCache()
{
  mDefaultFontDescription = FontDescription();

  mSystemFonts.clear();
  mDefaultFonts.clear();

  DestroyCharacterSets(mDefaultFontCharacterSets);
  mDefaultFontCharacterSets.Clear();

  ClearFallbackCache(mFallbackCache);
  mFallbackCache.clear();

  mFontIdCache.Clear();

  ClearCharacterSetFromFontFaceCache();
  mFontFaceCache.clear();

  mValidatedFontCache.clear();
  mFontDescriptionCache.clear();

  DestroyCharacterSets(mCharacterSetCache);
  mCharacterSetCache.Clear();

  mFontDescriptionSizeCache.clear();
  mFontDescriptionSizeCache.rehash(0); // Note : unordered_map.clear() didn't deallocate memory

  mEllipsisCache.Clear();
  mPixelBufferCache.clear();
  mEmbeddedItemCache.Clear();
  mBitmapFontCache.clear();

  mDefaultFontDescriptionCached = false;
}

void FontClient::Plugin::SetDpi(unsigned int horizontalDpi,
                                unsigned int verticalDpi)
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical   = verticalDpi;
}

void FontClient::Plugin::ResetSystemDefaults()
{
  mDefaultFontDescriptionCached = false;
}

void FontClient::Plugin::SetFontList(const FontDescription& fontDescription, FontList& fontList, CharacterSetList& characterSetList)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_DESCRIPTION(fontDescription, "");
  fontList.clear();

  FcPattern* fontFamilyPattern = CreateFontFamilyPattern(fontDescription); // Creates a pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // Match the pattern.
  FcFontSet* fontSet = FcFontSort(nullptr /* use default configure */,
                                  fontFamilyPattern,
                                  false /* don't trim */,
                                  nullptr,
                                  &result); // FcFontSort creates a font set that needs to be destroyed by calling FcFontSetDestroy.

  if(nullptr != fontSet)
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of fonts found : [%d]\n", fontSet->nfont);
    // Reserve some space to avoid reallocations.
    fontList.reserve(fontSet->nfont);

    for(int i = 0u; i < fontSet->nfont; ++i)
    {
      FcPattern* fontPattern = fontSet->fonts[i];

      FontPath path;

      // Skip fonts with no path
      if(GetFcString(fontPattern, FC_FILE, path))
      {
        // Retrieve the character set. Need to call FcCharSetDestroy to free the resources.
        FcCharSet* characterSet = nullptr;
        FcPatternGetCharSet(fontPattern, FC_CHARSET, 0u, &characterSet);

        // Increase the reference counter of the character set.
        characterSetList.PushBack(FcCharSetCopy(characterSet));

        fontList.push_back(FontDescription());
        FontDescription& newFontDescription = fontList.back();

        newFontDescription.path = std::move(path);

        int width  = 0;
        int weight = 0;
        int slant  = 0;
        GetFcString(fontPattern, FC_FAMILY, newFontDescription.family);
        GetFcInt(fontPattern, FC_WIDTH, width);
        GetFcInt(fontPattern, FC_WEIGHT, weight);
        GetFcInt(fontPattern, FC_SLANT, slant);
        newFontDescription.width  = IntToWidthType(width);
        newFontDescription.weight = IntToWeightType(weight);
        newFontDescription.slant  = IntToSlantType(slant);

        FONT_LOG_DESCRIPTION(newFontDescription, "");
      }
    }

    // Destroys the font set created by FcFontSort.
    FcFontSetDestroy(fontSet);
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  No fonts found.\n");
  }

  // Destroys the pattern created by FcPatternCreate in CreateFontFamilyPattern.
  FcPatternDestroy(fontFamilyPattern);
}

void FontClient::Plugin::GetDefaultFonts(FontList& defaultFonts)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  if(mDefaultFonts.empty())
  {
    FontDescription fontDescription;
    fontDescription.family = DEFAULT_FONT_FAMILY_NAME; // todo This could be set to the Platform font
    fontDescription.width  = DefaultFontWidth();
    fontDescription.weight = DefaultFontWeight();
    fontDescription.slant  = DefaultFontSlant();
    SetFontList(fontDescription, mDefaultFonts, mDefaultFontCharacterSets);
  }

  defaultFonts = mDefaultFonts;

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of default fonts : [%d]\n", mDefaultFonts.size());
}

void FontClient::Plugin::GetDefaultPlatformFontDescription(FontDescription& fontDescription)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  if(!mDefaultFontDescriptionCached)
  {
    // Clear any font config stored info in the caches.

    // Decrease the reference counter and eventually free the resources allocated by FcCharSet objects.
    DestroyCharacterSets(mDefaultFontCharacterSets);
    DestroyCharacterSets(mCharacterSetCache);
    mDefaultFontCharacterSets.Clear();
    mCharacterSetCache.Clear();

    for(auto& item : mFallbackCache)
    {
      // Decrease the reference counter and eventually free the resources allocated by FcCharSet objects.
      DestroyCharacterSets(*item.characterSets);

      delete item.characterSets;
      item.characterSets = nullptr;
    }

    // Set the character set pointer as null. Will be created again the next time IsCharacterSupportedByFont()
    ClearCharacterSetFromFontFaceCache();

    // FcInitBringUptoDate did not seem to reload config file as was still getting old default font.
    FcInitReinitialize();

    FcPattern* matchPattern = FcPatternCreate(); // Creates a pattern that needs to be destroyed by calling FcPatternDestroy.

    if(nullptr != matchPattern)
    {
      FcConfigSubstitute(nullptr, matchPattern, FcMatchPattern);
      FcDefaultSubstitute(matchPattern);

      FcCharSet* characterSet = nullptr;
      bool       matched      = MatchFontDescriptionToPattern(matchPattern, mDefaultFontDescription, &characterSet);

      // Caching the default font description
      if(matched)
      {
        // Copy default font description info.
        // Due to the type changed, we need to make some temperal font description.
        FontDescription tempFontDescription = mDefaultFontDescription;

        // Add the path to the cache.
        tempFontDescription.type = FontDescription::FACE_FONT;
        mFontDescriptionCache.push_back(tempFontDescription);

        // Set the index to the vector of paths to font file names.
        const FontDescriptionId fontDescriptionId = mFontDescriptionCache.size();

        FONT_LOG_DESCRIPTION(tempFontDescription, "default platform font");
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  default font fontDescriptionId : %d\n", fontDescriptionId);

        // Cache the index and the matched font's description.
        FontDescriptionCacheItem item(tempFontDescription,
                                      fontDescriptionId);

        mValidatedFontCache.push_back(std::move(item));
      }
      else
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  default font validation failed for font [%s]\n", mDefaultFontDescription.family.c_str());
      }

      // Decrease the reference counter of the character set as it's not stored.
      // Note. the cached default font description will increase reference counter by
      // mFontDescriptionCache. So we can decrease reference counter here.
      FcCharSetDestroy(characterSet);

      // Destroys the pattern created.
      FcPatternDestroy(matchPattern);
    }

    // Create again the character sets as they are not valid after FcInitReinitialize()

    for(const auto& description : mDefaultFonts)
    {
      mDefaultFontCharacterSets.PushBack(FcCharSetCopy(CreateCharacterSetFromDescription(description)));
    }

    for(const auto& description : mFontDescriptionCache)
    {
      mCharacterSetCache.PushBack(FcCharSetCopy(CreateCharacterSetFromDescription(description)));
    }

    for(auto& item : mFallbackCache)
    {
      if(nullptr != item.fallbackFonts)
      {
        if(nullptr == item.characterSets)
        {
          item.characterSets = new CharacterSetList;
        }

        for(const auto& description : *(item.fallbackFonts))
        {
          item.characterSets->PushBack(FcCharSetCopy(CreateCharacterSetFromDescription(description)));
        }
      }
    }

    mDefaultFontDescriptionCached = true;
  }

  fontDescription.path   = mDefaultFontDescription.path;
  fontDescription.family = mDefaultFontDescription.family;
  fontDescription.width  = mDefaultFontDescription.width;
  fontDescription.weight = mDefaultFontDescription.weight;
  fontDescription.slant  = mDefaultFontDescription.slant;

  FONT_LOG_DESCRIPTION(fontDescription, "");
}

void FontClient::Plugin::GetSystemFonts(FontList& systemFonts)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  if(mSystemFonts.empty())
  {
    InitSystemFonts();
  }

  systemFonts = mSystemFonts;
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of system fonts : [%d]\n", mSystemFonts.size());
}

void FontClient::Plugin::GetDescription(FontId           id,
                                        FontDescription& fontDescription) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", id);
  const FontId index = id - 1u;

  if((id > 0u) && (index < mFontIdCache.Count()))
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];
    switch(fontIdCacheItem.type)
    {
      case FontDescription::FACE_FONT:
      {
        for(const auto& item : mFontDescriptionSizeCache)
        {
          if(item.second == fontIdCacheItem.index)
          {
            fontDescription = *(mFontDescriptionCache.begin() + item.first.fontDescriptionId - 1u);

            FONT_LOG_DESCRIPTION(fontDescription, "");
            return;
          }
        }
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        fontDescription.type   = FontDescription::BITMAP_FONT;
        fontDescription.family = mBitmapFontCache[fontIdCacheItem.index].font.name;
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

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  No description found for the font ID %d\n", id);
}

PointSize26Dot6 FontClient::Plugin::GetPointSize(FontId id)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", id);

  PointSize26Dot6               pointSize     = TextAbstraction::FontClient::DEFAULT_POINT_SIZE;
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(id);
  if(fontCacheItem != nullptr)
  {
    pointSize = fontCacheItem->GetPointSize();
  }
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  point size : %d\n", pointSize);

  return pointSize;
}

bool FontClient::Plugin::IsCharacterSupportedByFont(FontId fontId, Character character)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "    font id : %d\n", fontId);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  character : %p\n", character);

  bool isSupported   = false;
  auto fontCacheItem = const_cast<FontCacheItemInterface*>(GetCachedFontItem(fontId));
  if(fontCacheItem != nullptr)
  {
    isSupported = fontCacheItem->IsCharacterSupported(character); // May cache
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  is supported : %s\n", (isSupported ? "true" : "false"));
  return isSupported;
}

const FontCacheItemInterface* FontClient::Plugin::GetCachedFontItem(FontId id) const
{
  const FontCacheIndex index = id - 1u;
  if((id > 0u) && (index < mFontIdCache.Count()))
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];
    switch(fontIdCacheItem.type)
    {
      case FontDescription::FACE_FONT:
      {
        return &mFontFaceCache[fontIdCacheItem.index];
      }
      case FontDescription::BITMAP_FONT:
      {
        return &mBitmapFontCache[fontIdCacheItem.index];
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
                                                bool                    preferColor)
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
      fontId = GetFontId(description, requestedPointSize, 0u);

      DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "     font id : %d\n", fontId);

      if(preferColor)
      {
        if((fontId > 0) &&
           (fontId - 1u < mFontIdCache.Count()))
        {
          const FontFaceCacheItem& item = mFontFaceCache[mFontIdCache[fontId - 1u].index];

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

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::FindDefaultFont(Character       charcode,
                                           PointSize26Dot6 requestedPointSize,
                                           bool            preferColor)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_REQUEST(charcode, requestedPointSize, preferColor);

  FontId fontId(0);

  // Create the list of default fonts if it has not been created.
  if(mDefaultFonts.empty())
  {
    FontDescription fontDescription;
    fontDescription.family = DEFAULT_FONT_FAMILY_NAME;
    fontDescription.width  = DefaultFontWidth();
    fontDescription.weight = DefaultFontWeight();
    fontDescription.slant  = DefaultFontSlant();

    SetFontList(fontDescription, mDefaultFonts, mDefaultFontCharacterSets);
  }
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of default fonts : %d\n", mDefaultFonts.size());

  // Traverse the list of default fonts.
  // Check for each default font if supports the character.
  fontId = FindFontForCharacter(mDefaultFonts, mDefaultFontCharacterSets, charcode, requestedPointSize, preferColor);

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::FindFallbackFont(Character              charcode,
                                            const FontDescription& preferredFontDescription,
                                            PointSize26Dot6        requestedPointSize,
                                            bool                   preferColor)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_REQUEST(charcode, requestedPointSize, preferColor);

  // The font id to be returned.
  FontId fontId = 0u;

  FontDescription fontDescription;

  // Fill the font description with the preferred font description and complete with the defaults.
  fontDescription.family = preferredFontDescription.family.empty() ? DEFAULT_FONT_FAMILY_NAME : preferredFontDescription.family;
  fontDescription.weight = ((FontWeight::NONE == preferredFontDescription.weight) ? DefaultFontWeight() : preferredFontDescription.weight);
  fontDescription.width  = ((FontWidth::NONE == preferredFontDescription.width) ? DefaultFontWidth() : preferredFontDescription.width);
  fontDescription.slant  = ((FontSlant::NONE == preferredFontDescription.slant) ? DefaultFontSlant() : preferredFontDescription.slant);

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  preferredFontDescription --> fontDescription\n");
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  [%s] --> [%s]\n", preferredFontDescription.family.c_str(), fontDescription.family.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontWeight::Name[preferredFontDescription.weight], FontWeight::Name[fontDescription.weight]);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontWidth::Name[preferredFontDescription.width], FontWidth::Name[fontDescription.width]);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontSlant::Name[preferredFontDescription.slant], FontSlant::Name[fontDescription.slant]);

  // Check first if the font's description has been queried before.
  FontList*         fontList         = nullptr;
  CharacterSetList* characterSetList = nullptr;

  if(!FindFallbackFontList(fontDescription, fontList, characterSetList))
  {
    fontList         = new FontList;
    characterSetList = new CharacterSetList;

    SetFontList(fontDescription, *fontList, *characterSetList);
#ifdef __APPLE__
    FontDescription appleColorEmoji;
    appleColorEmoji.family = "Apple Color Emoji";
    appleColorEmoji.width  = fontDescription.width;
    appleColorEmoji.weight = fontDescription.weight;
    appleColorEmoji.slant  = fontDescription.slant;
    FontList         emojiFontList;
    CharacterSetList emojiCharSetList;
    SetFontList(appleColorEmoji, emojiFontList, emojiCharSetList);

    std::move(fontList->begin(), fontList->end(), std::back_inserter(emojiFontList));
    emojiCharSetList.Insert(emojiCharSetList.End(), characterSetList->Begin(), characterSetList->End());
    *fontList         = std::move(emojiFontList);
    *characterSetList = std::move(emojiCharSetList);
#endif

    // Add the font-list to the cache.
    mFallbackCache.push_back(std::move(FallbackCacheItem(std::move(fontDescription), fontList, characterSetList)));
  }

  if(fontList && characterSetList)
  {
    fontId = FindFontForCharacter(*fontList, *characterSetList, charcode, requestedPointSize, preferColor);
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::GetFontId(const FontPath& path,
                                     PointSize26Dot6 requestedPointSize,
                                     FaceIndex       faceIndex,
                                     bool            cacheDescription)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "                path : [%s]\n", path.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);

  FontId id = 0u;

  if(nullptr != mFreeTypeLibrary)
  {
    FontId foundId = 0u;
    if(FindFont(path, requestedPointSize, faceIndex, foundId))
    {
      id = foundId;
    }
    else
    {
      id = CreateFont(path, requestedPointSize, faceIndex, cacheDescription);
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", id);
  return id;
}

FontId FontClient::Plugin::GetFontId(const FontDescription& fontDescription,
                                     PointSize26Dot6        requestedPointSize,
                                     FaceIndex              faceIndex)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_DESCRIPTION(fontDescription, "");

  // Special case when font Description don't have family information.
  // In this case, we just use default description family and path.
  const FontDescription& realFontDescription = fontDescription.family.empty() ? FontDescription(mDefaultFontDescription.path,
                                                                                                mDefaultFontDescription.family,
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
  if(FindBitmapFont(realFontDescription.family, fontId))
  {
    return fontId;
  }

  // Check if the font's description have been validated before.
  FontDescriptionId fontDescriptionId = 0u;

  if(!FindValidatedFont(realFontDescription,
                        fontDescriptionId))
  {
    // Use font config to validate the font's description.
    ValidateFont(realFontDescription,
                 fontDescriptionId);
  }

  FontCacheIndex fontCacheIndex = 0u;
  // Check if exists a pair 'fontDescriptionId, requestedPointSize' in the cache.
  if(!FindFont(fontDescriptionId, requestedPointSize, fontCacheIndex))
  {
    // Retrieve the font file name path.
    const FontDescription& description = *(mFontDescriptionCache.begin() + fontDescriptionId - 1u);

    // Retrieve the font id. Do not cache the description as it has been already cached.
    fontId = GetFontId(description.path,
                       requestedPointSize,
                       faceIndex,
                       false);

    fontCacheIndex                               = mFontIdCache[fontId - 1u].index;
    mFontFaceCache[fontCacheIndex].mCharacterSet = FcCharSetCopy(mCharacterSetCache[fontDescriptionId - 1u]);

    // Cache the pair 'fontDescriptionId, requestedPointSize' to improve the following queries.
    mFontDescriptionSizeCache.emplace(FontDescriptionSizeCacheKey(fontDescriptionId, requestedPointSize), fontCacheIndex);
  }
  else
  {
    fontId = mFontFaceCache[fontCacheIndex].mFontId + 1u;
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", fontId);
  return fontId;
}

FontId FontClient::Plugin::GetFontId(const BitmapFont& bitmapFont)
{
  for(const auto& item : mBitmapFontCache)
  {
    if(bitmapFont.name == item.font.name)
    {
      return item.id + 1u;
    }
  }

  BitmapFontCacheItem bitmapFontCacheItem(bitmapFont, mFontIdCache.Count());

  FontIdCacheItem fontIdCacheItem;
  fontIdCacheItem.type  = FontDescription::BITMAP_FONT;
  fontIdCacheItem.index = mBitmapFontCache.size();

  mBitmapFontCache.push_back(std::move(bitmapFontCacheItem));
  mFontIdCache.PushBack(fontIdCacheItem);

  return bitmapFontCacheItem.id + 1u;
}

void FontClient::Plugin::ValidateFont(const FontDescription& fontDescription,
                                      FontDescriptionId&     fontDescriptionId)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_DESCRIPTION(fontDescription, "");

  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern(fontDescription);

  FontDescription description;

  FcCharSet* characterSet = nullptr;
  bool       matched      = MatchFontDescriptionToPattern(fontFamilyPattern, description, &characterSet);
  FcPatternDestroy(fontFamilyPattern);

  if(matched && (nullptr != characterSet))
  {
    // Add the path to the cache.
    description.type = FontDescription::FACE_FONT;
    mFontDescriptionCache.push_back(description);

    // Set the index to the vector of paths to font file names.
    fontDescriptionId = mFontDescriptionCache.size();

    FONT_LOG_DESCRIPTION(description, "matched");
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  fontDescriptionId : %d\n", fontDescriptionId);

    // The reference counter of the character set has already been increased in MatchFontDescriptionToPattern.
    mCharacterSetCache.PushBack(characterSet);

    // Cache the index and the matched font's description.
    FontDescriptionCacheItem item(description,
                                  fontDescriptionId);

    mValidatedFontCache.push_back(std::move(item));

    if((fontDescription.family != description.family) ||
       (fontDescription.width != description.width) ||
       (fontDescription.weight != description.weight) ||
       (fontDescription.slant != description.slant))
    {
      // Cache the given font's description if it's different than the matched.
      FontDescriptionCacheItem item(fontDescription,
                                    fontDescriptionId);

      mValidatedFontCache.push_back(std::move(item));
    }
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font validation failed for font [%s]\n", fontDescription.family.c_str());
  }
}

void FontClient::Plugin::GetFontMetrics(FontId       fontId,
                                        FontMetrics& metrics)
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    fontCacheItem->GetFontMetrics(metrics, mDpiVertical);
  }
}

GlyphIndex FontClient::Plugin::GetGlyphIndex(FontId    fontId,
                                             Character charcode)
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
                                             Character variantSelector)
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
                                         bool       horizontal)
{
  if(VECTOR_GLYPH == type)
  {
    return GetVectorMetrics(array, size, horizontal);
  }

  return GetBitmapMetrics(array, size, horizontal);
}

bool FontClient::Plugin::GetBitmapMetrics(GlyphInfo* array,
                                          uint32_t   size,
                                          bool       horizontal)
{
  bool success(false);

  for(unsigned int i = 0; i < size; ++i)
  {
    GlyphInfo& glyph = array[i];

    const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(glyph.fontId);
    if(fontCacheItem != nullptr)
    {
      success = fontCacheItem->GetGlyphMetrics(glyph, mDpiVertical, horizontal);
    }
    // Check if it's an embedded image.
    else if((0u == glyph.fontId) && (0u != glyph.index) && (glyph.index <= mEmbeddedItemCache.Count()))
    {
      mEmbeddedItemCache[glyph.index - 1u].GetGlyphMetrics(glyph);
      success = true;
    }
  }

  return success;
}

bool FontClient::Plugin::GetVectorMetrics(GlyphInfo* array,
                                          uint32_t   size,
                                          bool       horizontal)
{
#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  bool success(true);

  for(unsigned int i = 0u; i < size; ++i)
  {
    FontId fontId = array[i].fontId;

    if((fontId > 0u) &&
       (fontId - 1u) < mFontIdCache.Count())
    {
      FontFaceCacheItem& font = mFontFaceCache[mFontIdCache[fontId - 1u].index];

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

void FontClient::Plugin::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth)
{
  data.isColorBitmap                          = false;
  data.isColorEmoji                           = false;
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    fontCacheItem->CreateBitmap(glyphIndex, data, outlineWidth, isItalicRequired, isBoldRequired);
  }
  else if((0u != glyphIndex) && (glyphIndex <= mEmbeddedItemCache.Count()))
  {
    // It's an embedded item.
    mEmbeddedItemCache[glyphIndex - 1u].CreateBitmap(mPixelBufferCache, data);
  }
}

PixelData FontClient::Plugin::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth)
{
  TextAbstraction::FontClient::GlyphBufferData data;

  CreateBitmap(fontId, glyphIndex, false, false, data, outlineWidth);

  return PixelData::New(data.buffer,
                        data.width * data.height * Pixel::GetBytesPerPixel(data.format),
                        data.width,
                        data.height,
                        data.format,
                        PixelData::FREE);
}

void FontClient::Plugin::CreateVectorBlob(FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight)
{
  blob       = nullptr;
  blobLength = 0;

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  if((fontId > 0u) &&
     (fontId - 1u < mFontIdCache.Count()))
  {
    const FontCacheIndex fontFaceId = mFontIdCache[fontId - 1u].index;
    FontFaceCacheItem&   font       = mFontFaceCache[fontFaceId];

    if(!font.mVectorFontId)
    {
      font.mVectorFontId = mVectorFontCache->GetFontId(font.mPath);
    }

    mVectorFontCache->GetVectorBlob(font.mVectorFontId, fontFaceId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight);
  }
#endif
}

const GlyphInfo& FontClient::Plugin::GetEllipsisGlyph(PointSize26Dot6 requestedPointSize)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize %d.\n", requestedPointSize);

  // First look into the cache if there is an ellipsis glyph for the requested point size.
  for(const auto& item : mEllipsisCache)
  {
    if(item.requestedPointSize == requestedPointSize)
    {
      // Use the glyph in the cache.
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  glyph id %d found in the cache.\n", item.glyph.index);
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "      font %d.\n", item.glyph.fontId);
      return item.glyph;
    }
  }

  // No glyph has been found. Create one.
  mEllipsisCache.PushBack(EllipsisItem());
  EllipsisItem& item = *(mEllipsisCache.End() - 1u);

  item.requestedPointSize = requestedPointSize;

  // Find a font for the ellipsis glyph.
  item.glyph.fontId = FindDefaultFont(ELLIPSIS_CHARACTER,
                                      requestedPointSize,
                                      false);

  // Set the character index to access the glyph inside the font.
  item.glyph.index = FT_Get_Char_Index(mFontFaceCache[mFontIdCache[item.glyph.fontId - 1u].index].mFreeTypeFace,
                                       ELLIPSIS_CHARACTER);

  GetBitmapMetrics(&item.glyph, 1u, true);

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  glyph id %d found in the cache.\n", item.glyph.index);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "      font %d.\n", item.glyph.fontId);
  return item.glyph;
}

bool FontClient::Plugin::IsColorGlyph(FontId fontId, GlyphIndex glyphIndex)
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  return fontCacheItem && fontCacheItem->IsColorGlyph(glyphIndex);
}

FT_FaceRec_* FontClient::Plugin::GetFreetypeFace(FontId fontId)
{
  const FontCacheItemInterface* fontCacheItem = GetCachedFontItem(fontId);
  if(fontCacheItem != nullptr)
  {
    return fontCacheItem->GetTypeface();
  }
  return nullptr;
}

FontDescription::Type FontClient::Plugin::GetFontType(FontId fontId)
{
  const FontId index = fontId - 1u;
  if((fontId > 0u) &&
     (index < mFontIdCache.Count()))
  {
    return mFontIdCache[index].type;
  }
  return FontDescription::INVALID;
}

bool FontClient::Plugin::AddCustomFontDirectory(const FontPath& path)
{
  // nullptr as first parameter means the current configuration is used.
  return FcConfigAppFontAddDir(nullptr, reinterpret_cast<const FcChar8*>(path.c_str()));
}

GlyphIndex FontClient::Plugin::CreateEmbeddedItem(const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat)
{
  EmbeddedItem embeddedItem;

  embeddedItem.pixelBufferId = 0u;
  embeddedItem.width         = description.width;
  embeddedItem.height        = description.height;

  pixelFormat = Pixel::A8;

  if(!description.url.empty())
  {
    // Check if the url is in the cache.
    PixelBufferId index = 0u;

    for(const auto& cacheItem : mPixelBufferCache)
    {
      ++index;
      if(cacheItem.url == description.url)
      {
        // The url is in the pixel buffer cache.
        // Set the index +1 to the vector.
        embeddedItem.pixelBufferId = index;
        break;
      }
    }

    Devel::PixelBuffer pixelBuffer;
    if(0u == embeddedItem.pixelBufferId)
    {
      // The pixel buffer is not in the cache. Create one and cache it.

      // Load the image from the url.
      pixelBuffer = LoadImageFromFile(description.url);

      // Create the cache item.
      PixelBufferCacheItem pixelBufferCacheItem;
      pixelBufferCacheItem.pixelBuffer = pixelBuffer;
      pixelBufferCacheItem.url         = description.url;

      // Store the cache item in the cache.
      mPixelBufferCache.push_back(std::move(pixelBufferCacheItem));

      // Set the pixel buffer id to the embedded item.
      embeddedItem.pixelBufferId = mPixelBufferCache.size();
    }
    else
    {
      // Retrieve the pixel buffer from the cache to set the pixel format.
      pixelBuffer = mPixelBufferCache[embeddedItem.pixelBufferId - 1u].pixelBuffer;
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
  unsigned int index = 0u;
  for(const auto& item : mEmbeddedItemCache)
  {
    ++index;
    if((item.pixelBufferId == embeddedItem.pixelBufferId) &&
       (item.width == embeddedItem.width) &&
       (item.height == embeddedItem.height))
    {
      return index;
    }
  }

  // Cache the embedded item.
  mEmbeddedItemCache.PushBack(embeddedItem);

  return mEmbeddedItemCache.Count();
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
  ;
}

void FontClient::Plugin::InitSystemFonts()
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  FcFontSet* fontSet = GetFcFontSet(); // Creates a FcFontSet that needs to be destroyed by calling FcFontSetDestroy.

  if(fontSet)
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  number of system fonts : %d\n", fontSet->nfont);

    // Reserve some space to avoid reallocations.
    mSystemFonts.reserve(fontSet->nfont);

    for(int i = 0u; i < fontSet->nfont; ++i)
    {
      FcPattern* fontPattern = fontSet->fonts[i];

      FontPath path;

      // Skip fonts with no path
      if(GetFcString(fontPattern, FC_FILE, path))
      {
        mSystemFonts.push_back(FontDescription());
        FontDescription& fontDescription = mSystemFonts.back();

        fontDescription.path = std::move(path);

        int width  = 0;
        int weight = 0;
        int slant  = 0;
        GetFcString(fontPattern, FC_FAMILY, fontDescription.family);
        GetFcInt(fontPattern, FC_WIDTH, width);
        GetFcInt(fontPattern, FC_WEIGHT, weight);
        GetFcInt(fontPattern, FC_SLANT, slant);
        fontDescription.width  = IntToWidthType(width);
        fontDescription.weight = IntToWeightType(weight);
        fontDescription.slant  = IntToSlantType(slant);

        FONT_LOG_DESCRIPTION(fontDescription, "");
      }
    }

    // Destroys the font set created.
    FcFontSetDestroy(fontSet);
  }
}

bool FontClient::Plugin::MatchFontDescriptionToPattern(FcPattern* pattern, Dali::TextAbstraction::FontDescription& fontDescription, FcCharSet** characterSet)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

  FcResult   result = FcResultMatch;
  FcPattern* match  = FcFontMatch(nullptr /* use default configure */, pattern, &result); // Creates a new font pattern that needs to be destroyed by calling FcPatternDestroy.

  const bool matched = nullptr != match;
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  pattern matched : %s\n", (matched ? "true" : "false"));

  if(matched)
  {
    int width  = 0;
    int weight = 0;
    int slant  = 0;
    GetFcString(match, FC_FILE, fontDescription.path);
    GetFcString(match, FC_FAMILY, fontDescription.family);
    GetFcInt(match, FC_WIDTH, width);
    GetFcInt(match, FC_WEIGHT, weight);
    GetFcInt(match, FC_SLANT, slant);
    fontDescription.width  = IntToWidthType(width);
    fontDescription.weight = IntToWeightType(weight);
    fontDescription.slant  = IntToSlantType(slant);

    // Retrieve the character set and increase the reference counter.
    FcPatternGetCharSet(match, FC_CHARSET, 0u, characterSet);
    *characterSet = FcCharSetCopy(*characterSet);

    // destroyed the matched pattern
    FcPatternDestroy(match);
    FONT_LOG_DESCRIPTION(fontDescription, "");
  }
  return matched;
}

_FcFontSet* FontClient::Plugin::GetFcFontSet() const
{
  FcFontSet* fontset = nullptr;

  // create a new pattern.
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern* pattern = FcPatternCreate();

  if(nullptr != pattern)
  {
    // create an object set used to define which properties are to be returned in the patterns from FcFontList.
    FcObjectSet* objectSet = FcObjectSetCreate();

    if(nullptr != objectSet)
    {
      // build an object set from a list of property names
      FcObjectSetAdd(objectSet, FC_FILE);
      FcObjectSetAdd(objectSet, FC_FAMILY);
      FcObjectSetAdd(objectSet, FC_WIDTH);
      FcObjectSetAdd(objectSet, FC_WEIGHT);
      FcObjectSetAdd(objectSet, FC_SLANT);

      // get a list of fonts
      // creates patterns from those fonts containing only the objects in objectSet and returns the set of unique such patterns
      fontset = FcFontList(nullptr /* the default configuration is checked to be up to date, and used */, pattern, objectSet); // Creates a FcFontSet that needs to be destroyed by calling FcFontSetDestroy.

      // clear up the object set
      FcObjectSetDestroy(objectSet);
    }

    // clear up the pattern
    FcPatternDestroy(pattern);
  }

  return fontset;
}

bool FontClient::Plugin::GetFcString(const FcPattern* const pattern,
                                     const char* const      n,
                                     std::string&           string)
{
  FcChar8*       file   = nullptr;
  const FcResult retVal = FcPatternGetString(pattern, n, 0u, &file);

  if(FcResultMatch == retVal)
  {
    // Have to use reinterpret_cast because FcChar8 is unsigned char*, not a const char*.
    string.assign(reinterpret_cast<const char*>(file));

    return true;
  }

  return false;
}

bool FontClient::Plugin::GetFcInt(const _FcPattern* const pattern, const char* const n, int& intVal)
{
  const FcResult retVal = FcPatternGetInteger(pattern, n, 0u, &intVal);

  if(FcResultMatch == retVal)
  {
    return true;
  }

  return false;
}

FontId FontClient::Plugin::CreateFont(const FontPath& path,
                                      PointSize26Dot6 requestedPointSize,
                                      FaceIndex       faceIndex,
                                      bool            cacheDescription)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "                path : [%s]\n", path.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);

  FontId id = 0u;

  // Create & cache new font face
  FT_Face ftFace;
  int     error = FT_New_Face(mFreeTypeLibrary,
                          path.c_str(),
                          0,
                          &ftFace);

  if(FT_Err_Ok == error)
  {
    // Check if a font is scalable.
    const bool isScalable           = (0 != (ftFace->face_flags & FT_FACE_FLAG_SCALABLE));
    const bool hasFixedSizedBitmaps = (0 != (ftFace->face_flags & FT_FACE_FLAG_FIXED_SIZES)) && (0 != ftFace->num_fixed_sizes);
    const bool hasColorTables       = (0 != (ftFace->face_flags & FT_FACE_FLAG_COLOR));
    FontId     fontFaceId           = 0u;

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "            isScalable : [%s]\n", (isScalable ? "true" : "false"));
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  hasFixedSizedBitmaps : [%s]\n", (hasFixedSizedBitmaps ? "true" : "false"));
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "        hasColorTables : [%s]\n", (hasColorTables ? "true" : "false"));

    // Check to see if the font contains fixed sizes?
    if(!isScalable && hasFixedSizedBitmaps)
    {
      PointSize26Dot6 actualPointSize = 0u;
      int             fixedSizeIndex  = 0;
      for(; fixedSizeIndex < ftFace->num_fixed_sizes; ++fixedSizeIndex)
      {
        const PointSize26Dot6 fixedSize = ftFace->available_sizes[fixedSizeIndex].size;
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
        actualPointSize = ftFace->available_sizes[fixedSizeIndex].size;
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
        FontFaceCacheItem fontFaceCacheItem(mFreeTypeLibrary, ftFace, path, requestedPointSize, faceIndex, metrics, fixedSizeIndex, fixedWidth, fixedHeight, hasColorTables);

        // Set the index to the font's id cache.
        fontFaceCacheItem.mFontId = mFontIdCache.Count();

        // Create the font id item to cache.
        FontIdCacheItem fontIdCacheItem;
        fontIdCacheItem.type = FontDescription::FACE_FONT;

        // Set the index to the FreeType font face cache.
        fontIdCacheItem.index = mFontFaceCache.size();
        fontFaceId            = fontIdCacheItem.index + 1u;

        // Cache the items.
        mFontFaceCache.emplace_back(std::move(fontFaceCacheItem));
        mFontIdCache.PushBack(fontIdCacheItem);

        // Set the font id to be returned.
        id = mFontIdCache.Count();
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
                                 requestedPointSize,
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
        FontFaceCacheItem fontFaceCacheItem(mFreeTypeLibrary, ftFace, path, requestedPointSize, faceIndex, metrics);

        // Set the index to the font's id cache.
        fontFaceCacheItem.mFontId = mFontIdCache.Count();

        // Create the font id item to cache.
        FontIdCacheItem fontIdCacheItem;
        fontIdCacheItem.type = FontDescription::FACE_FONT;

        // Set the index to the FreeType font face cache.
        fontIdCacheItem.index = mFontFaceCache.size();
        fontFaceId            = fontIdCacheItem.index + 1u;

        // Cache the items.
        mFontFaceCache.emplace_back(std::move(fontFaceCacheItem));
        mFontIdCache.PushBack(fontIdCacheItem);

        // Set the font id to be returned.
        id = mFontIdCache.Count();
      }
      else
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  FreeType Set_Char_Size error: %d for pointSize %d\n", error, requestedPointSize);
      }
    }

    if(0u != fontFaceId)
    {
      if(cacheDescription)
      {
        CacheFontPath(ftFace, fontFaceId, requestedPointSize, path);
      }
    }
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  FreeType New_Face error: %d for [%s]\n", error, path.c_str());
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font id : %d\n", id);
  return id;
}

bool FontClient::Plugin::FindFont(const FontPath& path,
                                  PointSize26Dot6 requestedPointSize,
                                  FaceIndex       faceIndex,
                                  FontId&         fontId) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "                path : [%s]\n", path.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of fonts in the cache : %d\n", mFontFaceCache.size());

  fontId = 0u;
  for(const auto& cacheItem : mFontFaceCache)
  {
    if(cacheItem.mRequestedPointSize == requestedPointSize &&
       cacheItem.mFaceIndex == faceIndex &&
       cacheItem.mPath == path)
    {
      fontId = cacheItem.mFontId + 1u;

      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font found, id : %d\n", fontId);
      return true;
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font not found\n");
  return false;
}

bool FontClient::Plugin::FindValidatedFont(const FontDescription& fontDescription,
                                           FontDescriptionId&     fontDescriptionId)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_DESCRIPTION(fontDescription, "");
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of validated fonts in the cache : %d\n", mValidatedFontCache.size());

  fontDescriptionId = 0u;

  for(const auto& item : mValidatedFontCache)
  {
    if(!fontDescription.family.empty() &&
       (fontDescription.family == item.fontDescription.family) &&
       (fontDescription.width == item.fontDescription.width) &&
       (fontDescription.weight == item.fontDescription.weight) &&
       (fontDescription.slant == item.fontDescription.slant))
    {
      fontDescriptionId = item.index;

      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  validated font description found, id : %d\n", fontDescriptionId);
      return true;
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  validated font description not found\n");
  return false;
}

bool FontClient::Plugin::FindFallbackFontList(const FontDescription& fontDescription,
                                              FontList*&             fontList,
                                              CharacterSetList*&     characterSetList)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  FONT_LOG_DESCRIPTION(fontDescription, "");
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of fallback font lists in the cache : %d\n", mFallbackCache.size());

  fontList = nullptr;

  for(const auto& item : mFallbackCache)
  {
    if(!fontDescription.family.empty() &&
       (fontDescription.family == item.fontDescription.family) &&
       (fontDescription.width == item.fontDescription.width) &&
       (fontDescription.weight == item.fontDescription.weight) &&
       (fontDescription.slant == item.fontDescription.slant))
    {
      fontList         = item.fallbackFonts;
      characterSetList = item.characterSets;

      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  fallback font list found.\n");
      return true;
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  fallback font list not found.\n");
  return false;
}

bool FontClient::Plugin::FindFont(FontDescriptionId fontDescriptionId,
                                  PointSize26Dot6   requestedPointSize,
                                  FontCacheIndex&   fontCacheIndex)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "   fontDescriptionId : %d\n", fontDescriptionId);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);

  fontCacheIndex = 0u;

  FontDescriptionSizeCacheKey key(fontDescriptionId, requestedPointSize);

  const auto& iter = mFontDescriptionSizeCache.find(key);
  if(iter != mFontDescriptionSizeCache.cend())
  {
    fontCacheIndex = iter->second;

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font found, index of font cache : %d\n", fontCacheIndex);
    return true;
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font not found.\n");
  return false;
}

bool FontClient::Plugin::FindBitmapFont(const FontFamily& bitmapFont, FontId& fontId) const
{
  fontId = 0u;

  for(const auto& item : mBitmapFontCache)
  {
    if(bitmapFont == item.font.name)
    {
      fontId = item.id + 1u;
      return true;
    }
  }

  return false;
}

bool FontClient::Plugin::IsScalable(const FontPath& path)
{
  bool isScalable = false;

  FT_Face ftFace;
  int     error = FT_New_Face(mFreeTypeLibrary,
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

  return isScalable;
}

bool FontClient::Plugin::IsScalable(const FontDescription& fontDescription)
{
  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern(fontDescription); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // match the pattern
  FcPattern* match      = FcFontMatch(nullptr /* use default configure */, fontFamilyPattern, &result); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.
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

void FontClient::Plugin::GetFixedSizes(const FontPath& path, Vector<PointSize26Dot6>& sizes)
{
  // Empty the caller container
  sizes.Clear();

  FT_Face ftFace;
  int     error = FT_New_Face(mFreeTypeLibrary,
                          path.c_str(),
                          0,
                          &ftFace);
  if(FT_Err_Ok != error)
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GetFixedSizes. FreeType Cannot check font path : [%s]\n", path.c_str());
  }

  // Fetch the number of fixed sizes available
  if(ftFace->num_fixed_sizes && ftFace->available_sizes)
  {
    for(int i = 0; i < ftFace->num_fixed_sizes; ++i)
    {
      sizes.PushBack(ftFace->available_sizes[i].size);
    }
  }
}

void FontClient::Plugin::GetFixedSizes(const FontDescription&   fontDescription,
                                       Vector<PointSize26Dot6>& sizes)
{
  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern(fontDescription); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // match the pattern
  FcPattern* match = FcFontMatch(nullptr /* use default configure */, fontFamilyPattern, &result); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

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

void FontClient::Plugin::CacheFontPath(FT_Face ftFace, FontId id, PointSize26Dot6 requestedPointSize, const FontPath& path)
{
  FontDescription description;
  description.path   = path;
  description.family = std::move(FontFamily(ftFace->family_name));
  description.weight = FontWeight::NONE;
  description.width  = FontWidth::NONE;
  description.slant  = FontSlant::NONE;

  // Note FreeType doesn't give too much info to build a proper font style.
  if(ftFace->style_flags & FT_STYLE_FLAG_ITALIC)
  {
    description.slant = FontSlant::ITALIC;
  }
  if(ftFace->style_flags & FT_STYLE_FLAG_BOLD)
  {
    description.weight = FontWeight::BOLD;
  }

  FontDescriptionId fontDescriptionId = 0u;
  if(!FindValidatedFont(description,
                        fontDescriptionId))
  {
    FcPattern* pattern = CreateFontFamilyPattern(description); // Creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcResult   result = FcResultMatch;
    FcPattern* match  = FcFontMatch(nullptr, pattern, &result); // FcFontMatch creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcCharSet* characterSet = nullptr;
    FcPatternGetCharSet(match, FC_CHARSET, 0u, &characterSet);

    const FontId fontFaceId                  = id - 1u;
    mFontFaceCache[fontFaceId].mCharacterSet = FcCharSetCopy(characterSet); // Increases the reference counter.

    // Destroys the created patterns.
    FcPatternDestroy(match);
    FcPatternDestroy(pattern);

    // Add the path to the cache.
    description.type = FontDescription::FACE_FONT;
    mFontDescriptionCache.push_back(description);

    // Set the index to the vector of paths to font file names.
    fontDescriptionId = mFontDescriptionCache.size();

    // Increase the reference counter and add the character set to the cache.
    mCharacterSetCache.PushBack(FcCharSetCopy(characterSet));

    // Cache the index and the font's description.
    mValidatedFontCache.push_back(std::move(FontDescriptionCacheItem(std::move(description),
                                                                     fontDescriptionId)));

    // Cache the pair 'fontDescriptionId, requestedPointSize' to improve the following queries.
    mFontDescriptionSizeCache.emplace(FontDescriptionSizeCacheKey(fontDescriptionId, requestedPointSize), fontFaceId);
  }
}

void FontClient::Plugin::ClearFallbackCache(std::vector<FallbackCacheItem>& fallbackCache)
{
  for(auto& item : fallbackCache)
  {
    if(nullptr != item.fallbackFonts)
    {
      delete item.fallbackFonts;
    }

    if(nullptr != item.characterSets)
    {
      // Free the resources allocated by the FcCharSet objects in the 'characterSets' vector.
      DestroyCharacterSets(*item.characterSets);
      delete item.characterSets;
    }
  }
}

void FontClient::Plugin::ClearCharacterSetFromFontFaceCache()
{
  for(auto& item : mFontFaceCache)
  {
    FcCharSetDestroy(item.mCharacterSet);
    item.mCharacterSet = nullptr;
  }
}

} // namespace Dali::TextAbstraction::Internal
