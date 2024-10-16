/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-defines.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline-impl.h>
// TODO: Caching
//#include <dali/internal/graphics/vulkan-impl/gles-graphics-pipeline-cache.h"
#include <dali/internal/graphics/vulkan-impl/vulkan-program.h>

namespace Dali::Graphics::Vulkan
{
const PipelineCreateInfo& Pipeline::GetCreateInfo() const
{
  return mPipeline->GetCreateInfo();
}

VulkanGraphicsController& Pipeline::GetController() const
{
  return mPipeline->GetController();
}

Pipeline::~Pipeline() = default;

Pipeline::Pipeline(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller)
{
  // Not cached
  mPipeline = std::make_unique<PipelineImpl>(createInfo, controller, nullptr);
}

Pipeline::Pipeline(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller, PipelineCache* pipelineCache)
{
  // TODO: pipeline caching. pipeline cache can be nullptr and be bypassed for now
  // This constructor isn't in use currently and will most likely change
  mPipeline = std::make_unique<PipelineImpl>(createInfo, controller, pipelineCache);
}

bool Pipeline::InitializeResource()
{
  return true;
}

void Pipeline::DiscardResource()
{
  // Send pipeline to discard queue if refcount is 0
  GetController().DiscardResource(this);
}

void Pipeline::DestroyResource()
{
  // Nothing to do here
}

bool Pipeline::operator==(const PipelineImpl* impl) const
{
  return false; // &mPipeline == impl; // TODO: needs caching
}

} // namespace Dali::Graphics::Vulkan
