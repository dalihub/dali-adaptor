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

// CLASS HEADER
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/trace.h>
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <dali/integration-api/graphics-sync-abstraction.h>
#include <dali/integration-api/pixel-data-integ.h>
#include <dali/internal/graphics/gles-impl/egl-sync-object.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-command-buffer.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-pipeline.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-program.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-render-pass.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-render-target.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-shader.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-texture.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-types.h>
#include <dali/internal/graphics/gles-impl/gles-sync-object.h>
#include <dali/internal/graphics/gles-impl/gles3-graphics-memory.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>
#include <dali/internal/system/common/environment-variables.h>
#include <any>

// Uncomment the following define to turn on frame dumping
//#define ENABLE_COMMAND_BUFFER_FRAME_DUMP 1
#include <dali/internal/graphics/gles-impl/egl-graphics-controller-debug.h>
DUMP_FRAME_INIT();

namespace Dali::Graphics
{
namespace
{
/**
 * @brief Custom deleter for all Graphics objects created
 * with use of the Controller.
 *
 * When Graphics object dies the unique pointer (Graphics::UniquePtr)
 * doesn't destroy it directly but passes the ownership back
 * to the Controller. The GLESDeleter is responsible for passing
 * the object to the discard queue (by calling Resource::DiscardResource()).
 */
template<typename T>
struct GLESDeleter
{
  GLESDeleter() = default;

  void operator()(T* object)
  {
    // GLES object deleter should skip discard queue if controller shutting down
    if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
    {
      // Discard resource (add it to discard queue)
      object->DiscardResource();
    }
    else
    {
      // Destroy and delete object otherwise
      if(DALI_LIKELY(object))
      {
        object->DestroyResource();
      }
      delete object;
    }
  }
};

template<>
struct GLESDeleter<GLES::SyncObject>
{
  GLESDeleter() = default;

  void operator()(GLES::SyncObject* object)
  {
    // Destroy and delete object otherwise
    if(DALI_LIKELY(object))
    {
      object->DestroyResource();
    }
    delete object;
  }
};

template<>
struct GLESDeleter<EGL::SyncObject>
{
  GLESDeleter() = default;

  void operator()(EGL::SyncObject* object)
  {
    // Destroy and delete object otherwise
    if(DALI_LIKELY(object))
    {
      object->DestroyResource();
    }
    delete object;
  }
};

/**
 * @brief Helper function allocating graphics object
 *
 * @param[in] info Create info structure
 * @param[in] controller Controller object
 * @param[out] out Unique pointer to the return object
 */
template<class GLESType, class GfxCreateInfo, class T>
auto NewObject(const GfxCreateInfo& info, EglGraphicsController& controller, T&& oldObject)
{
  // Use allocator
  using Type = typename T::element_type;
  using UPtr = Graphics::UniquePtr<Type>;
  if(info.allocationCallbacks)
  {
    auto* memory = info.allocationCallbacks->allocCallback(
      sizeof(GLESType),
      0,
      info.allocationCallbacks->userData);
    return UPtr(new(memory) GLESType(info, controller), GLESDeleter<GLESType>());
  }
  else // Use standard allocator
  {
    // We are given all object for recycling
    if(oldObject)
    {
      auto reusedObject = oldObject.release();
      // If succeeded, attach the object to the unique_ptr and return it back
      if(static_cast<GLESType*>(reusedObject)->TryRecycle(info, controller))
      {
        return UPtr(reusedObject, GLESDeleter<GLESType>());
      }
      else
      {
        // can't reuse so kill object by giving it back to original
        // unique pointer.
        oldObject.reset(reusedObject);
      }
    }

    // Create brand new object
    return UPtr(new GLESType(info, controller), GLESDeleter<GLESType>());
  }
}

template<class T0, class T1>
T0* CastObject(T1* apiObject)
{
  return static_cast<T0*>(apiObject);
}

// Maximum size of texture upload buffer.
const uint32_t TEXTURE_UPLOAD_MAX_BUFER_SIZE_MB = 1;

DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_EGL, false);

bool gIsShuttingDown = true; ///< Global static flag to ensure that we have single graphics controller instance per each UpdateRender thread loop.
} // namespace

bool EglGraphicsController::IsShuttingDown()
{
  return gIsShuttingDown;
}

EglGraphicsController::EglGraphicsController()
: mTextureDependencyChecker(*this),
  mSyncPool(*this)
{
}

EglGraphicsController::~EglGraphicsController()
{
  while(!mPresentationCommandBuffers.empty())
  {
    auto presentCommandBuffer = const_cast<GLES::CommandBuffer*>(mPresentationCommandBuffers.front());
    delete presentCommandBuffer;
    mPresentationCommandBuffers.pop();
  }
}

void EglGraphicsController::InitializeGLES(Integration::GlAbstraction& glAbstraction)
{
  DALI_LOG_RELEASE_INFO("Initializing Graphics Controller Phase 1\n");

  mGlAbstraction  = &glAbstraction;
  mContext        = std::make_unique<GLES::Context>(*this, mGlAbstraction);
  mCurrentContext = mContext.get();

  // Register shared context for framebuffers
  GLES::Framebuffer::SetSharedContext(mCurrentContext);

  static auto enableShaderUseProgramBinaryString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_SHADER_USE_PROGRAM_BINARY);
  mUseProgramBinary                              = enableShaderUseProgramBinaryString ? std::atoi(enableShaderUseProgramBinaryString) : false;
}

