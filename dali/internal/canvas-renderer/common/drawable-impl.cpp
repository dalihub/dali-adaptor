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
  if(pImpl)
  {
    delete pImpl;
  }
}

void Drawable::Create()
{
  if(!pImpl)
  {
    pImpl = Internal::Adaptor::DrawableFactory::New();
  }
}

bool Drawable::SetOpacity(float opacity)
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->SetOpacity(opacity);
}

float Drawable::GetOpacity() const
{
  if(!pImpl)
  {
    return 0.0f;
  }
  return pImpl->GetOpacity();
}

bool Drawable::Rotate(Degree degree)
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->Rotate(degree);
}

bool Drawable::Scale(float factor)
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->Scale(factor);
}

bool Drawable::Translate(Vector2 translate)
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->Translate(translate);
}

bool Drawable::Transform(const Dali::Matrix3& matrix)
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->Transform(matrix);
}

void Drawable::SetDrawableAdded(bool added)
{
  if(!pImpl)
  {
    return;
  }
  pImpl->SetDrawableAdded(added);
}

bool Drawable::IsDrawableAdded()
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->IsDrawableAdded();
}

void* Drawable::GetObject() const
{
  if(!pImpl)
  {
    return nullptr;
  }
  return pImpl->GetObject();
}

void Drawable::SetObject(const void* object)
{
  if(!pImpl)
  {
    return;
  }
  pImpl->SetObject(object);
}

void Drawable::SetChanged(bool changed)
{
  if(!pImpl)
  {
    return;
  }
  pImpl->SetChanged(changed);
}

bool Drawable::GetChanged() const
{
  if(!pImpl)
  {
    return false;
  }
  return pImpl->GetChanged();
}

void Drawable::SetDrawableType(Drawable::DrawableTypes type)
{
  if(!pImpl)
  {
    return;
  }
  pImpl->SetDrawableType(type);
}

Drawable::DrawableTypes Drawable::GetDrawableType() const
{
  if(!pImpl)
  {
    return Drawable::DrawableTypes::NONE;
  }
  return pImpl->GetDrawableType();
}

Dali::Internal::Adaptor::Drawable* Drawable::GetImplementation()
{
  return pImpl;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
