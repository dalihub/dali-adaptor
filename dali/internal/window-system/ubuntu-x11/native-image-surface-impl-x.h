#ifndef DALI_INTERNAL_WINDOWSYSTEM_X_NATIVE_IMAGE_SURFACE_IMPL_X_H
#define DALI_INTERNAL_WINDOWSYSTEM_X_NATIVE_IMAGE_SURFACE_IMPL_X_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>
#include <cstdint>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>
#include <dali/internal/imaging/ubuntu-x11/native-image-queue-producer-x.h>
#include <dali/internal/window-system/common/native-image-surface-impl.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * A native image surface backed by a queue this class drives itself.
 *
 * Where the queue can be handed to EGL as a native window, the graphics driver
 * dequeues a buffer, lets the client draw into it and enqueues it again inside
 * eglSwapBuffers(). No EGL here will do that, so this class takes the driver's
 * part explicitly:
 *
 *   PreRender()  dequeues a buffer from the queue and makes the GL surface current
 *   CanRender()  reports whether a buffer was available
 *   PostRender() reads the frame back into that buffer and enqueues it
 *
 * The client draws into a pbuffer, which is a default framebuffer, so its GL
 * code sees framebuffer zero just as it does on a device. The readback is
 * flipped on the CPU because glReadPixels() returns rows bottom-up while the
 * buffer is consumed top-down.
 */
class NativeImageSurfaceX : public Dali::Internal::Adaptor::NativeImageSurface
{
public:
  /**
   * @param [in] queue the NativeImageQueue pointer
   */
  NativeImageSurfaceX(Dali::NativeImageQueuePtr queue);

  /**
   * @brief Destructor
   */
  ~NativeImageSurfaceX();

public:
  /**
   * @copydoc Dali::NativeImageSurface::GetNativeRenderable()
   */
  Any GetNativeRenderable() override;

  /**
   * @copydoc Dali::NativeImageSurface::InitializeGraphics()
   */
  void InitializeGraphics() override;

  /**
   * @copydoc Dali::NativeImageSurface::InitializeGraphics()
   */
  void TerminateGraphics() override;

  /**
   * @copydoc Dali::NativeImageSurface::PreRender()
   */
  void PreRender() override;

  /**
   * @copydoc Dali::NativeImageSurface::PostRender()
   */
  void PostRender() override;

  /**
   * @copydoc Dali::NativeImageSurface::CanRender()
   */
  bool CanRender() override;

  /**
   * @copydoc Dali::NativeImageSurface::SetGraphicsConfig()
   */
  bool SetGraphicsConfig(bool depth, bool stencil, int msaa, int version) override;

private:
  NativeImageSurfaceX(const NativeImageSurfaceX&)            = delete;
  NativeImageSurfaceX& operator=(const NativeImageSurfaceX&) = delete;

  /**
   * @brief Chooses an EGL config matching the requested graphics configuration.
   * @return True if a config was found
   */
  bool ChooseConfig();

  /**
   * @brief Creates the pbuffer the client renders into, sized to @p width x @p height.
   *
   * A pbuffer is used rather than a framebuffer object so that the client's GL
   * code draws into a real default framebuffer.
   *
   * @return True if the pbuffer could be created
   */
  bool CreatePbuffer(uint32_t width, uint32_t height);

  /**
   * @brief Releases the pbuffer, if any.
   */
  void DestroyPbuffer();

  /**
   * @brief Returns the buffer currently dequeued, if it was never enqueued.
   *
   * The render frame callback may report that it produced nothing, in which case
   * PostRender() is not called and the buffer would otherwise stay checked out.
   */
  void CancelPendingBuffer();

private:
  Dali::NativeImageQueuePtr                           mQueue;
  Dali::Internal::Adaptor::NativeImageQueueProducerX* mProducer{nullptr};

  void* mXDisplay{nullptr}; ///< Opened here so the surface has its own EGL display
  void* mEglDisplay{nullptr};
  void* mEglConfig{nullptr};
  void* mEglContext{nullptr};
  void* mEglSurface{nullptr};

  uint32_t mSurfaceWidth{0};
  uint32_t mSurfaceHeight{0};

  /// glReadPixels() cannot write with an arbitrary row stride, and the rows have
  /// to be reversed anyway, so the frame lands here first.
  std::vector<uint8_t> mReadbackBuffer;

  Dali::Internal::Adaptor::NativeImageQueueProducerX::Buffer mCurrentBuffer;

  bool mDepth{false};
  bool mStencil{false};
  int  mMSAA{0};
  int  mGlesVersion{30};
  bool mGraphicsInitialized{false};
};

} // namespace Adaptor
} // namespace Internal
} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_WINDOWSYSTEM_X_NATIVE_IMAGE_SURFACE_IMPL_X_H
