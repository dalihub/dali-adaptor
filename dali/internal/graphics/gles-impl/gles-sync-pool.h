#ifndef DALI_GRAPHICS_GLES_SYNC_POOL_H
#define DALI_GRAPHICS_GLES_SYNC_POOL_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali::Graphics
{
class EglGraphicsController;

namespace GLES
{
class Context;

struct AgingSyncObject
{
  AgingSyncObject(Graphics::EglGraphicsController& controller, const Context* writeContext);
  ~AgingSyncObject();

  EglGraphicsController& controller;
  const Context*         writeContext;
  GLsync                 glSyncObject{0};
  uint8_t                age{2};
  bool                   syncing{false};
};
using AgingSyncPtrRef = std::unique_ptr<AgingSyncObject>&;

/**
 * A vector of current fence syncs. They only age if glWaitSync is called on them in the
 * same frame they are created, otherwise they are deleted.
 * They must be created in the writeContext, but can be synced from a readContext.
 * (Pool per context? - probably only ever used in resource context!)
 */
class SyncPool
{
public:
  explicit SyncPool(Graphics::EglGraphicsController& graphicsController)
  : mController(graphicsController)
  {
  }

  ~SyncPool();

  /**
   * Allocate a sync object in the writeContext
   * @param writeContext
   * @return An owned ptr to a sync object
   */
  AgingSyncObject* AllocateSyncObject(const Context* writeContext);

  /**
   * Wait on a sync object in any context
   * @param syncPoolObject The object to wait on.
   */
  void Wait(AgingSyncObject* syncPoolObject);

  /**
   * Delete the sync object if it's not needed.
   *
   */
  void FreeSyncObject(AgingSyncObject* agingSyncObject);

  /**
   * Age outstanding sync objects. Call at the end of each frame.
   * When a sync object is older than 2 frames, delete it.
   */
  void AgeSyncObjects();

private:
  std::vector<std::unique_ptr<AgingSyncObject>> mSyncObjects;
  EglGraphicsController&                        mController;
};

} // namespace GLES
} // namespace Dali::Graphics

#endif //DALI_GRAPHICS_GLES_SYNC_POOL_H
