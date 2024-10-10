#ifndef DALI_GRAPHICS_VULKAN_PROGRAM_H
#define DALI_GRAPHICS_VULKAN_PROGRAM_H

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

// CLASS HEADER
#include <dali/graphics-api/graphics-program.h>

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-program-create-info.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

namespace Dali::Graphics::Vulkan
{
class Reflection;
class ProgramImpl;

/**
 * @brief Wrapper for the program implementation
 *
 * This object is returned back to the client-side
 *
 * Vulkan has no concept of Program. The Vulkan program
 * within Graphics API is a set of shader stages linked together
 * so the reflection can do its work on it.
 */
class Program : public Graphics::Program, public Vulkan::ResourceBase
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] impl Pointer to valid implementation
   */
  explicit Program(ProgramImpl* impl)
  {
    // TODO: needs PipelineCache
  }

  Program(const Graphics::ProgramCreateInfo& createInfo, VulkanGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~Program() override;

  /**
   * @brief Returns reference to the Reflection object
   * @return Reflection
   */
  [[nodiscard]] const Vulkan::Reflection& GetReflection() const;

  /**
   * @brief Retrieves internal program implementation
   *
   * @return Valid pointer to the ProgramImpl object
   */
  [[nodiscard]] ProgramImpl* GetImplementation() const
  {
    return mProgram;
  }

  /**
   * @brief Returns controller
   *
   * @return controller
   */
  [[nodiscard]] VulkanGraphicsController& GetController() const;

  /**
   * @brief Returns create info structure
   *
   * @return create info structure
   */
  [[nodiscard]] const ProgramCreateInfo& GetCreateInfo() const;

  bool operator==(const Vulkan::Program& program) const
  {
    return (program.mProgram == mProgram);
  }

  bool operator==(const Vulkan::ProgramImpl* programImpl) const
  {
    return (programImpl == mProgram);
  }

  bool operator!=(const Vulkan::Program& program) const
  {
    return (program.mProgram != mProgram);
  }

  /**
   * @brief Initialize the resource
   */
  bool InitializeResource() override;

  /**
   * @brief Run by UniquePtr to discard resource
   */
  void DiscardResource() override;

  /**
   * @brief Destroying resources
   *
   * This function is kept for compatibility with Resource<> class
   * so can the object can be use with templated functions.
   */
  void DestroyResource() override;

  bool TryRecycle(const Graphics::ProgramCreateInfo& createInfo, VulkanGraphicsController& controller)
  {
    return false;
  }

private:
  ProgramImpl* mProgram{nullptr};
};
} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_PROGRAM_H
