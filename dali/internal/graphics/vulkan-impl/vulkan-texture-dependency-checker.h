#pragma once

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

#include <dali/public-api/common/vector-wrapper.h>
#include <cstdint>

namespace Dali::Graphics::Vulkan
{
class VulkanGraphicsController;
class Texture;
class RenderTarget;

/**
 * Class to handle dependency checks between textures from different render targets.
 *
 * When submitting a render target to generate a framebuffer attachment, the controller
 * will create a signal semaphore that is triggered once the command buffer has completed
 * execution.
 * The dependency checker ensures that any dependent render targets that use the attachment
 * texture listen to this semaphore on their cmd buffer submission.
 */
class TextureDependencyChecker
{
public:
  explicit TextureDependencyChecker(VulkanGraphicsController& controller)
  : mController(controller)
  {
  }
  ~TextureDependencyChecker() = default;

  /**
   * Resets the dependency graph for regeneration this frame
   */
  void Reset();

  /**
   * Add Texture dependencies.
   *
   * @param[in] texture The texture that's output by this render target
   * @param[in] renderTarget The render target that generates this texture
   */
  void AddTexture(const Vulkan::Texture* texture, Vulkan::RenderTarget* renderTarget);

  /**
   * Check if the given texture needs syncing before being read. This will add any
   * existing dependencies to the given render target (will be used in command
   * submission to set up waiting semaphores)
   */
  void CheckNeedsSync(const Vulkan::Texture* texture, Vulkan::RenderTarget* renderTarget);

  const uint32_t INVALID_DEPENDENCY_INDEX = 0xffffffff;

  /**
   * Add render target.
   *
   * @param[in] renderTarget The render target to add to the dependency graph
   */
  void AddRenderTarget(Vulkan::RenderTarget* renderTarget);

  /**
   * Remove render target
   * @param[in] renderTarget The render target to remove from the dependency graph
   */
  void RemoveRenderTarget(Vulkan::RenderTarget* renderTarget);

private:
  struct TextureDependency
  {
    const Vulkan::Texture* texture;
    Vulkan::RenderTarget*  generator;
  };
  VulkanGraphicsController&      mController;
  std::vector<TextureDependency> mDependencies;
  std::vector<RenderTarget*>     mRenderTargets; ///< Maintained list of all current render targets
  uint32_t                       mCurrentIndex{0};
};

} // namespace Dali::Graphics::Vulkan
