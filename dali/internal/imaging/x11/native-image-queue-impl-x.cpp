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
#include <dali/internal/imaging/x11/native-image-queue-impl-x.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeImageQueueX* NativeImageQueueX::New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
{
  NativeImageQueueX* image = new NativeImageQueueX(queueCount, width, height, colorFormat, nativeImageQueue);
  return image;
}

NativeImageQueueX::NativeImageQueueX(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
: mQueueCount(queueCount),
  mWidth(width),
  mHeight(height)
{
  DALI_LOG_ERROR("NativeImageQueueX::NativeImageQueueX: Not supported\n");
}

NativeImageQueueX::~NativeImageQueueX()
{
}

Any NativeImageQueueX::GetNativeImageQueue() const
{
  return Any();
}

void NativeImageQueueX::SetSize(uint32_t width, uint32_t height)
{
  mWidth  = width;
  mHeight = height;
}

void NativeImageQueueX::IgnoreSourceImage()
{
}

bool NativeImageQueueX::CanDequeueBuffer()
{
  return false;
}

uint8_t* NativeImageQueueX::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageQueue::BufferAccessType type)
{
  return nullptr;
}

bool NativeImageQueueX::EnqueueBuffer(uint8_t* buffer)
{
  return false;
}

void NativeImageQueueX::CancelDequeuedBuffer(uint8_t* buffer)
{
}

void NativeImageQueueX::FreeReleasedBuffers()
{
}

bool NativeImageQueueX::CreateResource()
{
  return true;
}

void NativeImageQueueX::DestroyResource()
{
}

uint32_t NativeImageQueueX::TargetTexture()
{
  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageQueueX::PrepareTexture()
{
  return Dali::NativeImageInterface::PrepareTextureResult::NOT_SUPPORTED;
}

bool NativeImageQueueX::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return false;
}

const char* NativeImageQueueX::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageQueueX::GetTextureTarget() const
{
  return 0;
}

Any NativeImageQueueX::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeImageQueueX::SourceChanged() const
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
