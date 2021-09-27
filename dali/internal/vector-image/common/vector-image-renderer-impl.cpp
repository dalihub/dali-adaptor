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
 *
 */

// CLASS HEADER
#include <dali/internal/vector-image/common/vector-image-renderer-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <third-party/nanosvg/nanosvg.h>
#include <third-party/nanosvg/nanosvgrast.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
const char* const UNITS("px");

// Type Registration
Dali::BaseHandle Create()
{
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::VectorImageRenderer), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

VectorImageRendererPtr VectorImageRenderer::New()
{
  VectorImageRendererPtr renderer = new VectorImageRenderer();
  if(renderer)
  {
    renderer->Initialize();
  }
  return renderer;
}

VectorImageRenderer::VectorImageRenderer()
: mPlugin(std::string()),
  mParsedImage(nullptr),
  mRasterizer(nullptr)
{
}

VectorImageRenderer::~VectorImageRenderer()
{
  if(mParsedImage)
  {
    nsvgDelete(mParsedImage);
  }

  if(mRasterizer)
  {
    nsvgDeleteRasterizer(mRasterizer);
  }
}

void VectorImageRenderer::Initialize()
{
  if(!mPlugin.IsValid())
  {
    mRasterizer = nsvgCreateRasterizer();
  }
}

bool VectorImageRenderer::Load(const Vector<uint8_t>& data, float dpi)
{
  if(mPlugin.IsValid())
  {
    return mPlugin.Load(data);
  }
  else
  {
    mParsedImage = nsvgParse(reinterpret_cast<char*>(data.Begin()), UNITS, dpi);
    if(!mParsedImage || !mParsedImage->shapes)
    {
      DALI_LOG_ERROR("VectorImageRenderer::Load: nsvgParse failed\n");
      return false;
    }
    return true;
  }
}

bool VectorImageRenderer::Rasterize(Dali::Devel::PixelBuffer& buffer, float scale)
{
  if(mPlugin.IsValid())
  {
    return mPlugin.Rasterize(buffer);
  }
  else
  {
    if(mParsedImage != nullptr)
    {
      int stride = buffer.GetWidth() * Pixel::GetBytesPerPixel(buffer.GetPixelFormat());
      nsvgRasterize(mRasterizer, mParsedImage, 0.0f, 0.0f, scale, buffer.GetBuffer(), buffer.GetWidth(), buffer.GetHeight(), stride);
      return true;
    }
    return false;
  }
}

void VectorImageRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  if(mPlugin.IsValid())
  {
    mPlugin.GetDefaultSize(width, height);
  }
  else
  {
    if(mParsedImage)
    {
      width  = mParsedImage->width;
      height = mParsedImage->height;
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
