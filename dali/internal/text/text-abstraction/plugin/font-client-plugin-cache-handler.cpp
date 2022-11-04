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
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-cache-handler.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <fontconfig/fontconfig.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/internal/text/text-abstraction/font-client-impl.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;

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
 * @brief Maximum size of glyph cache per each font face.
 */
constexpr std::size_t DEFAULT_GLYPH_CACHE_MAX         = 128;
constexpr std::size_t MINIMUM_SIZE_OF_GLYPH_CACHE_MAX = 3u;

constexpr auto MAX_NUMBER_OF_GLYPH_CACHE_ENV = "DALI_GLYPH_CACHE_MAX";

/**
 * @brief Get maximum size of glyph cache size from environment.
 * If not settuped, default as 128.
 * @note This value fixed when we call it first time.
 * @return The max size of glyph cache.
 */
inline size_t GetMaxNumberOfGlyphCache()
{
  using Dali::EnvironmentVariable::GetEnvironmentVariable;
  static auto numberString = GetEnvironmentVariable(MAX_NUMBER_OF_GLYPH_CACHE_ENV);
  static auto number       = numberString ? std::strtoul(numberString, nullptr, 10) : DEFAULT_GLYPH_CACHE_MAX;
  return (number < MINIMUM_SIZE_OF_GLYPH_CACHE_MAX) ? MINIMUM_SIZE_OF_GLYPH_CACHE_MAX : number;
}

} // namespace

namespace Dali::TextAbstraction::Internal
{
namespace
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
 * @brief Retrieves the fonts present in the platform.
 *
 * @note Need to call FcFontSetDestroy to free the allocated resources.
 *
 * @return A font fonfig data structure with the platform's fonts.
 */
_FcFontSet* GetFcFontSet()
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

/**
 * @brief Helper for GetDefaultFonts etc.
 *
 * @note CharacterSetList is a vector of FcCharSet that are reference counted. It's needed to call FcCharSetDestroy to decrease the reference counter.
 *
 * @param[in] fontDescription A font description.
 * @param[out] fontList A list of the fonts which are a close match for fontDescription.
 * @param[out] characterSetList A list of character sets which are a close match for fontDescription.
 */
void SetFontList(const FontDescription& fontDescription, FontList& fontList, CharacterSetList& characterSetList)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
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

        FONT_LOG_DESCRIPTION(newFontDescription, "new font");
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

} // namespace

FontClient::Plugin::CacheHandler::FallbackCacheItem::FallbackCacheItem(FontDescription&& font, FontList* fallbackFonts, CharacterSetList* characterSets)
: fontDescription{std::move(font)},
  fallbackFonts{fallbackFonts},
  characterSets{characterSets}
{
}

FontClient::Plugin::CacheHandler::FontDescriptionCacheItem::FontDescriptionCacheItem(const FontDescription& fontDescription,
                                                                                     FontDescriptionId      index)
: fontDescription{fontDescription},
  index{index}
{
}

FontClient::Plugin::CacheHandler::FontDescriptionCacheItem::FontDescriptionCacheItem(FontDescription&& fontDescription,
                                                                                     FontDescriptionId index)
: fontDescription{std::move(fontDescription)},
  index{index}
{
}

FontClient::Plugin::CacheHandler::FontDescriptionSizeCacheKey::FontDescriptionSizeCacheKey(FontDescriptionId fontDescriptionId,
                                                                                           PointSize26Dot6   requestedPointSize)
: fontDescriptionId(fontDescriptionId),
  requestedPointSize(requestedPointSize)
{
}

// CacheHandler
FontClient::Plugin::CacheHandler::CacheHandler()
: mDefaultFontDescription(),
  mSystemFonts(),
  mDefaultFonts(),
  mFontIdCache(),
  mFontFaceCache(),
  mValidatedFontCache(),
  mFontDescriptionCache(),
  mCharacterSetCache(),
  mFontDescriptionSizeCache(),
  mEllipsisCache(),
  mEmbeddedItemCache(),
  mGlyphCacheManager(new GlyphCacheManager(GetMaxNumberOfGlyphCache())),
  mLatestFoundFontDescription(),
  mLatestFoundFontDescriptionId(0u),
  mLatestFoundCacheKey(0, 0),
  mLatestFoundCacheIndex(0u),
  mDefaultFontDescriptionCached(false)
{
}

FontClient::Plugin::CacheHandler::~CacheHandler()
{
  ClearCache();
}

