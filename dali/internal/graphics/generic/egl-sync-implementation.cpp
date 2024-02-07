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

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
EglSyncObject::EglSyncObject(EglImplementation& eglImpl)
: mPollCounter(3),
  mEglImplementation(eglImpl)
{
  EGLDisplay display = mEglImplementation.GetDisplay();
  mEglSync           = eglCreateSync(display, EGL_SYNC_FENCE, NULL);
}

EglSyncObject::~EglSyncObject()
{
  if(mEglSync && mEglImplementation.IsGlesInitialized())
  {
    EGLDisplay display = mEglImplementation.GetDisplay();
    eglDestroySync(display, mEglSync);
  }
}

bool EglSyncObject::IsSynced()
{
  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync\n");
  auto result = eglClientWaitSync(mEglImplementation.GetDisplay(), mEglSync, 0 | EGL_SYNC_FLUSH_COMMANDS_BIT, 0);

  if(result == EGL_FALSE)
  {
    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglClientSyncWait failed: %#0.4x\n", error);
    }
  }
  else if(result == EGL_CONDITION_SATISFIED)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync Synced!\n");
    return true;
  }
  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync not synced :(\n");
  return false;
}

void EglSyncObject::Wait()
{
  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglWaitSync\n");
  if(!eglWaitSync(mEglImplementation.GetDisplay(), mEglSync, 0))
  {
    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglSyncWait failed: %#0.4x\n", error);
    }
  }
}

void EglSyncObject::ClientWait()
{
  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglWaitSync (blocking)\n");
  auto result = eglClientWaitSync(mEglImplementation.GetDisplay(), mEglSync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER);
  if(result == EGL_FALSE)
  {
    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglSyncWait failed: %#0.4x\n", error);
    }
  }
  else if(result == EGL_CONDITION_SATISFIED)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync Synced!\n");
  }
}

EglSyncImplementation::EglSyncImplementation()
: mEglImplementation(NULL),
  mSyncInitialized(false),
  mSyncInitializeFailed(false)
{
}

EglSyncImplementation::~EglSyncImplementation()
{
}

void EglSyncImplementation::Initialize(EglImplementation* eglImpl)
{
  mEglImplementation = eglImpl;
}

Integration::GraphicsSyncAbstraction::SyncObject* EglSyncImplementation::CreateSyncObject()
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  return new EglSyncObject(*mEglImplementation);
}

void EglSyncImplementation::DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  delete static_cast<EglSyncObject*>(syncObject);
}

void EglSyncImplementation::InitializeEglSync()
{
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
