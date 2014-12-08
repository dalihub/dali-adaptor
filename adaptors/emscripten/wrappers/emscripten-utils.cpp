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

#include "emscripten-utils.h"

// EXTERNAL INCLUDES


// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

Dali::Image CreateImageRGBA(unsigned int width, unsigned int height, const std::string& data)
{
  Dali::BufferImage b = Dali::BufferImage::New( width, height, Dali::Pixel::RGBA8888 );

  const Dali::PixelBuffer* from = reinterpret_cast<const Dali::PixelBuffer*>( data.c_str() );
  Dali::PixelBuffer* to = b.GetBuffer();

  unsigned int len = std::max( width * height * 4, data.size() );
  for(int i = 0; i < len; i++)
  {
    *to++ = *from++;
  }
  return b;
}

Dali::Image CreateImageRGB(unsigned int width, unsigned int height, const std::string& data)
{
  Dali::BufferImage b = Dali::BufferImage::New( width, height, Dali::Pixel::RGB888 );

  const Dali::PixelBuffer* from = reinterpret_cast<const Dali::PixelBuffer*>( data.c_str() );
  Dali::PixelBuffer* to = b.GetBuffer();

  unsigned int len = std::max( width * height * 3, data.size() );
  for(int i = 0; i < len; i++)
  {
    *to++ = *from++;
  }
  return b;
}

Dali::Image GetImage(const std::string& data)
{
  const uint8_t* const ptr = reinterpret_cast<const uint8_t * const>(data.c_str());
  return Dali::EncodedBufferImage::New(ptr, data.size());
}

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
