#ifndef DALI_GRAPHICS_GLES_SYNC_POOL_H
#define DALI_GRAPHICS_GLES_SYNC_POOL_H

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

// External Headers
#include <dali/graphics-api/graphics-types.h>
#include <dali/integration-api/gl-abstraction.h>
#include <memory>
#include <unordered_map>

namespace Dali
{
namespace Internal::Adaptor
{
class EglSyncObject;
}

namespace Graphics
{
class EglGraphicsController;

namespace GLES
{
class Context;

/**
 * A vector of current fence syncs. They only age if glWaitSync is called on them in the
 * same frame they are created, otherwise they are deleted.
 * They must be created in the writeContext, but can be synced from a readContext.
 *
 * To match the created context and destroy context, we use discarded sync objects queue per each context.
 * After when we can assume that eglMakeCurrent called, we can safely discard the sync objects.
 *
 * @note Before destroy the context, we should call InvalidateContext(), to ensure release memories.
 */
class SyncPool
{
public:
  using SyncObjectId = uint32_t;

  static constexpr SyncObjectId INVALID_SYNC_OBJECT_ID = 0u;

  enum class SyncContext
  {
    EGL, ///< Use EGL sync when syncing between multiple contexts
    GL   ///< Use GL sync when syncing in the same context
  };

  /**
   * A shared sync object that can be reused within the same frame and context.
   */
  class SharedSyncObject
  {
  public:
    /**
     * Constructor
     * @param[in] controller The graphics controller
     * @param[in] context The context associated with this sync object
     * @param[in] frameCount The frame count when this sync object was created
     * @param[in] useNativeFenceFd If true, use native fence FD for synchronization.
     *                              If false, use EGL sync object for synchronization.
     */
    SharedSyncObject(Graphics::EglGraphicsController& controller, const Context* context, uint32_t frameCount, bool useNativeFenceFd);

    /**
     * Destructor
     */
    ~SharedSyncObject();

    /**
     * Poll the native fence FD until it is signaled.
     *
     * This method polls the native fence FD and waits for it to be signaled.
     * After polling completes, the FD is automatically closed.
     *
     * @return True if the sync object was signaled, false otherwise.
     */
    bool Poll();

    /**
     * Wait on a sync object in any context in the CPU
     *
     * @return true if the sync object was signaled, false otherwise.
     */
    bool ClientWait();

    /**
     * Check if the native fence FD is supported.
     *
     * @return True if the native fence FD is supported, false otherwise.
     */
    bool IsFenceFdSupported();

    /**
     * Update the age of the sync object.
     *
     * This method decrements the age counter and marks the sync object as synced
     * when the age reaches zero.
     *
     * @return The old age value before decrementing. Returns 0 if the sync object
     *         was already synced.
     */
    uint8_t UpdateAge();

    /**
     * Check if this sync object matches the given criteria.
     *
     * This method is used to determine if an existing sync object can be reused
     * for the specified context, frame count, and native fence FD configuration.
     *
     * @param[in] context The context to match against
     * @param[in] frameCount The frame count to match against
     * @param[in] useNativeFenceFd Whether native fence FD is used
     * @return True if this sync object matches all criteria, false otherwise
     */
    bool Match(const Context* context, uint32_t frameCount, bool useNativeFenceFd);

  private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
  };

public:
  explicit SyncPool(Graphics::EglGraphicsController& graphicsController)
  : mController(graphicsController)
  {
  }

  ~SyncPool();

  /**
   * Allocate a sync object in the writeContext
   * @param writeContext
   * @return An unique id to a sync object
   */
  SyncObjectId AllocateSyncObject(const Context* writeContext, SyncContext syncContext);

  /**
   * Wait on a sync object in any context in the GPU
   * @param syncPoolObjectId The id of object to wait on. If it's invalid, do nothing.
   */
  void Wait(SyncObjectId syncPoolObjectId);

