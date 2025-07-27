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

// EXTERNAL INCLUDES
#include <dali/public-api/common/intrusive-ptr.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/offscreen-window.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>

namespace Dali
{
class Adaptor;
class Layer;
class NativeRenderSurface;

namespace Internal
{
/**
 * Implementation of the OffscreenWindow class.
 */
class OffscreenWindow : public Dali::Internal::Adaptor::SceneHolder
{
public:
  using WindowSize = Dali::OffscreenWindow::WindowSize;

  /**
   * @brief Create a new OffscreenWindow
   *
   * @param[in] width The initial width of the OffscreenWindow
   * @param[in] height The initial height of the OffscreenWindow
   * @param[in] surface The native surface handle of your platform
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  static OffscreenWindow* New(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent);

  OffscreenWindow() = default;

public:
  /**
   * Destructor
   */
  ~OffscreenWindow();

  /**
   * @copydoc Dali::OffscreenWindow::GetLayerCount
   */
  uint32_t GetLayerCount() const;

  /**
   * @copydoc Dali::OffscreenWindow::GetLayer
   */
  Dali::Layer GetLayer(uint32_t depth) const;

  /**
   * @copydoc Dali::OffscreenWindow::GetSize
   */
  WindowSize GetSize() const;

  /**
   * @copydoc Dali::OffscreenWindow::GetNativeHandle
   */
  Any GetNativeHandle() const override;

  /**
   * @copydoc Dali::OffscreenWindow::SetPostRenderCallback
   */
  void SetPostRenderCallback(CallbackBase* callback);

  /**
   * @copydoc Dali::OffscreenWindow::SetFrameRenderedCallback
   */
  void SetFrameRenderedCallback(CallbackBase* callback);

  /*
   * @brief Initialize the OffscreenWindow
   * @param[in] isDefaultWindow Whether the OffscreenWindow is a default one or not
   */
  void Initialize(bool isDefaultWindow);

private:
  /**
   * This function is called after drawing by dali.
   */
  void OnPostRender();

  /**
   * @brief Get the native render surface
   * @return The render surface
   */
  NativeRenderSurface* GetNativeRenderSurface() const;

private:
  /**
   * Private constructor
   *
   * @param[in] width The initial width of the OffscreenWindow
   * @param[in] height The initial height of the OffscreenWindow
   * @param[in] surface The native surface handle
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  OffscreenWindow(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent);

  // Undefined
  OffscreenWindow(const OffscreenWindow&);
  OffscreenWindow& operator=(OffscreenWindow&);

private:
  TriggerEventFactory::TriggerEventPtr mRenderNotification;
  std::unique_ptr<CallbackBase>        mPostRenderCallback;
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

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H
