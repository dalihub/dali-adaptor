#pragma once

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/egl-implementation.h>

namespace Dali::Internal::Adaptor
{

/**
 * DisplayConnection implementation
 */
class DisplayConnectionCocoa : public Dali::Internal::Adaptor::DisplayConnection
{
public:

  /**
   * @brief Default constructor
   */
  DisplayConnectionCocoa();

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
   * @copydoc Dali::DisplayConnection::InitializeEgl
   */
  bool InitializeEgl(EglInterface& egl);

  /**
  * @copydoc Dali::DisplayConnection::InitializeGraphics
  */
  bool InitializeGraphics();

  /**
  * @copydoc Dali::Internal::Adaptor::DisplayConnection::SetSurfaceType
  */
  void SetSurfaceType( Dali::RenderSurfaceInterface::Type type );

  /**
  * @copydoc Dali::Internal::Adaptor::DisplayConnection::SetGraphicsInterface
  */
  void SetGraphicsInterface( GraphicsInterface& graphics );

public:

  /**
   * Destructor
   */
  virtual ~DisplayConnectionCocoa();

private:

  // Undefined
  DisplayConnectionCocoa(const DisplayConnectionCocoa&) = delete;

  // Undefined
  DisplayConnectionCocoa& operator=(const DisplayConnectionCocoa& rhs) = delete;

private:

  GraphicsInterface *mGraphics; ///< The graphics interface
};

} // namespace Dali::Internal::Adaptor
