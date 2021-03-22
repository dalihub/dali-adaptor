/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/camera-player.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>

// INTERNAL INCLUDES
#include <dali/internal/camera/common/camera-player-impl.h>

namespace Dali
{
CameraPlayer::CameraPlayer()
{
}

CameraPlayer::CameraPlayer(Internal::Adaptor::CameraPlayer* internal)
: BaseHandle(internal)
{
}

CameraPlayer::~CameraPlayer()
{
}

CameraPlayer CameraPlayer::New()
{
  Internal::Adaptor::CameraPlayerPtr player = Internal::Adaptor::CameraPlayer::New();
  return CameraPlayer(player.Get());
}

CameraPlayer::CameraPlayer(const CameraPlayer& player)
: BaseHandle(player)
{
}

CameraPlayer& CameraPlayer::operator=(const CameraPlayer& player)
{
  if(*this != player)
  {
    BaseHandle::operator=(player);
  }
  return *this;
}

CameraPlayer CameraPlayer::DownCast(BaseHandle handle)
{
  return CameraPlayer(dynamic_cast<Internal::Adaptor::CameraPlayer*>(handle.GetObjectPtr()));
}

void CameraPlayer::SetWindowRenderingTarget(Dali::Window target)
{
  GetImplementation(*this).SetWindowRenderingTarget(target);
}

void CameraPlayer::SetNativeImageRenderingTarget(Dali::NativeImageSourcePtr target)
{
  GetImplementation(*this).SetNativeImageRenderingTarget(target);
}

void CameraPlayer::SetDisplayArea(DisplayArea area)
{
  GetImplementation(*this).SetDisplayArea(area);
}

void CameraPlayer::SetCameraPlayer(Any handle)
{
  GetImplementation(*this).SetCameraPlayer(handle);
}

} // namespace Dali
