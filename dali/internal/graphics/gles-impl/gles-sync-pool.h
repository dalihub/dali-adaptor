#ifndef DALI_GRAPHICS_GLES_SYNC_POOL_H
#define DALI_GRAPHICS_GLES_SYNC_POOL_H

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
 */

#include <dali/graphics-api/graphics-types.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/ordered-set.h>
#include <dali/public-api/common/vector-wrapper.h>

#include <cstdint>
#include <memory>

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

  enum class SyncContext
  {
    EGL, ///< Use EGL sync when syncing between multiple contexts
    GL   ///< Use GL sync when syncing in the same context
  };

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
   * Check whether given object is synced in the CPU
   * @param syncPoolObjectId The id of object to check synced.
   * @return true if the sync object was signaled, false if it timed out
   */
  bool IsSynced(SyncObjectId syncPoolObjectId);

  /**
   * Wait on a sync object in any context in the GPU
   * @param syncPoolObjectId The id of object to wait on.
   */
  void Wait(SyncObjectId syncPoolObjectId);

  /**
   * Wait on a sync object in any context in the CPU
   * @param syncPoolObjectId The id of object to wait on.
   * @return true if the sync object was signaled, false if it timed out
   */
  bool ClientWait(SyncObjectId syncPoolObjectId);

  /**
   * Delete the sync object if it's not needed.
   * @param syncPoolObjectId The id of object to delete.
   */
  void FreeSyncObject(SyncObjectId syncPoolObjectId);

  /**
   * Age outstanding sync objects. Call at the end of each frame.
   * When a sync object is older than 2 frames, delete it.
   */
  void AgeSyncObjects();

public: /// Contexts relative API
  /**
   * Delete all sync objects that were created by given context.
   *
   * @param[in] currentContext The current context which will delete its sync objects.
   */
  void ProcessDiscardSyncObjects(const Context* currentContext);

  /**
   * Notify that given context will be destroyed soon.
   * Let we remove all sync objects created by given context.
   * @param invalidatedContext The context which will be called eglDestroyContext soon.
   */
  void InvalidateContext(const Context* invalidatedContext);

private:
  struct AgingSyncObject
  {
    AgingSyncObject(Graphics::EglGraphicsController& controller, const Context* writeContext, bool egl = false);
    ~AgingSyncObject();

    EglGraphicsController& controller;
    const Context*         writeContext;
    union
    {
      GLsync                            glSyncObject;
      Internal::Adaptor::EglSyncObject* eglSyncObject;
    };
    uint8_t age{2};
    bool    syncing{false};
    bool    egl{false};

    bool IsSynced();
    void Wait();
    bool ClientWait();
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

  EglGraphicsController& mController;

  SyncObjectId mSyncObjectId{0u};
};

} // namespace GLES
} // namespace Graphics
} // namespace Dali

#endif //DALI_GRAPHICS_GLES_SYNC_POOL_H
