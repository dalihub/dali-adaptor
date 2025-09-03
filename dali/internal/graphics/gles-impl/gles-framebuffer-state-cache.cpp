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

// CLASS HEADER
#include "gles-framebuffer-state-cache.h"

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

namespace Dali::Graphics
{
namespace GLES
{
namespace
{
const uint32_t INITIAL_FRAMEBUFFER_STATE = 0u;
}

FrameBufferStateCache::FrameBufferStateCache()
{
}

FrameBufferStateCache::~FrameBufferStateCache() = default;

GLbitfield FrameBufferStateCache::GetClearMask(GLbitfield mask, bool forceClear, bool scissorTestEnabled)
{
  if(scissorTestEnabled)
  {
    // don't do anything if scissor test is enabled, in the future we could
    // potentially keep track of frame buffer size vs scissor test size to see if the entire
    // buffer is cleared or not.
    return mask;
  }

  FrameBufferState* state = GetFrameBufferState(mCurrentFrameBufferId);
  if(!state)
  {
    DALI_LOG_ERROR("FrameBuffer not found %d \n", mCurrentFrameBufferId);
    return mask;
  }

  // if we are forcing the clear operation, then just update the internal cached values
  if(forceClear)
  {
    SetClearState(state, mask);
    return mask;
  }

  // use the cached values
  if(mask & GL_COLOR_BUFFER_BIT)
  {
    // check if color buffer is currently clean
    if(state->mState & COLOR_BUFFER_CLEAN)
    {
      // remove clear color buffer flag from bitmask, no need to clear twice
      mask &= ~GL_COLOR_BUFFER_BIT;
    }
  }
  if(mask & GL_DEPTH_BUFFER_BIT)
  {
    // check if depth buffer is currently clean
    if(state->mState & DEPTH_BUFFER_CLEAN)
    {
      // remove clear depth buffer flag from bitmask, no need to clear twice
      mask &= ~GL_DEPTH_BUFFER_BIT;
    }
  }
  if(mask & GL_STENCIL_BUFFER_BIT)
  {
    // check if stencil buffer is currently clean
    if(state->mState & STENCIL_BUFFER_CLEAN)
    {
      // remove clear stencil buffer flag from bitmask, no need to clear twice

      mask &= ~GL_STENCIL_BUFFER_BIT;
    }
  }

  // set the clear state based, what's about to be cleared
  SetClearState(state, mask);

  return mask;
}

void FrameBufferStateCache::SetCurrentFrameBuffer(const GLuint framebufferId)
{
  mCurrentFrameBufferId = framebufferId;
}

GLuint FrameBufferStateCache::GetCurrentFrameBuffer() const
{
  return mCurrentFrameBufferId;
}

void FrameBufferStateCache::FrameBufferCreated(const GLuint framebufferId)
{
  // check the frame buffer doesn't exist already
  FrameBufferState* state = GetFrameBufferState(framebufferId);
  if(state)
  {
    DALI_LOG_ERROR("FrameBuffer already exists%d \n", framebufferId);
    // reset its state
    state->mState = INITIAL_FRAMEBUFFER_STATE;
    return;
  }

  FrameBufferState newFrameBuffer(framebufferId);
  mFrameBufferStates.PushBack(newFrameBuffer);
}

void FrameBufferStateCache::FrameBufferDeleted(const GLuint framebufferId)
{
  FrameBufferStateVector::Iterator iter    = mFrameBufferStates.Begin();
  FrameBufferStateVector::Iterator endIter = mFrameBufferStates.End();

  for(; iter != endIter; ++iter)
  {
    if((*iter).mId == framebufferId)
    {
      mFrameBufferStates.Erase(iter);
      return;
    }
  }
  DALI_LOG_ERROR("FrameBuffer not found %d \n", framebufferId);
}

void FrameBufferStateCache::DrawOperation(bool colorBuffer, bool depthBuffer, bool stencilBuffer)
{
  FrameBufferState* state = GetFrameBufferState(mCurrentFrameBufferId);
  if(!state)
  {
    // an error will have already been logged by the clear operation
    return;
  }

  if(colorBuffer)
  {
    // un-set the clean bit
    state->mState &= ~COLOR_BUFFER_CLEAN;
  }
  if(depthBuffer)
  {
    // un-set the clean bit
    state->mState &= ~DEPTH_BUFFER_CLEAN;
  }
  if(stencilBuffer)
  {
    // un-set the clean bit
    state->mState &= ~STENCIL_BUFFER_CLEAN;
  }
}

void FrameBufferStateCache::Reset()
{
  mFrameBufferStates.Clear();

  // create the default frame buffer
  FrameBufferCreated(0);
}

void FrameBufferStateCache::SetClearState(FrameBufferState* state, GLbitfield mask)
{
  if(mask & GL_COLOR_BUFFER_BIT)
  {
    // set the color buffer to clean
    state->mState |= COLOR_BUFFER_CLEAN;
  }
  if(mask & GL_DEPTH_BUFFER_BIT)
  {
    // set the depth buffer to clean
    state->mState |= DEPTH_BUFFER_CLEAN;
  }
  if(mask & GL_STENCIL_BUFFER_BIT)
  {
    // set the stencil buffer to clean
    state->mState |= STENCIL_BUFFER_CLEAN;
  }
}

FrameBufferStateCache::FrameBufferState* FrameBufferStateCache::GetFrameBufferState(GLuint framebufferId)
{
  for(FrameBufferStateVector::SizeType i = 0; i < mFrameBufferStates.Count(); ++i)
  {
    FrameBufferState& state = mFrameBufferStates[i];
    if(state.mId == framebufferId)
    {
      return &state;
    }
  }
  return nullptr;
}

} // namespace GLES

} // namespace Dali::Graphics
