#ifndef DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H
#define DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H

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
#include <dali/devel-api/adaptor-framework/offscreen-window.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>

namespace Dali
{
class Layer;

namespace Internal
{

namespace Adaptor
{

class OffscreenRenderSurface;

/**
 * Implementation of the OffscreenWindow class.
 */
class OffscreenWindow : public Dali::Internal::Adaptor::SceneHolder
{
public:
  using WindowSize = Dali::OffscreenWindow::WindowSize;

  /**
   * @copydoc Dali::OffscreenWindow::New
   */
  static OffscreenWindow* New();

  OffscreenWindow() = default;

public:
  /**
   * Destructor
   */
  ~OffscreenWindow();

  /**
   * @copydoc Dali::OffscreenWindow::SetNativeImage
   */
  void SetNativeImage(NativeImageSourcePtr nativeImage);

  /**
   * @copydoc Dali::OffscreenWindow::GetSize
   */
  WindowSize GetSize() const;

  /**
   * @copydoc Dali::OffscreenWindow::AddPostRenderSyncCallback
   */
  void AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback);

  /**
   * @copydoc Dali::OffscreenWindow::AddPostRenderAsyncCallback
   */
  void AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback);

public: // Dali::Internal::Adaptor::SceneHolder
  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnAdaptorSet
   */
  void OnAdaptorSet(Dali::Adaptor& adaptor) override;

  /**
   * @copydoc Dali::OffscreenWindow::GetNativeHandle
   */
  Any GetNativeHandle() const override
  {
    return Any();
  }

  /**
   * @brief Initialize the OffscreenWindow
   */
  void Initialize();

private:
  /**
   * Private constructor
   *
   * @param[in] surface The native surface handle
   */
  OffscreenWindow(NativeImageSourcePtr nativeImage);

  // Undefined
  OffscreenWindow(const OffscreenWindow&);
  OffscreenWindow& operator=(OffscreenWindow&);

  /**
   * @brief PostRenderSyncCallback
   */
  void PostRenderSyncCallback();

  /**
   * @brief PostRenderAsyncCallback
   * @param fenceFd File descriptor for the fence to verify GPU rendering completion
   */
  void PostRenderAsyncCallback(int32_t fenceFd);

private:
  OffscreenRenderSurface*       mOffscreenSurface{nullptr};
  std::unique_ptr<CallbackBase> mPostRenderSyncCallback{};
  std::unique_ptr<CallbackBase> mPostRenderAsyncCallback{};

  uint32_t mWidth{0};
  uint32_t mHeight{0};
};

inline OffscreenWindow& GetImplementation(Dali::OffscreenWindow& offscreenWindow)
{
  DALI_ASSERT_ALWAYS(offscreenWindow && "Dali::OffscreenWindow handle is empty");

  BaseObject& handle = offscreenWindow.GetBaseObject();

  return static_cast<OffscreenWindow&>(handle);
}

inline const OffscreenWindow& GetImplementation(const Dali::OffscreenWindow& offscreenWindow)
{
  DALI_ASSERT_ALWAYS(offscreenWindow && "Dali::OffscreenWindow handle is empty");

  const BaseObject& handle = offscreenWindow.GetBaseObject();

  return static_cast<const OffscreenWindow&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H
