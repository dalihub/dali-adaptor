#ifndef DALI_GRAPHICS_VULKAN_SHADER_H
#define DALI_GRAPHICS_VULKAN_SHADER_H

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

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-shader-create-info.h>
#include <dali/graphics-api/graphics-shader.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

namespace Dali::Graphics::Vulkan
{
class ShaderImpl;
class Shader : public Graphics::Shader
{
public:
  /**
   * @brief Constructor
   *
   * @param[in] impl Pointer to valid implementation
   */
  explicit Shader(ShaderImpl* impl); // TODO: this should be controlled by cache

  Shader(const Graphics::ShaderCreateInfo& createInfo, VulkanGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~Shader() override;

  [[nodiscard]] ShaderImpl* GetImplementation() const
  {
    return mShader;
  }

  [[nodiscard]] const ShaderCreateInfo& GetCreateInfo() const;

  bool operator==(const Vulkan::Shader& shader) const
  {
    return (shader.mShader == mShader);
  }

  bool operator==(const Vulkan::ShaderImpl* shaderImpl) const
  {
    return (shaderImpl == mShader);
  }

  bool operator!=(const Vulkan::Shader& shader) const
  {
    return (shader.mShader != mShader);
  }

  /**
   * @brief Called when UniquePtr<> on client-side dies.
   */
  void DiscardResource();

  /**
   * @brief Destroying Vulkan resources
   *
   * This function is kept for compatibility with Resource<> class
   * so can the object can be use with templated functions.
   */
  void DestroyResource()
  {
    // nothing to do here
  }
  bool TryRecycle(const Graphics::ShaderCreateInfo& createInfo, VulkanGraphicsController& controller);

  [[nodiscard]] uint32_t GetGLSLVersion() const;

private:
  ShaderImpl* mShader{nullptr};
};

} // namespace Dali::Graphics::Vulkan

#endif
