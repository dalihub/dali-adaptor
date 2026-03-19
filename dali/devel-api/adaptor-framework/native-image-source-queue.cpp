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
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>
#include <dali/integration-api/string-utils.h>
#include <dali/internal/imaging/common/native-image-factory.h>
#include <dali/internal/imaging/common/native-image-queue-impl.h>

using Dali::Integration::ToDaliString;
using Dali::Integration::ToStdString;

namespace Dali
{
NativeImageSourceQueuePtr NativeImageSourceQueue::New(uint32_t width, uint32_t height, ColorFormat colorFormat)
{
  Any                       empty;
  NativeImageSourceQueuePtr image = new NativeImageSourceQueue(0, width, height, colorFormat, empty);
  if(image->mImpl)
  {
    return image;
  }
  return nullptr;
}

Any NativeImageSourceQueue::GetNativeImageSourceQueue()
{
  return mImpl->GetNativeImageQueue();
}

void NativeImageSourceQueue::SetSize(uint32_t width, uint32_t height)
{
  return mImpl->SetSize(width, height);
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

Dali::NativeImageInterface::PrepareTextureResult NativeImageSourceQueue::PrepareTexture()
{
  return mImpl->PrepareTexture();
}

uint32_t NativeImageSourceQueue::GetQueueCount() const
{
  return mImpl->GetQueueCount();
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

bool NativeImageSourceQueue::ApplyNativeFragmentShader(String& shader)
{
  return ApplyNativeFragmentShader(shader, 1);
}

bool NativeImageSourceQueue::ApplyNativeFragmentShader(String& shader, int mask)
{
  std::string stdShader   = ToStdString(shader);
  bool        returnValue = mImpl->ApplyNativeFragmentShader(stdShader, mask);
  shader                  = ToDaliString(stdShader);
  return returnValue;
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

Rect<uint32_t> NativeImageSourceQueue::GetUpdatedArea()
{
  return mImpl->GetUpdatedArea();
}

void NativeImageSourceQueue::PostRender()
{
  mImpl->PostRender();
}

NativeImageSourceQueue::NativeImageSourceQueue(uint32_t queueCount, uint32_t width, uint32_t height, ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  auto factory = Dali::Internal::Adaptor::GetNativeImageFactory();
  mImpl        = factory->CreateNativeImageQueue(queueCount, width, height, static_cast<NativeImageQueue::ColorFormat>(colorFormat), nativeImageSourceQueue).release();
}

NativeImageSourceQueue::~NativeImageSourceQueue()
{
  delete mImpl;
}

} // namespace Dali
