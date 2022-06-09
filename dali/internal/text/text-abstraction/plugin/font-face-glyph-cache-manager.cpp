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
 */

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>
#include <dali/internal/text/text-abstraction/plugin/font-face-glyph-cache-manager.h>

// EXTERNAL INCLUDES
#include FT_BITMAP_H

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
namespace
{
} // namespace

GlyphCacheManager::GlyphCacheManager(FT_Face ftFace, std::size_t maxNumberOfGlyphCache)
: mFreeTypeFace(ftFace),
  mGlyphCacheMaxSize(maxNumberOfGlyphCache),
  mLRUGlyphCache(mGlyphCacheMaxSize)
{
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager Create with maximum size : %d\n", static_cast<int>(mGlyphCacheMaxSize));
}
GlyphCacheManager::~GlyphCacheManager()
{
  while(!mLRUGlyphCache.IsEmpty())
  {
    auto removedData = mLRUGlyphCache.Pop();

    // Release Glyph data resource
    removedData.ReleaseGlyphData();
  }
  mLRUGlyphCache.Clear();
}

bool GlyphCacheManager::GetGlyphCacheDataFromIndex(
  const GlyphIndex& index,
  const FT_Int32&   flag,
  const bool&       isBoldRequired,
  GlyphCacheData&   glyphData,
  FT_Error&         error)
{
  // Append some error value here instead of FT_Err_Ok.
  error = static_cast<FT_Error>(-1);

  const GlyphCacheKey key  = GlyphCacheKey(index, flag, isBoldRequired);
  auto                iter = mLRUGlyphCache.Find(key);

  if(iter == mLRUGlyphCache.End())
  {
    // If cache size is full, remove oldest glyph.
    if(mLRUGlyphCache.IsFull())
    {
      auto removedData = mLRUGlyphCache.Pop();

      DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::GetGlyphCacheDataFromIndex. Remove oldest cache for glyph : %p\n", removedData.mGlyph);

      // Release Glyph data resource
      removedData.ReleaseGlyphData();
    }

    const bool loadSuccess = LoadGlyphDataFromIndex(index, flag, isBoldRequired, glyphData, error);
    if(loadSuccess)
    {
      // Copy and cached data.
      mLRUGlyphCache.Push(key, glyphData);

      DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::GetGlyphCacheDataFromIndex. Create cache for index : %u flag : %d isBold : %d isBitmap : %d, glyph : %p\n", index, static_cast<int>(flag), isBoldRequired, glyphData.mIsBitmap, glyphData.mGlyph);
    }

    return loadSuccess;
  }
  else
  {
    error = FT_Err_Ok;

    // We already notify that we use this glyph. And now, copy cached data.
    glyphData = iter->element;

    DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::GetGlyphCacheDataFromIndex. Find cache for index : %u flag : %d isBold : %d isBitmap : %d, glyph : %p\n", index, static_cast<int>(flag), isBoldRequired, glyphData.mIsBitmap, glyphData.mGlyph);
    return true;
  }
}

bool GlyphCacheManager::LoadGlyphDataFromIndex(
  const GlyphIndex& index,
  const FT_Int32&   flag,
  const bool&       isBoldRequired,
  GlyphCacheData&   glyphData,
  FT_Error&         error)
{
  error = FT_Load_Glyph(mFreeTypeFace, index, flag);
  if(FT_Err_Ok == error)
  {
    glyphData.mStyleFlags = mFreeTypeFace->style_flags;

    const bool isEmboldeningRequired = isBoldRequired && !(glyphData.mStyleFlags & FT_STYLE_FLAG_BOLD);
    if(isEmboldeningRequired)
    {
      // Does the software bold.
      FT_GlyphSlot_Embolden(mFreeTypeFace->glyph);
    }

    glyphData.mGlyphMetrics = mFreeTypeFace->glyph->metrics;
    glyphData.mIsBitmap     = false;
    // Load glyph
    error = FT_Get_Glyph(mFreeTypeFace->glyph, &glyphData.mGlyph);

    if(glyphData.mGlyph->format == FT_GLYPH_FORMAT_BITMAP)
    {
      // Copy original glyph infomation. Due to we use union, we should keep original handle.
      FT_Glyph bitmapGlyph = glyphData.mGlyph;

      // Copy rendered bitmap
      // TODO : Is there any way to keep bitmap buffer without copy?
      glyphData.mBitmap  = new FT_Bitmap();
      *glyphData.mBitmap = mFreeTypeFace->glyph->bitmap;

      // New allocate buffer
      size_t bufferSize = 0;
      switch(glyphData.mBitmap->pixel_mode)
      {
        case FT_PIXEL_MODE_GRAY:
        {
          if(glyphData.mBitmap->pitch == static_cast<int>(glyphData.mBitmap->width))
          {
            bufferSize = static_cast<size_t>(glyphData.mBitmap->width) * static_cast<size_t>(glyphData.mBitmap->rows);
          }
          break;
        }
#ifdef FREETYPE_BITMAP_SUPPORT
        case FT_PIXEL_MODE_BGRA:
        {
          if(glyphData.mBitmap->pitch == static_cast<int>(glyphData.mBitmap->width << 2u))
          {
            bufferSize = (static_cast<size_t>(glyphData.mBitmap->width) * static_cast<size_t>(glyphData.mBitmap->rows)) << 2u;
          }
          break;
        }
#endif
        default:
        {
          DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GlyphCacheManager::LoadGlyphDataFromIndex. FontClient Unable to create Bitmap of this PixelType\n");
          break;
        }
      }

      if(bufferSize > 0)
      {
        glyphData.mIsBitmap       = true;
        glyphData.mBitmap->buffer = new uint8_t[bufferSize];
        memcpy(glyphData.mBitmap->buffer, mFreeTypeFace->glyph->bitmap.buffer, bufferSize);
      }
      else
      {
        DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GlyphCacheManager::LoadGlyphDataFromIndex. Bitmap glyph buffer size is zero\n");
        delete glyphData.mBitmap;
        glyphData.mBitmap = nullptr;
        error             = static_cast<FT_Error>(-1);
      }

      // Release glyph data.
      FT_Done_Glyph(bitmapGlyph);
    }

    if(FT_Err_Ok == error)
    {
      return true;
    }
  }
  return false;
}

