/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// Class header
#include <dali/internal/graphics/gles-impl/gles-sync-pool.h>

// External Headers
#include <dali/graphics-api/graphics-sync-object-create-info.h>

// Internal Headers
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>

namespace Dali::Graphics::GLES
{
AgingSyncObject::AgingSyncObject(Graphics::EglGraphicsController& controller, const Context* writeContext)
: controller(controller),
  writeContext(writeContext)
{
  auto gl = controller.GetGL();
  if(gl)
  {
    glSyncObject = gl->FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }
}

AgingSyncObject::~AgingSyncObject()
{
  auto gl = controller.GetGL();
  if(gl && glSyncObject != nullptr)
  {
    gl->DeleteSync(glSyncObject);
  }
}

SyncPool::~SyncPool() = default;

AgingSyncObject* SyncPool::AllocateSyncObject(const Context* writeContext)
{
  std::unique_ptr<AgingSyncObject> syncObject = std::make_unique<AgingSyncObject>(mController, writeContext);
  mSyncObjects.push_back(std::move(syncObject));
  return mSyncObjects.back().get();
}

void SyncPool::Wait(AgingSyncObject* syncPoolObject)
{
  auto gl = mController.GetGL();
  if(gl && syncPoolObject->glSyncObject != nullptr)
  {
    syncPoolObject->syncing = true;
    gl->WaitSync(syncPoolObject->glSyncObject, 0, GL_TIMEOUT_IGNORED);
  }
}

void SyncPool::FreeSyncObject(AgingSyncObject* agingSyncObject)
{
  auto iter = std::find_if(mSyncObjects.begin(), mSyncObjects.end(), [&agingSyncObject](AgingSyncPtrRef agingSyncPtr) { return agingSyncPtr.get() == agingSyncObject; });
  if(iter != mSyncObjects.end())
  {
    iter->reset();
  }
}

/**
 * Age sync objects. Call at the end of each frame.
 * When a sync object is older than 2 frames, delete it.
 */
void SyncPool::AgeSyncObjects()
{
  if(!mSyncObjects.empty())
  {
    // Age the remaining sync objects.
    for(auto& agingSyncObject : mSyncObjects)
    {
      if(agingSyncObject != nullptr && agingSyncObject->glSyncObject != 0)
      {
        if(agingSyncObject->age > 0)
        {
          agingSyncObject->age--;
        }
        else
        {
          agingSyncObject.reset();
        }
      }
    }
  }
  // Move any old sync objects to the end of the list, and then remove them all.
  mSyncObjects.erase(std::remove_if(mSyncObjects.begin(), mSyncObjects.end(), [&](std::unique_ptr<AgingSyncObject>& agingSyncObject) { return agingSyncObject == nullptr; }),
                     mSyncObjects.end());
}

} // namespace Dali::Graphics::GLES
