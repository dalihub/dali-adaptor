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
 *
 */

// CLASS HEADER
#include <dali/internal/text/text-abstraction/font-client-impl.h>

// EXTERNAL INCLUDES
#include <condition_variable>
#include <mutex>
#include <thread>
#if defined(VCONF_ENABLED)
#include <vconf.h>
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/thread-settings.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/system/common/logging.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>
#include <dali/internal/window-system/common/window-system.h>

#include <dali/devel-api/text-abstraction/glyph-info.h>

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

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
Dali::TextAbstraction::FontClient FontClient::gPreCreatedFontClient(NULL);
std::thread                       gPreCacheThread;
std::thread                       gPreLoadThread;
std::mutex                        gMutex;
std::condition_variable           gPreCacheCond;
std::condition_variable           gPreLoadCond;
bool                              gPreCacheThreadReady;
bool                              gPreLoadThreadReady;

/* TODO: This is to prevent duplicate calls of font pre-cache.
 * We may support this later, but currently we can't guarantee the behaviour
 * if there is a pre-cache call from the user after the font client has been created. */
bool gFontPreCacheAvailable = true;
bool gFontPreLoadAvailable  = true;

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

      if(gPreLoadThread.joinable())
      {
        gPreLoadThread.join();
        FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient PreLoad thread join\n");
      }

      if(gPreCreatedFontClient)
      {
        fontClientHandle = gPreCreatedFontClient;
        gPreCreatedFontClient.Reset(); // No longer needed
      }
      else
      {
        fontClientHandle = Dali::TextAbstraction::FontClient(new FontClient);
      }

      fontClientHandle.InitDefaultFontDescription();

      gFontPreCacheAvailable = false;
      gFontPreLoadAvailable  = false;

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
  // Pre-cached font client already exists or pre-cache thread already running.
  // Font client pre-cache includes caching of the default font description.
  if(gPreCreatedFontClient && !gFontPreCacheAvailable)
  {
    return gPreCreatedFontClient;
  }

  if(!gPreCreatedFontClient)
  {
    gPreCreatedFontClient = Dali::TextAbstraction::FontClient(new FontClient);
  }

  gPreCreatedFontClient.InitDefaultFontDescription();

  return gPreCreatedFontClient;
}

void FontClient::PreCacheRun(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool syncCreation)
{
  SetThreadName("FontThread-fc");

  if(syncCreation)
  {
    std::unique_lock<std::mutex> lock(gMutex);
    gPreCacheThreadReady = true;
    gPreCacheCond.notify_one();
    lock.unlock();
  }

  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "BEGIN: DALI_TEXT_PRECACHE_RUN\n");
  GetImplementation(gPreCreatedFontClient).FontPreCache(fallbackFamilyList, extraFamilyList, localeFamily);
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "END: DALI_TEXT_PRECACHE_RUN\n");
}

void FontClient::PreLoadRun(const FontPathList& fontPathList, const FontPathList& memoryFontPathList, bool syncCreation)
{
  SetThreadName("FontThread-ft");

  if(syncCreation)
  {
    std::unique_lock<std::mutex> lock(gMutex);
    gPreLoadThreadReady = true;
    gPreLoadCond.notify_one();
    lock.unlock();
  }

  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "BEGIN: DALI_TEXT_FONT_PRELOAD_RUN\n");
  GetImplementation(gPreCreatedFontClient).FontPreLoad(fontPathList, memoryFontPathList);
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "END: DALI_TEXT_FONT_PRELOAD_RUN\n");
}

