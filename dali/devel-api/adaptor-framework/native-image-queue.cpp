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

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/native-image-queue.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-factory.h>
#include <dali/internal/imaging/common/native-image-queue-impl.h>

namespace Dali
{
NativeImageQueuePtr NativeImageQueue::New(uint32_t width, uint32_t height, ColorFormat colorFormat)
{
  Any                 empty;
  NativeImageQueuePtr image = new NativeImageQueue(0, width, height, colorFormat, empty);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

NativeImageQueuePtr NativeImageQueue::New(uint32_t queueCount, uint32_t width, uint32_t height, ColorFormat colorFormat)
{
  Any                 empty;
  NativeImageQueuePtr image = new NativeImageQueue(queueCount, width, height, colorFormat, empty);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

NativeImageQueuePtr NativeImageQueue::New(Any nativeImageQueue)
{
  // ColorFormat will be ignored.
  NativeImageQueuePtr image = new NativeImageQueue(0, 0, 0, ColorFormat::BGRA8888, nativeImageQueue);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

Any NativeImageQueue::GetNativeImageQueue()
{
  return mImpl->GetNativeImageQueue();
}

void NativeImageQueue::SetSize(uint32_t width, uint32_t height)
{
  return mImpl->SetSize(width, height);
}

void NativeImageQueue::IgnoreSourceImage()
{
  mImpl->IgnoreSourceImage();
}

bool NativeImageQueue::CanDequeueBuffer()
{
  return mImpl->CanDequeueBuffer();
}

uint8_t* NativeImageQueue::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return mImpl->DequeueBuffer(width, height, stride, BufferAccessType::WRITE);
}

uint8_t* NativeImageQueue::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, BufferAccessType type)
{
  return mImpl->DequeueBuffer(width, height, stride, type);
}

bool NativeImageQueue::EnqueueBuffer(uint8_t* buffer)
{
  return mImpl->EnqueueBuffer(buffer);
}

void NativeImageQueue::CancelDequeuedBuffer(uint8_t* buffer)
{
  mImpl->CancelDequeuedBuffer(buffer);
}

void NativeImageQueue::FreeReleasedBuffers()
{
  mImpl->FreeReleasedBuffers();
}

void NativeImageQueue::SetQueueUsageHint(QueueUsageType type)
{
  mImpl->SetQueueUsageHint(type);
}

bool NativeImageQueue::CreateResource()
{
  return mImpl->CreateResource();
}

void NativeImageQueue::DestroyResource()
{
  mImpl->DestroyResource();
}

uint32_t NativeImageQueue::TargetTexture()
{
  return mImpl->TargetTexture();
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageQueue::PrepareTexture()
{
  return mImpl->PrepareTexture();
}

uint32_t NativeImageQueue::GetQueueCount() const
{
  return mImpl->GetQueueCount();
}

uint32_t NativeImageQueue::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t NativeImageQueue::GetHeight() const
{
  return mImpl->GetHeight();
}

bool NativeImageQueue::RequiresBlending() const
{
  return mImpl->RequiresBlending();
}

int NativeImageQueue::GetTextureTarget() const
{
  return mImpl->GetTextureTarget();
}

bool NativeImageQueue::ApplyNativeFragmentShader(std::string& shader)
{
  return ApplyNativeFragmentShader(shader, 1);
}

bool NativeImageQueue::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return mImpl->ApplyNativeFragmentShader(shader, mask);
}

const char* NativeImageQueue::GetCustomSamplerTypename() const
{
  return mImpl->GetCustomSamplerTypename();
}

Any NativeImageQueue::GetNativeImageHandle() const
{
  return mImpl->GetNativeImageHandle();
}

bool NativeImageQueue::SourceChanged() const
{
  return mImpl->SourceChanged();
}

Rect<uint32_t> NativeImageQueue::GetUpdatedArea()
{
  return mImpl->GetUpdatedArea();
}

void NativeImageQueue::PostRender()
{
  mImpl->PostRender();
}

NativeImageInterface::Extension* NativeImageQueue::GetExtension()
{
  return mImpl->GetNativeImageInterfaceExtension();
}

NativeImageQueue::NativeImageQueue(uint32_t queueCount, uint32_t width, uint32_t height, ColorFormat colorFormat, Any nativeImageQueue)
{
  auto factory = Dali::Internal::Adaptor::GetNativeImageFactory();
  mImpl        = factory->CreateNativeImageQueue(queueCount, width, height, colorFormat, nativeImageQueue).release();
}

NativeImageQueue::~NativeImageQueue()
{
  delete mImpl;
}

} // namespace Dali
