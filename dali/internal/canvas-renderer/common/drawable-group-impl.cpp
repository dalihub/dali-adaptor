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
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

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
  return Dali::CanvasRenderer::DrawableGroup::New();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::DrawableGroup), typeid(Dali::CanvasRenderer::Drawable), Create);

} // unnamed namespace

DrawableGroupPtr DrawableGroup::New()
{
  auto* drawableGroup = new DrawableGroup();
  drawableGroup->Initialize();
  return drawableGroup;
}

DrawableGroup::DrawableGroup()
#ifdef THORVG_SUPPORT
: mTvgScene(nullptr)
#endif
{
}

DrawableGroup::~DrawableGroup()
{
}

void DrawableGroup::Initialize()
{
#ifdef THORVG_SUPPORT
  mTvgScene = tvg::Scene::gen().release();
  if(!mTvgScene)
  {
    DALI_LOG_ERROR("DrawableGroup is null [%p]\n", this);
  }

  Drawable::SetObject(static_cast<void*>(mTvgScene));
  Drawable::SetType(Drawable::Types::DRAWABLE_GROUP);
#endif
}

bool DrawableGroup::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgScene)
  {
    DALI_LOG_ERROR("DrawableGroup is null\n");
    return false;
  }

  Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(drawable);
  if(drawableImpl.IsAdded())
  {
    DALI_LOG_ERROR("Already added [%p][%p]\n", this, &drawable);
    return false;
  }

  drawableImpl.SetAdded(true);
  mDrawables.push_back(drawable);
  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

bool DrawableGroup::RemoveDrawable(Dali::CanvasRenderer::Drawable drawable)
{
#ifdef THORVG_SUPPORT
  DrawableGroup::DrawableVector::iterator it = std::find(mDrawables.begin(), mDrawables.end(), drawable);
  if(it != mDrawables.end())
  {
    Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(*it);
    drawableImpl.SetAdded(false);

    mDrawables.erase(it);

    Drawable::SetChanged(true);

    return true;
  }

#endif
  return false;
}

bool DrawableGroup::RemoveAllDrawables()
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgScene)
  {
    DALI_LOG_ERROR("DrawableGroup is null\n");
    return false;
  }

  for(auto& it : mDrawables)
  {
    Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(it);
    drawableImpl.SetAdded(false);
  }

  mDrawables.clear();

  if(static_cast<tvg::Scene*>(mTvgScene)->clear() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("RemoveAllDrawables() fail.\n");
    return false;
  }

  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

DrawableGroup::DrawableVector DrawableGroup::GetDrawables() const
{
  return mDrawables;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
