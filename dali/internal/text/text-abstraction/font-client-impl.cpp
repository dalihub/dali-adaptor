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
#include <dali/internal/text/text-abstraction/font-client-impl.h>

// EXTERNAL INCLUDES
#include <thread>
#if defined(VCONF_ENABLED)
#include <vconf.h>
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/system/common/logging.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>
#include <dali/internal/window-system/common/window-system.h>

#include <dali/devel-api/text-abstraction/glyph-info.h>

#define FONT_LOG_MESSAGE(level, format, ...)                                   \
  do                                                                           \
  {                                                                            \
    char buffer[256];                                                          \
    int result = std::snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
    if (result >= static_cast<int>(sizeof(buffer)))                            \
    {                                                                          \
      std::string log("Font log message is too long to fit in the buffer.\n"); \
      Dali::TizenPlatform::LogMessage(Dali::Integration::Log::ERROR, log);     \
      break;                                                                   \
    }                                                                          \
    std::string log(buffer);                                                   \
    Dali::TizenPlatform::LogMessage(level, log);                               \
  } while(0)

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
Dali::TextAbstraction::FontClient FontClient::gPreInitializedFontClient(NULL);
Dali::TextAbstraction::FontClient FontClient::gPreCachedFontClient(NULL);
std::thread gPreCacheThread;
/* TODO: This is to prevent duplicate calls of font pre-cache.
 * We may support this later, but currently we can't guarantee the behaviour
 * if there is a pre-cache call from the user after the font client has been created. */
bool gFontPreCacheAvailable = true;

FontClient::FontClient()
: mPlugin(nullptr),
  mDpiHorizontal(0),
  mDpiVertical(0)
{
}

FontClient::~FontClient()
{
  delete mPlugin;
}

Dali::TextAbstraction::FontClient FontClient::Get()
{
  Dali::TextAbstraction::FontClient fontClientHandle;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::TextAbstraction::FontClient));
    if(handle)
    {
      // If so, downcast the handle
      FontClient* impl = dynamic_cast<Dali::TextAbstraction::Internal::FontClient*>(handle.GetObjectPtr());
      fontClientHandle = Dali::TextAbstraction::FontClient(impl);
    }
    else // create and register the object
    {
      if(gPreCacheThread.joinable())
      {
        gPreCacheThread.join();
        FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient PreCache thread join\n");
      }

      if(gPreInitializedFontClient)
      {
        fontClientHandle = gPreInitializedFontClient;
        gPreInitializedFontClient.Reset(); // No longer needed
      }
      else if(gPreCachedFontClient)
      {
        // TODO: Currently font pre-caching is not available in the candidate process.
        fontClientHandle = gPreCachedFontClient;
        gPreCachedFontClient.Reset(); // No longer needed
      }
      else
      {
        fontClientHandle = Dali::TextAbstraction::FontClient(new FontClient);
      }

      gFontPreCacheAvailable = false;

      uint32_t horizontalDpi, verticalDpi;
      fontClientHandle.GetDpi(horizontalDpi, verticalDpi);
      if(horizontalDpi == 0u || verticalDpi == 0u)
      {
        horizontalDpi = verticalDpi = 0u;
        Dali::Internal::Adaptor::WindowSystem::GetDpi(horizontalDpi, verticalDpi);
        fontClientHandle.SetDpi(horizontalDpi, verticalDpi);
      }

      service.Register(typeid(fontClientHandle), fontClientHandle);
    }
  }

  return fontClientHandle;
}

Dali::TextAbstraction::FontClient FontClient::PreInitialize()
{
  gPreInitializedFontClient = Dali::TextAbstraction::FontClient(new FontClient);

  // Make DefaultFontDescription cached
  Dali::TextAbstraction::FontDescription defaultFontDescription;
  gPreInitializedFontClient.GetDefaultPlatformFontDescription(defaultFontDescription);

  return gPreInitializedFontClient;
}

void FontClient::PreCacheRun(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily)
{
  if(!gPreCachedFontClient)
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "BEGIN: DALI_TEXT_PRECACHE_RUN\n");
    Dali::TextAbstraction::FontClient fontClient = Dali::TextAbstraction::FontClient(new FontClient);
    GetImplementation(fontClient).FontPreCache(fallbackFamilyList, extraFamilyList, localeFamily);
    gPreCachedFontClient = fontClient;
    gFontPreCacheAvailable = false;
    FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "END: DALI_TEXT_PRECACHE_RUN\n");
  }
  else
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient pre-cache run failed, as a pre-cached font client already exists.\n");
  }
}

