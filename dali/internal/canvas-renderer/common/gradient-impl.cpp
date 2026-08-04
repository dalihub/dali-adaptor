/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

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

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Gradient), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

Gradient::Gradient()
: mChanged(false)
#ifdef THORVG_SUPPORT
  ,
  mTvgFill(nullptr)
#endif
{
}

Gradient::~Gradient()
{
#ifdef THORVG_SUPPORT
  if(mTvgFill)
  {
    delete mTvgFill;
  }
#endif
}

bool Gradient::AddColorStop(float offset, const Vector4& color)
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return false;
  }

  // ThorVG replaces the whole stop list on each call, so the existing stops are
  // read back and re-applied together with the appended one.
  const tvg::Fill::ColorStop* existingStops = nullptr;
  const uint32_t              existingCount = mTvgFill->colorStops(&existingStops);

  std::vector<tvg::Fill::ColorStop> stops;
  stops.reserve(existingCount + 1u);
  for(uint32_t i = 0u; i < existingCount; ++i)
  {
    stops.push_back(existingStops[i]);
  }

  tvg::Fill::ColorStop stop;
  stop.offset = offset;
  stop.r      = static_cast<uint8_t>(color.r * 255.0f);
  stop.g      = static_cast<uint8_t>(color.g * 255.0f);
  stop.b      = static_cast<uint8_t>(color.b * 255.0f);
  stop.a      = static_cast<uint8_t>(color.a * 255.0f);
  stops.push_back(stop);

  if(mTvgFill->colorStops(stops.data(), static_cast<uint32_t>(stops.size())) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("AddColorStop() fail.\n");
    return false;
  }

  SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool Gradient::ClearColorStops()
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return false;
  }

  if(mTvgFill->colorStops(nullptr, 0u) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("ClearColorStops() fail.\n");
    return false;
  }

  SetChanged(true);
  return true;
#else
  return false;
#endif
}

uint32_t Gradient::GetColorStopCount() const
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return 0u;
  }

  return mTvgFill->colorStops(nullptr);
#else
  return 0u;
#endif
}

float Gradient::GetColorStopOffset(uint32_t index) const
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return 0.0f;
  }

  const tvg::Fill::ColorStop* stops = nullptr;
  const uint32_t              count = mTvgFill->colorStops(&stops);
  if(!stops || index >= count)
  {
    DALI_LOG_ERROR("GetColorStopOffset() index %u out of range [0, %u)\n", index, count);
    return 0.0f;
  }

  return stops[index].offset;
#else
  return 0.0f;
#endif
}

Vector4 Gradient::GetColorStopColor(uint32_t index) const
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return Vector4::ZERO;
  }

  const tvg::Fill::ColorStop* stops = nullptr;
  const uint32_t              count = mTvgFill->colorStops(&stops);
  if(!stops || index >= count)
  {
    DALI_LOG_ERROR("GetColorStopColor() index %u out of range [0, %u)\n", index, count);
    return Vector4::ZERO;
  }

  return Vector4(stops[index].r / 255.0f, stops[index].g / 255.0f, stops[index].b / 255.0f, stops[index].a / 255.0f);
#else
  return Vector4::ZERO;
#endif
}

bool Gradient::SetSpread(Dali::CanvasRenderer::Gradient::Spread spread)
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return false;
  }
  if(mTvgFill->spread(static_cast<tvg::FillSpread>(spread)) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("SetSpread() fail.\n");
    return false;
  }
  SetChanged(true);

  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Gradient::Spread Gradient::GetSpread() const
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return Dali::CanvasRenderer::Gradient::Spread::PAD;
  }

  tvg::FillSpread spread = mTvgFill->spread();

  return static_cast<Dali::CanvasRenderer::Gradient::Spread>(spread);
#else
  return Dali::CanvasRenderer::Gradient::Spread::PAD;
#endif
}

void Gradient::SetObject(const void* object)
{
#ifdef THORVG_SUPPORT
  if(object)
  {
    mTvgFill = static_cast<tvg::Fill*>(const_cast<void*>(object));
  }
#endif
}

void* Gradient::GetObject() const
{
#ifdef THORVG_SUPPORT
  return static_cast<void*>(mTvgFill);
#else
  return nullptr;
#endif
}

void Gradient::SetChanged(bool changed)
{
  if(!mChanged && changed)
  {
    Adaptor::Get().RequestProcessEventsAndUpdate();
  }
  mChanged = !!changed;
}

bool Gradient::GetChanged() const
{
  return mChanged;
}
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
