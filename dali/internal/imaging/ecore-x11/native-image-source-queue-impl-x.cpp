/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/ecore-x11/native-image-source-queue-impl-x.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#define TBM_SURFACE_QUEUE_SIZE  3

const char* FRAGMENT_PREFIX = "\n";
const char* SAMPLER_TYPE = "sampler2D";

}

NativeImageSourceQueueX* NativeImageSourceQueueX::New( uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
{
  NativeImageSourceQueueX* image = new NativeImageSourceQueueX( width, height, depth, nativeImageSourceQueue );
  return image;
}

NativeImageSourceQueueX::NativeImageSourceQueueX( uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
: mWidth( width ),
  mHeight( height )
{
  DALI_LOG_ERROR( "NativeImageSourceQueueX::NativeImageSourceQueueX: Not supported\n" );
}

NativeImageSourceQueueX::~NativeImageSourceQueueX()
{
}

Any NativeImageSourceQueueX::GetNativeImageSourceQueue() const
{
  return Any();
}

void NativeImageSourceQueueX::SetSource( Any source )
{
}

void NativeImageSourceQueueX::SetSize( uint32_t width, uint32_t height )
{
  mWidth = width;
  mHeight = height;
}

bool NativeImageSourceQueueX::GlExtensionCreate()
{
  return true;
}

void NativeImageSourceQueueX::GlExtensionDestroy()
{
}

uint32_t NativeImageSourceQueueX::TargetTexture()
{
  return 0;
}

void NativeImageSourceQueueX::PrepareTexture()
{
}

const char* NativeImageSourceQueueX::GetCustomFragmentPreFix()
{
  return FRAGMENT_PREFIX;
}

const char* NativeImageSourceQueueX::GetCustomSamplerTypename()
{
  return SAMPLER_TYPE;
}

int NativeImageSourceQueueX::GetEglImageTextureTarget()
{
  return 0;
}

Any NativeImageSourceQueueX::GetNativeImageHandle() const
{
  return Any( 0 );
}

bool NativeImageSourceQueueX::IsSetSource() const
{
  return false;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
