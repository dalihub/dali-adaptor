#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/any.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/integration-api/render-surface.h>
#include <dali/graphics/graphics-interface.h>


namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class DisplayConnection;
}
}

class DisplayConnection
{
public:

  /**
   * @brief Create an initialized DisplayConnection.
   *
   * @param[in] graphics The abstracted graphics interface
   * @return A handle to a newly allocated DisplayConnection resource.
   */
  static DisplayConnection* New( Dali::Graphics::GraphicsInterface& graphics );

  /**
   * @brief Create an initialized DisplayConnection.
   * Native surface will need this instead of DisplayConnection::New()
   *
   * @param[in] graphics The abstracted graphics interface
   * @param[in] type Render surface type
   * @return A handle to a newly allocated DisplayConnection resource.
   */
  static DisplayConnection* New( Dali::Graphics::GraphicsInterface& graphics, RenderSurface::Type type );

  /**
   * @brief Create a DisplayConnection handle; this can be initialised with DisplayConnection::New().
   *
   * Calling member functions with an uninitialised handle is not allowed.
   */
  DisplayConnection();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~DisplayConnection();

  /**
   * @brief Get display
   *
   * @return display
   */
  Any GetDisplay();

  /**
   * @brief Consumes any possible events on the queue so that there is no leaking between frames
   */
  void ConsumeEvents();

  /**
   * @brief Initialize the display
   */
  bool Initialize();

public:

  /**
   * @brief This constructor is used by DisplayConnection New() methods.
   *
   * @param [in] handle A pointer to a newly allocated DisplayConnection resource
   */
  explicit DALI_INTERNAL DisplayConnection(Internal::Adaptor::DisplayConnection* impl);

private:

  std::unique_ptr<Internal::Adaptor::DisplayConnection> mImpl;
};

}

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_H
