#ifndef __DALI_RENDER_SURFACE_H__
#define __DALI_RENDER_SURFACE_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @addtogroup CAPI_DALI_FRAMEWORK
 * @{
 */

// EXTERNAL INCLUDES
#include <boost/any.hpp>
#include <string>
#include <dali/public-api/math/rect.h>

namespace Dali DALI_IMPORT_API
{

typedef Dali::Rect<int> PositionSize;

/**
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
   * enumeration of surface types
   */
  enum SurfaceType
  {
    NO_SURFACE,     ///< not configured
    PIXMAP,         ///< Pixmap
    WINDOW,         ///< Window
    NATIVE_BUFFER   ///< Native Buffer
  };

  /**
   * When application uses pixmap surface, it can select rendering mode
   * RENDER_SYNC : application should call RenderSync() after posting the offscreen to onscreen
   * RENDER_#FPS : the maximum performance will be limited designated number of frame
   */
  enum RenderMode
  {
    RENDER_DEFAULT = -1,
    RENDER_SYNC = 0,
    RENDER_24FPS = 24,
    RENDER_30FPS = 30,
    RENDER_60FPS = 60
  };

  /**
   * Constructor
   * Application or Adaptor needs to create the appropriate concrete RenderSurface type.
   * @see CreateDefaultSurface
   */
  RenderSurface();

  /**
   * Virtual Destructor
  */
  virtual ~RenderSurface();

  /**
   * @returns the surface type
   */
  virtual SurfaceType GetType() = 0;

  /**
   * Returns the window or pixmap surface
   * @return surface
   */
  virtual boost::any GetSurface() = 0;

  /**
   * Returns the display
   * @return display
   */
  virtual boost::any GetDisplay() = 0;

  /**
   * Return the size and position of the surface
   */
  virtual PositionSize GetPositionSize() const = 0;

  /**
   * Set frame update rate for pixmap surface type
   */
  virtual void SetRenderMode(RenderMode mode) = 0;

  /**
   * Get current fps for pixmap surface type
   */
  virtual RenderMode GetRenderMode() const = 0;

private:

  /**
   * Undefined copy constructor and assignment operator. RenderSurface cannot be copied
   */
  RenderSurface( const RenderSurface& rhs );
  RenderSurface& operator=( const RenderSurface& rhs );

};

/**
 * Default surface factory function.
 * A surface is created with the given type.
 *
 * @param [in] type the type of surface to create
 * @param [in] positionSize the position and size of the surface to create
 * @param [in] name optional name of surface passed in
 */
RenderSurface* CreateDefaultSurface( RenderSurface::SurfaceType type, PositionSize positionSize, const std::string& name = "" );

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_RENDER_SURFACE_H__
