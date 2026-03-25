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
#include <dali/internal/canvas-renderer/common/linear-gradient-impl.h>

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
  return Dali::CanvasRenderer::LinearGradient::New();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::LinearGradient), typeid(Dali::CanvasRenderer::Gradient), Create);

} // unnamed namespace

LinearGradientPtr LinearGradient::New()
{
  auto* linearGradient = new LinearGradient();
  linearGradient->Initialize();
  return linearGradient;
}

LinearGradient::LinearGradient()
#ifdef THORVG_SUPPORT
: mTvgLinearGradient(nullptr)
#endif
{
}

LinearGradient::~LinearGradient()
{
}

void LinearGradient::Initialize()
{
#ifdef THORVG_SUPPORT
  mTvgLinearGradient = tvg::LinearGradient::gen().release();
  if(!mTvgLinearGradient)
  {
    DALI_LOG_ERROR("LinearGradient is null [%p]\n", this);
  }

  Gradient::SetObject(static_cast<void*>(mTvgLinearGradient));
#endif
}

bool LinearGradient::SetBounds(Vector2 firstPoint, Vector2 secondPoint)
{
#ifdef THORVG_SUPPORT
  if(!Gradient::GetObject() || !mTvgLinearGradient)
  {
    DALI_LOG_ERROR("LinearGradient is null\n");
    return false;
  }

  if(mTvgLinearGradient->linear(firstPoint.x, firstPoint.y, secondPoint.x, secondPoint.y) != tvg::Result::Success)
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

bool LinearGradient::GetBounds(Vector2& firstPoint, Vector2& secondPoint) const
{
#ifdef THORVG_SUPPORT
  if(!Gradient::GetObject() || !mTvgLinearGradient)
  {
    DALI_LOG_ERROR("LinearGradient is null\n");
    return false;
  }

  if(mTvgLinearGradient->linear(&firstPoint.x, &firstPoint.y, &secondPoint.x, &secondPoint.y) != tvg::Result::Success)
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
