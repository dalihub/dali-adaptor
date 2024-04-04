#ifndef DALI_GRAPHICS_GLES_SYNC_OBJECT_H
#define DALI_GRAPHICS_GLES_SYNC_OBJECT_H

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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-sync-object-create-info.h>
#include <dali/graphics-api/graphics-sync-object.h>

// INTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-resource.h>

namespace Dali::Internal::Adaptor
{
class EglSyncImplementation;
class EglSyncObject;
} // namespace Dali::Internal::Adaptor

namespace Dali::Graphics::GLES
{
using SyncObjectResource = Resource<Graphics::SyncObject, Graphics::SyncObjectCreateInfo>;

/**
 * Class that maintains a glFenceSync object.
 */
class SyncObject : public SyncObjectResource
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo Valid createInfo structure
   * @param[in] controller Reference to the controller
   */
  SyncObject(const Graphics::SyncObjectCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~SyncObject() override;

  /**
   * @brief Called when GL resources are destroyed
   */
  void DestroyResource() override;

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  bool InitializeResource() override;

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;

  /**
   * Determine if the synchronisation object has been signalled.
   *
   * @return false if the sync object has not been signalled, true if it has been signalled (and
   * can now be destroyed)
   */
  bool IsSynced() override;

private:
  GLsync mGlSyncObject;
};

} // namespace Dali::Graphics::GLES

#endif //DALI_GRAPHICS_GLES_SYNC_OBJECT_H
