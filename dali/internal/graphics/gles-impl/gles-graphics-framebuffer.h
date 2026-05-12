#ifndef DALI_GRAPHICS_GLES_FRAMEBUFFER_H
#define DALI_GRAPHICS_GLES_FRAMEBUFFER_H

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
 *
 */

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-framebuffer-create-info.h>
#include <dali/graphics-api/graphics-framebuffer.h>
#include <dali/integration-api/gl-abstraction.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
class Context;
using FramebufferResource = Resource<Graphics::Framebuffer, Graphics::FramebufferCreateInfo>;

class Framebuffer : public FramebufferResource
{
public:
  /**
   * @brief Set the shared context. All GLES::Framebuffer class will use given context.
   */
  static void SetSharedContext(Context* context);

  /**
   * @brief Constructor
   * @param[in] createInfo Valid createInfo structure
   * @param[in] controller Reference to the controller
   */
  Framebuffer(const Graphics::FramebufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~Framebuffer() override;

  /**
   * @brief Called when GL resources are destroyed
   */
  void DestroyResource() override;

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  bool InitializeResource() override;

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;

  /**
   * Used to bind the framebuffer, e.g. when offscreen changes
   */
  void Bind();

  [[nodiscard]] uint32_t GetGlFramebufferId() const;

  [[nodiscard]] uint32_t GetGlDepthBufferId() const;

  [[nodiscard]] uint32_t GetGlStencilBufferId() const;

  /**
   * @brief Updates the depth/stencil state
   * @param[in] depthStencilState The new depth/stencil state
   */
  void UpdateDepthStencilState(const Graphics::DepthStencilState& depthStencilState);

  /**
   * @brief Invalidate Renderbuffer after render completed.
   * Note : We should call this API before framebuffer unbind.
   */
  void InvalidateDepthStencilRenderBuffers();

private:
  /**
   * Attach a texture to the specified attachment point
   * @param[in] texture The texture to bind
   * @param[in] attachmentId The attachment point to bind it to
   * @param[in] layerId The texture layer (e.g. for cubemap)
   * @param[in] levelId The texture mipmap level
   */
  void AttachTexture(const Graphics::Texture* texture, uint32_t attachmentId, uint32_t layerId, uint32_t levelId);

  /**
   * @brief Bind and create render buffers if need.
   * @note Must be called after glBindFramebuffer.
   */
  void PrepareRenderBuffer();

private:
  uint32_t mFramebufferId{0u};
  uint32_t mDepthBufferId{0u};
  uint32_t mStencilBufferId{0u};
  uint32_t mMultisamples{1u};

  bool mInitialized : 1;
  bool mDepthBufferUsed : 1;
  bool mStencilBufferUsed : 1;

  // Attached renderbuffer state
  bool   mAttachedDepthWrite : 1;
  bool   mAttachedStencilWrite : 1;
  GLenum mAttachedAttachment;
  GLenum mAttachedInternalFormat;
};

} // namespace Dali::Graphics::GLES

#endif
