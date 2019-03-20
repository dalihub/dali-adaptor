#ifndef DALI_INTERNAL_WINDOWSYSTEM_ANDROID_DISPLAY_CONNECTION_IMPL_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_ANDROID_DISPLAY_CONNECTION_IMPL_ECORE_WL_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/display-connection-impl.h>

namespace Dali
{

class DisplayConnection;

namespace Internal
{

namespace Adaptor
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
  Any GetDisplay();

  /**
   * @copydoc Dali::DisplayConnection::ConsumeEvents
   */
  void ConsumeEvents();

  /**
   * @copydoc Dali::DisplayConnection::InitializeGraphics
   */
  bool InitializeGraphics();

  /**
   * @brief Sets the surface type
   * @param[in] type The surface type
   */
  void SetSurfaceType( Integration::RenderSurface::Type type );

  /**
   * @brief Sets the graphics interface
   * @param[in] graphics The graphics interface
   */
  void SetGraphicsInterface( GraphicsInterface& graphics );

public:

  /**
   * Destructor
   */
  virtual ~DisplayConnectionAndroid();

protected:

  // Undefined
  DisplayConnectionAndroid(const DisplayConnectionAndroid&);

  // Undefined
  DisplayConnectionAndroid& operator=(const DisplayConnectionAndroid& rhs);

private:
  EGLNativeDisplayType mDisplay;        ///< EGL display for rendering
  Integration::RenderSurface::Type mSurfaceType;     ///< The surface type
  GraphicsInterface* mGraphics;         ///< The graphics interface
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ANDROID_DISPLAY_CONNECTION_IMPL_ECORE_WL_H
