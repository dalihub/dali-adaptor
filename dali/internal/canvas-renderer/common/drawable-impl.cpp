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
#include <dali/internal/canvas-renderer/common/drawable-factory.h>
#include <dali/internal/canvas-renderer/common/drawable-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Drawable::Drawable() = default;

Drawable::~Drawable()
{
  if(mImpl)
  {
    delete mImpl;
  }
}

void Drawable::Create()
{
  if(!mImpl)
  {
    mImpl = Internal::Adaptor::DrawableFactory::New();
  }
}

bool Drawable::SetOpacity(float opacity)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->SetOpacity(opacity);
}

float Drawable::GetOpacity() const
{
  if(!mImpl)
  {
    return 0.0f;
  }
  return mImpl->GetOpacity();
}

bool Drawable::Rotate(Degree degree)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->Rotate(degree);
}

bool Drawable::Scale(float factor)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->Scale(factor);
}

bool Drawable::Translate(Vector2 translate)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->Translate(translate);
}

bool Drawable::Transform(const Dali::Matrix3& matrix)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->Transform(matrix);
}

Rect<float> Drawable::GetBoundingBox() const
{
  if(!mImpl)
  {
    return Rect<float>(0, 0, 0, 0);
  }
  return mImpl->GetBoundingBox();
}

bool Drawable::SetClipPath(Dali::CanvasRenderer::Drawable& clip)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->SetClipPath(clip);
}

bool Drawable::SetMask(Dali::CanvasRenderer::Drawable& mask, Dali::CanvasRenderer::Drawable::MaskType type)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->SetMask(mask, type);
}

Dali::CanvasRenderer::Drawable Drawable::GetCompositionDrawable() const
{
  if(!mImpl)
  {
    return Dali::CanvasRenderer::Drawable();
  }
  return mImpl->GetCompositionDrawable();
}

Dali::Internal::Adaptor::Drawable::CompositionType Drawable::GetCompositionType() const
{
  if(!mImpl)
  {
    return Dali::Internal::Adaptor::Drawable::CompositionType::NONE;
  }
  return mImpl->GetCompositionType();
}

void Drawable::SetAdded(bool added)
{
  if(!mImpl)
  {
    return;
  }
  mImpl->SetAdded(added);
}

bool Drawable::IsAdded() const
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->IsAdded();
}

void* Drawable::GetObject() const
{
  if(!mImpl)
  {
    return nullptr;
  }
  return mImpl->GetObject();
}

void Drawable::SetObject(const void* object)
{
  if(!mImpl)
  {
    return;
  }
  mImpl->SetObject(object);
}

void Drawable::SetChanged(bool changed)
{
  if(!mImpl)
  {
    return;
  }
  mImpl->SetChanged(changed);
}

bool Drawable::GetChanged() const
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->GetChanged();
}

void Drawable::SetType(Drawable::Types type)
{
  if(!mImpl)
  {
    return;
  }
  mImpl->SetType(type);
}

Drawable::Types Drawable::GetType() const
{
  if(!mImpl)
  {
    return Drawable::Types::NONE;
  }
  return mImpl->GetType();
}

Dali::Internal::Adaptor::Drawable* Drawable::GetImplementation()
{
  return mImpl;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
