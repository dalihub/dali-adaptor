/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

#include "gles-context.h"
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/public-api/math/math-utils.h>

#include "egl-graphics-controller.h"
#include "gles-graphics-buffer.h"
#include "gles-graphics-pipeline.h"
#include "gles-graphics-program.h"
#include "gles-graphics-render-pass.h"
#include "gles-graphics-render-target.h"
#include "gles-texture-dependency-checker.h"

#include <dali/internal/graphics/common/egl-include.h>

#include <map>
#include <unordered_map>

namespace Dali::Graphics::GLES
{
struct Context::Impl
{
  explicit Impl(EglGraphicsController& controller, Integration::GlAbstraction* gl)
  : mController(controller),
    mGL(gl)
  {
  }

  ~Impl() = default;

  /**
   * Binds (and creates) VAO
   *
   * VAO is fixed per program so it has to be created only once assuming
   * that VertexInputState has been set correctly for the pipeline.
   *
   */
  void BindProgramVAO(const GLES::ProgramImpl* program, const VertexInputState& vertexInputState)
  {
    // Calculate attributes location hash unordered.
    std::size_t hash = 0;
    for(const auto& attr : vertexInputState.attributes)
    {
      // Make unordered hash value by location.
      // Note : This hash function varified for locations only under < 20.
      std::size_t salt = attr.location + 1;
      hash += salt << (sizeof(std::size_t) * 6);
      salt *= salt;
      salt ^= attr.location;
      hash += salt << (sizeof(std::size_t) * 4);
      salt *= salt;
      hash += salt;
    }

    auto* gl = GetGL();
    if(!gl) // early out if no gl
    {
      return;
    }

    if(DALI_UNLIKELY(!mDiscardedVAOList.empty()))
    {
      gl->DeleteVertexArrays(static_cast<Dali::GLsizei>(mDiscardedVAOList.size()), mDiscardedVAOList.data());
      mDiscardedVAOList.clear();
    }

    auto iter = mProgramVAOMap.find(program);
    if(iter != mProgramVAOMap.end())
    {
      auto attributeIter = iter->second.find(hash);
      if(attributeIter != iter->second.end())
      {
        if(mProgramVAOCurrentState != attributeIter->second)
        {
          mProgramVAOCurrentState = attributeIter->second;
          gl->BindVertexArray(attributeIter->second);

          // Binding VAO seems to reset the index buffer binding so the cache must be reset
          mGlStateCache.mBoundElementArrayBufferId = 0;
        }
        return;
      }
    }

    uint32_t vao;
    gl->GenVertexArrays(1, &vao);
    gl->BindVertexArray(vao);

    // Binding VAO seems to reset the index buffer binding so the cache must be reset
    mGlStateCache.mBoundElementArrayBufferId = 0;

    mProgramVAOMap[program][hash] = vao;
    for(const auto& attr : vertexInputState.attributes)
    {
      gl->EnableVertexAttribArray(attr.location);
    }

    mProgramVAOCurrentState = vao;
  }

  /**
   * Sets the initial GL state.
   */
  void InitializeGlState()
  {
    auto* gl = GetGL();
    if(gl)
    {
      mGlStateCache.mClearColorSet        = false;
      mGlStateCache.mColorMask            = true;
      mGlStateCache.mStencilMask          = 0xFF;
      mGlStateCache.mBlendEnabled         = false;
      mGlStateCache.mDepthBufferEnabled   = false;
      mGlStateCache.mDepthMaskEnabled     = false;
      mGlStateCache.mScissorTestEnabled   = false;
      mGlStateCache.mStencilBufferEnabled = false;

      gl->Disable(GL_DITHER);

      mGlStateCache.mBoundArrayBufferId        = 0;
      mGlStateCache.mBoundElementArrayBufferId = 0;
      mGlStateCache.mActiveTextureUnit         = 0;

      mGlStateCache.mBlendFuncSeparateSrcRGB   = BlendFactor::ONE;
      mGlStateCache.mBlendFuncSeparateDstRGB   = BlendFactor::ZERO;
      mGlStateCache.mBlendFuncSeparateSrcAlpha = BlendFactor::ONE;
      mGlStateCache.mBlendFuncSeparateDstAlpha = BlendFactor::ZERO;

      // initial state is GL_FUNC_ADD for both RGB and Alpha blend modes
      mGlStateCache.mBlendEquationSeparateModeRGB   = BlendOp::ADD;
      mGlStateCache.mBlendEquationSeparateModeAlpha = BlendOp::ADD;

      mGlStateCache.mCullFaceMode = CullMode::NONE; // By default cullface is disabled, front face is set to CCW and cull face is set to back

      // Initialze vertex attribute cache
      memset(&mGlStateCache.mVertexAttributeCachedState, 0, sizeof(mGlStateCache.mVertexAttributeCachedState));
      memset(&mGlStateCache.mVertexAttributeCurrentState, 0, sizeof(mGlStateCache.mVertexAttributeCurrentState));

      // Initialize bound 2d texture cache
      memset(&mGlStateCache.mBoundTextureId, 0, sizeof(mGlStateCache.mBoundTextureId));

      mGlStateCache.mFrameBufferStateCache.Reset();

      GLint maxTextures;
      gl->GetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextures);
      DALI_LOG_RELEASE_INFO("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: %d\n", maxTextures);
    }
  }

  /**
   * Flushes vertex attribute location changes to the driver
   */
  void FlushVertexAttributeLocations()
  {
    auto* gl = GetGL();
    if(gl)
    {
      for(unsigned int i = 0; i < MAX_ATTRIBUTE_CACHE_SIZE; ++i)
      {
        // see if the cached state is different to the actual state
        if(mGlStateCache.mVertexAttributeCurrentState[i] != mGlStateCache.mVertexAttributeCachedState[i])
        {
          // it's different so make the change to the driver and update the cached state
          mGlStateCache.mVertexAttributeCurrentState[i] = mGlStateCache.mVertexAttributeCachedState[i];

          if(mGlStateCache.mVertexAttributeCurrentState[i])
          {
            gl->EnableVertexAttribArray(i);
          }
          else
          {
            gl->DisableVertexAttribArray(i);
          }
        }
      }
    }
  }

