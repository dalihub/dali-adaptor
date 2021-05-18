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
#include <dali/internal/canvas-renderer/ubuntu/drawable-group-impl-ubuntu.h>

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
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::DrawableGroup), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

DrawableGroupUbuntu* DrawableGroupUbuntu::New()
{
  return new DrawableGroupUbuntu();
}

DrawableGroupUbuntu::DrawableGroupUbuntu()
#ifdef THORVG_SUPPORT
: mTvgScene(nullptr)
#endif
{
  Initialize();
}

DrawableGroupUbuntu::~DrawableGroupUbuntu()
{
}

void DrawableGroupUbuntu::Initialize()
{
#ifdef THORVG_SUPPORT
  mTvgScene = tvg::Scene::gen().release();
  if(!mTvgScene)
  {
    DALI_LOG_ERROR("DrawableGroup is null [%p]\n", this);
  }

  Drawable::Create();
  Drawable::SetObject(static_cast<void*>(mTvgScene));
  Drawable::SetDrawableType(Drawable::DrawableTypes::DRAWABLE_GROUP);
#endif
}

bool DrawableGroupUbuntu::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgScene)
  {
    DALI_LOG_ERROR("DrawableGroup is null\n");
    return false;
  }

  for(auto& it : mDrawables)
  {
    if(it == drawable)
    {
      DALI_LOG_ERROR("Already added [%p][%p]\n", this, &drawable);
      return false;
    }
  }

  Internal::Adaptor::Drawable& drawableImpl = Dali::GetImplementation(drawable);
  if(drawableImpl.IsDrawableAdded())
  {
    DALI_LOG_ERROR("Already added somewhere [%p][%p]\n", this, &drawable);
    return false;
  }

  drawableImpl.SetDrawableAdded(true);
  mDrawables.push_back(drawable);
  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

bool DrawableGroupUbuntu::Clear()
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
    drawableImpl.SetDrawableAdded(false);
  }

  mDrawables.clear();

  if(static_cast<tvg::Scene*>(mTvgScene)->clear() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Clear() fail.\n");
    return false;
  }

  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

std::vector<Dali::CanvasRenderer::Drawable> DrawableGroupUbuntu::GetDrawables()
{
  return mDrawables;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
