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
#include <dali/internal/canvas-renderer/common/gradient-factory.h>
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Gradient::Gradient() = default;

Gradient::~Gradient()
{
  if(mImpl)
  {
    delete mImpl;
  }
}

void Gradient::Create()
{
  if(!mImpl)
  {
    mImpl = Internal::Adaptor::GradientFactory::New();
  }
}

bool Gradient::SetColorStops(Dali::CanvasRenderer::Gradient::ColorStops& colorStops)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->SetColorStops(colorStops);
}

Dali::CanvasRenderer::Gradient::ColorStops Gradient::GetColorStops() const
{
  if(!mImpl)
  {
    return Dali::CanvasRenderer::Gradient::ColorStops();
  }
  return mImpl->GetColorStops();
}

bool Gradient::SetSpread(Dali::CanvasRenderer::Gradient::Spread spread)
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->SetSpread(spread);
}

Dali::CanvasRenderer::Gradient::Spread Gradient::GetSpread() const
{
  if(!mImpl)
  {
    return Dali::CanvasRenderer::Gradient::Spread::PAD;
  }
  return mImpl->GetSpread();
}

void* Gradient::GetObject() const
{
  if(!mImpl)
  {
    return nullptr;
  }
  return mImpl->GetObject();
}

void Gradient::SetObject(const void* object)
{
  if(!mImpl)
  {
    return;
  }
  mImpl->SetObject(object);
}

void Gradient::SetChanged(bool changed)
{
  if(!mImpl)
  {
    return;
  }
  mImpl->SetChanged(changed);
}

bool Gradient::GetChanged() const
{
  if(!mImpl)
  {
    return false;
  }
  return mImpl->GetChanged();
}

Dali::Internal::Adaptor::Gradient* Gradient::GetImplementation()
{
  return mImpl;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
