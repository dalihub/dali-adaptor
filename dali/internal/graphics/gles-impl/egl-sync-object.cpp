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

// CLASS HEADER
#include <dali/internal/graphics/gles-impl/egl-sync-object.h>

// EXTERNAL HEADERS

// INTERNAL HEADERS
#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <dali/internal/graphics/gles/egl-sync-implementation.h>

namespace Dali::Graphics::EGL
{
SyncObject::SyncObject(const Graphics::SyncObjectCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: SyncObjectResource(createInfo, controller),
  mEglSyncImplementation(controller.GetEglSyncImplementation()),
  mEglSyncObject(nullptr)
{
}

SyncObject::~SyncObject()
{
}

void SyncObject::DestroyResource()
{
}

bool SyncObject::InitializeResource()
{
  // Initialized not from a resource queue, but from a command.
  mEglSyncObject = static_cast<Internal::Adaptor::EglSyncObject*>(mEglSyncImplementation.CreateSyncObject());
  return true;
}

void SyncObject::DiscardResource()
{
  // Called from custom deleter.
  // Don't use discard queue, drop immediately.
  mEglSyncImplementation.DestroySyncObject(mEglSyncObject);
  mEglSyncObject = nullptr;
}

bool SyncObject::IsSynced()
{
  if(mEglSyncObject)
  {
    return mEglSyncObject->IsSynced();
  }
  return false;
}

} // namespace Dali::Graphics::EGL
