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
#if !(defined(DALI_PROFILE_UBUNTU) || defined(ANDROID) || defined(WIN32) || defined(__APPLE__))
#include <vconf.h>
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-plugin-impl.h>

#include <dali/devel-api/text-abstraction/glyph-info.h>

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
Dali::TextAbstraction::FontClient FontClient::gPreInitializedFontClient(NULL);

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
      if(gPreInitializedFontClient)
      {
        fontClientHandle = gPreInitializedFontClient;
        gPreInitializedFontClient.Reset(); // No longer needed
      }
      else
      {
        fontClientHandle = Dali::TextAbstraction::FontClient(new FontClient);
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

#if !(defined(DALI_PROFILE_UBUNTU) || defined(ANDROID) || defined(WIN32) || defined(__APPLE__))
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
