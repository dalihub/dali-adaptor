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
#include <dali/internal/canvas-renderer/generic/canvas-renderer-impl-generic.h>

// EXTERNAL INCLUDES
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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

CanvasRendererGeneric* CanvasRendererGeneric::New(const Vector2& viewBox)
{
  return new CanvasRendererGeneric(viewBox);
}

CanvasRendererGeneric::CanvasRendererGeneric(const Vector2& viewBox)
{
}

CanvasRendererGeneric::~CanvasRendererGeneric()
{
}

bool CanvasRendererGeneric::Commit()
{
  return false;
}

Devel::PixelBuffer CanvasRendererGeneric::GetPixelBuffer()
{
  return Devel::PixelBuffer();
}

bool CanvasRendererGeneric::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
  return false;
}

bool CanvasRendererGeneric::IsCanvasChanged() const
{
  return false;
}

bool CanvasRendererGeneric::Rasterize()
{
  return false;
}

bool CanvasRendererGeneric::RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
  return false;
}

bool CanvasRendererGeneric::RemoveAllDrawables()
{
  return false;
}

bool CanvasRendererGeneric::SetSize(Vector2 size)
{
  return false;
}

Vector2 CanvasRendererGeneric::GetSize() const
{
  return Vector2::ZERO;
}

bool CanvasRendererGeneric::SetViewBox(const Vector2& viewBox)
{
  return false;
}

const Vector2& CanvasRendererGeneric::GetViewBox()
{
  return Vector2::ZERO;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
