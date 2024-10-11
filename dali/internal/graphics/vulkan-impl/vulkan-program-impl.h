#ifndef DALI_GRAPHICS_VULKAN_PROGRAM_IMPL_H
#define DALI_GRAPHICS_VULKAN_PROGRAM_IMPL_H

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
 * @brief Program implementation
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
   *
   * @param[in] poolCapacity the capacity of pool
   * @param maxPoolCounts number of pools, last added pool will be removed
   * @return index of descriptor pool or -1 if unable to create pool
   */
  [[nodiscard]] int AddDescriptorPool(uint32_t poolCapacity, uint32_t maxPoolCounts);

  [[nodiscard]] vk::DescriptorSet AllocateDescriptorSet(int poolIndex);

private:
  friend class Program;

  struct Impl;
  std::unique_ptr<Impl> mImpl; // TODO: see if we need it (PipelineCache related)
};

} // namespace Dali::Graphics::Vulkan

#endif //DALI_GRAPHICS_VULKAN_PROGRAM_IMPL_H
