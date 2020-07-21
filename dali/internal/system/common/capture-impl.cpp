/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/capture-impl.h>

// EXTERNAL INCLUDES
#include <fstream>
#include <string.h>
#include <dali/devel-api/common/stage.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/devel-api/adaptor-framework/native-image-source-devel.h>

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
: mQuality( DEFAULT_QUALITY ),
  mTimer(),
  mPath(),
  mNativeImageSourcePtr( NULL ),
  mFileSave( false )
{
}

Capture::Capture( Dali::CameraActor cameraActor )
: mQuality( DEFAULT_QUALITY ),
  mCameraActor( cameraActor ),
  mTimer(),
  mPath(),
  mNativeImageSourcePtr( NULL ),
  mFileSave( false )
{
}

Capture::~Capture()
{
  DeleteNativeImageSource();
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

void Capture::Start( Dali::Actor source, const Dali::Vector2& size, const std::string &path, const Dali::Vector4& clearColor, const uint32_t quality )
{
  mQuality = quality;
  Start( source, size, path, clearColor );
}

void Capture::Start( Dali::Actor source, const Dali::Vector2& size, const std::string &path, const Dali::Vector4& clearColor )
{
  DALI_ASSERT_ALWAYS(path.size() > 4 && "Path is invalid.");

  // Increase the reference count focely to avoid application mistake.
  Reference();

  mPath = path;
  if( mPath.size() > 0 )
  {
    mFileSave = true;
  }

  DALI_ASSERT_ALWAYS(source && "Source is NULL.");

  UnsetResources();
  SetupResources( size, clearColor, source );
}

Dali::NativeImageSourcePtr Capture::GetNativeImageSource() const
{
  return mNativeImageSourcePtr;
}

Dali::Capture::CaptureFinishedSignalType& Capture::FinishedSignal()
{
  return mFinishedSignal;
}

void Capture::CreateNativeImageSource( const Vector2& size )
{
  Dali::Adaptor& adaptor = Dali::Adaptor::Get();

  DALI_ASSERT_ALWAYS(adaptor.IsAvailable() && "Dali::Adaptor is not available.");

  DALI_ASSERT_ALWAYS(!mNativeImageSourcePtr && "NativeImageSource is already created.");

  // create the NativeImageSource object with our surface
  mNativeImageSourcePtr = Dali::NativeImageSource::New( size.width, size.height, Dali::NativeImageSource::COLOR_DEPTH_DEFAULT );
}

void Capture::DeleteNativeImageSource()
{
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

  // Create a FrameBuffer object with depth attachments.
  mFrameBuffer = Dali::FrameBuffer::New( mNativeTexture.GetWidth(), mNativeTexture.GetHeight(), Dali::FrameBuffer::Attachment::DEPTH );
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
    mCameraActor.SetProperty( Dali::Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER );
    mCameraActor.SetProperty( Dali::Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER );
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

  DALI_ASSERT_ALWAYS( mRenderTask && "RenderTask is NULL." );

  Dali::RenderTaskList taskList = Dali::Stage::GetCurrent().GetRenderTaskList();
  taskList.RemoveTask( mRenderTask );
  mRenderTask.Reset();
}

bool Capture::IsRenderTaskSetup()
{
  return mCameraActor && mRenderTask;
}

void Capture::SetupResources( const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source )
{
  CreateNativeImageSource( size );

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
}

void Capture::OnRenderFinished( Dali::RenderTask& task )
{
  Dali::Capture::FinishState state = Dali::Capture::FinishState::SUCCEEDED;

  mTimer.Stop();

  if( mFileSave )
  {
    if( !SaveFile() )
    {
      state = Dali::Capture::FinishState::FAILED;
      DALI_LOG_ERROR( "Fail to Capture Path[%s]", mPath.c_str() );
    }
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

bool Capture::SaveFile()
{
  DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "mNativeImageSourcePtr is NULL");

  return Dali::DevelNativeImageSource::EncodeToFile( *mNativeImageSourcePtr, mPath, mQuality );
}

}  // End of namespace Adaptor

}  // End of namespace Internal

}  // End of namespace Dali
