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
#include <dali/internal/imaging/ubuntu-x11/native-image-source-queue-impl-x.h>

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
namespace
{
#define TBM_SURFACE_QUEUE_SIZE 3
} // namespace

NativeImageSourceQueueX* NativeImageSourceQueueX::New(uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  NativeImageSourceQueueX* image = new NativeImageSourceQueueX(width, height, colorFormat, nativeImageSourceQueue);
  return image;
}

NativeImageSourceQueueX::NativeImageSourceQueueX(uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
: mWidth(width),
  mHeight(height)
{
  DALI_LOG_ERROR("NativeImageSourceQueueX::NativeImageSourceQueueX: Not supported\n");
}

NativeImageSourceQueueX::~NativeImageSourceQueueX()
{
}

Any NativeImageSourceQueueX::GetNativeImageSourceQueue() const
{
  return Any();
}

void NativeImageSourceQueueX::SetSize(uint32_t width, uint32_t height)
{
  mWidth  = width;
  mHeight = height;
}

void NativeImageSourceQueueX::IgnoreSourceImage()
{
}

bool NativeImageSourceQueueX::CanDequeueBuffer()
{
  return false;
}

uint8_t* NativeImageSourceQueueX::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return nullptr;
}

bool NativeImageSourceQueueX::EnqueueBuffer(uint8_t* buffer)
{
  return false;
}

void NativeImageSourceQueueX::FreeReleasedBuffers()
{
}

bool NativeImageSourceQueueX::CreateResource()
{
  return true;
}

void NativeImageSourceQueueX::DestroyResource()
{
}

uint32_t NativeImageSourceQueueX::TargetTexture()
{
  return 0;
}

void NativeImageSourceQueueX::PrepareTexture()
{
}

bool NativeImageSourceQueueX::ApplyNativeFragmentShader(std::string& shader)
{
  return false;
}

const char* NativeImageSourceQueueX::GetCustomSamplerTypename() const
{
  return nullptr;
}

int NativeImageSourceQueueX::GetTextureTarget() const
{
  return 0;
}

Any NativeImageSourceQueueX::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeImageSourceQueueX::SourceChanged() const
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
