#ifndef DALI_GRAPHICS_VULKAN_PIPELINE_H
#define DALI_GRAPHICS_VULKAN_PIPELINE_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-pipeline-create-info.h>
#include <dali/graphics-api/graphics-pipeline.h>
#include <string.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>

namespace Dali::Graphics::Vulkan
{
class Program;
class PipelineImpl;
class PipelineCache;
/**
 * @brief Pipeline class wraps the PipelineImpl
 */
class Pipeline : public Graphics::Pipeline
{
public:
  Pipeline() = delete;

  Pipeline(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller);

  /**
   *
   * @param createInfo
   * @param controller
   * @param pipelineCache
   */
  Pipeline(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller, PipelineCache* pipelineCache);

  /**
   * @brief Destructor
   */
  ~Pipeline() override;

  /**
   * @brief Returns pipeline implementation
   *
   * @return Valid pipeline implementation
   */
  [[nodiscard]] Vulkan::PipelineImpl& GetImpl() const
  {
    return *mPipeline;
  }

  /**
   * @brief Returns create info structure
   *
   * @return Valid create info structure
   */
  [[nodiscard]] const PipelineCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns controller
   *
   * @return reference to Controller
   */
  [[nodiscard]] VulkanGraphicsController& GetController() const;

  bool operator==(const PipelineImpl* impl) const;

  /**
   * @brief Run by UniquePtr to discard resource
   */
  void DiscardResource();

  /**
   * @brief Destroy resource
   *
   * Despite this class doesn't inherit Resource it must provide
   * (so it won't duplicate same data) same set of functions
   * so it can work with resource management functions of Controller.
   */
  void DestroyResource();

private:
  std::unique_ptr<PipelineImpl> mPipeline; // TODO: it may need to be changed when we have caching
};

} // namespace Dali::Graphics::Vulkan
#endif