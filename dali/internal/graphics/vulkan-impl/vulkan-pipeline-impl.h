#ifndef DALI_GRAPHICS_VULKAN_PIPELINE_IMPL_H
#define DALI_GRAPHICS_VULKAN_PIPELINE_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-pipeline-create-info.h>
#include <dali/graphics-api/graphics-pipeline.h>
#include <string.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-reflection.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>

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

  vk::Pipeline GetVkPipeline() const;

  void InitializePipeline();

  const Vulkan::Program* GetProgram() const;

  /**
   * Compares the depth stencil states and the color write mask.
   * @param[in] state The depth/stencil states
   * @param[in] colorWriteMask The color write mask (usually true)
   */
  bool ComparePipelineDepthStencilState(vk::PipelineDepthStencilStateCreateInfo& state,
                                        bool                                     colorWriteMask);

  vk::Pipeline CloneInheritedVkPipeline(vk::PipelineDepthStencilStateCreateInfo& dsState,
                                        bool                                     colorWriteMask);

private:
  /**
   * @brief Validates render pass compatibility
   *
   * This function checks if the current cached render pass is compatible with the
   * current framebuffer, and remove incompatible pipelines from the caches.
   */
  void ValidateRenderPassCompatibility();

  /**
   * @brief Gets the current render pass implementation from the render target
   * @return Handle to the current render pass implementation, or empty handle if not available
   */
  RenderPassHandle GetCurrentRenderPassImpl();

  /**
   * @brief Clears the pipeline caches
   *
   * This function destroys all pipelines in both the depth state cache and main pipeline cache,
   */
  void ClearPipelineCaches();

  /**
   * @brief Removes pipelines with incompatible render passes from the cache
   *
   * This function selectively removes only pipelines that are incompatible with the specified
   * render pass, preserving compatible pipelines for better performance.
   * @param[in] currentRenderPass The render pass to check compatibility against
   */
  void RemoveIncompatiblePipelines(RenderPassHandle currentRenderPass);

  /**
   * @brief Finds an existing pipeline that matches the specified depth state and render pass
   *
   * This function searches both the depth state cache and main pipeline cache to find
   * a reusable pipeline that matches the given depth state and is compatible with the
   * current render pass.
   * @param[in] dsState The depth stencil state to match
   * @param[in] colorWriteMask the color write mask to match (enabled/disabled)
   * @param[in] currentRenderPassImpl The current render pass implementation
   * @return Found pipeline, or nullptr if no matching pipeline exists
   */
  vk::Pipeline FindExistingPipeline(
    vk::PipelineDepthStencilStateCreateInfo& dsState,
    bool                                     colorWriteMask,
    RenderPassHandle                         currentRenderPassImpl);

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

  struct RenderPassPipelinePair
  {
    RenderPassHandle renderPass;
    vk::Pipeline     pipeline;
  };

  // Main pipeline cache for render pass compatibility checking
  // Stores pipelines with their associated render passes for reuse when render passes are compatible
  std::vector<RenderPassPipelinePair> mVkPipelines;

  vk::GraphicsPipelineCreateInfo           mVkPipelineCreateInfo{};
  vk::PipelineVertexInputStateCreateInfo   mVertexInputState{};
  vk::PipelineInputAssemblyStateCreateInfo mInputAssemblyState{};
  vk::PipelineViewportStateCreateInfo      mViewportState{};
  vk::PipelineRasterizationStateCreateInfo mRasterizationState{};
  vk::PipelineMultisampleStateCreateInfo   mMultisampleState{};
  vk::PipelineDepthStencilStateCreateInfo  mDepthStencilState{};
  vk::PipelineColorBlendStateCreateInfo    mColorBlendState{};
  vk::PipelineDynamicStateCreateInfo       mDynamicState{};

  struct DepthStatePipelineHashed
  {
    uint32_t                                hash;
    vk::PipelineDepthStencilStateCreateInfo ds;
    vk::Pipeline                            pipeline;
    bool                                    colorWriteMask{true};
  };

  // Depth state cache for fast hash lookups during frequent depth state switching
  // Stores pipelines with their depth state hash for quick reuse when the same depth state is needed
  std::vector<DepthStatePipelineHashed> mPipelineForDepthStateCache;
};

} // namespace Dali::Graphics::Vulkan
#endif
