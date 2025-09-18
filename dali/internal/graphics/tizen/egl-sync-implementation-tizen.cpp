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
 *
 */

// CLASS HEADER
#include <dali/internal/graphics/gles/egl-sync-implementation.h>

// EXTERNAL INCLUDES

#ifdef _ARCH_ARM_

#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <fcntl.h>
#include <unistd.h>

#endif

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-debug.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/system/common/system-error-print.h>

#ifdef _ARCH_ARM_

namespace
{
// function pointers
static PFNEGLCREATESYNCKHRPROC           eglCreateSyncKHR           = NULL;
static PFNEGLCLIENTWAITSYNCKHRPROC       eglClientWaitSyncKHR       = NULL;
static PFNEGLDESTROYSYNCKHRPROC          eglDestroySyncKHR          = NULL;
static PFNEGLWAITSYNCKHRPROC             eglWaitSyncKHR             = NULL;
static PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID = NULL;

DALI_INIT_TIME_CHECKER_FILTER(gTimeCheckerFilter, DALI_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);

} // namespace

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

EglSyncObject::EglSyncObject(EglImplementation& eglImpl, EglSyncObject::SyncType type)
: mEglSync(NULL),
  mEglImplementation(eglImpl)
{
  EGLDisplay display  = mEglImplementation.GetDisplay();
  EGLenum    syncType = (type == SyncType::FENCE_SYNC) ? EGL_SYNC_FENCE_KHR : EGL_SYNC_NATIVE_FENCE_ANDROID;

  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  mEglSync = eglCreateSyncKHR(display, syncType, NULL);
  DALI_TIME_CHECKER_END_WITH_MESSAGE_GENERATOR(gTimeCheckerFilter, [&](std::ostringstream& oss)
  {
    oss << "eglCreateSyncKHR(" << ((syncType == EGL_SYNC_FENCE_KHR) ? "EGL_SYNC_FENCE_KHR" : "EGL_SYNC_NATIVE_FENCE_ANDROID") << ")";
  });
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
  if(mEglSync != NULL && mEglImplementation.IsGlesInitialized())
  {
    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    eglDestroySyncKHR(mEglImplementation.GetDisplay(), mEglSync);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglDestroySyncKHR");

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

    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    EGLint result = eglClientWaitSyncKHR(mEglImplementation.GetDisplay(), mEglSync, 0, 0ull);

    EGLint error = eglGetError();
    if(EGL_SUCCESS != error)
    {
      DALI_LOG_ERROR("eglClientWaitSyncKHR failed %#0.4x\n", error);
    }
    else if(result == EGL_CONDITION_SATISFIED_KHR)
    {
      synced = true;
    }
    DALI_TIME_CHECKER_END_WITH_MESSAGE_GENERATOR(gTimeCheckerFilter, [&](std::ostringstream& oss)
    {
      oss << "eglClientWaitSyncKHR(no timeout) synced : " << synced;
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
    auto result = eglWaitSyncKHR(mEglImplementation.GetDisplay(), mEglSync, 0);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglWaitSyncKHR");

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
    auto result = eglClientWaitSyncKHR(mEglImplementation.GetDisplay(), mEglSync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglClientWaitSyncKHR(forever)");

    if(result == EGL_FALSE)
    {
      Egl::PrintError(eglGetError());
    }
#if defined(DEBUG_ENABLED)
    else if(result == EGL_CONDITION_SATISFIED_KHR)
    {
      synced = true;
    }
#endif
  }
  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync(%p, 0, FOREVER) %s\n", mEglSync, synced ? "Synced" : "NOT SYNCED");
}

int32_t EglSyncObject::DuplicateNativeFenceFD()
{
  if(mEglSync != NULL && eglDupNativeFenceFDANDROID)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglDupNativeFenceFDANDROID\n");

    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    int32_t fenceFd = eglDupNativeFenceFDANDROID(mEglImplementation.GetDisplay(), mEglSync);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglDupNativeFenceFDANDROID");

    if(fenceFd == EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
      Egl::PrintError(eglGetError());
      return -1;
    }
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglDupNativeFenceFDANDROID [%d]\n", fenceFd);

    int flags = fcntl(fenceFd, F_GETFL);
    if(flags == -1)
    {
      DALI_LOG_ERROR("fcntl F_GETFL failed\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
      close(fenceFd);
      return -1;
    }
    else
    {
      if(fcntl(fenceFd, F_SETFL, flags | O_NONBLOCK) == -1)
      {
        DALI_LOG_ERROR("fcntl F_SETFL failed\n");
        DALI_PRINT_SYSTEM_ERROR_LOG();
        close(fenceFd);
        return -1;
      }
    }

    return fenceFd;
  }

  DALI_LOG_ERROR("eglDupNativeFenceFDANDROID is not supported!\n");
  return -1;
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

Integration::GraphicsSyncAbstraction::SyncObject* EglSyncImplementation::CreateSyncObject()
{
  return CreateSyncObject(EglSyncObject::SyncType::FENCE_SYNC);
}

void EglSyncImplementation::DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");

  if(mSyncInitialized == false)
  {
    InitializeEglSync();
  }

  mSyncObjects.EraseObject(static_cast<EglSyncObject*>(syncObject));
  delete static_cast<EglSyncObject*>(syncObject);
}

Integration::GraphicsSyncAbstraction::SyncObject* EglSyncImplementation::CreateSyncObject(EglSyncObject::SyncType type)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  if(mSyncInitialized == false)
  {
    InitializeEglSync();
  }

  auto* syncObject = new EglSyncObject(*mEglImplementation, type);
  mSyncObjects.PushBack(syncObject);
  return syncObject;
}

void EglSyncImplementation::InitializeEglSync()
{
  if(!mSyncInitializeFailed)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglGetProcAddress");
    eglCreateSyncKHR           = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(eglGetProcAddress("eglCreateSyncKHR"));
    eglClientWaitSyncKHR       = reinterpret_cast<PFNEGLCLIENTWAITSYNCKHRPROC>(eglGetProcAddress("eglClientWaitSyncKHR"));
    eglWaitSyncKHR             = reinterpret_cast<PFNEGLWAITSYNCKHRPROC>(eglGetProcAddress("eglWaitSyncKHR"));
    eglDestroySyncKHR          = reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(eglGetProcAddress("eglDestroySyncKHR"));
    eglDupNativeFenceFDANDROID = reinterpret_cast<PFNEGLDUPNATIVEFENCEFDANDROIDPROC>(eglGetProcAddress("eglDupNativeFenceFDANDROID"));
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

EglSyncObject::EglSyncObject(EglImplementation& eglImpl, EglSyncObject::SyncType type)
: mEglSync(NULL),
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

EGLint EglSyncObject::DuplicateNativeFenceFD()
{
  return -1;
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
  return CreateSyncObject(EglSyncObject::SyncType::FENCE_SYNC);
}

void EglSyncImplementation::DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  delete static_cast<EglSyncObject*>(syncObject);
}

Integration::GraphicsSyncAbstraction::SyncObject* EglSyncImplementation::CreateSyncObject(EglSyncObject::SyncType type)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  return new EglSyncObject(*mEglImplementation, type);
}

void EglSyncImplementation::InitializeEglSync()
{
}

#endif

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
