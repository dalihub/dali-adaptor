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

// CLASS HEADER
#include <dali/internal/text/text-abstraction/plugin/font-face-glyph-cache-manager.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>

// EXTERNAL INCLUDES
#include FT_BITMAP_H

#if defined(DEBUG_ENABLED)
extern Dali::Integration::Log::Filter* gFontClientLogFilter;
#endif

namespace Dali::TextAbstraction::Internal
{
namespace
{
constexpr uint32_t THRESHOLD_WIDTH_FOR_RLE4_COMPRESSION = 8; // The smallest width of glyph that we use RLE4 method.
} // namespace

GlyphCacheManager::GlyphCacheManager(std::size_t maxNumberOfGlyphCache)
: mGlyphCacheMaxSize(maxNumberOfGlyphCache),
  mLRUGlyphCache(mGlyphCacheMaxSize)
{
  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager Create with maximum size : %d\n", static_cast<int>(mGlyphCacheMaxSize));
}

GlyphCacheManager::~GlyphCacheManager()
{
  ClearCache();
}

bool GlyphCacheManager::GetGlyphCacheDataFromIndex(
  const FT_Face    freeTypeFace,
  const GlyphIndex index,
  const FT_Int32   flag,
  const bool       isBoldRequired,
  GlyphCacheData&  glyphData,
  FT_Error&        error)
{
  // Append some error value here instead of FT_Err_Ok.
  error = static_cast<FT_Error>(-1);

  const GlyphCacheKey key  = GlyphCacheKey(freeTypeFace, index, flag, isBoldRequired);
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

    const bool loadSuccess = LoadGlyphDataFromIndex(freeTypeFace, index, flag, isBoldRequired, glyphData, error);
    if(loadSuccess)
    {
      // Copy and cached data.
      mLRUGlyphCache.Push(key, glyphData);

      DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::GetGlyphCacheDataFromIndex. Create cache for face : %p, index : %u flag : %d isBold : %d isBitmap : %d, glyph : %p\n", freeTypeFace, index, static_cast<int>(flag), isBoldRequired, glyphData.mIsBitmap, glyphData.mGlyph);
    }

    return loadSuccess;
  }
  else
  {
    error = FT_Err_Ok;

    // We already notify that we use this glyph. And now, copy cached data.
    glyphData = mLRUGlyphCache.GetElement(iter);

    DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::GetGlyphCacheDataFromIndex. Find cache for face : %p, index : %u flag : %d isBold : %d isBitmap : %d, glyph : %p\n", freeTypeFace, index, static_cast<int>(flag), isBoldRequired, glyphData.mIsBitmap, glyphData.mGlyph);
    return true;
  }
}