void FontClient::PreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool useThread)
{
  if(!gFontPreCacheAvailable)
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient pre-cache has been completed or the font client has already been created.\n");
    return;
  }

  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient PreCache fallbackFamilyList : %zu\n", fallbackFamilyList.size());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient PreCache extraFamilyList    : %zu\n", extraFamilyList.size());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient PreCache localeFamily       : %s\n", localeFamily.c_str());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient PreCache useThread          : %d\n", useThread);

  if(gPreCacheThread.joinable())
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient pre-cache thread already running.\n");
  }
  else
  {
    if(useThread)
    {
      gPreCacheThread = std::thread(PreCacheRun, fallbackFamilyList, extraFamilyList, localeFamily);
    }
    else
    {
      PreCacheRun(fallbackFamilyList, extraFamilyList, localeFamily);
    }
  }
}

void FontClient::ClearCache()
{
  if(mPlugin)
  {
    mPlugin->ClearCache();
  }
}

void FontClient::SetDpi(unsigned int horizontalDpi, unsigned int verticalDpi)
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical   = verticalDpi;

  // Allow DPI to be set without loading plugin
  if(mPlugin)
  {
    mPlugin->SetDpi(horizontalDpi, verticalDpi);
  }
}

void FontClient::GetDpi(unsigned int& horizontalDpi, unsigned int& verticalDpi)
{
  horizontalDpi = mDpiHorizontal;
  verticalDpi   = mDpiVertical;
}

int FontClient::GetDefaultFontSize()
{
  int fontSize(-1);

#if defined(VCONF_ENABLED)
  vconf_get_int(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &fontSize);
#endif

  return fontSize;
}

void FontClient::ResetSystemDefaults()
{
  CreatePlugin();

  mPlugin->ResetSystemDefaults();
}

void FontClient::GetDefaultFonts(FontList& defaultFonts)
{
  CreatePlugin();

  mPlugin->GetDefaultFonts(defaultFonts);
}

void FontClient::FontPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily)
{
  CreatePlugin();

  mPlugin->FontPreCache(fallbackFamilyList, extraFamilyList, localeFamily);
}

void FontClient::GetDefaultPlatformFontDescription(FontDescription& fontDescription)
{
  CreatePlugin();

  mPlugin->GetDefaultPlatformFontDescription(fontDescription);
}

void FontClient::GetDescription(FontId fontId, FontDescription& fontDescription)
{
  CreatePlugin();

  mPlugin->GetDescription(fontId, fontDescription);
}

PointSize26Dot6 FontClient::GetPointSize(FontId fontId)
{
  CreatePlugin();

  return mPlugin->GetPointSize(fontId);
}

bool FontClient::IsCharacterSupportedByFont(FontId fontId, Character character)
{
  CreatePlugin();

  return mPlugin->IsCharacterSupportedByFont(fontId, character);
}

void FontClient::GetSystemFonts(FontList& systemFonts)
{
  CreatePlugin();

  mPlugin->GetSystemFonts(systemFonts);
}

FontId FontClient::FindDefaultFont(Character       charcode,
                                   PointSize26Dot6 requestedPointSize,
                                   bool            preferColor)
{
  CreatePlugin();

  return mPlugin->FindDefaultFont(charcode,
                                  requestedPointSize,
                                  preferColor);
}

FontId FontClient::FindFallbackFont(Character              charcode,
                                    const FontDescription& preferredFontDescription,
                                    PointSize26Dot6        requestedPointSize,
                                    bool                   preferColor)
{
  CreatePlugin();

  return mPlugin->FindFallbackFont(charcode,
                                   preferredFontDescription,
                                   requestedPointSize,
                                   preferColor);
}

bool FontClient::IsScalable(const FontPath& path)
{
  CreatePlugin();

  return mPlugin->IsScalable(path);
}

bool FontClient::IsScalable(const FontDescription& fontDescription)
{
  CreatePlugin();

  return mPlugin->IsScalable(fontDescription);
}

void FontClient::GetFixedSizes(const FontPath& path, Dali::Vector<PointSize26Dot6>& sizes)
{
  CreatePlugin();

  mPlugin->GetFixedSizes(path, sizes);
}

void FontClient::GetFixedSizes(const FontDescription&         fontDescription,
                               Dali::Vector<PointSize26Dot6>& sizes)
{
  CreatePlugin();

  mPlugin->GetFixedSizes(fontDescription, sizes);
}

bool FontClient::HasItalicStyle(FontId fontId) const
{
  if(!mPlugin)
  {
    return false;
  }
  return mPlugin->HasItalicStyle(fontId);
}

FontId FontClient::GetFontId(const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex)
{
  CreatePlugin();

  return mPlugin->GetFontIdByPath(path,
                                  requestedPointSize,
                                  faceIndex,
                                  true);
}

FontId FontClient::GetFontId(const FontDescription& fontDescription,
                             PointSize26Dot6        requestedPointSize,
                             FaceIndex              faceIndex)
{
  CreatePlugin();

  return mPlugin->GetFontId(fontDescription,
                            requestedPointSize,
                            faceIndex);
}

