#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_RESOURCE_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_RESOURCE_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

namespace Dali::Graphics::Vulkan
{
class VulkanGraphicsController;

/**
 * @brief Base class for the Graphics resource.
 */
template<class BASE, class CreateInfo>
class Resource : public BASE
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo CreateInfo structure for the resource
   * @param[in] controller Reference to the controller
   */
  Resource(const CreateInfo& createInfo, VulkanGraphicsController& controller)
  : mCreateInfo(createInfo),
    mController(controller)
  {
  }

  /**
   * @brief Tries to recycle Graphics resource
   *
   * If False returned, the object must be initialized with use of constructor
   *
   * By default, all graphics resources are non-recyclable
   *
   * @param[in] createInfo CreateInfo structure of new object
   * @param[in] controller Reference to the controller
   * @return True on success, False otherwise
   */
  virtual bool TryRecycle(const CreateInfo& createInfo, VulkanGraphicsController& controller)
  {
    return false;
  }

  /**
   * @brief Destructor
   */
  ~Resource() override = default;

  /**
   * @brief Retrieves create info structure
   * @return Reference to the create info structure
   */
  [[nodiscard]] const CreateInfo& GetCreateInfo() const
  {
    return mCreateInfo;
  }

  /**
   * @brief Retrieves controller
   * @return Reference to the controller object
   */
  [[nodiscard]] VulkanGraphicsController& GetController() const
  {
    return mController;
  }

  /**
   * @brief Destroys resource
   *
   * This function must be implemented by the derived class.
   * It should perform final destruction of used GL resources.
   */
  virtual void DestroyResource() = 0;

  /**
   * @brief Initializes resource
   *
   * This function must be implemented by the derived class.
   * It should initialize all necessary GL resources.
   *
   * @return True on success
   */
  virtual bool InitializeResource() = 0;

  /**
   * @brief Discards resource by adding it to the discard queue
   */
  virtual void DiscardResource() = 0;

  /**
   * @brief returns pointer to base
   * @return
   */
  BASE* GetBase()
  {
    return this;
  }

protected:
  CreateInfo                mCreateInfo; ///< CreateInfo structure
  VulkanGraphicsController& mController; ///< Reference to the Controller object
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RESOURCE_H
