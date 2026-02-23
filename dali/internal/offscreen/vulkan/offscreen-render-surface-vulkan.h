#ifndef DALI_INTERNAL_OFFSCREEN_VULKAN_OFFSCREEN_RENDER_SURFACE_VULKAN_H
#define DALI_INTERNAL_OFFSCREEN_VULKAN_OFFSCREEN_RENDER_SURFACE_VULKAN_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/offscreen/common/offscreen-render-surface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Vulkan implementation of offscreen render surface
 */
class DALI_ADAPTOR_API OffscreenRenderSurfaceVulkan : public OffscreenRenderSurface
{
public:
  /**
   * @brief Create an offscreen surface to render to.
   */
  OffscreenRenderSurfaceVulkan();

  /**
   * @brief Destructor
   */
  virtual ~OffscreenRenderSurfaceVulkan();

public: // from Dali::Internal::Adaptor::OffscreenRenderSurface
  /**
   * @copydoc Dali::Internal::Adaptor::OffscreenRenderSurface::OnAdaptorSet()
   */
  void OnAdaptorSet(Dali::RenderTask renderTask) override;

  /**
   * @copydoc Dali::Internal::Adaptor::OffscreenRenderSurface::SetNativeImage()
   */
  void SetNativeImage(Dali::NativeImageInterfacePtr nativeImage) override;

  /**
   * @copydoc Dali::Internal::Adaptor::OffscreenRenderSurface::GetNativeImage()
   */
  Dali::NativeImageInterfacePtr GetNativeImage() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::OffscreenRenderSurface::AddPostRenderSyncCallback()
   */
  void AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback) override;

  /**
   * @copydoc Dali::Internal::Adaptor::OffscreenRenderSurface::AddPostRenderAsyncCallback()
   */
  void AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback) override;

public: // from Dali::Integration::RenderSurfaceInterface
  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetPositionSize()
   */
  PositionSize GetPositionSize() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetDpi()
   */
  void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetSurfaceOrientation()
   */
  int GetSurfaceOrientation() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetScreenOrientation()
   */
  int GetScreenOrientation() const override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::InitializeGraphics()
   */
  void InitializeGraphics() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::CreateSurface()
   */
  void CreateSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::DestroySurface()
   */
  void DestroySurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::ReplaceGraphicsSurface()
   */
  bool ReplaceGraphicsSurface() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::MoveResize()
   */
  void MoveResize(Dali::PositionSize positionSize) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::Resize()
   */
  void Resize(Uint16Pair size) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StartRender()
   */
  void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::PreRender()
   */
  bool PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::PostRender()
   */
  void PostRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StopRender()
   */
  void StopRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::SetThreadSynchronization
   */
  void SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization) override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::ReleaseLock()
   */
  void ReleaseLock() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetSurfaceType()
   */
  Dali::Integration::RenderSurfaceInterface::Type GetSurfaceType() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::MakeContextCurrent()
   */
  void MakeContextCurrent() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetDepthBufferRequired()
   */
  Integration::DepthBufferAvailable GetDepthBufferRequired() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::GetStencilBufferRequired()
   */
  Integration::StencilBufferAvailable GetStencilBufferRequired() override;

protected:
  // Undefined
  OffscreenRenderSurfaceVulkan(const OffscreenRenderSurfaceVulkan&) = delete;

  // Undefined
  OffscreenRenderSurfaceVulkan& operator=(const OffscreenRenderSurfaceVulkan& rhs) = delete;

private: // Data
  NativeImageInterfacePtr mNativeImage{};

}; // class OffscreenRenderSurfaceVulkan

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_VULKAN_OFFSCREEN_RENDER_SURFACE_VULKAN_H