void EglGraphicsController::Initialize(Integration::GraphicsSyncAbstraction& syncImplementation,
                                       Graphics::GraphicsInterface&          graphicsInterface)
{
  DALI_LOG_RELEASE_INFO("Initializing Graphics Controller Phase 2\n");
  auto* syncImplPtr = static_cast<Internal::Adaptor::EglSyncImplementation*>(&syncImplementation);

  DALI_ASSERT_ALWAYS(gIsShuttingDown && "Don't initialize Phase 2 EglGraphicsController twice");
  gIsShuttingDown = false;

  mEglSyncImplementation = syncImplPtr;
  mGraphics              = &graphicsInterface;
}

void EglGraphicsController::FrameStart()
{
  mCapacity = 0; // Reset the command buffer capacity at the start of the frame.
}

void EglGraphicsController::SetResourceBindingHints(const std::vector<SceneResourceBinding>& resourceBindings)
{
  // Do nothing.
}

void EglGraphicsController::SubmitCommandBuffers(const SubmitInfo& submitInfo)
{
  uint32_t totalNumCmds    = 0;
  uint32_t totalNumBuffers = 0;
  for(auto& cmdbuf : submitInfo.cmdBuffer)
  {
    // Push command buffers
    auto* commandBuffer = static_cast<GLES::CommandBuffer*>(cmdbuf);
    mCapacity += commandBuffer->GetCapacity();
    uint32_t              numCmds = 0;
    [[maybe_unused]] auto cmdPtr  = commandBuffer->GetCommands(numCmds);
    totalNumCmds += numCmds;
    totalNumBuffers++;
    mCommandQueue.push(commandBuffer);
  }

  // If flush bit set, flush all pending tasks
  if(submitInfo.flags & (0 | SubmitFlagBits::FLUSH))
  {
    Flush();
  }
}

void EglGraphicsController::WaitIdle()
{
  Flush();
}

