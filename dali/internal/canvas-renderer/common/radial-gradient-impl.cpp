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
#include <dali/internal/canvas-renderer/common/radial-gradient-impl.h>

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
  return Dali::CanvasRenderer::RadialGradient::New();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::RadialGradient), typeid(Dali::CanvasRenderer::Gradient), Create);

} // unnamed namespace

RadialGradientPtr RadialGradient::New()
{
  auto* radialGradient = new RadialGradient();
  radialGradient->Initialize();
  return radialGradient;
}

RadialGradient::RadialGradient()
#ifdef THORVG_SUPPORT
: mTvgRadialGradient(nullptr)
#endif
{
}

RadialGradient::~RadialGradient()
{
}

void RadialGradient::Initialize()
{
#ifdef THORVG_SUPPORT
#ifdef THORVG_VERSION_1
  mTvgRadialGradient = tvg::RadialGradient::gen();
#else
  mTvgRadialGradient = tvg::RadialGradient::gen().release();
#endif
  if(!mTvgRadialGradient)
  {
    DALI_LOG_ERROR("RadialGradient is null [%p]\n", this);
  }

  Gradient::SetObject(static_cast<void*>(mTvgRadialGradient));
#endif
}

bool RadialGradient::SetBounds(Vector2 centerPoint, float radius)
{
#ifdef THORVG_SUPPORT
  if(!Gradient::GetObject() || !mTvgRadialGradient)
  {
    DALI_LOG_ERROR("RadialGradient is null\n");
    return false;
  }

#ifdef THORVG_VERSION_1
  if(mTvgRadialGradient->radial(centerPoint.x, centerPoint.y, radius, centerPoint.x, centerPoint.y, 0) != tvg::Result::Success)
#else
  if(mTvgRadialGradient->radial(centerPoint.x, centerPoint.y, radius) != tvg::Result::Success)
#endif
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

bool RadialGradient::GetBounds(Vector2& centerPoint, float& radius) const
{
#ifdef THORVG_SUPPORT
  if(!Gradient::GetObject() || !mTvgRadialGradient)
  {
    DALI_LOG_ERROR("RadialGradient is null\n");
    return false;
  }

#ifdef THORVG_VERSION_1
  if(mTvgRadialGradient->radial(&centerPoint.x, &centerPoint.y, &radius, &centerPoint.x, &centerPoint.y, nullptr) != tvg::Result::Success)
#else
  if(mTvgRadialGradient->radial(&centerPoint.x, &centerPoint.y, &radius) != tvg::Result::Success)
#endif
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
