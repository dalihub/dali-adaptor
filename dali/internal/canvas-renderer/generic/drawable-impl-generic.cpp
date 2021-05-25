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
#include <dali/internal/canvas-renderer/generic/drawable-impl-generic.h>

// EXTERNAL INCLUDES
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

DrawableGeneric* DrawableGeneric::New()
{
  return new DrawableGeneric();
}

DrawableGeneric::DrawableGeneric()
{
}

DrawableGeneric::~DrawableGeneric()
{
}

bool DrawableGeneric::SetOpacity(float opacity)
{
  return false;
}

float DrawableGeneric::GetOpacity() const
{
  return 0;
}

bool DrawableGeneric::Rotate(Degree degree)
{
  return false;
}

bool DrawableGeneric::Scale(float factor)
{
  return false;
}

bool DrawableGeneric::Translate(Vector2 translate)
{
  return false;
}

bool DrawableGeneric::Transform(const Dali::Matrix3& matrix)
{
  return false;
}

Rect<float> DrawableGeneric::GetBoundingBox() const
{
  return Rect<float>(0, 0, 0, 0);
}

void DrawableGeneric::SetDrawableAdded(bool added)
{
}

void DrawableGeneric::SetObject(const void* object)
{
}

void* DrawableGeneric::GetObject() const
{
  return nullptr;
}

void DrawableGeneric::SetChanged(bool changed)
{
}

bool DrawableGeneric::GetChanged() const
{
  return false;
}
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