void FontClient::Plugin::CacheHandler::ClearCache()
{
  // delete cached glyph informations before clear mFontFaceCache.
  mGlyphCacheManager->ClearCache();

  mDefaultFontDescription = FontDescription();

  mSystemFonts.clear();
  mDefaultFonts.clear();

  DestroyCharacterSets(mDefaultFontCharacterSets);
  mDefaultFontCharacterSets.Clear();

  ClearFallbackCache();
  mFallbackCache.clear();

  mFontIdCache.clear();

  ClearCharacterSetFromFontFaceCache();
  mFontFaceCache.clear();

  mValidatedFontCache.clear();
  mFontDescriptionCache.clear();

  DestroyCharacterSets(mCharacterSetCache);
  mCharacterSetCache.Clear();

  mFontDescriptionSizeCache.clear();
  mFontDescriptionSizeCache.rehash(0); // Note : unordered_map.clear() didn't deallocate memory

  mEllipsisCache.clear();
  mPixelBufferCache.clear();
  mEmbeddedItemCache.clear();
  mBitmapFontCache.clear();

  mLatestFoundFontDescription.family.clear();
  mLatestFoundCacheKey = FontDescriptionSizeCacheKey(0, 0);

  mDefaultFontDescriptionCached = false;
}

void FontClient::Plugin::CacheHandler::ResetSystemDefaults()
{
  mDefaultFontDescriptionCached = false;
}

// Clear cache area

void FontClient::Plugin::CacheHandler::ClearFallbackCache()
{
  for(auto& item : mFallbackCache)
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

void FontClient::Plugin::CacheHandler::ClearCharacterSetFromFontFaceCache()
{
  for(auto& item : mFontFaceCache)
  {
    FcCharSetDestroy(item.mCharacterSet);
    item.mCharacterSet = nullptr;
  }
}

void FontClient::Plugin::CacheHandler::ClearCharacterSet()
{
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
}

void FontClient::Plugin::CacheHandler::CreateCharacterSet()
{
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
}

// System / Default

void FontClient::Plugin::CacheHandler::InitSystemFonts()
{
  if(mSystemFonts.empty())
  {
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

          FONT_LOG_DESCRIPTION(fontDescription, "system fonts");
        }
      }

      // Destroys the font set created.
      FcFontSetDestroy(fontSet);
    }
  }
}

void FontClient::Plugin::CacheHandler::InitDefaultFonts()
{
  if(mDefaultFonts.empty())
  {
    FontDescription fontDescription;
    fontDescription.family = DefaultFontFamily(); // todo This could be set to the Platform font
    fontDescription.width  = DefaultFontWidth();
    fontDescription.weight = DefaultFontWeight();
    fontDescription.slant  = DefaultFontSlant();
    SetFontList(fontDescription, mDefaultFonts, mDefaultFontCharacterSets);
  }
}

void FontClient::Plugin::CacheHandler::InitDefaultFontDescription()
{
  if(!mDefaultFontDescriptionCached)
  {
    // Clear any font config stored info in the caches.
    ClearCharacterSet();

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
        const FontDescriptionId fontDescriptionId = static_cast<FontDescriptionId>(mFontDescriptionCache.size());

        FONT_LOG_DESCRIPTION(tempFontDescription, "default platform font");
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  default font fontDescriptionId : %d\n", fontDescriptionId);

        // Cache the index and the matched font's description.
        CacheValidateFont(std::move(tempFontDescription), fontDescriptionId);
      }
      else
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  default font validation failed for font [%s]\n", mDefaultFontDescription.family.c_str());
      }

      // Decrease the reference counter of the character set as it's not stored.
      // Note. the cached default font description will increase reference counter by
      // mFontDescriptionCache in CreateCharacterSet(). So we can decrease reference counter here.
      FcCharSetDestroy(characterSet);

      // Destroys the pattern created.
      FcPatternDestroy(matchPattern);
    }

    // Create again the character sets as they are not valid after FcInitReinitialize()
    CreateCharacterSet();

    mDefaultFontDescriptionCached = true;
  }
}

// Validate

