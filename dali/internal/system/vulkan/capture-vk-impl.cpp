/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr int32_t  SHADER_VERSION_NATIVE_IMAGE_SOURCE_AVAILABLE = 300;
constexpr uint32_t TIME_OUT_DURATION                            = 1000;
} // namespace

Capture::Capture()
: mQuality(DEFAULT_QUALITY),
  mTimer(),
  mPath(),
  mNativeImageSourcePtr(nullptr),
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
  mNativeImageSourcePtr(nullptr),
  mFileSave(false),
  mUseDefaultCamera(!cameraActor),
  mSceneOffCameraAfterCaptureFinished(false)
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

CapturePtr Capture::New(Dali::CameraActor cameraActor)
{
  CapturePtr pWorker = new Capture(cameraActor);

  return pWorker;
}

void Capture::Start(Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string& path, const Dali::Vector4& clearColor, const uint32_t quality)
{
}

void Capture::Start(Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string& path, const Dali::Vector4& clearColor)
{
}

void Capture::SetImageQuality(uint32_t quality)
{
}

Dali::NativeImageSourcePtr Capture::GetNativeImageSource() const
{
  return nullptr;
}

Dali::Texture Capture::GetTexture() const
{
  return Dali::Texture();
}

Dali::Devel::PixelBuffer Capture::GetCapturedBuffer()
{
  return Dali::Devel::PixelBuffer();
}

Dali::Capture::CaptureFinishedSignalType& Capture::FinishedSignal()
{
  return mFinishedSignal;
}

void Capture::CreateTexture(const Vector2& size)
{
}

void Capture::DeleteNativeImageSource()
{
}

void Capture::CreateFrameBuffer()
{
}

void Capture::DeleteFrameBuffer()
{
}

bool Capture::IsFrameBufferCreated()
{
  return false;
}

void Capture::SetupRenderTask(const Dali::Vector2& position, const Dali::Vector2& size, Dali::Actor source, const Dali::Vector4& clearColor)
{
}

void Capture::UnsetRenderTask()
{
}

bool Capture::IsRenderTaskSetup()
{
  return false;
}

void Capture::SetupResources(const Dali::Vector2& position, const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source)
{
}

void Capture::UnsetResources()
{
}

void Capture::OnRenderFinished(Dali::RenderTask& task)
{
}

bool Capture::OnTimeOut()
{
  return false;
}

bool Capture::SaveFile()
{
  return false;
}

} // End of namespace Adaptor

} // End of namespace Internal

} // End of namespace Dali
