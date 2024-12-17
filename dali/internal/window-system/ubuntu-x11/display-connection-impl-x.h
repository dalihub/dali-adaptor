#ifndef DALI_INTERNAL_WINDOWSYSTEM_ECOREX_DISPLAY_CONNECTION_IMPL_ECORE_X_H
#define DALI_INTERNAL_WINDOWSYSTEM_ECOREX_DISPLAY_CONNECTION_IMPL_ECORE_X_H

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
#include <dali/internal/window-system/common/display-connection-impl.h>
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>

namespace Dali
{
class DisplayConnection;

namespace Internal::Adaptor
{
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
   * @copydoc Dali::Internal::Adaptor::DisplayConnection::SetSurfaceType
   */
  void SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type) override;

public:
  /**
   * Destructor
   */
  ~DisplayConnectionX11() override;

  // Undefined
  DisplayConnectionX11(const DisplayConnectionX11&) = delete;

  // Undefined
  DisplayConnectionX11& operator=(const DisplayConnectionX11& rhs) = delete;

public:
  XDisplay* mDisplay; ///< X-display for rendering
};

} // namespace Internal::Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ECOREX_DISPLAY_CONNECTION_IMPL_ECORE_X_H
