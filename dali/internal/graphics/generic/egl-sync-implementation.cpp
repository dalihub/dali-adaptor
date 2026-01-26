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
 *
 */

// CLASS HEADER
#include <dali/internal/graphics/gles/egl-sync-implementation.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-debug.h>
#include <dali/internal/graphics/gles/egl-implementation.h>

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogSyncFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_FENCE_SYNC");
#endif

namespace
{
DALI_INIT_TIME_CHECKER_FILTER(gTimeCheckerFilter, DALI_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);
} // namespace

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
EglSyncObject::EglSyncObject(EglImplementation& eglImpl, SyncObject::SyncType type)
: mEglSync(NULL),
  mEglImplementation(eglImpl)
{
  EGLDisplay display = mEglImplementation.GetDisplay();

  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  mEglSync = eglCreateSync(display, EGL_SYNC_FENCE, NULL);
  DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglCreateSync");

  if(mEglSync == EGL_NO_SYNC)
  {
    DALI_LOG_ERROR("eglCreateSync failed %#0.4x\n", eglGetError());
    mEglSync = NULL;
  }
  else
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglCreateSync Success: %p\n", mEglSync);
  }
}

EglSyncObject::~EglSyncObject()
{
  if(mEglSync != NULL && mEglImplementation.IsGlesInitialized())
  {
    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    eglDestroySync(mEglImplementation.GetDisplay(), mEglSync);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglDestroySync");

    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglDestroySync failed %#0.4x\n", error);
    }
    else
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglDestroySync Success: %p\n", mEglSync);
    }
  }
}

bool EglSyncObject::IsSynced()
{
  bool synced = false;

  if(mEglSync != NULL)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync no timeout\n");

    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    EGLint result = eglClientWaitSync(mEglImplementation.GetDisplay(), mEglSync, 0, 0ull);

    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglClientWaitSync failed %#0.4x\n", error);
    }
    else if(result == EGL_CONDITION_SATISFIED)
    {
      synced = true;
    }
    DALI_TIME_CHECKER_END_WITH_MESSAGE_GENERATOR(gTimeCheckerFilter, [&](std::ostringstream& oss)
    {
      oss << "eglClientWaitSync(no timeout) synced : " << synced;
    });
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync(%p, 0, 0) %s\n", mEglSync, synced ? "Synced" : "NOT SYNCED");
  return synced;
}

void EglSyncObject::Wait()
{
  if(mEglSync != NULL)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglWaitSync\n");

    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    auto result = eglWaitSync(mEglImplementation.GetDisplay(), mEglSync, 0);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglWaitSync");

    if(EGL_FALSE == result)
    {
      Egl::PrintError(eglGetError());
    }
    else
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglWaitSync() %p synced!\n", mEglSync);
    }
  }
}

void EglSyncObject::ClientWait()
{
#if defined(DEBUG_ENABLED)
  bool synced = false;
#endif
  if(mEglSync != NULL)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync FOREVER\n");

    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    auto result = eglClientWaitSync(mEglImplementation.GetDisplay(), mEglSync, EGL_SYNC_FLUSH_COMMANDS_BIT, EGL_FOREVER);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglClientWaitSync(forever)");

    if(result == EGL_FALSE)
    {
      Egl::PrintError(eglGetError());
    }
#if defined(DEBUG_ENABLED)
    else if(result == EGL_CONDITION_SATISFIED)
    {
      synced = true;
    }
#endif
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync(%p, 0, FOREVER) %s\n", mEglSync, synced ? "Synced" : "NOT SYNCED");
}

int32_t EglSyncObject::DuplicateNativeFenceFD()
{
  return -1;
}

bool EglSyncObject::Poll()
{
  return false;
}

void EglSyncObject::DestroySyncObject()
{
}

EglSyncImplementation::EglSyncImplementation()
: mEglImplementation(NULL),
  mSyncInitialized(false),
  mSyncInitializeFailed(false)
{
}

EglSyncImplementation::~EglSyncImplementation()
{
  for(auto& syncObject : mSyncObjects)
  {
    delete static_cast<EglSyncObject*>(syncObject);
  }
  mSyncObjects.Clear();
}

void EglSyncImplementation::Initialize(EglImplementation* eglImpl)
{
  mEglImplementation = eglImpl;
}

Integration::GraphicsSyncAbstraction::SyncObject* EglSyncImplementation::CreateSyncObject(SyncObject::SyncType type)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  auto* syncObject = new EglSyncObject(*mEglImplementation, type);
  mSyncObjects.PushBack(syncObject);
  return syncObject;
}

void EglSyncImplementation::DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  mSyncObjects.EraseObject(static_cast<EglSyncObject*>(syncObject));
  delete static_cast<EglSyncObject*>(syncObject);
}

void EglSyncImplementation::InitializeEglSync()
{
}

bool NativeFence::PollFD(int32_t fenceFd)
{
  return false;
}

void NativeFence::CloseFD(int32_t fenceFd)
{
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