bool FontClient::Plugin::CacheHandler::FindValidatedFont(const FontDescription& fontDescription,
                                                         FontDescriptionId&     fontDescriptionId)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of validated fonts in the cache : %zu\n", mValidatedFontCache.size());

  fontDescriptionId = 0u;

  // Fast cut if inputed family is empty.
  if(DALI_UNLIKELY(fontDescription.family.empty()))
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  validated font description not found / fontDescription.family is empty!\n");
    return false;
  }

  // Heuristic optimize code : Compare with latest found item.
  if((fontDescription.width == mLatestFoundFontDescription.width) &&
     (fontDescription.weight == mLatestFoundFontDescription.weight) &&
     (fontDescription.slant == mLatestFoundFontDescription.slant) &&
     (fontDescription.family == mLatestFoundFontDescription.family))
  {
    fontDescriptionId = mLatestFoundFontDescriptionId;

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  validated font description same as latest, id : %d\n", fontDescriptionId);
    return true;
  }

  for(const auto& item : mValidatedFontCache)
  {
    if((fontDescription.width == item.fontDescription.width) &&
       (fontDescription.weight == item.fontDescription.weight) &&
       (fontDescription.slant == item.fontDescription.slant) &&
       (fontDescription.family == item.fontDescription.family))
    {
      fontDescriptionId = item.index;

      mLatestFoundFontDescription   = fontDescription;
      mLatestFoundFontDescriptionId = fontDescriptionId;

      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  validated font description found, id : %d\n", fontDescriptionId);
      return true;
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  validated font description not found\n");
  return false;
}

void FontClient::Plugin::CacheHandler::ValidateFont(const FontDescription& fontDescription,
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
    fontDescriptionId = static_cast<FontDescriptionId>(mFontDescriptionCache.size());

    FONT_LOG_DESCRIPTION(description, "matched");
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  fontDescriptionId : %d\n", fontDescriptionId);

    // The reference counter of the character set has already been increased in MatchFontDescriptionToPattern.
    mCharacterSetCache.PushBack(characterSet);

    if((fontDescription.family != description.family) ||
       (fontDescription.width != description.width) ||
       (fontDescription.weight != description.weight) ||
       (fontDescription.slant != description.slant))
    {
      // Cache the given font's description if it's different than the matched.
      CacheValidateFont(std::move(FontDescription(fontDescription)), fontDescriptionId);
    }

    // Cache the index and the matched font's description.
    CacheValidateFont(std::move(description), fontDescriptionId);
  }
  else
  {
    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font validation failed for font [%s]\n", fontDescription.family.c_str());
  }
}

void FontClient::Plugin::CacheHandler::CacheValidateFont(FontDescription&& fontDescription,
                                                         FontDescriptionId validatedFontId)
{
  mValidatedFontCache.emplace_back(std::move(FontDescriptionCacheItem(fontDescription, validatedFontId)));
}

// Fallback

bool FontClient::Plugin::CacheHandler::FindFallbackFontList(const FontDescription& fontDescription,
                                                            FontList*&             fontList,
                                                            CharacterSetList*&     characterSetList) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of fallback font lists in the cache : %zu\n", mFallbackCache.size());

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

void FontClient::Plugin::CacheHandler::CacheFallbackFontList(FontDescription&&  fontDescription,
                                                             FontList*&         fontList,
                                                             CharacterSetList*& characterSetList)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);

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
  mFallbackCache.push_back(std::move(CacheHandler::FallbackCacheItem(std::move(fontDescription), fontList, characterSetList)));
}

// Font / FontFace

bool FontClient::Plugin::CacheHandler::FindFontByPath(const FontPath& path,
                                                      PointSize26Dot6 requestedPointSize,
                                                      FaceIndex       faceIndex,
                                                      FontId&         fontId) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "                path : [%s]\n", path.c_str());
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "  number of fonts in the cache : %zu\n", mFontFaceCache.size());

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

bool FontClient::Plugin::CacheHandler::FindFont(FontDescriptionId fontDescriptionId,
                                                PointSize26Dot6   requestedPointSize,
                                                FontCacheIndex&   fontCacheIndex)
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "   fontDescriptionId : %d\n", fontDescriptionId);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize);

  fontCacheIndex = 0u;

  const FontDescriptionSizeCacheKey key(fontDescriptionId, requestedPointSize);

  // Heuristic optimize code : Compare with latest found item.
  if(key == mLatestFoundCacheKey)
  {
    fontCacheIndex = mLatestFoundCacheIndex;

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font same as latest, index of font cache : %d\n", fontCacheIndex);
    return true;
  }

  const auto& iter = mFontDescriptionSizeCache.find(key);
  if(iter != mFontDescriptionSizeCache.cend())
  {
    fontCacheIndex = iter->second;

    mLatestFoundCacheKey   = key;
    mLatestFoundCacheIndex = fontCacheIndex;

    DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font found, index of font cache : %d\n", fontCacheIndex);
    return true;
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  font not found.\n");
  return false;
}

