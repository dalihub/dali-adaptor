#ifndef DALI_INTERNAL_ADAPTOR_EGL_SYNC_IMPLEMENTATION_H
#define DALI_INTERNAL_ADAPTOR_EGL_SYNC_IMPLEMENTATION_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/ordered-set.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

#include <dali/integration-api/graphics-sync-abstraction.h>
#include <dali/internal/graphics/common/egl-include.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EglImplementation;

class EglSyncObject : public Integration::GraphicsSyncAbstraction::SyncObject
{
public:
  /**
   * Constructor
   */
  EglSyncObject(EglImplementation& eglSyncImpl);

  /**
   * Destructor
   */
  virtual ~EglSyncObject();

  bool IsSynced() override;

  /**
   * Set up a GPU wait (returns immediately on CPU) for this sync. Can work across
   * EGL contexts.
   */
  void Wait() override;

  /**
   * Wait on the CPU until the GPU executes this sync. Warning: could be a long time!
   * Can work across EGL contexts.
   */
  void ClientWait() override;

private:
#ifdef _ARCH_ARM_
  EGLSyncKHR mEglSync;
#else
  EGLSync mEglSync;
  int     mPollCounter; // Implementations without fence sync use a 3 frame counter
#endif
  EglImplementation& mEglImplementation;
};

/**
 * It provides fence syncing for resources such as FrameBuffers using EGL extensions
 *
 * Sync objects are created in the render thread after a render instruction
 * has been processed (i.e. GL draw calls have completed for a given FB), and
 * tested in the update thread.
 */
class EglSyncImplementation : public Integration::GraphicsSyncAbstraction
{
public:
  /**
   * Constructor
   */
  EglSyncImplementation();

  /**
   * Destructor
   */
  ~EglSyncImplementation();

  /**
   * Initialize the sync object with the Egl implementation.
   * @param[in] impl The EGL implementation (to access display)
   */
  void Initialize(EglImplementation* impl);

  /**
   * Create a sync object that can be polled
   */
  Integration::GraphicsSyncAbstraction::SyncObject* CreateSyncObject() override;

  /**
   * Destroy a sync object
   */
  void DestroySyncObject(Integration::GraphicsSyncAbstraction::SyncObject* syncObject) override;

private:
  /**
   * Set up the function pointers
   */
  void InitializeEglSync();

private:
  typedef Integration::OrderedSet<EglSyncObject, false> SyncContainer;
  typedef SyncContainer::Iterator                       SyncIter;

  EglImplementation* mEglImplementation;    ///< Egl implementation (to get display)
  bool               mSyncInitialized;      ///< Flag to perform initialization on first use
  bool               mSyncInitializeFailed; ///< Flag to avoid reloading functions if failed once

  SyncContainer mSyncObjects;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_EGL_ADAPTOR_SYNC_IMPLEMENTATION_H
