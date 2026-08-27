/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include "gles-texture-dependency-checker.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>
#include <algorithm>

namespace DALI_NAMESPACE::Graphics::GLES
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogSyncFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_FENCE_SYNC");
#endif

SyncPool::SyncContext GetTextureDependencySyncContext(EglGraphicsController& controller)
{
  return controller.GetEglSyncImplementation().UseGlSyncForTextureDependencies()
           ? SyncPool::SyncContext::GL
           : SyncPool::SyncContext::EGL;
}

void FreeAllBackwardSyncObjectSyncs(EglGraphicsController& controller, TextureDependencyChecker::BackwardDependenciesMap& dependencies)
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    std::unordered_set<TextureDependencyChecker::SyncObjectId> called;
    for(auto& item : dependencies)
    {
      for(auto& backwardDependency : item.second)
      {
        auto syncId = backwardDependency.agingSyncObjectId;
        if(DALI_LIKELY(called.insert(syncId).second))
        {
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::Backward FreeSyncObject");
          controller.GetSyncPool().FreeSyncObject(syncId);
        }
      }
    }
    dependencies.clear();
  }
}
void FreeBackwardSyncObjectSyncs(EglGraphicsController& controller, TextureDependencyChecker::BackwardDependenciesMap& dependencies, const GLES::Texture* texture)
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    auto iter = dependencies.find(texture);
    if(iter != dependencies.end())
    {
      std::unordered_set<TextureDependencyChecker::SyncObjectId> called;
      for(auto& backwardDependency : iter->second)
      {
        auto syncId = backwardDependency.agingSyncObjectId;
        if(DALI_LIKELY(called.insert(syncId).second))
        {
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::Backward FreeSyncObject");
          controller.GetSyncPool().FreeSyncObject(syncId);
        }
      }
      dependencies.erase(iter);
    }
  }
}
void WaitBackwardSyncObjectSyncs(EglGraphicsController& controller, TextureDependencyChecker::BackwardDependenciesMap& dependencies, const Context* writeContext, const GLES::Texture* texture)
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    auto iter = dependencies.find(texture);
    if(iter != dependencies.end())
    {
      std::unordered_set<TextureDependencyChecker::SyncObjectId> called;
      for(auto& backwardDependency : iter->second)
      {
        if(writeContext != backwardDependency.readContext)
        {
          auto syncId = backwardDependency.agingSyncObjectId;
          if(DALI_LIKELY(called.insert(syncId).second))
          {
            DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::Backward Insert GPU WAIT");
            controller.GetSyncPool().Wait(syncId);

            // Now we can remove sync object.
            // Note that DALi use only single context for FBO case.
            // If FBO be used for multiple context, we cannot free sync object & remove backwardDependency now.
            // Also, we could call Wait(syncId) as already freed sync object. It will be ignored internall.
            controller.GetSyncPool().FreeSyncObject(syncId);
          }
        }
      }
      dependencies.erase(iter);
    }
  }
}
} // namespace

TextureDependencyChecker::~TextureDependencyChecker()
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    for(auto& forwardDependency : mForwardDependencies)
    {
      for(auto texture : forwardDependency.textures)
      {
        texture->SetDependencyIndex(0xffffffff);
      }
      mController.GetSyncPool().FreeSyncObject(forwardDependency.agingSyncObjectId);
    }
    mForwardDependencies.clear();
  }
}

void TextureDependencyChecker::Reset()
{
  // Free sync objects for FBO texture dependencies
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    for(auto& forwardDependency : mForwardDependencies)
    {
      for(auto texture : forwardDependency.textures)
      {
        texture->SetDependencyIndex(0xffffffff);
      }
      if(!forwardDependency.syncing)
      {
        mController.GetSyncPool().FreeSyncObject(forwardDependency.agingSyncObjectId);
      }
    }
  }
  mForwardDependencies.clear();

  // Reset all native texture's state as prepared
  if(DALI_UNLIKELY(!mNativeTextureDependencies.empty()))
  {
    NativeTextureSet errorTextureSet;
    for(auto& iter : mNativeTextureDependencies)
    {
      auto& textureSet = iter.second;
      for(const auto* texture : textureSet)
      {
        errorTextureSet.insert(texture);
      }
    }

    for(const auto* texture : errorTextureSet)
    {
      // Ensure to call ResetPrepare() one more time if CreateNativeTextureSync() not be called.
      const_cast<GLES::Texture*>(texture)->ResetPrepare();
      mUsedNativeTextureSet.insert(texture);
    }

    decltype(mNativeTextureDependencies)().swap(mNativeTextureDependencies); // Clear the map and free memory
  }
  if(!mUsedNativeTextureSet.empty())
  {
    for(const auto* texture : mUsedNativeTextureSet)
    {
      const_cast<GLES::Texture*>(texture)->ResetPrepare();
    }
    decltype(mUsedNativeTextureSet)().swap(mUsedNativeTextureSet); // Clear the set and free memory
  }

  // Free sync objects from previous frame's read texture dependencies
  FreeAllBackwardSyncObjectSyncs(mController, mBackwardDependencies[mPrevFrameIndex]);

  // Swap frame indices for double buffering
  mCurrentFrameIndex ^= 1;
  mPrevFrameIndex ^= 1;
}

