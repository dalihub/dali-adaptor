#pragma once

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

#include <dali/public-api/common/vector-wrapper.h>
#include <cstdint>
#include <unordered_map>

namespace Dali::Graphics::Vulkan
{
class VulkanGraphicsController;
class Texture;
class RenderTarget;

/**
 * Class to handle dependency checks between textures from different
 * render targets.
 *
 * The dependency graph is held between this class and the render targets.
 * This class holds a list of generators; each render target holds a list
 * of dependent targets. The dependency graph is completely regenerated
 * each frame. (Could be ignored if we know render instructions haven't
 * changed this frame)
 *
 * There needs to be a barrier of some description between render
 * targets that generate textures and render targets that consume the
 * textures.  This checker ensures that each render target has a list
 * of dependent render targets that it relies on; so that the chosen
 * barrier can be invoked at the right time.
 *
 * Several mechanisms exist in Vulkan to set up different kinds of barrier:
 *
 * In the Submit queue, we can set up signal semaphores to signal when
 * each submitted command buffer has been processed. We can also set up
 * wait semaphores to delay processing other command buffers until
 * their dependent semaphores have been signalled. This is the current
 * implementation.
 *
 * Another possibility is to ensure that the texture draw has an image
 * barrier between the ImageLayout::eColorAttachmentOptimal state and
 * the ImageLayout::eShaderReadOnlyOptimal state. We can insert this
 * barrier prior to the first draw call in a command buffer that
 * utilizes this texture (we already delay binding textures until the
 * draw call). Adding this barrier should be a matter of invoking
 * Image::CreateMemoryBarrier in the
 * CommandBufferImpl::PrepareForDraw().
 *
 * Further possibilities exist. We could use sub-passes and define
 * appropriate barriers in there instead.(But, very complicated, and
 * payoff may be minimal)
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
  void AddTexture(Texture* texture, RenderTarget* renderTarget);

  /**
   * Check if the given texture needs syncing before being read. This will add any
   * existing dependencies to the given render target (will be used in command
   * submission to set up waiting semaphores)
   */
  void CheckNeedsSync(Texture* texture, RenderTarget* renderTarget);

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
  VulkanGraphicsController& mController;
  struct TextureGenerator
  {
    Vulkan::Texture*                   texture{nullptr};
    std::vector<Vulkan::RenderTarget*> generators; // More than one generator might write to this texture per frame, this is in render instruction order (i.e. final render task order)
  };
  std::unordered_map<Vulkan::RenderTarget*, uint32_t> mLookupByRenderTarget; // Use indexed lookup into mTextureGenerators
  std::unordered_map<Vulkan::Texture*, uint32_t>      mLookupByTexture;      // Use indexed lookup into mTextureGenerators
  std::vector<TextureGenerator>                       mTextureGenerators;    // current set of textures & generating render targets
  std::vector<Vulkan::RenderTarget*>                  mRenderTargets;        // set of all render targets that may consume textures.
};

} // namespace Dali::Graphics::Vulkan
