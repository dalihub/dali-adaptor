#ifndef DALI_INTERNAL_EGL_IMPLEMENTATION_H
#define DALI_INTERNAL_EGL_IMPLEMENTATION_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/list-wrapper.h>
#include <dali/public-api/common/vector-wrapper.h>

#include <dali/integration-api/core-enumerations.h>
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/math/rect.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/egl-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * EglImplementation class provides an EGL implementation.
 */
class EglImplementation : public EglInterface
{
public:
  /**
   * Constructor
   * @param[in] multiSamplingLevel The Multi-sampling level required
   * @param[in] depthBufferRequired Whether the depth buffer is required
   * @param[in] stencilBufferRequired Whether the stencil buffer is required
   * @param[in] partialUpdatedRequired Whether the partial update is required
   */
  EglImplementation(int                                 multiSamplingLevel,
                    Integration::DepthBufferAvailable   depthBufferRequired,
                    Integration::StencilBufferAvailable stencilBufferRequired,
                    Integration::PartialUpdateAvailable partialUpdateRequired);

  /**
   * Destructor
   */
  ~EglImplementation() override;

public:
  /**
   * (Called from RenderSurface, not RenderThread, so not in i/f, hence, not virtual)
   * Initialize GL
   * @param display The display
   * @param isOwnSurface whether the surface is own or not
   * @return true on success, false on failure
   */
  bool InitializeGles(EGLNativeDisplayType display, bool isOwnSurface = true);

  /**
    * Create the OpenGL context for the shared resource.
    * @return true if successful
    */
  bool CreateContext() override;

  /**
    * Create the OpenGL context for the window.
    * @return true if successful
    */
  bool CreateWindowContext(EGLContext& mEglContext);

  /**
    * Destroy the OpenGL context.
    */
  void DestroyContext(EGLContext& eglContext);

  /**
    * Destroy the OpenGL surface.
    */
  void DestroySurface(EGLSurface& eglSurface);

  /**
   * Make the OpenGL context current
   */
  void MakeContextCurrent(EGLSurface eglSurface, EGLContext eglContext) override;

  /**
   * clear the OpenGL context
   */
  void MakeContextNull();

  /**
   * @brief Make the OpenGL surface current
   *
   * @param pixmap The pixmap to replace the current surface
   * @param eglSurface The eglSurface to replace the current OpenGL surface.
   */
  void MakeCurrent(EGLNativePixmapType pixmap, EGLSurface eglSurface);

  /**
   * Terminate GL
   */
  void TerminateGles() override;

  /**
   * Checks if GL is initialised
   * @return true if it is
   */
  bool IsGlesInitialized() const;

  /**
   * Performs an OpenGL swap buffers command
   */
  void SwapBuffers(EGLSurface& eglSurface) override;

  /**
   * Gets current back buffer age
   */
  EGLint GetBufferAge(EGLSurface& eglSurface) const;

  /**
   * Performs an OpenGL set damage command with damaged rects
   */
  void SetDamageRegion(EGLSurface& eglSurface, std::vector<Rect<int>>& damagedRects);

  /**
   * Performs an OpenGL swap buffers command with damaged rects
   */
  virtual void SwapBuffers(EGLSurface& eglSurface, const std::vector<Rect<int>>& damagedRects);

  /**
   * Performs an OpenGL copy buffers command
   */
  void CopyBuffers(EGLSurface& eglSurface) override;

  /**
   * Performs an EGL wait GL command
   */
  void WaitGL() override;

  /**
   * Choose config of egl
   * @param isWindowType whether the config for window or pixmap
   * @param colorDepth Bit per pixel value (ex. 32 or 24)
   * @return true if the eglChooseConfig is succeed.
  */
  bool ChooseConfig(bool isWindowType, ColorDepth depth);

