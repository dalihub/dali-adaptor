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
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-debug.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/time-service.h>

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogSyncFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_FENCE_SYNC");
#endif

namespace
{
static uint32_t gLogThreshold{0};
static bool     gLogEnabled{false};

static uint32_t GetPerformanceLogThresholdTime()
{
  auto     timeString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);
  uint32_t time       = timeString ? static_cast<uint32_t>(std::atoi(timeString)) : std::numeric_limits<uint32_t>::max();
  return time;
}

#define START_DURATION_CHECK()                         \
  uint64_t startTimeNanoSeconds = 0ull;                \
  uint64_t endTimeNanoSeconds   = 0ull;                \
  if(gLogEnabled)                                      \
  {                                                    \
    TimeService::GetNanoseconds(startTimeNanoSeconds); \
  }

#define FINISH_DURATION_CHECK(functionName)                                                                                                                \
  if(gLogEnabled)                                                                                                                                          \
  {                                                                                                                                                        \
    TimeService::GetNanoseconds(endTimeNanoSeconds);                                                                                                       \
    if(static_cast<uint32_t>((endTimeNanoSeconds - startTimeNanoSeconds) / 1000000ull) >= gLogThreshold)                                                   \
    {                                                                                                                                                      \
      DALI_LOG_RELEASE_INFO("%s takes long time! [%.6lf ms]\n", functionName, static_cast<double>(endTimeNanoSeconds - startTimeNanoSeconds) / 1000000.0); \
    }                                                                                                                                                      \
  }

#define FINISH_DURATION_CHECK_WITH_FORMAT(functionName, format, ...)                                                                                                                 \
  if(gLogEnabled)                                                                                                                                                                    \
  {                                                                                                                                                                                  \
    TimeService::GetNanoseconds(endTimeNanoSeconds);                                                                                                                                 \
    if(static_cast<uint32_t>((endTimeNanoSeconds - startTimeNanoSeconds) / 1000000ull) >= gLogThreshold)                                                                             \
    {                                                                                                                                                                                \
      DALI_LOG_RELEASE_INFO("%s takes long time! [%.6lf ms] " format "\n", functionName, static_cast<double>(endTimeNanoSeconds - startTimeNanoSeconds) / 1000000.0, ##__VA_ARGS__); \
    }                                                                                                                                                                                \
  }

} // namespace

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
EglSyncObject::EglSyncObject(EglImplementation& eglImpl)
: mEglSync(NULL),
  mPollCounter(3),
  mEglImplementation(eglImpl)
{
  EGLDisplay display = mEglImplementation.GetDisplay();

  START_DURATION_CHECK();
  mEglSync = eglCreateSync(display, EGL_SYNC_FENCE, NULL);
  FINISH_DURATION_CHECK("eglCreateSync");

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
    START_DURATION_CHECK();
    eglDestroySync(mEglImplementation.GetDisplay(), mEglSync);
    FINISH_DURATION_CHECK("eglDestroySync");

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

    START_DURATION_CHECK();
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
    FINISH_DURATION_CHECK_WITH_FORMAT("eglClientWaitSync(no timeout)", "synced : %d", synced);
  }

  DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglClientWaitSync(%p, 0, 0) %s\n", mEglSync, synced ? "Synced" : "NOT SYNCED");
  return synced;
}

void EglSyncObject::Wait()
{
  if(mEglSync != NULL)
  {
    DALI_LOG_INFO(gLogSyncFilter, Debug::General, "eglWaitSync\n");

    START_DURATION_CHECK();
    auto result = eglWaitSync(mEglImplementation.GetDisplay(), mEglSync, 0);
    FINISH_DURATION_CHECK("eglWaitSync");

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

    START_DURATION_CHECK();
    auto result = eglClientWaitSync(mEglImplementation.GetDisplay(), mEglSync, EGL_SYNC_FLUSH_COMMANDS_BIT, EGL_FOREVER);
    FINISH_DURATION_CHECK("eglClientWaitSync(forever)");

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

EglSyncImplementation::EglSyncImplementation()
: mEglImplementation(NULL),
  mSyncInitialized(false),
  mSyncInitializeFailed(false)
{
  gLogThreshold = GetPerformanceLogThresholdTime();
  gLogEnabled   = gLogThreshold < std::numeric_limits<uint32_t>::max() ? true : false;
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
