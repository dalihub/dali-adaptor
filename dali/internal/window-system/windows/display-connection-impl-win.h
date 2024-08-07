#ifndef DALI_INTERNAL_WIN_DIPLAY_CONNECTION_H
#define DALI_INTERNAL_WIN_DIPLAY_CONNECTION_H

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
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/window-system/common/display-connection-impl.h>
#include <dali/public-api/object/base-object.h>

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
class DisplayConnectionWin : public Dali::Internal::Adaptor::DisplayConnection
{
public:
  /**
   * @brief Default constructor
   */
  DisplayConnectionWin();

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
  * @copydoc Dali::Internal::Adaptor::DisplayConnection::SetSurfaceType
  */
  void SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type) override;

public:
  /**
   * Destructor
   */
  ~DisplayConnectionWin() override;

  // Undefined
  DisplayConnectionWin(const DisplayConnectionWin&) = delete;

  // Undefined
  DisplayConnectionWin& operator=(const DisplayConnectionWin& rhs) = delete;

private:
  GraphicsInterface* mGraphics; ///< The graphics interface
  HDC                mDisplay{};
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WIN_DIPLAY_CONNECTION_H
