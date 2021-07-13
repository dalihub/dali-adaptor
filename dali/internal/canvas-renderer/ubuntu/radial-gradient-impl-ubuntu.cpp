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
#include <dali/internal/canvas-renderer/ubuntu/radial-gradient-impl-ubuntu.h>

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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::RadialGradient), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

RadialGradientUbuntu* RadialGradientUbuntu::New()
{
  return new RadialGradientUbuntu();
}

RadialGradientUbuntu::RadialGradientUbuntu()
#ifdef THORVG_SUPPORT
: mTvgRadialGradient(nullptr)
#endif
{
  Initialize();
}

RadialGradientUbuntu::~RadialGradientUbuntu()
{
}

void RadialGradientUbuntu::Initialize()
{
#ifdef THORVG_SUPPORT
  mTvgRadialGradient = tvg::RadialGradient::gen().release();
  if(!mTvgRadialGradient)
  {
    DALI_LOG_ERROR("RadialGradient is null [%p]\n", this);
  }

  Gradient::Create();
  Gradient::SetObject(static_cast<void*>(mTvgRadialGradient));
#endif
}

bool RadialGradientUbuntu::SetBounds(Vector2 centerPoint, float radius)
{
#ifdef THORVG_SUPPORT
  if(!Gradient::GetObject() || !mTvgRadialGradient)
  {
    DALI_LOG_ERROR("RadialGradient is null\n");
    return false;
  }

  if(mTvgRadialGradient->radial(centerPoint.x, centerPoint.y, radius) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("SetBounds() fail.\n");
    return false;
  }

  Gradient::SetChanged(true);

  return true;
#else
  return false;
#endif
}

bool RadialGradientUbuntu::GetBounds(Vector2& centerPoint, float& radius) const
{
#ifdef THORVG_SUPPORT
  if(!Gradient::GetObject() || !mTvgRadialGradient)
  {
    DALI_LOG_ERROR("RadialGradient is null\n");
    return false;
  }

  if(mTvgRadialGradient->radial(&centerPoint.x, &centerPoint.y, &radius) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("GetBounds() fail.\n");
    return false;
  }

  return true;
#else
  return false;
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
