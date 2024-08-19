/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>
#include <dali/internal/canvas-renderer/common/gradient-impl.h>
#include <dali/internal/canvas-renderer/common/shape-impl.h>

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

CanvasRendererTizen* CanvasRendererTizen::New(const Vector2& viewBox)
{
  return new CanvasRendererTizen(viewBox);
}

CanvasRendererTizen::CanvasRendererTizen(const Vector2& viewBox)
:
#ifdef THORVG_SUPPORT
  mRasterizedTexture(),
  mMutex(),
  mTvgCanvas(nullptr),
  mTvgRoot(nullptr),
  mNativeImageQueue(nullptr),
#endif
  mSize(Vector2::ZERO),
  mViewBox(Vector2::ZERO),
  mChanged(false)
{
  Initialize(viewBox);
}

CanvasRendererTizen::~CanvasRendererTizen()
{
#ifdef THORVG_SUPPORT
  mDrawables.clear();

  //Terminate ThorVG Engine
  tvg::Initializer::term(tvg::CanvasEngine::Sw);
#endif
}

void CanvasRendererTizen::Initialize(const Vector2& viewBox)
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

bool CanvasRendererTizen::Commit()
{
#ifdef THORVG_SUPPORT
  Mutex::ScopedLock lock(mMutex);

  if(mSize.width < 1.0f || mSize.height < 1.0f)
  {
    DALI_LOG_ERROR("Size is zero [%p]\n", this);
    return false;
  }

  bool changed = false;

  for(auto& it : mDrawables)
  {
    if(HaveDrawablesChanged(it))
    {
      UpdateDrawablesChanged(it, false);
      changed = true;
    }
  }

  if(!changed && !mChanged)
  {
    return false;
  }
  else
  {
    MakeTargetBuffer(mSize);
    mChanged = false;
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

  if(mViewBox != mSize && mViewBox.width > 0 && mViewBox.height > 0)
  {
    auto scaleX = mSize.width / mViewBox.width;
    auto scaleY = mSize.height / mViewBox.height;
    mTvgRoot->scale(scaleX < scaleY ? scaleX : scaleY);
  }

  if(mTvgCanvas->push(move(scene)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("ThorVG canvas push fail [%p]\n", this);
    return false;
  }

  return true;
#else
  return false;
#endif
}

Dali::Texture CanvasRendererTizen::GetRasterizedTexture()
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

bool CanvasRendererTizen::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
#ifdef THORVG_SUPPORT
  Internal::Adaptor::Drawable& drawableImpl = GetImplementation(drawable);
  if(drawableImpl.IsAdded())
  {
    DALI_LOG_ERROR("Already added [%p][%p]\n", this, &drawable);
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

#ifdef THORVG_SUPPORT
bool CanvasRendererTizen::HaveDrawablesChanged(const Dali::CanvasRenderer::Drawable& drawable) const
{
  const Internal::Adaptor::Drawable& drawableImpl = GetImplementation(drawable);
  if(drawableImpl.GetChanged())
  {
    return true;
  }
  Dali::CanvasRenderer::Drawable compositeDrawable = drawableImpl.GetCompositionDrawable();
  if(DALI_UNLIKELY(compositeDrawable))
  {
    Internal::Adaptor::Drawable& compositeDrawableImpl = Dali::GetImplementation(compositeDrawable);
    if(compositeDrawableImpl.GetChanged())
    {
      return true;
    }
  }

  if(drawableImpl.GetType() == Drawable::Types::DRAWABLE_GROUP)
  {
    const Dali::CanvasRenderer::DrawableGroup& group             = static_cast<const Dali::CanvasRenderer::DrawableGroup&>(drawable);
    const Internal::Adaptor::DrawableGroup&    drawableGroupImpl = Dali::GetImplementation(group);
    DrawableGroup::DrawableVector              drawables         = drawableGroupImpl.GetDrawables();
    for(auto& it : drawables)
    {
      if(HaveDrawablesChanged(it))
      {
        return true;
      }
    }
  }
  else if(drawableImpl.GetType() == Drawable::Types::SHAPE)
  {
    const Dali::CanvasRenderer::Shape& shape        = static_cast<const Dali::CanvasRenderer::Shape&>(drawable);
    Dali::CanvasRenderer::Gradient     fillGradient = shape.GetFillGradient();
    if(DALI_UNLIKELY(fillGradient))
    {
      Internal::Adaptor::Gradient& fillGradientImpl = Dali::GetImplementation(fillGradient);
      if(fillGradientImpl.GetChanged())
      {
        return true;
      }
    }

    Dali::CanvasRenderer::Gradient strokeGradient = shape.GetStrokeGradient();
    if(DALI_UNLIKELY(strokeGradient))
    {
      Internal::Adaptor::Gradient& strokeGradientImpl = Dali::GetImplementation(strokeGradient);
      if(strokeGradientImpl.GetChanged())
      {
        return true;
      }
    }
  }

  return false;
}

void CanvasRendererTizen::UpdateDrawablesChanged(Dali::CanvasRenderer::Drawable& drawable, bool changed)
{
  Internal::Adaptor::Drawable& drawableImpl = GetImplementation(drawable);
  drawableImpl.SetChanged(changed);

  Dali::CanvasRenderer::Drawable compositeDrawable = drawableImpl.GetCompositionDrawable();
  if(DALI_UNLIKELY(compositeDrawable))
  {
    Internal::Adaptor::Drawable& compositeDrawableImpl = Dali::GetImplementation(compositeDrawable);
    compositeDrawableImpl.SetChanged(changed);
  }

  if(drawableImpl.GetType() == Drawable::Types::DRAWABLE_GROUP)
  {
    Dali::CanvasRenderer::DrawableGroup& group             = static_cast<Dali::CanvasRenderer::DrawableGroup&>(drawable);
    Internal::Adaptor::DrawableGroup&    drawableGroupImpl = Dali::GetImplementation(group);
    DrawableGroup::DrawableVector        drawables         = drawableGroupImpl.GetDrawables();
    for(auto& it : drawables)
    {
      UpdateDrawablesChanged(it, changed);
    }
  }
  else if(drawableImpl.GetType() == Drawable::Types::SHAPE)
  {
    Dali::CanvasRenderer::Shape&   shape        = static_cast<Dali::CanvasRenderer::Shape&>(drawable);
    Dali::CanvasRenderer::Gradient fillGradient = shape.GetFillGradient();
    if(DALI_UNLIKELY(fillGradient))
    {
      Internal::Adaptor::Gradient& fillGradientImpl = Dali::GetImplementation(fillGradient);
      fillGradientImpl.SetChanged(changed);
    }

    Dali::CanvasRenderer::Gradient strokeGradient = shape.GetStrokeGradient();
    if(DALI_UNLIKELY(strokeGradient))
    {
      Internal::Adaptor::Gradient& strokeGradientImpl = Dali::GetImplementation(strokeGradient);
      strokeGradientImpl.SetChanged(changed);
    }
  }
}
#endif

bool CanvasRendererTizen::IsCanvasChanged() const
{
#ifdef THORVG_SUPPORT
  if(mChanged)
  {
    return true;
  }

  for(auto& it : mDrawables)
  {
    if(HaveDrawablesChanged(it))
    {
      return true;
    }
  }
#endif
  return false;
}

bool CanvasRendererTizen::Rasterize()
{
#ifdef THORVG_SUPPORT
  Mutex::ScopedLock lock(mMutex);

  if(mNativeImageQueue && mNativeImageQueue->CanDequeueBuffer())
  {
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

bool CanvasRendererTizen::RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
#ifdef THORVG_SUPPORT
  DrawableGroup::DrawableVector::iterator it = std::find(mDrawables.begin(), mDrawables.end(), drawable);
  if(it != mDrawables.end())
  {
    Internal::Adaptor::Drawable& drawableImpl = GetImplementation(drawable);
    drawableImpl.SetAdded(false);

    mDrawables.erase(it);
    mChanged = true;

    return true;
  }

#endif
  return false;
}

bool CanvasRendererTizen::RemoveAllDrawables()
{
#ifdef THORVG_SUPPORT
  for(auto& it : mDrawables)
  {
    Internal::Adaptor::Drawable& drawableImpl = GetImplementation(it);
    drawableImpl.SetAdded(false);
  }

  mDrawables.clear();
  mChanged = true;

  return true;
#else
  return false;
#endif
}

bool CanvasRendererTizen::SetSize(Vector2 size)
{
  if(size.width < 1.0f || size.height < 1.0f)
  {
    return false;
  }

  if(size != mSize)
  {
    mSize = size;
    if(mViewBox == Vector2::ZERO)
    {
      mViewBox = size;
    }
    mChanged = true;
  }

  return true;
}

Vector2 CanvasRendererTizen::GetSize() const
{
  return mSize;
}

bool CanvasRendererTizen::SetViewBox(const Vector2& viewBox)
{
  if(viewBox.width < 1.0f || viewBox.height < 1.0f)
  {
    return false;
  }

  if(viewBox != mViewBox)
  {
    mViewBox = viewBox;
    mChanged = true;
  }

  return true;
}

const Vector2& CanvasRendererTizen::GetViewBox()
{
  return mViewBox;
}

void CanvasRendererTizen::MakeTargetBuffer(const Vector2& size)
{
#ifdef THORVG_SUPPORT
  if(!mNativeImageQueue)
  {
    mNativeImageQueue = Dali::NativeImageSourceQueue::New(size.width, size.height, Dali::NativeImageSourceQueue::ColorFormat::BGRA8888);
  }
  else
  {
    mNativeImageQueue->SetSize(static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height));
  }
#endif
}

#ifdef THORVG_SUPPORT
void CanvasRendererTizen::PushDrawableToGroup(Dali::CanvasRenderer::Drawable& drawable, tvg::Scene* parent)
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
  else if(type == Drawable::Types::SHAPE)
  {
    //FillGradient
    Dali::CanvasRenderer::Shape&   shape        = static_cast<Dali::CanvasRenderer::Shape&>(drawable);
    Dali::CanvasRenderer::Gradient fillGradient = shape.GetFillGradient();
    if(DALI_UNLIKELY(fillGradient))
    {
      Internal::Adaptor::Gradient& fillGradientImpl          = Dali::GetImplementation(fillGradient);
      tvg::Fill*                   tvgDuplicatedFillGradient = static_cast<tvg::Fill*>(fillGradientImpl.GetObject())->duplicate();
      if(!tvgDuplicatedFillGradient)
      {
        DALI_LOG_ERROR("Invalid gradient object [%p]\n", this);
        return;
      }
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->fill(std::unique_ptr<tvg::Fill>(tvgDuplicatedFillGradient)) != tvg::Result::Success)
      {
        DALI_LOG_ERROR("Tvg gradient set fail [%p]\n", this);
        return;
      }
    }

    //StrokeGradient
    Dali::CanvasRenderer::Gradient strokeGradient = shape.GetStrokeGradient();
    if(DALI_UNLIKELY(strokeGradient))
    {
      Internal::Adaptor::Gradient& strokeGradientImpl          = Dali::GetImplementation(strokeGradient);
      tvg::Fill*                   tvgDuplicatedStrokeGradient = static_cast<tvg::Fill*>(strokeGradientImpl.GetObject())->duplicate();
      if(!tvgDuplicatedStrokeGradient)
      {
        DALI_LOG_ERROR("Invalid gradient object [%p]\n", this);
        return;
      }
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->stroke(std::unique_ptr<tvg::Fill>(tvgDuplicatedStrokeGradient)) != tvg::Result::Success)
      {
        DALI_LOG_ERROR("Tvg gradient set fail [%p]\n", this);
        return;
      }
    }
  }

  Dali::CanvasRenderer::Drawable compositeDrawable = drawableImpl.GetCompositionDrawable();
  if(DALI_UNLIKELY(compositeDrawable))
  {
    Internal::Adaptor::Drawable& compositeDrawableImpl = Dali::GetImplementation(compositeDrawable);
    tvg::Paint*                  tvgCompositeObject    = static_cast<tvg::Paint*>(compositeDrawableImpl.GetObject());
    if(tvgCompositeObject)
    {
      tvg::Paint*     tvgDuplicatedCompositeObject = tvgCompositeObject->duplicate();
      Drawable::Types type                         = compositeDrawableImpl.GetType();

      if(type == Drawable::Types::DRAWABLE_GROUP)
      {
        Dali::CanvasRenderer::DrawableGroup& compositeGroup             = static_cast<Dali::CanvasRenderer::DrawableGroup&>(compositeDrawable);
        Internal::Adaptor::DrawableGroup&    compositeDrawableGroupImpl = Dali::GetImplementation(compositeGroup);
        DrawableGroup::DrawableVector        compositeDrawables         = compositeDrawableGroupImpl.GetDrawables();
        for(auto& it : compositeDrawables)
        {
          PushDrawableToGroup(it, static_cast<tvg::Scene*>(tvgDuplicatedCompositeObject));
        }
      }

      if(tvgDuplicatedObject->composite(std::unique_ptr<tvg::Paint>(tvgDuplicatedCompositeObject), static_cast<tvg::CompositeMethod>(drawableImpl.GetCompositionType())) != tvg::Result::Success)
      {
        DALI_LOG_ERROR("Tvg composite fail [%p]\n", this);
        return;
      }
    }
  }

  if(parent->push(std::unique_ptr<tvg::Paint>(tvgDuplicatedObject)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Tvg push fail [%p]\n", this);
    return;
  }
}
#endif

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
