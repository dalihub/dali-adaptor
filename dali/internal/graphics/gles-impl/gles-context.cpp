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
#include <unordered_set>

namespace
{
DALI_INIT_TIME_CHECKER_FILTER(gTimeCheckerFilter, DALI_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);

constexpr uint32_t CLEAR_CACHED_NATIVE_TEXTURE_THRESHOLD = 100u;

constexpr uint32_t CPU_ALLOCATED_UBO_INDEX       = 0u;
constexpr uint32_t GPU_ALLOCATED_UBO_INDEX_BEGIN = 1u;

/**
 * Memory compare working on 4-byte types. Since all types used in shaders are
 * size of 4*N then no need for size and alignment checks.
 */
template<class A, class B>
inline bool memcmp4(A* a, B* b, size_t size)
{
  auto* pa = reinterpret_cast<const uint32_t*>(a);
  auto* pb = reinterpret_cast<const uint32_t*>(b);
  size >>= 2;
  while(size-- && *pa++ == *pb++);
  return (-1u == size);
};
} // namespace

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
    auto* gl = GetGL();
    if(DALI_UNLIKELY(!gl)) // early out if no gl
    {
      return;
    }

    if(DALI_UNLIKELY(!mDiscardedVAOList.empty()))
    {
      gl->DeleteVertexArrays(static_cast<Dali::GLsizei>(mDiscardedVAOList.size()), mDiscardedVAOList.data());
      mDiscardedVAOList.clear();
    }

    // Calculate attributes location hash unordered.
    AttributesHash hash = 0;
    for(const auto& attr : vertexInputState.attributes)
    {
      // Make unordered hash value by location.
      // Note : This hash function varified for locations only under < 20.
      AttributesHash salt = attr.location + 1;
      hash += salt << (sizeof(AttributesHash) * 6);
      salt *= salt;
      salt ^= attr.location;
      hash += salt << (sizeof(AttributesHash) * 4);
      salt *= salt;
      hash += salt;
    }

    auto iter = mProgramVAOMap.find(program);
    if(iter != mProgramVAOMap.end())
    {
      auto attributeIter = iter->second.find(hash);
      if(attributeIter != iter->second.end())
      {
        if(mVertexBufferChangedCount != attributeIter->second.vertexBufferChangedCount)
        {
          attributeIter->second.vertexBufferChangedCount = mVertexBufferChangedCount;
          if(mProgramVAOCurrentState == attributeIter->second.vao)
          {
            // Note that we should unbound and rebound VAO if we want to set changed data buffer by glVertexAttribPointer
            gl->BindVertexArray(0u);
            mProgramVAOCurrentState = 0u;
          }
        }

        if(mProgramVAOCurrentState != attributeIter->second.vao)
        {
          mProgramVAOCurrentState = attributeIter->second.vao;
          gl->BindVertexArray(mProgramVAOCurrentState);

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

    mProgramVAOMap[program][hash] = {vao, mVertexBufferChangedCount};
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
    if(DALI_LIKELY(gl))
    {
      // TODO : We just need to invalidate cache, instead of reset them as default.
      // InitializeGlState() Could be called by DrawNative case, which we should match as cached info and real gl state.
      mGlStateCache.mClearColorSet        = false;
      mGlStateCache.mColorMask            = true;
      mGlStateCache.mStencilMask          = 0xFF;
      mGlStateCache.mDepthBufferEnabled   = false;
      mGlStateCache.mDepthMaskEnabled     = false;
      mGlStateCache.mScissorTestEnabled   = false;
      mGlStateCache.mStencilBufferEnabled = false;

      mGlStateCache.mCullFaceMode = CullMode::NONE; // By default cullface is disabled, front face is set to CCW and cull face is set to back

      gl->Disable(GL_DITHER);

      mGlStateCache.mBoundArrayBufferId        = 0;
      mGlStateCache.mBoundElementArrayBufferId = 0;
      mGlStateCache.mActiveTextureUnit         = MAX_TEXTURE_UNITS; // Set MAX_TEXTURE_UNITS for initialize state.

      mGlStateCache.mBlendStateCache.InvalidateCache();

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
    if(DALI_LIKELY(gl))
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
    if(DALI_LIKELY(gl))
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
   * Prepare to buffer range cache for performance.
   * We could skip various memory reserving when BindBufferRange called.
   */
  void PrepareBufferRangeCache(size_t maxBindings)
  {
    if(mUniformBufferBindingCache.Count() < maxBindings)
    {
      const auto oldCount = mUniformBufferBindingCache.Count();
      mUniformBufferBindingCache.ResizeUninitialized(maxBindings);
      for(auto i = oldCount; i < maxBindings; ++i)
      {
        mUniformBufferBindingCache[i].buffer = nullptr;
      }
    }
  }

  /**
   * Binds and cache buffer ranges.
   * Cache information 'MUST' be cleard when buffer pointer changed, or some programs invalidated.
   */
  void BindBufferRange(const UniformBufferBindingDescriptor& binding)
  {
    auto* gl = GetGL();
    if(DALI_UNLIKELY(!gl)) // early out if no gl
    {
      return;
    }

    DALI_ASSERT_DEBUG(mUniformBufferBindingCache.Count() > binding.binding && "PrepareBufferRangeCache not called!");

    auto& cachedBinding = mUniformBufferBindingCache[binding.binding];

    if(!memcmp4(&cachedBinding, &binding, sizeof(UniformBufferBindingDescriptor)))
    {
      // Cache not hit. Update cache and call glBindBufferRange
      memcpy(&cachedBinding, &binding, sizeof(UniformBufferBindingDescriptor));
      gl->BindBufferRange(GL_UNIFORM_BUFFER, binding.binding, binding.buffer->GetGLBuffer(), GLintptr(binding.offset), GLintptr(binding.dataSize));
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

  Dali::Vector<Graphics::TextureBinding> mCurrentTextureBindings{};
  std::unordered_set<GLES::Texture*>     mPreparedNativeTextures{};

  GLES::IndexBufferBindingDescriptor mCurrentIndexBufferBinding{};

  struct VertexBufferBinding
  {
    GLES::Buffer* buffer{nullptr};
    uint32_t      offset{0u};
  };

  // Currently bound buffers
  std::vector<VertexBufferBindingDescriptor> mCurrentVertexBufferBindings{};
  std::vector<uint32_t>                      mCurrentVertexBufferChangedCount{};
  uint32_t                                   mVertexBufferChangedCount{0u}; ///< Increase if any of vertex buffer changed at BindVertexBuffer

  // Currently bound UBOs (check if it's needed per program!)
  Dali::Vector<UniformBufferBindingDescriptor> mCurrentUBOBindings{};

  // Keep bind buffer range. Should be cleared if program changed.
  Dali::Vector<UniformBufferBindingDescriptor> mUniformBufferBindingCache;

  // Current render pass and render target
  const GLES::RenderTarget* mCurrentRenderTarget{nullptr};
  const GLES::RenderPass*   mCurrentRenderPass{nullptr};

  // Each context must have own VAOs as they cannot be shared
  struct VAOInformations
  {
    uint32_t vao;
    uint32_t vertexBufferChangedCount; ///< Latest mVertexBufferChangedCount value if VAO bounded
  };
  using AttributesHash   = std::size_t;
  using AttributesVAOMap = std::map<AttributesHash, VAOInformations>;
  using ProgramVAOMap    = std::unordered_map<const GLES::ProgramImpl*, AttributesVAOMap>;

  ProgramVAOMap mProgramVAOMap;              ///< GL program-VAO map
  uint32_t      mProgramVAOCurrentState{0u}; ///< Currently bound VAO

  GLStateCache mGlStateCache{}; ///< GL status cache

  std::vector<Dali::GLuint> mDiscardedVAOList{};

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
  ClearCachedNativeTexture();

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
  if(DALI_UNLIKELY(!gl)) // Early out if no gl
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

      ClearUniformBufferCache();
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

  bool needDraw = true;

  // @warning Assume that binding.binding is strictly linear in the same order as mCurrentTextureBindings
  // elements. This avoids having to sort the bindings.
  for(const auto& binding : mImpl->mCurrentTextureBindings)
  {
    if(currentSampler >= samplers.size())
    {
      // Don't bind more textures than there are active samplers.
      break;
    }

    DALI_ASSERT_DEBUG(binding.texture && "GLES::Texture not assigned!");

    auto texture = const_cast<GLES::Texture*>(static_cast<const GLES::Texture*>(binding.texture));

    // Texture may not have been initialized yet...(tbm_surface timing issue?)
    if(!texture->GetGLTexture())
    {
      DALI_LOG_ERROR("[ERROR] NativeImage is not initialized yet! Do not render it\n");
      needDraw = false;
    }

    // Warning, this may cause glWaitSync to occur on the GPU, or glClientWaitSync to block the CPU.
    dependencyChecker.CheckNeedsSync(this, texture, true);
    texture->Bind(binding);

    // Should call Prepare only if native texture resource created.
    if(texture->GetGLTexture() && texture->IsNativeTexture())
    {
      if(DALI_UNLIKELY(!texture->PrepareNativeTexture(this)))
      {
        DALI_LOG_ERROR("[ERROR] NativeImage prepare failed! Do not render it\n");
        needDraw = false;
      }

      // Must call it after Prepare(), and must cache even if prepare failed.
      mImpl->mPreparedNativeTextures.insert(texture);
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
  if(DALI_LIKELY(needDraw))
  {
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
                         drawCall.firstOffset,
                         drawCall.draw.vertexCount);
        }
        else
        {
          gl->DrawArraysInstanced(GLESTopology(ia->topology),
                                  drawCall.firstOffset,
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

        auto     indexBufferFormat = GLIndexFormat(binding.format).format;
        uint32_t offset            = binding.offset + drawCall.firstOffset; ///< Already byte value by buffer format at Render::Geometry
        if(drawCall.drawIndexed.instanceCount == 0)
        {
          if(drawCall.drawIndexed.vertexOffset == 0)
          {
            gl->DrawElements(GLESTopology(ia->topology),
                             drawCall.drawIndexed.indexCount,
                             indexBufferFormat,
                             reinterpret_cast<void*>(offset));
          }
          else
          {
            // // TODO : Implement DrawElementsBaseVertex here. Support over OpenGL ES 3.2
            // gl->DrawElementsBaseVertex(GLESTopology(ia->topology),
            //                            drawCall.drawIndexed.indexCount,
            //                            indexBufferFormat,
            //                            reinterpret_cast<void*>(offset),
            //                            drawCall.drawIndexed.vertexOffset);
          }
        }
        else
        {
          if(drawCall.drawIndexed.vertexOffset == 0)
          {
            gl->DrawElementsInstanced(GLESTopology(ia->topology),
                                      drawCall.drawIndexed.indexCount,
                                      indexBufferFormat,
                                      reinterpret_cast<void*>(offset),
                                      drawCall.drawIndexed.instanceCount);
          }
          else
          {
            // // TODO : Implement DrawElementsInstancedBaseVertex here. Support over OpenGL ES 3.2
            // gl->DrawElementsInstancedBaseVertex(GLESTopology(ia->topology),
            //                                     drawCall.drawIndexed.indexCount,
            //                                     indexBufferFormat,
            //                                     reinterpret_cast<void*>(offset),
            //                                     drawCall.drawIndexed.instanceCount,
            //                                     drawCall.drawIndexed.vertexOffset);
          }
        }
        break;
      }
      case DrawCallDescriptor::Type::DRAW_INDEXED_INDIRECT:
      {
        // // TODO : Implement DrawElementsIndirect here. Support over OpenGL ES 3.1
        // BindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCall.drawIndexedIndirect.buffer->GetGLBuffer());

        // const auto& binding = mImpl->mCurrentIndexBufferBinding;
        // BindBuffer(GL_ELEMENT_ARRAY_BUFFER, binding.buffer->GetGLBuffer());

        // mImpl->mGlStateCache.mFrameBufferStateCache.DrawOperation(mImpl->mGlStateCache.mColorMask,
        //                                                           mImpl->mGlStateCache.DepthBufferWriteEnabled(),
        //                                                           mImpl->mGlStateCache.StencilBufferWriteEnabled());

        // // For GLES3+ we use VAO, for GLES2 internal cache
        // if(!hasGLES3)
        // {
        //   mImpl->FlushVertexAttributeLocations();
        // }

        // auto indexBufferFormat = GLIndexFormat(binding.format).format;

        // uint32_t offset = drawCall.drawIndexedIndirect.offset;
        // for(uint32_t drawCount = 0u; drawCount < drawCall.drawIndexedIndirect.drawCount; ++drawCount)
        // {
        //   gl->DrawElementsIndirect(GLESTopology(ia->topology),
        //                           indexBufferFormat,
        //                           reinterpret_cast<void*>(offset));
        //   offset += drawCall.drawIndexedIndirect.stride;
        // }
        break;
      }
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
  // We can assume that bindings is sorted by binding number.
  // So we can only copy the data
  mImpl->mCurrentTextureBindings.ResizeUninitialized(count);
  memcpy(mImpl->mCurrentTextureBindings.Begin(), bindings, sizeof(Graphics::TextureBinding) * count);
}

void Context::BindVertexBuffers(const GLES::VertexBufferBindingDescriptor* bindings, uint32_t count)
{
  if(count > mImpl->mCurrentVertexBufferBindings.size())
  {
    mImpl->mCurrentVertexBufferBindings.resize(count);
    mImpl->mCurrentVertexBufferChangedCount.resize(count, 0u);
    mImpl->mVertexBuffersChanged = true;
  }
  bool bufferChanged = false;

  // Copy only set slots
  auto toIter      = mImpl->mCurrentVertexBufferBindings.begin();
  auto changedIter = mImpl->mCurrentVertexBufferChangedCount.begin();
  for(auto fromIter = bindings, end = bindings + count; fromIter != end; ++fromIter)
  {
    if(fromIter->buffer != nullptr)
    {
      if(toIter->buffer != fromIter->buffer || toIter->offset != fromIter->offset)
      {
        mImpl->mVertexBuffersChanged = true;
        *toIter                      = *fromIter;
      }

      // Check whether buffer data has been changed from latest bounded cases.
      if(fromIter->buffer->GetBufferChangedCount() != *changedIter)
      {
        bufferChanged = true;
        *changedIter  = fromIter->buffer->GetBufferChangedCount();
      }
      ++toIter;
      ++changedIter;
    }
  }

  if(bufferChanged)
  {
    mImpl->mVertexBuffersChanged = true;

    // Increate vertex buffer changed count.
    // It will be used whetner we need to re-call VertexAttribPointer or not.
    ++mImpl->mVertexBufferChangedCount;
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
                                 uint32_t                              uboCount)
{
  // We can assume that bindings is sorted by binding number.
  // So we can only copy the data
  mImpl->mCurrentUBOBindings.ResizeUninitialized(uboCount);
  memcpy(mImpl->mCurrentUBOBindings.Begin(), uboBindings, sizeof(UniformBufferBindingDescriptor) * uboCount);
}

void Context::ResolveBlendState()
{
  auto* gl = mImpl->GetGL();
  if(DALI_UNLIKELY(!gl)) // Early out if no gl
  {
    return;
  }

  const auto& newBlendState   = mImpl->mNewPipeline->GetCreateInfo().colorBlendState;
  auto&       blendStateCache = mImpl->mGlStateCache.mBlendStateCache;

  // Update cache data only here.
  if(newBlendState)
  {
    if(blendStateCache.mBlendEnabled != newBlendState->blendEnable)
    {
      blendStateCache.mBlendEnabled = newBlendState->blendEnable;
      blendStateCache.mBlendCacheChangedFlag |= BlendStateCache::BLEND_ENABLED_CHANGED;
    }

    // Only valid values are attached if blend enbled.
    if(newBlendState->blendEnable)
    {
      BlendFactor newSrcRGB(newBlendState->srcColorBlendFactor);
      BlendFactor newDstRGB(newBlendState->dstColorBlendFactor);
      BlendFactor newSrcAlpha(newBlendState->srcAlphaBlendFactor);
      BlendFactor newDstAlpha(newBlendState->dstAlphaBlendFactor);

      if(blendStateCache.mBlendFuncSeparateSrcRGB != newSrcRGB ||
         blendStateCache.mBlendFuncSeparateDstRGB != newDstRGB ||
         blendStateCache.mBlendFuncSeparateSrcAlpha != newSrcAlpha ||
         blendStateCache.mBlendFuncSeparateDstAlpha != newDstAlpha)
      {
        blendStateCache.mBlendFuncSeparateSrcRGB   = newSrcRGB;
        blendStateCache.mBlendFuncSeparateDstRGB   = newDstRGB;
        blendStateCache.mBlendFuncSeparateSrcAlpha = newSrcAlpha;
        blendStateCache.mBlendFuncSeparateDstAlpha = newDstAlpha;
        blendStateCache.mBlendCacheChangedFlag |= BlendStateCache::BLEND_FUNC_CHANGED;
      }

      if(blendStateCache.mBlendEquationSeparateModeRGB != newBlendState->colorBlendOp ||
         blendStateCache.mBlendEquationSeparateModeAlpha != newBlendState->alphaBlendOp)
      {
        blendStateCache.mBlendEquationSeparateModeRGB   = newBlendState->colorBlendOp;
        blendStateCache.mBlendEquationSeparateModeAlpha = newBlendState->alphaBlendOp;
        blendStateCache.mBlendCacheChangedFlag |= BlendStateCache::BLEND_EQUATION_CHANGED;
      }
    }
  }

  // Call changed blend state to gl now.
  if(blendStateCache.mBlendCacheChangedFlag & BlendStateCache::BLEND_ENABLED_CHANGED)
  {
    blendStateCache.mBlendCacheChangedFlag &= ~BlendStateCache::BLEND_ENABLED_CHANGED;
    blendStateCache.mBlendEnabled ? gl->Enable(GL_BLEND) : gl->Disable(GL_BLEND);
  }

  if(!blendStateCache.mBlendEnabled) // Early out if blend disabled
  {
    return;
  }

  if(blendStateCache.mBlendCacheChangedFlag & BlendStateCache::BLEND_FUNC_CHANGED)
  {
    blendStateCache.mBlendCacheChangedFlag &= ~BlendStateCache::BLEND_FUNC_CHANGED;
    if(blendStateCache.mBlendFuncSeparateSrcRGB == blendStateCache.mBlendFuncSeparateSrcAlpha &&
       blendStateCache.mBlendFuncSeparateDstRGB == blendStateCache.mBlendFuncSeparateDstAlpha)
    {
      gl->BlendFunc(GLBlendFunc(blendStateCache.mBlendFuncSeparateSrcRGB),
                    GLBlendFunc(blendStateCache.mBlendFuncSeparateDstRGB));
    }
    else
    {
      gl->BlendFuncSeparate(GLBlendFunc(blendStateCache.mBlendFuncSeparateSrcRGB),
                            GLBlendFunc(blendStateCache.mBlendFuncSeparateDstRGB),
                            GLBlendFunc(blendStateCache.mBlendFuncSeparateSrcAlpha),
                            GLBlendFunc(blendStateCache.mBlendFuncSeparateDstAlpha));
    }
  }

  if(blendStateCache.mBlendCacheChangedFlag & BlendStateCache::BLEND_EQUATION_CHANGED)
  {
    blendStateCache.mBlendCacheChangedFlag &= ~BlendStateCache::BLEND_EQUATION_CHANGED;
    if(blendStateCache.mBlendEquationSeparateModeRGB == blendStateCache.mBlendEquationSeparateModeAlpha)
    {
      gl->BlendEquation(GLBlendOp(blendStateCache.mBlendEquationSeparateModeRGB));
      if(blendStateCache.mBlendEquationSeparateModeRGB >= Graphics::ADVANCED_BLEND_OPTIONS_START)
      {
        gl->BlendBarrier();
      }
    }
    else
    {
      gl->BlendEquationSeparate(GLBlendOp(blendStateCache.mBlendEquationSeparateModeRGB),
                                GLBlendOp(blendStateCache.mBlendEquationSeparateModeAlpha));
    }
  }
}

void Context::ResolveRasterizationState()
{
  auto* gl = mImpl->GetGL();
  if(DALI_UNLIKELY(!gl)) // Early out if no gl
  {
    return;
  }

  // DevNode : If mImpl->mCurrentPipeline == nullptr mean, this is the first states of render loop.
  //           To make cull face cache works more effective, let we call glCullFace
  //           At least 1 time for the first render loops.
  const bool  forciblyCallCullFace  = mImpl->mCurrentPipeline == nullptr;
  const auto& newRasterizationState = mImpl->mNewPipeline->GetCreateInfo().rasterizationState;

  if(!newRasterizationState)
  {
    // Should be call blend API at least once, to ensure that cache value is valid.
    if(DALI_UNLIKELY(forciblyCallCullFace))
    {
      if(mImpl->mGlStateCache.mCullFaceMode == CullMode::NONE)
      {
        gl->Disable(GL_CULL_FACE);
      }
      else
      {
        gl->Enable(GL_CULL_FACE);
        gl->CullFace(GLCullMode(mImpl->mGlStateCache.mCullFaceMode));
      }
    }
    return;
  }

  if(forciblyCallCullFace ||
     mImpl->mGlStateCache.mCullFaceMode != newRasterizationState->cullMode)
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
  // TODO: implement polygon mode (fill, line, points)
  //       seems like we don't support it (no glPolygonMode())
}

void Context::ResolveUniformBuffers()
{
  // Resolve standalone uniforms if we have binding
  if(DALI_LIKELY(mImpl->mCurrentUBOBindings.Count() > CPU_ALLOCATED_UBO_INDEX))
  {
    if(mImpl->mCurrentUBOBindings[CPU_ALLOCATED_UBO_INDEX].buffer)
    {
      ResolveStandaloneUniforms(mImpl->mCurrentUBOBindings[CPU_ALLOCATED_UBO_INDEX]);
    }
    if(mImpl->mCurrentUBOBindings.Count() >= GPU_ALLOCATED_UBO_INDEX_BEGIN)
    {
      ResolveGpuUniformBuffers(mImpl->mCurrentUBOBindings.Begin() + GPU_ALLOCATED_UBO_INDEX_BEGIN, mImpl->mCurrentUBOBindings.Count() - GPU_ALLOCATED_UBO_INDEX_BEGIN);
    }
  }
}

void Context::ResolveGpuUniformBuffers(const UniformBufferBindingDescriptor* uboBindingPtr, uint32_t uboCount)
{
  mImpl->PrepareBufferRangeCache(uboCount);
  while(uboCount--)
  {
    const auto& binding = *(uboBindingPtr++);
    if(DALI_LIKELY(binding.buffer && binding.dataSize > 0u))
    {
      mImpl->BindBufferRange(binding);
    }
  }
}

void Context::ResolveStandaloneUniforms(const UniformBufferBindingDescriptor& standaloneUniformBinding)
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
    const auto ptr = reinterpret_cast<const char*>(standaloneUniformBinding.buffer->GetCPUAllocatedAddress()) + standaloneUniformBinding.offset;
    // Update program uniforms
    program->GetImplementation()->UpdateStandaloneUniformBlock(ptr);
  }
}

void Context::BeginRenderPass(const BeginRenderPassDescriptor& renderPassBegin)
{
  auto* gl = mImpl->GetGL();
  if(DALI_UNLIKELY(!gl)) // Early out if no gl
  {
    return;
  }

  auto& renderPass   = *renderPassBegin.renderPass;
  auto& renderTarget = *renderPassBegin.renderTarget;

  const auto& targetInfo = renderTarget.GetCreateInfo();

  if(targetInfo.surface)
  {
    // Bind surface FB
    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);
    mImpl->mGlStateCache.mFrameBufferStateCache.SetCurrentFrameBuffer(0);
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
    if(DALI_LIKELY(gl) && framebuffer)
    {
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

    if(dependencyChecker.GetNativeTextureCount() > 0)
    {
      dependencyChecker.MarkNativeTextureSyncContext(this);
#ifndef DALI_PROFILE_TV
      /// Only TV profile should not create egl sync object before eglSwapBuffers, due to DDK bug. 2024-12-13. eunkiki.hong
      dependencyChecker.CreateNativeTextureSync(this);
#endif
    }

    if(DALI_LIKELY(gl) &&
       (framebuffer
#ifndef DALI_PROFILE_TV
        || (dependencyChecker.GetNativeTextureCount() > 0)
#endif
          ))
    {
      // Need to call glFlush or eglSwapBuffer after create sync object.
      gl->Flush();
    }

    if(DALI_LIKELY(gl) && framebuffer)
    {
      InvalidateDepthStencilRenderBuffers(framebuffer);
    }
  }

  // Remove native texture list if it stored too many items.
  if(DALI_UNLIKELY(mImpl->mPreparedNativeTextures.size() > CLEAR_CACHED_NATIVE_TEXTURE_THRESHOLD))
  {
    ClearCachedNativeTexture();
  }
}

void Context::ReadPixels(uint8_t* buffer)
{
  if(buffer && mImpl->mCurrentRenderTarget)
  {
    GLES::Framebuffer* framebuffer = mImpl->mCurrentRenderTarget->GetFramebuffer();
    auto*              gl          = mImpl->GetGL();
    if(DALI_LIKELY(gl) && framebuffer)
    {
      gl->Finish(); // To guarantee ReadPixels.
      gl->ReadPixels(0, 0, framebuffer->GetCreateInfo().size.width, framebuffer->GetCreateInfo().size.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
  }
}

void Context::ClearState()
{
  mImpl->mCurrentTextureBindings.Clear();
  mImpl->mCurrentUBOBindings.Clear();
}

void Context::ClearVertexBufferCache()
{
  mImpl->mCurrentVertexBufferBindings.clear();
  mImpl->mCurrentVertexBufferChangedCount.clear();
  mImpl->mVertexBuffersChanged   = true;
  mImpl->mProgramVAOCurrentState = 0;
  ++mImpl->mVertexBufferChangedCount;
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    if(!(mImpl->mController.GetGLESVersion() >= GLESVersion::GLES_30))
    {
      memset(&mImpl->mGlStateCache.mVertexAttributeCachedState, 0, sizeof(mImpl->mGlStateCache.mVertexAttributeCachedState));
      memset(&mImpl->mGlStateCache.mVertexAttributeCurrentState, 0, sizeof(mImpl->mGlStateCache.mVertexAttributeCurrentState));
    }
  }
}

void Context::ClearUniformBufferCache()
{
  mImpl->mUniformBufferBindingCache.Clear();
}

void Context::ClearCachedNativeTexture()
{
  DALI_LOG_DEBUG_INFO("Context[%p] call ClearCachedNativeTexture : %zu\n", this, mImpl->mPreparedNativeTextures.size());
  for(auto* nativeTexture : mImpl->mPreparedNativeTextures)
  {
    nativeTexture->InvalidateCachedContext(this);
  }
  mImpl->mPreparedNativeTextures.clear();
}

void Context::InvalidateDepthStencilRenderBuffers(GLES::Framebuffer* framebuffer)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl && framebuffer))
  {
    if(framebuffer->GetGlStencilBufferId() != 0u)
    {
      GLenum attachments[] = {GL_DEPTH, GL_STENCIL};
      gl->InvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
    }
    else if(framebuffer->GetGlDepthBufferId() != 0u)
    {
      GLenum attachment = GL_DEPTH;
      gl->InvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
    }
  }
}

void Context::ColorMask(bool enabled)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl) && enabled != mImpl->mGlStateCache.mColorMask)
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
  if(DALI_LIKELY(gl) && mask > 0)
  {
    gl->Clear(mask);
  }
}

void Context::SetScissorTestEnabled(bool scissorEnabled)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl) && mImpl->mGlStateCache.mScissorTestEnabled != scissorEnabled)
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
  if(DALI_LIKELY(gl) && stencilEnable != mImpl->mGlStateCache.mStencilBufferEnabled)
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
  if(DALI_LIKELY(gl) && writeMask != mImpl->mGlStateCache.mStencilMask)
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
  if(DALI_LIKELY(gl) &&
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
  if(DALI_LIKELY(gl) &&
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
  if(DALI_LIKELY(gl) && compareOp != mImpl->mGlStateCache.mDepthFunction)
  {
    mImpl->mGlStateCache.mDepthFunction = compareOp;

    gl->DepthFunc(GLCompareOp(compareOp).op);
  }
}

void Context::SetDepthTestEnable(bool depthTestEnable)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl) && depthTestEnable != mImpl->mGlStateCache.mDepthBufferEnabled)
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
  if(DALI_LIKELY(gl) && depthWriteEnable != mImpl->mGlStateCache.mDepthMaskEnabled)
  {
    mImpl->mGlStateCache.mDepthMaskEnabled = depthWriteEnable;

    gl->DepthMask(depthWriteEnable);
  }
}

