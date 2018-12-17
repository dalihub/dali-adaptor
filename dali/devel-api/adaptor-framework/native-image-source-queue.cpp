/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>
#include <dali/internal/imaging/common/native-image-source-factory.h>

namespace Dali
{

NativeImageSourceQueuePtr NativeImageSourceQueue::New( uint32_t width, uint32_t height, ColorDepth depth )
{
  Any empty;
  NativeImageSourceQueuePtr image = new NativeImageSourceQueue( width, height, depth, empty );
  if( image->mImpl )
  {
    return image;
  }
  return nullptr;
}

NativeImageSourceQueuePtr NativeImageSourceQueue::New( Any nativeImageSourceQueue )
{
  NativeImageSourceQueuePtr image = new NativeImageSourceQueue( 0, 0, COLOR_DEPTH_DEFAULT, nativeImageSourceQueue );
  if( image->mImpl )
  {
    return image;
  }
  return nullptr;
}

Any NativeImageSourceQueue::GetNativeImageSourceQueue()
{
  return mImpl->GetNativeImageSourceQueue();
}

void NativeImageSourceQueue::SetSize( uint32_t width, uint32_t height )
{
  return mImpl->SetSize( width, height );
}

bool NativeImageSourceQueue::GlExtensionCreate()
{
  return mImpl->GlExtensionCreate();
}

void NativeImageSourceQueue::GlExtensionDestroy()
{
  mImpl->GlExtensionDestroy();
}

uint32_t NativeImageSourceQueue::TargetTexture()
{
  return mImpl->TargetTexture();
}

void NativeImageSourceQueue::PrepareTexture()
{
  mImpl->PrepareTexture();
}

uint32_t NativeImageSourceQueue::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t NativeImageSourceQueue::GetHeight() const
{
  return mImpl->GetHeight();
}

bool NativeImageSourceQueue::RequiresBlending() const
{
  return mImpl->RequiresBlending();
}

NativeImageInterface::Extension* NativeImageSourceQueue::GetExtension()
{
  return mImpl->GetNativeImageInterfaceExtension();
}

NativeImageSourceQueue::NativeImageSourceQueue( uint32_t width, uint32_t height, ColorDepth depth, Any nativeImageSourceQueue )
{
  auto factory = Dali::Internal::Adaptor::GetNativeImageSourceFactory();
  mImpl = factory->CreateNativeImageSourceQueue( width, height, depth, nativeImageSourceQueue );
}

NativeImageSourceQueue::~NativeImageSourceQueue()
{
}

} // namespace Dali
