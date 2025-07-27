#ifndef DALI_NATIVE_RENDER_SURFACE_H
#define DALI_NATIVE_RENDER_SURFACE_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * Native interface of render surface.
 */
class DALI_ADAPTOR_API NativeRenderSurface : public Dali::Integration::RenderSurfaceInterface
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
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   * @param renderNotification to use
   */
  virtual void SetRenderNotification(TriggerEventInterface* renderNotification) = 0;

  /**
   * @brief Get the native renderable handle
   * @return The native renderable handle
   */
  virtual Any GetNativeRenderable() = 0;

  /**
   * @brief Sets a callback that is called when the frame rendering is done by the graphics driver.
   * @param callback The function to call
   */
  virtual void SetFrameRenderedCallback(CallbackBase* callback) = 0;

private: // from NativeRenderSurface
  /**
   * @brief Create a renderable
   */
  virtual void CreateNativeRenderable() = 0;

protected:
  // Undefined
  NativeRenderSurface(const NativeRenderSurface&) = delete;

  // Undefined
  NativeRenderSurface& operator=(const NativeRenderSurface& rhs) = delete;
};

} // namespace Dali

#endif // DALI_NATIVE_RENDER_SURFACE_H
