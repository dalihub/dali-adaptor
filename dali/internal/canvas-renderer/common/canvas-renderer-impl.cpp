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
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
CanvasRenderer::CanvasRenderer()
{
}

CanvasRenderer::~CanvasRenderer() = default;

bool CanvasRenderer::Commit()
{
  return false;
}

Devel::PixelBuffer CanvasRenderer::GetPixelBuffer()
{
  return Devel::PixelBuffer();
}

bool CanvasRenderer::AddDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
  return false;
}

bool CanvasRenderer::IsCanvasChanged() const
{
  return false;
}

bool CanvasRenderer::Rasterize()
{
  return false;
}

bool CanvasRenderer::RemoveDrawable(Dali::CanvasRenderer::Drawable& drawable)
{
  return false;
}

bool CanvasRenderer::RemoveAllDrawables()
{
  return false;
}

bool CanvasRenderer::SetSize(const Vector2& size)
{
  return false;
}

const Vector2& CanvasRenderer::GetSize()
{
  return Vector2::ZERO;
}

bool CanvasRenderer::SetViewBox(const Vector2& viewBox)
{
  return false;
}

const Vector2& CanvasRenderer::GetViewBox()
{
  return Vector2::ZERO;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
