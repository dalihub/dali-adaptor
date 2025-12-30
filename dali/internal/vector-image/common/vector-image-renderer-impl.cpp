/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#ifndef THORVG_SUPPORT
const char* const UNITS("px");
#endif

// Type Registration
Dali::BaseHandle Create()
{
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::VectorImageRenderer), typeid(Dali::BaseHandle), Create);

#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorImageLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_IMAGE");
#endif

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
: mPicture(nullptr)
#else
: mParsedImage(nullptr),
  mRasterizer(nullptr)
#endif
{
}

VectorImageRenderer::~VectorImageRenderer()
{
  Mutex::ScopedLock lock(mMutex);
#ifdef THORVG_SUPPORT
#ifdef THORVG_VERSION_1
  if(mPicture)
  {
    tvg::Paint::rel(mPicture);
    mPicture = nullptr;
  }
  tvg::Initializer::term();
#else
  // NOTE: Initializer::term() will call clear() internally.
  // However, due to the delete on mPicture, a crash occurs for the paint
  // that has already been deleted in clear() of term().
  // Therefore, it temporarily performs a non-free clear().
  mSwCanvas->clear(false);

  if(mPicture)
  {
    delete(mPicture);
    mPicture = nullptr;
  }

  tvg::Initializer::term(tvg::CanvasEngine::Sw);
#endif
#else
  if(mParsedImage)
  {
    nsvgDelete(mParsedImage);
    mParsedImage = nullptr;
  }
  if(mRasterizer)
  {
    nsvgDeleteRasterizer(mRasterizer);
    mRasterizer = nullptr;
  }
#endif
}

void VectorImageRenderer::Initialize()
{
#ifdef THORVG_SUPPORT
#ifdef THORVG_VERSION_1
  tvg::Initializer::init(0);
  mSwCanvas = tvg::SwCanvas::gen();
#else
  tvg::Initializer::init(tvg::CanvasEngine::Sw, 0);

  mSwCanvas = tvg::SwCanvas::gen();
  mSwCanvas->mempool(tvg::SwCanvas::MempoolPolicy::Individual);
  mSwCanvas->reserve(1); // has one picture
#endif
#else
  mRasterizer = nsvgCreateRasterizer();
#endif
}

bool VectorImageRenderer::Load(const Vector<uint8_t>& data, float dpi)
{
  Mutex::ScopedLock lock(mMutex);

#ifdef THORVG_SUPPORT
  if(!mSwCanvas)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Load Canvas Object is null [%p]\n", this);
    return false;
  }

    if(!mPicture)
    {
#ifdef THORVG_VERSION_1
      mPicture = tvg::Picture::gen();
#else
      mPicture = tvg::Picture::gen().release();
#endif
      if(!mPicture)
      {
        DALI_LOG_ERROR("VectorImageRenderer::Load: Picture gen Fail [%p]\n", this);
        return false;
      }
    }
  else
  {
    return true;
  }

#ifdef THORVG_VERSION_1
  tvg::Result ret = mPicture->load(reinterpret_cast<char*>(data.Begin()), data.Size(), "svg", nullptr, true);
#else
  tvg::Result ret = mPicture->load(reinterpret_cast<char*>(data.Begin()), data.Size(), true);
#endif

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

    // Destroy mPicture and make it as nullptr, so we can notify that we fail to load svg file.
    if(mPicture)
    {
#ifdef THORVG_VERSION_1
      tvg::Paint::rel(mPicture);
#else
      delete(mPicture);
#endif
      mPicture = nullptr;
    }

    return false;
  }

  float w, h;
  mPicture->size(&w, &h);
  mDefaultWidth  = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

#else
  if(mParsedImage)
  {
    return true;
  }

  mParsedImage = nsvgParse(reinterpret_cast<char*>(data.Begin()), UNITS, dpi);
  if(!mParsedImage || !mParsedImage->shapes)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Load: nsvgParse failed\n");

    // Destroy mParsedImage and make it as nullptr, so we can notify that we fail to load svg file.
    if(mParsedImage)
    {
      nsvgDelete(mParsedImage);
      mParsedImage = nullptr;
    }

    return false;
  }

  mDefaultWidth  = mParsedImage->width;
  mDefaultHeight = mParsedImage->height;
#endif

  DALI_LOG_INFO(gVectorImageLogFilter, Debug::Verbose, "Load success! DefaultSize [%u x %u] [%p]\n", mDefaultWidth, mDefaultHeight, this);
  mIsLoaded.store(true);

  return true;
}

