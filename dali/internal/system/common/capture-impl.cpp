/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/debug.h>
#include <dali/integration-api/pixel-data-integ.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <string.h>
#include <fstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/devel-api/adaptor-framework/native-image-source-devel.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static constexpr uint32_t ORDER_INDEX_CAPTURE_RENDER_TASK = 1000;
constexpr uint32_t        TIME_OUT_DURATION               = 1000;

DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_IMAGE_PERFORMANCE_MARKER, false);
} // namespace

Capture::Capture()
: mQuality(DEFAULT_QUALITY),
  mTimer(),
  mPath(),
  mFileSave(false),
  mUseDefaultCamera(true),
  mSceneOffCameraAfterCaptureFinished(false)
{
}

Capture::Capture(Dali::CameraActor cameraActor)
: mQuality(DEFAULT_QUALITY),
  mCameraActor(cameraActor),
  mTimer(),
  mPath(),
  mFileSave(false),
  mUseDefaultCamera(!cameraActor),
  mSceneOffCameraAfterCaptureFinished(false)
{
}

Capture::~Capture()
{
  if(Adaptor::IsAvailable())
  {
    Adaptor::Get().UnregisterProcessorOnce(*this, true);
  }
  mTexture.Reset();
}

CapturePtr Capture::New()
{
  DALI_ASSERT_ALWAYS(Graphics::GetCurrentGraphicsBackend() == Graphics::Backend::GLES && "Only GLES Backend Supported");

  CapturePtr pWorker = new Capture();

  return pWorker;
}

CapturePtr Capture::New(Dali::CameraActor cameraActor)
{
  DALI_ASSERT_ALWAYS(Graphics::GetCurrentGraphicsBackend() == Graphics::Backend::GLES && "Only GLES Backend Supported");

  CapturePtr pWorker = new Capture(cameraActor);

  return pWorker;
}

void Capture::Start(Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string& path, const Dali::Vector4& clearColor, const uint32_t quality)
{
  if(mInCapture)
  {
    DALI_LOG_ERROR("Capture is already requested.\n");
    return;
  }
  mQuality = quality;
  Start(source, position, size, path, clearColor);
}

void Capture::Start(Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string& path, const Dali::Vector4& clearColor)
{
  if(mInCapture)
  {
    DALI_LOG_ERROR("Capture is already requested.\n");
    return;
  }

  if(!source)
  {
    return;
  }

  // Increase the reference count focely to avoid application mistake.
  Reference();

  UnsetResources();
  SetupResources(position, size, clearColor, source);

  mPath = path;
  if(!mPath.empty())
  {
    mFileSave = true;
  }
  else
  {
    mFileSave = false;
  }
  mRenderTask.KeepRenderResult();

  mInCapture = true;
  Adaptor::Get().RegisterProcessorOnce(*this, true);
}

void Capture::SetImageQuality(uint32_t quality)
{
  mQuality = quality;
}

void Capture::SetExclusive(bool exclusive)
{
  if(mIsExclusive != exclusive)
  {
    mIsExclusive = exclusive;
    if(mRenderTask)
    {
      mRenderTask.SetExclusive(mIsExclusive);
    }
  }
}

bool Capture::IsExclusive() const
{
  return mIsExclusive;
}

Dali::NativeImageSourcePtr Capture::GetNativeImageSource()
{
  Dali::NativeImageSourcePtr result;
  if(mRenderTask)
  {
    Dali::PixelData pixelData = mRenderTask.GetRenderResult();
    if(pixelData)
    {
      auto                 pixelDataBuffer      = Dali::Integration::GetPixelDataBuffer(pixelData);
      NativeImageSourcePtr nativeImageSourcePtr = Dali::NativeImageSource::New(pixelData.GetWidth(), pixelData.GetHeight(), Dali::NativeImageSource::COLOR_DEPTH_32); // Texture pixel format is RGBA8888

      if(Dali::DevelNativeImageSource::SetPixels(*nativeImageSourcePtr, pixelDataBuffer.buffer, pixelData.GetPixelFormat()))
      {
        result = nativeImageSourcePtr;
      }
    }
  }
  return result;
}

Dali::Texture Capture::GetTexture() const
{
  return mTexture;
}

