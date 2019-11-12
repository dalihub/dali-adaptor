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
#include <dali/internal/system/tizen-wayland/tizen-wearable/capture-impl.h>

// EXTERNAL INCLUDES
#include <fstream>
#include <string.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>

namespace
{
unsigned int TIME_OUT_DURATION = 1000;
}

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

Capture::Capture()
: mTimer(),
  mPath(),
  mNativeImageSourcePtr( NULL ),
  mTbmSurface( NULL )
{
}

Capture::Capture( Dali::CameraActor cameraActor )
: mCameraActor( cameraActor ),
  mTimer(),
  mPath(),
  mNativeImageSourcePtr( NULL ),
  mTbmSurface( NULL )
{
}

Capture::~Capture()
{
}

CapturePtr Capture::New()
{
  CapturePtr pWorker = new Capture();

  return pWorker;
}

CapturePtr Capture::New( Dali::CameraActor cameraActor )
{
  CapturePtr pWorker = new Capture( cameraActor );

  return pWorker;
}

void Capture::Start( Dali::Actor source, const Dali::Vector2& size, const std::string &path, const Dali::Vector4& clearColor )
{
  DALI_ASSERT_ALWAYS(path.size() > 4 && "Path is invalid.");

  // Increase the reference count focely to avoid application mistake.
  Reference();

  mPath = path;

  DALI_ASSERT_ALWAYS(source && "Source is NULL.");

  UnsetResources();
  SetupResources( size, clearColor, source );
}

Dali::Capture::CaptureFinishedSignalType& Capture::FinishedSignal()
{
  return mFinishedSignal;
}

void Capture::CreateSurface( const Vector2& size )
{
  DALI_ASSERT_ALWAYS(!mTbmSurface && "mTbmSurface is already created.");

  mTbmSurface = tbm_surface_create( size.width, size.height, TBM_FORMAT_RGBA8888 );
}

void Capture::DeleteSurface()
{
  DALI_ASSERT_ALWAYS(mTbmSurface && "mTbmSurface is empty.");

  tbm_surface_destroy( mTbmSurface );
  mTbmSurface = NULL;
}

void Capture::ClearSurface( const Vector2& size )
{
  DALI_ASSERT_ALWAYS(mTbmSurface && "mTbmSurface is empty.");

  tbm_surface_info_s surface_info;

  if( tbm_surface_map( mTbmSurface, TBM_SURF_OPTION_WRITE, &surface_info ) == TBM_SURFACE_ERROR_NONE )
  {
    //DALI_ASSERT_ALWAYS(surface_info.bpp == 32 && "unsupported tbm format");

    unsigned char* ptr = surface_info.planes[0].ptr;
    memset( ptr, 0, surface_info.size ); // TODO: support color

    if( tbm_surface_unmap( mTbmSurface ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Fail to unmap tbm_surface\n" );
    }
  }
  else
  {
     DALI_ASSERT_ALWAYS(0 && "tbm_surface_map failed");
  }
}

bool Capture::IsSurfaceCreated()
{
  return mTbmSurface != 0;
}

void Capture::CreateNativeImageSource()
{
  Dali::Adaptor& adaptor = Dali::Adaptor::Get();

  DALI_ASSERT_ALWAYS(adaptor.IsAvailable() && "Dali::Adaptor is not available.");

  DALI_ASSERT_ALWAYS(mTbmSurface && "mTbmSurface is empty.");

  DALI_ASSERT_ALWAYS(!mNativeImageSourcePtr && "NativeImageSource is already created.");

  // create the NativeImageSource object with our surface
  mNativeImageSourcePtr = Dali::NativeImageSource::New( mTbmSurface );
}

void Capture::DeleteNativeImageSource()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "mNativeImageSource is NULL.");

  mNativeImageSourcePtr.Reset();
}

bool Capture::IsNativeImageSourceCreated()
{
  return mNativeImageSourcePtr;
}

void Capture::CreateFrameBuffer()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "NativeImageSource is NULL.");

  DALI_ASSERT_ALWAYS(!mFrameBuffer && "FrameBuffer is already created.");

  mNativeTexture = Dali::Texture::New( *mNativeImageSourcePtr );

  // Create a FrameBuffer object with no default attachments.
  mFrameBuffer = Dali::FrameBuffer::New( mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), Dali::FrameBuffer::Attachment::NONE );
  // Add a color attachment to the FrameBuffer object.
  mFrameBuffer.AttachColorTexture( mNativeTexture );
}

void Capture::DeleteFrameBuffer()
{
  DALI_ASSERT_ALWAYS(mFrameBuffer && "FrameBuffer is NULL.");

  mFrameBuffer.Reset();
  mNativeTexture.Reset();
}

bool Capture::IsFrameBufferCreated()
{
  return mFrameBuffer;
}

