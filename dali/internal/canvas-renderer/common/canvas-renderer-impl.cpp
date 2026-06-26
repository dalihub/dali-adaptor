/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <algorithm>
#include <cmath>

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
#ifdef THORVG_VERSION_1
    if(mTvgCanvas)
    {
      delete mTvgCanvas;
      mTvgCanvas = nullptr;
    }
    tvg::Initializer::term();
#else
    tvg::Initializer::term(tvg::CanvasEngine::Sw);
#endif
  }
  catch(const std::exception& ex)
  {
    DALI_LOG_ERROR("Exception during tvg::Initializer::term()! : %s\n", ex.what());
  }
#endif
}

void CanvasRenderer::Initialize(const Vector2& viewBox)
{
#ifdef THORVG_SUPPORT
#ifdef THORVG_VERSION_1
  if(tvg::Initializer::init(0 /*threads*/) != tvg::Result::Success)
#else
  if(tvg::Initializer::init(tvg::CanvasEngine::Sw, 0 /*threads*/) != tvg::Result::Success)
#endif
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

  // When auto-padding is enabled and an effect is active, render into a buffer enlarged by the
  // effect margin so the offset/blurred result is not clipped. The content visual scales the
  // larger texture back into the view, so the content appears slightly smaller but is never clipped.
  double effectMargin = 0.0;
#ifdef THORVG_VERSION_1
  if(mEffectAutoPadding)
  {
    effectMargin = GetEffectMargin();
  }
#endif
  MakeTargetBuffer(Vector2(mSize.width + static_cast<float>(effectMargin) * 2.0f,
                           mSize.height + static_cast<float>(effectMargin) * 2.0f));
  mChanged = false;

#ifdef THORVG_VERSION_1
  if(mTvgCanvas->remove() != tvg::Result::Success)
#else
  if(mTvgCanvas->clear() != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("ThorVG canvas clear fail [%p]\n", this);
    return false;
  }

#ifdef THORVG_VERSION_1
  mTvgRoot = tvg::Scene::gen();
#else
  auto scene = tvg::Scene::gen();
  mTvgRoot   = scene.get();
#endif
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

#ifdef THORVG_VERSION_1
  // Shift the content inward by the auto-padding margin so the effect overflow lands inside the
  // enlarged buffer. translate() composes after scale(), so the margin is in final pixel space.
  if(effectMargin > 0.0)
  {
    mTvgRoot->translate(static_cast<float>(effectMargin), static_cast<float>(effectMargin));
  }

  // Apply the scene effect to the freshly generated root scene.
  // mTvgRoot is regenerated on every Commit(), so the effect is re-added each time and is
  // not affected by the duplicate() calls inside PushDrawableToGroup().
  if(mEffect.type == EffectType::DropShadow && mEffect.sigma > 0.0)
  {
    if(mTvgRoot->add(tvg::SceneEffect::DropShadow,
                     mEffect.colorR,
                     mEffect.colorG,
                     mEffect.colorB,
                     mEffect.opacity,
                     mEffect.angle,
                     mEffect.distance,
                     mEffect.sigma,
                     100) != tvg::Result::Success)
    {
      DALI_LOG_ERROR("ThorVG DropShadow add failed [%p]\n", this);
    }
  }
  else if(mEffect.type == EffectType::GaussianBlur && mEffect.sigma > 0.0)
  {
    if(mTvgRoot->add(tvg::SceneEffect::GaussianBlur,
                     mEffect.sigma,
                     0 /*direction: both*/,
                     0 /*border: duplicate*/,
                     100) != tvg::Result::Success)
    {
      DALI_LOG_ERROR("ThorVG GaussianBlur add failed [%p]\n", this);
    }
  }
#endif

#ifdef THORVG_VERSION_1
  if(mTvgCanvas->add(mTvgRoot) != tvg::Result::Success)
#else
  if(mTvgCanvas->push(move(scene)) != tvg::Result::Success)
#endif
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

bool CanvasRenderer::SetDropShadow(const Dali::Vector4& color, float offsetX, float offsetY, float blurRadius)
{
#ifdef THORVG_SUPPORT
  // Convert the blur radius to the Gaussian sigma used by ThorVG (sigma = blurRadius * 0.5).
  const double sigma = static_cast<double>(blurRadius) * 0.5;

  // Convert the cartesian (offsetX, offsetY) offset to the polar (angle, distance) form ThorVG
  // expects. ThorVG's DropShadow derives its pixel offset as (see tvgSwPostEffect.cpp):
  //   offset.x =  distance * sin(angle)
  //   offset.y = -distance * cos(angle)   (screen +y points down)
  // i.e. the angle is measured from the vertical axis (0 = up, 90 = right, 180 = down), so the
  // inverse of a cartesian offset (offsetX right+, offsetY down+) is angle = atan2(offsetX, -offsetY).
  const double distance = std::hypot(static_cast<double>(offsetX), static_cast<double>(offsetY));
  double       angle    = (distance > 0.0)
                            ? std::atan2(static_cast<double>(offsetX), static_cast<double>(-offsetY)) * (180.0 / M_PI)
                            : 0.0;
  if(angle < 0.0)
  {
    angle += 360.0;
  }

  mEffect          = {};
  mEffect.type     = EffectType::DropShadow;
  mEffect.colorR   = std::clamp(static_cast<int>(color.r * 255.0f + 0.5f), 0, 255);
  mEffect.colorG   = std::clamp(static_cast<int>(color.g * 255.0f + 0.5f), 0, 255);
  mEffect.colorB   = std::clamp(static_cast<int>(color.b * 255.0f + 0.5f), 0, 255);
  mEffect.opacity  = std::clamp(static_cast<int>(color.a * 255.0f + 0.5f), 0, 255);
  mEffect.angle    = angle;
  mEffect.distance = distance;
  mEffect.sigma    = sigma;

  mChanged = true;
  return true;
#else
  return false;
#endif
}

bool CanvasRenderer::ClearDropShadow()
{
#ifdef THORVG_SUPPORT
  if(mEffect.type != EffectType::DropShadow)
  {
    return true;
  }
  mEffect  = {};
  mChanged = true;
  return true;
#else
  return false;
#endif
}

bool CanvasRenderer::HasDropShadow() const
{
#ifdef THORVG_SUPPORT
  return mEffect.type == EffectType::DropShadow;
#else
  return false;
#endif
}

bool CanvasRenderer::SetGaussianBlur(float blurRadius)
{
#ifdef THORVG_SUPPORT
  mEffect       = {};
  mEffect.type  = EffectType::GaussianBlur;
  mEffect.sigma = static_cast<double>(blurRadius) * 0.5; // ThorVG Gaussian sigma = blurRadius * 0.5

  mChanged = true;
  return true;
#else
  return false;
#endif
}

bool CanvasRenderer::ClearGaussianBlur()
{
#ifdef THORVG_SUPPORT
  if(mEffect.type != EffectType::GaussianBlur)
  {
    return true;
  }
  mEffect  = {};
  mChanged = true;
  return true;
#else
  return false;
#endif
}

bool CanvasRenderer::HasGaussianBlur() const
{
#ifdef THORVG_SUPPORT
  return mEffect.type == EffectType::GaussianBlur;
#else
  return false;
#endif
}

void CanvasRenderer::SetEffectAutoPaddingEnable(bool enable)
{
#ifdef THORVG_SUPPORT
  if(mEffectAutoPadding != enable)
  {
    mEffectAutoPadding = enable;
    mChanged           = true;
  }
#endif
}

bool CanvasRenderer::IsEffectAutoPaddingEnabled() const
{
#ifdef THORVG_SUPPORT
  return mEffectAutoPadding;
#else
  return false;
#endif
}

double CanvasRenderer::GetEffectMargin() const
{
#ifdef THORVG_SUPPORT
  if(mEffect.type == EffectType::None || mEffect.sigma <= 0.0)
  {
    return 0.0;
  }
  // ~3 sigma covers the visible Gaussian tail; a drop shadow additionally extends by its offset.
  double margin = mEffect.sigma * 3.0;
  if(mEffect.type == EffectType::DropShadow)
  {
    margin += mEffect.distance;
  }
  return std::ceil(margin);
#else
  return 0.0;
#endif
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
#ifdef THORVG_VERSION_1
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->fill(tvgDuplicatedFillGradient) != tvg::Result::Success)
#else
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->fill(std::unique_ptr<tvg::Fill>(tvgDuplicatedFillGradient)) != tvg::Result::Success)
#endif
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
#ifdef THORVG_VERSION_1
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->strokeFill(tvgDuplicatedStrokeGradient) != tvg::Result::Success)
#else
      if(static_cast<tvg::Shape*>(tvgDuplicatedObject)->stroke(std::unique_ptr<tvg::Fill>(tvgDuplicatedStrokeGradient)) != tvg::Result::Success)