void FontClient::Plugin::CacheHandler::CacheFontDescriptionSize(FontDescriptionId fontDescriptionId, PointSize26Dot6 requestedPointSize, FontCacheIndex fontCacheIndex)
{
  mFontDescriptionSizeCache.emplace(FontDescriptionSizeCacheKey(fontDescriptionId, requestedPointSize), fontCacheIndex);
}

void FontClient::Plugin::CacheHandler::CacheFontPath(FT_Face ftFace, FontId fontId, PointSize26Dot6 requestedPointSize, const FontPath& path)
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
  if(!FindValidatedFont(description, fontDescriptionId))
  {
    // TODO : Due to the FontClient pattern match process, we cannot pass dali-toolkit UTC.
    // Can't we use ValidateFont here?
    /*
    // Use font config to validate the font's description.
    ValidateFont(description, fontDescriptionId);

    const FontCacheIndex fontCacheIndex          = mFontIdCache[fontId - 1u].index;
    mFontFaceCache[fontCacheIndex].mCharacterSet = FcCharSetCopy(mCharacterSetCache[fontDescriptionId - 1u]); // Increases the reference counter.

    // Cache the pair 'fontDescriptionId, requestedPointSize' to improve the following queries.
    mFontDescriptionSizeCache.emplace(CacheHandler::FontDescriptionSizeCacheKey(fontDescriptionId, requestedPointSize), fontCacheIndex);
    */

    FcPattern* pattern = CreateFontFamilyPattern(description); // Creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcResult   result = FcResultMatch;
    FcPattern* match  = FcFontMatch(nullptr, pattern, &result); // FcFontMatch creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcCharSet* characterSet = nullptr;
    FcPatternGetCharSet(match, FC_CHARSET, 0u, &characterSet);

    const FontCacheIndex fontCacheIndex          = mFontIdCache[fontId - 1u].index;
    mFontFaceCache[fontCacheIndex].mCharacterSet = FcCharSetCopy(characterSet); // Increases the reference counter.

    // Destroys the created patterns.
    FcPatternDestroy(match);
    FcPatternDestroy(pattern);

    // Add the path to the cache.
    description.type = FontDescription::FACE_FONT;
    mFontDescriptionCache.push_back(description);

    // Set the index to the vector of paths to font file names.
    fontDescriptionId = static_cast<FontDescriptionId>(mFontDescriptionCache.size());

    // Increase the reference counter and add the character set to the cache.
    mCharacterSetCache.PushBack(FcCharSetCopy(characterSet));

    // Cache the index and the font's description.
    CacheValidateFont(std::move(description), fontDescriptionId);

    // Cache the pair 'fontDescriptionId, requestedPointSize' to improve the following queries.
    CacheFontDescriptionSize(fontDescriptionId, requestedPointSize, fontCacheIndex);
  }
}

FontId FontClient::Plugin::CacheHandler::CacheFontFaceCacheItem(FontFaceCacheItem&& fontFaceCacheItem)
{
  // Set the index to the font's id cache.
  fontFaceCacheItem.mFontId = static_cast<FontId>(mFontIdCache.size());

  // Create the font id item to cache.
  FontIdCacheItem fontIdCacheItem;
  fontIdCacheItem.type = FontDescription::FACE_FONT;

  // Set the index to the FreeType font face cache.
  fontIdCacheItem.index = static_cast<FontCacheIndex>(mFontFaceCache.size());

  // Cache the items.
  mFontFaceCache.emplace_back(std::move(fontFaceCacheItem));
  mFontIdCache.emplace_back(std::move(fontIdCacheItem));

  // Set the font id to be returned.
  FontId fontId = static_cast<FontId>(mFontIdCache.size());

  return fontId;
}

// Ellipsis

bool FontClient::Plugin::CacheHandler::FindEllipsis(PointSize26Dot6 requestedPointSize, EllipsisCacheIndex& ellipsisCacheIndex) const
{
  DALI_LOG_TRACE_METHOD(gFontClientLogFilter);
  DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  requestedPointSize %d.\n", requestedPointSize);

  ellipsisCacheIndex = 0u;

  // First look into the cache if there is an ellipsis glyph for the requested point size.
  for(const auto& item : mEllipsisCache)
  {
    if(item.requestedPointSize == requestedPointSize)
    {
      // Use the glyph in the cache.
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "  glyph id %d found in the cache.\n", item.glyph.index);
      DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "      font %d.\n", item.glyph.fontId);
      ellipsisCacheIndex = item.index;
      return true;
    }
  }
  return false;
}

