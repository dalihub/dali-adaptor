#ifndef DALI_INTERNAL_WINDOWSYSTEM_DISPLAY_CONNECTION_IMPL_ANDROID_H
#define DALI_INTERNAL_WINDOWSYSTEM_DISPLAY_CONNECTION_IMPL_ANDROID_H

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

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/display-connection-impl.h>

namespace Dali
{
class DisplayConnection;

namespace Internal::Adaptor
{
/**
 * DisplayConnection implementation
 */
class DisplayConnectionAndroid : public Dali::Internal::Adaptor::DisplayConnection
{
public:
  /**
   * @brief Default constructor
   */
  DisplayConnectionAndroid();

  /**
   * @brief Create an initialized DisplayConnection.
   *
   * @return A handle to a newly allocated DisplayConnection resource.
   */
  static DisplayConnection* New();

public:
  /**
   * @copydoc Dali::DisplayConnection::GetDisplay
   */
  Any GetDisplay() override;

  /**
   * @copydoc Dali::DisplayConnection::GetNativeGraphicsDisplay
   */
  Any GetNativeGraphicsDisplay() override;

  /**
   * @copydoc Dali::DisplayConnection::ConsumeEvents
   */
  void ConsumeEvents() override;

  /**
   * @brief Sets the surface type
   * @param[in] type The surface type
   */
  void SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type) override;

public:
  /**
   * Destructor
   */
  ~DisplayConnectionAndroid() override;

  // Undefined
  DisplayConnectionAndroid(const DisplayConnectionAndroid&) = delete;

  // Undefined
  DisplayConnectionAndroid& operator=(const DisplayConnectionAndroid& rhs) = delete;

private:
  EGLNativeDisplayType mDisplay; ///< EGL display for rendering
};

} // namespace Internal::Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_DISPLAY_CONNECTION_IMPL_ANDROID_H
