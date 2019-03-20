#ifndef DALI_ANDROID_PIXMAP_RENDER_SURFACE_H
#define DALI_ANDROID_PIXMAP_RENDER_SURFACE_H

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

// INTERNAL INCLUDES
#include <dali/integration-api/egl-interface.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/window-system/common/pixmap-render-surface.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/graphics/common/graphics-interface.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/threading/conditional-wait.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class PixmapRenderSurfaceAndroid : public PixmapRenderSurface
{
public:

  /**
    * @param [in] positionSize the position and size of the surface
    * @param [in] surface can be a X-window or X-pixmap (type must be unsigned int).
    * @param [in] isTransparent if it is true, surface has 32 bit color depth, otherwise, 24 bit
    */
  PixmapRenderSurfaceAndroid( Dali::PositionSize positionSize, Any surface, bool isTransparent = false );

  /**
   * @brief Destructor
   */
  virtual ~PixmapRenderSurfaceAndroid();

public: // from WindowRenderSurface

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::GetSurface()
   */
  virtual Any GetSurface() override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::SetRenderNotification()
   */
  virtual void SetRenderNotification( TriggerEventInterface* renderNotification ) override;

public: // from Dali::Integration::RenderSurface

  /**
   * @copydoc Dali::Integration::RenderSurface::GetPositionSize()
   */
  virtual PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetDpi()
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::InitializeGraphics()
   */
  virtual void InitializeGraphics() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::CreateSurface()
   */
  virtual void CreateSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::DestroySurface()
   */
  virtual void DestroySurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::ReplaceGraphicsSurface()
   */
  virtual bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::MoveResize()
   */
  virtual void MoveResize( Dali::PositionSize positionSize) override {}

  /**
   * @copydoc Dali::Integration::RenderSurface::StartRender()
   */
  virtual void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::PreRender()
   */
  virtual bool PreRender( bool resizingSurface ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::PostRender()
   */
  virtual void PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::StopRender()
   */
  virtual void StopRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::SetThreadSynchronization
   */
  virtual void SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization ) override;

  /**
   * @copydoc Dali::Integration::RenderSurface::GetSurfaceType()
   */
  virtual Integration::RenderSurface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::Integration::RenderSurface::MakeContextCurrent()
   */
  virtual void MakeContextCurrent() override;

private: // from PixmapRenderSurface

  /**
   * @copydoc Dali::Integration::RenderSurface::ReleaseLock()
   */
  virtual void ReleaseLock() override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::Initialize()
   */
  virtual void Initialize( Any surface ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::Initialize()
   */
  virtual void CreateRenderable() override;

  /**
   * @copydoc Dali::Internal::Adaptor::PixmapRenderSurface::Initialize()
   */
  virtual void UseExistingRenderable( unsigned int surfaceId ) override;

private:

  ANativeWindow* GetWindow( Any surface ) const;

private: // Data

  static const int BUFFER_COUNT = 2;
  GraphicsInterface*              mGraphics;               ///< Graphics interface
  Dali::DisplayConnection*        mDisplayConnection;      ///< Display connection
  PositionSize                    mPosition;               ///< Position
  TriggerEventInterface*          mRenderNotification;     ///< Render notification trigger
  ColorDepth                      mColorDepth;             ///< Color depth of surface (32 bit or 24 bit)
  bool                            mOwnSurface;             ///< Whether we own the surface (responsible for deleting it)

  int                             mProduceBufferIndex;
  int                             mConsumeBufferIndex;
  EGLSurface                      mEglSurfaces[BUFFER_COUNT];
  ThreadSynchronizationInterface* mThreadSynchronization;     ///< A pointer to the thread-synchronization
  ConditionalWait                 mPixmapCondition;           ///< condition to share pixmap
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_ANDROID_PIXMAP_RENDER_SURFACE_H