Dali::Devel::PixelBuffer Capture::GetCapturedBuffer()
{
  Devel::PixelBuffer pixelBuffer;
  if(mRenderTask)
  {
    Dali::PixelData pixelData = mRenderTask.GetRenderResult();
    if(pixelData)
    {
      auto pixelDataBuffer = Dali::Integration::GetPixelDataBuffer(pixelData);
      pixelBuffer          = Dali::Devel::PixelBuffer::New(pixelData.GetWidth(), pixelData.GetHeight(), pixelData.GetPixelFormat());
      memcpy(pixelBuffer.GetBuffer(), pixelDataBuffer.buffer, pixelDataBuffer.bufferSize);
    }
  }
  return pixelBuffer;
}

Dali::Capture::CaptureFinishedSignalType& Capture::FinishedSignal()
{
  return mFinishedSignal;
}

void Capture::CreateTexture(const Vector2& size)
{
  mTexture = Dali::Texture::New(TextureType::TEXTURE_2D, Pixel::RGBA8888, unsigned(size.width), unsigned(size.height));
}

void Capture::CreateFrameBuffer()
{
  if(!mFrameBuffer)
  {
    // Create a FrameBuffer object with depth attachments.
    mFrameBuffer = Dali::FrameBuffer::New(mTexture.GetWidth(), mTexture.GetHeight(), Dali::FrameBuffer::Attachment::DEPTH);
    // Add a color attachment to the FrameBuffer object.
    mFrameBuffer.AttachColorTexture(mTexture);
  }
}

void Capture::DeleteFrameBuffer()
{
  if(mFrameBuffer)
  {
    mFrameBuffer.Reset();
  }
}

bool Capture::IsFrameBufferCreated()
{
  return static_cast<bool>(mFrameBuffer);
}

void Capture::SetupRenderTask(const Dali::Vector2& position, const Dali::Vector2& size, Dali::Actor source, const Dali::Vector4& clearColor)
{
  if(!source)
  {
    DALI_LOG_ERROR("Source is empty\n");
    return;
  }

  Dali::Integration::SceneHolder sceneHolder = Dali::Integration::SceneHolder::Get(source);
  if(!sceneHolder)
  {
    DALI_LOG_ERROR("The source is not added on the scene\n");
    return;
  }

  mSource = source;

  if(!mCameraActor)
  {
    mUseDefaultCamera = true;
    mCameraActor      = Dali::CameraActor::New(size);
    // Because input position and size are for 2 dimentional area,
    // default z-directional position of the camera is required to be used for the new camera position.
    float   cameraDefaultZPosition = mCameraActor.GetProperty<float>(Dali::Actor::Property::POSITION_Z);
    Vector2 positionTransition     = position + size / 2;
    mCameraActor.SetProperty(Dali::Actor::Property::POSITION, Vector3(positionTransition.x, positionTransition.y, cameraDefaultZPosition));
    mCameraActor.SetProperty(Dali::Actor::Property::PARENT_ORIGIN, ParentOrigin::TOP_LEFT);
    mCameraActor.SetProperty(Dali::Actor::Property::ANCHOR_POINT, AnchorPoint::CENTER);
  }

  // Camera must be scene on. Add camera to window.
  if(!mCameraActor.GetProperty<bool>(Dali::Actor::Property::CONNECTED_TO_SCENE))
  {
    if(!mUseDefaultCamera)
    {
      DALI_LOG_ERROR("Camera must be on scene. Camera is connected to window now.\n");
    }
    sceneHolder.Add(mCameraActor);
    mSceneOffCameraAfterCaptureFinished = true;
  }

  if(!mFrameBuffer)
  {
    DALI_LOG_ERROR("Frame buffer is not created.\n");
    return;
  }

  mSceneHolderHandle            = sceneHolder;
  Dali::RenderTaskList taskList = sceneHolder.GetRenderTaskList();
  mRenderTask                   = taskList.CreateTask();
  mRenderTask.SetOrderIndex(ORDER_INDEX_CAPTURE_RENDER_TASK);
  mRenderTask.SetRefreshRate(Dali::RenderTask::REFRESH_ONCE);
  mRenderTask.SetSourceActor(source);
  mRenderTask.SetCameraActor(mCameraActor);
  mRenderTask.SetScreenToFrameBufferFunction(Dali::RenderTask::FULLSCREEN_FRAMEBUFFER_FUNCTION);
  mRenderTask.SetFrameBuffer(mFrameBuffer);
  mRenderTask.SetClearColor(clearColor);
  mRenderTask.SetClearEnabled(true);
  mRenderTask.SetExclusive(mIsExclusive);
  mRenderTask.SetProperty(Dali::RenderTask::Property::REQUIRES_SYNC, true);
  mRenderTask.FinishedSignal().Connect(this, &Capture::OnRenderFinished);
  mRenderTask.GetCameraActor().SetInvertYAxis(true);
}

