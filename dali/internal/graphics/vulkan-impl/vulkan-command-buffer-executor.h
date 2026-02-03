#ifndef DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_EXECUTOR_H
#define DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_EXECUTOR_H

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
 */

#include <dali/graphics-api/graphics-command-buffer-create-info.h>
#include <dali/graphics-api/graphics-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-stored-command-buffer.h>

namespace Dali::Graphics::Vulkan
{
class CommandBufferImpl;
class Swapchain;
class RenderTarget;
class StoredCommandPool;
class StoredCommandBuffer;
using CommandBufferResource = Resource<Graphics::CommandBuffer, Graphics::CommandBufferCreateInfo>;

/**
 * This class is used to execute recorded commands in a StoredCommandBuffer and write
 * them to a vulkan command buffer.
 */
class CommandBufferExecutor
{
public:
  explicit CommandBufferExecutor(VulkanGraphicsController& controller);
  ~CommandBufferExecutor();

  void ProcessCommandBuffer(StoredCommandBuffer* storedCommandBuffer, CommandBufferImpl* commandBufferImpl);

  void Begin(CommandBufferImpl* commandBufferImpl, const Graphics::CommandBufferBeginInfo& info);

  void End(CommandBufferImpl* commandBufferImpl);

  /**
   * @brief Begins render pass
   *
   * The function initialises rendering for specified RenderPass object
   * onto renderTarget. renderArea defines the scissor rect. Depends on the
   * renderPass spec, the clearValues may be used.
   *
   * Calling EndRenderPass() is necessary to finalize the render pass.
   *
   */
  void BeginRenderPass(CommandBufferImpl* commandBufferImpl, const BeginRenderPassDescriptor& renderPassBegin);

  /**
   * @brief Resets CommandBufferImpl
   *
   * This function resets the command buffer and discards all previously
   * recorded commands.
   *
   * Since the allocation may use internal memory pool of the CommandBufferImpl,
   * resetting doesn't have to discard all the resources (for example, it doesn't
   * need to destroy command but only move the pointer to the beginning of
   * the command buffer).
   *
   * It is useful if the command buffer has to be re-recorded frequently, for example,
   * every frame.
   */
  void Reset();

  /**
   * @brief Setup the stencil function and how subsequent draws will affect the stencil buffer.
   *
   * @param[in] failOp What happens to stencil buffer if drawing a pixel fails the stencil test
   * @param[in] passOp What happens to stencil buffer if drawing a pixel passes stencil & depth test
   * @param[in] depthFailOp What happens to stencil buffer if drawing a pixel passes stencil but fails depth test.
   * @param[in] compareOp How the stencil buffer, reference and compareMask are combined to determine whether to draw a pixel or not.
   * @param[in] reference A reference value that is ANDed with the mask in the compare op.
   * @param[in] compareMask The bitplanes from the stencil buffer that are active.
   */
  void SetStencilState(CommandBufferImpl*  commandBufferImpl,
                       Graphics::CompareOp compareOp,
                       uint32_t            reference,
                       uint32_t            compareMask,
                       Graphics::StencilOp failOp,
                       Graphics::StencilOp passOp,
                       Graphics::StencilOp depthFailOp);

  void SetStencilTest(CommandBufferImpl* commandBufferImpl, bool enabled);
  void SetDepthCompare(CommandBufferImpl* commandBufferImpl, Graphics::CompareOp compareOp);
  void SetDepthTest(CommandBufferImpl* commandBufferImpl, bool enabled);
  void SetDepthWrite(CommandBufferImpl* commandBufferImpl, bool enabled);

private:
  VulkanGraphicsController& mController;
  RenderTarget*             mRenderTarget{nullptr};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_COMMAND_BUFFER_H
