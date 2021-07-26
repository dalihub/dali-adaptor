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
#include <dali/internal/canvas-renderer/generic/picture-impl-generic.h>

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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Picture), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

PictureGeneric* PictureGeneric::New()
{
  return new PictureGeneric();
}

PictureGeneric::PictureGeneric()
{
}

PictureGeneric::~PictureGeneric()
{
}

bool PictureGeneric::Load(const std::string& url)
{
  return false;
}

bool PictureGeneric::SetSize(Vector2 size)
{
  return false;
}

Vector2 PictureGeneric::GetSize() const
{
  return Vector2::ZERO;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
