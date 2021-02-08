#ifndef DALI_EGL_GRAPHICS_CONTROLLER_H
#define DALI_EGL_GRAPHICS_CONTROLLER_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/graphics-api/graphics-controller.h>

namespace Dali
{
namespace Integration
{
class GlAbstraction;
class GlSyncAbstraction;
class GlContextHelperAbstraction;
} // namespace Integration

namespace Graphics
{
/**
 * EGL Implementation of the graphics controller.
 *
 * Temporarily holds the old GL abstractions whilst dali-core is migrated to the new API.
 */
DALI_IMPORT_API class EglGraphicsController : public Graphics::Controller
{
public:
  EglGraphicsController() = default;

  ~EglGraphicsController() override = default;

  /**
   * Initialize the GLES abstraction. This can be called from the main thread.
   */
  void InitializeGLES(Integration::GlAbstraction& glAbstraction);

  /**
   * Initialize with a reference to the GL abstractions.
   *
   * Note, this is now executed in the render thread, after core initialization
   */
  void Initialize(Integration::GlSyncAbstraction&          glSyncAbstraction,
                  Integration::GlContextHelperAbstraction& glContextHelperAbstraction);

  Integration::GlAbstraction&              GetGlAbstraction() override;
  Integration::GlSyncAbstraction&          GetGlSyncAbstraction() override;
  Integration::GlContextHelperAbstraction& GetGlContextHelperAbstraction() override;

  /**
   * @copydoc Dali::Graphics::SubmitCommandBuffers()
   */
  void SubmitCommandBuffers(const SubmitInfo& submitInfo) override
  {
  }

  /**
   * @copydoc Dali::Graphics::PresentRenderTarget()
   */
  void PresentRenderTarget(RenderTarget* renderTarget) override
  {
  }

  /**
   * @copydoc Dali::Graphics::WaitIdle()
   */
  void WaitIdle() override
  {
  }

  /**
   * @copydoc Dali::Graphics::Pause()
   */
  void Pause() override
  {
  }

  /**
   * @copydoc Dali::Graphics::Resume()
   */
  void Resume() override
  {
  }

  /**
   * @copydoc Dali::Graphics::UpdateTextures()
   */
  void UpdateTextures(const std::vector<TextureUpdateInfo>&       updateInfoList,
                      const std::vector<TextureUpdateSourceInfo>& sourceList) override
  {
  }

  /**
   * @copydoc Dali::Graphics::EnableDepthStencilBuffer()
   */
  bool EnableDepthStencilBuffer(bool enableDepth, bool enableStencil) override
  {
    return {};
  }

  /**
   * @copydoc Dali::Graphics::RunGarbageCollector()
   */
  void RunGarbageCollector(size_t numberOfDiscardedRenderers) override
  {
  }

  /**
   * @copydoc Dali::Graphics::DiscardUnusedResources()
   */
  void DiscardUnusedResources() override
  {
  }

  /**
   * @copydoc Dali::Graphics::IsDiscardQueueEmpty()
   */
  bool IsDiscardQueueEmpty() override
  {
    return {};
  }

  /**
   * @copydoc Dali::Graphics::IsDrawOnResumeRequired()
   */
  bool IsDrawOnResumeRequired() override
  {
    return {};
  }

  /**
   * @copydoc Dali::Graphics::CreateBuffer()
   */
  Graphics::UniquePtr<Buffer> CreateBuffer(const BufferCreateInfo& bufferCreateInfo, Graphics::UniquePtr<Buffer>&& oldBuffer) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateCommandBuffer()
   */
  Graphics::UniquePtr<CommandBuffer> CreateCommandBuffer(const CommandBufferCreateInfo& commandBufferCreateInfo, Graphics::UniquePtr<CommandBuffer>&& oldCommandBuffer) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateRenderPass()
   */
  Graphics::UniquePtr<RenderPass> CreateRenderPass(const RenderPassCreateInfo& renderPassCreateInfo, Graphics::UniquePtr<RenderPass>&& oldRenderPass) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateTexture()
   */
  Graphics::UniquePtr<Texture> CreateTexture(const TextureCreateInfo& textureCreateInfo, Graphics::UniquePtr<Texture>&& oldTexture) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateFramebuffer()
   */
  Graphics::UniquePtr<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& framebufferCreateInfo, Graphics::UniquePtr<Framebuffer>&& oldFramebuffer) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreatePipeline()
   */
  Graphics::UniquePtr<Pipeline> CreatePipeline(const PipelineCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Pipeline>&& oldPipeline) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateShader()
   */
  Graphics::UniquePtr<Shader> CreateShader(const ShaderCreateInfo& shaderCreateInfo, Graphics::UniquePtr<Shader>&& oldShader) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateSampler()
   */
  Graphics::UniquePtr<Sampler> CreateSampler(const SamplerCreateInfo& samplerCreateInfo, Graphics::UniquePtr<Sampler>&& oldSampler) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::CreateRenderTarget()
   */
  Graphics::UniquePtr<RenderTarget> CreateRenderTarget(const RenderTargetCreateInfo& renderTargetCreateInfo, Graphics::UniquePtr<RenderTarget>&& oldRenderTarget) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::MapBufferRange()
   */
  Graphics::UniquePtr<Memory> MapBufferRange(const MapBufferInfo& mapInfo) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::MapTextureRange()
   */
  Graphics::UniquePtr<Memory> MapTextureRange(const MapTextureInfo& mapInfo) override
  {
    return nullptr;
  }

  /**
   * @copydoc Dali::Graphics::UnmapMemory()
   */
  void UnmapMemory(Graphics::UniquePtr<Memory> memory) override
  {
  }
  /**
   * @copydoc Dali::Graphics::GetTextureMemoryRequirements()
   */
  MemoryRequirements GetTextureMemoryRequirements(Texture& texture) const override
  {
    return {};
  }

  /**
   * @copydoc Dali::Graphics::GetBufferMemoryRequirements()
   */
  MemoryRequirements GetBufferMemoryRequirements(Buffer& buffer) const override
  {
    return {};
  }

  /**
   * @copydoc Dali::Graphics::GetTextureProperties()
   */
  const TextureProperties& GetTextureProperties(const Texture& texture) override
  {
    // for compiler not to moan
    static TextureProperties dummy{};
    return dummy;
  }

  /**
   * @copydoc Dali::Graphics::PipelineEquals()
   */
  [[nodiscard]] bool PipelineEquals(const Pipeline& pipeline0, const Pipeline& pipeline1) const override
  {
    return {};
  }

  [[nodiscard]] Integration::GlAbstraction* GetGL() const
  {
    return mGlAbstraction;
  }

private:
  Integration::GlAbstraction*              mGlAbstraction{nullptr};
  Integration::GlSyncAbstraction*          mGlSyncAbstraction{nullptr};
  Integration::GlContextHelperAbstraction* mGlContextHelperAbstraction{nullptr};
};

} // namespace Graphics

} // namespace Dali

#endif //DALI_EGL_GRAPHICS_CONTROLLER_H
