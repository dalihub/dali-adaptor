#ifndef DALI_INTERNAL_WINDOWSYSTEM_ECOREWL_DISPLAY_CONNECTION_IMPL_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_ECOREWL_DISPLAY_CONNECTION_IMPL_ECORE_WL_H

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
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
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
class DisplayConnectionEcoreWl : public Dali::Internal::Adaptor::DisplayConnection
{
public:
  /**
   * @brief Default constructor
   */
  DisplayConnectionEcoreWl();

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
  ~DisplayConnectionEcoreWl() override;

  // Undefined
  DisplayConnectionEcoreWl(const DisplayConnectionEcoreWl&) = delete;

  // Undefined
  DisplayConnectionEcoreWl& operator=(const DisplayConnectionEcoreWl& rhs) = delete;

private:
  wl_display*                               mDisplay;     ///< Wayland-display for rendering
  Integration::RenderSurfaceInterface::Type mSurfaceType; ///< The surface type
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ECOREWL_DISPLAY_CONNECTION_IMPL_ECORE_WL_H
