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

// Class header
#include <dali/internal/graphics/gles-impl/gles-sync-pool.h>

// External Headers
#include <dali/devel-api/threading/mutex.h>
#include <dali/graphics-api/graphics-sync-object-create-info.h>

// Internal Headers
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gLogSyncFilter;
#endif

namespace Dali::Graphics::GLES
{

struct SyncPool::SharedSyncObject::Impl
{
  Impl(Graphics::EglGraphicsController& controller, const Context* _context, uint32_t _frameCount, bool useNativeFenceFd)
  : controller(controller),
    context(_context),
    frameCount(_frameCount)
  {
    eglSyncObject = static_cast<Internal::Adaptor::EglSyncObject*>(controller.GetEglSyncImplementation().CreateSyncObject(Integration::GraphicsSyncAbstraction::SyncObject::SyncType::NATIVE_FENCE_SYNC));

    if(DALI_LIKELY(eglSyncObject) && useNativeFenceFd)
    {
      fenceFd = eglSyncObject->DuplicateNativeFenceFD();

      if(fenceFd != -1)
      {
        // Destroy the egl sync object. We don't need it.
        controller.GetEglSyncImplementation().DestroySyncObject(eglSyncObject);
        eglSyncObject = nullptr;
      }
    }
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "SyncPool::SharedSyncObject::Impl: [%d] [%p]\n", fenceFd, this);
  }

  ~Impl()
  {
    if(fenceFd != -1)
    {
      Internal::Adaptor::NativeFence::CloseFD(fenceFd);
    }

    if(eglSyncObject)
    {
      controller.GetEglSyncImplementation().DestroySyncObject(eglSyncObject);
    }

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "SyncPool::SharedSyncObject::~Impl: [%p]\n", this);
  }

  uint8_t UpdateAge()
  {
    Dali::Mutex::ScopedLock lock(mutex);
    uint8_t                 oldAge = age;
    if(age > 0)
    {
      age--;
    }
    if(oldAge == 0)
    {
      synced = true;

      if(fenceFd != -1)
      {
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Set synced [%d]\n", fenceFd);

        Internal::Adaptor::NativeFence::CloseFD(fenceFd);
        fenceFd = -1;
      }
    }
    return oldAge;
  }

  bool ClientWait()
  {
    Dali::Mutex::ScopedLock lock(mutex);

    if(synced)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already synced [%p]\n", this);
      return true;
    }

    if(eglSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "ClientWait() [%p]\n", this);
      eglSyncObject->ClientWait();
      synced = true;

      controller.GetEglSyncImplementation().DestroySyncObject(eglSyncObject);
      eglSyncObject = nullptr;
    }
    else
    {
      Poll();
    }

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "ClientWait(); Result: %s\n", synced ? "Synced" : "NOT SYNCED");

    return synced;
  }

  bool Poll()
  {
    Dali::Mutex::ScopedLock lock(mutex);

    if(synced)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already synced [%d]\n", fenceFd);
      return true;
    }
    else
    {
      if(fenceFd != -1)
      {
        DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::ClientWait(); Poll [%d]\n", fenceFd);

        synced  = Internal::Adaptor::NativeFence::PollFD(fenceFd);
        fenceFd = -1;
      }
      else
      {
        DALI_LOG_ERROR("Something wrong! [%p]\n", this);
      }
    }

    return synced;
  }

  bool IsFenceFdSupported()
  {
    return fenceFd == -1 ? false : true;
  }

  EglGraphicsController&            controller;
  const Context*                    context{nullptr};
  Internal::Adaptor::EglSyncObject* eglSyncObject{nullptr};
  Mutex                             mutex;

  uint32_t frameCount{0u};
  int32_t  fenceFd{-1};
  uint8_t  age{3u};
  bool     synced{false};
};

SyncPool::SharedSyncObject::SharedSyncObject(Graphics::EglGraphicsController& controller, const Context* context, uint32_t frameCount, bool useNativeFenceFd)
{
  mImpl = std::unique_ptr<Impl>(new Impl(controller, context, frameCount, useNativeFenceFd));
}

SyncPool::SharedSyncObject::~SharedSyncObject() = default;

bool SyncPool::SharedSyncObject::Poll()
{
  return mImpl->Poll();
}

bool SyncPool::SharedSyncObject::ClientWait()
{
  return mImpl->ClientWait();
}

bool SyncPool::SharedSyncObject::IsFenceFdSupported()
{
  return mImpl->IsFenceFdSupported();
}

uint8_t SyncPool::SharedSyncObject::UpdateAge()
{
  return mImpl->UpdateAge();
}

