#ifndef DALI_CAMERA_PLAYER_PLUGIN_H
#define DALI_CAMERA_PLAYER_PLUGIN_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/adaptor-framework/window.h>

// EXTERNAL INCLUDES
#include <dali/public-api/math/rect.h>

namespace Dali
{
class Any;
using DisplayArea = Dali::Rect<int>;

/**
 * @brief CameraPlayerPlugin is an abstract interface, used by dali-adaptor to access camera player plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 */
class CameraPlayerPlugin
{
public:
  /**
   * @brief Constructor.
   */
  CameraPlayerPlugin()
  {
  }

  /**
   * @brief Destructor.
   */
  virtual ~CameraPlayerPlugin()
  {
  }

  /**
   * @brief Sets camera rendering target for window
   * @param[in] target The target for camera rendering, window surface
   */
  virtual void SetWindowRenderingTarget(Dali::Window target) = 0;

  /**
   * @brief Sets camera rendering target for native image
   * @param[in] target The target for camera rendering, native image source
   */
  virtual void SetNativeImageRenderingTarget(Dali::NativeImageSourcePtr target) = 0;

  /**
   * @brief Sets the area of camera display.
   * @param[in] area The left-top position and size of the camera display area
   */
  virtual void SetDisplayArea(DisplayArea area) = 0;

  /**
   * @brief Set the camera player handle.
   * @param[in] handle The handle for camera player
   */
  virtual void SetCameraPlayer(Any handle) = 0;
};

} // namespace Dali

#endif // DALI_CAMERA_PLAYER_PLUGIN_H
