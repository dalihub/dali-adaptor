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
#include <dali/internal/imaging/tizen/native-image-source-queue-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
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

const char* FRAGMENT_PREFIX = "#extension GL_OES_EGL_image_external:require\n";
const char* SAMPLER_TYPE = "samplerExternalOES";

int FORMATS_BLENDING_REQUIRED[] = {
  TBM_FORMAT_ARGB4444, TBM_FORMAT_ABGR4444,
  TBM_FORMAT_RGBA4444, TBM_FORMAT_BGRA4444,
  TBM_FORMAT_RGBX5551, TBM_FORMAT_BGRX5551,
  TBM_FORMAT_ARGB1555, TBM_FORMAT_ABGR1555,
  TBM_FORMAT_RGBA5551, TBM_FORMAT_BGRA5551,
  TBM_FORMAT_ARGB8888, TBM_FORMAT_ABGR8888,
  TBM_FORMAT_RGBA8888, TBM_FORMAT_BGRA8888,
  TBM_FORMAT_ARGB2101010, TBM_FORMAT_ABGR2101010,
  TBM_FORMAT_RGBA1010102, TBM_FORMAT_BGRA1010102
};

const int NUM_FORMATS_BLENDING_REQUIRED = 18;

}

NativeImageSourceQueueTizen* NativeImageSourceQueueTizen::New( unsigned int width, unsigned int height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
{
  NativeImageSourceQueueTizen* image = new NativeImageSourceQueueTizen( width, height, depth, nativeImageSourceQueue );
  DALI_ASSERT_DEBUG( image && "NativeImageSourceQueueTizen allocation failed." );

  if( image )
  {
    image->Initialize( depth );
  }

  return image;
}

NativeImageSourceQueueTizen::NativeImageSourceQueueTizen( unsigned int width, unsigned int height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
  : mWidth( width ),
    mHeight( height ),
    mTbmQueue( NULL ),
    mConsumeSurface( NULL ),
    mOwnTbmQueue( false ),
    mBlendingRequired( false )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );
  mTbmQueue = GetSurfaceFromAny( nativeImageSourceQueue );

  if( mTbmQueue != NULL )
  {
    mBlendingRequired = CheckBlending( tbm_surface_queue_get_format( mTbmQueue ) );
    mWidth = tbm_surface_queue_get_width( mTbmQueue );
    mHeight = tbm_surface_queue_get_height( mTbmQueue );
  }
}

NativeImageSourceQueueTizen::~NativeImageSourceQueueTizen()
{
  if( mOwnTbmQueue )
  {
    DestroyQueue();
  }
}

void NativeImageSourceQueueTizen::Initialize( Dali::NativeImageSourceQueue::ColorDepth depth )
{
  if( mTbmQueue != NULL || mWidth == 0 || mHeight == 0 )
  {
    return;
  }

  int format = TBM_FORMAT_ARGB8888;

  switch( depth )
  {
    case Dali::NativeImageSourceQueue::COLOR_DEPTH_DEFAULT:
    case Dali::NativeImageSourceQueue::COLOR_DEPTH_32:
    {
      format = TBM_FORMAT_ARGB8888;
      mBlendingRequired = true;
      break;
    }
    case Dali::NativeImageSourceQueue::COLOR_DEPTH_24:
    {
      format = TBM_FORMAT_RGB888;
      mBlendingRequired = false;
      break;
    }
    default:
    {
      DALI_LOG_WARNING( "Wrong color depth.\n" );
      return;
    }
  }

  mTbmQueue = tbm_surface_queue_create( TBM_SURFACE_QUEUE_SIZE, mWidth, mHeight, format, 0 );

  mOwnTbmQueue = true;
}

tbm_surface_queue_h NativeImageSourceQueueTizen::GetSurfaceFromAny( Any source ) const
{
  if( source.Empty() )
  {
    return NULL;
  }

  if( source.GetType() == typeid( tbm_surface_queue_h ) )
  {
    return AnyCast< tbm_surface_queue_h >( source );
  }
  else
  {
    return NULL;
  }
}

Any NativeImageSourceQueueTizen::GetNativeImageSourceQueue() const
{
  return Any( mTbmQueue );
}

void NativeImageSourceQueueTizen::SetSource( Any source )
{
  if( mOwnTbmQueue )
  {
    DestroyQueue();
  }

  mTbmQueue = GetSurfaceFromAny( source );

  if( mTbmQueue != NULL )
  {
    mBlendingRequired = CheckBlending( tbm_surface_queue_get_format( mTbmQueue ) );
    mWidth = tbm_surface_queue_get_width( mTbmQueue );
    mHeight = tbm_surface_queue_get_height( mTbmQueue );
  }
}

bool NativeImageSourceQueueTizen::GlExtensionCreate()
{
  return true;
}

void NativeImageSourceQueueTizen::GlExtensionDestroy()
{

}

unsigned int NativeImageSourceQueueTizen::TargetTexture()
{
  return 0;
}

void NativeImageSourceQueueTizen::PrepareTexture()
{
  tbm_surface_h oldSurface = mConsumeSurface;

  bool needToWait = ( mConsumeSurface == NULL ) ? true : false;

  if( tbm_surface_queue_can_acquire( mTbmQueue, needToWait ) )
  {
    if( tbm_surface_queue_acquire( mTbmQueue, &mConsumeSurface ) != TBM_SURFACE_QUEUE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to aquire a tbm_surface\n" );
      return;
    }
  }

  if( oldSurface && oldSurface != mConsumeSurface )
  {
    if( tbm_surface_internal_is_valid( oldSurface ) )
    {
      tbm_surface_queue_release( mTbmQueue, oldSurface );
    }
  }
}

const char* NativeImageSourceQueueTizen::GetCustomFragmentPreFix()
{
  return FRAGMENT_PREFIX;
}

const char* NativeImageSourceQueueTizen::GetCustomSamplerTypename()
{
  return SAMPLER_TYPE;
}

int NativeImageSourceQueueTizen::GetEglImageTextureTarget()
{
  return -1;
}

Any NativeImageSourceQueueTizen::GetNativeImageHandle() const
{
  return Any( 0 );
}

bool NativeImageSourceQueueTizen::IsSetSource() const
{
  return false;
}

void NativeImageSourceQueueTizen::DestroyQueue()
{
  if( mConsumeSurface )
  {
    tbm_surface_internal_unref( mConsumeSurface );

    if( tbm_surface_internal_is_valid( mConsumeSurface ) )
    {
      tbm_surface_queue_release( mTbmQueue, mConsumeSurface );
    }
  }

  if( mTbmQueue != NULL )
  {
    tbm_surface_queue_destroy( mTbmQueue );
  }

  mTbmQueue = NULL;
  mOwnTbmQueue = false;
}

bool NativeImageSourceQueueTizen::CheckBlending( int format )
{
  for( int i = 0; i < NUM_FORMATS_BLENDING_REQUIRED; ++i )
  {
    if( format == FORMATS_BLENDING_REQUIRED[i] )
    {
      return true;
    }
  }

  return false;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
