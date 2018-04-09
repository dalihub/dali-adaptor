#ifndef DALI_INTERNAL_WINDOWSYSTEM_ECOREWL_DISPLAY_CONNECTION_IMPL_ECORE_WL2_H
#define DALI_INTERNAL_WINDOWSYSTEM_ECOREWL_DISPLAY_CONNECTION_IMPL_ECORE_WL2_H

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

// EXTERNAL INCLUDES
#include <wayland-egl.h>
#include <Ecore_Wl2.h>
#include <dali/integration-api/wayland/wl-types.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-connection-impl.h>
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
class DisplayConnectionEcoreWl2 : public Dali::Internal::Adaptor::DisplayConnection
{
public:

  /**
   * @brief Default constructor
   */
  DisplayConnectionEcoreWl2();

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
   * @copydoc Dali::DisplayConnection::GetDpi with native window
   */
  static void GetDpi(Any nativeWindow, unsigned int& dpiHorizontal, unsigned int& dpiVertical);

  /**
   * @copydoc Dali::DisplayConnection::ConsumeEvents
   */
  void ConsumeEvents();

  /**
   * @copydoc Dali::DisplayConnection::InitializeEgl
   */
  bool InitializeEgl(EglInterface& egl);

  /**
   * @brief Sets surface type
   */
  void SetSurfaceType( RenderSurface::Type type );



public:

  /**
   * Destructor
   */
  virtual ~DisplayConnectionEcoreWl2();

protected:

  /**
   * @brief Gets display connection for native surface
   */
  EGLNativeDisplayType GetNativeDisplay();

  /**
   * @brief Release display connection for native surface
   */
  void ReleaseNativeDisplay();

  // Undefined
  DisplayConnectionEcoreWl2(const DisplayConnectionEcoreWl2&);

  // Undefined
  DisplayConnectionEcoreWl2& operator=(const DisplayConnectionEcoreWl2& rhs);

private:
  EGLNativeDisplayType mDisplay;        ///< Wayland-display for rendering
  RenderSurface::Type mSurfaceType;
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ECOREWL_DISPLAY_CONNECTION_IMPL_ECORE_WL2_H
