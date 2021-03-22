#ifndef DALI_CAMERA_PLAYER_IMPL_H
#define DALI_CAMERA_PLAYER_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/camera-player-plugin.h>
#include <dali/devel-api/adaptor-framework/camera-player.h>

namespace Dali
{
class Any;

namespace Internal
{
namespace Adaptor
{
class CameraPlayer;

typedef IntrusivePtr<CameraPlayer> CameraPlayerPtr;

/**
 * @brief CameraPlayer class is used for camera preview.
 */
class CameraPlayer : public Dali::BaseObject
{
public:
  /**
   * @brief Creates a new CameraPlayer handle
   * @return CameraPlayer pointer
   */
  static CameraPlayerPtr New();

  /**
   * @copydoc Dali::CameraPlayer::SetWindowRenderingTarget()
   */
  void SetWindowRenderingTarget(Dali::Window target);

  /**
   * @copydoc Dali::CameraPlayer::SetNativeImageRenderingTarget()
   */
  void SetNativeImageRenderingTarget(Dali::NativeImageSourcePtr target);

  /**
   * @copydoc Dali::CameraPlayer::SetDisplayArea()
   */
  void SetDisplayArea(DisplayArea area);

  /**
   * @brief Dali::CameraPlayer::SetCameraPlayer()
   */
  void SetCameraPlayer(Any handle);

private:
  /**
   * @brief Constructor.
   */
  CameraPlayer();

  /**
   * @brief Destructor.
   */
  virtual ~CameraPlayer();

  // Undefined copy constructor
  CameraPlayer(const CameraPlayer& player);

  // Undefined assignment operator
  CameraPlayer& operator=(const CameraPlayer& player);

  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:
  Dali::CameraPlayerPlugin* mPlugin; ///< camera player plugin handle
  void*                     mHandle; ///< Handle for the loaded library

  using CreateCameraPlayerFunction  = Dali::CameraPlayerPlugin* (*)();
  using DestroyCameraPlayerFunction = void (*)(Dali::CameraPlayerPlugin* plugin);

  CreateCameraPlayerFunction  mCreateCameraPlayerPtr;
  DestroyCameraPlayerFunction mDestroyCameraPlayerPtr;
};

} // namespace Adaptor
} // namespace Internal

inline static Internal::Adaptor::CameraPlayer& GetImplementation(Dali::CameraPlayer& player)
{
  DALI_ASSERT_ALWAYS(player && "CameraPlayer handle is empty.");

  BaseObject& handle = player.GetBaseObject();

  return static_cast<Internal::Adaptor::CameraPlayer&>(handle);
}

inline static const Internal::Adaptor::CameraPlayer& GetImplementation(const Dali::CameraPlayer& player)
{
  DALI_ASSERT_ALWAYS(player && "CameraPlayer handle is empty.");

  const BaseObject& handle = player.GetBaseObject();

  return static_cast<const Internal::Adaptor::CameraPlayer&>(handle);
}

} // namespace Dali

#endif // DALI_CAMERA_PLAYER_IMPL_H
