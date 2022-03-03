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
#include <dali/internal/vector-image/common/vector-image-renderer-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

#ifndef THORVG_SUPPORT
// INTERNAL INCLUDES
#include <third-party/nanosvg/nanosvg.h>
#include <third-party/nanosvg/nanosvgrast.h>
#endif

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
#ifdef THORVG_SUPPORT
: mPicture(nullptr),
  mDefaultWidth(0),
  mDefaultHeight(0)
#else
: mParsedImage(nullptr),
  mRasterizer(nullptr)
#endif
{
}

VectorImageRenderer::~VectorImageRenderer()
{
#ifdef THORVG_SUPPORT

  //NOTE: Initializer::term() will call clear() internally.
  //However, due to the delete on mPicture, a crash occurs for the paint
  //that has already been deleted in clear() of term().
  //Therefore, it temporarily performs a non-free clear().
  mSwCanvas->clear(false);

  if(mPicture)
  {
    delete(mPicture);
  }

  tvg::Initializer::term(tvg::CanvasEngine::Sw);
#else
  if(mParsedImage)
  {
    nsvgDelete(mParsedImage);
  }

  if(mRasterizer)
  {
    nsvgDeleteRasterizer(mRasterizer);
  }
#endif
}

void VectorImageRenderer::Initialize()
{
#ifdef THORVG_SUPPORT
  tvg::Initializer::init(tvg::CanvasEngine::Sw, 0);

  mSwCanvas = tvg::SwCanvas::gen();
  mSwCanvas->mempool(tvg::SwCanvas::MempoolPolicy::Individual);
  mSwCanvas->reserve(1); //has one picture
#else
  mRasterizer  = nsvgCreateRasterizer();
#endif
}

bool VectorImageRenderer::Load(const Vector<uint8_t>& data, float dpi)
{
#ifdef THORVG_SUPPORT
  if(!mSwCanvas)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Load Canvas Object is null [%p]\n", this);
    return false;
  }

  if(!mPicture)
  {
    mPicture = tvg::Picture::gen().release();
    if(!mPicture)
    {
      DALI_LOG_ERROR("VectorImageRenderer::Load: Picture gen Fail [%p]\n", this);
      return false;
    }
  }

  tvg::Result ret = mPicture->load(reinterpret_cast<char*>(data.Begin()), data.Size(), true);

  if(ret != tvg::Result::Success)
  {
    switch(ret)
    {
      case tvg::Result::InvalidArguments:
      {
        DALI_LOG_ERROR("VectorImageRenderer::Load Load fail(Invalid arguments) Size:%d [%p]\n", data.Size(), this);
        break;
      }
      case tvg::Result::NonSupport:
      {
        DALI_LOG_ERROR("VectorImageRenderer::Load Load fail(Invalid SVG) Size:%d [%p]\n", data.Size(), this);
        break;
      }
      case tvg::Result::Unknown:
      {
        DALI_LOG_ERROR("VectorImageRenderer::Load Load fail(Parse fail) Size:%d [%p]\n", data.Size(), this);
        break;
      }
      default:
      {
        DALI_LOG_ERROR("VectorImageRenderer::Load Load fail / Size:%d [%p]\n", data.Size(), this);
        break;
      }
    }
    return false;
  }

  float w, h;
  mPicture->size(&w, &h);
  mDefaultWidth  = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

  return true;
#else
  mParsedImage = nsvgParse(reinterpret_cast<char*>(data.Begin()), UNITS, dpi);
  if(!mParsedImage || !mParsedImage->shapes)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Load: nsvgParse failed\n");
    return false;
  }
  return true;
#endif
}

bool VectorImageRenderer::Rasterize(Dali::Devel::PixelBuffer& buffer, float scale)
{
#ifdef THORVG_SUPPORT
  if(!mSwCanvas || !mPicture)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: either Canvas[%p] or Picture[%p] is invalid [%p]\n", mSwCanvas.get(), mPicture, this);
    return false;
  }

  mSwCanvas->clear(false);

  auto pBuffer = buffer.GetBuffer();
  if(!pBuffer)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: pixel buffer is null [%p]\n", this);
    return false;
  }

  auto width  = buffer.GetWidth();
  auto height = buffer.GetHeight();

  mSwCanvas->target(reinterpret_cast<uint32_t*>(pBuffer), width, width, height, tvg::SwCanvas::ABGR8888);

  DALI_LOG_RELEASE_INFO("VectorImageRenderer::Rasterize: Buffer[%p] size[%d x %d]! [%p]\n", pBuffer, width, height, this);

  mPicture->size(width, height);

  /* We can push everytime since we cleared the canvas just before. */
  if(mSwCanvas->push(std::unique_ptr<tvg::Picture>(mPicture)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: Picture push fail [%p]\n", this);
    return false;
  }

  auto ret = mSwCanvas->draw();
  if(ret != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: Draw fail %d [%p]\n", static_cast<int>(ret), this);
    return false;
  }

  mSwCanvas->sync();

  return true;
#else
  if(mParsedImage != nullptr)
  {
    int stride = buffer.GetWidth() * Pixel::GetBytesPerPixel(buffer.GetPixelFormat());
    nsvgRasterize(mRasterizer, mParsedImage, 0.0f, 0.0f, scale, buffer.GetBuffer(), buffer.GetWidth(), buffer.GetHeight(), stride);
    return true;
  }
  return false;
#endif
}

void VectorImageRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
#ifdef THORVG_SUPPORT
  width  = mDefaultWidth;
  height = mDefaultHeight;
#else
  if(mParsedImage)
  {
    width  = mParsedImage->width;
    height = mParsedImage->height;
  }
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