bool SyncPool::SharedSyncObject::Match(const Context* context, uint32_t frameCount, bool useNativeFenceFd)
{
  if(mImpl->context == context && mImpl->frameCount == frameCount && useNativeFenceFd)
  {
    return true;
  }
  return false;
}

SyncPool::AgingSyncObject::AgingSyncObject(Graphics::EglGraphicsController& controller, const Context* writeContext, bool _egl)
: controller(controller),
  writeContext(writeContext),
  egl(_egl)
{
  if(egl)
  {
    eglSyncObject = static_cast<Internal::Adaptor::EglSyncObject*>(controller.GetEglSyncImplementation().CreateSyncObject(Integration::GraphicsSyncAbstraction::SyncObject::SyncType::FENCE_SYNC));
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::cons; EGL::CreateSyncObject: %p\n", eglSyncObject);
  }
  else
  {
    auto* gl = controller.GetGL();
    if(DALI_LIKELY(gl))
    {
      glSyncObject = gl->FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
  }
}

SyncPool::AgingSyncObject::~AgingSyncObject()
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
      auto* gl = controller.GetGL();
      if(DALI_LIKELY(gl) && glSyncObject != nullptr)
      {
        gl->DeleteSync(glSyncObject);
      }
    }
  }
}

bool SyncPool::AgingSyncObject::ClientWait()
{
  if(synced)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already synced\n");
    return true;
  }

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
    auto* gl = controller.GetGL();
    if(DALI_LIKELY(gl) && glSyncObject)
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

void SyncPool::AgingSyncObject::Wait()
{
  if(synced)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already synced\n");
    return;
  }

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
    auto* gl = controller.GetGL();
    if(DALI_LIKELY(gl) && glSyncObject)
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgingSyncObject::Wait(); glWaitSync\n");
      gl->WaitSync(glSyncObject, 0, 0ull);
    }
  }
  synced = true;
}

uint8_t SyncPool::AgingSyncObject::Age()
{
  uint8_t oldAge = age;
  if(age > 0)
  {
    age--;
  }
  return oldAge;
}

SyncPool::~SyncPool() = default;

SyncPool::SyncObjectId SyncPool::AllocateSyncObject(const Context* writeContext, SyncPool::SyncContext syncContext)
{
  auto agingSyncObject = std::make_unique<AgingSyncObject>(mController, writeContext, (syncContext == SyncContext::EGL));

  auto syncPoolObjectId = ++mSyncObjectId;
  if(DALI_UNLIKELY(syncPoolObjectId == INVALID_SYNC_OBJECT_ID))
  {
    syncPoolObjectId = ++mSyncObjectId;
  }

  // Take ownership of sync object
  mSyncObjects.insert(std::make_pair(syncPoolObjectId, std::move(agingSyncObject)));

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "context = %p, type = %d, [%d]\n", writeContext, syncContext, syncPoolObjectId);

  return syncPoolObjectId;
}

void SyncPool::Wait(SyncPool::SyncObjectId syncPoolObjectId)
{
  AgingSyncObject* agingSyncObject = GetAgingSyncObject(syncPoolObjectId);

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "[%d]\n", syncPoolObjectId);

  if(DALI_LIKELY(agingSyncObject != nullptr))
  {
    agingSyncObject->Wait();
  }
  else
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already synced [%d]\n", syncPoolObjectId);
  }
}

bool SyncPool::ClientWait(SyncPool::SyncObjectId syncPoolObjectId)
{
  AgingSyncObject* agingSyncObject = GetAgingSyncObject(syncPoolObjectId);

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "[%d]\n", syncPoolObjectId);

  if(DALI_LIKELY(agingSyncObject != nullptr))
  {
    return agingSyncObject->ClientWait();
  }
  else
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already synced [%d]\n", syncPoolObjectId);
  }

  return true;
}

void SyncPool::FreeSyncObject(SyncPool::SyncObjectId syncPoolObjectId)
{
  auto iter = mSyncObjects.find(syncPoolObjectId);
  if(iter != mSyncObjects.end())
  {
    // Move memory of sync object to discard queue
    DiscardAgingSyncObject(std::move(iter->second));

    mSyncObjects.erase(iter);

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Free: [%d]\n", syncPoolObjectId);
  }
  else
  {
    // Already freed. Do nothing
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already freed: [%d]\n", syncPoolObjectId);
  }
}

/**
 * Age sync objects. Call at the end of each frame.
 * When a sync object is older than 2 frames, delete it.
 */
