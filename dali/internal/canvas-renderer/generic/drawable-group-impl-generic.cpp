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
#include <dali/internal/canvas-renderer/generic/drawable-group-impl-generic.h>

// EXTERNAL INCLUDES
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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::DrawableGroup), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

DrawableGroupGeneric* DrawableGroupGeneric::New()
{
  return new DrawableGroupGeneric();
}

DrawableGroupGeneric::DrawableGroupGeneric()
{
}

DrawableGroupGeneric::~DrawableGroupGeneric()
{
}

bool DrawableGroupGeneric::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
  return false;
}

bool DrawableGroupGeneric::RemoveDrawable(Dali::CanvasRenderer::Drawable drawable)
{
  return false;
}

bool DrawableGroupGeneric::RemoveAllDrawables()
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
