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
#include <dali/internal/canvas-renderer/generic/gradient-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Gradient), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

GradientGeneric* GradientGeneric::New()
{
  return new GradientGeneric();
}

GradientGeneric::GradientGeneric()
{
}

GradientGeneric::~GradientGeneric()
{
}

bool GradientGeneric::SetColorStops(Dali::CanvasRenderer::Gradient::ColorStops& colorStops)
{
  return false;
}

Dali::CanvasRenderer::Gradient::ColorStops GradientGeneric::GetColorStops() const
{
  return Dali::CanvasRenderer::Gradient::ColorStops();
}

bool GradientGeneric::SetSpread(Dali::CanvasRenderer::Gradient::Spread spread)
{
  return false;
}

Dali::CanvasRenderer::Gradient::Spread GradientGeneric::GetSpread() const
{
  return Dali::CanvasRenderer::Gradient::Spread::PAD;
}

void GradientGeneric::SetObject(const void* object)
{
}

void* GradientGeneric::GetObject() const
{
  return nullptr;
}

void GradientGeneric::SetChanged(bool changed)
{
}

bool GradientGeneric::GetChanged() const
{
  return false;
}
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