void FontClient::PreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool useThread, bool syncCreation)
{
  if(!gFontPreCacheAvailable)
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient pre-cache run failed, as a pre-cached font client already exists.\n");
    return;
  }

  if(gPreCacheThread.joinable())
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient pre-cache thread already running.\n");
  }

  if(!gPreCreatedFontClient)
  {
    gPreCreatedFontClient = Dali::TextAbstraction::FontClient(new FontClient);
  }

  gFontPreCacheAvailable = false;

  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient FontConfig PreCache fallbackFamilyList : %zu\n", fallbackFamilyList.size());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient FontConfig PreCache extraFamilyList    : %zu\n", extraFamilyList.size());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient FontConfig PreCache localeFamily       : %s\n", localeFamily.c_str());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient FontConfig PreCache useThread          : %d\n", useThread);

  if(useThread)
  {
    if(syncCreation)
    {
      // The main thread wakes up upon receiving a notification from the pre-cache thread.
      // If it doesn't receive a notification within the specified time, it wakes up due to a timeout.
      const std::chrono::milliseconds timeout(1000);
      gPreCacheThreadReady = false;
      std::unique_lock<std::mutex> lock(gMutex);
      FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "BEGIN: DALI_TEXT_PRECACHE_THREAD_SYNC_CREATION\n");
      gPreCacheThread = std::thread(PreCacheRun, fallbackFamilyList, extraFamilyList, localeFamily, syncCreation);
      gPreCacheCond.wait_for(lock, timeout, []{return gPreCacheThreadReady;});
      FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "END: DALI_TEXT_PRECACHE_THREAD_SYNC_CREATION\n");
    }
    else
    {
      gPreCacheThread = std::thread(PreCacheRun, fallbackFamilyList, extraFamilyList, localeFamily, syncCreation);
    }
  }
  else
  {
    GetImplementation(gPreCreatedFontClient).FontPreCache(fallbackFamilyList, extraFamilyList, localeFamily);
  }
}

void FontClient::PreLoad(const FontPathList& fontPathList, const FontPathList& memoryFontPathList, bool useThread, bool syncCreation)
{
  if(!gFontPreLoadAvailable)
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient font pre-load run failed, as a pre-loaded font client already exists.\n");
    return;
  }

  if(gPreLoadThread.joinable())
  {
    FONT_LOG_MESSAGE(Dali::Integration::Log::ERROR, "FontClient font pre-load thread already running.\n");
    return;
  }

  if(!gPreCreatedFontClient)
  {
    gPreCreatedFontClient = Dali::TextAbstraction::FontClient(new FontClient);
  }

  gFontPreLoadAvailable = false;

  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient Font PreLoad fontPathList       : %zu\n", fontPathList.size());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient Font PreLoad memoryFontPathList : %zu\n", memoryFontPathList.size());
  FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "FontClient Font PreLoad useThread          : %d\n", useThread);

  if(useThread)
  {
    if(syncCreation)
    {
      // The main thread wakes up upon receiving a notification from the pre-load thread.
      // If it doesn't receive a notification within the specified time, it wakes up due to a timeout.
      const std::chrono::milliseconds timeout(1000);
      gPreLoadThreadReady = false;
      std::unique_lock<std::mutex> lock(gMutex);
      FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "BEGIN: DALI_TEXT_FONT_PRELOAD_THREAD_SYNC_CREATION\n");
      gPreLoadThread = std::thread(PreLoadRun, fontPathList, memoryFontPathList, syncCreation);
      gPreLoadCond.wait_for(lock, timeout, []{return gPreLoadThreadReady;});
      FONT_LOG_MESSAGE(Dali::Integration::Log::INFO, "END: DALI_TEXT_FONT_PRELOAD_THREAD_SYNC_CREATION\n");
    }
    else
    {
      gPreLoadThread = std::thread(PreLoadRun, fontPathList, memoryFontPathList, syncCreation);
    }
  }
  else
  {
    GetImplementation(gPreCreatedFontClient).FontPreLoad(fontPathList, memoryFontPathList);
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

void FontClient::FontPreLoad(const FontPathList& fontPathList, const FontPathList& memoryFontPathList)
{
  CreatePlugin();

  mPlugin->FontPreLoad(fontPathList, memoryFontPathList);
}

void FontClient::InitDefaultFontDescription()
{
  CreatePlugin();

  mPlugin->InitDefaultFontDescription();
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

void FontClient::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::GlyphBufferData& data, int outlineWidth)
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
  std::scoped_lock lock(gMutex);
  if(!mPlugin)
  {
    mPlugin = new Plugin(mDpiHorizontal, mDpiVertical);
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
