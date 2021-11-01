/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/internal/text/text-abstraction/plugin/embedded-item.h>

#include <dali/internal/text/text-abstraction/plugin/font-client-utils.h>

namespace Dali::TextAbstraction::Internal
{
void EmbeddedItem::GetGlyphMetrics(GlyphInfo& glyph)
{
  glyph.width       = static_cast<float>(width);
  glyph.height      = static_cast<float>(height);
  glyph.xBearing    = 0.f;
  glyph.yBearing    = glyph.height;
  glyph.advance     = glyph.width;
  glyph.scaleFactor = 1.f;
}

void EmbeddedItem::CreateBitmap(const std::vector<PixelBufferCacheItem>&            pixelBufferCache,
                                Dali::TextAbstraction::FontClient::GlyphBufferData& data)
{
  data.width  = width;
  data.height = height;
  if(0u != pixelBufferId)
  {
    Devel::PixelBuffer pixelBuffer = pixelBufferCache[pixelBufferId - 1u].pixelBuffer;
    if(pixelBuffer)
    {
      ConvertBitmap(data, pixelBuffer.GetWidth(), pixelBuffer.GetHeight(), pixelBuffer.GetBuffer());

      // Sets the pixel format.
      data.format = pixelBuffer.GetPixelFormat();
    }
  }
  else
  {
    // Creates the output buffer
    const unsigned int bufferSize = data.width * data.height * 4u;
    data.buffer                   = new unsigned char[bufferSize]; // @note The caller is responsible for deallocating the bitmap data using delete[].

    memset(data.buffer, 0u, bufferSize);

    // Just creates a void buffer. Doesn't matter what pixel format is set as is the application code the responsible of filling it.
  }
}

} // namespace Dali::TextAbstraction::Internal
