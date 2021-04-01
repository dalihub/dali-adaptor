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
  mTvgPaint(nullptr)
{
}

DrawableTizen::~DrawableTizen()
{
  if(mTvgPaint && !mAdded)
  {
    delete mTvgPaint;
  }
}

bool DrawableTizen::SetOpacity(float opacity)
{
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
}

float DrawableTizen::GetOpacity() const
{
  if(!mTvgPaint)
  {
    DALI_LOG_ERROR("Drawable is null [%p]\n", this);
    return 0;
  }
  return (float)mTvgPaint->opacity() / 255.f;
}

bool DrawableTizen::Rotate(Degree degree)
{
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
}

bool DrawableTizen::Scale(float factor)
{
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
}

bool DrawableTizen::Translate(Vector2 translate)
{
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
}

bool DrawableTizen::Transform(const Dali::Matrix3& matrix)
{
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
}

void DrawableTizen::SetDrawableAdded(bool added)
{
  mAdded = !!added;
}

void* DrawableTizen::GetObject() const
{
  return static_cast<void*>(mTvgPaint);
}

void DrawableTizen::SetObject(const void* object)
{
  if(object)
  {
    mTvgPaint = static_cast<tvg::Paint*>((void*)object);
  }
  else
  {
    if(mAdded)
    {
      mTvgPaint = nullptr;
    }
    if(mTvgPaint)
    {
      delete mTvgPaint;
    }
  }
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
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
