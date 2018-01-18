#ifndef DALI_INTERNAL_WINDOWSYSTEM_WAYLAND_DISPLAY_CONNECTION_IMPL_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_WAYLAND_DISPLAY_CONNECTION_IMPL_WL_H

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <wayland-egl.h>
#include <dali/integration-api/wayland/wl-types.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/public-api/object/base-object.h>
#include <dali/internal/graphics/gles20/egl-implementation.h>

namespace Dali
{

class RenderSurface;
class DisplayConnection;

namespace Internal
{

namespace Adaptor
{

/**
 * DisplayConnection implementation
 */
class DisplayConnection : public Dali::BaseObject
{
public:

  /**
   * @brief Default constructor
   */
  DisplayConnection();

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
   * @copydoc Dali::DisplayConnection::GetDpi
   */
  static void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical);

  /**
   * @copydoc Dali::DisplayConnection::ConsumeEvents
   */
  void ConsumeEvents();

  /**
   * @copydoc Dali::DisplayConnection::InitializeEgl
   */
  bool InitializeEgl(EglInterface& egl);

  void SetSurfaceType( RenderSurface::Type type );

public:

  /**
   * Destructor
   */
  virtual ~DisplayConnection();

protected:

  EGLNativeDisplayType GetNativeDisplay();

  void ReleaseNativeDisplay();

  // Undefined
  DisplayConnection(const DisplayConnection&);

  // Undefined
  DisplayConnection& operator=(const DisplayConnection& rhs);

private:
  WlDisplay*   mDisplay;        ///< Wayland-display for rendering
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_WAYLAND_DISPLAY_CONNECTION_IMPL_WL_H
