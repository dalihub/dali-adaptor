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
#include <GLES2/gl2.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
EglSyncObject::EglSyncObject(EglImplementation& eglImpl, EglSyncObject::SyncType type)
: mEglSync(nullptr),
  mEglImplementation(eglImpl)
{
  static_cast<void>(type);

  // Windows backend last-resort fallback for callers that cannot use the GLES
  // fence path. The pinned ANGLE D3D backend reports EGL 1.4 without EGL fence
  // extensions. Rendering may still appear correct due to ANGLE or driver
  // scheduling, but cross-context ordering is not guaranteed without an
  // explicit barrier. glFinish() guarantees producer completion, at the cost
  // of a CPU/GPU stall that can reduce multi-pass rendering performance.
  glFinish();
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

struct EglSyncImplementation::Impl
{
};

EglSyncImplementation::EglSyncImplementation()
: mEglImplementation(nullptr),
  mSyncObjects(),
  mImpl(nullptr),
  mUseGlSyncForTextureDependencies(true)
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

void EglSyncImplementation::DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  mSyncObjects.EraseObject(static_cast<EglSyncObject*>(syncObject));
  delete static_cast<EglSyncObject*>(syncObject);
}

Integration::GraphicsSyncAbstraction::SyncObject* EglSyncImplementation::CreateSyncObject(EglSyncObject::SyncType type)
{
  DALI_ASSERT_ALWAYS(mEglImplementation && "Sync Implementation not initialized");
  auto* syncObject = new EglSyncObject(*mEglImplementation, type);
  mSyncObjects.PushBack(syncObject);
  return syncObject;
}

void EglSyncImplementation::InitializeEglSync()
{
}

bool NativeFence::PollFD(int32_t fenceFd)
{
  static_cast<void>(fenceFd);
  return false;
}

void NativeFence::CloseFD(int32_t fenceFd)
{
  static_cast<void>(fenceFd);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
