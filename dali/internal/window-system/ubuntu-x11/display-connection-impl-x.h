#ifndef DALI_INTERNAL_WINDOWSYSTEM_ECOREX_DISPLAY_CONNECTION_IMPL_ECORE_X_H
#define DALI_INTERNAL_WINDOWSYSTEM_ECOREX_DISPLAY_CONNECTION_IMPL_ECORE_X_H

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

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-connection-impl.h>
#include <dali/public-api/object/base-object.h>
#include <dali/internal/graphics/gles20/egl-implementation.h>
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>

namespace Dali
{

class RenderSurface;
class DisplayConnection;

namespace Internal
{

namespace Adaptor
{


class Impl
{
public:

  XDisplay*  mDisplay;        ///< X-display for rendering
};


/**
 * DisplayConnection implementation
 */
class DisplayConnectionX11 : public Dali::Internal::Adaptor::DisplayConnection
{
public:

  /**
   * @brief Default constructor
   */
  DisplayConnectionX11();

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
  virtual ~DisplayConnectionX11();

protected:

  // Undefined
  DisplayConnectionX11(const DisplayConnectionX11&) = delete;

  // Undefined
  DisplayConnectionX11& operator=(const DisplayConnectionX11& rhs) = delete;

private:

public:

  XDisplay*  mDisplay;        ///< X-display for rendering

};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ECOREX_DISPLAY_CONNECTION_IMPL_ECORE_X_H
