#ifndef DALI_GRAPHICS_GLES_CONTEXT_H
#define DALI_GRAPHICS_GLES_CONTEXT_H

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

#include <dali/graphics-api/graphics-command-buffer.h>
#include "gles-context-state-cache.h"
#include "gles-graphics-types.h"

namespace Dali::Graphics
{
class EglGraphicsController;
namespace GLES
{
class Pipeline;
class RenderPass;
class RenderTarget;
class Texture;
class TextureDependencyChecker;

/**
 * @brief Context represents single GLES context
 */
class Context
{
public:
  explicit Context(EglGraphicsController& controller, Integration::GlAbstraction* glAbstraction);

  ~Context();

  /**
   * @brief Flushes the context
   *
   * Flushes the context by issuing GL calls to set the required
   * state. Causes a glWaitSync if any drawn textures are dependent
   * on another context.
   *
   * @param[in] reset If true then state is reset unconditionally
   * @param[in] drawCall the draws that need flushing
   * @param[in] dependencyChecker The texture dependecy checker
   */
  void Flush(bool reset, const GLES::DrawCallDescriptor& drawCall, GLES::TextureDependencyChecker& dependencyChecker);

  /**
   * @brief Returns context Id
   * @return
   */
  uint32_t GetContextId() const
  {
    return 0;
  }

  /**
   * @brief Binds textures to the context
   *
   * Bindings are merged, they are resolved at later time
   * when the draw call is issued. Then the texture binding
   * slots (GL active texture binding) are enabled/disabled accordingly
   * to the uniform layout.
   *
   * @param[in] bindings List of bindings
   *
   */
  void BindTextures(const Graphics::TextureBinding* bindings, uint32_t count);

  /**
   * @brief Vertex buffers to bind
   *
   * The bindings are taken from a command buffer being
   * currently processed and copied into the local storage.
   */

  void BindVertexBuffers(const GLES::VertexBufferBindingDescriptor* bindings, uint32_t count);

  /**
   * @brief Binds index buffer
   *
   * @param indexBufferBinding
   */
  void BindIndexBuffer(const IndexBufferBindingDescriptor& indexBufferBinding);

  /**
   * @brief Binds pipeline to the context
   * @param newPipeline
   */
  void BindPipeline(const GLES::Pipeline* newPipeline);

  /**
   * @brief Binds uniform buffers to the context
   *
   * @param[in] uboBindings UBO bindings include emulated (legacy) UBO object
   * @param[in] uboCount The number of UBO binding include emulated (legacy) UBO object
   */
  void BindUniformBuffers(const UniformBufferBindingDescriptor* uboBindings, uint32_t uboCount);

  /**
   * @brief Resolves blend state on the currently attached pipeline
   */
  void ResolveBlendState();

  /**
   * @brief Resolves rasterization state on the currently attached pipeline
   */
  void ResolveRasterizationState();

  /**
   * @brief Resolves uniform buffers and binds data to the pipeline
   */
  void ResolveUniformBuffers();

  /**
   * @brief Special usecase for legacy shaders, called by ResolveUniformBuffers()
   */
  void ResolveStandaloneUniforms(const UniformBufferBindingDescriptor& standaloneUniformBinding);

  /**
   * @brief Resolves GPU-based uniform buffers
   */
  void ResolveGpuUniformBuffers(const UniformBufferBindingDescriptor* uboBindingPtr, uint32_t uboCount);

  /**
   * @brief Begins render pass for specified render target
   *
   * @param[in] renderPass render pass object to begin
   */
  void BeginRenderPass(const BeginRenderPassDescriptor& renderPassBegin);

  /**
   * @brief Ends render pass
   *
   * Ending render pass is necessary in order to ensure
   * proper explicit synchronization is in place
   */
  void EndRenderPass(TextureDependencyChecker& checker);

  /**
   * @brief Request to read pixels
   * @param[out] buffer to load pixel data.
   */
  void ReadPixels(uint8_t* buffer);