FontId FontClient::GetFontId(const BitmapFont& bitmapFont)
{
  CreatePlugin();

  return mPlugin->GetFontId(bitmapFont);
}

void FontClient::GetFontMetrics(FontId fontId, FontMetrics& metrics)
{
  CreatePlugin();

  mPlugin->GetFontMetrics(fontId, metrics);
}

GlyphIndex FontClient::GetGlyphIndex(FontId fontId, Character charcode)
{
  CreatePlugin();

  return mPlugin->GetGlyphIndex(fontId, charcode);
}

GlyphIndex FontClient::GetGlyphIndex(FontId fontId, Character charcode, Character variantSelector)
{
  CreatePlugin();

  return mPlugin->GetGlyphIndex(fontId, charcode, variantSelector);
}

bool FontClient::GetGlyphMetrics(GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal)
{
  CreatePlugin();

  return mPlugin->GetGlyphMetrics(array, size, type, horizontal);
}

void FontClient::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth)
{
  CreatePlugin();

  mPlugin->CreateBitmap(fontId, glyphIndex, isItalicRequired, isBoldRequired, data, outlineWidth);
}

PixelData FontClient::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth)
{
  CreatePlugin();

  return mPlugin->CreateBitmap(fontId, glyphIndex, outlineWidth);
}

void FontClient::CreateVectorBlob(FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight)
{
  CreatePlugin();

  mPlugin->CreateVectorBlob(fontId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight);
}

const GlyphInfo& FontClient::GetEllipsisGlyph(PointSize26Dot6 requestedPointSize)
{
  CreatePlugin();

  return mPlugin->GetEllipsisGlyph(requestedPointSize);
}

bool FontClient::IsColorGlyph(FontId fontId, GlyphIndex glyphIndex)
{
  CreatePlugin();

  return mPlugin->IsColorGlyph(fontId, glyphIndex);
}

GlyphIndex FontClient::CreateEmbeddedItem(const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat)
{
  CreatePlugin();

  return mPlugin->CreateEmbeddedItem(description, pixelFormat);
}

void FontClient::EnableAtlasLimitation(bool enabled)
{
  CreatePlugin();
  return mPlugin->EnableAtlasLimitation(enabled);
}

bool FontClient::IsAtlasLimitationEnabled() const
{
  if(mPlugin)
  {
    return mPlugin->IsAtlasLimitationEnabled();
  }
  return TextAbstraction::FontClient::DEFAULT_ATLAS_LIMITATION_ENABLED;
}

Size FontClient::GetMaximumTextAtlasSize() const
{
  if(mPlugin)
  {
    return mPlugin->GetMaximumTextAtlasSize();
  }
  return TextAbstraction::FontClient::MAX_TEXT_ATLAS_SIZE;
}

Size FontClient::GetDefaultTextAtlasSize() const
{
  if(mPlugin)
  {
    return mPlugin->GetDefaultTextAtlasSize();
  }
  return TextAbstraction::FontClient::DEFAULT_TEXT_ATLAS_SIZE;
}

Size FontClient::GetCurrentMaximumBlockSizeFitInAtlas() const
{
  if(mPlugin)
  {
    return mPlugin->GetCurrentMaximumBlockSizeFitInAtlas();
  }
  return TextAbstraction::FontClient::DEFAULT_TEXT_ATLAS_SIZE;
}

bool FontClient::SetCurrentMaximumBlockSizeFitInAtlas(const Size& currentMaximumBlockSizeFitInAtlas)
{
  CreatePlugin();
  return mPlugin->SetCurrentMaximumBlockSizeFitInAtlas(currentMaximumBlockSizeFitInAtlas);
}

uint32_t FontClient::GetNumberOfPointsPerOneUnitOfPointSize() const
{
  if(mPlugin)
  {
    return mPlugin->GetNumberOfPointsPerOneUnitOfPointSize();
  }
  return TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  ;
}

FT_FaceRec_* FontClient::GetFreetypeFace(FontId fontId)
{
  CreatePlugin();

  return mPlugin->GetFreetypeFace(fontId);
}

FontDescription::Type FontClient::GetFontType(FontId fontId)
{
  CreatePlugin();

  return mPlugin->GetFontType(fontId);
}

bool FontClient::AddCustomFontDirectory(const FontPath& path)
{
  CreatePlugin();

  return mPlugin->AddCustomFontDirectory(path);
}

HarfBuzzFontHandle FontClient::GetHarfBuzzFont(FontId fontId)
{
  CreatePlugin();

  return mPlugin->GetHarfBuzzFont(fontId);
}

void FontClient::CreatePlugin()
{
  if(!mPlugin)
  {
    mPlugin = new Plugin(mDpiHorizontal, mDpiVertical);
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