void EglGraphicsController::Shutdown()
{
  DALI_ASSERT_ALWAYS(!gIsShuttingDown && "Don't call EglGraphicsController::Shutdown twice");
  gIsShuttingDown = true;

  // Final flush
  Flush();

  // Invalidate shared context for framebuffers
  GLES::Framebuffer::SetSharedContext(nullptr);

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

void EglGraphicsController::PresentRenderTarget(RenderTarget* renderTarget)
{
  GLES::CommandBuffer* presentCommandBuffer{nullptr};
  if(mPresentationCommandBuffers.empty())
  {
    CommandBufferCreateInfo info;
    info.SetLevel(CommandBufferLevel::PRIMARY);
    info.fixedCapacity   = 1; // only one command
    presentCommandBuffer = new GLES::CommandBuffer(info, *this);
  }
  else
  {
    presentCommandBuffer = const_cast<GLES::CommandBuffer*>(mPresentationCommandBuffers.front());
    presentCommandBuffer->Reset();
    mPresentationCommandBuffers.pop();
  }
  presentCommandBuffer->PresentRenderTarget(static_cast<GLES::RenderTarget*>(renderTarget));
  SubmitInfo submitInfo;
  submitInfo.cmdBuffer = {presentCommandBuffer};
  submitInfo.flags     = 0 | SubmitFlagBits::FLUSH;
  SubmitCommandBuffers(submitInfo);
}

void EglGraphicsController::ResolvePresentRenderTarget(GLES::RenderTarget* renderTarget)
{
  mCurrentContext->InvalidateDepthStencilBuffers();

  if(DALI_LIKELY(renderTarget) && renderTarget->GetCreateInfo().surface)
  {
    auto* surfaceInterface = reinterpret_cast<Dali::Integration::RenderSurfaceInterface*>(renderTarget->GetCreateInfo().surface);
    surfaceInterface->MakeContextCurrent();
    surfaceInterface->PostRender();

    // Delete discarded surface context sync objects, and create new syncfence for NativeImage texture.
    // NOTE : We can assume that surface context is become current now.
    //        And also can asusme that ResolvePresentRenderTarget() will be called at most 1 times per each frame.
    const auto* currentSurfaceContext = GetSurfaceContext(surfaceInterface);
    mSyncPool.ProcessDiscardSyncObjects(currentSurfaceContext);

#ifdef DALI_PROFILE_TV
    /// Only TV profile should not create egl sync object before eglSwapBuffers, due to DDK bug. 2024-12-13. eunkiki.hong

    // NOTE : We need to call eglCreateSyncKHR after eglSwapBuffer if that sync will not be used 'before' swap buffer.
    //        Since given sync fence try to store rendering result of current frame which we usually don't need,
    //        so GPU memory increased.
    if(mTextureDependencyChecker.GetNativeTextureCount() > 0)
    {
      mTextureDependencyChecker.CreateNativeTextureSync(currentSurfaceContext);

      // Need to call glFlush or eglSwapBuffer after create sync object.
      mGlAbstraction->Flush();
    }
#endif
  }
  else
  {
    DALI_LOG_ERROR("ResolvePresentRenderTarget() failed! render target : %p\n", renderTarget);
  }
}

void EglGraphicsController::PostRender()
{
#ifdef DALI_PROFILE_TV
  /// Only TV profile should not create egl sync object before eglSwapBuffers, due to DDK bug. 2024-12-13. eunkiki.hong
  // eglCreateSyncKHR for FBO case.
  if(mTextureDependencyChecker.GetNativeTextureCount() > 0)
  {
    mTextureDependencyChecker.CreateNativeTextureSync(mCurrentContext);

    // Need to call glFlush or eglSwapBuffer after create sync object.
    mGlAbstraction->Flush();
  }
#endif

  mTextureDependencyChecker.Reset();
  mSyncPool.AgeSyncObjects();

  // Delete discarded resource context sync objects.
  // NOTE : We can assume that current context is resource context now.
  mSyncPool.ProcessDiscardSyncObjects(mCurrentContext);
}

Integration::GlAbstraction& EglGraphicsController::GetGlAbstraction()
{
  DALI_ASSERT_DEBUG(mGlAbstraction && "Graphics controller not initialized");
  return *mGlAbstraction;
}

Integration::GraphicsConfig& EglGraphicsController::GetGraphicsConfig()
{
  DALI_ASSERT_DEBUG(mGlAbstraction && "Graphics controller not initialized");
  return *mGlAbstraction;
}

Internal::Adaptor::EglSyncImplementation& EglGraphicsController::GetEglSyncImplementation()
{
  DALI_ASSERT_DEBUG(mEglSyncImplementation && "Sync implementation not initialized");
  return *mEglSyncImplementation;
}

Graphics::UniquePtr<CommandBuffer> EglGraphicsController::CreateCommandBuffer(
  const CommandBufferCreateInfo&       commandBufferCreateInfo,
  Graphics::UniquePtr<CommandBuffer>&& oldCommandBuffer)
{
  return NewObject<GLES::CommandBuffer>(commandBufferCreateInfo, *this, std::move(oldCommandBuffer));
}

Graphics::UniquePtr<RenderPass> EglGraphicsController::CreateRenderPass(const RenderPassCreateInfo& renderPassCreateInfo, Graphics::UniquePtr<RenderPass>&& oldRenderPass)
{
  return NewObject<GLES::RenderPass>(renderPassCreateInfo, *this, std::move(oldRenderPass));
}

Graphics::UniquePtr<Texture>
EglGraphicsController::CreateTexture(const TextureCreateInfo& textureCreateInfo, Graphics::UniquePtr<Texture>&& oldTexture)
{
  return NewObject<GLES::Texture>(textureCreateInfo, *this, std::move(oldTexture));
}

Graphics::UniquePtr<Buffer> EglGraphicsController::CreateBuffer(
  const BufferCreateInfo& bufferCreateInfo, Graphics::UniquePtr<Buffer>&& oldBuffer)
{
  return NewObject<GLES::Buffer>(bufferCreateInfo, *this, std::move(oldBuffer));
}

Graphics::UniquePtr<Framebuffer> EglGraphicsController::CreateFramebuffer(
  const FramebufferCreateInfo& framebufferCreateInfo, Graphics::UniquePtr<Framebuffer>&& oldFramebuffer)
{
  return NewObject<GLES::Framebuffer>(framebufferCreateInfo, *this, std::move(oldFramebuffer));
}

Graphics::UniquePtr<Pipeline> EglGraphicsController::CreatePipeline(
  const PipelineCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  // Create pipeline cache if needed
  if(!mPipelineCache)
  {
    mPipelineCache = std::make_unique<GLES::PipelineCache>(*this);
  }

  return mPipelineCache->GetPipeline(pipelineCreateInfo, std::move(oldPipeline));
}

Graphics::UniquePtr<Program> EglGraphicsController::CreateProgram(
  const ProgramCreateInfo& programCreateInfo, UniquePtr<Program>&& oldProgram)
{
  // Create pipeline cache if needed
  if(!mPipelineCache)
  {
    mPipelineCache = std::make_unique<GLES::PipelineCache>(*this);
  }

  return mPipelineCache->GetProgram(programCreateInfo, std::move(oldProgram));
}

Graphics::UniquePtr<Shader> EglGraphicsController::CreateShader(const ShaderCreateInfo& shaderCreateInfo, Graphics::UniquePtr<Shader>&& oldShader)
{
  // Create pipeline cache if needed
  if(!mPipelineCache)
  {
    mPipelineCache = std::make_unique<GLES::PipelineCache>(*this);
  }
  return mPipelineCache->GetShader(shaderCreateInfo, std::move(oldShader));
}

Graphics::UniquePtr<Sampler> EglGraphicsController::CreateSampler(const SamplerCreateInfo& samplerCreateInfo, Graphics::UniquePtr<Sampler>&& oldSampler)
{
  return NewObject<GLES::Sampler>(samplerCreateInfo, *this, std::move(oldSampler));
}

Graphics::UniquePtr<RenderTarget> EglGraphicsController::CreateRenderTarget(const RenderTargetCreateInfo& renderTargetCreateInfo, Graphics::UniquePtr<RenderTarget>&& oldRenderTarget)
{
  return NewObject<GLES::RenderTarget>(renderTargetCreateInfo, *this, std::move(oldRenderTarget));
}

Graphics::UniquePtr<SyncObject> EglGraphicsController::CreateSyncObject(const SyncObjectCreateInfo& syncObjectCreateInfo,
                                                                        UniquePtr<SyncObject>&&     oldSyncObject)
{
  if(GetGLESVersion() < GLES::GLESVersion::GLES_30)
  {
    return NewObject<EGL::SyncObject>(syncObjectCreateInfo, *this, std::move(oldSyncObject));
  }
  else
  {
    return NewObject<GLES::SyncObject>(syncObjectCreateInfo, *this, std::move(oldSyncObject));
  }
}

MemoryRequirements EglGraphicsController::GetBufferMemoryRequirements(Buffer& buffer) const
{
  MemoryRequirements requirements{};

  auto gl = GetGL();
  if(gl)
  {
    GLint align;
    gl->GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
    requirements.alignment = align;
  }
  return requirements;
}

TextureProperties EglGraphicsController::GetTextureProperties(const Texture& texture)
{
  const GLES::Texture* glesTexture = static_cast<const GLES::Texture*>(&texture);
  const auto&          createInfo  = glesTexture->GetCreateInfo();

  TextureProperties properties{};
  properties.format       = createInfo.format;
  properties.compressed   = glesTexture->IsCompressed();
  properties.extent2D     = createInfo.size;
  properties.nativeHandle = glesTexture->GetGLTexture();
  // TODO: Skip format1, emulated, packed, directWriteAccessEnabled of TextureProperties for now

  return properties;
}

const Graphics::Reflection& EglGraphicsController::GetProgramReflection(const Graphics::Program& program)
{
  return static_cast<const Graphics::GLES::Program*>(&program)->GetReflection();
}

void EglGraphicsController::CreateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface)
{
  std::unique_ptr<GLES::Context> context = std::make_unique<GLES::Context>(*this, mGlAbstraction);
  mSurfaceContexts.push_back(std::move(std::make_pair(surface, std::move(context))));
}

