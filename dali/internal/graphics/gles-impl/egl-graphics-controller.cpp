/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <dali/integration-api/graphics-sync-abstraction.h>
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
#include <dali/internal/graphics/gles/egl-sync-implementation.h>
#include <dali/public-api/common/dali-common.h>

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
    // Discard resource (add it to discard queue)
    object->DiscardResource();
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

} // namespace

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
  DALI_LOG_RELEASE_INFO("Initializing New Graphics Controller #1\n");
  mGlAbstraction  = &glAbstraction;
  mContext        = std::make_unique<GLES::Context>(*this);
  mCurrentContext = mContext.get();
}

void EglGraphicsController::Initialize(Integration::GraphicsSyncAbstraction&    syncImplementation,
                                       Integration::GlContextHelperAbstraction& glContextHelperAbstraction,
                                       Internal::Adaptor::GraphicsInterface&    graphicsInterface)
{
  DALI_LOG_RELEASE_INFO("Initializing New Graphics Controller #2\n");
  auto* syncImplPtr = static_cast<Internal::Adaptor::EglSyncImplementation*>(&syncImplementation);

  mEglSyncImplementation      = syncImplPtr;
  mGlContextHelperAbstraction = &glContextHelperAbstraction;
  mGraphics                   = &graphicsInterface;
}

