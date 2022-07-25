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
#include <dali/internal/vector-animation/common/vector-animation-renderer-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES

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

Dali::TypeRegistration type(typeid(Dali::VectorAnimationRenderer), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

VectorAnimationRendererPtr VectorAnimationRenderer::New()
{
  VectorAnimationRendererPtr renderer = new VectorAnimationRenderer();
  return renderer;
}

VectorAnimationRenderer::VectorAnimationRenderer()
: mPlugin(std::string())
{
}

VectorAnimationRenderer::~VectorAnimationRenderer()
{
}

void VectorAnimationRenderer::Finalize()
{
  mPlugin.Finalize();
}

bool VectorAnimationRenderer::Load(const std::string& url)
{
  return mPlugin.Load(url);
}

void VectorAnimationRenderer::SetRenderer(Dali::Renderer renderer)
{
  mPlugin.SetRenderer(renderer);
}

void VectorAnimationRenderer::SetSize(uint32_t width, uint32_t height)
{
  mPlugin.SetSize(width, height);
}

bool VectorAnimationRenderer::Render(uint32_t frameNumber)
{
  return mPlugin.Render(frameNumber);
}

uint32_t VectorAnimationRenderer::GetTotalFrameNumber() const
{
  return mPlugin.GetTotalFrameNumber();
}

float VectorAnimationRenderer::GetFrameRate() const
{
  return mPlugin.GetFrameRate();
}

void VectorAnimationRenderer::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  mPlugin.GetDefaultSize(width, height);
}

void VectorAnimationRenderer::GetLayerInfo(Property::Map& map) const
{
  mPlugin.GetLayerInfo(map);
}

bool VectorAnimationRenderer::GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const
{
  return mPlugin.GetMarkerInfo(marker, startFrame, endFrame);
}

void VectorAnimationRenderer::InvalidateBuffer()
{
  mPlugin.InvalidateBuffer();
}

void VectorAnimationRenderer::AddPropertyValueCallback(const std::string& keyPath, Dali::VectorAnimationRenderer::VectorProperty property, CallbackBase* callback, int32_t id)
{
  mPlugin.AddPropertyValueCallback(keyPath, property, callback, id);
}

Dali::VectorAnimationRenderer::UploadCompletedSignalType& VectorAnimationRenderer::UploadCompletedSignal()
{
  return mPlugin.UploadCompletedSignal();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
