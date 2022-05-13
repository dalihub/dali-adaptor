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
#include <dali/devel-api/adaptor-framework/vector-image-renderer.h>

// INTERNAL INCLUDES
#include <dali/internal/vector-image/common/vector-image-renderer-impl.h>

namespace Dali
{
VectorImageRenderer VectorImageRenderer::New()
{
  Internal::Adaptor::VectorImageRendererPtr imageRenderer = Internal::Adaptor::VectorImageRenderer::New();
  return VectorImageRenderer(imageRenderer.Get());
}

VectorImageRenderer::VectorImageRenderer()
{
}

VectorImageRenderer::~VectorImageRenderer()
{
}

VectorImageRenderer::VectorImageRenderer(Internal::Adaptor::VectorImageRenderer* internal)
: BaseHandle(internal)
{
}

bool VectorImageRenderer::Load(const Vector<uint8_t>& data, float dpi)
{
  return GetImplementation(*this).Load(data, dpi);
}

bool VectorImageRenderer::IsLoaded() const
{
  return GetImplementation(*this).IsLoaded();
}

Dali::Devel::PixelBuffer VectorImageRenderer::Rasterize(uint32_t width, uint32_t height)
{
  return GetImplementation(*this).Rasterize(width, height);
}

void VectorImageRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  GetImplementation(*this).GetDefaultSize(width, height);
}

} // namespace Dali