void EglGraphicsController::SubmitCommandBuffers(const SubmitInfo& submitInfo)
{
  for(auto& cmdbuf : submitInfo.cmdBuffer)
  {
    // Push command buffers
    mCommandQueue.push(static_cast<GLES::CommandBuffer*>(cmdbuf));
  }

  // If flush bit set, flush all pending tasks
  if(submitInfo.flags & (0 | SubmitFlagBits::FLUSH))
  {
    Flush();
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

  auto* rt = static_cast<GLES::RenderTarget*>(renderTarget);
  if(rt->GetCreateInfo().surface)
  {
    auto* surfaceInterface = reinterpret_cast<Dali::RenderSurfaceInterface*>(rt->GetCreateInfo().surface);
    surfaceInterface->MakeContextCurrent();
    surfaceInterface->PostRender();
  }
}

Integration::GlAbstraction& EglGraphicsController::GetGlAbstraction()
{
  DALI_ASSERT_DEBUG(mGlAbstraction && "Graphics controller not initialized");
  return *mGlAbstraction;
}

Integration::GlContextHelperAbstraction& EglGraphicsController::GetGlContextHelperAbstraction()
{
  DALI_ASSERT_DEBUG(mGlContextHelperAbstraction && "Graphics controller not initialized");
  return *mGlContextHelperAbstraction;
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
  // Create program cache if needed
  if(!mPipelineCache)
  {
    mPipelineCache = std::make_unique<GLES::PipelineCache>(*this);
  }

  return mPipelineCache->GetProgram(programCreateInfo, std::move(oldProgram));
}

Graphics::UniquePtr<Shader> EglGraphicsController::CreateShader(const ShaderCreateInfo& shaderCreateInfo, Graphics::UniquePtr<Shader>&& oldShader)
{
  return NewObject<GLES::Shader>(shaderCreateInfo, *this, std::move(oldShader));
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

const Graphics::Reflection& EglGraphicsController::GetProgramReflection(const Graphics::Program& program)
{
  return static_cast<const Graphics::GLES::Program*>(&program)->GetReflection();
}

void EglGraphicsController::CreateSurfaceContext(Dali::RenderSurfaceInterface* surface)
{
  std::unique_ptr<GLES::Context> context = std::make_unique<GLES::Context>(*this);
  mSurfaceContexts.push_back(std::move(std::make_pair(surface, std::move(context))));
}

void EglGraphicsController::DeleteSurfaceContext(Dali::RenderSurfaceInterface* surface)
{
  mSurfaceContexts.erase(std::remove_if(
                           mSurfaceContexts.begin(), mSurfaceContexts.end(), [surface](SurfaceContextPair& iter) { return surface == iter.first; }),
                         mSurfaceContexts.end());
}

void EglGraphicsController::ActivateResourceContext()
{
  mCurrentContext = mContext.get();
  mCurrentContext->GlContextCreated();
}

void EglGraphicsController::ActivateSurfaceContext(Dali::RenderSurfaceInterface* surface)
{
  if(surface && mGraphics->IsResourceContextSupported())
  {
    auto iter = std::find_if(mSurfaceContexts.begin(), mSurfaceContexts.end(), [surface](SurfaceContextPair& iter) {
      return (iter.first == surface);
    });

    if(iter != mSurfaceContexts.end())
    {
      mCurrentContext = iter->second.get();
      mCurrentContext->GlContextCreated();
    }
  }
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
  // Process textures
  ProcessDiscardQueue<GLES::Texture>(mDiscardTextureQueue);

  // Process buffers
  ProcessDiscardQueue<GLES::Buffer>(mDiscardBufferQueue);

  // Process Framebuffers
  ProcessDiscardQueue<GLES::Framebuffer>(mDiscardFramebufferQueue);

  // Process RenderPass
  ProcessDiscardQueue<GLES::RenderPass>(mDiscardRenderPassQueue);

  // Process RenderTarget
  ProcessDiscardQueue<GLES::RenderTarget>(mDiscardRenderTargetQueue);

  // Process pipelines
  ProcessDiscardQueue(mDiscardPipelineQueue);

  // Process programs
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
        mCurrentContext->Flush(false, cmd.draw);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED:
      {
        mCurrentContext->Flush(false, cmd.draw);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED_INDIRECT:
      {
        mCurrentContext->Flush(false, cmd.draw);
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

      case GLES::CommandType::SET_STENCIL_FUNC:
      {
        mCurrentContext->StencilFunc(cmd.stencilFunc.compareOp,
                                     cmd.stencilFunc.reference,
                                     cmd.stencilFunc.compareMask);
        break;
      }

      case GLES::CommandType::SET_STENCIL_WRITE_MASK:
      {
        mCurrentContext->StencilMask(cmd.stencilWriteMask.mask);
        break;
      }

      case GLES::CommandType::SET_STENCIL_OP:
      {
        mCurrentContext->StencilOp(cmd.stencilOp.failOp,
                                   cmd.stencilOp.depthFailOp,
                                   cmd.stencilOp.passOp);
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
          mGraphics->ActivateSurfaceContext(static_cast<Dali::RenderSurfaceInterface*>(targetInfo.surface));
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
        mCurrentContext->EndRenderPass();

        auto syncObject = const_cast<GLES::SyncObject*>(static_cast<const GLES::SyncObject*>(cmd.endRenderPass.syncObject));
        if(syncObject)
        {
          syncObject->InitializeResource();
        }
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

        mCurrentContext->PrepareForNativeRendering();

        CallbackBase::ExecuteReturn<bool>(*info->callback, info->userData);

        mCurrentContext->RestoreFromNativeRendering();
        break;
      }
    }
  }
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
  while(!mTextureUpdateRequests.empty())
  {
    TextureUpdateRequest& request = mTextureUpdateRequests.front();

    auto& info   = request.first;
    auto& source = request.second;

    if(source.sourceType == Graphics::TextureUpdateSourceInfo::Type::MEMORY)
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

      auto*                sourceBuffer = reinterpret_cast<uint8_t*>(source.memorySource.memory);
      std::vector<uint8_t> tempBuffer;
      if(mGlAbstraction->TextureRequiresConverting(srcFormat, destFormat, isSubImage))
      {
        // Convert RGB to RGBA if necessary.
        texture->TryConvertPixelData(source.memorySource.memory, info.srcFormat, createInfo.format, info.srcSize, info.srcExtent2D.width, info.srcExtent2D.height, tempBuffer);
        sourceBuffer = &tempBuffer[0];
        srcFormat    = destFormat;
        srcType      = GLES::GLTextureFormatType(createInfo.format).type;
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
                                     sourceBuffer);
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
                                               sourceBuffer);
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
                                        sourceBuffer);
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
                                                  sourceBuffer);
        }
      }
      // free staging memory
      free(source.memorySource.memory);
    }
    else
    {
      // TODO: other sources
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

        char* stagingBuffer = reinterpret_cast<char*>(malloc(info.srcSize));
        std::copy(&reinterpret_cast<char*>(source.memorySource.memory)[info.srcOffset],
                  reinterpret_cast<char*>(source.memorySource.memory) + info.srcSize,
                  stagingBuffer);

        mTextureUploadTotalCPUMemoryUsed += info.srcSize;

        // store staging buffer
        source.memorySource.memory = stagingBuffer;
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
  if(mTextureUploadTotalCPUMemoryUsed > TEXTURE_UPLOAD_MAX_BUFER_SIZE_MB * 1024)
  {
    Flush();
    mTextureUploadTotalCPUMemoryUsed = 0;
  }
}

void EglGraphicsController::ProcessTextureMipmapGenerationQueue()
{
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
  mGraphics->ActivateResourceContext();

  // Mapping buffer requires the object to be created NOW
  // Workaround - flush now, otherwise there will be given a staging buffer
  // in case when the buffer is not there yet
  ProcessCreateQueues();

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

} // namespace Dali::Graphics
