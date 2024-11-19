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

// Class header
#include <dali/internal/graphics/gles-impl/gles-sync-pool.h>

// External Headers
#include <dali/graphics-api/graphics-sync-object-create-info.h>

// Internal Headers
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gLogSyncFilter;
#endif

namespace Dali::Graphics::GLES
{
AgingSyncObject::AgingSyncObject(Graphics::EglGraphicsController& controller, const Context* writeContext, bool _egl)
: controller(controller),
  writeContext(writeContext),
  egl(_egl)
{
  if(egl)
  {
    eglSyncObject = static_cast<Internal::Adaptor::EglSyncObject*>(controller.GetEglSyncImplementation().CreateSyncObject());
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::cons; EGL::CreateSyncObject: %p\n", eglSyncObject);
  }
  else
  {
    auto gl = controller.GetGL();
    if(gl)
    {
      glSyncObject = gl->FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
  }
}

AgingSyncObject::~AgingSyncObject()
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    if(egl)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::dstr; EGL::DestroySyncObject: %p\n", eglSyncObject);
      controller.GetEglSyncImplementation().DestroySyncObject(eglSyncObject);
    }
    else
    {
      auto gl = controller.GetGL();
      if(gl && glSyncObject != nullptr)
      {
        gl->DeleteSync(glSyncObject);
      }
    }
  }
}

bool AgingSyncObject::IsSynced()
{
  bool synced = false;
  if(egl)
  {
    if(eglSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::IsSynced(); EGL::ClientWaitSync\n");
      synced = eglSyncObject->IsSynced();
    }
  }
  else
  {
    auto gl = controller.GetGL();
    if(gl && glSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::IsSynced(); glClientWaitSync 0ms\n");
      const GLuint64 TIMEOUT = 0; //0ms!
      GLenum         result  = gl->ClientWaitSync(glSyncObject, GL_SYNC_FLUSH_COMMANDS_BIT, TIMEOUT);

      synced = (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED);
    }
  }
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::IsSynced(); Result: %s\n", synced ? "Synced" : "NOT SYNCED");
  return synced;
}

bool AgingSyncObject::ClientWait()
{
  bool synced = false;
  if(egl)
  {
    if(eglSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::ClientWait(); EGL::ClientWaitSync\n");
      eglSyncObject->ClientWait();
      synced = true;
    }
  }
  else
  {
    auto gl = controller.GetGL();
    if(gl && glSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::ClientWait(); glClientWaitSync 1ms\n");
      const GLuint64 TIMEOUT = 1000000; //1ms!
      GLenum         result  = gl->ClientWaitSync(glSyncObject, GL_SYNC_FLUSH_COMMANDS_BIT, TIMEOUT);

      synced = (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED);
    }
  }
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::ClientWait(); Result: %s\n", synced ? "Synced" : "NOT SYNCED");
  return synced;
}

void AgingSyncObject::Wait()
{
  if(egl)
  {
    if(eglSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::Wait(); EGL::WaitSync\n");
      eglSyncObject->Wait();
    }
  }
  else
  {
    auto gl = controller.GetGL();
    if(gl && glSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::Wait(); glWaitSync\n");
      gl->WaitSync(glSyncObject, 0, 0ull);
    }
  }
}

SyncPool::~SyncPool() = default;

AgingSyncObject* SyncPool::AllocateSyncObject(const Context* writeContext, SyncPool::SyncContext syncContext)
{
  AgingSyncObject* agingSyncObject = new AgingSyncObject(mController, writeContext, (syncContext == SyncContext::EGL));

  // Take ownership of sync object
  mSyncObjects.PushBack(agingSyncObject);
  return agingSyncObject;
}

bool SyncPool::IsSynced(AgingSyncObject* agingSyncObject)
{
  if(DALI_LIKELY(agingSyncObject != nullptr))
  {
    return agingSyncObject->IsSynced();
  }
  return false;
}

void SyncPool::Wait(AgingSyncObject* agingSyncObject)
{
  if(DALI_LIKELY(agingSyncObject != nullptr))
  {
    agingSyncObject->syncing = true;
    agingSyncObject->Wait();
  }
}

bool SyncPool::ClientWait(AgingSyncObject* agingSyncObject)
{
  if(DALI_LIKELY(agingSyncObject != nullptr))
  {
    return agingSyncObject->ClientWait();
  }
  return false;
}

void SyncPool::FreeSyncObject(AgingSyncObject* agingSyncObject)
{
  if(DALI_LIKELY(agingSyncObject != nullptr))
  {
    // Release memory of sync object
    mSyncObjects.EraseObject(agingSyncObject);
  }
}

/**
 * Age sync objects. Call at the end of each frame.
 * When a sync object is older than 2 frames, delete it.
 */
void SyncPool::AgeSyncObjects()
{
  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgeSyncObjects: count: %d\n", mSyncObjects.Count());

  if(!mSyncObjects.IsEmpty())
  {
    // Age the remaining sync objects.
    for(auto iter = mSyncObjects.Begin(); iter != mSyncObjects.End();)
    {
      auto* agingSyncObject = (*iter);
      if(agingSyncObject != nullptr && (agingSyncObject->glSyncObject != 0 || agingSyncObject->eglSyncObject != nullptr) && agingSyncObject->age > 0)
      {
        --agingSyncObject->age;
        ++iter;
      }
      else
      {
        // Release memory of sync object
        iter = mSyncObjects.Erase(iter);
      }
    }
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgeSyncObjects: count after erase: %d\n", mSyncObjects.Count());
}

} // namespace Dali::Graphics::GLES
