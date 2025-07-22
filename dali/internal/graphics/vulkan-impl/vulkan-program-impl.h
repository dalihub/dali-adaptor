#ifndef DALI_GRAPHICS_VULKAN_PROGRAM_IMPL_H
#define DALI_GRAPHICS_VULKAN_PROGRAM_IMPL_H

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
 */

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-program-create-info.h>
#include <dali/graphics-api/graphics-program.h>
#include <vulkan/vulkan.hpp>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

namespace Dali::Graphics::Vulkan
{
class Reflection;

/**
 * @brief Program implementation.
 *
 * Program implementation is owned only by the PipelineCache
 *
 * Like pipeline, it's created and managed by the PipelineCache
 */
class ProgramImpl
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] createInfo Valid create info structure
   * @param[in] controller Valid reference to the controller object
   */
  ProgramImpl(const Graphics::ProgramCreateInfo& createInfo, VulkanGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~ProgramImpl();

  /**
   * @brief Destroys Vulkan resources associated with the Program
   *
   * @return True on success
   */
  bool Destroy();

  /**
   * @brief Creates Vulkan resource for this Program
   *
   * @return True on success
   */
  bool Create();

  /**
   * @brief Preprocesses shaders
   */
  bool Preprocess();

  /**
   * @brief Returns Graphics program id
   *
   * Since Vulkan has no concept of Program, the Graphics resource
   * should be returned.
   *
   * @return Graphics program id
   */
  [[nodiscard]] uint32_t GetSPIRVProgram() const;

  /**
   * @brief Increases ref count
   *
   * @return new refcount
   */
  uint32_t Retain();

  /**
   * @brief Decreases ref count
   *
   * @return New refcount
   */
  uint32_t Release();

  /**
   * @brief Retrieves ref count
   * @return Refcount value
   */
  [[nodiscard]] uint32_t GetRefCount() const;

  /**
   * @brief Returns reflection
   *
   * @return Valid reflection associated with the Program
   */
  [[nodiscard]] const Vulkan::Reflection& GetReflection() const;

  /**
   * @brief Returns controller
   *
   * @return Valid Controller object
   */
  [[nodiscard]] VulkanGraphicsController& GetController() const;

  /**
   * @brief Returns create info structure
   *
   * @return Reference to valid create info structure
   */
  [[nodiscard]] const ProgramCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns list of shader stages
   *
   * @return List of shader stages
   */
  [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& GetVkPipelineShaderStageCreateInfoList() const;

  /**
   * @brief Returns parameter value specified by parameterId
   *
   * This function can be used as a backdoor into the implementation
   * used to retrieve internal data.
   *
   * @param[in] parameterId Integer parameter id
   * @param[out] out Pointer to write to
   *
   * @return True on success
   */
  bool GetParameter(uint32_t parameterId, void* out);

  /**
   * @brief Add a descriptor pool
   * @param[in] poolCapacity the capacity of pool
   */
  void AddDescriptorPool(uint32_t poolCapacity);

  /**
   * @brief Allocates descriptor sets for the current frame
   * @param[in] frameIndex Current frame index
   * @param[in] maxSetsPerFrame Maximum number of descriptor sets needed for this frame
   * @return True on success
   */
  [[nodiscard]] bool AllocateDescriptorSetsForFrame(uint32_t frameIndex, uint32_t maxSetsPerFrame);

  /**
   * @brief Pre-allocate a batch of descriptor sets for the given descriptor pool in the given frame
   * @param[in] frameIndex Index of the frame in flight
   * @param[in] pool Descriptor pool
   * @param[in] setCount Number of descriptor sets to allocate
   */
  void PreAllocateDescriptorSetsFromPool(uint32_t frameIndex, vk::DescriptorPool pool, uint32_t setCount);

  /**
   * @brief Grow the descriptor pool for a given frame and pre-allocate new sets
   * @param[in] frameIndex Index of the frame in flight
   * @param[in] newCapacity Desired new maximum sets for this frame
   * @return True if pool was grown (or already large enough), false on failure
   */
  [[nodiscard]] bool GrowDescriptorPool(uint32_t frameIndex, uint32_t newCapacity);

  /**
   * @brief Gets the next available descriptor set for the current frame
   * @param[in] frameIndex Current frame index
   * @return Descriptor set handle or VK_NULL_HANDLE if none available
   */
  [[nodiscard]] vk::DescriptorSet GetNextDescriptorSetForFrame(uint32_t frameIndex);

  /**
   * @brief Resets descriptor sets for the frame so that they can be reused
   * @param[in] frameIndex Frame index to reset
   */
  void ResetDescriptorSetsForFrame(uint32_t frameIndex);

private:
  friend class Program;

  struct Impl;
  std::unique_ptr<Impl> mImpl; // TODO: see if we need it (PipelineCache related)
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_PROGRAM_IMPL_H
