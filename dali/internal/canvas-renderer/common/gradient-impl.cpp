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
#include <dali/internal/canvas-renderer/common/gradient-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
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

bool Gradient::SetColorStops(Dali::CanvasRenderer::Gradient::ColorStops& colorStops)
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return false;
  }
  SetChanged(true);

  tvg::Fill::ColorStop* tvgColorStops = (tvg::Fill::ColorStop*)alloca(sizeof(tvg::Fill::ColorStop) * colorStops.Count());

  for(unsigned int i = 0u; i < colorStops.Count(); ++i)
  {
    tvgColorStops[i].offset = colorStops[i].offset;
    tvgColorStops[i].r      = colorStops[i].color.r * 255.0f;
    tvgColorStops[i].g      = colorStops[i].color.g * 255.0f;
    tvgColorStops[i].b      = colorStops[i].color.b * 255.0f;
    tvgColorStops[i].a      = colorStops[i].color.a * 255.0f;
  }

  if(mTvgFill->colorStops(tvgColorStops, colorStops.Count()) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("SetColorStops() fail.\n");
    return false;
  }

  return true;
#else
  return false;
#endif
}

Dali::CanvasRenderer::Gradient::ColorStops Gradient::GetColorStops() const
{
#ifdef THORVG_SUPPORT
  if(!mTvgFill)
  {
    DALI_LOG_ERROR("Fill(Gradient) is null [%p]\n", this);
    return Dali::CanvasRenderer::Gradient::ColorStops();
  }

  const tvg::Fill::ColorStop* tvgColorStops = nullptr;
  uint32_t                    count         = 0;

  count = mTvgFill->colorStops(&tvgColorStops);
  if(!tvgColorStops || count <= 0)
  {
    DALI_LOG_ERROR("GetColorStops() fail.\n");
    return Dali::CanvasRenderer::Gradient::ColorStops();
  }

  Dali::CanvasRenderer::Gradient::ColorStops colorStops;

  colorStops.Reserve(count);

  for(unsigned int i = 0u; i < count; ++i)
  {
    Dali::CanvasRenderer::Gradient::ColorStop stop = {tvgColorStops[i].offset, Vector4(tvgColorStops[i].r / 255.0f, tvgColorStops[i].g / 255.0f, tvgColorStops[i].b / 255.0f, tvgColorStops[i].a / 255.0f)};

    colorStops.PushBack(stop);
  }
  return colorStops;
#else
  return Dali::CanvasRenderer::Gradient::ColorStops();
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
  if(!mChanged && changed) Dali::Stage::GetCurrent().KeepRendering(0.0f);
  mChanged = !!changed;
}

bool Gradient::GetChanged() const
{
  return mChanged;
}
} // namespace Adaptor

} // namespace Internal

} // namespace Dali
