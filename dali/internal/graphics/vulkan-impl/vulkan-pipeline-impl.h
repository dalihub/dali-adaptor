#ifndef DALI_GRAPHICS_VULKAN_PIPELINE_IMPL_H
#define DALI_GRAPHICS_VULKAN_PIPELINE_IMPL_H

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
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>

namespace Dali::Graphics::Vulkan
{
class Program;
class PipelineCache; // TODO
/**
 * @brief PipelineImpl is the implementation of Pipeline
 *
 * PipelineImpl is owned by the pipeline cache. The client-side
 * will receive Graphics::Pipeline objects which are only
 * wrappers for this implementation. The lifecycle of
 * PipelineImpl is managed by the PipelineCache.
 */
class PipelineImpl
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo valid TextureCreateInfo structure
   * @param[in] controller Reference to the Controller
   * @param[in] pipelineCache Pointer to valid pipeline cache or nullptr if not using cache
   */
  PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller, PipelineCache* pipelineCache);

  /**
   * @brief Destructor
   */
  ~PipelineImpl();

  /**
   * @brief Binds pipeline
   */
  void Bind();

  /**
   * @brief Increases ref count
   */
  void Retain();

  /**
   * @brief Decreases ref count
   */
  void Release();

  /**
   * @brief Retrieves ref count
   * @return Refcount value
   */
  [[nodiscard]] uint32_t GetRefCount() const;

  /**
   * @brief Returns PipelineCreateInfo structure
   *
   * @return PipelineCreateInfo structure
   */
  [[nodiscard]] const PipelineCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns controller
   *
   * @return Reference to the Controller
   */
  [[nodiscard]] VulkanGraphicsController& GetController() const;

private:
  void InitializePipeline();

  void InitializeVertexInputState(vk::PipelineVertexInputStateCreateInfo& out);
  void InitializeInputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo& out) const;
  void InitializeViewportState(vk::PipelineViewportStateCreateInfo& out);
  void InitializeMultisampleState(vk::PipelineMultisampleStateCreateInfo& out);
  void InitializeRasterizationState(vk::PipelineRasterizationStateCreateInfo& out) const;
  bool InitializeDepthStencilState(vk::PipelineDepthStencilStateCreateInfo& out);
  void InitializeColorBlendState(vk::PipelineColorBlendStateCreateInfo& out);

  /**
   * @brief Helper function. Copies state if pointer is set
   */
  template<class T>
  void CopyStateIfSet(const T* sourceState, T& copyState, T** destState)
  {
    *destState = nullptr;
    if(sourceState)
    {
      copyState  = *sourceState;
      *destState = &copyState;
    }
  }

  /**
   * @brief Helper function. Copies const state if pointer is set
   */
  template<class T>
  void CopyStateIfSet(const T* sourceState, T& copyState, const T** destState)
  {
    *destState = nullptr;
    if(sourceState)
    {
      copyState  = *sourceState;
      *destState = &copyState;
    }
  }

  struct PipelineState;
  std::unique_ptr<PipelineState> mPipelineState;
  VulkanGraphicsController&      mController;
  PipelineCreateInfo             mCreateInfo;

  // Vertex input state data
  std::vector<vk::VertexInputBindingDescription>   mVertexInputBindingDescriptionList;
  std::vector<vk::VertexInputAttributeDescription> mVertexInputAttributeDescriptionList;

  // Viewport state data
  vk::Viewport mViewport;
  vk::Rect2D   mScissor;

  // Blend state data (using vector if we support more than one)
  std::vector<vk::PipelineColorBlendAttachmentState> mBlendStateAttachments;

  std::vector<vk::DynamicState> mDynamicStates;

  vk::Pipeline mVkPipeline;
};

} // namespace Dali::Graphics::Vulkan
#endif