FontClient::Plugin::CacheHandler::EllipsisCacheIndex FontClient::Plugin::CacheHandler::CacheEllipsis(EllipsisItem&& ellipsisItem)
{
  EllipsisCacheIndex ellipsisCacheIndex = static_cast<EllipsisCacheIndex>(mEllipsisCache.size());

  mEllipsisCache.emplace_back(std::move(ellipsisItem));

  return ellipsisCacheIndex;
}

// Bitmap font

bool FontClient::Plugin::CacheHandler::FindBitmapFont(const FontFamily& bitmapFontFamily, FontId& fontId) const
{
  fontId = 0u;

  for(const auto& item : mBitmapFontCache)
  {
    if(bitmapFontFamily == item.font.name)
    {
      fontId = item.id + 1u;
      return true;
    }
  }

  return false;
}

FontId FontClient::Plugin::CacheHandler::CacheBitmapFontCacheItem(BitmapFontCacheItem&& bitmapFontCacheItem)
{
  // Set the index to the font's id cache.
  bitmapFontCacheItem.id = static_cast<FontId>(mFontIdCache.size());

  // Create the font id item to cache.
  CacheHandler::FontIdCacheItem fontIdCacheItem;
  fontIdCacheItem.type = FontDescription::BITMAP_FONT;

  // Set the index to the Bitmap font face cache.
  fontIdCacheItem.index = static_cast<FontCacheIndex>(mBitmapFontCache.size());

  // Cache the items.
  mBitmapFontCache.emplace_back(std::move(bitmapFontCacheItem));
  mFontIdCache.emplace_back(std::move(fontIdCacheItem));

  // Set the font id to be returned.
  FontId fontId = static_cast<FontId>(mFontIdCache.size());

  return fontId;
}

// Embedded

bool FontClient::Plugin::CacheHandler::FindEmbeddedPixelBufferId(const std::string& url, PixelBufferId& pixelBufferId) const
{
  pixelBufferId = 0u;

  for(const auto& cacheItem : mPixelBufferCache)
  {
    if(cacheItem.url == url)
    {
      // The url is in the pixel buffer cache.
      pixelBufferId = cacheItem.id;
      return true;
    }
  }

  return false;
}

PixelBufferId FontClient::Plugin::CacheHandler::CacheEmbeddedPixelBuffer(const std::string& url)
{
  PixelBufferId pixelBufferId = 0u;

  // Load the image from the url.
  Devel::PixelBuffer pixelBuffer = LoadImageFromFile(url);
  if(pixelBuffer)
  {
    // Create the cache item.
    PixelBufferCacheItem pixelBufferCacheItem;
    pixelBufferCacheItem.pixelBuffer = pixelBuffer;
    pixelBufferCacheItem.url         = url;
    pixelBufferCacheItem.id          = static_cast<PixelBufferId>(mPixelBufferCache.size() + 1u);

    // Store the cache item in the cache.
    mPixelBufferCache.emplace_back(std::move(pixelBufferCacheItem));

    // Set the pixel buffer id to be returned.
    pixelBufferId = static_cast<PixelBufferId>(mPixelBufferCache.size());
  }
  return pixelBufferId;
}

bool FontClient::Plugin::CacheHandler::FindEmbeddedItem(PixelBufferId pixelBufferId, uint32_t width, uint32_t height, GlyphIndex& index) const
{
  index = 0u;

  for(const auto& cacheItem : mEmbeddedItemCache)
  {
    if((cacheItem.pixelBufferId == pixelBufferId) &&
       (cacheItem.width == width) &&
       (cacheItem.height == height))
    {
      index = cacheItem.index;
      return true;
    }
  }

  return false;
}

GlyphIndex FontClient::Plugin::CacheHandler::CacheEmbeddedItem(EmbeddedItem&& embeddedItem)
{
  embeddedItem.index = static_cast<GlyphIndex>(mEmbeddedItemCache.size() + 1u);

  // Cache the embedded item.
  mEmbeddedItemCache.emplace_back(std::move(embeddedItem));

  // Set the font id to be returned.
  GlyphIndex index = static_cast<GlyphIndex>(mEmbeddedItemCache.size());

  return index;
}

} // namespace Dali::TextAbstraction::Internal
