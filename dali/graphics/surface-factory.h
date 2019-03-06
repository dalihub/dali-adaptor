#ifndef DALI_GRAPHICS_SURFACE_FACTORY_H
#define DALI_GRAPHICS_SURFACE_FACTORY_H

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

#include <dali/public-api/object/any.h>
#include <memory>

namespace Dali
{
class RenderSurface;

namespace Graphics
{

/**
 * Native window interface
 */
class NativeWindowInterface
{
public:

  /**
   * @brief Get the native window handle
   * @return The native window handle
   */
  virtual Any GetNativeWindow() = 0;

  /**
   * @brief Get the native window id
   * @return The native window id
   */
  virtual int GetNativeWindowId() = 0;
};

class SurfaceFactory
{
public:
  SurfaceFactory()          = default;
  virtual ~SurfaceFactory() = default;

  /**
   * Create a new surface factory
   * @param[in] renderSurface The render surface for which the surface factory will create a graphics surface.
   */
  static std::unique_ptr<SurfaceFactory> New( NativeWindowInterface& renderSurface );

  SurfaceFactory( const SurfaceFactory& ) = delete;
  SurfaceFactory& operator=( const SurfaceFactory& ) = delete;
};

} // Namespace Graphics
} // Namespace Dali

#endif // DALI_GRAPHICS_SURFACE_FACTORY_H
