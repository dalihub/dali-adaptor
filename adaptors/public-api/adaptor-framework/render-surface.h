#ifndef __DALI_RENDER_SURFACE_H__
#define __DALI_RENDER_SURFACE_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <string>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/any.h>

namespace Dali
{

/**
 * @brief The position and size of the render surface.
 */
typedef Dali::Rect<int> PositionSize;

/**
 * @brief Interface for a render surface onto which Dali draws.
 *
 * Dali::Adaptor requires a render surface to draw on to. This is
 * usually a window in the native windowing system, or some other
 * mapped pixel buffer.
 *
 * Dali::Application will automatically create a render surface using a window.
 *
 * The implementation of the factory method below should choose an appropriate
 * implementation of RenderSurface for the given platform
 */
class RenderSurface
{
public:
  /**
   * @brief enumeration of surface types
   */
  enum SurfaceType
  {
    NO_SURFACE,     ///< not configured
    PIXMAP,         ///< Pixmap
    WINDOW,         ///< Window
    NATIVE_BUFFER   ///< Native Buffer
  };

  /**
   * @brief Constructor
   * Inlined as this is a pure abstract interface
   */
  RenderSurface() {}

  /**
   * @brief Virtual Destructor.
   * Inlined as this is a pure abstract interface
   */
  virtual ~RenderSurface() {}

  /**
   * @brief returns the surface type.
   * @return the surface type
   */
  virtual SurfaceType GetType() = 0;

  /**
   * @brief Returns the window or pixmap surface.
   * @return surface
   */
  virtual Any GetSurface() = 0;

  /**
   * @brief Returns the display.
   * @return display
   */
  virtual Any GetDisplay() = 0;

  /**
   * @brief Return the size and position of the surface.
   * @return The position and size
   */
  virtual PositionSize GetPositionSize() const = 0;

private:

  /**
   * @brief Undefined copy constructor. RenderSurface cannot be copied
   */
  RenderSurface( const RenderSurface& rhs );

  /**
   * @brief Undefined assignment operator. RenderSurface cannot be copied
   */
  RenderSurface& operator=( const RenderSurface& rhs );

};

} // namespace Dali

#endif // __DALI_RENDER_SURFACE_H__