void Capture::UnsetRenderTask()
{
  mTimer.Reset();

  if(mSceneOffCameraAfterCaptureFinished && mCameraActor)
  {
    if(!mUseDefaultCamera)
    {
      DALI_LOG_ERROR("Camera is disconnected from window now.\n");
    }
    mSceneOffCameraAfterCaptureFinished = false;
    mCameraActor.Unparent();
    mCameraActor.Reset();
  }

  Dali::Integration::SceneHolder sceneHolder = mSceneHolderHandle.GetHandle();
  if(mRenderTask && sceneHolder)
  {
    Dali::RenderTaskList taskList = sceneHolder.GetRenderTaskList();
    taskList.RemoveTask(mRenderTask);
  }
  mRenderTask.ClearRenderResult();
  mRenderTask.Reset();
  mSource.Reset();
  mTexture.Reset();
  mSceneHolderHandle.Reset();
}

bool Capture::IsRenderTaskSetup()
{
  return mCameraActor && mRenderTask;
}

void Capture::SetupResources(const Dali::Vector2& position, const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source)
{
  CreateTexture(size);

  CreateFrameBuffer();

  SetupRenderTask(position, size, source, clearColor);
}

void Capture::UnsetResources()
{
  if(IsRenderTaskSetup())
  {
    UnsetRenderTask();
  }

  if(IsFrameBufferCreated())
  {
    DeleteFrameBuffer();
  }
}

void Capture::OnRenderFinished(Dali::RenderTask& task)
{
  if(!mRenderTask || mRenderTask != task)
  {
    DALI_LOG_DEBUG_INFO("Old RenderFinished callback comes. Ignore\n");
    return;
  }

  // Stop timer before file save (legacy behavior)
  if(DALI_LIKELY(mTimer))
  {
    mTimer.Stop();
  }

  if(mFileSave)
  {
    if(Adaptor::IsAvailable())
    {
      if(mCaptureFileSaveTask)
      {
        DALI_LOG_DEBUG_INFO("Cancel file save Path[%s]\n", mPath.c_str());
        Dali::AsyncTaskManager::Get().RemoveTask(mCaptureFileSaveTask);
        mCaptureFileSaveTask.Reset();
      }

      DALI_LOG_DEBUG_INFO("Request to save Capture Path[%s]\n", mPath.c_str());
      Dali::PixelData pixelData = mRenderTask.GetRenderResult();
      mCaptureFileSaveTask      = new Capture::CaptureFileSaveTask(mRenderTask, pixelData, mPath, mQuality, MakeCallback(this, &Capture::OnFileSaveCompleted));
      Dali::AsyncTaskManager::Get().AddTask(mCaptureFileSaveTask);
    }
    else
    {
      DALI_LOG_ERROR("Fail to Capture Path[%s] (Adaptor is invalidated)\n", mPath.c_str());
      EmitCaptureFinished(false);
    }
    return;
  }

  EmitCaptureFinished(true);
}

bool Capture::OnTimeOut()
{
  EmitCaptureFinished(false);

  return false;
}

void Capture::OnFileSaveCompleted(CaptureFileSaveTaskPtr task)
{
  if(DALI_LIKELY(mCaptureFileSaveTask && static_cast<AsyncTask*>(mCaptureFileSaveTask.Get()) == task.Get()))
  {
    if(!mRenderTask || mRenderTask != mCaptureFileSaveTask->GetRenderTask())
    {
      DALI_LOG_DEBUG_INFO("Old OnFileSaveCompleted callback comes. Ignore\n");
      return;
    }

    const bool fileSaved = mCaptureFileSaveTask->IsFileSaved();
    mCaptureFileSaveTask.Reset();
    EmitCaptureFinished(fileSaved);
  }
}

