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
#include <dali/internal/canvas-renderer/common/drawable-group-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
DrawableGroup::DrawableGroup() = default;

DrawableGroup::~DrawableGroup() = default;

bool DrawableGroup::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
  return false;
}

bool DrawableGroup::Clear()
{
  return false;
}

DrawableGroup::DrawableVector DrawableGroup::GetDrawables() const
{
  return DrawableGroup::DrawableVector();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