  /**
   * Either enables or disables a vertex attribute location in the cache
   * The changes won't take affect until FlushVertexAttributeLocations is called
   * @param location attribute location
   * @param state attribute state
   */
  void SetVertexAttributeLocation(unsigned int location, bool state)
  {
    auto* gl = GetGL();
    if(gl)
    {
      if(location >= MAX_ATTRIBUTE_CACHE_SIZE)
      {
        // not cached, make the gl call through context
        if(state)
        {
          gl->EnableVertexAttribArray(location);
        }
        else
        {
          gl->DisableVertexAttribArray(location);
        }
      }
      else
      {
        // set the cached state, it will be set at the next draw call
        // if it's different from the current driver state
        mGlStateCache.mVertexAttributeCachedState[location] = state;
      }
    }
  }

  /**
   * Get the pointer to the GL implementation
   * @return The GL implementation, nullptr if the context has not been created or shutting down
   */
  [[nodiscard]] inline Integration::GlAbstraction* GetGL() const
  {
    return mGlContextCreated ? mGL : nullptr;
  }

  EglGraphicsController&      mController;
  Integration::GlAbstraction* mGL{nullptr};

  const GLES::PipelineImpl* mCurrentPipeline{nullptr}; ///< Currently bound pipeline
  const GLES::PipelineImpl* mNewPipeline{nullptr};     ///< New pipeline to be set on flush

  std::vector<Graphics::TextureBinding> mCurrentTextureBindings{};
  std::vector<Graphics::SamplerBinding> mCurrentSamplerBindings{};
  GLES::IndexBufferBindingDescriptor    mCurrentIndexBufferBinding{};

  struct VertexBufferBinding
  {
    GLES::Buffer* buffer{nullptr};
    uint32_t      offset{0u};
  };

  // Currently bound buffers
  std::vector<VertexBufferBindingDescriptor> mCurrentVertexBufferBindings{};

  // Currently bound UBOs (check if it's needed per program!)
  std::vector<UniformBufferBindingDescriptor> mCurrentUBOBindings{};
  UniformBufferBindingDescriptor              mCurrentStandaloneUBOBinding{};

  // Current render pass and render target
  const GLES::RenderTarget* mCurrentRenderTarget{nullptr};
  const GLES::RenderPass*   mCurrentRenderPass{nullptr};

  // Each context must have own VAOs as they cannot be shared
  std::unordered_map<const GLES::ProgramImpl*, std::map<std::size_t, uint32_t>> mProgramVAOMap;              ///< GL program-VAO map
  uint32_t                                                                      mProgramVAOCurrentState{0u}; ///< Currently bound VAO
  GLStateCache                                                                  mGlStateCache{};             ///< GL status cache
  std::vector<Dali::GLuint>                                                     mDiscardedVAOList{};

  bool mGlContextCreated{false};    ///< True if the OpenGL context has been created
  bool mVertexBuffersChanged{true}; ///< True if BindVertexBuffers changed any buffer bindings

  EGLContext mNativeDrawContext{0u}; ///< Native rendering EGL context compatible with window context

  EGLSurface mCacheDrawReadSurface{0u};    ///< cached 'read' surface
  EGLSurface mCacheDrawWriteSurface{0u};   ///< cached 'write' surface
  EGLContext mCacheEGLGraphicsContext{0u}; ///< cached window context
};

Context::Context(EglGraphicsController& controller, Integration::GlAbstraction* glAbstraction)
{
  mImpl = std::make_unique<Impl>(controller, glAbstraction);
}

Context::~Context()
{
  // Destroy native rendering context if one exists
  if(mImpl->mNativeDrawContext)
  {
    eglDestroyContext(eglGetCurrentDisplay(), mImpl->mNativeDrawContext);
    mImpl->mNativeDrawContext = EGL_NO_CONTEXT;
  }
}