  /**
   * @brief Returns the cache of GL state in the context
   * @return the reference of GL state cache (which can be modified)
   */
  GLStateCache& GetGLStateCache();

  /**
   * @brief Called when the GL context has been created.
   */
  void GlContextCreated();

  /**
   * @brief Called when the GL context has been destroyed.
   */
  void GlContextDestroyed();

  /**
   * @brief Invalidates the cached pipeline object in the context if it matches
   * This is called before the pipeline is deleted
   *
   * @param[in] pipeline The pipeline
   */
  void InvalidateCachedPipeline(GLES::Pipeline* pipeline);

  /**
   * @brief Invalidates the cached native texture whenever given context prepared before.
   *
   * @param[in] nativeTexture The native texture which might be cached in this context.
   */
  void InvalidateCachedNativeTexture(GLES::Texture* nativeTexture);

  /**
   * @brief Sets up EGL context for native rendering
   *
   * - The native rendering uses dedicated context
   * - There is one EGL native rendering context per GLES::Context object
   * - Native rendering context is compatible with the window/surface context
   * - Native rendering context dies with GLES::Context object
   *
   * When native rendering is about to be executed, the dedicated EGL context
   * is acquired (created or reused) and made current. The Window/Surface context
   * is cached to be restored afterwards.
   */
  void PrepareForNativeRendering();

  /**
   * @brief Restores window/surface context after native rendering.
   */
  void RestoreFromNativeRendering();

  void ActiveTexture(uint32_t textureBindingIndex);
  void BindTexture(GLenum target, BoundTextureType textureTypeId, uint32_t textureId);
  void GenerateMipmap(GLenum target);

  /**
   * Binds the buffer.
   * @return true if the buffer was actually bound, false if it's cached
   */
  bool BindBuffer(GLenum target, uint32_t bufferId);

  /**
   * @brief Invalidate Renderbuffer for given framebuffer if they using.
   * Note : We should call this API before framebuffer unbind.
   * @param[in] framebuffer The framebuffer to be invalidate
   */
  void InvalidateDepthStencilRenderBuffers(GLES::Framebuffer* framebuffer);

  void ColorMask(bool enabled);
  void ClearStencilBuffer();
  void ClearDepthBuffer();
  void ClearBuffer(uint32_t mask, bool forceClear);
  void SetScissorTestEnabled(bool scissorEnabled);
  void SetStencilTestEnable(bool stencilEnable);
  void StencilMask(uint32_t writeMask);
  void StencilFunc(Graphics::CompareOp compareOp,
                   uint32_t            reference,
                   uint32_t            compareMask);
  void StencilOp(Graphics::StencilOp failOp,
                 Graphics::StencilOp depthFailOp,
                 Graphics::StencilOp passOp);
  void SetDepthCompareOp(Graphics::CompareOp compareOp);
  void SetDepthTestEnable(bool depthTestEnable);
  void SetDepthWriteEnable(bool depthWriteEnable);

  /**
   * @brief Remove cached texture id and mark we should bind 0 at next frame.
   */
  void ResetTextureCache();

  /**
   * @brief Remove cached buffer id and mark we should bind buffers as 0 at next frame.
   */
  void ResetBufferCache();

  /**
   * @brief Reset GLES default states
   * @param[in] callGlFunction Call gl functions to match with cached state or not.
   */
  void ResetGLESState(bool callGlFunction = false);

private:
  /**
   * @brief Clear current state
   */
  void ClearState();

  /**
   * @brief Clear vertex buffer relative caches.
   */
  void ClearVertexBufferCache();

  /**
   * @brief Clear cached bind buffer state
   */
  void ClearUniformBufferCache();

  /**
   * @brief Clear cached native textures and notify to invalidate this context.
   */
  void ClearCachedNativeTexture();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};
} // namespace GLES
} // namespace Dali::Graphics
#endif // DALI_GRAPHICS_GLES_CONTEXT_H