void Context::ActiveTexture(uint32_t textureBindingIndex)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl) && mImpl->mGlStateCache.mActiveTextureUnit != textureBindingIndex)
  {
    mImpl->mGlStateCache.mActiveTextureUnit = textureBindingIndex;

    gl->ActiveTexture(GL_TEXTURE0 + textureBindingIndex);
  }
}

void Context::BindTexture(GLenum target, BoundTextureType textureTypeId, uint32_t textureId)
{
  uint32_t typeId = static_cast<uint32_t>(textureTypeId);
  auto*    gl     = mImpl->GetGL();
  if(DALI_LIKELY(gl) && mImpl->mGlStateCache.mBoundTextureId[mImpl->mGlStateCache.mActiveTextureUnit][typeId] != textureId)
  {
    mImpl->mGlStateCache.mBoundTextureId[mImpl->mGlStateCache.mActiveTextureUnit][typeId] = textureId;

    gl->BindTexture(target, textureId);
  }
}

void Context::GenerateMipmap(GLenum target)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl))
  {
    gl->GenerateMipmap(target);
  }
}

bool Context::BindBuffer(GLenum target, uint32_t bufferId)
{
  auto* gl = mImpl->GetGL();
  if(DALI_LIKELY(gl))
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
  if(DALI_LIKELY(gl))
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
            auto vao = attributeHashPair.second.vao;

            // Do not delete vao now. (Since Context might not be current.)
            mImpl->mDiscardedVAOList.emplace_back(vao);
          }

          // Clear cached Vertex buffer.
          ResetBufferCache();
          ClearVertexBufferCache();

          mImpl->mProgramVAOMap.erase(iter);
        }
      }
    }
  }
}