void Context::Flush(bool reset, const GLES::DrawCallDescriptor& drawCall, GLES::TextureDependencyChecker& dependencyChecker)
{
  auto* gl = mImpl->GetGL();
  if(!gl) // Early out if no gl
  {
    return;
  }

  static const bool hasGLES3(mImpl->mController.GetGLESVersion() >= GLESVersion::GLES_30);

  // early out if neither current nor new pipelines are set
  // this behaviour may be valid so no assert
  if(!mImpl->mCurrentPipeline && !mImpl->mNewPipeline)
  {
    return;
  }

  // Execute states if pipeline is changed
  const auto currentProgram = mImpl->mCurrentPipeline ? static_cast<const GLES::Program*>(mImpl->mCurrentPipeline->GetCreateInfo().programState->program) : nullptr;

  // case when new pipeline has been set
  const GLES::Program* newProgram = nullptr;

  if(mImpl->mNewPipeline)
  {
    newProgram = static_cast<const GLES::Program*>(mImpl->mNewPipeline->GetCreateInfo().programState->program);
  }

  if(!currentProgram && !newProgram)
  {
    // Early out if we have no program for this pipeline.
    DALI_LOG_ERROR("No program defined for pipeline\n");
    return;
  }

  // If this draw uses a different pipeline _AND_ the pipeline has a different GL Program,
  // Then bind the new program. Ensure vertex atrributes are set.

  bool programChanged = false;
  if(mImpl->mNewPipeline && mImpl->mCurrentPipeline != mImpl->mNewPipeline)
  {
    if(!currentProgram || currentProgram->GetImplementation()->GetGlProgram() != newProgram->GetImplementation()->GetGlProgram())
    {
      mImpl->mNewPipeline->Bind(newProgram->GetImplementation()->GetGlProgram());
      programChanged = true;
    }

    // Blend state
    ResolveBlendState();

    // Resolve rasterization state
    ResolveRasterizationState();
  }

  // Resolve uniform buffers
  ResolveUniformBuffers();

  // Bind textures
  // Map binding# to sampler location
  const auto& reflection = !newProgram ? currentProgram->GetReflection() : newProgram->GetReflection();
  const auto& samplers   = reflection.GetSamplers();

  uint32_t currentSampler = 0;
  uint32_t currentElement = 0;

  // @warning Assume that binding.binding is strictly linear in the same order as mCurrentTextureBindings
  // elements. This avoids having to sort the bindings.
  for(const auto& binding : mImpl->mCurrentTextureBindings)
  {
    if(currentSampler >= samplers.size())
    {
      // Don't bind more textures than there are active samplers.
      break;
    }

    auto texture = const_cast<GLES::Texture*>(static_cast<const GLES::Texture*>(binding.texture));

    // Texture may not have been initialized yet...(tbm_surface timing issue?)
    if(!texture->GetGLTexture())
    {
      texture->InitializeResource();
    }

    // Warning, this may cause glWaitSync to occur on the GPU, or glClientWaitSync to block the CPU.
    dependencyChecker.CheckNeedsSync(this, texture, true);
    texture->Bind(binding);

    if(texture->IsNativeTexture())
    {
      texture->Prepare();
      dependencyChecker.MarkNativeTexturePrepared(texture);
    }

    if(programChanged)
    {
      // @warning Assume that location of array elements is sequential.
      // @warning GL does not guarantee this, but in practice, it is.
      gl->Uniform1i(samplers[currentSampler].location + currentElement,
                    samplers[currentSampler].offset + currentElement);
      ++currentElement;
      if(currentElement >= samplers[currentSampler].elementCount)
      {
        ++currentSampler;
        currentElement = 0;
      }
    }
  }

  const auto& pipelineState    = mImpl->mNewPipeline ? mImpl->mNewPipeline->GetCreateInfo() : mImpl->mCurrentPipeline->GetCreateInfo();
  const auto& vertexInputState = pipelineState.vertexInputState;

  // for each attribute bind vertices, unless the pipeline+buffer is the same
  if(programChanged || mImpl->mVertexBuffersChanged)
  {
    if(hasGLES3)
    {
      mImpl->BindProgramVAO(static_cast<const GLES::Program*>(pipelineState.programState->program)->GetImplementation(), *vertexInputState);
    }

    for(const auto& attr : vertexInputState->attributes)
    {
      // Enable location
      if(!hasGLES3)
      {
        mImpl->SetVertexAttributeLocation(attr.location, true);
      }

      const auto& bufferSlot    = mImpl->mCurrentVertexBufferBindings[attr.binding];
      const auto& bufferBinding = vertexInputState->bufferBindings[attr.binding];

      auto glesBuffer = bufferSlot.buffer->GetGLBuffer();

      BindBuffer(GL_ARRAY_BUFFER, glesBuffer); // Cached

      if(attr.format == VertexInputFormat::FLOAT ||
         attr.format == VertexInputFormat::FVECTOR2 ||
         attr.format == VertexInputFormat::FVECTOR3 ||
         attr.format == VertexInputFormat::FVECTOR4)
      {
        gl->VertexAttribPointer(attr.location, // Not cached...
                                GLVertexFormat(attr.format).size,
                                GLVertexFormat(attr.format).format,
                                GL_FALSE,
                                bufferBinding.stride,
                                reinterpret_cast<void*>(attr.offset));
      }
      else
      {
        gl->VertexAttribIPointer(attr.location,
                                 GLVertexFormat(attr.format).size,
                                 GLVertexFormat(attr.format).format,
                                 bufferBinding.stride,
                                 reinterpret_cast<void*>(attr.offset));
      }

      if(hasGLES3)
      {
        switch(bufferBinding.inputRate)
        {
          case Graphics::VertexInputRate::PER_VERTEX:
          {
            gl->VertexAttribDivisor(attr.location, 0);
            break;
          }
          case Graphics::VertexInputRate::PER_INSTANCE:
          {
            //@todo Get actual instance rate...
            gl->VertexAttribDivisor(attr.location, 1);
            break;
          }
        }
      }
    }

    // Reset vertex buffer changed flag now.
    mImpl->mVertexBuffersChanged = false;
  }

  // Resolve topology
  const auto& ia = pipelineState.inputAssemblyState;

  // Resolve draw call
  switch(drawCall.type)
  {
    case DrawCallDescriptor::Type::DRAW:
    {
      mImpl->mGlStateCache.mFrameBufferStateCache.DrawOperation(mImpl->mGlStateCache.mColorMask,
                                                                mImpl->mGlStateCache.DepthBufferWriteEnabled(),
                                                                mImpl->mGlStateCache.StencilBufferWriteEnabled());
      // For GLES3+ we use VAO, for GLES2 internal cache
      if(!hasGLES3)
      {
        mImpl->FlushVertexAttributeLocations();
      }

      if(drawCall.draw.instanceCount == 0)
      {
        gl->DrawArrays(GLESTopology(ia->topology),
                       drawCall.draw.firstVertex,
                       drawCall.draw.vertexCount);
      }
      else
      {
        gl->DrawArraysInstanced(GLESTopology(ia->topology),
                                drawCall.draw.firstVertex,
                                drawCall.draw.vertexCount,
                                drawCall.draw.instanceCount);
      }
      break;
    }
    case DrawCallDescriptor::Type::DRAW_INDEXED:
    {
      const auto& binding = mImpl->mCurrentIndexBufferBinding;
      BindBuffer(GL_ELEMENT_ARRAY_BUFFER, binding.buffer->GetGLBuffer());

      mImpl->mGlStateCache.mFrameBufferStateCache.DrawOperation(mImpl->mGlStateCache.mColorMask,
                                                                mImpl->mGlStateCache.DepthBufferWriteEnabled(),
                                                                mImpl->mGlStateCache.StencilBufferWriteEnabled());

      // For GLES3+ we use VAO, for GLES2 internal cache
      if(!hasGLES3)
      {
        mImpl->FlushVertexAttributeLocations();
      }

      auto indexBufferFormat = GLIndexFormat(binding.format).format;
      if(drawCall.drawIndexed.instanceCount == 0)
      {
        gl->DrawElements(GLESTopology(ia->topology),
                         drawCall.drawIndexed.indexCount,
                         indexBufferFormat,
                         reinterpret_cast<void*>(binding.offset));
      }
      else
      {
        gl->DrawElementsInstanced(GLESTopology(ia->topology),
                                  drawCall.drawIndexed.indexCount,
                                  indexBufferFormat,
                                  reinterpret_cast<void*>(binding.offset),
                                  drawCall.drawIndexed.instanceCount);
      }
      break;
    }
    case DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT:
    {
      break;
    }
  }

  ClearState();

  // Change pipeline
  if(mImpl->mNewPipeline)
  {
    mImpl->mCurrentPipeline = mImpl->mNewPipeline;
    mImpl->mNewPipeline     = nullptr;
  }
}

