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

// CLASS HEADER
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-command-buffer.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-pipeline.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-render-pass.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-render-target.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-shader.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-texture.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-types.h>
#include <dali/internal/graphics/gles-impl/gles3-graphics-memory.h>
#include <dali/public-api/common/dali-common.h>
#include "gles-graphics-program.h"

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

} // namespace

EglGraphicsController::~EglGraphicsController() = default;

void EglGraphicsController::InitializeGLES(Integration::GlAbstraction& glAbstraction)
{
  DALI_LOG_RELEASE_INFO("Initializing New Graphics Controller #1\n");
  mGlAbstraction = &glAbstraction;
  mContext       = std::make_unique<GLES::Context>(*this);
}

void EglGraphicsController::Initialize(Integration::GlSyncAbstraction&          glSyncAbstraction,
                                       Integration::GlContextHelperAbstraction& glContextHelperAbstraction,
                                       Internal::Adaptor::GraphicsInterface&    graphicsInterface)
{
  DALI_LOG_RELEASE_INFO("Initializing New Graphics Controller #2\n");
  mGlSyncAbstraction          = &glSyncAbstraction;
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
  // Use command buffer to execute presentation (we should pool it)
  CommandBufferCreateInfo info;
  info.SetLevel(CommandBufferLevel::PRIMARY);
  info.fixedCapacity        = 1; // only one command
  auto presentCommandBuffer = new GLES::CommandBuffer(info, *this);
  presentCommandBuffer->PresentRenderTarget(static_cast<GLES::RenderTarget*>(renderTarget));
  SubmitInfo submitInfo;
  submitInfo.cmdBuffer = {presentCommandBuffer};
  submitInfo.flags     = 0 | SubmitFlagBits::FLUSH;
  SubmitCommandBuffers(submitInfo);
}

void EglGraphicsController::ResolvePresentRenderTarget(GLES::RenderTarget* renderTarget)
{
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

Integration::GlSyncAbstraction& EglGraphicsController::GetGlSyncAbstraction()
{
  DALI_ASSERT_DEBUG(mGlSyncAbstraction && "Graphics controller not initialized");
  return *mGlSyncAbstraction;
}

Integration::GlContextHelperAbstraction& EglGraphicsController::GetGlContextHelperAbstraction()
{
  DALI_ASSERT_DEBUG(mGlContextHelperAbstraction && "Graphics controller not initialized");
  return *mGlContextHelperAbstraction;
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

const Graphics::Reflection& EglGraphicsController::GetProgramReflection(const Graphics::Program& program)
{
  return static_cast<const Graphics::GLES::Program*>(&program)->GetReflection();
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

  // Process pipelines
  ProcessDiscardQueue<GLES::Pipeline>(mDiscardPipelineQueue);

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

void EglGraphicsController::ProcessCommandBuffer(GLES::CommandBuffer& commandBuffer)
{
  for(auto& cmd : commandBuffer.GetCommands())
  {
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
        mContext->BindTextures(cmd.bindTextures.textureBindings);
        break;
      }
      case GLES::CommandType::BIND_VERTEX_BUFFERS:
      {
        auto& bindings = cmd.bindVertexBuffers.vertexBufferBindings;
        mContext->BindVertexBuffers(bindings);
        break;
      }
      case GLES::CommandType::BIND_UNIFORM_BUFFER:
      {
        auto& bindings = cmd.bindUniformBuffers;
        mContext->BindUniformBuffers(bindings.uniformBufferBindings, bindings.standaloneUniformsBufferBinding);
        break;
      }
      case GLES::CommandType::BIND_INDEX_BUFFER:
      {
        mContext->BindIndexBuffer(cmd.bindIndexBuffer);
        break;
      }
      case GLES::CommandType::BIND_SAMPLERS:
      {
        break;
      }
      case GLES::CommandType::BIND_PIPELINE:
      {
        auto pipeline = static_cast<const GLES::Pipeline*>(cmd.bindPipeline.pipeline);
        mContext->BindPipeline(pipeline);
        break;
      }
      case GLES::CommandType::DRAW:
      {
        mContext->Flush(false, cmd.draw);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED:
      {
        mContext->Flush(false, cmd.draw);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED_INDIRECT:
      {
        mContext->Flush(false, cmd.draw);
        break;
      }
      case GLES::CommandType::SET_SCISSOR: // @todo Consider correcting for orientation here?
      {
        mGlAbstraction->Scissor(cmd.scissor.region.x, cmd.scissor.region.y, cmd.scissor.region.width, cmd.scissor.region.height);
        break;
      }
      case GLES::CommandType::SET_SCISSOR_TEST:
      {
        if(cmd.scissorTest.enable)
        {
          mGlAbstraction->Enable(GL_SCISSOR_TEST);
        }
        else
        {
          mGlAbstraction->Disable(GL_SCISSOR_TEST);
        }
        break;
      }
      case GLES::CommandType::SET_VIEWPORT: // @todo Consider correcting for orientation here?
      {
        mGlAbstraction->Viewport(cmd.viewport.region.x, cmd.viewport.region.y, cmd.viewport.region.width, cmd.viewport.region.height);
        break;
      }
      case GLES::CommandType::BEGIN_RENDERPASS:
      {
        mContext->BeginRenderPass(cmd.beginRenderPass);
        break;
      }
      case GLES::CommandType::END_RENDERPASS:
      {
        mContext->EndRenderPass();
        break;
      }
      case GLES::CommandType::PRESENT_RENDER_TARGET:
      {
        ResolvePresentRenderTarget(cmd.presentRenderTarget.targetToPresent);

        // push this command buffer to the discard queue
        mDiscardCommandBufferQueue.push(&commandBuffer);
        break;
      }
      case GLES::CommandType::EXECUTE_COMMAND_BUFFERS:
      {
        // Process secondary command buffers
        // todo: check validity of the secondaries
        //       there are operations which are illigal to be done
        //       within secondaries.
        for(auto& buf : cmd.executeCommandBuffers.buffers)
        {
          ProcessCommandBuffer(*static_cast<GLES::CommandBuffer*>(buf));
        }
        break;
      }
    }
  }
}

void EglGraphicsController::ProcessCommandQueues()
{
  // TODO: command queue per context, sync between queues should be
  // done externally
  currentFramebuffer = nullptr;

  while(!mCommandQueue.empty())
  {
    auto cmdBuf = mCommandQueue.front();
    mCommandQueue.pop();
    ProcessCommandBuffer(*cmdBuf);
  }
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
      // GPU memory must be already allocated (glTexImage2D())
      auto*       texture    = static_cast<GLES::Texture*>(info.dstTexture);
      const auto& createInfo = texture->GetCreateInfo();

      mGlAbstraction->PixelStorei(GL_UNPACK_ALIGNMENT, 1);

      mGlAbstraction->BindTexture(GL_TEXTURE_2D, texture->GetGLTexture());
      mGlAbstraction->TexSubImage2D(GL_TEXTURE_2D,
                                    info.level,
                                    info.dstOffset2D.x,
                                    info.dstOffset2D.y,
                                    info.srcExtent2D.width,
                                    info.srcExtent2D.height,
                                    GLES::GLTextureFormatType(createInfo.format).format,
                                    GLES::GLTextureFormatType(createInfo.format).type,
                                    source.memorySource.memory);

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
