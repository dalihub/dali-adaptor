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
#include <dali/devel-api/adaptor-framework/canvas-renderer.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-factory.h>
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
CanvasRenderer CanvasRenderer::New(const Vector2& viewBox)
{
  return CanvasRenderer(Internal::Adaptor::CanvasRendererFactory::New(viewBox));
}

CanvasRenderer::CanvasRenderer()
{
}

CanvasRenderer::~CanvasRenderer()
{
}

CanvasRenderer::CanvasRenderer(Internal::Adaptor::CanvasRenderer* internal)
: BaseHandle(internal)
{
}

bool CanvasRenderer::Commit()
{
  return GetImplementation(*this).Commit();
}

Devel::PixelBuffer CanvasRenderer::GetPixelBuffer()
{
  return GetImplementation(*this).GetPixelBuffer();
}

bool CanvasRenderer::AddDrawable(Drawable& drawable)
{
  return GetImplementation(*this).AddDrawable(drawable);
}

bool CanvasRenderer::IsCanvasChanged() const
{
  return GetImplementation(*this).IsCanvasChanged();
}

bool CanvasRenderer::Rasterize()
{
  return GetImplementation(*this).Rasterize();
}

bool CanvasRenderer::SetSize(const Vector2& size)
{
  return GetImplementation(*this).SetSize(size);
}

const Vector2& CanvasRenderer::GetSize()
{
  return GetImplementation(*this).GetSize();
}

} // namespace Dali