void Context::BindTextures(const Graphics::TextureBinding* bindings, uint32_t count)
{
  // for each texture allocate slot
  for(auto i = 0u; i < count; ++i)
  {
    auto& binding = bindings[i];

    // Resize binding array if needed
    if(mImpl->mCurrentTextureBindings.size() <= binding.binding)
    {
      mImpl->mCurrentTextureBindings.resize(binding.binding + 1);
    }
    // Store the binding details
    mImpl->mCurrentTextureBindings[binding.binding] = binding;
  }
}

void Context::BindVertexBuffers(const GLES::VertexBufferBindingDescriptor* bindings, uint32_t count)
{
  if(count > mImpl->mCurrentVertexBufferBindings.size())
  {
    mImpl->mCurrentVertexBufferBindings.resize(count);
  }
  // Copy only set slots
  auto toIter = mImpl->mCurrentVertexBufferBindings.begin();
  for(auto fromIter = bindings, end = bindings + count; fromIter != end; ++fromIter)
  {
    if(fromIter->buffer != nullptr)
    {
      if(toIter->buffer != fromIter->buffer || toIter->offset != fromIter->offset)
      {
        mImpl->mVertexBuffersChanged = true;
      }
      *toIter++ = *fromIter;
    }
  }
}

void Context::BindIndexBuffer(const IndexBufferBindingDescriptor& indexBufferBinding)
{
  mImpl->mCurrentIndexBufferBinding = indexBufferBinding;
}

void Context::BindPipeline(const GLES::Pipeline* newPipeline)
{
  DALI_ASSERT_ALWAYS(newPipeline && "Invalid pipeline");
  mImpl->mNewPipeline = &newPipeline->GetPipeline();
}

void Context::BindUniformBuffers(const UniformBufferBindingDescriptor* uboBindings,
                                 uint32_t                              uboCount,
                                 const UniformBufferBindingDescriptor& standaloneBindings)
{
  if(standaloneBindings.buffer)
  {
    mImpl->mCurrentStandaloneUBOBinding = standaloneBindings;
  }

  if(uboCount && uboCount > mImpl->mCurrentUBOBindings.size())
  {
    mImpl->mCurrentUBOBindings.resize(uboCount);
  }

  auto it = uboBindings;
  for(auto i = 0u; i < uboCount; ++i)
  {
    if(it->buffer)
    {
      mImpl->mCurrentUBOBindings[i] = *it;
    }
    ++it;
  }
}

void Context::ResolveBlendState()
{
  const auto& currentBlendState = mImpl->mCurrentPipeline ? mImpl->mCurrentPipeline->GetCreateInfo().colorBlendState : nullptr;
  const auto& newBlendState     = mImpl->mNewPipeline->GetCreateInfo().colorBlendState;

  // TODO: prevent leaking the state
  if(!newBlendState)
  {
    return;
  }

  auto* gl = mImpl->GetGL();
  if(!gl) // Early out if no gl
  {
    return;
  }

  if(!currentBlendState || currentBlendState->blendEnable != newBlendState->blendEnable)
  {
    if(newBlendState->blendEnable != mImpl->mGlStateCache.mBlendEnabled)
    {
      mImpl->mGlStateCache.mBlendEnabled = newBlendState->blendEnable;
      newBlendState->blendEnable ? gl->Enable(GL_BLEND) : gl->Disable(GL_BLEND);
    }
  }

  if(!newBlendState->blendEnable)
  {
    return;
  }

  BlendFactor newSrcRGB(newBlendState->srcColorBlendFactor);
  BlendFactor newDstRGB(newBlendState->dstColorBlendFactor);
  BlendFactor newSrcAlpha(newBlendState->srcAlphaBlendFactor);
  BlendFactor newDstAlpha(newBlendState->dstAlphaBlendFactor);

  if(!currentBlendState ||
     currentBlendState->srcColorBlendFactor != newSrcRGB ||
     currentBlendState->dstColorBlendFactor != newDstRGB ||
     currentBlendState->srcAlphaBlendFactor != newSrcAlpha ||
     currentBlendState->dstAlphaBlendFactor != newDstAlpha)
  {
    if((mImpl->mGlStateCache.mBlendFuncSeparateSrcRGB != newSrcRGB) ||
       (mImpl->mGlStateCache.mBlendFuncSeparateDstRGB != newDstRGB) ||
       (mImpl->mGlStateCache.mBlendFuncSeparateSrcAlpha != newSrcAlpha) ||
       (mImpl->mGlStateCache.mBlendFuncSeparateDstAlpha != newDstAlpha))
    {
      mImpl->mGlStateCache.mBlendFuncSeparateSrcRGB   = newSrcRGB;
      mImpl->mGlStateCache.mBlendFuncSeparateDstRGB   = newDstRGB;
      mImpl->mGlStateCache.mBlendFuncSeparateSrcAlpha = newSrcAlpha;
      mImpl->mGlStateCache.mBlendFuncSeparateDstAlpha = newDstAlpha;

      if(newSrcRGB == newSrcAlpha && newDstRGB == newDstAlpha)
      {
        gl->BlendFunc(GLBlendFunc(newSrcRGB), GLBlendFunc(newDstRGB));
      }
      else
      {
        gl->BlendFuncSeparate(GLBlendFunc(newSrcRGB), GLBlendFunc(newDstRGB), GLBlendFunc(newSrcAlpha), GLBlendFunc(newDstAlpha));
      }
    }
  }

  if(!currentBlendState ||
     currentBlendState->colorBlendOp != newBlendState->colorBlendOp ||
     currentBlendState->alphaBlendOp != newBlendState->alphaBlendOp)
  {
    if(mImpl->mGlStateCache.mBlendEquationSeparateModeRGB != newBlendState->colorBlendOp ||
       mImpl->mGlStateCache.mBlendEquationSeparateModeAlpha != newBlendState->alphaBlendOp)
    {
      mImpl->mGlStateCache.mBlendEquationSeparateModeRGB   = newBlendState->colorBlendOp;
      mImpl->mGlStateCache.mBlendEquationSeparateModeAlpha = newBlendState->alphaBlendOp;

      if(newBlendState->colorBlendOp == newBlendState->alphaBlendOp)
      {
        gl->BlendEquation(GLBlendOp(newBlendState->colorBlendOp));
        if(newBlendState->colorBlendOp >= Graphics::ADVANCED_BLEND_OPTIONS_START)
        {
          gl->BlendBarrier();
        }
      }
      else
      {
        gl->BlendEquationSeparate(GLBlendOp(newBlendState->colorBlendOp), GLBlendOp(newBlendState->alphaBlendOp));
      }
    }
  }
}

