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
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-command-buffer.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-pipeline.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-shader.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-texture.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-types.h>
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

void EglGraphicsController::InitializeGLES(Integration::GlAbstraction& glAbstraction)
{
  DALI_LOG_RELEASE_INFO("Initializing New Graphics Controller #1\n");
  mGlAbstraction = &glAbstraction;
  mContext       = std::make_unique<GLES::Context>(*this);
}

void EglGraphicsController::Initialize(Integration::GlSyncAbstraction&          glSyncAbstraction,
                                       Integration::GlContextHelperAbstraction& glContextHelperAbstraction)
{
  DALI_LOG_RELEASE_INFO("Initializing New Graphics Controller #2\n");
  mGlSyncAbstraction          = &glSyncAbstraction;
  mGlContextHelperAbstraction = &glContextHelperAbstraction;
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

Graphics::UniquePtr<CommandBuffer>
EglGraphicsController::CreateCommandBuffer(const CommandBufferCreateInfo&       commandBufferCreateInfo,
                                           Graphics::UniquePtr<CommandBuffer>&& oldCommandBuffer)
{
  return NewObject<GLES::CommandBuffer>(commandBufferCreateInfo, *this, std::move(oldCommandBuffer));
}

Graphics::UniquePtr<Texture>
EglGraphicsController::CreateTexture(const TextureCreateInfo& textureCreateInfo, Graphics::UniquePtr<Texture>&& oldTexture)
{
  return NewObject<GLES::Texture>(textureCreateInfo, *this, std::move(oldTexture));
}

Graphics::UniquePtr<Buffer>
EglGraphicsController::CreateBuffer(const BufferCreateInfo& bufferCreateInfo, Graphics::UniquePtr<Buffer>&& oldBuffer)
{
  return NewObject<GLES::Buffer>(bufferCreateInfo, *this, std::move(oldBuffer));
}

Graphics::UniquePtr<Pipeline> EglGraphicsController::CreatePipeline(const PipelineCreateInfo& pipelineCreateInfo, Graphics::UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  // Create pipeline cache if needed
  if(!mPipelineCache)
  {
    mPipelineCache = std::make_unique<GLES::PipelineCache>(*this);
  }

  return mPipelineCache->GetPipeline(pipelineCreateInfo, std::move(oldPipeline));
}

Graphics::UniquePtr<Program> EglGraphicsController::CreateProgram(const ProgramCreateInfo& programCreateInfo, UniquePtr<Program>&& oldProgram)
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

void EglGraphicsController::ProcessDiscardQueues()
{
  // Process textures
  ProcessDiscardQueue<GLES::Texture>(mDiscardTextureQueue);

  // Process buffers
  ProcessDiscardQueue<GLES::Buffer>(mDiscardBufferQueue);

  // Process pipelines
  ProcessDiscardQueue<GLES::Pipeline>(mDiscardPipelineQueue);

  // Process programs
  ProcessDiscardQueue<GLES::Program>(mDiscardProgramQueue);
}

void EglGraphicsController::ProcessCreateQueues()
{
  // Process textures
  ProcessCreateQueue(mCreateTextureQueue);

  // Process buffers
  ProcessCreateQueue(mCreateBufferQueue);
}

void EglGraphicsController::ProcessCommandQueues()
{
  // TODO: command queue per context, sync between queues should be
  // done externally

  while(!mCommandQueue.empty())
  {
    auto cmdBuf = mCommandQueue.front();
    mCommandQueue.pop();

    for(auto& cmd : cmdBuf->GetCommands())
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
          mContext->BindPipeline(cmd.bindPipeline.pipeline);
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
      }
    }
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
  // Mapping buffer requires the object to be created NOW
  // Workaround - flush now, otherwise there will be given a staging buffer
  // in case when the buffer is not there yet
  ProcessCreateQueues();

  return Graphics::UniquePtr<Memory>(new GLES::Memory(mapInfo, *this));
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
