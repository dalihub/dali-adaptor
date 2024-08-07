#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_IMPL_H

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
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/public-api/object/base-object.h>

namespace Dali
{
class DisplayConnection;

namespace Internal::Adaptor
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
  DisplayConnection() = default;

public:
  /**
   * @copydoc Dali::DisplayConnection::GetDisplay
   */
  virtual Any GetDisplay() = 0;

  /**
   * @brief Get display for the graphics backend
   *
   * @return EGLNativeDisplayType or vulkan display type
   */
  virtual Any GetNativeGraphicsDisplay() = 0;

  /**
   * @copydoc Dali::DisplayConnection::ConsumeEvents
   */
  virtual void ConsumeEvents() = 0;

  /**
   * Sets the render surface type
   * @param[in] type The render surface type
   */
  virtual void SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type) = 0;

public:
  /**
   * Destructor
   */
  ~DisplayConnection() override = default;

  // Undefined
  DisplayConnection(const DisplayConnection&) = delete;

  // Undefined
  DisplayConnection& operator=(const DisplayConnection& rhs) = delete;
};

} // namespace Internal::Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_IMPL_H
