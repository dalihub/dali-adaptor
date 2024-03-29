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
#include <dali/internal/canvas-renderer/tizen/drawable-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Drawable), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

DrawableTizen* DrawableTizen::New()
{
  return new DrawableTizen();
}

DrawableTizen::DrawableTizen()
: mAdded(false),
  mChanged(false),
  mType(Drawable::Types::NONE),
  mCompositionType(Drawable::CompositionType::NONE),
  mCompositionDrawable()
#ifdef THORVG_SUPPORT
  ,
  mTvgPaint(nullptr)
#endif
{
}

DrawableTizen::~DrawableTizen()
{
#ifdef THORVG_SUPPORT
  if(mTvgPaint)
  {
    delete mTvgPaint;
  }
#endif
}

bool DrawableTizen::SetOpacity(float opacity)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null [%p]\n", this);
    return false;
  }
  if(mTvgPaint->opacity(round(opacity * 255.f)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Set opacity fail [%p]\n", this);
    return false;
  }
  SetChanged(true);
  return true;
#else
  return false;
#endif
}

float DrawableTizen::GetOpacity() const
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null [%p]\n", this);
    return 0;
  }
  return (float)mTvgPaint->opacity() / 255.f;
#else
  return 0;
#endif
}

bool DrawableTizen::Rotate(Degree degree)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return false;
  }

  if(mTvgPaint->rotate(degree.degree) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Rotate fail.\n");
    return false;
  }
  SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool DrawableTizen::Scale(float factor)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return false;
  }

  if(mTvgPaint->scale(factor) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Scale fail.\n");
    return false;
  }
  SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool DrawableTizen::Translate(Vector2 translate)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return false;
  }

  if(mTvgPaint->translate(translate.x, translate.y) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Translate fail.\n");
    return false;
  }
  SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool DrawableTizen::Transform(const Dali::Matrix3& matrix)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return false;
  }

  tvg::Matrix tvgMatrix = {matrix.AsFloat()[0], matrix.AsFloat()[1], matrix.AsFloat()[2], matrix.AsFloat()[3], matrix.AsFloat()[4], matrix.AsFloat()[5], matrix.AsFloat()[6], matrix.AsFloat()[7], matrix.AsFloat()[8]};

  if(mTvgPaint->transform(tvgMatrix) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Transform fail.\n");
    return false;
  }
  SetChanged(true);
  return true;
#else
  return false;
#endif
}

Rect<float> DrawableTizen::GetBoundingBox() const
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return Rect<float>(0, 0, 0, 0);
  }

  float x, y, width, height;
  x = y = width = height = 0;

  if(mTvgPaint->bounds(&x, &y, &width, &height) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Get bounds fail.\n");
    return Rect<float>(0, 0, 0, 0);
  }
  return Rect<float>(x, y, width, height);
#else
  return Rect<float>(0, 0, 0, 0);
#endif
}

bool DrawableTizen::SetClipPath(Dali::CanvasRenderer::Drawable& clip)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return false;
  }

  Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(clip);
  if(drawableImpl.IsAdded())
  {
    DALI_LOG_ERROR("Already used [%p][%p]\n", this, &clip);
    return false;
  }

  drawableImpl.SetAdded(true);
  mCompositionDrawable = clip;
  mCompositionType     = Drawable::CompositionType::CLIP_PATH;
  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

bool DrawableTizen::SetMask(Dali::CanvasRenderer::Drawable& mask, Dali::CanvasRenderer::Drawable::MaskType type)
{
#ifdef THORVG_SUPPORT
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null\n");
    return false;
  }

  Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(mask);
  if(drawableImpl.IsAdded())
  {
    DALI_LOG_ERROR("Already used [%p][%p]\n", this, &mask);
    return false;
  }

  drawableImpl.SetAdded(true);
  mCompositionDrawable = mask;
  switch(type)
  {
    case Dali::CanvasRenderer::Drawable::MaskType::ALPHA:
    {
      mCompositionType = Drawable::CompositionType::ALPHA_MASK;
      break;
    }
    case Dali::CanvasRenderer::Drawable::MaskType::ALPHA_INVERSE:
    {
      mCompositionType = Drawable::CompositionType::ALPHA_MASK_INVERSE;
      break;
    }
    default:
    {
      mCompositionType = Drawable::CompositionType::ALPHA_MASK;
      break;
    }
  }
  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Drawable DrawableTizen::GetCompositionDrawable() const
{
  return mCompositionDrawable;
}

Drawable::CompositionType DrawableTizen::GetCompositionType() const
{
  return mCompositionType;
}

void DrawableTizen::SetAdded(bool added)
{
  mAdded = !!added;
}

bool DrawableTizen::IsAdded() const
{
  return mAdded;
}

void* DrawableTizen::GetObject() const
{
#ifdef THORVG_SUPPORT
  return static_cast<void*>(mTvgPaint);
#else
  return nullptr;
#endif
}

void DrawableTizen::SetObject(const void* object)
{
#ifdef THORVG_SUPPORT
  if(object)
  {
    mTvgPaint = static_cast<tvg::Paint*>(const_cast<void*>(object));
  }
#endif
}

void DrawableTizen::SetChanged(bool changed)
{
  if(!mChanged && changed) Dali::Stage::GetCurrent().KeepRendering(0.0f);
  mChanged = !!changed;
}

bool DrawableTizen::GetChanged() const
{
  return mChanged;
}

void DrawableTizen::SetType(Drawable::Types type)
{
  mType = type;
}

Drawable::Types DrawableTizen::GetType() const
{
  return mType;
}
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
