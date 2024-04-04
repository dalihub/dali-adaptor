#ifndef __DALI_INTEGRATION_NATIVE_RENDER_SURFACE_FACTORY_H__
#define __DALI_INTEGRATION_NATIVE_RENDER_SURFACE_FACTORY_H__

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

// EXTERNAL INCLUDES
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

class NativeRenderSurface;

/**
 * Factory function for native surface
 * A native surface is created.
 *
 * @param [in] positionSize the position and size of the surface to create
 * @param [in] isTransparent Whether the surface has an alpha channel
 */
NativeRenderSurface* CreateNativeSurface( PositionSize positionSize,
                                          bool isTransparent );

} // namespace Dali

#endif // __DALI_INTEGRATION_NATIVE_RENDER_SURFACE_FACTORY_H__
