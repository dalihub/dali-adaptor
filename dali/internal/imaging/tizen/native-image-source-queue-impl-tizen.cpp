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
#include <dali/internal/imaging/tizen/native-image-source-queue-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
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

NativeImageSourceQueueTizen* NativeImageSourceQueueTizen::New( uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
{
  NativeImageSourceQueueTizen* image = new NativeImageSourceQueueTizen( width, height, depth, nativeImageSourceQueue );
  DALI_ASSERT_DEBUG( image && "NativeImageSourceQueueTizen allocation failed." );

  if( image )
  {
    image->Initialize( depth );
  }

  return image;
}

NativeImageSourceQueueTizen::NativeImageSourceQueueTizen( uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
: mMutex(),
  mWidth( width ),
  mHeight( height ),
  mTbmQueue( NULL ),
  mConsumeSurface( NULL ),
  mEglImages(),
  mEglGraphics( NULL ),
  mEglImageExtensions( NULL ),
  mOwnTbmQueue( false ),
  mBlendingRequired( false )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );

  GraphicsInterface* graphics = &( Adaptor::GetImplementation( Adaptor::Get() ).GetGraphicsInterface() );
  mEglGraphics = static_cast<EglGraphics *>(graphics);

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
    if( mTbmQueue != NULL )
    {
      tbm_surface_queue_destroy( mTbmQueue );
    }
  }
}

void NativeImageSourceQueueTizen::Initialize( Dali::NativeImageSourceQueue::ColorDepth depth )
{
  if( mWidth == 0 || mHeight == 0 )
  {
    return;
  }

  if( mTbmQueue == NULL )
  {
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
    if( !mTbmQueue )
    {
      DALI_LOG_ERROR( "NativeImageSourceQueueTizen::Initialize: tbm_surface_queue_create is failed! [%p]\n", mTbmQueue );
      return;
    }

    mOwnTbmQueue = true;
  }
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

void NativeImageSourceQueueTizen::SetSize( uint32_t width, uint32_t height )
{
  Dali::Mutex::ScopedLock lock( mMutex );

  tbm_surface_queue_reset( mTbmQueue, width, height, tbm_surface_queue_get_format( mTbmQueue ) );

  mWidth = width;
  mHeight = height;

  ResetEglImageList();
}

bool NativeImageSourceQueueTizen::GlExtensionCreate()
{
  mEglImageExtensions = mEglGraphics->GetImageExtensions();
  DALI_ASSERT_DEBUG( mEglImageExtensions );

  return true;
}

void NativeImageSourceQueueTizen::GlExtensionDestroy()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  ResetEglImageList();
}

uint32_t NativeImageSourceQueueTizen::TargetTexture()
{
  return 0;
}

void NativeImageSourceQueueTizen::PrepareTexture()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  tbm_surface_h oldSurface = mConsumeSurface;

  if( tbm_surface_queue_can_acquire( mTbmQueue, 0 ) )
  {
    if( tbm_surface_queue_acquire( mTbmQueue, &mConsumeSurface ) != TBM_SURFACE_QUEUE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to aquire a tbm_surface\n" );
      return;
    }

    if( oldSurface )
    {
      if( tbm_surface_internal_is_valid( oldSurface ) )
      {
        tbm_surface_queue_release( mTbmQueue, oldSurface );
      }
    }

    if( mConsumeSurface )
    {
      bool existing = false;
      for( auto&& iter : mEglImages )
      {
        if( iter.first == mConsumeSurface )
        {
          // Find the surface in the existing list
          existing = true;
          mEglImageExtensions->TargetTextureKHR( iter.second );
          break;
        }
      }

      if( !existing )
      {
        // Push the surface
        tbm_surface_internal_ref( mConsumeSurface );

        void* eglImageKHR = mEglImageExtensions->CreateImageKHR( reinterpret_cast< EGLClientBuffer >( mConsumeSurface ) );
        mEglImageExtensions->TargetTextureKHR( eglImageKHR );

        mEglImages.push_back( EglImagePair( mConsumeSurface, eglImageKHR) );
      }
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
  return GL_TEXTURE_EXTERNAL_OES;
}

void NativeImageSourceQueueTizen::ResetEglImageList()
{
  if( mConsumeSurface )
  {
    if( tbm_surface_internal_is_valid( mConsumeSurface ) )
    {
      tbm_surface_queue_release( mTbmQueue, mConsumeSurface );
    }
    mConsumeSurface = NULL;
  }

  for( auto&& iter : mEglImages )
  {
    mEglImageExtensions->DestroyImageKHR( iter.second );

    tbm_surface_internal_unref( iter.first );
  }
  mEglImages.clear();
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
