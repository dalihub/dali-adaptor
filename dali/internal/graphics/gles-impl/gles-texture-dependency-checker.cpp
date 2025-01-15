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
#include "gles-texture-dependency-checker.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gLogSyncFilter;
#endif

namespace Dali::Graphics::GLES
{
TextureDependencyChecker::~TextureDependencyChecker()
{
  if(DALI_LIKELY(!Dali::Graphics::EglGraphicsController::IsShuttingDown()))
  {
    for(auto& textureDependency : mFramebufferTextureDependencies)
    {
      for(auto texture : textureDependency.textures)
      {
        texture->SetDependencyIndex(0xffffffff);
      }
      mController.GetSyncPool().FreeSyncObject(textureDependency.agingSyncObjectId);
    }
    mFramebufferTextureDependencies.clear();

    for(uint32_t nativeIndex = 0u; nativeIndex < 2u; ++nativeIndex)
    {
      for(auto& nativeTextureDependency : mNativeTextureDependencies[nativeIndex])
      {
        mController.GetSyncPool().FreeSyncObject(nativeTextureDependency.agingSyncObjectId);
      }
      mNativeTextureDependencies[nativeIndex].clear();
    }
  }
}
void TextureDependencyChecker::Reset()
{
  for(auto& textureDependency : mFramebufferTextureDependencies)
  {
    for(auto texture : textureDependency.textures)
    {
      texture->SetDependencyIndex(0xffffffff);
    }
    if(!textureDependency.syncing)
    {
      mController.GetSyncPool().FreeSyncObject(textureDependency.agingSyncObjectId);
    }
  }
  mFramebufferTextureDependencies.clear();

  if(!mNativeTextureDependencies[0].empty() || !mNativeTextureDependencies[1].empty())
  {
    DALI_ASSERT_ALWAYS(mIsFirstPreparedNativeTextureDependency && "MarkNativeTextureSyncContext should be called before PostRender!");

    // Remove all infomations about previous native textures
    for(auto& nativeTextureDependency : mNativeTextureDependencies[mPreviousNativeTextureDependencyIndex])
    {
      mController.GetSyncPool().FreeSyncObject(nativeTextureDependency.agingSyncObjectId);
    }
    mNativeTextureDependencies[mPreviousNativeTextureDependencyIndex].clear();

    // Reset all native texture's state as prepared.
    // TODO : Is their any more good place to call this logic?
    for(auto& nativeTextureDependency : mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex])
    {
      for(auto& texture : nativeTextureDependency.textures)
      {
        const_cast<GLES::Texture*>(texture)->ResetPrepare();
      }
    }

    mCurrentNativeTextureDependencyIndex = __sync_fetch_and_xor(&mPreviousNativeTextureDependencyIndex, 1);
  }
}

void TextureDependencyChecker::AddTextures(const GLES::Context* writeContext, const GLES::Framebuffer* framebuffer)
{
  uint32_t index = mFramebufferTextureDependencies.size();
  mFramebufferTextureDependencies.emplace_back();
  auto& textureDependency = mFramebufferTextureDependencies.back();

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
      textureDependency.textures.push_back(texture);
      texture->SetDependencyIndex(index);
    }
  }
  textureDependency.writeContext = const_cast<GLES::Context*>(writeContext);
  textureDependency.framebuffer  = const_cast<GLES::Framebuffer*>(framebuffer);

  // We have to check on different EGL contexts: The shared resource context is used to write to fbos,
  // but they are usually drawn onto separate scene context.
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::AddTextures() Allocating sync object\n");
  textureDependency.agingSyncObjectId = mController.GetSyncPool().AllocateSyncObject(writeContext, SyncPool::SyncContext::EGL);
}