  /**
    * Create an OpenGL surface using a window
    * @param window The window to create the surface on
    * @param colorDepth Bit per pixel value (ex. 32 or 24)
    * @return Handle to an on-screen EGL window surface (the requester has an ownership of this egl surface)
    */
  EGLSurface CreateSurfaceWindow(EGLNativeWindowType window, ColorDepth depth);

  /**
   * Create the OpenGL surface using a pixmap
   * @param pixmap The pixmap to create the surface on
   * @param colorDepth Bit per pixel value (ex. 32 or 24)
   * @return Handle to an off-screen EGL pixmap surface (the requester has an ownership of this egl surface)
   */
  EGLSurface CreateSurfacePixmap(EGLNativePixmapType pixmap, ColorDepth depth);

  /**
   * Replaces the render surface
   * @param[in] window, the window to create the new surface on
   * @return true if the context was lost due to a change in display
   *         between old surface and new surface
   */
  bool ReplaceSurfaceWindow(EGLNativeWindowType window, EGLSurface& eglSurface, EGLContext& eglContext);

  /**
   * Replaces the render surface
   * @param[in] pixmap, the pixmap to replace the new surface on
   * @param[out] eglSurface, the eglSurface is created using a pixmap.
   * @return true if the context was lost due to a change in x-display
   *         between old surface and new surface
   */
  bool ReplaceSurfacePixmap(EGLNativePixmapType pixmap, EGLSurface& eglSurface);

  /**
   * Sets gles version
   */
  void SetGlesVersion(const int32_t glesVersion);

  /**
   * Sets Whether the frame is the first after Resume.
   */
  void SetFirstFrameAfterResume();

  /**
   * returns the display with which this object was initialized
   * @return the EGL Display.
   */
  EGLDisplay GetDisplay() const;

  /**
   * Returns the EGL context
   * @return the EGL context.
   */
  EGLContext GetContext() const;

  /**
   * Returns the gles version
   * @return the gles version
   */
  int32_t GetGlesVersion() const;

  /**
   * Returns whether the surfaceless context is supported
   * @return true if the surfaceless context is supported
   */
  bool IsSurfacelessContextSupported() const;

  /**
   * @brief Wait until all rendering calls for the currently context are executed
   */
  void WaitClient();

  /**
   * @brief Returns whether the partial update is required.
   * @return true if the partial update is required.
   */
  bool IsPartialUpdateRequired() const;

private:
  Vector<EGLint> mContextAttribs;

  EGLNativeDisplayType mEglNativeDisplay;

  EGLNativeWindowType mEglNativeWindow;

  EGLNativePixmapType mCurrentEglNativePixmap;

  EGLDisplay mEglDisplay;
  EGLConfig  mEglConfig;
  EGLContext mEglContext; ///< The resource context holding assets such as textures to be shared

  typedef std::vector<EGLContext> EglWindowContextContainer;
  EglWindowContextContainer       mEglWindowContexts; ///< The EGL context for the window

  EGLSurface mCurrentEglSurface;
  EGLContext mCurrentEglContext;

  typedef std::vector<EGLSurface> EglWindowSurfaceContainer;
  EglWindowSurfaceContainer       mEglWindowSurfaces; ///< The EGL surface for the window

  int32_t  mMultiSamplingLevel;
  int32_t  mGlesVersion;
  uint32_t mLogThreshold{0};

  ColorDepth mColorDepth;

  bool mGlesInitialized;
  bool mIsOwnSurface;
  bool mIsWindow;
  bool mDepthBufferRequired;
  bool mStencilBufferRequired;
  bool mPartialUpdateRequired;
  bool mIsSurfacelessContextSupported;
  bool mIsKhrCreateContextSupported;
  bool mLogEnabled{false};

  uint32_t                           mSwapBufferCountAfterResume;
  PFNEGLSETDAMAGEREGIONKHRPROC       mEglSetDamageRegionKHR;
  PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC mEglSwapBuffersWithDamageKHR;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_EGL_IMPLEMENTATION_H