void Context::ResolveRasterizationState()
{
  const auto& currentRasterizationState = mImpl->mCurrentPipeline ? mImpl->mCurrentPipeline->GetCreateInfo().rasterizationState : nullptr;
  const auto& newRasterizationState     = mImpl->mNewPipeline->GetCreateInfo().rasterizationState;

  // TODO: prevent leaking the state
  if(!newRasterizationState)
  {
    return;
  }

  auto* gl = mImpl->GetGL();
  if(!gl) // Early out if no gl
  {
    return;
  }

  if(!currentRasterizationState ||
     currentRasterizationState->cullMode != newRasterizationState->cullMode)
  {
    if(mImpl->mGlStateCache.mCullFaceMode != newRasterizationState->cullMode)
    {
      mImpl->mGlStateCache.mCullFaceMode = newRasterizationState->cullMode;
      if(newRasterizationState->cullMode == CullMode::NONE)
      {
        gl->Disable(GL_CULL_FACE);
      }
      else
      {
        gl->Enable(GL_CULL_FACE);
        gl->CullFace(GLCullMode(newRasterizationState->cullMode));
      }
    }
  }
  // TODO: implement polygon mode (fill, line, points)
  //       seems like we don't support it (no glPolygonMode())
}

void Context::ResolveUniformBuffers()
{
  // Resolve standalone uniforms if we have binding
  if(mImpl->mCurrentStandaloneUBOBinding.buffer)
  {
    ResolveStandaloneUniforms();
  }
  if(!mImpl->mCurrentUBOBindings.empty())
  {
    ResolveGpuUniformBuffers();
  }
}

void Context::ResolveGpuUniformBuffers()
{
  if(auto* gl = mImpl->GetGL())
  {
    auto i = 0u;
    for(auto& binding : mImpl->mCurrentUBOBindings)
    {
      gl->BindBufferRange(GL_UNIFORM_BUFFER, i++, binding.buffer->GetGLBuffer(), GLintptr(binding.offset), GLintptr(binding.dataSize));
    }
  }
}

void Context::ResolveStandaloneUniforms()
{
  // Find reflection for program
  const GLES::Program* program{nullptr};

  if(mImpl->mNewPipeline)
  {
    program = static_cast<const GLES::Program*>(mImpl->mNewPipeline->GetCreateInfo().programState->program);
  }
  else if(mImpl->mCurrentPipeline)
  {
    program = static_cast<const GLES::Program*>(mImpl->mCurrentPipeline->GetCreateInfo().programState->program);
  }

  if(program)
  {
    const auto ptr = reinterpret_cast<const char*>(mImpl->mCurrentStandaloneUBOBinding.buffer->GetCPUAllocatedAddress()) + mImpl->mCurrentStandaloneUBOBinding.offset;
    // Update program uniforms
    program->GetImplementation()->UpdateStandaloneUniformBlock(ptr);
  }
}