bool VectorImageRenderer::IsLoaded() const
{
  return mIsLoaded.load();
}

Dali::Devel::PixelBuffer VectorImageRenderer::Rasterize(uint32_t width, uint32_t height)
{
  Mutex::ScopedLock lock(mMutex);

  if(width == 0)
  {
    if(mDefaultWidth == 0)
    {
      DALI_LOG_ERROR("Invalid size [%d, %d]\n", width, height);
      return Devel::PixelBuffer();
    }
    else
    {
      width = mDefaultWidth;
    }
  }

  if(height == 0)
  {
    if(mDefaultHeight == 0)
    {
      DALI_LOG_ERROR("Invalid size [%d, %d]\n", width, height);
      return Devel::PixelBuffer();
    }
    else
    {
      height = mDefaultHeight;
    }
  }

#ifdef THORVG_SUPPORT
  if(!mSwCanvas || !mPicture)
  {
#ifdef THORVG_VERSION_1
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: either Canvas[%p] or Picture[%p] is invalid [%p]\n", mSwCanvas, mPicture, this);
#else
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: either Canvas[%p] or Picture[%p] is invalid [%p]\n", mSwCanvas.get(), mPicture, this);
#endif
    return Devel::PixelBuffer();
  }

  Devel::PixelBuffer pixelBuffer = Devel::PixelBuffer::New(width, height, Dali::Pixel::RGBA8888);

#ifdef THORVG_VERSION_1
  mSwCanvas->sync();
#else
  mSwCanvas->clear(false);
#endif

  auto pBuffer = pixelBuffer.GetBuffer();
  if(!pBuffer)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: pixel buffer is null [%p]\n", this);
    return Devel::PixelBuffer();
  }

#ifdef THORVG_VERSION_1
  mSwCanvas->target(reinterpret_cast<uint32_t*>(pBuffer), width, width, height, tvg::ColorSpace::ABGR8888);
#else
  mSwCanvas->target(reinterpret_cast<uint32_t*>(pBuffer), width, width, height, tvg::SwCanvas::ABGR8888);
#endif
  DALI_LOG_INFO(gVectorImageLogFilter, Debug::Verbose, "Buffer[%p] size[%d x %d]! [%p]\n", pBuffer, width, height, this);

  mPicture->size(width, height);

#ifdef THORVG_VERSION_1
  const auto& paintList = mSwCanvas->paints();
  if (std::find(paintList.begin(), paintList.end(), mPicture) == paintList.end())
  {
    if(mSwCanvas->add(mPicture) != tvg::Result::Success)
    {
      DALI_LOG_ERROR("VectorImageRenderer::Rasterize: Picture push fail [%p]\n", this);
      return Devel::PixelBuffer();
    }
  }

  auto ret = mSwCanvas->draw(true);
#else
  /* We can push everytime since we cleared the canvas just before. */
  if(mSwCanvas->push(std::unique_ptr<tvg::Picture>(mPicture)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: Picture push fail [%p]\n", this);
    return Devel::PixelBuffer();
  }

  auto ret = mSwCanvas->draw();
#endif

  if(ret != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorImageRenderer::Rasterize: Draw fail %d [%p]\n", static_cast<int>(ret), this);
    return Devel::PixelBuffer();
  }

  mSwCanvas->sync();

  return pixelBuffer;
#else
  if(mParsedImage != nullptr)
  {
    Devel::PixelBuffer pixelBuffer = Devel::PixelBuffer::New(width, height, Dali::Pixel::RGBA8888);

    float scaleX = static_cast<float>(width) / (mDefaultWidth > 0 ? static_cast<float>(mDefaultWidth) : 1.0f);
    float scaleY = static_cast<float>(height) / (mDefaultHeight > 0 ? static_cast<float>(mDefaultHeight) : 1.0f);
    float scale  = scaleX < scaleY ? scaleX : scaleY;
    int   stride = pixelBuffer.GetWidth() * Pixel::GetBytesPerPixel(Dali::Pixel::RGBA8888);

    nsvgRasterize(mRasterizer, mParsedImage, 0.0f, 0.0f, scale, pixelBuffer.GetBuffer(), width, height, stride);
    return pixelBuffer;
  }
  return Devel::PixelBuffer();
#endif
}

void VectorImageRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  if(IsLoaded())
  {
    width  = mDefaultWidth;
    height = mDefaultHeight;
  }
  else
  {
    DALI_LOG_ERROR("Image is not loaded yet. Default size will be set to 0x0\n");
    width  = 0;
    height = 0;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
