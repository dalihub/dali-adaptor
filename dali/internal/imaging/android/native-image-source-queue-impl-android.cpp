/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/android/native-image-source-queue-impl-android.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeImageSourceQueueAndroid* NativeImageSourceQueueAndroid::New(uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  NativeImageSourceQueueAndroid* image = new NativeImageSourceQueueAndroid(width, height, colorFormat, nativeImageSourceQueue);
  return image;
}

NativeImageSourceQueueAndroid::NativeImageSourceQueueAndroid(uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
: mWidth(width),
  mHeight(height)
{
  DALI_LOG_ERROR("NativeImageSourceQueueAndroid::NativeImageSourceQueueAndroid: Not supported\n");
}

NativeImageSourceQueueAndroid::~NativeImageSourceQueueAndroid()
{
}

Any NativeImageSourceQueueAndroid::GetNativeImageSourceQueue() const
{
  return Any();
}

void NativeImageSourceQueueAndroid::SetSize(uint32_t width, uint32_t height)
{
  mWidth  = width;
  mHeight = height;
}

void NativeImageSourceQueueAndroid::IgnoreSourceImage()
{
}

bool NativeImageSourceQueueAndroid::CanDequeueBuffer()
{
  return false;
}

uint8_t* NativeImageSourceQueueAndroid::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return nullptr;
}

bool NativeImageSourceQueueAndroid::EnqueueBuffer(uint8_t* buffer)
{
  return false;
}

void NativeImageSourceQueueAndroid::FreeReleasedBuffers()
{
}

bool NativeImageSourceQueueAndroid::CreateResource()
{
  return true;
}

void NativeImageSourceQueueAndroid::DestroyResource()
{
}

uint32_t NativeImageSourceQueueAndroid::TargetTexture()
{
  return 0;
}

void NativeImageSourceQueueAndroid::PrepareTexture()
{
}

bool NativeImageSourceQueueAndroid::ApplyNativeFragmentShader(std::string& shader)
{
  return false;
}

const char* NativeImageSourceQueueAndroid::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageSourceQueueAndroid::GetTextureTarget() const
{
  return GL_TEXTURE_2D;
}

Any NativeImageSourceQueueAndroid::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeImageSourceQueueAndroid::SourceChanged() const
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