void Context::BeginRenderPass(const BeginRenderPassDescriptor& renderPassBegin)
{
  auto& renderPass   = *renderPassBegin.renderPass;
  auto& renderTarget = *renderPassBegin.renderTarget;

  const auto& targetInfo = renderTarget.GetCreateInfo();

  auto* gl = mImpl->GetGL();
  if(!gl) // Early out if no gl
  {
    return;
  }

  if(targetInfo.surface)
  {
    // Bind surface FB
    BindFrameBuffer(GL_FRAMEBUFFER, 0);
  }
  else if(targetInfo.framebuffer)
  {
    // bind framebuffer and swap.
    auto framebuffer = renderTarget.GetFramebuffer();
    framebuffer->Bind();
  }

  // clear (ideally cache the setup)

  // In GL we assume that the last attachment is depth/stencil (we may need
  // to cache extra information inside GLES RenderTarget if we want to be
  // more specific in case of MRT)

  const auto& attachments = *renderPass.GetCreateInfo().attachments;
  const auto& color0      = attachments[0];
  GLuint      mask        = 0;

  if(color0.loadOp == AttachmentLoadOp::CLEAR)
  {
    mask |= GL_COLOR_BUFFER_BIT;

    // Set clear color
    // Something goes wrong here if Alpha mask is GL_TRUE
    ColorMask(true);

    const auto clearValues = renderPassBegin.clearValues.Ptr();

    if(!Dali::Equals(mImpl->mGlStateCache.mClearColor.r, clearValues[0].color.r) ||
       !Dali::Equals(mImpl->mGlStateCache.mClearColor.g, clearValues[0].color.g) ||
       !Dali::Equals(mImpl->mGlStateCache.mClearColor.b, clearValues[0].color.b) ||
       !Dali::Equals(mImpl->mGlStateCache.mClearColor.a, clearValues[0].color.a) ||
       !mImpl->mGlStateCache.mClearColorSet)
    {
      gl->ClearColor(clearValues[0].color.r,
                     clearValues[0].color.g,
                     clearValues[0].color.b,
                     clearValues[0].color.a);

      mImpl->mGlStateCache.mClearColorSet = true;
      mImpl->mGlStateCache.mClearColor    = Vector4(clearValues[0].color.r,
                                                 clearValues[0].color.g,
                                                 clearValues[0].color.b,
                                                 clearValues[0].color.a);
    }
  }

  // check for depth stencil
  if(attachments.size() > 1)
  {
    const auto& depthStencil = attachments.back();
    if(depthStencil.loadOp == AttachmentLoadOp::CLEAR)
    {
      if(!mImpl->mGlStateCache.mDepthMaskEnabled)
      {
        mImpl->mGlStateCache.mDepthMaskEnabled = true;
        gl->DepthMask(true);
      }
      mask |= GL_DEPTH_BUFFER_BIT;
    }
    if(depthStencil.stencilLoadOp == AttachmentLoadOp::CLEAR)
    {
      if(mImpl->mGlStateCache.mStencilMask != 0xFF)
      {
        mImpl->mGlStateCache.mStencilMask = 0xFF;
        gl->StencilMask(0xFF);
      }
      mask |= GL_STENCIL_BUFFER_BIT;
    }
  }

  SetScissorTestEnabled(true);
  gl->Scissor(renderPassBegin.renderArea.x, renderPassBegin.renderArea.y, renderPassBegin.renderArea.width, renderPassBegin.renderArea.height);
  ClearBuffer(mask, true);
  SetScissorTestEnabled(false);

  mImpl->mCurrentRenderPass   = &renderPass;
  mImpl->mCurrentRenderTarget = &renderTarget;
}

void Context::EndRenderPass(GLES::TextureDependencyChecker& dependencyChecker)
{
  if(mImpl->mCurrentRenderTarget)
  {
    GLES::Framebuffer* framebuffer = mImpl->mCurrentRenderTarget->GetFramebuffer();
    auto*              gl          = mImpl->GetGL();
    if(framebuffer && gl)
    {
      gl->Flush();

      /* @todo Full dependency checking would need to store textures in Begin, and create
       * fence objects here; but we're going to draw all fbos on shared context in serial,
       * so no real need (yet). Might want to consider ensuring order of render passes,
       * but that needs doing in the controller, and would need doing before ProcessCommandQueues.
       *
       * Currently up to the client to create render tasks in the right order.
       */

      /* Create fence sync objects. Other contexts can then wait on these fences before reading
       * textures.
       */
      dependencyChecker.AddTextures(this, framebuffer);
    }
  }

  if(dependencyChecker.GetNativeTextureCount() > 0)
  {
    dependencyChecker.CreateNativeTextureSync(this);
  }
}