void GlyphCacheManager::ResizeBitmapGlyph(
  const GlyphIndex& index,
  const FT_Int32&   flag,
  const bool&       isBoldRequired,
  const uint32_t&   desiredWidth,
  const uint32_t&   desiredHeight)
{
  FT_Error       error;
  GlyphCacheData originGlyphData;
  if(GetGlyphCacheDataFromIndex(index, flag, isBoldRequired, originGlyphData, error))
  {
    if(DALI_LIKELY(originGlyphData.mIsBitmap && originGlyphData.mBitmap))
    {
      const bool requiredResize = (originGlyphData.mBitmap->rows != desiredHeight) || (originGlyphData.mBitmap->width != desiredWidth);
      if(requiredResize)
      {
        const GlyphCacheKey key  = GlyphCacheKey(index, flag, isBoldRequired);
        auto                iter = mLRUGlyphCache.Find(key);

        GlyphCacheData& destinationGlpyhData = iter->element;

        const ImageDimensions inputDimensions(destinationGlpyhData.mBitmap->width, destinationGlpyhData.mBitmap->rows);
        const ImageDimensions desiredDimensions(desiredWidth, desiredHeight);

        uint8_t* desiredBuffer = nullptr;

        switch(destinationGlpyhData.mBitmap->pixel_mode)
        {
          case FT_PIXEL_MODE_GRAY:
          {
            if(destinationGlpyhData.mBitmap->pitch == static_cast<int>(destinationGlpyhData.mBitmap->width))
            {
              desiredBuffer = new uint8_t[desiredWidth * desiredHeight];
              // Resize bitmap here.
              Dali::Internal::Platform::LanczosSample1BPP(destinationGlpyhData.mBitmap->buffer,
                                                          inputDimensions,
                                                          destinationGlpyhData.mBitmap->width,
                                                          desiredBuffer,
                                                          desiredDimensions);
            }
            break;
          }
#ifdef FREETYPE_BITMAP_SUPPORT
          case FT_PIXEL_MODE_BGRA:
          {
            if(destinationGlpyhData.mBitmap->pitch == static_cast<int>(destinationGlpyhData.mBitmap->width << 2u))
            {
              desiredBuffer = new uint8_t[(desiredWidth * desiredHeight) << 2u];
              // Resize bitmap here.
              Dali::Internal::Platform::LanczosSample4BPP(destinationGlpyhData.mBitmap->buffer,
                                                          inputDimensions,
                                                          destinationGlpyhData.mBitmap->width,
                                                          desiredBuffer,
                                                          desiredDimensions);
            }
            break;
          }
#endif
          default:
          {
            DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GlyphCacheManager::ResizeBitmapGlyph. FontClient Unable to create Bitmap of this PixelType\n");
            break;
          }
        }

        if(desiredBuffer)
        {
          // Success to resize bitmap glyph.
          // Release origin bitmap buffer.
          delete[] destinationGlpyhData.mBitmap->buffer;

          // Replace as desired buffer and size.
          destinationGlpyhData.mBitmap->buffer = desiredBuffer;
          destinationGlpyhData.mBitmap->width  = desiredWidth;
          destinationGlpyhData.mBitmap->rows   = desiredHeight;
          switch(destinationGlpyhData.mBitmap->pixel_mode)
          {
            case FT_PIXEL_MODE_GRAY:
            {
              destinationGlpyhData.mBitmap->pitch = desiredWidth;
              break;
            }
#ifdef FREETYPE_BITMAP_SUPPORT
            case FT_PIXEL_MODE_BGRA:
            {
              destinationGlpyhData.mBitmap->pitch = desiredWidth << 2u;
              break;
            }
#endif
          }
        }
      }
    }
  }
}

void GlyphCacheManager::GlyphCacheData::ReleaseGlyphData()
{
  if(mIsBitmap && mBitmap)
  {
    // Created FT_Bitmap object must be released with FT_Bitmap_Done
    delete[] mBitmap->buffer;
    delete mBitmap;
    mBitmap = nullptr;
  }
  else if(mGlyph)
  {
    // Created FT_Glyph object must be released with FT_Done_Glyph
    FT_Done_Glyph(mGlyph);
    mGlyph = nullptr;
  }
}

} // namespace Dali::TextAbstraction::Internal
