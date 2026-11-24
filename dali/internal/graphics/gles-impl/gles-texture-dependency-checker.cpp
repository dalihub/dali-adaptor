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

// Forward sync uses this fence type; backward sync uses the same choice.
SyncPool::SyncContext GetTextureDependencySyncContext(EglGraphicsController& controller)
{
  return controller.GetEglSyncImplementation().UseGlSyncForTextureDependencies()
           ? SyncPool::SyncContext::GL
           : SyncPool::SyncContext::EGL;
}

// Remove pending reads of one texture using its reverse context index.
void RemoveTextureFromPendingReads(TextureDependencyChecker::ReadTexturesByContext& readTexturesByContext, TextureDependencyChecker::ReadContextsByTexture& readContextsByTexture, const Texture* texture)
{
  auto readContexts = readContextsByTexture.find(texture);
  if(readContexts == readContextsByTexture.end())
  {
    return;
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::RemoveTextureFromPendingReads texture=%p contexts=%zu\n", texture, readContexts->second.size());
  for(const auto* context : readContexts->second)
  {
    auto readTextures = readTexturesByContext.find(context);
    if(readTextures != readTexturesByContext.end())
    {
      readTextures->second.erase(texture);
      if(readTextures->second.empty())
      {
        readTexturesByContext.erase(readTextures);
      }
    }
  }
  readContextsByTexture.erase(readContexts);
}

// Remove a context's pending reads from both directions of the index.
void RemoveContextFromPendingReads(TextureDependencyChecker::ReadTexturesByContext& readTexturesByContext, TextureDependencyChecker::ReadContextsByTexture& readContextsByTexture, const Context* context)
{
  auto readTextures = readTexturesByContext.find(context);
  if(readTextures == readTexturesByContext.end())
  {
    return;
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::RemoveContextFromPendingReads context=%p textures=%zu\n", context, readTextures->second.size());
  for(const auto* texture : readTextures->second)
  {
    auto readContexts = readContextsByTexture.find(texture);
    if(readContexts != readContextsByTexture.end())
    {
      readContexts->second.erase(context);
      if(readContexts->second.empty())
      {
        readContextsByTexture.erase(readContexts);
      }
    }
  }
  readTexturesByContext.erase(readTextures);
}

// Remove a backward texture reference and report whether it was the last one for its fence.
bool ReleaseBackwardSyncReference(TextureDependencyChecker::BackwardSyncReferenceCounts& referenceCounts, TextureDependencyChecker::SyncObjectId syncId)
{
  if(syncId == TextureDependencyChecker::INVALID_SYNC_OBJECT_ID)
  {
    return false;
  }

  auto count = referenceCounts.find(syncId);
  DALI_ASSERT_DEBUG(count != referenceCounts.end() && count->second > 0u);
  if(DALI_UNLIKELY(count == referenceCounts.end() || count->second == 0u))
  {
    return false;
  }

  const size_t remainingReferences = --count->second;
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::ReleaseBackwardSyncReference syncId=%u remaining=%zu\n", syncId, remainingReferences);
  if(remainingReferences == 0u)
  {
    referenceCounts.erase(count);
    return true;
  }
  return false;
}

// Retire the previous frame's backward dependencies and their fences.
void FreeAllBackwardSyncObjectSyncs(EglGraphicsController& controller, TextureDependencyChecker::BackwardDependenciesMap& dependencies, TextureDependencyChecker::BackwardSyncReferenceCounts& referenceCounts)
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    for(auto& item : dependencies)
    {
      for(auto& backwardDependency : item.second)
      {
        auto syncId = backwardDependency.agingSyncObjectId;
        if(ReleaseBackwardSyncReference(referenceCounts, syncId))
        {
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::FreeAllBackwardSyncObjectSyncs syncId=%u\n", syncId);
          controller.GetSyncPool().FreeSyncObject(syncId);
        }
      }
    }
    dependencies.clear();
  }
}

// Discard one texture's backward dependencies without freeing shared fences early.
void FreeBackwardSyncObjectSyncs(EglGraphicsController& controller, TextureDependencyChecker::BackwardDependenciesMap (&dependencies)[2], TextureDependencyChecker::BackwardSyncReferenceCounts& referenceCounts, const Texture* texture)
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    for(auto& frameDependencies : dependencies)
    {
      auto iter = frameDependencies.find(texture);
      if(iter != frameDependencies.end())
      {
        for(const auto& dependency : iter->second)
        {
          const auto syncId = dependency.agingSyncObjectId;
          if(ReleaseBackwardSyncReference(referenceCounts, syncId))
          {
            DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::FreeBackwardSyncObjectSyncs texture=%p syncId=%u\n", texture, syncId);
            controller.GetSyncPool().FreeSyncObject(syncId);
          }
        }
        frameDependencies.erase(iter);
      }
    }
  }
}

// Insert a shared-context wait for each distinct backward fence on this texture.
void WaitBackwardSyncObjectSyncs(EglGraphicsController& controller, TextureDependencyChecker::BackwardDependenciesMap& dependencies, TextureDependencyChecker::BackwardSyncReferenceCounts& referenceCounts, const Context* writeContext, const GLES::Texture* texture)
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
        if(writeContext != backwardDependency.readContext)
        {
          if(DALI_LIKELY(called.insert(syncId).second))
          {
            DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::Backward Insert GPU WAIT syncId=%u texture=%p readContext=%p writeContext=%p\n", syncId, texture, backwardDependency.readContext, writeContext);
            controller.GetSyncPool().Wait(syncId);

            // DevNote: All FBO writes use the single shared resource context. A GPU wait
            // in that context orders subsequent FBO writes, so one wait per backward
            // fence is sufficient even when other textures still reference it.
            // If FBO writes ever use multiple contexts, track waits per writer context
            // and retain the fence until every required context has inserted its wait.
            // A later Wait(syncId) on this shared context is ignored after the free.
            controller.GetSyncPool().FreeSyncObject(syncId);
          }
        }
        ReleaseBackwardSyncReference(referenceCounts, syncId);
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
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::Reset() syncId=%u writeContext=%p textures=%zu Ignored since nobody is waiting\n", forwardDependency.agingSyncObjectId, forwardDependency.writeContext, forwardDependency.textures.size());
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
  FreeAllBackwardSyncObjectSyncs(mController, mBackwardDependencies[mPrevFrameIndex], mBackwardSyncReferenceCounts);

  // Swap frame indices for double buffering
  mCurrentFrameIndex ^= 1;
  mPrevFrameIndex ^= 1;
}