#endif
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

#ifdef THORVG_VERSION_1
    switch(drawableImpl.GetCompositionType())
    {
      case Drawable::CompositionType::NONE:
      default:
      {
        break;
      }
      case Drawable::CompositionType::CLIP_PATH:
      {
        if(tvgDuplicatedObject->clip(static_cast<tvg::Shape*>(tvgDuplicatedCompositeObject)) != tvg::Result::Success)
        {
          DALI_LOG_ERROR("Tvg composite by clip fail [%p]\n", this);
          return;
        }
        break;
      }
      case Drawable::CompositionType::ALPHA_MASK:
      {
        if(tvgDuplicatedObject->mask(tvgDuplicatedCompositeObject, tvg::MaskMethod::Alpha) != tvg::Result::Success)
        {
          DALI_LOG_ERROR("Tvg composite by alpha mask fail [%p]\n", this);
          return;
        }
        break;
      }
      case Drawable::CompositionType::ALPHA_MASK_INVERSE:
      {
        if(tvgDuplicatedObject->mask(tvgDuplicatedCompositeObject, tvg::MaskMethod::InvAlpha) != tvg::Result::Success)
        {
          DALI_LOG_ERROR("Tvg composite by alpha mask inverse fail [%p]\n", this);
          return;
        }
        break;
      }
    }
#else
    if(tvgDuplicatedObject->composite(std::unique_ptr<tvg::Paint>(tvgDuplicatedCompositeObject), static_cast<tvg::CompositeMethod>(drawableImpl.GetCompositionType())) != tvg::Result::Success)
    {
      DALI_LOG_ERROR("Tvg composite fail [%p]\n", this);
      return;
    }
#endif
  }

#ifdef THORVG_VERSION_1
  if(parent->add(tvgDuplicatedObject) != tvg::Result::Success)
#else
  if(parent->push(std::unique_ptr<tvg::Paint>(tvgDuplicatedObject)) != tvg::Result::Success)
#endif
  {
    DALI_LOG_ERROR("Tvg push fail [%p]\n", this);
    return;
  }
}
#endif

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
