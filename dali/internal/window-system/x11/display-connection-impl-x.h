#ifndef DALI_INTERNAL_WINDOW_SYSTEM_X11_DISPLAY_CONNECTION_IMPL_X_H
#define DALI_INTERNAL_WINDOW_SYSTEM_X11_DISPLAY_CONNECTION_IMPL_X_H

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
#include <X11/Xlib.h>
#include <dali/internal/window-system/common/display-connection-impl.h>

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
   * @copydoc Dali::DisplayConnection::ConsumeEvents
   */
  void ConsumeEvents() override;

  /**
   * @copydoc Dali::Internal::Adaptor::DisplayConnection::SetSurfaceType
   */
  void SetSurfaceType(Dali::RenderSurfaceInterface::Type type) override;

public:
  /**
   * Destructor
   */
  ~DisplayConnectionX11() override;

  DisplayConnectionX11(const DisplayConnectionX11&) = delete;

  DisplayConnectionX11& operator=(const DisplayConnectionX11& rhs) = delete;

public:
  ::Display* mDisplay; ///< X-display for rendering
};

} // namespace Internal::Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_X11_DISPLAY_CONNECTION_IMPL_X_H
