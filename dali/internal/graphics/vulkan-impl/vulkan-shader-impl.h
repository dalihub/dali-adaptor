#ifndef DALI_GRAPHICS_VULKAN_SHADER_IMPL_H
#define DALI_GRAPHICS_VULKAN_SHADER_IMPL_H

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

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-shader-create-info.h>
#include <dali/graphics-api/graphics-shader.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-handle.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class ShaderImpl;
using ShaderHandle = Handle<class ShaderImpl>;

class ShaderImpl : public VkSharedResource
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo Valid createInfo structure
   * @param[in] controller Reference to the controller
   */
  ShaderImpl(const Graphics::ShaderCreateInfo& createInfo, VulkanGraphicsController& controller);

  /**
   * @brief destructor
   */
  ~ShaderImpl();

  /**
   * @brief Destroys Vulkan shader module
   */
  void DestroyShaderModule();

  /**
   * Whilst unreferenced, increase the flush count and return it
   *
   * @return The new flush count
   */
  [[nodiscard]] uint32_t IncreaseFlushCount();

  /**
   * Get the flush count whilst unreferenced
   *
   * @return the flush count
   */
  [[nodiscard]] uint32_t GetFlushCount() const;

  /**
   * @brief Compiles shader
   *
   * @return True on success
   */
  [[nodiscard]] bool Compile() const;

  /**
   * @brief Returns Vulkan resource
   * @return Valid Vulkan shader resource
   */
  vk::ShaderModule GetVkShaderModule() const;

  /**
   * @brief Returns create info structure
   * @return Returns valid create info structure
   */
  [[nodiscard]] const ShaderCreateInfo& GetCreateInfo() const;

  /**
   * @brief Returns reference to the graphics controller
   * @return Valid reference to the graphics controller
   */
  [[nodiscard]] VulkanGraphicsController& GetController() const;

  /**
   * Strips legacy prefix from the GLSL source code if necessary
   * @param info valid ShaderCreateInfo structure
   * @param[out] startIndex Start index of the source code
   * @param[out] glslVersion Detected GLSL version of legacy shader
   * @param[out] finalDataSize Size of trimmed data
   */
  static void StripLegacyCodeIfNeeded(const ShaderCreateInfo& info, size_t& startIndex, uint32_t& glslVersion, size_t& finalDataSize);

  /**
   * @brief Sets preprocess code
   * @param[in] data Valid pointer to the new source code
   * @param[in] size Size of the source code
   */
  void SetPreprocessedCode(void* data, uint32_t size);

  /**
   * @brief Returns GLSL version
   * @return Returns valid GLSL version or 0 if undefined
   */
  [[nodiscard]] uint32_t GetGLSLVersion() const;

private:
  friend class Shader;
  struct Impl;
  std::unique_ptr<Impl> mImpl{nullptr}; // TODO: see if we need it (PipelineCache related)
};

} // namespace Dali::Graphics::Vulkan

#endif
