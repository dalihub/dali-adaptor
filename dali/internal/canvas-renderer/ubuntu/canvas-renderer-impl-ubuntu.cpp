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
#include <dali/internal/canvas-renderer/ubuntu/canvas-renderer-impl-ubuntu.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
// Type Registration
Dali::BaseHandle Create()
{
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

CanvasRendererUbuntu* CanvasRendererUbuntu::New(const Vector2& viewBox)
{
  return new CanvasRendererUbuntu(viewBox);
}

CanvasRendererUbuntu::CanvasRendererUbuntu(const Vector2& viewBox)
: mPixelBuffer(nullptr),
#ifdef THORVG_SUPPORT
  mTvgCanvas(nullptr),
  mTvgRoot(nullptr),
#endif
  mSize(0, 0),
  mViewBox(0, 0),
  mChanged(false)
{
  Initialize(viewBox);
}

CanvasRendererUbuntu::~CanvasRendererUbuntu()
{
#ifdef THORVG_SUPPORT
  mDrawables.clear();

  //Terminate ThorVG Engine
  tvg::Initializer::term(tvg::CanvasEngine::Sw);
#endif
}

void CanvasRendererUbuntu::Initialize(const Vector2& viewBox)
{
#ifdef THORVG_SUPPORT
  if(tvg::Initializer::init(tvg::CanvasEngine::Sw, 0 /*threads*/) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("ThorVG engine initialize failed\n");
  }
  mTvgCanvas = tvg::SwCanvas::gen();

  mSize = mViewBox = viewBox;
  if(viewBox.width < 1.0f || viewBox.height < 1.0f)
  {
    return;
  }

  MakeTargetBuffer(mSize);
#endif
}

bool CanvasRendererUbuntu::Commit()
{
#ifdef THORVG_SUPPORT
  bool changed = false;

  for(auto& it : mDrawables)
  {
    Internal::Adaptor::Drawable& drawableImpl = GetImplementation(it);
    if(drawableImpl.GetChanged())
    {
      changed = true;
      drawableImpl.SetChanged(false);
    }
  }

  if(!changed && !mChanged)
  {
    return false;
  }
  else
  {
    if(!mPixelBuffer.GetBuffer())
    {
      MakeTargetBuffer(mSize);
      mChanged = false;
    }
  }

  if(mSize.width < 1.0f || mSize.height < 1.0f)
  {
    DALI_LOG_ERROR("Size is zero [%p]\n", this);
    return false;
  }

  if(mTvgCanvas->clear() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("ThorVG canvas clear fail [%p]\n", this);
    return false;
  }

  auto scene = tvg::Scene::gen();
  mTvgRoot   = scene.get();
  for(auto& it : mDrawables)
  {
    PushDrawableToGroup(it, mTvgRoot);
  }

  if(mTvgCanvas->push(move(scene)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("ThorVG canvas push fail [%p]\n", this);
    return false;
  }

  if(mViewBox != mSize)
  {
    auto scaleX = mSize.width / mViewBox.width;
    auto scaleY = mSize.height / mViewBox.height;
    mTvgRoot->scale(scaleX < scaleY ? scaleX : scaleY);
  }

  mTvgCanvas->update(mTvgRoot);

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

Devel::PixelBuffer CanvasRendererUbuntu::GetPixelBuffer()
{
  return mPixelBuffer;
}

bool CanvasRendererUbuntu::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
#ifdef THORVG_SUPPORT
  Internal::Adaptor::Drawable& drawableImpl = GetImplementation(drawable);
  if(drawableImpl.IsAdded())
  {
    DALI_LOG_ERROR("Already added [%p][%p]\n", this, &drawable);
    return false;
  }

  if(mSize.width < 1.0f || mSize.height < 1.0f)
  {
    DALI_LOG_ERROR("Size is zero [%p]\n", this);
    return false;
  }

  drawableImpl.SetAdded(true);
  mDrawables.push_back(drawable);
  mChanged = true;

  return true;
#else
  return false;
#endif
}

bool CanvasRendererUbuntu::SetSize(const Vector2& size)
{
  if(size.width < 1.0f || size.height < 1.0f)
  {
    return false;
  }

  if(size != mSize)
  {
    mSize = size;
    MakeTargetBuffer(size);
  }

  mChanged = true;

  return true;
}

const Vector2& CanvasRendererUbuntu::GetSize()
{
  return mSize;
}

void CanvasRendererUbuntu::MakeTargetBuffer(const Vector2& size)
{
#ifdef THORVG_SUPPORT
  mPixelBuffer = Devel::PixelBuffer::New(size.width, size.height, Dali::Pixel::RGBA8888);

  unsigned char* pBuffer;
  pBuffer = mPixelBuffer.GetBuffer();

  if(!pBuffer)
  {
    DALI_LOG_ERROR("Pixel buffer create to fail [%p]\n", this);
    return;
  }

  mTvgCanvas->target(reinterpret_cast<uint32_t*>(pBuffer), size.width, size.width, size.height, tvg::SwCanvas::ABGR8888);
#endif
}

#ifdef THORVG_SUPPORT
void CanvasRendererUbuntu::PushDrawableToGroup(Dali::CanvasRenderer::Drawable& drawable, tvg::Scene* group)
{
  Internal::Adaptor::Drawable& drawableImpl        = Dali::GetImplementation(drawable);
  tvg::Paint*                  tvgDuplicatedObject = static_cast<tvg::Paint*>(drawableImpl.GetObject())->duplicate();
  if(!tvgDuplicatedObject)
  {
    DALI_LOG_ERROR("Invalid drawable object [%p]\n", this);
    return;
  }
  Drawable::Types type = drawableImpl.GetType();

  if(type == Drawable::Types::DRAWABLE_GROUP)
  {
    Dali::CanvasRenderer::DrawableGroup& group             = static_cast<Dali::CanvasRenderer::DrawableGroup&>(drawable);
    Internal::Adaptor::DrawableGroup&    drawableGroupImpl = Dali::GetImplementation(group);
    DrawableGroup::DrawableVector        drawables         = drawableGroupImpl.GetDrawables();
    for(auto& it : drawables)
    {
      PushDrawableToGroup(it, static_cast<tvg::Scene*>(tvgDuplicatedObject));
    }
  }

  if(group->push(std::move(std::unique_ptr<tvg::Paint>(tvgDuplicatedObject))) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Tvg push fail [%p]\n", this);
    return;
  }
}
#endif

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
