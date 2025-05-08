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
 */

// CLASS HEADER
#include <dali/internal/graphics/gles-impl/gles-sync-object.h>

// EXTERNAL HEADERS

// INTERNAL HEADERS
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>

namespace Dali::Graphics::GLES
{
SyncObject::SyncObject(const Graphics::SyncObjectCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: SyncObjectResource(createInfo, controller),
  mGlSyncObject(0)
{
}

SyncObject::~SyncObject()
{
}

void SyncObject::DestroyResource()
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    auto* gl = mController.GetGL();
    if(DALI_LIKELY(gl))
    {
      gl->DeleteSync(mGlSyncObject);
    }
    mGlSyncObject = 0;
  }
}

bool SyncObject::InitializeResource()
{
  // Initialized not from a resource queue, but from a command.
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl))
  {
    mGlSyncObject = gl->FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }
  return true;
}

void SyncObject::DiscardResource()
{
  // Don't use discard queue, drop immediately.
  DALI_ASSERT_ALWAYS(false && "SyncObject don't support DiscardResource() method. Please check your code.");
}

bool SyncObject::IsSynced()
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl) && mGlSyncObject)
  {
    GLenum result = gl->ClientWaitSync(mGlSyncObject, 0, 0ull);
    return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
  }
  return false;
}

} // namespace Dali::Graphics::GLES