void EglGraphicsController::DeleteSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface)
{
  auto       iter       = mSurfaceContexts.begin();
  auto       newEndIter = mSurfaceContexts.begin();
  const auto endIter    = mSurfaceContexts.end();
  for(; iter != endIter; ++iter)
  {
    if(iter->first != surface)
    {
      if(newEndIter != iter)
      {
        *newEndIter = std::move(*iter);
      }
      newEndIter++;
      continue;
    }
    else
    {
      // Mark as given context will be deleted soon.
      // It will make sync object id that created by given context
      // become invalidated.
      mSyncPool.InvalidateContext(iter->second.get());
    }
  }

  mSurfaceContexts.erase(newEndIter, endIter);
}

void EglGraphicsController::ActivateResourceContext()
{
  mCurrentContext = mContext.get();
  mCurrentContext->GlContextCreated();
  if(!mSharedContext)
  {
    auto eglGraphics = dynamic_cast<Dali::Internal::Adaptor::EglGraphics*>(mGraphics);
    if(eglGraphics)
    {
      mSharedContext = eglGraphics->GetEglImplementation().GetContext();
    }
  }
}

void EglGraphicsController::ActivateSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface)
{
  if(surface && mGraphics->IsResourceContextSupported())
  {
    auto* context = GetSurfaceContext(surface);

    if(context)
    {
      mCurrentContext = context;
      mCurrentContext->GlContextCreated();
    }
  }
}

GLES::Context* EglGraphicsController::GetSurfaceContext(Dali::Integration::RenderSurfaceInterface* surface) const
{
  if(DALI_LIKELY(surface))
  {
    auto iter = std::find_if(mSurfaceContexts.begin(), mSurfaceContexts.end(), [surface](const SurfaceContextPair& iter) { return (iter.first == surface); });

    if(iter != mSurfaceContexts.end())
    {
      return iter->second.get();
    }
  }
  return nullptr;
}

void EglGraphicsController::AddTexture(GLES::Texture& texture)
{
  // Assuming we are on the correct context
  mCreateTextureQueue.push(&texture);
}

void EglGraphicsController::AddBuffer(GLES::Buffer& buffer)
{
  // Assuming we are on the correct context
  mCreateBufferQueue.push(&buffer);
}

void EglGraphicsController::AddFramebuffer(GLES::Framebuffer& framebuffer)
{
  // Assuming we are on the correct context
  mCreateFramebufferQueue.push(&framebuffer);
}

void EglGraphicsController::ProcessDiscardQueues()
{
  DALI_TRACE_SCOPE(gTraceFilter, "DALI_EGL_CONTROLLER_DISCARD_QUEUE");

  // Process textures
  ProcessDiscardSet<GLES::Texture>(mDiscardTextureSet);

  // Process buffers
  ProcessDiscardQueue<GLES::Buffer>(mDiscardBufferQueue);

  // Process Framebuffers
  ProcessDiscardQueue<GLES::Framebuffer>(mDiscardFramebufferQueue);

  // Process RenderPass
  ProcessDiscardQueue<GLES::RenderPass>(mDiscardRenderPassQueue);

  // Process RenderTarget
  ProcessDiscardQueue<GLES::RenderTarget>(mDiscardRenderTargetQueue);

  // Process pipelines
  if(mPipelineCache && !mDiscardPipelineQueue.empty())
  {
    mPipelineCache->MarkPipelineCacheFlushRequired();
  }
  ProcessDiscardQueue(mDiscardPipelineQueue);

  // Process programs
  if(mPipelineCache && !mDiscardProgramQueue.empty())
  {
    mPipelineCache->MarkProgramCacheFlushRequired();
  }
  ProcessDiscardQueue<GLES::Program>(mDiscardProgramQueue);

  // Process shaders
  ProcessDiscardQueue<GLES::Shader>(mDiscardShaderQueue);

  // Process samplers
  ProcessDiscardQueue<GLES::Sampler>(mDiscardSamplerQueue);

  // Process command buffers
  ProcessDiscardQueue<GLES::CommandBuffer>(mDiscardCommandBufferQueue);
}

