#ifndef DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_ANDROID_H
#define DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_ANDROID_H

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

class OffscreenWindowAndroid : public Dali::Internal::OffscreenWindow
{
public:

  static OffscreenWindowAndroid* New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent );

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
   * @copydoc Dali::OffscreenWindow::GetNativeHandle
   */
  Dali::Any GetNativeHandle() const override;

  /*
   * @brief Initialize the OffscreenWindow
   * @param[in] isDefaultWindow Whether the OffscreenWindow is a default one or not
   */
  void Initialize( bool isDefaultWindow );

public:  // Signals

  /**
   * @copydoc Dali::OffscreenWindow::PostRenderSignal
   */
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
  OffscreenWindowAndroid( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent );

  /**
   * Destructor
   */
  virtual ~OffscreenWindowAndroid();

  // Undefined
  OffscreenWindowAndroid( const OffscreenWindowAndroid& );
  OffscreenWindowAndroid& operator=( OffscreenWindowAndroid& );

  /*
   * @brief Initialize the OffscreenWindow (for internal use)
   */
  void Initialize();

private:

  PostRenderSignalType                      mPostRenderSignal;
  bool                                      mIsTranslucent;
};

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_ANDROID_H