void Context::InvalidateCachedNativeTexture(GLES::Texture* nativeTexture)
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    mImpl->mPreparedNativeTextures.erase(nativeTexture);
  }
}

void Context::PrepareForNativeRendering()
{
  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);

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

    {
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglQueryContext(Native)");
      eglQueryContext(display, mImpl->mController.GetSharedContext(), EGL_CONFIG_ID, &configId);
    }

    EGLint configAttribs[3];
    configAttribs[0] = EGL_CONFIG_ID;
    configAttribs[1] = configId;
    configAttribs[2] = EGL_NONE;

    EGLConfig config;
    EGLint    numConfigs;
    {
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglChooseConfig(Native)");
      if(eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) != EGL_TRUE)
      {
        DALI_LOG_ERROR("eglChooseConfig failed!\n");
        return;
      }
    }

    auto version = int(mImpl->mController.GetGLESVersion());

    std::vector<EGLint> attribs;
    attribs.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
    attribs.push_back(version / 10);
    attribs.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
    attribs.push_back(version % 10);
    attribs.push_back(EGL_NONE);

    {
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglCreateContext(Native)");
      mImpl->mNativeDrawContext = eglCreateContext(display, config, mImpl->mController.GetSharedContext(), attribs.data());
      if(mImpl->mNativeDrawContext == EGL_NO_CONTEXT)
      {
        DALI_LOG_ERROR("eglCreateContext failed!\n");
        return;
      }
    }
  }

  eglMakeCurrent(display, drawSurface, readSurface, mImpl->mNativeDrawContext);

  DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "PrepareForNativeRendering");
}

void Context::ResetBufferCache()
{
  mImpl->mGlStateCache.ResetBufferCache();
  ClearUniformBufferCache();
}

void Context::ResetGLESState()
{
  mImpl->mGlStateCache.ResetTextureCache();
  mImpl->mCurrentPipeline = nullptr;

  mImpl->mCurrentIndexBufferBinding = {};

  ClearState();
  ResetBufferCache();
  ClearVertexBufferCache();
  ClearCachedNativeTexture();
  mImpl->InitializeGlState();
}

void Context::RestoreFromNativeRendering()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "RestoreFromNativeRendering");
  auto display = eglGetCurrentDisplay();

  // bring back original context
  eglMakeCurrent(display, mImpl->mCacheDrawWriteSurface, mImpl->mCacheDrawReadSurface, mImpl->mCacheEGLGraphicsContext);
}

} // namespace Dali::Graphics::GLES
