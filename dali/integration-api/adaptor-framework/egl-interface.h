#ifndef DALI_INTEGRATION_EGL_INTERFACE_H
#define DALI_INTEGRATION_EGL_INTERFACE_H

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
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/math/rect.h>

namespace Dali
{
/**
 * EglInterface provides an interface for managing EGL contexts
 */
class EglInterface
{
public:
  /**
   * Choose config of egl
   * @param isWindowType whether the config for window or pixmap
   * @param colorDepth Bit per pixel value (ex. 32 or 24)
   * @return true if the eglChooseConfig is succeed.
   */
  virtual bool ChooseConfig(bool isWindowType, ColorDepth depth) = 0;

  /**
   * Create the OpenGL context.
   * @return true if successful
   */
  virtual bool CreateContext() = 0;

  /**
   * Create the OpenGL context for the window.
   * @return true if successful
   */
  virtual bool CreateWindowContext(EGLContext& mEglContext) = 0;

  /**
   * Create an OpenGL surface using a window
   * @param window The window to create the surface on
   * @param colorDepth Bit per pixel value (ex. 32 or 24)
   * @return Handle to an on-screen EGL window surface (the requester has an ownership of this egl surface)
   */
  virtual EGLSurface CreateSurfaceWindow(EGLNativeWindowType window, ColorDepth depth) = 0;

  /**
   * Destroy the OpenGL context.
   */
  virtual void DestroyContext(EGLContext& eglContext) = 0;

  /**
   * Destroy the OpenGL surface.
   */
  virtual void DestroySurface(EGLSurface& eglSurface) = 0;

  /**
   * Make the OpenGL context current
   */
  virtual void MakeContextCurrent(EGLSurface eglSurface, EGLContext eglContext) = 0;

  /**
   * Terminate GL
   */
  virtual void TerminateGles() = 0;

  /**
   * Replaces the render surface
   * @param[in] window, the window to create the new surface on
   * @return true if the context was lost due to a change in display
   *         between old surface and new surface
   */
  virtual bool ReplaceSurfaceWindow(EGLNativeWindowType window, EGLSurface& eglSurface, EGLContext& eglContext) = 0;

  /**
   * @brief Returns whether the partial update is required.
   * @return true if the partial update is required.
   */
  virtual bool IsPartialUpdateRequired() const = 0;

  /**
   * Gets current back buffer age
   */
  virtual int GetBufferAge(EGLSurface& eglSurface) const = 0;

  /**
   * Performs an OpenGL set damage command with damaged rects
   */
  virtual void SetDamageRegion(EGLSurface& eglSurface, std::vector<Rect<int>>& damagedRects) = 0;

  /**
   * Performs an OpenGL swap buffers command
   */
  virtual void SwapBuffers(EGLSurface& eglSurface) = 0;

  /**
   * Performs an OpenGL swap buffers command
   */
  virtual void SwapBuffers(EGLSurface& eglSurface, const std::vector<Rect<int>>& damagedRects) = 0;

  /**
   * Performs an OpenGL copy buffers command
   */
  virtual void CopyBuffers(EGLSurface& eglSurface) = 0;

  /**
   * Performs an EGL wait GL command
   */
  virtual void WaitGL() = 0;

protected:
  /**
   * Virtual protected destructor, no deletion through this interface
   */
  virtual ~EglInterface()
  {
  }
};

} // namespace Dali

#endif // DALI_INTEGRATION_EGL_INTERFACE_H
