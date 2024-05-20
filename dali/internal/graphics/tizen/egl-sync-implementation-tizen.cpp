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

#ifdef _ARCH_ARM_

#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#endif

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-debug.h>
#include <dali/internal/graphics/gles/egl-implementation.h>

#ifdef _ARCH_ARM_

// function pointers
static PFNEGLCREATESYNCKHRPROC     eglCreateSyncKHR     = NULL;
static PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR = NULL;
static PFNEGLDESTROYSYNCKHRPROC    eglDestroySyncKHR    = NULL;
static PFNEGLWAITSYNCKHRPROC       eglWaitSyncKHR       = NULL;

#endif

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogSyncFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_FENCE_SYNC");
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#ifdef _ARCH_ARM_

EglSyncObject::EglSyncObject(EglImplementation& eglImpl)
: mEglSync(NULL),
  mEglImplementation(eglImpl)
{
  EGLDisplay display = mEglImplementation.GetDisplay();
  mEglSync           = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, NULL);
  if(mEglSync == EGL_NO_SYNC_KHR)
  {
    DALI_LOG_ERROR("eglCreateSyncKHR failed %#0.4x\n", eglGetError());
    mEglSync = NULL;
  }
  else
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglCreateSyncKHR Success: %p\n", mEglSync);
  }
}

EglSyncObject::~EglSyncObject()
{
  if(mEglSync != NULL)
  {
    eglDestroySyncKHR(mEglImplementation.GetDisplay(), mEglSync);
    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglDestroySyncKHR failed %#0.4x\n", error);
    }
    else
    {
      DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglDestroySyncKHR Success: %p\n", mEglSync);
    }
  }
}

bool EglSyncObject::IsSynced()
{
  bool synced = false;

  if(mEglSync != NULL)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync no timeout\n");
    EGLint result = eglClientWaitSyncKHR(mEglImplementation.GetDisplay(), mEglSync, 0, 0ull);
    EGLint error  = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglClientWaitSyncKHR failed %#0.4x\n", error);
    }
    else if(result == EGL_CONDITION_SATISFIED_KHR)
    {
      synced = true;
    }
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync(%p, 0, 0) %s\n", mEglSync, synced ? "Synced" : "NOT SYNCED");
  return synced;
}

void EglSyncObject::ClientWait()
{
  bool synced = false;
  if(mEglSync != nullptr)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync FOREVER\n");
    auto result = eglClientWaitSyncKHR(mEglImplementation.GetDisplay(), mEglSync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR);
    if(result == EGL_FALSE)
    {
      Egl::PrintError(eglGetError());
    }
    else if(result == EGL_CONDITION_SATISFIED_KHR)
    {
      synced = true;
    }
  }
  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync(%p, 0, FOREVER) %s\n", mEglSync, synced ? "Synced" : "NOT SYNCED");
}

void EglSyncObject::Wait()
{
  if(mEglSync != nullptr)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglWaitSync\n");
    auto result = eglWaitSyncKHR(mEglImplementation.GetDisplay(), mEglSync, 0);
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
  if(mSyncInitialized == false)
  {
    InitializeEglSync();
  }

  auto* syncObject = new EglSyncObject(*mEglImplementation);
  mSyncObjects.PushBack(syncObject);
  return syncObject;
}

void EglSyncImplementation::DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");

  if(mSyncInitialized == false)
  {
    InitializeEglSync();
  }

  for(SyncIter iter = mSyncObjects.Begin(), end = mSyncObjects.End(); iter != end; ++iter)
  {
    if(*iter == syncObject)
    {
      mSyncObjects.Erase(iter);
      break;
    }
  }
  delete static_cast<EglSyncObject*>(syncObject);
}

void EglSyncImplementation::InitializeEglSync()
{
  if(!mSyncInitializeFailed)
  {
    eglCreateSyncKHR     = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(eglGetProcAddress("eglCreateSyncKHR"));
    eglClientWaitSyncKHR = reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(eglGetProcAddress("eglClientWaitSyncKHR"));
    eglWaitSyncKHR       = reinterpret_cast<PFNEGLWAITSYNCKHRPROC>(eglGetProcAddress("eglWaitSyncKHR"));
    eglDestroySyncKHR    = reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(eglGetProcAddress("eglDestroySyncKHR"));
  }

  if(eglCreateSyncKHR && eglClientWaitSyncKHR && eglWaitSyncKHR && eglDestroySyncKHR)
  {
    mSyncInitialized = true;
  }
  else
  {
    mSyncInitializeFailed = true;
  }
}

#else

EglSyncObject::EglSyncObject(EglImplementation& eglImpl)
: mEglSync(NULL),
  mPollCounter(3),
  mEglImplementation(eglImpl)
{
}

EglSyncObject::~EglSyncObject()
{
}

bool EglSyncObject::IsSynced()
{
  return true;
}

void EglSyncObject::Wait()
{
}

void EglSyncObject::ClientWait()
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

#endif

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
