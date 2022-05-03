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
#include <dali/integration-api/graphics-sync-abstraction.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles-impl/gles-context.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-buffer.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-command-buffer.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-framebuffer.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-pipeline-cache.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-pipeline.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-reflection.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-sampler.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-shader.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-texture.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-types.h>
#include <dali/internal/graphics/gles-impl/gles2-graphics-memory.h>

namespace Dali
{
namespace Integration
{
class GlAbstraction;
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
class EglGraphicsController : public Graphics::Controller
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
  void Initialize(Integration::GraphicsSyncAbstraction&    syncImplementation,
                  Integration::GlContextHelperAbstraction& glContextHelperAbstraction,
                  Internal::Adaptor::GraphicsInterface&    graphicsInterface);

  Integration::GlAbstraction&               GetGlAbstraction() override;
  Integration::GlContextHelperAbstraction&  GetGlContextHelperAbstraction() override;
  Internal::Adaptor::EglSyncImplementation& GetEglSyncImplementation();

  /**
   * @copydoc Dali::Graphics::SubmitCommandBuffers()
   */
  void SubmitCommandBuffers(const SubmitInfo& submitInfo) override;

  /**
   * @copydoc Dali::Graphics::PresentRenderTarget()
   */
  void PresentRenderTarget(RenderTarget* renderTarget) override;

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

    ClearTextureUpdateQueue();

    // Remove all create queue and command queue.
    // Note that all memory are already deallocated at Final flush.
    mCreateTextureQueue              = {};
    mCreateBufferQueue               = {};
    mCreateFramebufferQueue          = {};
    mTextureMipmapGenerationRequests = {};
    mCommandQueue                    = {};

    if(mContext)
    {
      mContext->GlContextDestroyed();
    }

    for(auto&& context : mSurfaceContexts)
    {
      if(context.second)
      {
        context.second->GlContextDestroyed();
      }
    }
  }

  /**
   * @copydoc Dali::Graphics::UpdateTextures()
   */
  void UpdateTextures(const std::vector<TextureUpdateInfo>&       updateInfoList,
                      const std::vector<TextureUpdateSourceInfo>& sourceList) override;

  /**
   * @copydoc Dali::Graphics::GenerateTextureMipmaps()
   */
  void GenerateTextureMipmaps(const Texture& texture) override;

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
  Graphics::UniquePtr<RenderPass> CreateRenderPass(const RenderPassCreateInfo& renderPassCreateInfo, Graphics::UniquePtr<RenderPass>&& oldRenderPass) override;

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
  Graphics::UniquePtr<RenderTarget> CreateRenderTarget(const RenderTargetCreateInfo& renderTargetCreateInfo, Graphics::UniquePtr<RenderTarget>&& oldRenderTarget) override;

  /**
   * @copydoc Dali::Graphics::CreateSyncObject()
   */
  Graphics::UniquePtr<SyncObject> CreateSyncObject(const SyncObjectCreateInfo&       syncObjectCreateInfo,
                                                   Graphics::UniquePtr<SyncObject>&& oldSyncObject) override;

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

