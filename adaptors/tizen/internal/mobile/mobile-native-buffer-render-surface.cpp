/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "mobile-native-buffer-render-surface.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>
#include <native-buffer-pool.h>

// INTERANL INCLUDES
#include <internal/common/gl/egl-implementation.h>
#include <internal/common/trigger-event.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gRenderSurfaceLogFilter;
#endif

namespace ECoreX
{

NativeBufferRenderSurface::NativeBufferRenderSurface( native_buffer_provider* provider,
                                                      native_buffer_pool* pool,
                                                      unsigned int maxBufferCount,
                                                      Dali::PositionSize positionSize,
                                                      Any surface,
                                                      Any display,
                                                      const std::string& name,
                                                      bool isTransparent )
: RenderSurface( Dali::RenderSurface::NATIVE_BUFFER, positionSize, surface, display, "native_buffer", isTransparent ),
  mProvider( provider ),
  mPool( pool ),
  mMaxBufferCount( maxBufferCount ),
  mIsAcquired( false )
{
  DALI_ASSERT_ALWAYS(maxBufferCount > 0);
  Init( surface );
}

NativeBufferRenderSurface::~NativeBufferRenderSurface()
{
  DALI_LOG_WARNING("%d native buffer will be destroyed\n", mBuffers.size());

  // destroy buffers
  NativeBufferContainer::iterator bufferIter;
  for (bufferIter = mBuffers.begin(); bufferIter != mBuffers.end(); ++bufferIter)
  {
    native_buffer_destroy((*bufferIter));
  }
  mBuffers.clear();
}

Ecore_X_Drawable NativeBufferRenderSurface::GetDrawable()
{
  return (Ecore_X_Drawable)0;
}

Dali::RenderSurface::SurfaceType NativeBufferRenderSurface::GetType()
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );
  return Dali::RenderSurface::NATIVE_BUFFER;
}

Any NativeBufferRenderSurface::GetSurface()
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );
  return Any();
}

void NativeBufferRenderSurface::InitializeEgl( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  EglImplementation& eglImpl = static_cast<EglImplementation&>( egl );
  eglImpl.InitializeGles( reinterpret_cast< EGLNativeDisplayType >( mMainDisplay ), false /* external surface */);

  eglImpl.ChooseConfig(false, mColorDepth);
}

native_buffer* NativeBufferRenderSurface::CreateNativeBuffer()
{
  native_buffer* buffer;

  buffer = native_buffer_create(mProvider, mPosition.width, mPosition.height,
    mColorDepth == COLOR_DEPTH_32? mPosition.width * 4 : mPosition.width * 3, /* stride will be deprecated */
    mColorDepth == COLOR_DEPTH_32? NATIVE_BUFFER_FORMAT_BGRA_8888 : NATIVE_BUFFER_FORMAT_RGB_888,
    NATIVE_BUFFER_USAGE_3D_RENDER);

  if(buffer)
  {
    // insert buffer to list
    mBuffers.push_back(buffer);
  }

  return buffer;
}

void NativeBufferRenderSurface::CreateEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  EglImplementation& eglImpl = static_cast<EglImplementation&>( egl );

  // create one buffer
  native_buffer_pool_add_buffer(mPool, CreateNativeBuffer());

  DALI_ASSERT_ALWAYS( native_buffer_pool_acquire_surface( mPool, eglImpl.GetDisplay(), eglImpl.GetContext() ) == STATUS_SUCCESS );
  mIsAcquired = true;
}

void NativeBufferRenderSurface::DestroyEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  // Remove buffers from pool
  native_buffer_pool_reset(mPool);
}

bool NativeBufferRenderSurface::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  return false;
}

bool NativeBufferRenderSurface::PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction )
{
  EglImplementation& eglImpl = static_cast<EglImplementation&>( egl );

  if(mIsAcquired)
  {
    mIsAcquired = false;
    return true;
  }

  // Attempt to acquire a surface for rendering
  while( !mIsStopped && native_buffer_pool_get_input_buffer_count(mPool) < 1 )
  {
    if(mBuffers.size() <= mMaxBufferCount)
    {
      // create one buffer
      native_buffer_pool_add_buffer(mPool, CreateNativeBuffer());
    }
    else
    {
      usleep( 5 * 1000 );  // polling per 5 msec
    }
  }

  if( !mIsStopped && native_buffer_pool_acquire_surface( mPool, eglImpl.GetDisplay(), eglImpl.GetContext() ) != STATUS_SUCCESS )
  {
    DALI_LOG_ERROR("Failed to acquire native buffer surface (# queue : %d)\n", native_buffer_pool_get_input_buffer_count(mPool));
  }

  return !mIsStopped; // fail if it is stopped
}

void NativeBufferRenderSurface::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, unsigned int timeDelta, SyncMode syncMode )
{
  glAbstraction.Flush();

  // release the surface to allow consumer usage
  if(native_buffer_pool_release_surface( mPool ) != STATUS_SUCCESS)
  {
    DALI_LOG_ERROR("Failed to release native buffer surface (# queue : %d)\n", native_buffer_pool_get_input_buffer_count(mPool));
  }

  // create damage for client applications which wish to know the update timing
  if( mRenderNotification )
  {
    // use notification trigger
    // Tell the event-thread to render the pixmap
    mRenderNotification->Trigger();
  }

  // Do render synchronisation
  DoRenderSync( timeDelta, syncMode );
}

void NativeBufferRenderSurface::CreateXRenderable()
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  // nothing to do
}

void NativeBufferRenderSurface::UseExistingRenderable( unsigned int surfaceId )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  // nothing to do
}

} // namespace ECoreX

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
