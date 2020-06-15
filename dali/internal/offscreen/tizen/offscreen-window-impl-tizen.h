#ifndef DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_TIZEN_H
#define DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_TIZEN_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
// #include <memory>

// #include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
// #include <dali/public-api/common/intrusive-ptr.h>
// #include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-window-impl.h>

namespace Dali
{
class Adaptor;
class Layer;
class NativeRenderSurface;
class TriggerEventInterface;

namespace Internal
{

class OffscreenWindowTizen : public Dali::Internal::OffscreenWindow
{
public:

  static OffscreenWindowTizen* New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent );

public:

  /**
   * @copydoc Dali::OffscreenWindow::GetLayerCount
   */
  uint32_t GetLayerCount() const;

  /**
   * @copydoc Dali::OffscreenWindow::GetLayer
   */
  Dali::Layer GetLayer( uint32_t depth ) const;

  /**
   * @copydoc Dali::OffscreenWindow::GetSize
   */
  WindowSize GetSize() const;

  /**
   * @brief Gets the native handle.
   * @note When users call this function, it wraps the actual type used by the underlying system.
   * @return The native handle or an empty handle
   */
  Dali::Any GetNativeHandle() const override;

  /*
   * @brief Initialize the OffscreenWindow
   * @param[in] offscreenApplication The OffscreenApplication instance to be used to intialize the new window
   * @param[in] isDefaultWindow Whether the OffscreenWindow is a default one or not
   */
  void Initialize( OffscreenApplication* offscreenApplication, bool isDefaultWindow );

  OffscreenWindow::PostRenderSignalType& PostRenderSignal();

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
   * @param[in] surface The tbm surface queue handle
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  OffscreenWindowTizen( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent );

  /**
   * Destructor
   */
  virtual ~OffscreenWindowTizen();

  // Undefined
  OffscreenWindowTizen( const OffscreenWindowTizen& );
  OffscreenWindowTizen& operator=( OffscreenWindowTizen& );

  /**
   * This is for initialization of this OffscreenWindow in case it is created before adaptor is running.
   */
  void OnPreInitOffscreenApplication();

  /*
   * @brief Initialize the OffscreenWindow (for internal use)
   */
  void Initialize();

private:

  Dali::Adaptor*                            mAdaptor;
  std::unique_ptr< TriggerEventInterface >  mRenderNotification;
  PostRenderSignalType                      mPostRenderSignal;
  bool                                      mIsTranslucent;
};

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_TIZEN_H