void Context::ReadPixels(uint8_t* buffer)
{
  if(buffer && mImpl->mCurrentRenderTarget)
  {
    GLES::Framebuffer* framebuffer = mImpl->mCurrentRenderTarget->GetFramebuffer();
    auto*              gl          = mImpl->GetGL();
    if(framebuffer && gl)
    {
      gl->Finish(); // To guarantee ReadPixels.
      gl->ReadPixels(0, 0, framebuffer->GetCreateInfo().size.width, framebuffer->GetCreateInfo().size.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
  }
}


void Context::ClearState()
{
  mImpl->mCurrentTextureBindings.clear();
  mImpl->mCurrentUBOBindings.clear();
}

void Context::ColorMask(bool enabled)
{
  auto* gl = mImpl->GetGL();
  if(gl && enabled != mImpl->mGlStateCache.mColorMask)
  {
    mImpl->mGlStateCache.mColorMask = enabled;
    gl->ColorMask(enabled, enabled, enabled, enabled);
  }
}

void Context::ClearStencilBuffer()
{
  ClearBuffer(GL_STENCIL_BUFFER_BIT, false);
}

void Context::ClearDepthBuffer()
{
  ClearBuffer(GL_DEPTH_BUFFER_BIT, false);
}

void Context::ClearBuffer(uint32_t mask, bool forceClear)
{
  mask     = mImpl->mGlStateCache.mFrameBufferStateCache.GetClearMask(mask, forceClear, mImpl->mGlStateCache.mScissorTestEnabled);
  auto* gl = mImpl->GetGL();
  if(mask > 0 && gl)
  {
    gl->Clear(mask);
  }
}

void Context::InvalidateDepthStencilBuffers()
{
  if(auto* gl = mImpl->GetGL())
  {
    GLenum attachments[] = {GL_DEPTH, GL_STENCIL};
    gl->InvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
  }
}

void Context::SetScissorTestEnabled(bool scissorEnabled)
{
  auto* gl = mImpl->GetGL();
  if(gl && mImpl->mGlStateCache.mScissorTestEnabled != scissorEnabled)
  {
    mImpl->mGlStateCache.mScissorTestEnabled = scissorEnabled;

    if(scissorEnabled)
    {
      gl->Enable(GL_SCISSOR_TEST);
    }
    else
    {
      gl->Disable(GL_SCISSOR_TEST);
    }
  }
}

void Context::SetStencilTestEnable(bool stencilEnable)
{
  auto* gl = mImpl->GetGL();
  if(gl && stencilEnable != mImpl->mGlStateCache.mStencilBufferEnabled)
  {
    mImpl->mGlStateCache.mStencilBufferEnabled = stencilEnable;

    if(stencilEnable)
    {
      gl->Enable(GL_STENCIL_TEST);
    }
    else
    {
      gl->Disable(GL_STENCIL_TEST);
    }
  }
}

void Context::StencilMask(uint32_t writeMask)
{
  auto* gl = mImpl->GetGL();
  if(gl && writeMask != mImpl->mGlStateCache.mStencilMask)
  {
    mImpl->mGlStateCache.mStencilMask = writeMask;

    gl->StencilMask(writeMask);
  }
}

void Context::StencilFunc(Graphics::CompareOp compareOp,
                          uint32_t            reference,
                          uint32_t            compareMask)
{
  auto* gl = mImpl->GetGL();
  if(gl &&
     (compareOp != mImpl->mGlStateCache.mStencilFunc ||
      reference != mImpl->mGlStateCache.mStencilFuncRef ||
      compareMask != mImpl->mGlStateCache.mStencilFuncMask))
  {
    mImpl->mGlStateCache.mStencilFunc     = compareOp;
    mImpl->mGlStateCache.mStencilFuncRef  = reference;
    mImpl->mGlStateCache.mStencilFuncMask = compareMask;

    gl->StencilFunc(GLCompareOp(compareOp).op, reference, compareMask);
  }
}

void Context::StencilOp(Graphics::StencilOp failOp,
                        Graphics::StencilOp depthFailOp,
                        Graphics::StencilOp passOp)
{
  auto* gl = mImpl->GetGL();
  if(gl &&
     (failOp != mImpl->mGlStateCache.mStencilOpFail ||
      depthFailOp != mImpl->mGlStateCache.mStencilOpDepthFail ||
      passOp != mImpl->mGlStateCache.mStencilOpDepthPass))
  {
    mImpl->mGlStateCache.mStencilOpFail      = failOp;
    mImpl->mGlStateCache.mStencilOpDepthFail = depthFailOp;
    mImpl->mGlStateCache.mStencilOpDepthPass = passOp;

    gl->StencilOp(GLStencilOp(failOp).op, GLStencilOp(depthFailOp).op, GLStencilOp(passOp).op);
  }
}

void Context::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  auto* gl = mImpl->GetGL();
  if(gl && compareOp != mImpl->mGlStateCache.mDepthFunction)
  {
    mImpl->mGlStateCache.mDepthFunction = compareOp;

    gl->DepthFunc(GLCompareOp(compareOp).op);
  }
}

void Context::SetDepthTestEnable(bool depthTestEnable)
{
  auto* gl = mImpl->GetGL();
  if(gl && depthTestEnable != mImpl->mGlStateCache.mDepthBufferEnabled)
  {
    mImpl->mGlStateCache.mDepthBufferEnabled = depthTestEnable;

    if(depthTestEnable)
    {
      gl->Enable(GL_DEPTH_TEST);
    }
    else
    {
      gl->Disable(GL_DEPTH_TEST);
    }
  }
}

void Context::SetDepthWriteEnable(bool depthWriteEnable)
{
  auto* gl = mImpl->GetGL();
  if(gl && depthWriteEnable != mImpl->mGlStateCache.mDepthMaskEnabled)
  {
    mImpl->mGlStateCache.mDepthMaskEnabled = depthWriteEnable;

    gl->DepthMask(depthWriteEnable);
  }
}

void Context::ActiveTexture(uint32_t textureBindingIndex)
{
  auto* gl = mImpl->GetGL();
  if(gl && mImpl->mGlStateCache.mActiveTextureUnit != textureBindingIndex)
  {
    mImpl->mGlStateCache.mActiveTextureUnit = textureBindingIndex;

    gl->ActiveTexture(GL_TEXTURE0 + textureBindingIndex);
  }
}

void Context::BindTexture(GLenum target, BoundTextureType textureTypeId, uint32_t textureId)
{
  uint32_t typeId = static_cast<uint32_t>(textureTypeId);
  auto*    gl     = mImpl->GetGL();
  if(gl && mImpl->mGlStateCache.mBoundTextureId[mImpl->mGlStateCache.mActiveTextureUnit][typeId] != textureId)
  {
    mImpl->mGlStateCache.mBoundTextureId[mImpl->mGlStateCache.mActiveTextureUnit][typeId] = textureId;

    gl->BindTexture(target, textureId);
  }
}

void Context::GenerateMipmap(GLenum target)
{
  if(auto* gl = mImpl->GetGL())
  {
    gl->GenerateMipmap(target);
  }
}

bool Context::BindBuffer(GLenum target, uint32_t bufferId)
{
  if(auto* gl = mImpl->GetGL())
  {
    switch(target)
    {
      case GL_ARRAY_BUFFER:
      {
        if(mImpl->mGlStateCache.mBoundArrayBufferId == bufferId)
        {
          return false;
        }
        mImpl->mGlStateCache.mBoundArrayBufferId = bufferId;
        break;
      }
      case GL_ELEMENT_ARRAY_BUFFER:
      {
        if(mImpl->mGlStateCache.mBoundElementArrayBufferId == bufferId)
        {
          return false;
        }
        mImpl->mGlStateCache.mBoundElementArrayBufferId = bufferId;
        break;
      }
    }

    // Cache miss. Bind buffer.
    gl->BindBuffer(target, bufferId);
    return true;
  }
  return false;
}

void Context::DrawBuffers(uint32_t count, const GLenum* buffers)
{
  if(auto* gl = mImpl->GetGL())
  {
    mImpl->mGlStateCache.mFrameBufferStateCache.DrawOperation(mImpl->mGlStateCache.mColorMask,
                                                              mImpl->mGlStateCache.DepthBufferWriteEnabled(),
                                                              mImpl->mGlStateCache.StencilBufferWriteEnabled());

    gl->DrawBuffers(count, buffers);
  }
}

void Context::BindFrameBuffer(GLenum target, uint32_t bufferId)
{
  if(auto* gl = mImpl->GetGL())
  {
    mImpl->mGlStateCache.mFrameBufferStateCache.SetCurrentFrameBuffer(bufferId);

    gl->BindFramebuffer(target, bufferId);
  }
}

void Context::GenFramebuffers(uint32_t count, uint32_t* framebuffers)
{
  if(auto* gl = mImpl->GetGL())
  {
    gl->GenFramebuffers(count, framebuffers);
    mImpl->mGlStateCache.mFrameBufferStateCache.FrameBuffersCreated(count, framebuffers);
  }
}

void Context::DeleteFramebuffers(uint32_t count, uint32_t* framebuffers)
{
  if(auto* gl = mImpl->GetGL())
  {
    mImpl->mGlStateCache.mFrameBufferStateCache.FrameBuffersDeleted(count, framebuffers);

    gl->DeleteFramebuffers(count, framebuffers);
  }
}

GLStateCache& Context::GetGLStateCache()
{
  return mImpl->mGlStateCache;
}

void Context::GlContextCreated()
{
  if(!mImpl->mGlContextCreated)
  {
    mImpl->mGlContextCreated = true;

    // Set the initial GL state
    mImpl->InitializeGlState();
  }
}

void Context::GlContextDestroyed()
{
  mImpl->mGlContextCreated = false;
}

void Context::InvalidateCachedPipeline(GLES::Pipeline* pipeline)
{
  // Since the pipeline is deleted, invalidate the cached pipeline.
  if(mImpl->mCurrentPipeline == &pipeline->GetPipeline())
  {
    mImpl->mCurrentPipeline = nullptr;
  }

  // Remove cached VAO map
  auto* gl = mImpl->GetGL();
  if(gl)
  {
    const auto* program = pipeline->GetCreateInfo().programState->program;
    if(program)
    {
      const auto* programImpl = static_cast<const GLES::Program*>(program)->GetImplementation();
      if(programImpl)
      {
        auto iter = mImpl->mProgramVAOMap.find(programImpl);
        if(iter != mImpl->mProgramVAOMap.end())
        {
          for(auto& attributeHashPair : iter->second)
          {
            auto vao = attributeHashPair.second;

            // Do not delete vao now. (Since Context might not be current.)
            mImpl->mDiscardedVAOList.emplace_back(vao);
            if(mImpl->mProgramVAOCurrentState == vao)
            {
              mImpl->mProgramVAOCurrentState = 0u;
            }
          }

          // Clear cached Vertex buffer.
          mImpl->mCurrentVertexBufferBindings.clear();

          mImpl->mProgramVAOMap.erase(iter);
        }
      }
    }
  }
}

void Context::PrepareForNativeRendering()
{
  // this should be pretty much constant
  auto display     = eglGetCurrentDisplay();
  auto drawSurface = eglGetCurrentSurface(EGL_DRAW);
  auto readSurface = eglGetCurrentSurface(EGL_READ);
  auto context     = eglGetCurrentContext();

  // push the surface and context data to the impl
  // It's needed to restore context
  if(!mImpl->mCacheEGLGraphicsContext)
  {
    mImpl->mCacheDrawWriteSurface   = drawSurface;
    mImpl->mCacheDrawReadSurface    = readSurface;
    mImpl->mCacheEGLGraphicsContext = context;
  }

  if(!mImpl->mNativeDrawContext)
  {
    EGLint configId{0u};
    eglQueryContext(display, mImpl->mController.GetSharedContext(), EGL_CONFIG_ID, &configId);

    EGLint configAttribs[3];
    configAttribs[0] = EGL_CONFIG_ID;
    configAttribs[1] = configId;
    configAttribs[2] = EGL_NONE;

    EGLConfig config;
    EGLint    numConfigs;
    if(eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) != EGL_TRUE)
    {
      DALI_LOG_ERROR("eglChooseConfig failed!\n");
      return;
    }

    auto version = int(mImpl->mController.GetGLESVersion());

    std::vector<EGLint> attribs;
    attribs.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
    attribs.push_back(version / 10);
    attribs.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
    attribs.push_back(version % 10);
    attribs.push_back(EGL_NONE);

    mImpl->mNativeDrawContext = eglCreateContext(display, config, mImpl->mController.GetSharedContext(), attribs.data());
    if(mImpl->mNativeDrawContext == EGL_NO_CONTEXT)
    {
      DALI_LOG_ERROR("eglCreateContext failed!\n");
      return;
    }
  }

  eglMakeCurrent(display, drawSurface, readSurface, mImpl->mNativeDrawContext);
  // make sure it's current window context
  eglMakeCurrent(display, mImpl->mCacheDrawWriteSurface, mImpl->mCacheDrawReadSurface, mImpl->mCacheEGLGraphicsContext);
}

void Context::ResetGLESState()
{
  mImpl->mGlStateCache.ResetBufferCache();
  mImpl->mGlStateCache.ResetTextureCache();
  mImpl->mCurrentPipeline = nullptr;
  mImpl->mCurrentUBOBindings.clear();
  mImpl->mCurrentTextureBindings.clear();
  mImpl->mCurrentVertexBufferBindings.clear();
  mImpl->mCurrentRenderTarget       = nullptr;
  mImpl->mCurrentRenderPass         = nullptr;
  mImpl->mVertexBuffersChanged      = true;
  mImpl->mCurrentIndexBufferBinding = {};
  mImpl->mCurrentSamplerBindings    = {};
  mImpl->mProgramVAOCurrentState    = 0;

  ClearState();
  mImpl->InitializeGlState();
}

void Context::RestoreFromNativeRendering()
{
  auto display = eglGetCurrentDisplay();

  // bring back original context
  eglMakeCurrent(display, mImpl->mCacheDrawWriteSurface, mImpl->mCacheDrawReadSurface, mImpl->mCacheEGLGraphicsContext);
}

} // namespace Dali::Graphics::GLES
