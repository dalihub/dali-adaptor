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
 *
 */

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-factory.h>
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>

namespace Dali
{
NativeImageSourceQueuePtr NativeImageSourceQueue::New(uint32_t width, uint32_t height, ColorFormat colorFormat)
{
  Any                       empty;
  NativeImageSourceQueuePtr image = new NativeImageSourceQueue(width, height, colorFormat, empty);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

NativeImageSourceQueuePtr NativeImageSourceQueue::New(Any nativeImageSourceQueue)
{
  //ColorFormat will be ignored.
  NativeImageSourceQueuePtr image = new NativeImageSourceQueue(0, 0, ColorFormat::BGRA8888, nativeImageSourceQueue);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

Any NativeImageSourceQueue::GetNativeImageSourceQueue()
{
  return mImpl->GetNativeImageSourceQueue();
}

void NativeImageSourceQueue::SetSize(uint32_t width, uint32_t height)
{
  return mImpl->SetSize(width, height);
}

void NativeImageSourceQueue::IgnoreSourceImage()
{
  mImpl->IgnoreSourceImage();
}

bool NativeImageSourceQueue::CanDequeueBuffer()
{
  return mImpl->CanDequeueBuffer();
}

uint8_t* NativeImageSourceQueue::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return mImpl->DequeueBuffer(width, height, stride);
}

bool NativeImageSourceQueue::EnqueueBuffer(uint8_t* buffer)
{
  return mImpl->EnqueueBuffer(buffer);
}

void NativeImageSourceQueue::FreeReleasedBuffers()
{
  mImpl->FreeReleasedBuffers();
}

bool NativeImageSourceQueue::CreateResource()
{
  return mImpl->CreateResource();
}

void NativeImageSourceQueue::DestroyResource()
{
  mImpl->DestroyResource();
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

int NativeImageSourceQueue::GetTextureTarget() const
{
  return mImpl->GetTextureTarget();
}

bool NativeImageSourceQueue::ApplyNativeFragmentShader(std::string& shader)
{
  return mImpl->ApplyNativeFragmentShader(shader);
}

const char* NativeImageSourceQueue::GetCustomSamplerTypename() const
{
  return mImpl->GetCustomSamplerTypename();
}

Any NativeImageSourceQueue::GetNativeImageHandle() const
{
  return mImpl->GetNativeImageHandle();
}

bool NativeImageSourceQueue::SourceChanged() const
{
  return mImpl->SourceChanged();
}

NativeImageInterface::Extension* NativeImageSourceQueue::GetExtension()
{
  return mImpl->GetNativeImageInterfaceExtension();
}

NativeImageSourceQueue::NativeImageSourceQueue(uint32_t width, uint32_t height, ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  auto factory = Dali::Internal::Adaptor::GetNativeImageSourceFactory();
  mImpl        = factory->CreateNativeImageSourceQueue(width, height, colorFormat, nativeImageSourceQueue);
}

NativeImageSourceQueue::~NativeImageSourceQueue()
{
}

} // namespace Dali
