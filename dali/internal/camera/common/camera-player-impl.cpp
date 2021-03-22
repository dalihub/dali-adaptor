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
#include <dali/internal/camera/common/camera-player-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/type-registry.h>
#include <dlfcn.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
const char* CAMERA_PLUGIN_SO("libdali2-camera-player-plugin.so");

Dali::BaseHandle Create()
{
  return Dali::CameraPlayer::New();
}

Dali::TypeRegistration type(typeid(Dali::CameraPlayer), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

CameraPlayerPtr CameraPlayer::New()
{
  CameraPlayerPtr player = new CameraPlayer();
  player->Initialize();
  return player;
}

CameraPlayer::CameraPlayer()
: mPlugin(NULL),
  mHandle(NULL),
  mCreateCameraPlayerPtr(NULL),
  mDestroyCameraPlayerPtr(NULL)
{
}

CameraPlayer::~CameraPlayer()
{
  if(mHandle != NULL)
  {
    if(mDestroyCameraPlayerPtr != NULL)
    {
      mDestroyCameraPlayerPtr(mPlugin);
    }

    dlclose(mHandle);
  }
}

void CameraPlayer::Initialize()
{
  char* error = NULL;

  mHandle = dlopen(CAMERA_PLUGIN_SO, RTLD_LAZY);

  error = dlerror();
  if(mHandle == NULL || error != NULL)
  {
    DALI_LOG_ERROR("CameraPlayer::Initialize(), dlopen error: %s\n", error);
    return;
  }

  mCreateCameraPlayerPtr = reinterpret_cast<CreateCameraPlayerFunction>(dlsym(mHandle, "CreateCameraPlayerPlugin"));

  error = dlerror();
  if(mCreateCameraPlayerPtr == NULL || error != NULL)
  {
    DALI_LOG_ERROR("Can't load symbol CreateCameraPlayerPlugin(), error: %s\n", error);
    return;
  }

  mPlugin = mCreateCameraPlayerPtr();

  if(mPlugin == NULL)
  {
    DALI_LOG_ERROR("Can't create the CameraPlayerPlugin object\n");
    return;
  }

  mDestroyCameraPlayerPtr = reinterpret_cast<DestroyCameraPlayerFunction>(dlsym(mHandle, "DestroyCameraPlayerPlugin"));

  error = dlerror();
  if(mDestroyCameraPlayerPtr == NULL || error != NULL)
  {
    DALI_LOG_ERROR("Can't load symbol DestroyCameraPlayerPlugin(), error: %s\n", error);
    return;
  }
}

void CameraPlayer::SetWindowRenderingTarget(Dali::Window target)
{
  if(mPlugin != NULL)
  {
    mPlugin->SetWindowRenderingTarget(target);
  }
}

void CameraPlayer::SetNativeImageRenderingTarget(Dali::NativeImageSourcePtr target)
{
  if(mPlugin != NULL)
  {
    mPlugin->SetNativeImageRenderingTarget(target);
  }
}

void CameraPlayer::SetDisplayArea(DisplayArea area)
{
  if(mPlugin != NULL)
  {
    mPlugin->SetDisplayArea(area);
  }
}

void CameraPlayer::SetCameraPlayer(Any handle)
{
  if(mPlugin != NULL)
  {
    mPlugin->SetCameraPlayer(handle);
  }
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
