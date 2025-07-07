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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable-group.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>

namespace Dali
{
CanvasRenderer::DrawableGroup CanvasRenderer::DrawableGroup::New()
{
  Internal::Adaptor::DrawableGroupPtr drawableGroup = Internal::Adaptor::DrawableGroup::New();
  return DrawableGroup(drawableGroup.Get());
}

CanvasRenderer::DrawableGroup::DrawableGroup()
{
}

CanvasRenderer::DrawableGroup::~DrawableGroup()
{
}

CanvasRenderer::DrawableGroup::DrawableGroup(Internal::Adaptor::DrawableGroup* impl)
: CanvasRenderer::Drawable(impl)
{
}

bool CanvasRenderer::DrawableGroup::AddDrawable(Drawable& drawable)
{
  return GetImplementation(*this).AddDrawable(drawable);
}

bool CanvasRenderer::DrawableGroup::RemoveDrawable(Drawable drawable)
{
  return GetImplementation(*this).RemoveDrawable(drawable);
}

bool CanvasRenderer::DrawableGroup::RemoveAllDrawables()
{
  return GetImplementation(*this).RemoveAllDrawables();
}

} // namespace Dali
