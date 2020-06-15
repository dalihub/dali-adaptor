#ifndef DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H
#define DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H

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
#include <memory>

#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <dali/integration-api/adaptor-framework/trigger-event-interface.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/offscreen-window.h>

namespace Dali
{
class Adaptor;
class Layer;
class NativeRenderSurface;
class TriggerEventInterface;

namespace Internal
{

/**
 * Implementation of the OffscreenWindow class.
 */
class OffscreenWindow : public Dali::Internal::Adaptor::SceneHolder,
                       public ConnectionTracker
{
public:
  typedef Dali::OffscreenWindow::WindowSize WindowSize;

  typedef Dali::OffscreenWindow::PostRenderSignalType PostRenderSignalType;


  /**
   * @brief Create a new OffscreenWindow
   *
   * @param[in] width The initial width of the OffscreenWindow
   * @param[in] height The initial height of the OffscreenWindow
   * @param[in] surface The native surface handle of your platform
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   */
  static OffscreenWindow* New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent );

  OffscreenWindow() = default;

public:

  /**
   * Destructor
   */
  ~OffscreenWindow() = default;

  /**
   * @copydoc Dali::OffscreenWindow::GetLayerCount
   */
  virtual uint32_t GetLayerCount() const = 0;

  /**
   * @copydoc Dali::OffscreenWindow::GetLayer
   */
  virtual Dali::Layer GetLayer( uint32_t depth ) const= 0;

  /**
   * @copydoc Dali::OffscreenWindow::GetSize
   */
  virtual WindowSize GetSize() const= 0;

  /**
   * @brief Gets the native handle.
   * @note When users call this function, it wraps the actual type used by the underlying system.
   * @return The native handle or an empty handle
   */
  virtual Dali::Any GetNativeHandle() const override = 0;

  virtual PostRenderSignalType& PostRenderSignal() = 0;

  /*
   * @brief Initialize the OffscreenWindow
   * @param[in] offscreenApplication The OffscreenApplication instance to be used to intialize the new OffscreenWindow
   * @param[in] isDefaultWindow Whether the OffscreenWindow is a default one or not
   */
  virtual void Initialize( OffscreenApplication* offscreenApplication, bool isDefaultWindow ) = 0;

private:
  /**
   * This function is called after drawing by dali.
   */
  virtual void OnPostRender() = 0;

  /**
   * @brief Get the native render surface
   * @return The render surface
   */
  virtual NativeRenderSurface* GetNativeRenderSurface() const = 0;

private:

  // Undefined
  OffscreenWindow( const OffscreenWindow& );
  OffscreenWindow& operator=( OffscreenWindow& );

  /**
   * This is for initialization of this OffscreenWindow in case it is created before adaptor is running.
   */
  void OnPreInitOffscreenApplication();

  /*
   * @brief Initialize the OffscreenWindow (for internal use)
   */
  void Initialize();
};

inline OffscreenWindow& GetImplementation( Dali::OffscreenWindow& offscreenWindow )
{
  DALI_ASSERT_ALWAYS( offscreenWindow && "Dali::OffscreenWindow handle is empty" );

  BaseObject& handle = offscreenWindow.GetBaseObject();

  return static_cast<OffscreenWindow&>( handle );
}

inline const OffscreenWindow& GetImplementation( const Dali::OffscreenWindow& offscreenWindow )
{
  DALI_ASSERT_ALWAYS( offscreenWindow && "Dali::OffscreenWindow handle is empty" );

  const BaseObject& handle = offscreenWindow.GetBaseObject();

  return static_cast<const OffscreenWindow&>( handle );
}

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_WINDOW_IMPL_H
