#ifndef DALI_INTERNAL_ADAPTOR_EGL_SYNC_IMPLEMENTATION_H
#define DALI_INTERNAL_ADAPTOR_EGL_SYNC_IMPLEMENTATION_H

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
  EglSyncObject(EglImplementation& eglSyncImpl, SyncObject::SyncType type);

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

  /**
   * Duplicates a native fence file descriptor from an EGLSync object.
   *
   * This method creates a duplicate of the native fence file descriptor associated
   * with this EGL sync object. The native fence FD can be used to synchronize
   * GPU operations across different processes or contexts.
   *
   * Wait() and ClientWait() can't be used after this method succeeds.
   * Poll() should be used to wait in the case.
   *
   * @return The duplicated native fence file descriptor, or -1 on failure.
   */
  int32_t DuplicateNativeFenceFD();

  /**
   * Wait until the duplicated native fence FD is signaled.
   *
   * @return True if the FD is signaled before timeout, false otherwise.
   */
  bool Poll();

private:
  /**
   * Destroy the egl sync object.
   */
  void DestroySyncObject();

private:
#if defined(_ARCH_ARM_) || defined(__aarch64__)
  EGLSyncKHR mEglSync;
  int32_t    mFenceFd;
#else
  EGLSync mEglSync;
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
   * Create a sync object with the specified type that can be polled.
   *
   * @param[in] type The type of sync object to create (FENCE_SYNC or NATIVE_FENCE_SYNC)
   * @return Pointer to the created sync object, or nullptr if creation fails
   *
   * @note The caller is responsible for destroying the sync object using
   *       DestroySyncObject() when it is no longer needed.
   */
  Integration::GraphicsSyncAbstraction::SyncObject* CreateSyncObject(SyncObject::SyncType type) override;

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

/**
 * Utility functions for native fence file descriptor operations.
 *
 * This namespace provides helper functions for working with native fence file
 * descriptors used for GPU synchronization across processes or contexts.
 */
namespace NativeFence
{
/**
 * Poll on a native fence file descriptor until it is signaled.
 *
 * This method waits for the native fence FD to be signaled, indicating that
 * the associated GPU operation has completed. After polling completes, the
 * file descriptor is automatically closed.
 *
 * @param[in] fenceFd The native fence file descriptor to poll.
 * @return True if the FD is signaled before timeout, false otherwise.
 */
bool PollFD(int32_t fenceFd);

/**
 * Close a native fence file descriptor.
 *
 * This method closes the specified native fence file descriptor, releasing
 * the associated system resources.
 *
 * @param[in] fenceFd The native fence file descriptor to close.
 */
void CloseFD(int32_t fenceFd);
} // namespace NativeFence

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_EGL_ADAPTOR_SYNC_IMPLEMENTATION_H
