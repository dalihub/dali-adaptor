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
#include <dali/internal/canvas-renderer/generic/canvas-renderer-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
} // unnamed namespace

CanvasRendererPtr CanvasRendererGeneric::New(const Vector2& viewBox)
{
  auto* canvasRenderer = new CanvasRendererGeneric(viewBox);
  canvasRenderer->Initialize(viewBox);
  return canvasRenderer;
}

CanvasRendererGeneric::CanvasRendererGeneric(const Vector2& viewBox)
: CanvasRenderer(viewBox)
#ifdef THORVG_SUPPORT
  ,
  mPixelBuffer(nullptr)
#endif
{
}

Dali::Texture CanvasRendererGeneric::OnGetRasterizedTexture()
{
#ifdef THORVG_SUPPORT
  if(mPixelBuffer)
  {
    auto width  = mPixelBuffer.GetWidth();
    auto height = mPixelBuffer.GetHeight();
    if(width <= 0 || height <= 0)
    {
      return Dali::Texture();
    }

    Dali::PixelData pixelData = Devel::PixelBuffer::Convert(mPixelBuffer);

    if(!mRasterizedTexture || mRasterizedTexture.GetWidth() != width || mRasterizedTexture.GetHeight() != height)
    {
      mRasterizedTexture = Dali::Texture::New(Dali::TextureType::TEXTURE_2D, Dali::Pixel::BGRA8888, width, height);
    }

    mRasterizedTexture.Upload(pixelData);
  }
  return mRasterizedTexture;
#else
  return Dali::Texture();
#endif
}

bool CanvasRendererGeneric::OnRasterize()
{
#ifdef THORVG_SUPPORT
  Mutex::ScopedLock lock(mMutex);

  if(mTvgCanvas->draw() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("ThorVG Draw fail [%p]\n", this);
    return false;
  }

  mTvgCanvas->sync();

  return true;
#else
  return false;
#endif
}

void CanvasRendererGeneric::OnMakeTargetBuffer(const Vector2& size)
{
#ifdef THORVG_SUPPORT
  mPixelBuffer = Devel::PixelBuffer::New(size.width, size.height, Dali::Pixel::BGRA8888);

  unsigned char* pBuffer;
  pBuffer = mPixelBuffer.GetBuffer();

  if(!pBuffer)
  {
    DALI_LOG_ERROR("Pixel buffer create to fail [%p]\n", this);
    return;
  }

  mTvgCanvas->target(reinterpret_cast<uint32_t*>(pBuffer), size.width, size.width, size.height, tvg::SwCanvas::ARGB8888);
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