void Capture::EmitCaptureFinished(bool successed)
{
  Dali::Capture::FinishState state = successed ? Dali::Capture::FinishState::SUCCEEDED : Dali::Capture::FinishState::FAILED;

  mInCapture = false;

  Dali::Capture handle(this);
  mFinishedSignal.Emit(handle, state);

  // Don't unset resources when capture re-start during finished signal.
  if(!mInCapture)
  {
    UnsetResources();
  }

  // Decrease the reference count forcely. It is increased at Start().
  Unreference();
}

void Capture::Process(bool postProcessor)
{
  if(mInCapture)
  {
    mTimer = Dali::Timer::New(TIME_OUT_DURATION);
    mTimer.TickSignal().Connect(this, &Capture::OnTimeOut);
    mTimer.Start();
  }
}

// CaptureFileSaveTask

Capture::CaptureFileSaveTask::CaptureFileSaveTask(Dali::RenderTask renderTask, Dali::PixelData pixelData, std::string path, uint32_t quality, CallbackBase* callback)
: AsyncTask(callback),
  mRenderTask(std::move(renderTask)),
  mPixelData(std::move(pixelData)),
  mPath(std::move(path)),
  mQuality(quality),
  mFileSaved(false)
{
}

Capture::CaptureFileSaveTask::~CaptureFileSaveTask() = default;

void Capture::CaptureFileSaveTask::Process()
{
  if(mPixelData)
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_CAPTURE_FILE_SAVE");

    auto               pixelDataBuffer = Dali::Integration::GetPixelDataBuffer(mPixelData);
    Devel::PixelBuffer pixelBuffer;

    uint8_t* buffer = nullptr;

    auto pixelFormat = mPixelData.GetPixelFormat();

    if(!Dali::Pixel::IsCompressed(pixelFormat) && Dali::Pixel::HasAlpha(pixelFormat))
    {
      switch(pixelFormat)
      {
        case Pixel::RGBA8888:
        case Pixel::BGRA8888:
        {
          pixelBuffer = Dali::Devel::PixelBuffer::New(pixelDataBuffer.width, pixelDataBuffer.height, pixelFormat);

          buffer = pixelBuffer.GetBuffer();
          memcpy(buffer, pixelDataBuffer.buffer, pixelDataBuffer.bufferSize);

          const uint32_t bpp         = Dali::Pixel::GetBytesPerPixel(pixelFormat);
          const uint32_t strideBytes = pixelBuffer.GetStrideBytes();

          for(uint32_t y = 0u; y < mPixelData.GetHeight(); ++y)
          {
            for(uint32_t x = 0u; x < mPixelData.GetWidth(); ++x)
            {
              uint8_t& c0 = buffer[y * strideBytes + x * bpp + 0u];
              uint8_t& c1 = buffer[y * strideBytes + x * bpp + 1u];
              uint8_t& c2 = buffer[y * strideBytes + x * bpp + 2u];

              uint8_t& alpha = buffer[y * strideBytes + x * bpp + 3u];

              if(alpha == 0u)
              {
                c0 = c1 = c2 = 0u;
              }
              else if(alpha == 255u)
              {
                // do nothing
              }
              else
              {
                c0 = static_cast<uint8_t>((static_cast<uint16_t>(std::min(alpha, c0)) * 255u + alpha / 2) / alpha);
                c1 = static_cast<uint8_t>((static_cast<uint16_t>(std::min(alpha, c1)) * 255u + alpha / 2) / alpha);
                c2 = static_cast<uint8_t>((static_cast<uint16_t>(std::min(alpha, c2)) * 255u + alpha / 2) / alpha);
              }
            }
          }
          break;
        }
        default:
        {
          // TODO : Support more cases
          break;
        }
      }
    }
    else
    {
      buffer = pixelDataBuffer.buffer;
    }

    if(DALI_LIKELY(buffer) && Dali::EncodeToFile(buffer, mPath, mPixelData.GetPixelFormat(), mPixelData.GetWidth(), mPixelData.GetHeight(), mQuality))
    {
      mFileSaved = true;
    }
    else
    {
      DALI_LOG_ERROR("(Capture) Fail to save file to path[%s]\n", mPath.c_str());
    }
  }
  else
  {
    DALI_LOG_ERROR("(Capture) Fail to save file - PixelData is nullptr. path[%s]\n", mPath.c_str());
  }
}

} // End of namespace Adaptor

} // End of namespace Internal

} // End of namespace Dali
