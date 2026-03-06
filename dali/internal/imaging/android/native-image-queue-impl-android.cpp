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
#include <dali/internal/imaging/android/native-image-queue-impl-android.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeImageQueueAndroid* NativeImageQueueAndroid::New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
{
  NativeImageQueueAndroid* image = new NativeImageQueueAndroid(queueCount, width, height, colorFormat, nativeImageQueue);
  return image;
}

NativeImageQueueAndroid::NativeImageQueueAndroid(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
: mQueueCount(queueCount),
  mWidth(width),
  mHeight(height)
{
  DALI_LOG_ERROR("NativeImageQueueAndroid::NativeImageQueueAndroid: Not supported\n");
}

NativeImageQueueAndroid::~NativeImageQueueAndroid()
{
}

Any NativeImageQueueAndroid::GetNativeImageQueue() const
{
  return Any();
}

void NativeImageQueueAndroid::SetSize(uint32_t width, uint32_t height)
{
  mWidth  = width;
  mHeight = height;
}

void NativeImageQueueAndroid::IgnoreSourceImage()
{
}

bool NativeImageQueueAndroid::CanDequeueBuffer()
{
  return false;
}

uint8_t* NativeImageQueueAndroid::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageQueue::BufferAccessType type)
{
  return nullptr;
}

bool NativeImageQueueAndroid::EnqueueBuffer(uint8_t* buffer)
{
  return false;
}

void NativeImageQueueAndroid::CancelDequeuedBuffer(uint8_t* buffer)
{
}

void NativeImageQueueAndroid::FreeReleasedBuffers()
{
}

bool NativeImageQueueAndroid::CreateResource()
{
  return true;
}

void NativeImageQueueAndroid::DestroyResource()
{
}

uint32_t NativeImageQueueAndroid::TargetTexture()
{
  return 0;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageQueueAndroid::PrepareTexture()
{
  return Dali::NativeImageInterface::PrepareTextureResult::NOT_SUPPORTED;
}

bool NativeImageQueueAndroid::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  return false;
}

const char* NativeImageQueueAndroid::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageQueueAndroid::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

Any NativeImageQueueAndroid::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeImageQueueAndroid::SourceChanged() const
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
