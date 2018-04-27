#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/render-surface.h>

namespace Dali
{

class TriggerEventInterface;

namespace Internal
{
namespace Adaptor
{

/**
 * Window interface of render surface.
 */
class WindowRenderSurface : public Dali::RenderSurface
{
public:

  /**
   * @brief Default constructor
   */
  WindowRenderSurface() = default;

  /**
   * @brief Destructor
   */
  virtual ~WindowRenderSurface() = default;

public: // API

  /**
   * @brief Get the render surface the adaptor is using to render to.
   * @return reference to current render surface
   */
  virtual Any GetWindow() = 0;

  /**
   * @brief Map window
   */
  virtual void Map() = 0;

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   * @param renderNotification to use
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) = 0;

  /**
   * @brief Sets whether the surface is transparent or not.
   * @param[in] transparent Whether the surface is transparent
   */
  virtual void SetTransparency( bool transparent ) = 0;

  /**
   * Request surface rotation
   * @param[in] angle A new angle of the surface
   * @param[in] width A new width of the surface
   * @param[in] height A new height of the surface
   */
  virtual void RequestRotation( int angle, int width, int height ) = 0;

protected:

  /**
   * @brief Second stage construction
   */
  virtual void Initialize( Any surface ) = 0;

  /**
   * @brief Create window
   */
  virtual void CreateRenderable() = 0;

  /**
   * @brief Use an existing render surface
   * @param surfaceId the id of the surface
   */
  virtual void UseExistingRenderable( unsigned int surfaceId ) = 0;

protected:

  // Undefined
  WindowRenderSurface(const WindowRenderSurface&) = delete;

  // Undefined
  WindowRenderSurface& operator=(const WindowRenderSurface& rhs) = delete;

}; // class WindowRenderSurface

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_RENDER_SURFACE_H
