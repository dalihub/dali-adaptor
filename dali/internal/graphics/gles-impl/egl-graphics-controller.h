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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-controller.h>
#include <queue>

// INTERNAL INCLUDES
#include "gles-context.h"
#include "gles-graphics-buffer.h"
#include "gles-graphics-command-buffer.h"
#include "gles-graphics-framebuffer.h"
#include "gles-graphics-pipeline-cache.h"
#include "gles-graphics-pipeline.h"
#include "gles-graphics-reflection.h"
#include "gles-graphics-sampler.h"
#include "gles-graphics-shader.h"
#include "gles-graphics-texture.h"
#include "gles-graphics-types.h"
#include "gles2-graphics-memory.h"

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
namespace GLES
{
class CommandBuffer;
class PipelineCache;
} // namespace GLES

/**
 * EGL Implementation of the graphics controller.
 *
 * Temporarily holds the old GL abstractions whilst dali-core is migrated to the new API.
 */
DALI_IMPORT_API class EglGraphicsController : public Graphics::Controller
{
public:
  /**
   * @brief Deault constructor
   */
  EglGraphicsController() = default;

  /**
   * @brief Destructor
   */
  ~EglGraphicsController() override;

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
  void SubmitCommandBuffers(const SubmitInfo& submitInfo) override;

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
   * @copydoc Dali::Graphics::Shutdown()
   */
  void Shutdown() override
  {
    mIsShuttingDown = true;
  }

  /**
   * @copydoc Dali::Graphics::Destroy()
   */
  void Destroy() override
  {
    // Final flush
    Flush();
  }

  /**
   * @copydoc Dali::Graphics::UpdateTextures()
   */
  void UpdateTextures(const std::vector<TextureUpdateInfo>&       updateInfoList,
                      const std::vector<TextureUpdateSourceInfo>& sourceList) override;

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
  Graphics::UniquePtr<Buffer> CreateBuffer(const BufferCreateInfo& bufferCreateInfo, Graphics::UniquePtr<Buffer>&& oldBuffer) override;

  /**
   * @copydoc Dali::Graphics::CreateCommandBuffer()
   */
  Graphics::UniquePtr<CommandBuffer> CreateCommandBuffer(const CommandBufferCreateInfo& commandBufferCreateInfo, Graphics::UniquePtr<CommandBuffer>&& oldCommandBuffer) override;

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
  Graphics::UniquePtr<Texture> CreateTexture(const TextureCreateInfo& textureCreateInfo, Graphics::UniquePtr<Texture>&& oldTexture) override;

  /**
   * @copydoc Dali::Graphics::CreateFramebuffer()
   */
  Graphics::UniquePtr<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& framebufferCreateInfo, Graphics::UniquePtr<Framebuffer>&& oldFramebuffer) override;

  /**
   * @copydoc Dali::Graphics::CreatePipeline()
   */
  Graphics::UniquePtr<Pipeline> CreatePipeline(const PipelineCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Pipeline>&& oldPipeline) override;

  /**
   * @copydoc Dali::Graphics::CreateProgram()
   */
  Graphics::UniquePtr<Program> CreateProgram(const ProgramCreateInfo& programCreateInfo, UniquePtr<Program>&& oldProgram) override;

  /**
   * @copydoc Dali::Graphics::CreateShader()
   */
  Graphics::UniquePtr<Shader> CreateShader(const ShaderCreateInfo& shaderCreateInfo, Graphics::UniquePtr<Shader>&& oldShader) override;

  /**
   * @copydoc Dali::Graphics::CreateSampler()
   */
  Graphics::UniquePtr<Sampler> CreateSampler(const SamplerCreateInfo& samplerCreateInfo, Graphics::UniquePtr<Sampler>&& oldSampler) override;

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
  Graphics::UniquePtr<Memory> MapBufferRange(const MapBufferInfo& mapInfo) override;

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
   * @copydoc Dali::Graphics::Controller::GetPipelineReflection()
   */

  [[nodiscard]] const Reflection& GetProgramReflection(const Graphics::Program& program) override;

  /**
   * @copydoc Dali::Graphics::PipelineEquals()
   */
  [[nodiscard]] bool PipelineEquals(const Pipeline& pipeline0, const Pipeline& pipeline1) const override
  {
    return {};
  }

  [[nodiscard]] Integration::GlAbstraction* GetGL() const
  {
    if(mIsShuttingDown)
    {
      return nullptr;
    }
    return mGlAbstraction;
  }

  // Internal
  void AddTexture(GLES::Texture& texture);

  /**
   * @brief Adds buffer to the creation queue
   * @param buffer
   */
  void AddBuffer(GLES::Buffer& buffer);

  /**
   * @brief Adds framebuffer to the creation queue
   * @param buffer
   */
  void AddFramebuffer(GLES::Framebuffer& framebuffer);

  /**
   * @brief Pushes Bufer to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] texture Pointer to the texture
   */
  void DiscardResource(GLES::Texture* texture)
  {
    mDiscardTextureQueue.push(texture);
  }

  /**
   * @brief Pushes Buffer to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] buffer Pointer to the buffer object
   */
  void DiscardResource(GLES::Buffer* buffer)
  {
    mDiscardBufferQueue.push(buffer);
  }

  /**
   * @brief Pushes framebuffer to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] framebuffer Pointer to the framebuffer object
   */
  void DiscardResource(GLES::Framebuffer* framebuffer)
  {
    mDiscardFramebufferQueue.push(framebuffer);
  }

  /**
   * @brief Pushes Program to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the program
   */
  void DiscardResource(GLES::Program* program)
  {
    mDiscardProgramQueue.push(program);
  }

  /**
   * @brief Pushes Shader to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the Shader
   */
  void DiscardResource(GLES::Shader* shader)
  {
    mDiscardShaderQueue.push(shader);
  }

  /**
   * @brief Pushes CommandBuffer to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the CommandBuffer
   */
  void DiscardResource(GLES::CommandBuffer* commandBuffer)
  {
    mDiscardCommandBufferQueue.push(commandBuffer);
  }

  /**
   * @brief Pushes Sampler to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the Sampler
   */
  void DiscardResource(GLES::Sampler* sampler)
  {
    mDiscardSamplerQueue.push(sampler);
  }

  /**
   * @brief Pushes Pipeline to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the pipeline
   */
  void DiscardResource(GLES::Pipeline* pipeline)
  {
    mDiscardPipelineQueue.push(pipeline);
  }

  /**
   * @brief Flushes all pending updates
   *
   * Function flushes all pending resource constructions,
   * executes command buffers and empties discard queues.
   */
  void Flush()
  {
    // Process creations
    ProcessCreateQueues();

    // Process updates
    ProcessTextureUpdateQueue();

    // Process main command queue
    ProcessCommandQueues();

    // Process discards
    ProcessDiscardQueues();

    // Flush pipeline cache to remove unused pipelines
    if(mPipelineCache)
    {
      mPipelineCache->FlushCache();
    }
  }

  // Test update to tick controller, usually it will run own thread
  void ProcessDiscardQueues();

  /**
   * @brief Processes a create queue for type specified
   *
   * @param[in,out] queue Reference to the create queue
   */
  template<class T>
  void ProcessCreateQueue(T& queue)
  {
    while(!queue.empty())
    {
      auto* object = queue.front();
      queue.pop();

      // Initialize texture
      if(!object->InitializeResource())
      {
        // TODO: handle error
      }
    }
  }

  /**
   * @brief Processes a create queue for type specified
   *
   * @param[in,out] queue Reference to the create queue
   */
  template<class U, class T>
  void ProcessDiscardQueue(T& queue)
  {
    while(!queue.empty())
    {
      auto* object = queue.front();

      // Destroy
      object->DestroyResource();

      // Free
      auto* clbk = object->GetCreateInfo().allocationCallbacks;
      if(clbk)
      {
        // Call destructor
        object->~U();

        // Free memory
        clbk->freeCallback(object, clbk->userData);
      }
      else
      {
        delete object;
      }
      queue.pop();
    }
  }

  /**
   * @brief Processes all resource create queues
   */
  void ProcessCreateQueues();

  /**
   * @brief Process command queues and buffers
   */
  void ProcessCommandQueues();

  /**
   * @brief Executes all pending texture updates
   */
  void ProcessTextureUpdateQueue();

  /**
   * @brief Returns program custom parameter
   *
   * This function can be used as a backdoor in order to retrieve
   * certain data out of implementation
   *
   * @param[in] program Valid Program object
   * @param parameterId Integer id of parameter
   * @param outData Output data
   * @return True if parameter retrieved
   */
  bool GetProgramParameter(Graphics::Program& program, uint32_t parameterId, void* outData) override;

  /**
   * @brief Returns pipeline cache object
   *
   * @return Valid pipeline cache object
   */
  [[nodiscard]] GLES::PipelineCache& GetPipelineCache() const;

  /**
   * @brief Returns runtime supported GLES version
   *
   * @return GLES version enum
   */
  GLES::GLESVersion GetGLESVersion() const
  {
    // TODO: return proper version but for now we can
    // test fallbacks
    return GLES::GLESVersion::GLES_20;
  }

  bool IsShuttingDown() const
  {
    return mIsShuttingDown;
  }