  [[nodiscard]] Internal::Adaptor::GraphicsInterface* GetGraphicsInterface() const
  {
    return mGraphics;
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
   * @brief Pushes RenderPass to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the RenderPass
   */
  void DiscardResource(GLES::RenderPass* renderPass)
  {
    mDiscardRenderPassQueue.push(renderPass);
  }

  /**
   * @brief Pushes RenderTarget to the discard queue
   *
   * Function is called from the UniquePtr custom deleter.
   *
   * @param[in] program Pointer to the RenderTarget
   */
  void DiscardResource(GLES::RenderTarget* renderTarget)
  {
    mDiscardRenderTargetQueue.push(renderTarget);
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
   * @brief Clears the texture update queue
   */
  void ClearTextureUpdateQueue()
  {
    // Remove remained CPU-allocated texture memory
    while(!mTextureUpdateRequests.empty())
    {
      auto& request = mTextureUpdateRequests.front();
      auto& source  = request.second;

      if(source.sourceType == Graphics::TextureUpdateSourceInfo::Type::MEMORY)
      {
        // free staging memory
        free(source.memorySource.memory);
      }
      mTextureUpdateRequests.pop();
    }
  }

  /**
   * @brief Flushes all pending updates
   *
   * Function flushes all pending resource constructions,
   * executes command buffers and empties discard queues.
   */
  void Flush()
  {
    if(DALI_LIKELY(!mIsShuttingDown))
    {
      if(!mCreateTextureQueue.empty() ||
         !mCreateBufferQueue.empty() ||
         !mCreateFramebufferQueue.empty() ||
         !mTextureUpdateRequests.empty() ||
         !mTextureMipmapGenerationRequests.empty())
      {
        mGraphics->ActivateResourceContext();
      }

      // Process creations
      ProcessCreateQueues();

      // Process updates
      ProcessTextureUpdateQueue();

      // Process texture mipmap generation requests
      ProcessTextureMipmapGenerationQueue();

      // Process main command queue
      ProcessCommandQueues();
    }

    // Reset texture cache in the contexts while destroying textures
    ResetTextureCache();

    // Reset buffer cache in the contexts while destroying buffers
    ResetBufferCache();

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
   * @brief Processes a discard queue for type specified
   *
   * @param[in,out] queue Reference to the discard queue
   */
  template<class U, class T>
  void ProcessDiscardQueue(T& queue)
  {
    while(!queue.empty())
    {
      auto* object = const_cast<U*>(queue.front());

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
   * @brief Processes a discard queue for pipeline
   *
   * @param[in,out] queue Reference to the create queue
   */
  void ProcessDiscardQueue(std::queue<GLES::Pipeline*>& queue)
  {
    while(!queue.empty())
    {
      auto* object = const_cast<GLES::Pipeline*>(queue.front());

      // Inform the contexts to invalidate the pipeline if cached
      if(mContext)
      {
        mContext->InvalidateCachedPipeline(object);
      }

      for(auto&& context : mSurfaceContexts)
      {
        if(context.second)
        {
          context.second->InvalidateCachedPipeline(object);
        }
      }

      // Destroy
      object->DestroyResource();

      // Free
      auto* clbk = object->GetCreateInfo().allocationCallbacks;
      if(clbk)
      {
        // Call destructor
        using GLESPipeline = GLES::Pipeline;
        object->~GLESPipeline();

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
   * @brief Executes all pending texture mipmap generation
   */
  void ProcessTextureMipmapGenerationQueue();

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
    return mGLESVersion;
  }

  /**
   * @brief Sets runtime supported GLES version
   *
   * @param[in] glesVersion The runtime supported GLES version
   */
  void SetGLESVersion(GLES::GLESVersion glesVersion)
  {
    mGLESVersion = glesVersion;
  }

  bool IsShuttingDown() const
  {
    return mIsShuttingDown;
  }

  /**
   * @brief Reset texture cache in the contexts
   */
  void ResetTextureCache()
  {
    if(mContext)
    {
      mContext->GetGLStateCache().ResetTextureCache();
    }

    for(auto& context : mSurfaceContexts)
    {
      if(context.second)
      {
        context.second->GetGLStateCache().ResetTextureCache();
      }
    }
  }

  /**
   * @brief Reset buffer cache in the contexts
   */
  void ResetBufferCache()
  {
    if(mContext)
    {
      mContext->GetGLStateCache().ResetBufferCache();
    }

    for(auto& context : mSurfaceContexts)
    {
      if(context.second)
      {
        context.second->GetGLStateCache().ResetBufferCache();
      }
    }
  }

  void ProcessCommandBuffer(const GLES::CommandBuffer& commandBuffer);

  // Resolves presentation
  void ResolvePresentRenderTarget(GLES::RenderTarget* renderTarget);

  /**
   * Creates a GLES context for the given render surface
   *
   * @param[in] surface The surface whose GLES context to be created.
   */
  void CreateSurfaceContext(Dali::RenderSurfaceInterface* surface);

  /**
   * Deletes a GLES context
   *
   * @param[in] surface The surface whose GLES context to be deleted.
   */
  void DeleteSurfaceContext(Dali::RenderSurfaceInterface* surface);

  /**
   * Activate the resource context (shared surfaceless context)
   */
  void ActivateResourceContext();

  /**
   * Activate the surface context
   *
   * @param[in] surface The surface whose context to be switched to.
   */
  void ActivateSurfaceContext(Dali::RenderSurfaceInterface* surface);

  /**
   * @brief Returns the current context
   *
   * @return the current context
   */
  GLES::Context* GetCurrentContext() const
  {
    return mCurrentContext;
  }

private:
  Integration::GlAbstraction*              mGlAbstraction{nullptr};
  Integration::GlContextHelperAbstraction* mGlContextHelperAbstraction{nullptr};

  Internal::Adaptor::EglSyncImplementation* mEglSyncImplementation{nullptr};
  Internal::Adaptor::GraphicsInterface*     mGraphics{nullptr}; // Pointer to owning structure via interface.

  std::queue<GLES::Texture*> mCreateTextureQueue;  ///< Create queue for texture resource
  std::queue<GLES::Texture*> mDiscardTextureQueue; ///< Discard queue for texture resource

  std::queue<GLES::Buffer*> mCreateBufferQueue;  ///< Create queue for buffer resource
  std::queue<GLES::Buffer*> mDiscardBufferQueue; ///< Discard queue for buffer resource

  std::queue<GLES::Program*>             mDiscardProgramQueue;       ///< Discard queue for program resource
  std::queue<GLES::Pipeline*>            mDiscardPipelineQueue;      ///< Discard queue of pipelines
  std::queue<GLES::RenderPass*>          mDiscardRenderPassQueue;    ///< Discard queue for renderpass resource
  std::queue<GLES::RenderTarget*>        mDiscardRenderTargetQueue;  ///< Discard queue for rendertarget resource
  std::queue<GLES::Shader*>              mDiscardShaderQueue;        ///< Discard queue of shaders
  std::queue<GLES::Sampler*>             mDiscardSamplerQueue;       ///< Discard queue of samplers
  std::queue<const GLES::CommandBuffer*> mDiscardCommandBufferQueue; ///< Discard queue of command buffers
  std::queue<GLES::Framebuffer*>         mCreateFramebufferQueue;    ///< Create queue for framebuffer resource
  std::queue<GLES::Framebuffer*>         mDiscardFramebufferQueue;   ///< Discard queue for framebuffer resource

  std::queue<GLES::CommandBuffer*> mCommandQueue; ///< we may have more in the future

  using TextureUpdateRequest = std::pair<TextureUpdateInfo, TextureUpdateSourceInfo>;
  std::queue<TextureUpdateRequest> mTextureUpdateRequests;

  std::queue<const GLES::Texture*> mTextureMipmapGenerationRequests; ///< Queue for texture mipmap generation requests

  GLES::Context*                 mCurrentContext{nullptr}; ///< The current context
  std::unique_ptr<GLES::Context> mContext{nullptr};        ///< Context object handling command buffers execution
  using SurfaceContextPair = std::pair<Dali::RenderSurfaceInterface*, std::unique_ptr<GLES::Context>>;
  std::vector<SurfaceContextPair> mSurfaceContexts; ///< Vector of surface context objects handling command buffers execution

  std::unique_ptr<GLES::PipelineCache> mPipelineCache{nullptr}; ///< Internal pipeline cache

  GLES::GLESVersion mGLESVersion{GLES::GLESVersion::GLES_20}; ///< Runtime supported GLES version
  uint32_t          mTextureUploadTotalCPUMemoryUsed{0u};

  bool mIsShuttingDown{false}; ///< Indicates whether the controller is shutting down

  std::queue<const GLES::CommandBuffer*> mPresentationCommandBuffers{}; ///< Queue of reusable command buffers used by presentation engine
};

} // namespace Graphics

} // namespace Dali

#endif //DALI_EGL_GRAPHICS_CONTROLLER_H
