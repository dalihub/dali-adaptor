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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-linear-gradient.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/linear-gradient-impl.h>

namespace Dali
{
CanvasRenderer::LinearGradient CanvasRenderer::LinearGradient::New()
{
  Internal::Adaptor::LinearGradientPtr linearGradient = Internal::Adaptor::LinearGradient::New();
  return LinearGradient(linearGradient.Get());
}

CanvasRenderer::LinearGradient::LinearGradient()
{
}

CanvasRenderer::LinearGradient::~LinearGradient()
{
}

CanvasRenderer::LinearGradient::LinearGradient(Internal::Adaptor::LinearGradient* impl)
: CanvasRenderer::Gradient(impl)
{
}

bool CanvasRenderer::LinearGradient::SetBounds(Vector2 firstPoint, Vector2 secondPoint)
{
  return GetImplementation(*this).SetBounds(firstPoint, secondPoint);
}

bool CanvasRenderer::LinearGradient::GetBounds(Vector2& firstPoint, Vector2& secondPoint) const
{
  return GetImplementation(*this).GetBounds(firstPoint, secondPoint);
}

} // namespace Dali
