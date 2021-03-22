#ifndef DALI_CAMERA_PLAYER_H
#define DALI_CAMERA_PLAYER_H

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
#include <dali/public-api/object/base-handle.h>

//INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/camera-player-plugin.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class Any;

namespace Internal
{
namespace Adaptor
{
class CameraPlayer;
} // namespace Adaptor

} // namespace Internal

/**
 * @brief CameraPlayer class is used for camera playback.
 */
class DALI_ADAPTOR_API CameraPlayer : public BaseHandle
{
public:
  /**
   * @brief Constructor.
   */
  CameraPlayer();

  /**
   * @brief Destructor.
   */
  ~CameraPlayer();

  /**
   * @brief Creates a new instance of a CameraPlayer.
   */
  static CameraPlayer New();

  /**
   * @brief Copy constructor.
   *
   * @param[in] player CameraPlayer to copy. The copied player will point at the same implementation
   */
  CameraPlayer(const CameraPlayer& player);

  /**
   * @brief Assignment operator.
   *
   * @param[in] player The CameraPlayer to assign from.
   * @return The updated CameraPlayer.
   */
  CameraPlayer& operator=(const CameraPlayer& player);

  /**
   * @brief Downcast a handle to CameraPlayer handle.
   *
   * If handle points to a CameraPlayer the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a CameraPlayer or an uninitialized handle
   */
  static CameraPlayer DownCast(BaseHandle handle);

  /**
   * @brief Sets camera rendering target.
   * @param[in] target The target for camera rendering, window surface
   */
  void SetWindowRenderingTarget(Dali::Window target);

  /**
   * @brief Sets camera rendering target.
   * @param[in] target The target for camera rendering, native image source
   */
  void SetNativeImageRenderingTarget(Dali::NativeImageSourcePtr target);

  /**
   * @brief Sets the area of camera display.
   * @param[in] area The left-top position and size of the camera display area
   */
  void SetDisplayArea(DisplayArea area);

  /**
   * @brief Sets the camera player handle
   * @param[in] handle The handle for camera player
   */
  void SetCameraPlayer(Any handle);

private: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL CameraPlayer(Internal::Adaptor::CameraPlayer* internal);
};

} // namespace Dali

#endif // DALI_CAMERA_PLAYER_H
