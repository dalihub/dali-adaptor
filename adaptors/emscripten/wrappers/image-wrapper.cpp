/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include "image-wrapper.h"

// EXTERNAL INCLUDES

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

Dali::BufferImage BufferImageNew(const std::string& data, unsigned int width, unsigned int height, Dali::Pixel::Format pixelFormat)
{
  Dali::BufferImage b = Dali::BufferImage::New( width, height, pixelFormat );

  const Dali::PixelBuffer* from = reinterpret_cast<const Dali::PixelBuffer*>( data.c_str() );
  Dali::PixelBuffer* to = b.GetBuffer();

  unsigned int len = std::min( width * height * GetBytesPerPixel(pixelFormat), data.size() );
  for(int i = 0; i < len; i++)
  {
    *to++ = *from++;
  }
  return b;
}

Dali::EncodedBufferImage EncodedBufferImageNew(const std::string& data)
{
  const uint8_t* const ptr = reinterpret_cast<const uint8_t * const>(data.c_str());
  return Dali::EncodedBufferImage::New(ptr, data.size());
}

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
