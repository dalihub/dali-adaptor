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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-radial-gradient.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/radial-gradient-factory.h>
#include <dali/internal/canvas-renderer/common/radial-gradient-impl.h>

namespace Dali
{
CanvasRenderer::RadialGradient CanvasRenderer::RadialGradient::New()
{
  return RadialGradient(Internal::Adaptor::RadialGradientFactory::New());
}

CanvasRenderer::RadialGradient::RadialGradient()
{
}

CanvasRenderer::RadialGradient::~RadialGradient()
{
}

CanvasRenderer::RadialGradient::RadialGradient(Internal::Adaptor::RadialGradient* impl)
: CanvasRenderer::Gradient(impl)
{
}

bool CanvasRenderer::RadialGradient::SetBounds(Vector2 centerPoint, float radius)
{
  return GetImplementation(*this).SetBounds(centerPoint, radius);
}

bool CanvasRenderer::RadialGradient::GetBounds(Vector2& centerPoint, float& radius) const
{
  return GetImplementation(*this).GetBounds(centerPoint, radius);
}

} // namespace Dali
