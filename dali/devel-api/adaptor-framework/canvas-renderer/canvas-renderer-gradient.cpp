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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-gradient.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

namespace Dali
{
// CanvasRenderer::Gradient
//
CanvasRenderer::Gradient::Gradient()
{
}

CanvasRenderer::Gradient::~Gradient()
{
}

CanvasRenderer::Gradient::Gradient(Internal::Adaptor::Gradient* pImpl)
: BaseHandle(pImpl)
{
}

bool CanvasRenderer::Gradient::SetColorStops(Dali::CanvasRenderer::Gradient::ColorStops& colorStops)
{
  return GetImplementation(*this).SetColorStops(colorStops);
}

Dali::CanvasRenderer::Gradient::ColorStops CanvasRenderer::Gradient::GetColorStops() const
{
  return GetImplementation(*this).GetColorStops();
}

bool CanvasRenderer::Gradient::SetSpread(Dali::CanvasRenderer::Gradient::Spread spread)
{
  return GetImplementation(*this).SetSpread(spread);
}

Dali::CanvasRenderer::Gradient::Spread CanvasRenderer::Gradient::GetSpread() const
{
  return GetImplementation(*this).GetSpread();
}

CanvasRenderer::Gradient CanvasRenderer::Gradient::DownCast(BaseHandle handle)
{
  return CanvasRenderer::Gradient(dynamic_cast<Internal::Adaptor::Gradient*>(handle.GetObjectPtr()));
}
} // namespace Dali
