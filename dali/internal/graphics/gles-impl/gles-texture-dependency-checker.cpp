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
void TextureDependencyChecker::Reset()
{
  for(auto& textureDependency : mTextureDependencies)
  {
    for(auto texture : textureDependency.textures)
    {
      texture->SetDependencyIndex(0xffffffff);
    }
    if(!textureDependency.syncing)
    {
      mController.GetSyncPool().FreeSyncObject(textureDependency.agingSyncObject);
    }
  }
  mTextureDependencies.clear();
}

void TextureDependencyChecker::AddTextures(const GLES::Context* writeContext, const GLES::Framebuffer* framebuffer)
{
  uint32_t index = mTextureDependencies.size();
  mTextureDependencies.emplace_back();
  TextureDependency& textureDependency = mTextureDependencies.back();

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
  textureDependency.agingSyncObject = mController.GetSyncPool().AllocateSyncObject(writeContext, SyncPool::SyncContext::EGL);
}

void TextureDependencyChecker::CheckNeedsSync(const GLES::Context* readContext, const GLES::Texture* texture, bool cpu)
{
  uint32_t dependencyIndex = texture->GetDependencyIndex();
  if(dependencyIndex < mTextureDependencies.size())
  {
    auto& textureDependency = mTextureDependencies[dependencyIndex];
    if(!textureDependency.syncing && textureDependency.writeContext != readContext)
    {
      // Needs syncing!
      textureDependency.syncing = true;

      if(cpu)
      {
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert CPU WAIT");
        mController.GetSyncPool().ClientWait(textureDependency.agingSyncObject);
      }
      else
      {
        // Wait on the sync object in GPU. This will ensure that the writeContext completes its tasks prior
        // to the sync point.
        // However, this may instead timeout, and we can't tell the difference (at least, for glFenceSync)
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "TextureDependencyChecker::CheckNeedsSync Insert GPU WAIT");
        mController.GetSyncPool().Wait(textureDependency.agingSyncObject);
      }
    }
  }
}

} // namespace Dali::Graphics::GLES