void TextureDependencyChecker::CheckNeedsSync(const GLES::Context* readContext, const GLES::Texture* texture, bool cpu)
{
  uint32_t dependencyIndex = texture->GetDependencyIndex();
  if(dependencyIndex < mFramebufferTextureDependencies.size())
  {
    auto& textureDependency = mFramebufferTextureDependencies[dependencyIndex];
    if(!textureDependency.syncing && textureDependency.writeContext != readContext && textureDependency.agingSyncObjectId != INVALID_SYNC_OBJECT_ID)
    {
      // Needs syncing!
      textureDependency.syncing = true;

      if(cpu)
      {
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert CPU WAIT");
        const bool synced = mController.GetSyncPool().ClientWait(textureDependency.agingSyncObjectId);
        if(DALI_LIKELY(synced))
        {
          // Object discarded, and will be free when write context be currnt.
          mController.GetSyncPool().FreeSyncObject(textureDependency.agingSyncObjectId);
          textureDependency.agingSyncObjectId = INVALID_SYNC_OBJECT_ID;
        }
      }
      else
      {
        // Wait on the sync object in GPU. This will ensure that the writeContext completes its tasks prior
        // to the sync point.
        // However, this may instead timeout, and we can't tell the difference (at least, for glFenceSync)
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert GPU WAIT");
        mController.GetSyncPool().Wait(textureDependency.agingSyncObjectId);
      }
    }
  }

  // Native dependency check
  if(texture->IsNativeTexture())
  {
    // TODO : Optimize here. For now, we don't have too much EndPass call. So just keep this logic.
    for(auto& nativeTextureDependency : mNativeTextureDependencies[mPreviousNativeTextureDependencyIndex])
    {
      if(nativeTextureDependency.synced || nativeTextureDependency.agingSyncObjectId == INVALID_SYNC_OBJECT_ID)
      {
        // Fast-out if we know it is already synced, or if there's no sync object.
        continue;
      }

      auto iter = nativeTextureDependency.textures.find(texture);
      if(iter != nativeTextureDependency.textures.end())
      {
        if(cpu)
        {
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync (for native) Insert CPU WAIT");
          nativeTextureDependency.synced = mController.GetSyncPool().ClientWait(nativeTextureDependency.agingSyncObjectId);

          if(DALI_LIKELY(nativeTextureDependency.synced))
          {
            // Object discarded, and will be free when write context be currnt.
            mController.GetSyncPool().FreeSyncObject(nativeTextureDependency.agingSyncObjectId);
            nativeTextureDependency.agingSyncObjectId = INVALID_SYNC_OBJECT_ID;
          }
        }
        else
        {
          // Wait on the sync object in GPU. This will ensure that the writeContext completes its tasks prior
          // to the sync point.
          // However, this may instead timeout, and we can't tell the difference (at least, for glFenceSync)
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync (for native) Insert GPU WAIT");
          mController.GetSyncPool().Wait(nativeTextureDependency.agingSyncObjectId);
        }

        nativeTextureDependency.textures.erase(iter);
      }
    }
  }
}

void TextureDependencyChecker::MarkNativeTexturePrepared(const GLES::Texture* texture)
{
  if(DALI_LIKELY(texture->IsNativeTexture()))
  {
    if(mIsFirstPreparedNativeTextureDependency)
    {
      mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex].emplace_back();
      mIsFirstPreparedNativeTextureDependency = false;
    }

    if(DALI_LIKELY(!mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex].empty()))
    {
      auto& nativeTextureDependency = mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex].back();
      nativeTextureDependency.textures.insert(texture);
    }
  }
}

void TextureDependencyChecker::DiscardNativeTexture(const GLES::Texture* texture)
{
  if(DALI_LIKELY(texture->IsNativeTexture()))
  {
    for(uint32_t nativeIndex = 0u; nativeIndex < 2u; ++nativeIndex)
    {
      for(auto iter = mNativeTextureDependencies[nativeIndex].begin(); iter != mNativeTextureDependencies[nativeIndex].end();)
      {
        auto& nativeTextureDependency = *iter;

        bool isErased = false;

        auto jter = nativeTextureDependency.textures.find(texture);
        if(jter != nativeTextureDependency.textures.end())
        {
          nativeTextureDependency.textures.erase(jter);
          if(nativeTextureDependency.textures.empty())
          {
            mController.GetSyncPool().FreeSyncObject(nativeTextureDependency.agingSyncObjectId);
            iter = mNativeTextureDependencies[nativeIndex].erase(iter);

            isErased = true;
          }
        }

        if(!isErased)
        {
          ++iter;
        }
      }
    }
  }
}

void TextureDependencyChecker::MarkNativeTextureSyncContext(const GLES::Context* writeContext)
{
  if(mIsFirstPreparedNativeTextureDependency)
  {
    return;
  }

  // Reset flag
  mIsFirstPreparedNativeTextureDependency = true;

  if(DALI_LIKELY(!mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex].empty()))
  {
    auto& nativeTextureDependency        = mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex].back();
    nativeTextureDependency.writeContext = writeContext; // Store write context
  }
}

void TextureDependencyChecker::CreateNativeTextureSync(const GLES::Context* currentContext)
{
  // TODO : Optimize here. For now, we don't have too much EndPass call. So just keep this logic.
  for(auto& nativeTextureDependency : mNativeTextureDependencies[mCurrentNativeTextureDependencyIndex])
  {
    if(nativeTextureDependency.writeContext == currentContext && nativeTextureDependency.agingSyncObjectId == INVALID_SYNC_OBJECT_ID)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CreateNativeTextureSync(%p) Allocating sync object\n", currentContext);
      nativeTextureDependency.agingSyncObjectId = mController.GetSyncPool().AllocateSyncObject(currentContext, SyncPool::SyncContext::EGL);
    }
  }
}

} // namespace Dali::Graphics::GLES
