#ifndef __DALI_INTERNAL_NATIVE_BUFFER_RENDER_SURFACE_H__
#define __DALI_INTERNAL_NATIVE_BUFFER_RENDER_SURFACE_H__

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
#include <native-buffer-pool.h>
#include <dali/public-api/common/vector-wrapper.h>

// INTERNAL INCLUDES
#include <internal/common/ecore-x/ecore-x-render-surface.h>

using namespace std;

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace ECoreX
{

/**
 * NativeBuffer API compatible implementation of RenderSurface.
 */
class NativeBufferRenderSurface : public RenderSurface
{
public:

  /**
   * Constructor
   */
  NativeBufferRenderSurface( native_buffer_provider* provider,
                             native_buffer_pool* pool,
                             unsigned int maxBufferCount,
                             Dali::PositionSize positionSize,
                             Any surface,
                             Any display,
                             const std::string& name,
                             bool isTransparent );

  /**
   * Destructor
   */
  virtual ~NativeBufferRenderSurface();

public: // API

  /**
   * @copydoc Dali::Internal::Adaptor::ECoreX::RenderSurface::GetDrawable()
   */
  virtual Ecore_X_Drawable GetDrawable();

public: // from Dali::RenderSurface

  /**
   * @copydoc Dali::RenderSurface::GetType()
   */
  virtual Dali::RenderSurface::SurfaceType GetType();

  /**
   * @copydoc Dali::RenderSurface::GetSurface()
   */
  virtual Any GetSurface();

public: // from Internal::Adaptor::RenderSurface

  /// @copydoc Dali::Internal::Adaptor::RenderSurface::InitializeEgl
  virtual void InitializeEgl( EglInterface& egl );

  /// @copydoc Dali::Internal::Adaptor::RenderSurface::CreateEglSurface
  virtual void CreateEglSurface( EglInterface& egl );

  /// @copydoc Dali::Internal::Adaptor::RenderSurface::DestroyEglSurface
  virtual void DestroyEglSurface( EglInterface& egl );

  /// @copydoc Dali::Internal::Adaptor::RenderSurface::ReplaceEGLSurface
  virtual bool ReplaceEGLSurface( EglInterface& egl );

  /// @copydoc Dali::RenderSurface::PreRender
  virtual bool PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction );

  /// @copydoc Dali::RenderSurface::PostRender
  virtual void PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, unsigned int timeDelta, SyncMode syncMode );

protected:

  /**
   * Create XWindow
   */
  virtual void CreateXRenderable();

  /**
   * @copydoc Dali::Internal::Adaptor::ECoreX::RenderSurface::UseExistingRenderable
   */
  virtual void UseExistingRenderable( unsigned int surfaceId );

private:
  /**
   * Create native buffer
   */
  native_buffer* CreateNativeBuffer();

private: // Data
  native_buffer_provider*              mProvider;
  native_buffer_pool*                  mPool;

  typedef std::vector<native_buffer*>  NativeBufferContainer;
  NativeBufferContainer                mBuffers;

  unsigned int                         mMaxBufferCount;
  bool                                 mIsAcquired;
}; // class NativeBufferRenderSurface

} // namespace ECoreX

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // __DALI_INTERNAL_NATIVE_BUFFER_RENDER_SURFACE_H__