void SyncPool::AgeSyncObjects()
{
  mFrameCount++;

  if(!mSyncObjects.empty())
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgeSyncObjects: count: %d\n", mSyncObjects.size());

    // Age the remaining sync objects.
    for(auto iter = mSyncObjects.begin(); iter != mSyncObjects.end();)
    {
      auto* agingSyncObject = (iter->second).get();
      if(DALI_LIKELY(agingSyncObject))
      {
        if(agingSyncObject->Age() > 0)
        {
          ++iter;
        }
        else
        {
          DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Discard [%d]\n", iter->first);

          // Move memory of sync object to discard queue
          DiscardAgingSyncObject(std::move(iter->second));

          iter = mSyncObjects.erase(iter);
        }
      }
      else
      {
        // Something wrong
        iter = mSyncObjects.erase(iter);
      }
    }

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "AgeSyncObjects: count after erase: %d\n", mSyncObjects.size());
  }

  // Shared sync objects
  if(!mSharedSyncObjects.empty())
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "mSharedSyncObjects: count: %d\n", mSharedSyncObjects.size());

    // Age the remaining sync objects.
    for(auto iter = mSharedSyncObjects.begin(); iter != mSharedSyncObjects.end();)
    {
      if(iter->get()->UpdateAge() > 0)
      {
        ++iter;
      }
      else
      {
        iter = mSharedSyncObjects.erase(iter);
      }
    }

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "mSharedSyncObjects: count after erase: %d\n", mSharedSyncObjects.size());
  }
}

void SyncPool::ProcessDiscardSyncObjects(const Context* currentContext)
{
  auto iter = mDiscardSyncObjects.find(currentContext);
  if(iter != mDiscardSyncObjects.end())
  {
#ifdef DEBUG_ENABLED
    auto& agingSyncObjectsList = iter->second;

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "ProcessDiscardSyncObjects: context[%p], count: %zu\n", currentContext, agingSyncObjectsList.size());
#endif

    mDiscardSyncObjects.erase(iter);
  }
}

void SyncPool::InvalidateContext(const Context* invalidatedContext)
{
  if(!mSyncObjects.empty())
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "InvalidateContext: context[%p], count: %d\n", invalidatedContext, mSyncObjects.size());

    for(auto iter = mSyncObjects.begin(); iter != mSyncObjects.end();)
    {
      auto* agingSyncObject = (iter->second).get();
      if(agingSyncObject == nullptr ||
         agingSyncObject->writeContext == invalidatedContext)
      {
        // Release memory of sync object
        // Note : We don't need to call DiscardAgingSyncObject here.
        //        Even if current context is not given context, we should call it
        //        since some DALI objects should be destroyed.
        //        Don't worry about EGLSync leak, since it will be destroyed by eglDestroyContext.
        iter = mSyncObjects.erase(iter);
      }
      else
      {
        ++iter;
      }
    }

    DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "InvalidateContext: context[%p], count after erase: %d\n", mSyncObjects.size());
  }

  // Release discarded sync objects.
  ProcessDiscardSyncObjects(invalidatedContext);
}

void SyncPool::DiscardAgingSyncObject(std::unique_ptr<AgingSyncObject>&& agingSyncObject)
{
  const Context* currentContext = agingSyncObject->writeContext;
  mDiscardSyncObjects[currentContext].emplace_back(std::move(agingSyncObject));
}

SyncPool::AgingSyncObject* SyncPool::GetAgingSyncObject(SyncPool::SyncObjectId syncPoolObjectId) const
{
  auto iter = mSyncObjects.find(syncPoolObjectId);
  if(iter != mSyncObjects.end())
  {
    return iter->second.get();
  }
  return nullptr;
}

std::shared_ptr<SyncPool::SharedSyncObject> SyncPool::AllocateSharedSyncObject(bool useNativeFenceFd)
{
  const Context* context = mController.GetCurrentContext();

  // Find existing one
  for(auto&& iter : mSharedSyncObjects)
  {
    if(iter->Match(context, mFrameCount, useNativeFenceFd))
    {
      // We already have a sync object for the current context and current frame
      DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "Already allocated [%p] [%p, %d]\n", iter.get(), context, mFrameCount, useNativeFenceFd);
      return iter;
    }
  }

  // Otherwise, allocate a new one
  auto syncObject = std::make_shared<SyncPool::SharedSyncObject>(mController, context, mFrameCount, useNativeFenceFd);

  mSharedSyncObjects.push_back(syncObject);

  DALI_LOG_INFO(gLogSyncFilter, Debug::Verbose, "syncObject = %p, context = %p, frame = %d\n", syncObject.get(), context, mFrameCount);

  return syncObject;
}

} // namespace Dali::Graphics::GLES
