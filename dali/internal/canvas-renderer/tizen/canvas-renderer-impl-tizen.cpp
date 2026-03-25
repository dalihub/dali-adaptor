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
#include <dali/internal/canvas-renderer/tizen/canvas-renderer-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
#ifdef THORVG_SUPPORT
constexpr uint32_t CANVAS_RENDERER_QUEUE_SIZE = 2u; ///< The number of buffers in native image queue.
                                                    ///< Let we use double-buffer as default.
#endif

} // unnamed namespace

CanvasRendererPtr CanvasRendererTizen::New(const Vector2& viewBox)
{
  auto* canvasRenderer = new CanvasRendererTizen(viewBox);
  canvasRenderer->Initialize(viewBox);
  return canvasRenderer;
}

CanvasRendererTizen::CanvasRendererTizen(const Vector2& viewBox)
: CanvasRenderer(viewBox)
#ifdef THORVG_SUPPORT
  ,
  mNativeImageQueue(nullptr)
#endif
{
}

Dali::Texture CanvasRendererTizen::OnGetRasterizedTexture()
{
#ifdef THORVG_SUPPORT
  if(mNativeImageQueue)
  {
    if(!mRasterizedTexture)
    {
      mRasterizedTexture = Dali::Texture::New(*mNativeImageQueue);
    }
    return mRasterizedTexture;
  }
  else
  {
    return Dali::Texture();
  }
#else
  return Dali::Texture();
#endif
}

bool CanvasRendererTizen::OnRasterize()
{
#ifdef THORVG_SUPPORT
  Mutex::ScopedLock lock(mMutex);

  if(mNativeImageQueue)
  {
    bool canDequeue = mNativeImageQueue->CanDequeueBuffer();
    if(!canDequeue)
    {
      // Ignore the previous image which is inserted to the queue.
      mNativeImageQueue->IgnoreSourceImage();

      // Check again
      canDequeue = mNativeImageQueue->CanDequeueBuffer();
      if(!canDequeue)
      {
        return false;
      }
    }

    uint32_t width, height, stride;
    uint8_t* buffer = mNativeImageQueue->DequeueBuffer(width, height, stride);
    if(!buffer)
    {
      DALI_LOG_ERROR("Pixel buffer create to fail [%p]\n", this);
      return false;
    }

    mTvgCanvas->target(reinterpret_cast<uint32_t*>(buffer), stride / 4, width, height, tvg::SwCanvas::ARGB8888);

    mTvgCanvas->update(mTvgRoot);

    if(mTvgCanvas->draw() != tvg::Result::Success)
    {
      DALI_LOG_ERROR("ThorVG Draw fail [%p]\n", this);
      mNativeImageQueue->EnqueueBuffer(buffer);
      return false;
    }

    mTvgCanvas->sync();

    mNativeImageQueue->EnqueueBuffer(buffer);
  }
  else
  {
    return false;
  }

  return true;
#else
  return false;
#endif
}

void CanvasRendererTizen::OnMakeTargetBuffer(const Vector2& size)
{
#ifdef THORVG_SUPPORT
  if(!mNativeImageQueue)
  {
    mNativeImageQueue = Dali::NativeImageSourceQueue::New(CANVAS_RENDERER_QUEUE_SIZE, size.width, size.height, Dali::NativeImageSourceQueue::ColorFormat::BGRA8888);
  }
  else
  {
    mNativeImageQueue->SetSize(static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height));
  }
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
