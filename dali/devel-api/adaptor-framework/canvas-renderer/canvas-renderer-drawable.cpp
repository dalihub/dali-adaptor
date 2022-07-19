/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-drawable.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
// CanvasRenderer::Drawable
//
CanvasRenderer::Drawable::Drawable()
{
}

CanvasRenderer::Drawable::~Drawable()
{
}

CanvasRenderer::Drawable::Drawable(Internal::Adaptor::Drawable* pImpl)
: BaseHandle(pImpl)
{
}

bool CanvasRenderer::Drawable::SetOpacity(float opacity)
{
  return GetImplementation(*this).SetOpacity(opacity);
}

float CanvasRenderer::Drawable::GetOpacity() const
{
  return GetImplementation(*this).GetOpacity();
}

bool CanvasRenderer::Drawable::Rotate(Degree degree)
{
  return GetImplementation(*this).Rotate(degree);
}

bool CanvasRenderer::Drawable::Scale(float factor)
{
  return GetImplementation(*this).Scale(factor);
}

bool CanvasRenderer::Drawable::Translate(Vector2 translate)
{
  return GetImplementation(*this).Translate(translate);
}

bool CanvasRenderer::Drawable::Transform(const Dali::Matrix3& matrix)
{
  return GetImplementation(*this).Transform(matrix);
}

Rect<float> CanvasRenderer::Drawable::GetBoundingBox() const
{
  return GetImplementation(*this).GetBoundingBox();
}

bool CanvasRenderer::Drawable::SetClipPath(Drawable& clip)
{
  return GetImplementation(*this).SetClipPath(clip);
}

bool CanvasRenderer::Drawable::SetMask(Drawable& mask, MaskType type)
{
  return GetImplementation(*this).SetMask(mask, type);
}

CanvasRenderer::Drawable CanvasRenderer::Drawable::DownCast(BaseHandle handle)
{
  return CanvasRenderer::Drawable(dynamic_cast<Internal::Adaptor::Drawable*>(handle.GetObjectPtr()));
}
} // namespace Dali
