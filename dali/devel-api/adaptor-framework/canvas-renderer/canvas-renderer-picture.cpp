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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-picture.h>

// INTERNAL INCLUDES
#include <dali/internal/canvas-renderer/common/canvas-renderer-impl.h>
#include <dali/internal/canvas-renderer/common/picture-impl.h>

namespace Dali
{
CanvasRenderer::Picture CanvasRenderer::Picture::New()
{
  Internal::Adaptor::PicturePtr picture = Internal::Adaptor::Picture::New();
  return Picture(picture.Get());
}

CanvasRenderer::Picture::Picture()
{
}

CanvasRenderer::Picture::~Picture()
{
}

CanvasRenderer::Picture::Picture(Internal::Adaptor::Picture* impl)
: CanvasRenderer::Drawable(impl)
{
}

bool CanvasRenderer::Picture::Load(const std::string& url)
{
  return GetImplementation(*this).Load(url);
}

bool CanvasRenderer::Picture::SetSize(Vector2 size)
{
  return GetImplementation(*this).SetSize(size);
}

Vector2 CanvasRenderer::Picture::GetSize() const
{
  return GetImplementation(*this).GetSize();
}
} // namespace Dali
