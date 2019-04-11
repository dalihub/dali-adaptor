#ifndef __DALI_NATIVE_RENDER_SURFACE_H__
#define __DALI_NATIVE_RENDER_SURFACE_H__

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
#include <dali/public-api/dali-adaptor-common.h>

#ifdef DALI_ADAPTOR_COMPILATION
#include <dali/integration-api/render-surface-interface.h>
#else
#include <dali/integration-api/adaptors/render-surface-interface.h>
#endif

namespace Dali
{

class TriggerEventInterface;

/**
 * Native interface of render surface.
 */
class DALI_ADAPTOR_API NativeRenderSurface : public Dali::RenderSurfaceInterface
{
public:

  /**
   * @brief Default constructor
   */
  NativeRenderSurface() = default;

  /**
   * @brief Destructor
   */
  virtual ~NativeRenderSurface() = default;

public: // API

  /**
   * @brief Get the render surface the adaptor is using to render to.
   * @return reference to current render surface
   */
  virtual Any GetDrawable() = 0;

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   * @param renderNotification to use
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) = 0;

  /**
   * @brief Waits until surface is replaced
   * After tbm surface is acquired in PostRender, this function is finished.
   */
  virtual void WaitUntilSurfaceReplaced() = 0;

private: // from NativeRenderSurface

  /**
   * @brief Create a renderable
   */
  virtual void CreateNativeRenderable() = 0;

  /**
   * @brief Release a drawable
   */
  virtual void ReleaseDrawable() = 0;

protected:

  // Undefined
  NativeRenderSurface(const NativeRenderSurface&) = delete;

  // Undefined
  NativeRenderSurface& operator=(const NativeRenderSurface& rhs) = delete;

};

} // namespace Dali

#endif // __DALI_NATIVE_RENDER_SURFACE_H__