private:
  Integration::GlAbstraction*              mGlAbstraction{nullptr};
  Integration::GlSyncAbstraction*          mGlSyncAbstraction{nullptr};
  Integration::GlContextHelperAbstraction* mGlContextHelperAbstraction{nullptr};

  std::queue<GLES::Texture*> mCreateTextureQueue;  ///< Create queue for texture resource
  std::queue<GLES::Texture*> mDiscardTextureQueue; ///< Discard queue for texture resource

  std::queue<GLES::Buffer*> mCreateBufferQueue;  ///< Create queue for buffer resource
  std::queue<GLES::Buffer*> mDiscardBufferQueue; ///< Discard queue for buffer resource

  std::queue<GLES::Program*>       mDiscardProgramQueue;       ///< Discard queue for program resource
  std::queue<GLES::Pipeline*>      mDiscardPipelineQueue;      ///< Discard queue of pipelines
  std::queue<GLES::Shader*>        mDiscardShaderQueue;        ///< Discard queue of shaders
  std::queue<GLES::Sampler*>       mDiscardSamplerQueue;       ///< Discard queue of samplers
  std::queue<GLES::CommandBuffer*> mDiscardCommandBufferQueue; ///< Discard queue of command buffers
  std::queue<GLES::Framebuffer*>   mCreateFramebufferQueue;    ///< Create queue for framebuffer resource
  std::queue<GLES::Framebuffer*>   mDiscardFramebufferQueue;   ///< Discard queue for framebuffer resource

  std::queue<GLES::CommandBuffer*> mCommandQueue; ///< we may have more in the future

  using TextureUpdateRequest = std::pair<TextureUpdateInfo, TextureUpdateSourceInfo>;
  std::queue<TextureUpdateRequest> mTextureUpdateRequests;

  std::unique_ptr<GLES::Context> mContext{nullptr}; ///< Context object handling command buffers execution

  std::unique_ptr<GLES::PipelineCache> mPipelineCache{nullptr}; ///< Internal pipeline cache

  bool mIsShuttingDown{false}; ///< Indicates whether the controller is shutting down
};

} // namespace Graphics

} // namespace Dali

#endif //DALI_EGL_GRAPHICS_CONTROLLER_H
