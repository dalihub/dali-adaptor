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
#include "native-buffer-plugin-impl.h"

// EXTERNAL HEADERS
#include <Ecore_X.h>

#include <dali/public-api/dali-core.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <internal/common/accessibility-manager-impl.h>
#include <internal/common/adaptor-impl.h>
#include "mobile-render-surface-factory.h"
#include <internal/common/ecore-x/ecore-x-render-surface.h>
#include <internal/common/trigger-event.h>

namespace Dali
{

namespace SlpPlatform
{

class SlpPlatformAbstraction;

} // namespace SlpPlatform

namespace Integration
{

class Core;

} // namespace Integration

namespace Internal
{

namespace Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gLogFilter  = Integration::Log::Filter::New(Debug::Verbose, true, "LOG_NATIVE_BUFFER_PLUGIN");
#endif

} // namespace

NativeBufferPlugin::NativeBufferPlugin(Dali::NativeBufferPlugin& nbPlugin, unsigned int initialWidth, unsigned int initialHeight, bool isTransparent, unsigned int maxBufferCount, Dali::RenderSurface::RenderMode mode, const DeviceLayout& baseLayout )
: mNativeBufferPlugin( nbPlugin ),
  mAdaptor( NULL ),
  mSurface( NULL ),
  mRenderNotification( NULL ),
  mState( Stopped ),
  mInitialized( false ),
  mFirstRenderCompleteNotified( false )
{
  // create provider
  mProvider = native_buffer_provider_create(NATIVE_BUFFER_PROVIDER_CORE);

  //create pool
  mPool = native_buffer_pool_create( mProvider );

  // create surface
  mSurface = CreateSurface( initialWidth, initialHeight, isTransparent, maxBufferCount );

  // create adaptor
  CreateAdaptor( *mSurface, baseLayout );

  // render notification
  mRenderNotification = new TriggerEvent( boost::bind(&NativeBufferPlugin::OnRender, this ) );
  mSurface->SetRenderNotification( mRenderNotification );
  mSurface->SetRenderMode(mode);

  mState = Ready;
}

NativeBufferPlugin::~NativeBufferPlugin()
{
  if( mAdaptor )
  {
    Stop();

    // no more notifications
    delete mRenderNotification;

    delete mAdaptor;
    delete mSurface;

    native_buffer_pool_destroy(mPool);
    native_buffer_provider_destroy(mProvider);
  }
}

void NativeBufferPlugin::CreateAdaptor( ECoreX::RenderSurface &surface, const DeviceLayout& baseLayout )
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  mAdaptor = Internal::Adaptor::Adaptor::New( &surface, baseLayout );
}

ECoreX::RenderSurface* NativeBufferPlugin::CreateSurface( int width, int height, bool isTransparent, unsigned int maxBufferCount )
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  PositionSize positionSize( 0, 0, width, height );
  Any surface;
  Any display;

  // if we already have surface, reuse its display
  if( mSurface )
  {
    display = mSurface->GetMainDisplay();
  }

  ECoreX::RenderSurface* nbSurface = ECoreX::CreateNativeBufferSurface( mProvider, mPool, maxBufferCount, positionSize, surface, display, "no name", isTransparent );

  return nbSurface;
}

void NativeBufferPlugin::ChangeSurfaceSize(unsigned int width, unsigned int height)
{
  // TODO: not yet implemented
  DALI_LOG_WARNING("Not yet implemented!\n");
}

Vector2 NativeBufferPlugin::GetBufferCount()
{
  Vector2 retVal;
  retVal.x = native_buffer_pool_get_input_buffer_count(mPool);
  retVal.y = native_buffer_pool_get_output_buffer_count(mPool);

  return retVal;
}

void NativeBufferPlugin::Run()
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  if(mState == Ready)
  {
    // Run the adaptor
    mAdaptor->Start();
    mState = Running;

    OnInit();
  }
}

void NativeBufferPlugin::Pause()
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  if(mState == Running)
  {
    mAdaptor->Pause();
    mState = Suspended;

    mPauseSignalV2.Emit( mNativeBufferPlugin );
  }
}

void NativeBufferPlugin::Resume()
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  if(mState == Suspended)
  {
    mAdaptor->Resume();
    mState = Running;

    mResumeSignalV2.Emit( mNativeBufferPlugin );
  }
}

void NativeBufferPlugin::Stop()
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  if(mState != Stopped)
  {
    // Stop the adaptor
    mAdaptor->Stop();
    mState = Stopped;

    mTerminateSignalV2.Emit( mNativeBufferPlugin );
  }
}

void NativeBufferPlugin::OnInit()
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  mInitialized = true;
  mInitSignalV2.Emit( mNativeBufferPlugin );
}

/*
  * app can poll the get buffer after first rendered signal
  * Otherwise, app can get render signal whenever it rendered
  */
void NativeBufferPlugin::OnFirstRenderCompleted()
{
  DALI_LOG_TRACE_METHOD( gLogFilter );

  mFirstRenderCompletedSignalV2.Emit( mNativeBufferPlugin );
  mFirstRenderCompleteNotified = true;
}

/*
  * app can poll the get buffer after first rendered signal
  * Otherwise, app can get render signal whenever it rendered
  */
void NativeBufferPlugin::OnRender()
{
  // dirty set while adaptor is running
  if( Running == mState )
  {
    mRenderSignalV2.Emit( mNativeBufferPlugin );
  }
}

// TODO: need it?
void NativeBufferPlugin::RenderSync()
{
  if( NULL != mAdaptor )
  {
    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).RenderSync();
  }
}

native_buffer* NativeBufferPlugin::GetNativeBufferFromOutput()
{
  return native_buffer_pool_get_buffer(mPool);
}

bool NativeBufferPlugin::AddNativeBufferToInput(native_buffer* nativeBuffer)
{
  status_t result = native_buffer_pool_add_buffer(mPool, nativeBuffer);

  if(result != STATUS_SUCCESS)
  {
    DALI_LOG_WARNING("native_buffer_pool_add_buffer returns %d\n", result);
    return false;
  }

  return true;
}


} // namespace Adaptor

} // namespace Internal

} // namespace Dali
