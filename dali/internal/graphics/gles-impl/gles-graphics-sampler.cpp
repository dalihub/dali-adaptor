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
#include "gles-graphics-sampler.h"

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
Sampler::Sampler(const Graphics::SamplerCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: SamplerResource(createInfo, controller)
{
}

Sampler::~Sampler() = default;

void Sampler::DestroyResource()
{
  // For now, no GL resources are initialized so nothing to destroy
}

bool Sampler::InitializeResource()
{
  // For now, there is no support for the modern GL Sampler type (yet)
  return true;
}

void Sampler::DiscardResource()
{
  GetController().DiscardResource(this);
}
} // namespace Dali::Graphics::GLES