void TextureDependencyChecker::DiscardTexture(const GLES::Texture* texture)
{
  DiscardNativeTexture(texture);
  RemoveTextureFromPendingReads(mReadTextures, mReadContextsByTexture, texture);
  FreeBackwardSyncObjectSyncs(mController, mBackwardDependencies, mBackwardSyncReferenceCounts, texture);
}

void TextureDependencyChecker::InvalidateContext(const GLES::Context* invalidatedContext)
{
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::InvalidateContext invalidatedContext=%p\n", invalidatedContext);

  RemoveContextFromPendingReads(mReadTextures, mReadContextsByTexture, invalidatedContext);

  // Invalidate forward dependencies
  // Make CheckNeedsSync() create a new sync object always.
  for(auto& forwardDependency : mForwardDependencies)
  {
    forwardDependency.waitingContexts.erase(invalidatedContext);
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
  forwardDependency.agingSyncObjectId = mController.GetSyncPool().AllocateSyncObject(
    writeContext,
    GetTextureDependencySyncContext(mController));
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::AllocateForwardDependencySync syncId=%u writeContext=%p textures=%zu\n", forwardDependency.agingSyncObjectId, writeContext, forwardDependency.textures.size());

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
      if(forwardDependency.agingSyncObjectId != INVALID_SYNC_OBJECT_ID)
      {
        if(cpu)
        {
          // CPU-side wait (blocks until complete)
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert CPU WAIT syncId=%u texture=%p readContext=%p writeContext=%p\n", forwardDependency.agingSyncObjectId, texture, readContext, forwardDependency.writeContext);
          const bool synced = mController.GetSyncPool().ClientWait(forwardDependency.agingSyncObjectId);
          if(DALI_LIKELY(synced))
          {
            mController.GetSyncPool().FreeSyncObject(forwardDependency.agingSyncObjectId);
            forwardDependency.agingSyncObjectId = INVALID_SYNC_OBJECT_ID;
          }
        }
        else if(forwardDependency.waitingContexts.insert(readContext).second)
        {
          // GPU-side wait (non-blocking from CPU perspective)
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert GPU WAIT syncId=%u texture=%p readContext=%p writeContext=%p\n", forwardDependency.agingSyncObjectId, texture, readContext, forwardDependency.writeContext);
          mController.GetSyncPool().Wait(forwardDependency.agingSyncObjectId);
          forwardDependency.syncing = true;
        }
      }

      // Track this texture for Surface read -> FBO write synchronization
      if(mReadTextures[readContext].insert(texture).second)
      {
        mReadContextsByTexture[texture].insert(readContext);
      }
    }
  }
}

///< Surface read -> FBO write next frame synchronization (Backward)

bool TextureDependencyChecker::MarkFramebufferTextureRead(const Context* readContext)
{
  auto readTextures = mReadTextures.find(readContext);
  if(readTextures == mReadTextures.end())
  {
    return false;
  }

  // Create sync object for the read operation
  FramebufferTextureBackwardDependency backwardDependency;
  backwardDependency.readContext = readContext;

  backwardDependency.agingSyncObjectId = mController.GetSyncPool().AllocateSyncObject(
    readContext,
    GetTextureDependencySyncContext(mController));
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::MarkFramebufferTextureRead syncId=%u readContext=%p textures=%zu\n", backwardDependency.agingSyncObjectId, readContext, readTextures->second.size());

  // Each texture occurs once in this context's pending read set. A texture read
  // by another context has a separate dependency with that context's sync object.
  auto& currentFrameDependencies = mBackwardDependencies[mCurrentFrameIndex];
  for(const auto* texture : readTextures->second)
  {
    currentFrameDependencies[texture].push_back(backwardDependency);

    auto readContexts = mReadContextsByTexture.find(texture);
    DALI_ASSERT_DEBUG(readContexts != mReadContextsByTexture.end());
    if(DALI_LIKELY(readContexts != mReadContextsByTexture.end()))
    {
      readContexts->second.erase(readContext);
      if(readContexts->second.empty())
      {
        mReadContextsByTexture.erase(readContexts);
      }
    }
  }

  if(backwardDependency.agingSyncObjectId != INVALID_SYNC_OBJECT_ID)
  {
    mBackwardSyncReferenceCounts[backwardDependency.agingSyncObjectId] += readTextures->second.size();
  }

  mReadTextures.erase(readTextures);

  return true;
}

void TextureDependencyChecker::CheckFramebufferNeedsSync(const Context* writeContext, const Framebuffer* framebuffer)
{
  // DevNote: writeContext must be the single shared resource context for every
  // FBO write. If this changes, backward waits must be tracked per writer context.
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
      WaitBackwardSyncObjectSyncs(mController, mBackwardDependencies[mPrevFrameIndex], mBackwardSyncReferenceCounts, writeContext, texture);
      WaitBackwardSyncObjectSyncs(mController, mBackwardDependencies[mCurrentFrameIndex], mBackwardSyncReferenceCounts, writeContext, texture);
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