void TextureDependencyChecker::DiscardTexture(const GLES::Texture* texture)
{
  DiscardNativeTexture(texture);

  FreeBackwardSyncObjectSyncs(mController, mBackwardDependencies[mPrevFrameIndex], texture);
  FreeBackwardSyncObjectSyncs(mController, mBackwardDependencies[mCurrentFrameIndex], texture);
}

void TextureDependencyChecker::InvalidateContext(const GLES::Context* invalidatedContext)
{
  // Invalidate forward dependencies
  // Make CheckNeedsSync() create a new sync object always.
  for(auto& forwardDependency : mForwardDependencies)
  {
    if(forwardDependency.writeContext == invalidatedContext)
    {
      forwardDependency.writeContext = nullptr;
    }
  }

  // Invalidate backward dependencies
  // Make WaitBackwardSyncObjectSyncs() wait for the sync object always.
  for(auto& backwardDependencies : mBackwardDependencies[mPrevFrameIndex])
  {
    for(auto& backwardDependency : backwardDependencies.second)
    {
      if(backwardDependency.readContext == invalidatedContext)
      {
        backwardDependency.readContext = nullptr;
      }
    }
  }
  for(auto& backwardDependencies : mBackwardDependencies[mCurrentFrameIndex])
  {
    for(auto& backwardDependency : backwardDependencies.second)
    {
      if(backwardDependency.readContext == invalidatedContext)
      {
        backwardDependency.readContext = nullptr;
      }
    }
  }

  // Invalidate native texture dependencies
  // Make CreateNativeTextureSync() "do not" create a new sync object.
  mNativeTextureDependencies.erase(invalidatedContext);
}

///< FBO write -> Surface read synchronization (Forward)

void TextureDependencyChecker::PrepareForwardDependencySync(const GLES::Context* writeContext)
{
  mForwardDependencies.emplace_back();
  auto& forwardDependency        = mForwardDependencies.back();
  forwardDependency.writeContext = const_cast<GLES::Context*>(writeContext);
}

void TextureDependencyChecker::AddTextures(const GLES::Context* writeContext, const GLES::Framebuffer* framebuffer)
{
  DALI_ASSERT_ALWAYS(mForwardDependencies.size() > 0u && "PrepareForwardDependencySync() must be called before AddTextures()");
  auto& forwardDependency = mForwardDependencies.back();
  DALI_ASSERT_ALWAYS(forwardDependency.writeContext == writeContext && "Context mismatch in AddTextures()");
  uint32_t index = static_cast<uint32_t>(mForwardDependencies.size()) - 1u;

  // Collect all textures attached to the framebuffer (color, depth, stencil)
  for(int i = 0; i < 3; ++i)
  {
    GLES::Texture* texture{nullptr};
    switch(i)
    {
      case 0:
      {
        if(!framebuffer->GetCreateInfo().colorAttachments.empty())
        {
          texture = static_cast<GLES::Texture*>(framebuffer->GetCreateInfo().colorAttachments[0].texture);
        }
        break;
      }
      case 1:
        texture = static_cast<GLES::Texture*>(framebuffer->GetCreateInfo().depthStencilAttachment.depthTexture);
        break;
      case 2:
        texture = static_cast<GLES::Texture*>(framebuffer->GetCreateInfo().depthStencilAttachment.stencilTexture);
        break;
    }
    if(texture != nullptr)
    {
      forwardDependency.textures.push_back(texture);
      texture->SetDependencyIndex(index);
    }
  }
}

void TextureDependencyChecker::AllocateForwardDependencySync(const GLES::Context* writeContext)
{
  DALI_ASSERT_ALWAYS(mForwardDependencies.size() > 0u && "PrepareForwardDependencySync() must be called before AddTextures()");
  auto& forwardDependency = mForwardDependencies.back();
  DALI_ASSERT_ALWAYS(forwardDependency.writeContext == writeContext && "Context mismatch in AllocateForwardDependencySync()");

  // Allocate sync object for cross-context synchronization
  // The shared resource context writes to FBOs, but they are drawn on separate scene contexts
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::AddTextures() Allocating sync object\n");
  forwardDependency.agingSyncObjectId = mController.GetSyncPool().AllocateSyncObject(
    writeContext,
    GetTextureDependencySyncContext(mController));

  // For native image texture
  CreateNativeTextureSync(writeContext);
}