void Capture::SetupRenderTask( Dali::Actor source, const Dali::Vector4& clearColor )
{
  DALI_ASSERT_ALWAYS(source && "Source is empty.");

  mSource = source;

  // Check the original parent about source.
  mParent = mSource.GetParent();

  Dali::Stage stage = Dali::Stage::GetCurrent();
  Dali::Size stageSize = stage.GetSize();

  // Add to stage for rendering the source. If source isn't on the stage then it never be rendered.
  stage.Add( mSource );

  if( !mCameraActor )
  {
    mCameraActor = Dali::CameraActor::New( stageSize );
    mCameraActor.SetParentOrigin( ParentOrigin::CENTER );
    mCameraActor.SetAnchorPoint( AnchorPoint::CENTER );
  }

  stage.Add( mCameraActor );

  DALI_ASSERT_ALWAYS(mFrameBuffer && "Framebuffer is NULL.");

  DALI_ASSERT_ALWAYS(!mRenderTask && "RenderTask is already created.");

  Dali::RenderTaskList taskList = stage.GetRenderTaskList();
  mRenderTask = taskList.CreateTask();
  mRenderTask.SetRefreshRate( Dali::RenderTask::REFRESH_ONCE );
  mRenderTask.SetSourceActor( source );
  mRenderTask.SetCameraActor( mCameraActor );
  mRenderTask.SetScreenToFrameBufferFunction( Dali::RenderTask::FULLSCREEN_FRAMEBUFFER_FUNCTION );
  mRenderTask.SetFrameBuffer( mFrameBuffer );
  mRenderTask.SetClearColor( clearColor );
  mRenderTask.SetClearEnabled( true );
  mRenderTask.SetProperty( Dali::RenderTask::Property::REQUIRES_SYNC, true );
  mRenderTask.FinishedSignal().Connect( this, &Capture::OnRenderFinished );
  mRenderTask.GetCameraActor().SetInvertYAxis( true );

  mTimer = Dali::Timer::New( TIME_OUT_DURATION );
  mTimer.TickSignal().Connect( this, &Capture::OnTimeOut );
  mTimer.Start();
}

void Capture::UnsetRenderTask()
{
  DALI_ASSERT_ALWAYS(mCameraActor && "CameraActor is NULL.");

  if( mParent )
  {
    // Restore the parent of source.
    mParent.Add( mSource );
    mParent.Reset();
  }
  else
  {
    mSource.Unparent();
  }

  mSource.Reset();

  mTimer.Reset();

  mCameraActor.Unparent();
  mCameraActor.Reset();

  DALI_ASSERT_ALWAYS(mRenderTask && "RenderTask is NULL.");

  Dali::RenderTaskList taskList = Dali::Stage::GetCurrent().GetRenderTaskList();
  Dali::RenderTask firstTask = taskList.GetTask( 0u );

  // Stop rendering via frame-buffers as empty handle is used to clear target
  firstTask.SetFrameBuffer( Dali::FrameBuffer() );

  taskList.RemoveTask( mRenderTask );
  mRenderTask.Reset();
}

bool Capture::IsRenderTaskSetup()
{
  return mCameraActor && mRenderTask;
}

void Capture::SetupResources( const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source )
{
  CreateSurface( size );
  ClearSurface( size );

  CreateNativeImageSource();

  CreateFrameBuffer();

  SetupRenderTask( source, clearColor );
}

void Capture::UnsetResources()
{
  if( IsRenderTaskSetup() )
  {
    UnsetRenderTask();
  }

  if( IsFrameBufferCreated() )
  {
    DeleteFrameBuffer();
  }

  if( IsNativeImageSourceCreated() )
  {
    DeleteNativeImageSource();
  }

  if( IsSurfaceCreated() )
  {
    DeleteSurface();
  }
}

void Capture::OnRenderFinished( Dali::RenderTask& task )
{
  Dali::Capture::FinishState state = Dali::Capture::FinishState::SUCCEEDED;

  mTimer.Stop();

  if( !Save() )
  {
    state = Dali::Capture::FinishState::FAILED;
    DALI_LOG_ERROR("Fail to Capture mTbmSurface[%p] Path[%s]", mTbmSurface, mPath.c_str());
  }

  Dali::Capture handle( this );
  mFinishedSignal.Emit( handle, state );

  UnsetResources();

  // Decrease the reference count forcely. It is increased at Start().
  Unreference();
}

bool Capture::OnTimeOut()
{
  Dali::Capture::FinishState state = Dali::Capture::FinishState::FAILED;

  Dali::Capture handle( this );
  mFinishedSignal.Emit( handle, state );

  UnsetResources();

  // Decrease the reference count forcely. It is increased at Start().
  Unreference();

  return false;
}

bool Capture::Save()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "mNativeImageSourcePtr is NULL");

  return mNativeImageSourcePtr->EncodeToFile( mPath );
}

}  // End of namespace Adaptor

}  // End of namespace Internal

}  // End of namespace Dali
