#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_PIXMAP_RENDER_SURFACE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_PIXMAP_RENDER_SURFACE_H

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

#ifdef DALI_ADAPTOR_COMPILATION
#include <dali/integration-api/render-surface-interface.h>
#include <dali/graphics/graphics-interface.h>
#else
#include <dali/integration-api/adaptors/render-surface-interface.h>
#endif


namespace Dali
{

class TriggerEventInterface;

namespace Internal
{
namespace Adaptor
{

/**
 * Pixmap interface of render surface.
 */
class PixmapRenderSurface : public Dali::RenderSurfaceInterface
{
public:

  /**
   * @brief Default constructor
   */
  PixmapRenderSurface() = default;

  /**
   * @brief Destructor
   */
  virtual ~PixmapRenderSurface() = default;

public: // API

  /**
   * @brief Get the render surface the adaptor is using to render to.
   * @return reference to current render surface
   */
  virtual Any GetSurface() = 0;

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   * @param renderNotification to use
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) = 0;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() override
  {
    return mGraphics ? mGraphics->GetDepthBufferRequired() : Integration::DepthBufferAvailable::FALSE;
  }

  /**
   * @copydoc Dali::Integration::RenderSurface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() override
  {
    return mGraphics ? mGraphics->GetStencilBufferRequired() : Integration::StencilBufferAvailable::FALSE;
  }

private:

  /**
   * Second stage construction
   */
  virtual void Initialize( Any surface ) = 0;

  /**
   * @brief Create a renderable
   */
  virtual void CreateRenderable() = 0;

  /**
   * @brief Use an existing render surface
   * @param surfaceId the id of the surface
   */
  virtual void UseExistingRenderable( unsigned int surfaceId ) = 0;

protected:

  // Undefined
  PixmapRenderSurface(const PixmapRenderSurface&) = delete;

  // Undefined
  PixmapRenderSurface& operator=(const PixmapRenderSurface& rhs) = delete;

};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_PIXMAP_RENDER_SURFACE_H
