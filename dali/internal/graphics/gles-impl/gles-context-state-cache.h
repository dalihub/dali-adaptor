#ifndef DALI_GRAPHICS_GLES_CONTEXT_STATE_CACHE_H
#define DALI_GRAPHICS_GLES_CONTEXT_STATE_CACHE_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>
#include <cstring> ///< for memset

// INTERNAL INCLUDES
#include "gles-blend-state-cache.h"
#include "gles-framebuffer-state-cache.h"

namespace Dali::Graphics
{
namespace GLES
{
namespace
{
static constexpr unsigned int MAX_TEXTURE_UNITS        = 32; // As what is defined in gl-defines.h, which is more than DALi uses anyways
static constexpr unsigned int MAX_TEXTURE_TARGET       = 4;  // We only support GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP and GL_TEXTURE_EXTERNAL_OES
static constexpr unsigned int MAX_ATTRIBUTE_CACHE_SIZE = 8;  // Size of the VertexAttributeArray enables

static constexpr uint32_t INVALID_GRAPHICS_RESOURCE_ID = -1;
} // namespace

/**
 * @brief Cache of GL State per context
 */
struct GLStateCache
{
  /**
   * Reset the cached texture ids.
   */
  void ResetTextureCache()
  {
    // reset the cached texture id as INVALID_GRAPHICS_RESOURCE_ID
    // in case the driver re-uses them when creating new textures
    memset(&mBoundTextureId, 0xff, sizeof(mBoundTextureId));
  }

  /**
   * Reset the cached buffer ids.
   */
  void ResetBufferCache()
  {
    // reset the cached buffer id's
    // fixes problem where some drivers will a generate a buffer with the
    // same id, as the last deleted buffer id.
    mBoundArrayBufferId        = INVALID_GRAPHICS_RESOURCE_ID;
    mBoundElementArrayBufferId = INVALID_GRAPHICS_RESOURCE_ID;
  }

  /**
   * @return true if next draw operation will write to depth buffer
   */
  bool DepthBufferWriteEnabled() const
  {
    return mDepthBufferEnabled && mDepthMaskEnabled;
  }

  /**
   * @return true if next draw operation will write to stencil buffer
   */
  bool StencilBufferWriteEnabled() const
  {
    return mStencilBufferEnabled && (mStencilMask > 0);
  }

  // glEnable/glDisable states
  bool   mColorMask{true};
  GLuint mStencilMask{0xFF};
  bool   mDepthBufferEnabled{false};
  bool   mDepthMaskEnabled{false};
  bool   mScissorTestEnabled{false};
  bool   mStencilBufferEnabled{false};
  bool   mClearColorSet{false};

  // glBindBuffer() state
  GLuint mBoundArrayBufferId{0u};        ///< The ID passed to glBindBuffer(GL_ARRAY_BUFFER)
  GLuint mBoundElementArrayBufferId{0u}; ///< The ID passed to glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)

  // glBindTexture() state
  GLenum mActiveTextureUnit{MAX_TEXTURE_UNITS};
  GLuint mBoundTextureId[MAX_TEXTURE_UNITS][MAX_TEXTURE_TARGET]; ///< The ID passed to glBindTexture()

  // glStencilFunc() and glStencilOp() state.
  CompareOp mStencilFunc{CompareOp::ALWAYS};
  GLuint    mStencilFuncRef{0};
  GLuint    mStencilFuncMask{0xFFFFFFFF};
  StencilOp mStencilOpFail{StencilOp::KEEP};
  StencilOp mStencilOpDepthFail{StencilOp::KEEP};
  StencilOp mStencilOpDepthPass{StencilOp::KEEP};

  CompareOp mDepthFunction{CompareOp::LESS}; /// The depth function

  Vector4 mClearColor{Color::WHITE}; ///< clear color. never used until it's been set by the user

  CullMode mCullFaceMode{CullMode::NONE}; ///< Face culling mode

  // Vertex Attribute Buffer enable caching
  bool mVertexAttributeCachedState[MAX_ATTRIBUTE_CACHE_SIZE];  ///< Value cache for Enable Vertex Attribute
  bool mVertexAttributeCurrentState[MAX_ATTRIBUTE_CACHE_SIZE]; ///< Current state on the driver for Enable Vertex Attribute

  BlendStateCache       mBlendStateCache{};       ///< blend option cache.
  FrameBufferStateCache mFrameBufferStateCache{}; ///< frame buffer state cache
};

} // namespace GLES

} // namespace Dali::Graphics

#endif // DALI_GRAPHICS_GLES_CONTEXT_STATE_CACHE_H
