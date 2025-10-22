#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_RESOURCE_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_RESOURCE_H

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
 *
 */

#include <dali/graphics-api/graphics-types.h>

namespace Dali::Graphics::Vulkan
{
class VulkanGraphicsController;

/**
 * Interface class for graphics resources
 */
class ResourceBase
{
public: // Types
  enum class InitializationResult
  {
    NOT_SUPPORTED,
    INITIALIZED,
    NOT_INITIALIZED_YET
  };

public:
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
   * @return on success, either INITIALIZED or NOT_INITIALIZED_YET
   * if it's lazily created on use.
   */
  virtual InitializationResult InitializeResource() = 0;

  /**
   * @brief Discards resource by adding it to the discard queue
   */
  virtual void DiscardResource() = 0;

  virtual void InvokeDeleter() = 0;

  [[nodiscard]] virtual const Graphics::AllocationCallbacks* GetAllocationCallbacks() const = 0;

  virtual ~ResourceBase() = default;
};

class ResourceWithoutDeleter : public ResourceBase
{
  [[nodiscard]] const Graphics::AllocationCallbacks* GetAllocationCallbacks() const override
  {
    return nullptr;
  }

  void InvokeDeleter() override
  {
    delete this;
  }
};

/**
 * Resource must implement GetAllocationCallbacks and InvokeDeleter.
 */
class ResourceWithDeleter : public ResourceBase
{
public:
  ~ResourceWithDeleter() override = default;
};

/**
 * @brief Base class for the Graphics resource.
 * A graphics resource is any Graphics API object created by the controller that
 * requires lifecycle management.
 * It explicitly does not include Vulkan Impl objects that wrap a vkHandle.
 */
template<class GraphicsType, class CreateInfo>
class Resource : public GraphicsType, public ResourceWithDeleter
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
   * @brief returns pointer to base
   * @return
   */
  GraphicsType* GetBase()
  {
    return this;
  }

protected:
  CreateInfo                mCreateInfo; ///< CreateInfo structure
  VulkanGraphicsController& mController; ///< Reference to the Controller object
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RESOURCE_H