void TextureDependencyChecker::CheckNeedsSync(const GLES::Context* readContext, const GLES::Texture* texture, bool cpu)
{
  uint32_t dependencyIndex = texture->GetDependencyIndex();
  if(dependencyIndex < mForwardDependencies.size())
  {
    auto& forwardDependency = mForwardDependencies[dependencyIndex];
    if(forwardDependency.writeContext != readContext)
    {
      // Different context is reading this texture - may need sync
      if(!forwardDependency.syncing && forwardDependency.agingSyncObjectId != INVALID_SYNC_OBJECT_ID)
      {
        forwardDependency.syncing = true;

        if(cpu)
        {
          // CPU-side wait (blocks until complete)
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert CPU WAIT");
          const bool synced = mController.GetSyncPool().ClientWait(forwardDependency.agingSyncObjectId);
          if(DALI_LIKELY(synced))
          {
            mController.GetSyncPool().FreeSyncObject(forwardDependency.agingSyncObjectId);
            forwardDependency.agingSyncObjectId = INVALID_SYNC_OBJECT_ID;
          }
        }
        else
        {
          // GPU-side wait (non-blocking from CPU perspective)
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert GPU WAIT");
          mController.GetSyncPool().Wait(forwardDependency.agingSyncObjectId);
        }
      }

      // Track this texture for Surface read -> FBO write synchronization
      mReadTextures.push_back(texture);
    }
  }
}

///< Surface read -> FBO write next frame synchronization (Backward)

bool TextureDependencyChecker::MarkFramebufferTextureRead(const Context* readContext)
{
  if(mReadTextures.empty())
  {
    return false;
  }

  // Create sync object for the read operation
  FramebufferTextureBackwardDependency backwardDependency;
  backwardDependency.readContext = readContext;

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::MarkTextureReaded() Allocating sync object\n");
  backwardDependency.agingSyncObjectId = mController.GetSyncPool().AllocateSyncObject(
    readContext,
    GetTextureDependencySyncContext(mController));

  // Record all textures that were read in this frame
  // Note that all textures share the single sync object id.
  auto&                                    currentFrameDependencies = mBackwardDependencies[mCurrentFrameIndex];
  std::unordered_set<const GLES::Texture*> called;
  for(const auto* texture : mReadTextures)
  {
    if(DALI_LIKELY(called.insert(texture).second))
    {
      currentFrameDependencies[texture].push_back(backwardDependency);
    }
  }

  mReadTextures.clear();

  return true;
}

void TextureDependencyChecker::CheckFramebufferNeedsSync(const Context* writeContext, const Framebuffer* framebuffer)
{
  // Check all attachments (color, depth, stencil) for pending read syncs
  for(int i = 0; i < 3; ++i)
  {
    GLES::Texture* texture{nullptr};
    switch(i)
    {
      case 0:
      {
        if(!framebuffer->GetCreateInfo().colorAttachments.empty())
        {
          texture = static_cast<GLES::Texture*>(framebuffer->GetCreateInfo().colorAttachments[0].texture);
        }
        break;
      }
      case 1:
        texture = static_cast<GLES::Texture*>(framebuffer->GetCreateInfo().depthStencilAttachment.depthTexture);
        break;
      case 2:
        texture = static_cast<GLES::Texture*>(framebuffer->GetCreateInfo().depthStencilAttachment.stencilTexture);
        break;
    }
    if(texture != nullptr)
    {
      // Wait on any pending read syncs from previous frames
      WaitBackwardSyncObjectSyncs(mController, mBackwardDependencies[mPrevFrameIndex], writeContext, texture);
      WaitBackwardSyncObjectSyncs(mController, mBackwardDependencies[mCurrentFrameIndex], writeContext, texture);
    }
  }
}

///< NativeTexture dependency checker

void TextureDependencyChecker::MarkNativeTexturePrepared(const Context* context, const GLES::Texture* texture)
{
  if(DALI_LIKELY(texture->IsNativeTexture()))
  {
    mNativeTextureDependencies[context].insert(texture);
  }
}

bool TextureDependencyChecker::CreateNativeTextureSync(const Context* context)
{
  bool syncCreated = false;

  auto iter = mNativeTextureDependencies.find(context);
  if(iter != mNativeTextureDependencies.end())
  {
    for(const auto* texture : iter->second)
    {
      const_cast<GLES::Texture*>(texture)->ResetPrepare();
      syncCreated = true;
      mUsedNativeTextureSet.insert(texture);
    }
    mNativeTextureDependencies.erase(iter);
  }

  return syncCreated;
}

void TextureDependencyChecker::DiscardNativeTexture(const GLES::Texture* texture)
{
  if(texture->IsNativeTexture())
  {
    for(auto& iter : mNativeTextureDependencies)
    {
      auto& textureSet = iter.second;
      auto  setIter    = textureSet.find(texture);
      if(setIter != textureSet.end())
      {
        textureSet.erase(setIter);
      }
    }
    mUsedNativeTextureSet.erase(texture);
  }
}
} //namespace DALI_NAMESPACE::Graphics::GLES