void EglGraphicsController::ProcessCreateQueues()
{
  DALI_TRACE_SCOPE(gTraceFilter, "DALI_EGL_CONTROLLER_CREATE_QUEUE");
  // Process textures
  ProcessCreateQueue(mCreateTextureQueue);

  // Process buffers
  ProcessCreateQueue(mCreateBufferQueue);

  // Process framebuffers
  ProcessCreateQueue(mCreateFramebufferQueue);
}

void EglGraphicsController::ProcessCommandBuffer(const GLES::CommandBuffer& commandBuffer)
{
  auto       count    = 0u;
  const auto commands = commandBuffer.GetCommands(count);

  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CONTROLLER_PROCESS", [&](std::ostringstream& oss) { oss << "[commandCount:" << count << "]"; });

  for(auto i = 0u; i < count; ++i)
  {
    auto& cmd = commands[i];
    // process command
    switch(cmd.type)
    {
      case GLES::CommandType::FLUSH:
      {
        // Nothing to do here
        break;
      }
      case GLES::CommandType::BIND_TEXTURES:
      {
        mCurrentContext->BindTextures(cmd.bindTextures.textureBindings.Ptr(), cmd.bindTextures.textureBindingsCount);
        break;
      }
      case GLES::CommandType::BIND_VERTEX_BUFFERS:
      {
        auto bindings = cmd.bindVertexBuffers.vertexBufferBindings.Ptr();
        mCurrentContext->BindVertexBuffers(bindings, cmd.bindVertexBuffers.vertexBufferBindingsCount);
        break;
      }
      case GLES::CommandType::BIND_UNIFORM_BUFFER:
      {
        auto& bindings = cmd.bindUniformBuffers;
        mCurrentContext->BindUniformBuffers(bindings.uniformBufferBindingsCount ? bindings.uniformBufferBindings.Ptr() : nullptr, bindings.uniformBufferBindingsCount, bindings.standaloneUniformsBufferBinding);
        break;
      }
      case GLES::CommandType::BIND_INDEX_BUFFER:
      {
        mCurrentContext->BindIndexBuffer(cmd.bindIndexBuffer);
        break;
      }
      case GLES::CommandType::BIND_SAMPLERS:
      {
        break;
      }
      case GLES::CommandType::BIND_PIPELINE:
      {
        auto pipeline = static_cast<const GLES::Pipeline*>(cmd.bindPipeline.pipeline);
        mCurrentContext->BindPipeline(pipeline);
        break;
      }
      case GLES::CommandType::DRAW:
      {
        mCurrentContext->Flush(false, cmd.draw, mTextureDependencyChecker);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED:
      {
        mCurrentContext->Flush(false, cmd.draw, mTextureDependencyChecker);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED_INDIRECT:
      {
        mCurrentContext->Flush(false, cmd.draw, mTextureDependencyChecker);
        break;
      }
      case GLES::CommandType::SET_SCISSOR: // @todo Consider correcting for orientation here?
      {
        mGlAbstraction->Scissor(cmd.scissor.region.x, cmd.scissor.region.y, cmd.scissor.region.width, cmd.scissor.region.height);
        break;
      }
      case GLES::CommandType::SET_SCISSOR_TEST:
      {
        mCurrentContext->SetScissorTestEnabled(cmd.scissorTest.enable);
        break;
      }
      case GLES::CommandType::SET_VIEWPORT: // @todo Consider correcting for orientation here?
      {
        mGlAbstraction->Viewport(cmd.viewport.region.x, cmd.viewport.region.y, cmd.viewport.region.width, cmd.viewport.region.height);
        break;
      }

      case GLES::CommandType::SET_COLOR_MASK:
      {
        mCurrentContext->ColorMask(cmd.colorMask.enabled);
        break;
      }
      case GLES::CommandType::CLEAR_STENCIL_BUFFER:
      {
        mCurrentContext->ClearStencilBuffer();
        break;
      }
      case GLES::CommandType::CLEAR_DEPTH_BUFFER:
      {
        mCurrentContext->ClearDepthBuffer();
        break;
      }

      case GLES::CommandType::SET_STENCIL_TEST_ENABLE:
      {
        mCurrentContext->SetStencilTestEnable(cmd.stencilTest.enabled);
        break;
      }

      case GLES::CommandType::SET_STENCIL_STATE:
      {
        mCurrentContext->StencilFunc(cmd.stencilState.compareOp,
                                     cmd.stencilState.reference,
                                     cmd.stencilState.compareMask);
        mCurrentContext->StencilOp(cmd.stencilState.failOp,
                                   cmd.stencilState.depthFailOp,
                                   cmd.stencilState.passOp);
        break;
      }

      case GLES::CommandType::SET_STENCIL_WRITE_MASK:
      {
        mCurrentContext->StencilMask(cmd.stencilWriteMask.mask);
        break;
      }

      case GLES::CommandType::SET_DEPTH_COMPARE_OP:
      {
        mCurrentContext->SetDepthCompareOp(cmd.depth.compareOp);
        break;
      }
      case GLES::CommandType::SET_DEPTH_TEST_ENABLE:
      {
        mCurrentContext->SetDepthTestEnable(cmd.depth.testEnabled);
        break;
      }
      case GLES::CommandType::SET_DEPTH_WRITE_ENABLE:
      {
        mCurrentContext->SetDepthWriteEnable(cmd.depth.writeEnabled);
        break;
      }

      case GLES::CommandType::BEGIN_RENDERPASS:
      {
        auto&       renderTarget = *cmd.beginRenderPass.renderTarget;
        const auto& targetInfo   = renderTarget.GetCreateInfo();

        if(targetInfo.surface)
        {
          // switch to surface context
          mGraphics->ActivateSurfaceContext(static_cast<Dali::Integration::RenderSurfaceInterface*>(targetInfo.surface));
        }
        else if(targetInfo.framebuffer)
        {
          // switch to resource context
          mGraphics->ActivateResourceContext();
        }

        mCurrentContext->BeginRenderPass(cmd.beginRenderPass);

        break;
      }
      case GLES::CommandType::END_RENDERPASS:
      {
        mCurrentContext->EndRenderPass(mTextureDependencyChecker);

        // This sync object is to enable cpu to wait for rendering to complete, not gpu.
        // It's only needed for reading the framebuffer texture in the client.
        auto syncObject = const_cast<GLES::SyncObject*>(static_cast<const GLES::SyncObject*>(cmd.endRenderPass.syncObject));
        if(syncObject)
        {
          syncObject->InitializeResource();
        }
        break;
      }
      case GLES::CommandType::READ_PIXELS:
      {
        mCurrentContext->ReadPixels(cmd.readPixelsBuffer.buffer);
        break;
      }
      case GLES::CommandType::PRESENT_RENDER_TARGET:
      {
        ResolvePresentRenderTarget(cmd.presentRenderTarget.targetToPresent);

        // The command buffer will be pushed into the queue of presentation command buffers
        // for further reuse.
        if(commandBuffer.GetCreateInfo().fixedCapacity == 1)
        {
          mPresentationCommandBuffers.push(&commandBuffer);
        }
        break;
      }
      case GLES::CommandType::EXECUTE_COMMAND_BUFFERS:
      {
        // Process secondary command buffers
        // todo: check validity of the secondaries
        //       there are operations which are illigal to be done
        //       within secondaries.
        auto buffers = cmd.executeCommandBuffers.buffers.Ptr();
        for(auto j = 0u; j < cmd.executeCommandBuffers.buffersCount; ++j)
        {
          auto& buf = buffers[j];
          ProcessCommandBuffer(*static_cast<const GLES::CommandBuffer*>(buf));
        }
        break;
      }
      case GLES::CommandType::DRAW_NATIVE:
      {
        auto* info = &cmd.drawNative.drawNativeInfo;

        // ISOLATED execution mode will isolate GL graphics context from
        // DALi renderning pipeline which is the safest way of rendering
        // the 'injected' code.
        if(info->executionMode == DrawNativeExecutionMode::ISOLATED)
        {
          mCurrentContext->PrepareForNativeRendering();
        }

        if(info->glesNativeInfo.eglSharedContextStoragePointer)
        {
          auto* anyContext = reinterpret_cast<std::any*>(info->glesNativeInfo.eglSharedContextStoragePointer);
          *anyContext      = mSharedContext;
        }

        CallbackBase::ExecuteReturn<bool>(*info->callback, info->userData);
        if(info->executionMode == DrawNativeExecutionMode::ISOLATED)
        {
          mCurrentContext->RestoreFromNativeRendering();
        }
        else
        {
          // After native rendering reset all states and caches.
          // This is going to be called only when DIRECT execution mode is used
          // and some GL states need to be reset.
          // This does not guarantee that after execution a custom GL code
          // the main rendering pipeline will work correctly and it's a responsibility
          // of developer to make sure the GL states are not interfering with main
          // rendering pipeline (by restoring/cleaning up GL states after drawing).
          mCurrentContext->ResetGLESState();
        }
        break;
      }
    }
  }
  DALI_TRACE_END(gTraceFilter, "DALI_EGL_CONTROLLER_PROCESS");
}

void EglGraphicsController::ProcessCommandQueues()
{
  DUMP_FRAME_START();

  while(!mCommandQueue.empty())
  {
    auto cmdBuf = mCommandQueue.front();
    mCommandQueue.pop();

    DUMP_FRAME_COMMAND_BUFFER(cmdBuf);
    ProcessCommandBuffer(*cmdBuf);
  }

  DUMP_FRAME_END();
}

void EglGraphicsController::ProcessTextureUpdateQueue()
{
  if(mTextureUpdateRequests.empty())
  {
    return;
  }
  DALI_TRACE_SCOPE(gTraceFilter, "DALI_EGL_CONTROLLER_TEXTURE_UPDATE");
  while(!mTextureUpdateRequests.empty())
  {
    TextureUpdateRequest& request = mTextureUpdateRequests.front();

    auto& info   = request.first;
    auto& source = request.second;

    switch(source.sourceType)
    {
      case Graphics::TextureUpdateSourceInfo::Type::MEMORY:
      case Graphics::TextureUpdateSourceInfo::Type::PIXEL_DATA:
      {
        // GPU memory must be already allocated.

        // Check if it needs conversion
        auto*       texture            = static_cast<GLES::Texture*>(info.dstTexture);
        const auto& createInfo         = texture->GetCreateInfo();
        auto        srcFormat          = GLES::GLTextureFormatType(info.srcFormat).format;
        auto        srcType            = GLES::GLTextureFormatType(info.srcFormat).type;
        auto        destInternalFormat = GLES::GLTextureFormatType(createInfo.format).internalFormat;
        auto        destFormat         = GLES::GLTextureFormatType(createInfo.format).format;

        // From render-texture.cpp
        const bool isSubImage(info.dstOffset2D.x != 0 || info.dstOffset2D.y != 0 ||
                              info.srcExtent2D.width != (createInfo.size.width / (1 << info.level)) ||
                              info.srcExtent2D.height != (createInfo.size.height / (1 << info.level)));

        uint8_t* sourceBuffer                = nullptr;
        bool     sourceBufferReleaseRequired = false;
        if(source.sourceType == Graphics::TextureUpdateSourceInfo::Type::MEMORY)
        {
          sourceBuffer                = reinterpret_cast<uint8_t*>(source.memorySource.memory);
          sourceBufferReleaseRequired = true;
        }
        else
        {
          Dali::Integration::PixelDataBuffer pixelBufferData = Dali::Integration::GetPixelDataBuffer(source.pixelDataSource.pixelData);

          sourceBuffer                = pixelBufferData.buffer + info.srcOffset;
          sourceBufferReleaseRequired = Dali::Integration::IsPixelDataReleaseAfterUpload(source.pixelDataSource.pixelData) && info.srcOffset == 0u;
        }

        // Skip texture upload if given texture is already discarded for this render loop.
        if(mDiscardTextureSet.find(texture) == mDiscardTextureSet.end())
        {
          auto                 sourceStride = info.srcStride;
          std::vector<uint8_t> tempBuffer;

          uint8_t* srcBuffer = sourceBuffer;

          if(mGlAbstraction->TextureRequiresConverting(srcFormat, destFormat, isSubImage))
          {
            // Convert RGB to RGBA if necessary.
            if(texture->TryConvertPixelData(sourceBuffer, info.srcFormat, createInfo.format, info.srcSize, info.srcStride, info.srcExtent2D.width, info.srcExtent2D.height, tempBuffer))
            {
              srcBuffer    = &tempBuffer[0];
              sourceStride = 0u; // Converted buffer compacted. make stride as 0.
              srcFormat    = destFormat;
              srcType      = GLES::GLTextureFormatType(createInfo.format).type;
            }
          }

          // Calculate the maximum mipmap level for the texture
          texture->SetMaxMipMapLevel(std::max(texture->GetMaxMipMapLevel(), info.level));

          GLenum bindTarget{GL_TEXTURE_2D};
          GLenum target{GL_TEXTURE_2D};

          if(createInfo.textureType == Graphics::TextureType::TEXTURE_CUBEMAP)
          {
            bindTarget = GL_TEXTURE_CUBE_MAP;
            target     = GL_TEXTURE_CUBE_MAP_POSITIVE_X + info.layer;
          }

          mGlAbstraction->PixelStorei(GL_UNPACK_ALIGNMENT, 1);
          mGlAbstraction->PixelStorei(GL_UNPACK_ROW_LENGTH, sourceStride);

          mCurrentContext->BindTexture(bindTarget, texture->GetTextureTypeId(), texture->GetGLTexture());

          if(!isSubImage)
          {
            if(!texture->IsCompressed())
            {
              mGlAbstraction->TexImage2D(target,
                                         info.level,
                                         destInternalFormat,
                                         info.srcExtent2D.width,
                                         info.srcExtent2D.height,
                                         0,
                                         srcFormat,
                                         srcType,
                                         srcBuffer);
            }
            else
            {
              mGlAbstraction->CompressedTexImage2D(target,
                                                   info.level,
                                                   destInternalFormat,
                                                   info.srcExtent2D.width,
                                                   info.srcExtent2D.height,
                                                   0,
                                                   info.srcSize,
                                                   srcBuffer);
            }
          }
          else
          {
            if(!texture->IsCompressed())
            {
              mGlAbstraction->TexSubImage2D(target,
                                            info.level,
                                            info.dstOffset2D.x,
                                            info.dstOffset2D.y,
                                            info.srcExtent2D.width,
                                            info.srcExtent2D.height,
                                            srcFormat,
                                            srcType,
                                            srcBuffer);
            }
            else
            {
              mGlAbstraction->CompressedTexSubImage2D(target,
                                                      info.level,
                                                      info.dstOffset2D.x,
                                                      info.dstOffset2D.y,
                                                      info.srcExtent2D.width,
                                                      info.srcExtent2D.height,
                                                      srcFormat,
                                                      info.srcSize,
                                                      srcBuffer);
            }
          }
        }

        if(sourceBufferReleaseRequired && sourceBuffer != nullptr)
        {
          if(source.sourceType == Graphics::TextureUpdateSourceInfo::Type::MEMORY)
          {
            free(reinterpret_cast<void*>(sourceBuffer));
          }
          else
          {
            Dali::Integration::ReleasePixelDataBuffer(source.pixelDataSource.pixelData);
          }
        }
        break;
      }
      default:
      {
        // TODO: other sources
        break;
      }
    }

    mTextureUpdateRequests.pop();
  }
}

void EglGraphicsController::UpdateTextures(const std::vector<TextureUpdateInfo>&       updateInfoList,
                                           const std::vector<TextureUpdateSourceInfo>& sourceList)
{
  // Store updates
  for(auto& info : updateInfoList)
  {
    mTextureUpdateRequests.push(std::make_pair(info, sourceList[info.srcReference]));
    auto& pair = mTextureUpdateRequests.back();
    switch(pair.second.sourceType)
    {
      case Graphics::TextureUpdateSourceInfo::Type::MEMORY:
      {
        auto& info   = pair.first;
        auto& source = pair.second;

        // allocate staging memory and copy the data
        // TODO: using PBO with GLES3, this is just naive
        // oldschool way

        uint8_t* stagingBuffer = reinterpret_cast<uint8_t*>(malloc(info.srcSize));

        if(DALI_UNLIKELY(stagingBuffer == nullptr))
        {
          DALI_LOG_ERROR("malloc is failed. request malloc size : %u\n", info.srcSize);
        }
        else
        {
          uint8_t* srcMemory = &reinterpret_cast<uint8_t*>(source.memorySource.memory)[info.srcOffset];

          std::copy(srcMemory, srcMemory + info.srcSize, stagingBuffer);

          mTextureUploadTotalCPUMemoryUsed += info.srcSize;
        }

        // store staging buffer
        source.memorySource.memory = stagingBuffer;
        break;
      }
      case Graphics::TextureUpdateSourceInfo::Type::PIXEL_DATA:
      {
        // Increase CPU memory usage since ownership of PixelData is now on mTextureUpdateRequests.
        mTextureUploadTotalCPUMemoryUsed += info.srcSize;
        break;
      }
      case Graphics::TextureUpdateSourceInfo::Type::BUFFER:
      {
        // TODO, with PBO support
        break;
      }
      case Graphics::TextureUpdateSourceInfo::Type::TEXTURE:
      {
        // TODO texture 2 texture in-GPU copy
        break;
      }
    }
  }

  // If upload buffer exceeds maximum size, flush.
  if(mTextureUploadTotalCPUMemoryUsed > TEXTURE_UPLOAD_MAX_BUFER_SIZE_MB * 1024 * 1024)
  {
    Flush();
    mTextureUploadTotalCPUMemoryUsed = 0;
  }
}

void EglGraphicsController::ProcessTextureMipmapGenerationQueue()
{
  if(mTextureMipmapGenerationRequests.empty())
  {
    return;
  }
  DALI_TRACE_SCOPE(gTraceFilter, "DALI_EGL_CONTROLLER_TEXTURE_MIPMAP");
  while(!mTextureMipmapGenerationRequests.empty())
  {
    auto* texture = mTextureMipmapGenerationRequests.front();

    mCurrentContext->BindTexture(texture->GetGlTarget(), texture->GetTextureTypeId(), texture->GetGLTexture());
    mCurrentContext->GenerateMipmap(texture->GetGlTarget());

    mTextureMipmapGenerationRequests.pop();
  }
}

void EglGraphicsController::GenerateTextureMipmaps(const Graphics::Texture& texture)
{
  mTextureMipmapGenerationRequests.push(static_cast<const GLES::Texture*>(&texture));
}

Graphics::UniquePtr<Memory> EglGraphicsController::MapBufferRange(const MapBufferInfo& mapInfo)
{
  // Mapping buffer requires the object to be created NOW
  // Workaround - flush now, otherwise there will be given a staging buffer
  // in case when the buffer is not there yet
  if(!mCreateBufferQueue.empty())
  {
    mGraphics->ActivateResourceContext();
    ProcessCreateQueues();
  }

  if(GetGLESVersion() < GLES::GLESVersion::GLES_30)
  {
    return Graphics::UniquePtr<Memory>(new GLES::Memory2(mapInfo, *this));
  }
  else
  {
    return Graphics::UniquePtr<Memory>(new GLES::Memory3(mapInfo, *this));
  }
}

bool EglGraphicsController::GetProgramParameter(Graphics::Program& program, uint32_t parameterId, void* outData)
{
  return static_cast<GLES::Program*>(&program)->GetImplementation()->GetParameter(parameterId, outData);
}

GLES::PipelineCache& EglGraphicsController::GetPipelineCache() const
{
  return *mPipelineCache;
}

Graphics::Texture* EglGraphicsController::CreateTextureByResourceId(uint32_t resourceId, const Graphics::TextureCreateInfo& createInfo)
{
  Graphics::Texture*                     ret = nullptr;
  Graphics::UniquePtr<Graphics::Texture> texture;

  auto iter = mExternalTextureResources.find(resourceId);
  DALI_ASSERT_ALWAYS(iter == mExternalTextureResources.end());

  texture = CreateTexture(createInfo, std::move(texture));

  ret = texture.get();

  mExternalTextureResources.insert(std::make_pair(resourceId, std::move(texture)));

  return ret;
}

void EglGraphicsController::DiscardTextureFromResourceId(uint32_t resourceId)
{
  auto iter = mExternalTextureResources.find(resourceId);
  if(iter != mExternalTextureResources.end())
  {
    mExternalTextureResources.erase(iter);
  }
}

Graphics::Texture* EglGraphicsController::GetTextureFromResourceId(uint32_t resourceId)
{
  Graphics::Texture* ret = nullptr;

  auto iter = mExternalTextureResources.find(resourceId);
  if(iter != mExternalTextureResources.end())
  {
    ret = iter->second.get();
  }

  return ret;
}

Graphics::UniquePtr<Graphics::Texture> EglGraphicsController::ReleaseTextureFromResourceId(uint32_t resourceId)
{
  Graphics::UniquePtr<Graphics::Texture> texture;

  auto iter = mExternalTextureResources.find(resourceId);
  if(iter != mExternalTextureResources.end())
  {
    texture = std::move(iter->second);
    mExternalTextureResources.erase(iter);
  }

  return texture;
}

bool EglGraphicsController::HasClipMatrix() const
{
  return false;
}

const Matrix& EglGraphicsController::GetClipMatrix() const
{
  return Matrix::IDENTITY;
}

uint32_t EglGraphicsController::GetDeviceLimitation(Dali::Graphics::DeviceCapability capability)
{
  if(capability == DeviceCapability::MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT)
  {
    GLint i = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &i);
    return i;
  }
  return 0u;
}

} // namespace Dali::Graphics
