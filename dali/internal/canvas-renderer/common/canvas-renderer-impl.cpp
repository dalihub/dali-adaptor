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
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>

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

CanvasRenderer::CanvasRenderer(const Vector2& viewBox)
:
#ifdef THORVG_SUPPORT
  mRasterizedTexture(),
  mMutex(),
  mTvgCanvas(nullptr),
  mTvgRoot(nullptr),
#endif
  mSize(Vector2::ZERO),
  mViewBox(Vector2::ZERO),
  mChanged(false)
{
}

CanvasRenderer::~CanvasRenderer()
{
#ifdef THORVG_SUPPORT
  mDrawables.clear();

  try
  {
    // Terminate ThorVG Engine
    tvg::Initializer::term(tvg::CanvasEngine::Sw);
  }
  catch(const std::exception& ex)
  {
    DALI_LOG_ERROR("Exception during tvg::Initializer::term(tvg::CanvasEngine::Sw)! : %s\n", ex.what());
  }
#endif
}

void CanvasRenderer::Initialize(const Vector2& viewBox)
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

bool CanvasRenderer::Commit()
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

Dali::Texture CanvasRenderer::GetRasterizedTexture()
{
  return OnGetRasterizedTexture();
}

bool CanvasRenderer::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
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

bool CanvasRenderer::IsCanvasChanged() const
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

bool CanvasRenderer::Rasterize()
{
  return OnRasterize();
}

bool CanvasRenderer::RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable)
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

bool CanvasRenderer::RemoveAllDrawables()
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

bool CanvasRenderer::SetSize(Vector2 size)
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

Vector2 CanvasRenderer::GetSize() const
{
  return mSize;
}

bool CanvasRenderer::SetViewBox(const Vector2& viewBox)
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

const Vector2& CanvasRenderer::GetViewBox()
{
  return mViewBox;
}

void CanvasRenderer::MakeTargetBuffer(const Vector2& size)
{
  OnMakeTargetBuffer(size);
}

#ifdef THORVG_SUPPORT
bool CanvasRenderer::HaveDrawablesChanged(const Dali::CanvasRenderer::Drawable& drawable) const
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

void CanvasRenderer::UpdateDrawablesChanged(Dali::CanvasRenderer::Drawable& drawable, bool changed)
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

void CanvasRenderer::PushDrawableToGroup(Dali::CanvasRenderer::Drawable& drawable, tvg::Scene* parent)
{
  Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(drawable);
  tvg::Paint*                  tvgObject    = static_cast<tvg::Paint*>(drawableImpl.GetObject());
  if(DALI_UNLIKELY(!tvgObject))
  {
    DALI_LOG_ERROR("Invalid drawable object [%p]\n", this);
    return;
  }

  tvg::Paint*     tvgDuplicatedObject = tvgObject->duplicate();
  Drawable::Types type                = drawableImpl.GetType();

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
    // FillGradient
    Dali::CanvasRenderer::Shape&   shape        = static_cast<Dali::CanvasRenderer::Shape&>(drawable);
    Dali::CanvasRenderer::Gradient fillGradient = shape.GetFillGradient();
    if(fillGradient)
    {
      Internal::Adaptor::Gradient& fillGradientImpl = Dali::GetImplementation(fillGradient);
      tvg::Fill*                   tvgFillGradient  = static_cast<tvg::Fill*>(fillGradientImpl.GetObject());
      if(DALI_UNLIKELY(!tvgFillGradient))
      {
        DALI_LOG_ERROR("Invalid gradient object [%p]\n", this);
        return;
      }

      tvg::Fill* tvgDuplicatedFillGradient = tvgFillGradient->duplicate();
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->fill(std::unique_ptr<tvg::Fill>(tvgDuplicatedFillGradient)) != tvg::Result::Success)
      {
        DALI_LOG_ERROR("Tvg gradient set fail [%p]\n", this);
        return;
      }
    }

    // StrokeGradient
    Dali::CanvasRenderer::Gradient strokeGradient = shape.GetStrokeGradient();
    if(strokeGradient)
    {
      Internal::Adaptor::Gradient& strokeGradientImpl = Dali::GetImplementation(strokeGradient);
      tvg::Fill*                   tvgStrokeGradient  = static_cast<tvg::Fill*>(strokeGradientImpl.GetObject());
      if(DALI_UNLIKELY(!tvgStrokeGradient))
      {
        DALI_LOG_ERROR("Invalid gradient object [%p]\n", this);
        return;
      }

      tvg::Fill* tvgDuplicatedStrokeGradient = tvgStrokeGradient->duplicate();
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->stroke(std::unique_ptr<tvg::Fill>(tvgDuplicatedStrokeGradient)) != tvg::Result::Success)
      {
        DALI_LOG_ERROR("Tvg gradient set fail [%p]\n", this);
        return;
      }
    }
  }

  Dali::CanvasRenderer::Drawable compositeDrawable = drawableImpl.GetCompositionDrawable();
  if(compositeDrawable)
  {
    Internal::Adaptor::Drawable& compositeDrawableImpl = Dali::GetImplementation(compositeDrawable);
    tvg::Paint*                  tvgCompositeObject    = static_cast<tvg::Paint*>(compositeDrawableImpl.GetObject());
    if(DALI_UNLIKELY(!tvgCompositeObject))
    {
      DALI_LOG_ERROR("Invalid composite drawable object [%p]\n", this);
      return;
    }

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
