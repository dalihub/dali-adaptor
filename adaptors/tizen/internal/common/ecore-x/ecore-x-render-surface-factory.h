#ifndef __DALI_INTERNAL_ADAPTOR_ECOREX_RENDER_SURFACE_FACTORY_H__
#define __DALI_INTERNAL_ADAPTOR_ECOREX_RENDER_SURFACE_FACTORY_H__

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

// EXTERNAL INCLUDES
#include <boost/any.hpp>
#include <string>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES
#include <native-buffer-pool.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace ECoreX
{
class RenderSurface;

/**
 * Surface factory function for pixmap
 * A pixmap surface is created.
 *
 * @param [in] type the type of surface to create
 * @param [in] positionSize the position and size of the surface to create
 * @param [in] display X Pixmap to use, or null for default.
 * @param [in] display X Display to use, or null for default.
 * @param [in] name Name of surface passed in
 * @param [in] isTransparent Whether the surface has an alpha channel
 */
DALI_IMPORT_API RenderSurface* CreatePixmapSurface(
  PositionSize       positionSize,
  boost::any         surface,
  boost::any         display,
  const std::string& name,
  bool               isTransparent );

/**
 * Surface factory function for Native buffer
 * A native buffer surface is created.
 * @param [in] provider The provider
 * @param [in] pool The native buffer pool
 * @param [in] maxBufferCount The maximum number of buffers to create
 * @param [in] type the type of surface to create
 * @param [in] positionSize the position and size of the surface to create
 * @param [in] display X Pixmap to use, or null for default.
 * @param [in] display X Display to use, or null for default.
 * @param [in] name Name of surface passed in
 * @param [in] isTransparent Whether the surface has an alpha channel
 */
DALI_IMPORT_API RenderSurface* CreateNativeBufferSurface(
  native_buffer_provider* provider,
  native_buffer_pool*     pool,
  unsigned int            maxBufferCount,
  PositionSize            positionSize,
  boost::any              surface,
  boost::any              display,
  const std::string&      name,
  bool                    isTransparent );



}// ECoreX
}// Adaptor
}// Internal
}// Dali

#endif //  __DALI_INTERNAL_ADAPTOR_ECOREX_RENDER_SURFACE_FACTORY_H__