bool GlyphCacheManager::LoadGlyphDataFromIndex(
  const FT_Face    freeTypeFace,
  const GlyphIndex index,
  const FT_Int32   flag,
  const bool       isBoldRequired,
  GlyphCacheData&  glyphData,
  FT_Error&        error)
{
  error = FT_Load_Glyph(freeTypeFace, index, flag);
  if(FT_Err_Ok == error)
  {
    glyphData.mStyleFlags = freeTypeFace->style_flags;

    const bool isEmboldeningRequired = isBoldRequired && !(glyphData.mStyleFlags & FT_STYLE_FLAG_BOLD);
    if(isEmboldeningRequired)
    {
      // Does the software bold.
      FT_GlyphSlot_Embolden(freeTypeFace->glyph);
    }

    glyphData.mGlyphMetrics = freeTypeFace->glyph->metrics;
    glyphData.mIsBitmap     = false;
    // Load glyph
    error = FT_Get_Glyph(freeTypeFace->glyph, &glyphData.mGlyph);

    if(glyphData.mGlyph->format == FT_GLYPH_FORMAT_BITMAP)
    {
      // Copy original glyph infomation. Due to we use union, we should keep original handle.
      FT_Glyph bitmapGlyph = glyphData.mGlyph;

      // Copy rendered bitmap
      // TODO : Is there any way to keep bitmap buffer without copy?
      glyphData.mBitmap  = new FT_Bitmap();
      *glyphData.mBitmap = freeTypeFace->glyph->bitmap;

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
        glyphData.mBitmap->buffer = (uint8_t*)malloc(bufferSize * sizeof(uint8_t)); // @note The caller is responsible for deallocating the bitmap data using free.
        memcpy(glyphData.mBitmap->buffer, freeTypeFace->glyph->bitmap.buffer, bufferSize);
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
  const FT_Face    freeTypeFace,
  const GlyphIndex index,
  const FT_Int32   flag,
  const bool       isBoldRequired,
  const uint32_t   desiredWidth,
  const uint32_t   desiredHeight)
{
  if(desiredWidth * desiredHeight <= 0)
  {
    // Skip this API if desired size is zero
    return;
  }
  FT_Error       error;
  GlyphCacheData originGlyphData;
  if(GetGlyphCacheDataFromIndex(freeTypeFace, index, flag, isBoldRequired, originGlyphData, error))
  {
    if(DALI_LIKELY(originGlyphData.mIsBitmap && originGlyphData.mBitmap))
    {
      const bool requiredResize = (originGlyphData.mBitmap->rows != desiredHeight) || (originGlyphData.mBitmap->width != desiredWidth);
      if(requiredResize)
      {
        // originalGlyphData is copy data. For change cached information, we should access as iterator.
        const GlyphCacheKey key  = GlyphCacheKey(freeTypeFace, index, flag, isBoldRequired);
        auto                iter = mLRUGlyphCache.Find(key);

        GlyphCacheData& destinationGlpyhData = mLRUGlyphCache.GetElement(iter);

        const ImageDimensions inputDimensions(destinationGlpyhData.mBitmap->width, destinationGlpyhData.mBitmap->rows);
        const ImageDimensions desiredDimensions(desiredWidth, desiredHeight);

        uint8_t* desiredBuffer = nullptr;

        switch(destinationGlpyhData.mBitmap->pixel_mode)
        {
          case FT_PIXEL_MODE_GRAY:
          {
            if(destinationGlpyhData.mBitmap->pitch == static_cast<int>(destinationGlpyhData.mBitmap->width))
            {
              desiredBuffer = (uint8_t*)malloc(desiredWidth * desiredHeight * sizeof(uint8_t)); // @note The caller is responsible for deallocating the bitmap data using free.
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
              desiredBuffer = (uint8_t*)malloc((desiredWidth * desiredHeight * sizeof(uint8_t)) << 2u); // @note The caller is responsible for deallocating the bitmap data using free.
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
          free(destinationGlpyhData.mBitmap->buffer);

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

void GlyphCacheManager::CacheRenderedGlyphBuffer(
  const FT_Face               freeTypeFace,
  const GlyphIndex            index,
  const FT_Int32              flag,
  const bool                  isBoldRequired,
  const FT_Bitmap&            srcBitmap,
  const CompressionPolicyType policy)
{
  if(srcBitmap.width * srcBitmap.rows <= 0)
  {
    // Skip this API if rendered bitmap size is zero
    return;
  }
  FT_Error       error;
  GlyphCacheData originGlyphData;
  if(GetGlyphCacheDataFromIndex(freeTypeFace, index, flag, isBoldRequired, originGlyphData, error))
  {
    if(DALI_LIKELY(!originGlyphData.mIsBitmap && originGlyphData.mRenderedBuffer == nullptr))
    {
      // originalGlyphData is copy data. For change cached information, we should access as iterator.
      const GlyphCacheKey key  = GlyphCacheKey(freeTypeFace, index, flag, isBoldRequired);
      auto                iter = mLRUGlyphCache.Find(key);

      GlyphCacheData& destinationGlpyhData = mLRUGlyphCache.GetElement(iter);

      destinationGlpyhData.mRenderedBuffer = new TextAbstraction::FontClient::GlyphBufferData();
      if(DALI_UNLIKELY(!destinationGlpyhData.mRenderedBuffer))
      {
        DALI_LOG_ERROR("Allocate GlyphBufferData failed\n");
        return;
      }

      TextAbstraction::FontClient::GlyphBufferData& renderBuffer = *destinationGlpyhData.mRenderedBuffer;

      // Set basic informations.
      renderBuffer.width  = srcBitmap.width;
      renderBuffer.height = srcBitmap.rows;

      switch(srcBitmap.pixel_mode)
      {
        case FT_PIXEL_MODE_GRAY:
        {
          renderBuffer.format = Pixel::L8;

          if(policy == CompressionPolicyType::SPEED)
          {
            // If policy is SPEED, we will not compress bitmap.
            renderBuffer.compressionType = TextAbstraction::FontClient::GlyphBufferData::CompressionType::NO_COMPRESSION;
          }
          else
          {
            // If small enough glyph, compress as BPP4 method.
            if(srcBitmap.width < THRESHOLD_WIDTH_FOR_RLE4_COMPRESSION)
            {
              renderBuffer.compressionType = TextAbstraction::FontClient::GlyphBufferData::CompressionType::BPP_4;
            }
            else
            {
              renderBuffer.compressionType = TextAbstraction::FontClient::GlyphBufferData::CompressionType::RLE_4;
            }
          }

          const auto compressedBufferSize = TextAbstraction::FontClient::GlyphBufferData::Compress(srcBitmap.buffer, renderBuffer);
          if(DALI_UNLIKELY(compressedBufferSize == 0u))
          {
            DALI_ASSERT_DEBUG(0 == "Compress failed at FT_PIXEL_MODE_GRAY");
            DALI_LOG_ERROR("Compress failed. Ignore cache\n");
            delete destinationGlpyhData.mRenderedBuffer;
            destinationGlpyhData.mRenderedBuffer = nullptr;
            return;
          }
          break;
        }
#ifdef FREETYPE_BITMAP_SUPPORT
        case FT_PIXEL_MODE_BGRA:
        {
          // Copy buffer without compress
          renderBuffer.compressionType = TextAbstraction::FontClient::GlyphBufferData::CompressionType::NO_COMPRESSION;
          renderBuffer.format          = Pixel::BGRA8888;

          const auto compressedBufferSize = TextAbstraction::FontClient::GlyphBufferData::Compress(srcBitmap.buffer, renderBuffer);
          if(DALI_UNLIKELY(compressedBufferSize == 0u))
          {
            DALI_ASSERT_DEBUG(0 == "Compress failed at FT_PIXEL_MODE_BGRA");
            DALI_LOG_ERROR("Compress failed. Ignore cache\n");
            delete destinationGlpyhData.mRenderedBuffer;
            destinationGlpyhData.mRenderedBuffer = nullptr;
            return;
          }
          break;
        }
#endif
        default:
        {
          DALI_LOG_INFO(gFontClientLogFilter, Debug::General, "FontClient::Plugin::GlyphCacheManager::CacheRenderedGlyphBuffer. FontClient Unable to create Bitmap of this PixelType\n");
          delete destinationGlpyhData.mRenderedBuffer;
          destinationGlpyhData.mRenderedBuffer = nullptr;
          break;
        }
      }
    }
  }
}

void GlyphCacheManager::RemoveGlyphFromFace(const FT_Face freeTypeFace)
{
  uint32_t removedItemCount = 0;

  auto endIter = mLRUGlyphCache.End();
  for(auto iter = mLRUGlyphCache.Begin(); iter != endIter;)
  {
    // Check whether this cached item has inputed freeTypeFace as key.
    auto keyFace = mLRUGlyphCache.GetKey(iter).mFreeTypeFace;
    if(keyFace == freeTypeFace)
    {
      ++removedItemCount;
      iter = mLRUGlyphCache.Erase(iter);
    }
    else
    {
      ++iter;
    }
  }

  DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::RemoveGlyphFromFace. Remove all cached glyph with face : %p, removed glyph count : %u\n", freeTypeFace, removedItemCount);
}

void GlyphCacheManager::ClearCache(const std::size_t remainCount)
{
  if(remainCount == 0u)
  {
    // Release all data memory first.
    auto endIter = mLRUGlyphCache.End();
    for(auto iter = mLRUGlyphCache.Begin(); iter != endIter; ++iter)
    {
      // Get the reference of data. and release it.
      auto& removedData = mLRUGlyphCache.GetElement(iter);
      removedData.ReleaseGlyphData();
    }

    // Clear all cache.
    mLRUGlyphCache.Clear();
  }
  else
  {
    // While the cache count is bigger than remainCount, remove oldest glyph.
    while(mLRUGlyphCache.Count() > remainCount)
    {
      auto removedData = mLRUGlyphCache.Pop();

      DALI_LOG_INFO(gFontClientLogFilter, Debug::Verbose, "FontClient::Plugin::GlyphCacheManager::ClearCache[%zu / %zu]. Remove oldest cache for glyph : %p\n", mLRUGlyphCache.Count(), remainCount, removedData.mGlyph);

      // Release Glyph data resource
      removedData.ReleaseGlyphData();
    }
  }
}

void GlyphCacheManager::GlyphCacheData::ReleaseGlyphData()
{
  if(mIsBitmap && mBitmap)
  {
    // Created FT_Bitmap object must be released with FT_Bitmap_Done
    free(mBitmap->buffer); // This buffer created by malloc

    delete mBitmap;
    mBitmap = nullptr;
  }
  else if(mGlyph)
  {
    // Created FT_Glyph object must be released with FT_Done_Glyph
    FT_Done_Glyph(mGlyph);
    mGlyph = nullptr;
  }

  if(mRenderedBuffer)
  {
    delete mRenderedBuffer;
    mRenderedBuffer = nullptr;
  }

  mStyleFlags = 0;
}

} // namespace Dali::TextAbstraction::Internal
