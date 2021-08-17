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
#include <dali/internal/canvas-renderer/generic/linear-gradient-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::LinearGradient), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

LinearGradientGeneric* LinearGradientGeneric::New()
{
  return new LinearGradientGeneric();
}

LinearGradientGeneric::LinearGradientGeneric()
{
}

LinearGradientGeneric::~LinearGradientGeneric()
{
}

bool LinearGradientGeneric::SetBounds(Vector2 firstPoint, Vector2 secondPoint)
{
  return false;
}

bool LinearGradientGeneric::GetBounds(Vector2& firstPoint, Vector2& secondPoint) const
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
