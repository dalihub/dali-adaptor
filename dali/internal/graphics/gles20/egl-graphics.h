#ifndef DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H
#define DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H

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

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles20/gl-proxy-implementation.h>
#include <dali/internal/graphics/gles20/gl-implementation.h>
#include <dali/integration-api/egl-interface.h>

#include <dali/internal/graphics/gles20/egl-implementation.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles20/egl-sync-implementation.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class EglGraphics : public GraphicsInterface
{
public:

  /**
   * Constructor
   */
  EglGraphics();

  /**
   * Destructor
   */
  virtual ~EglGraphics();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::Initialize()
   */
  void Initialize( EnvironmentOptions* environmentOptions ) override;

  /**
   * Creates the graphics interface for EGL
   * @return The graphics interface for EGL
   */
  EglInterface* Create();

  /**
   * Gets the GL abstraction
   * @return The GL abstraction
   */
  Integration::GlAbstraction& GetGlAbstraction() const;

  /**
   * Gets the implementation of EGL
   * @return The implementation of EGL
   */
  EglImplementation& GetEglImplementation() const;

  /**
   * Gets the graphics interface for EGL
   * @return The graphics interface for EGL
   */
  EglInterface& GetEglInterface() const;

  /**
   * @copydoc Dali::Integration::GlAbstraction& GetGlesInterface()
   */
  GlImplementation& GetGlesInterface();

  /**
   * Gets the implementation of GlSyncAbstraction for EGL.
   * @return The implementation of GlSyncAbstraction for EGL.
   */
  EglSyncImplementation& GetSyncImplementation();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable& GetDepthBufferRequired();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired();

  /**
   * Gets the EGL image extension
   * @return The EGL image extension
   */
  EglImageExtensions* GetImageExtensions();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsInterface::Destroy()
   */
  void Destroy() override;

private:
  // Eliminate copy and assigned operations
  EglGraphics(const EglGraphics& rhs) = delete;
  EglGraphics& operator=(const EglGraphics& rhs) = delete;


private:
  std::unique_ptr< GlImplementation > mGLES;                    ///< GL implementation
  std::unique_ptr< EglImplementation > mEglImplementation;      ///< EGL implementation
  std::unique_ptr< EglImageExtensions > mEglImageExtensions;    ///< EGL image extension
  std::unique_ptr< EglSyncImplementation > mEglSync;            ///< GlSyncAbstraction implementation for EGL

  int mMultiSamplingLevel;                                      ///< The multiple sampling level
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_BASE_GRAPHICS_IMPLEMENTATION_H__
