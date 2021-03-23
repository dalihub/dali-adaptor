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
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/devel-api/adaptor-framework/native-image-source-devel.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
constexpr int32_t VERSION_NATIVE_IMAGE_SOURCE = 30;
constexpr uint32_t TIME_OUT_DURATION = 1000;
}

Capture::Capture()
: mQuality( DEFAULT_QUALITY ),
  mTimer(),
  mPath(),
  mNativeImageSourcePtr( NULL ),
  mFileSave( false ),
  mIsNativeImageSourcePossible(true)
{
}

Capture::Capture( Dali::CameraActor cameraActor )
: mQuality( DEFAULT_QUALITY ),
  mCameraActor( cameraActor ),
  mTimer(),
  mPath(),
  mNativeImageSourcePtr( NULL ),
  mFileSave( false ),
  mIsNativeImageSourcePossible(true)
{
}

Capture::~Capture()
{
  DeleteNativeImageSource();
  mTexture.Reset();
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

void Capture::Start( Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string &path, const Dali::Vector4& clearColor, const uint32_t quality )
{
  mQuality = quality;
  Start( source, position, size, path, clearColor );
}

void Capture::Start( Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string &path, const Dali::Vector4& clearColor )
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
  SetupResources( position, size, clearColor, source );
}

void Capture::SetImageQuality( uint32_t quality )
{
  mQuality = quality;
}

Dali::NativeImageSourcePtr Capture::GetNativeImageSource() const
{
  return mNativeImageSourcePtr;
}

Dali::Texture Capture::GetTexture()
{
  return mTexture;
}

Dali::Capture::CaptureFinishedSignalType& Capture::FinishedSignal()
{
  return mFinishedSignal;
}

void Capture::CreateTexture(const Vector2& size)
{
  Dali::Adaptor& adaptor = Dali::Adaptor::Get();

  DALI_ASSERT_ALWAYS(adaptor.IsAvailable() && "Dali::Adaptor is not available.");

  if(mIsNativeImageSourcePossible)
  {
    DALI_ASSERT_ALWAYS(!mNativeImageSourcePtr && "NativeImageSource is already created.");
    // create the NativeImageSource object with our surface
    mNativeImageSourcePtr = Dali::NativeImageSource::New(size.width, size.height, Dali::NativeImageSource::COLOR_DEPTH_DEFAULT);
    mTexture              = Dali::Texture::New(*mNativeImageSourcePtr);
  }
  else
  {
    mTexture = Dali::Texture::New(TextureType::TEXTURE_2D, Pixel::RGB888, unsigned(size.width), unsigned(size.height));
  }
}

void Capture::DeleteNativeImageSource()
{
  if(mNativeImageSourcePtr)
  {
    mNativeImageSourcePtr.Reset();
  }
}

void Capture::CreateFrameBuffer()
{
  DALI_ASSERT_ALWAYS(!mFrameBuffer && "FrameBuffer is already created.");

  // Create a FrameBuffer object with depth attachments.
  mFrameBuffer = Dali::FrameBuffer::New(mTexture.GetWidth(), mTexture.GetHeight(), Dali::FrameBuffer::Attachment::DEPTH);
  // Add a color attachment to the FrameBuffer object.
  mFrameBuffer.AttachColorTexture(mTexture);
}

void Capture::DeleteFrameBuffer()
{
  DALI_ASSERT_ALWAYS(mFrameBuffer && "FrameBuffer is NULL.");

  mFrameBuffer.Reset();
}

bool Capture::IsFrameBufferCreated()
{
  return mFrameBuffer;
}

void Capture::SetupRenderTask( const Dali::Vector2& position, const Dali::Vector2& size, Dali::Actor source, const Dali::Vector4& clearColor )
{
  DALI_ASSERT_ALWAYS(source && "Source is empty.");

  Dali::Window window = DevelWindow::Get( source );
  if( !window )
  {
    DALI_LOG_ERROR("The source is not added on the window\n");
    return;
  }

  mSource = source;

  if( !mCameraActor )
  {
    mCameraActor = Dali::CameraActor::New( size );
    // Because input position and size are for 2 dimentional area,
    // default z-directional position of the camera is required to be used for the new camera position.
    float cameraDefaultZPosition = mCameraActor.GetProperty<float>( Dali::Actor::Property::POSITION_Z );
    Vector2 positionTransition = position + size / 2;
    mCameraActor.SetProperty( Dali::Actor::Property::POSITION, Vector3( positionTransition.x, positionTransition.y, cameraDefaultZPosition ) );
    mCameraActor.SetProperty( Dali::Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT );
    mCameraActor.SetProperty( Dali::Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER );
  }

  window.Add( mCameraActor );

  DALI_ASSERT_ALWAYS(mFrameBuffer && "Framebuffer is NULL.");

  DALI_ASSERT_ALWAYS(!mRenderTask && "RenderTask is already created.");

  Dali::RenderTaskList taskList = window.GetRenderTaskList();
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
  if(!mIsNativeImageSourcePossible)
  {
    mFrameBuffer.CaptureRenderedResult();
  }

  mTimer = Dali::Timer::New( TIME_OUT_DURATION );
  mTimer.TickSignal().Connect( this, &Capture::OnTimeOut );
  mTimer.Start();
}

void Capture::UnsetRenderTask()
{
  DALI_ASSERT_ALWAYS(mCameraActor && "CameraActor is NULL.");

  mTimer.Reset();

  mCameraActor.Unparent();
  mCameraActor.Reset();

  DALI_ASSERT_ALWAYS( mRenderTask && "RenderTask is NULL." );

  Dali::Window window = DevelWindow::Get( mSource );
  Dali::RenderTaskList taskList = window.GetRenderTaskList();
  taskList.RemoveTask( mRenderTask );
  mRenderTask.Reset();
  mSource.Reset();
}

bool Capture::IsRenderTaskSetup()
{
  return mCameraActor && mRenderTask;
}

void Capture::SetupResources(const Dali::Vector2& position, const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source)
{
  Dali::Internal::Adaptor::Adaptor& adaptor     = Internal::Adaptor::Adaptor::GetImplementation(Internal::Adaptor::Adaptor::Get());
  GraphicsInterface*                graphics    = &adaptor.GetGraphicsInterface();
  auto                              eglGraphics = static_cast<EglGraphics*>(graphics);

  if(eglGraphics->GetEglImplementation().GetGlesVersion() < VERSION_NATIVE_IMAGE_SOURCE)
  {
    mIsNativeImageSourcePossible = false;
  }

  CreateTexture(size);

  CreateFrameBuffer();

  SetupRenderTask( position, size, source, clearColor );
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
  if(mIsNativeImageSourcePossible)
  {
    DALI_ASSERT_ALWAYS(mNativeImageSourcePtr && "mNativeImageSourcePtr is NULL");
    return Dali::DevelNativeImageSource::EncodeToFile(*mNativeImageSourcePtr, mPath, mQuality);
  }
  else
  {
    uint8_t* buffer = mFrameBuffer.GetRenderedBuffer();
    return Dali::EncodeToFile(buffer, mPath, Dali::Pixel::RGBA8888, mTexture.GetWidth(), mTexture.GetHeight(), mQuality);
  }
}

}  // End of namespace Adaptor

}  // End of namespace Internal

}  // End of namespace Dali