  /**
   * Wait on a sync object in any context in the CPU
   * @param syncPoolObjectId The id of object to wait on. If it's invalid, return false immediately.
   * @return true if the sync object was signaled, false if it timed out
   */
  bool ClientWait(SyncObjectId syncPoolObjectId);

  /**
   * Delete the sync object if it's not needed.
   * @param syncPoolObjectId The id of object to delete. If it's invalid, do nothing.
   */
  void FreeSyncObject(SyncObjectId syncPoolObjectId);

  /**
   * Age outstanding sync objects. Call at the end of each frame.
   * When a sync object is older than 2 frames, delete it.
   */
  void AgeSyncObjects();

  /**
   * Allocate a shared sync object for the current frame and context.
   *
   * This method implements sync object reuse optimization: when multiple sync objects
   * are requested within the same frame using the same context, only one sync object
   * is created and shared among all requests. This reduces GPU driver overhead and
   * improves performance by avoiding redundant sync object creation.
   *
   * The context is automatically obtained from the current context at the time of
   * the function call. The shared sync object is automatically managed by the pool
   * and will be aged and cleaned up at the end of the frame when AgeSyncObjects() is called.
   *
   * @param[in] useNativeFenceFd If true, use native fence FD for synchronization.
   *                              If false, use EGL sync object for synchronization.
   * @return A shared pointer to the sync object. Multiple calls within the same
   *         frame/context will return the same shared sync object.
   */
  std::shared_ptr<SharedSyncObject> AllocateSharedSyncObject(bool useNativeFenceFd);

public: /// Contexts relative API
  /**
   * Delete all sync objects that were created by given context.
   *
   * @param[in] currentContext The current context which will delete its sync objects.
   */
  void ProcessDiscardSyncObjects(const Context* currentContext);

  /**
   * Notify that given context will be destroyed soon.
   * Let us remove all sync objects created by given context.
   * @param invalidatedContext The context which will be called eglDestroyContext soon.
   */
  void InvalidateContext(const Context* invalidatedContext);

private:
  struct AgingSyncObject
  {
    AgingSyncObject(Graphics::EglGraphicsController& controller, const Context* writeContext, bool _egl);
    ~AgingSyncObject();

    void Wait();
    bool ClientWait();

    uint8_t Age();

    EglGraphicsController& controller;
    const Context*         writeContext;

    union
    {
      GLsync                            glSyncObject;
      Internal::Adaptor::EglSyncObject* eglSyncObject;
    };

    uint8_t age{3u};
    bool    synced{false};
    bool    egl{false};
  };

private:
  /**
   * Discard a sync object that was created by given context.
   * @param currentContext The current context which will delete its sync object.
   * @param agingSyncObject The sync object to delete.
   */
  void DiscardAgingSyncObject(std::unique_ptr<AgingSyncObject>&& agingSyncObject);

  /**
   * Get aging sync object from the container
   * @param syncPoolObjectId The id of object to get.
   * @return pointer to the aging sync object
   */
  AgingSyncObject* GetAgingSyncObject(SyncObjectId syncPoolObjectId) const;

private:
  using SyncObjectContainer = std::unordered_map<SyncObjectId, std::unique_ptr<AgingSyncObject>>;
  SyncObjectContainer mSyncObjects; ///< The list of sync objects in this pool (owned)

  using DiscardedSyncObjectContainer = std::unordered_map<const Context*, std::vector<std::unique_ptr<AgingSyncObject>>>;
  DiscardedSyncObjectContainer mDiscardSyncObjects; ///< The list of discarded sync objects per each context

  using SharedSyncObjectContainer = std::vector<std::shared_ptr<SharedSyncObject>>;
  SharedSyncObjectContainer mSharedSyncObjects;

  EglGraphicsController& mController;

  SyncObjectId mSyncObjectId{INVALID_SYNC_OBJECT_ID};
  uint32_t     mFrameCount{0u};
};

} // namespace GLES
} // namespace Graphics
} // namespace Dali

#endif //DALI_GRAPHICS_GLES_SYNC_POOL_H
