#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/point.h>
#include <dali/public-api/events/wheel-event.h>
#include <string>
#include <vector>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/public-api/adaptor-framework/style-change.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/window-system/common/damage-observer.h>
#include <dali/internal/window-system/common/rotation-event.h>
#include <dali/internal/graphics/gles/egl-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * WindowBase interface
 */
class WindowBase
{
public:

  /**
   * @brief Struct used to retrieve accessibility information
   */
  struct AccessibilityInfo
  {
    int gestureValue;
    int startX;
    int startY;
    int endX;
    int endY;
    int state;
    int eventTime;
  };

  // Window
  typedef Signal< void ( bool ) > IconifySignalType;
  typedef Signal< void ( bool ) > FocusSignalType;
  typedef Signal< void ( ) > OutputSignalType;
  typedef Signal< void ( ) > DeleteSignalType;
  typedef Signal< void ( const DamageArea& ) > DamageSignalType;
  typedef Signal< void ( const RotationEvent& ) > RotationSignalType;
  typedef Signal< void ( DevelWindow::EffectState, DevelWindow::EffectType ) > TransitionEffectEventSignalType;

  // Input events
  typedef Signal< void ( Integration::Point&, uint32_t ) > TouchEventSignalType;
  typedef Signal< void ( WheelEvent& ) > WheelEventSignalType;
  typedef Signal< void( Integration::KeyEvent& ) > KeyEventSignalType;

  // Clipboard
  typedef Signal< void ( void* ) > SelectionSignalType;

  // Accessibility
  typedef Signal< void ( StyleChange::Type ) > StyleSignalType;
  typedef Signal< void ( const AccessibilityInfo& ) > AccessibilitySignalType;

  /**
   * @brief Default constructor
   */
  WindowBase();

  /**
   * @brief Destructor
   */
  virtual ~WindowBase();

public:

  /**
   * @brief Get the native window handle
   * @return The native window handle
   */
  virtual Any GetNativeWindow() = 0;

  /**
   * @brief Get the native window id
   * @return The native window id
   */
  virtual int GetNativeWindowId() = 0;

  /**
   * @brief Create the egl window
   */
  virtual EGLNativeWindowType CreateEglWindow( int width, int height ) = 0;

  /**
   * @brief Destroy the egl window
   */
  virtual void DestroyEglWindow() = 0;

  /**
   * @brief Set the egl window rotation
   */
  virtual void SetEglWindowRotation( int angle ) = 0;

  /**
   * @brief Set the egl window buffer transform
   */
  virtual void SetEglWindowBufferTransform( int angle ) = 0;

  /**
   * @brief Set the egl window transform
   */
  virtual void SetEglWindowTransform( int angle ) = 0;

  /**
   * @brief Resize the egl window
   */
  virtual void ResizeEglWindow( Dali::PositionSize positionSize ) = 0;

  /**
   * @brief Returns whether the egl window support rotation or not
   */
  virtual bool IsEglWindowRotationSupported() = 0;

  /**
   * @brief Move the window
   */
  virtual void Move( Dali::PositionSize positionSize ) = 0;

  /**
   * @brief Resize the window
   */
  virtual void Resize( Dali::PositionSize positionSize ) = 0;

  /**
   * @brief Move and resize the window
   */
  virtual void MoveResize( Dali::PositionSize positionSize ) = 0;

  /**
   * @copydoc Dali::Window::SetClass()
   */
  virtual void SetClass( const std::string& name, const std::string& className ) = 0;

  /**
   * @copydoc Dali::Window::Raise()
   */
  virtual void Raise() = 0;

  /**
   * @copydoc Dali::Window::Lower()
   */
  virtual void Lower() = 0;

  /**
   * @copydoc Dali::Window::Activate()
   */
  virtual void Activate() = 0;

  /**
   * @copydoc Dali::Window::SetAvailableOrientations()
   */
  virtual void SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations ) = 0;

  /**
   * @copydoc Dali::Window::SetPreferredOrientation()
   */
  virtual void SetPreferredOrientation( Dali::Window::WindowOrientation orientation ) = 0;

  /**
   * @copydoc Dali::Window::SetAcceptFocus()
   */
  virtual void SetAcceptFocus( bool accept ) = 0;

  /**
   * @copydoc Dali::Window::Show()
   */
  virtual void Show() = 0;

  /**
   * @copydoc Dali::Window::Hide()
   */
  virtual void Hide() = 0;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHintCount()
   */
  virtual unsigned int GetSupportedAuxiliaryHintCount() const = 0;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHint()
   */
  virtual std::string GetSupportedAuxiliaryHint( unsigned int index ) const = 0;

  /**
   * @copydoc Dali::Window::AddAuxiliaryHint()
   */
  virtual unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value ) = 0;

  /**
   * @copydoc Dali::Window::RemoveAuxiliaryHint()
   */
  virtual bool RemoveAuxiliaryHint( unsigned int id ) = 0;

  /**
   * @copydoc Dali::Window::SetAuxiliaryHintValue()
   */
  virtual bool SetAuxiliaryHintValue( unsigned int id, const std::string& value ) = 0;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintValue()
   */
  virtual std::string GetAuxiliaryHintValue( unsigned int id ) const = 0;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintId()
   */
  virtual unsigned int GetAuxiliaryHintId( const std::string& hint ) const = 0;

  /**
   * @copydoc Dali::Window::SetInputRegion()
   */
  virtual void SetInputRegion( const Rect< int >& inputRegion ) = 0;

  /**
   * @copydoc Dali::Window::SetType()
   */
  virtual void SetType( Dali::Window::Type type ) = 0;

  /**
   * @copydoc Dali::Window::SetNotificationLevel()
   */
  virtual bool SetNotificationLevel( Dali::Window::NotificationLevel::Type level ) = 0;

  /**
   * @copydoc Dali::Window::GetNotificationLevel()
   */
  virtual Dali::Window::NotificationLevel::Type GetNotificationLevel() const = 0;

  /**
   * @copydoc Dali::Window::SetOpaqueState()
   */
  virtual void SetOpaqueState( bool opaque ) = 0;

  /**
   * @copydoc Dali::Window::SetScreenOffMode()
   */
  virtual bool SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode) = 0;

  /**
   * @copydoc Dali::Window::GetScreenOffMode()
   */
  virtual Dali::Window::ScreenOffMode::Type GetScreenOffMode() const = 0;

  /**
   * @copydoc Dali::Window::SetBrightness()
   */
  virtual bool SetBrightness( int brightness ) = 0;

  /**
   * @copydoc Dali::Window::GetBrightness()
   */
  virtual int GetBrightness() const = 0;

  /**
   * @copydoc Dali::KeyGrab::GrabKey()
   */
  virtual bool GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode ) = 0;

  /**
   * @copydoc Dali::KeyGrab::UngrabKey()
   */
  virtual bool UngrabKey( Dali::KEY key ) = 0;

  /**
   * @copydoc Dali::KeyGrab::GrabKeyList()
   */
  virtual bool GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result ) = 0;

  /**
   * @copydoc Dali::KeyGrab::UngrabKeyList()
   */
  virtual bool UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result ) = 0;

  /**
   * @brief Get DPI
   * @param[out] dpiHorizontal set to the horizontal dpi
   * @param[out] dpiVertical set to the vertical dpi
   */
  virtual void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) = 0;

  /**
   * @brief Get the screen rotation angle of the window
   */
  virtual int GetScreenRotationAngle() = 0;

  /**
   * @brief Set the rotation angle of the window
   */
  virtual void SetWindowRotationAngle( int degree ) = 0;

  /**
   * @brief Inform the window rotation is completed
   */
  virtual void WindowRotationCompleted( int degree, int width, int height ) = 0;

  /**
   * @copydoc Dali::Window::SetTransparency()
   */
  virtual void SetTransparency( bool transparent ) = 0;

  /**
   * @copydoc Dali::Window::SetParent()
   */
  virtual void SetParent( WindowBase* parentWinBase ) = 0;

  // Signals

  /**
   * @brief This signal is emitted when the window becomes iconified or deiconified.
   */
   IconifySignalType& IconifyChangedSignal();

  /**
   * @brief This signal is emitted when the window focus is changed.
   */
  FocusSignalType& FocusChangedSignal();

  /**
   * @brief This signal is emitted when the output is transformed.
   */
  OutputSignalType& OutputTransformedSignal();

  /**
   * @brief This signal is emitted when the window receives a delete request.
   */
  DeleteSignalType& DeleteRequestSignal();

  /**
   * @brief This signal is emitted when the window is damaged.
   */
  DamageSignalType& WindowDamagedSignal();

  /**
   * @brief This signal is emitted when a rotation event is recevied.
   */
  RotationSignalType& RotationSignal();

  /**
   * @brief This signal is emitted when a touch event is received.
   */
  TouchEventSignalType& TouchEventSignal();

  /**
   * @brief This signal is emitted when a mouse wheel is received.
   */
  WheelEventSignalType& WheelEventSignal();

  /**
   * @brief This signal is emitted when a key event is received.
   */
  KeyEventSignalType& KeyEventSignal();

  /**
   * @brief This signal is emitted when the source window notifies us the content in clipboard is selected.
   */
  SelectionSignalType& SelectionDataSendSignal();

  /**
   * @brief This signal is emitted when the source window sends us about the selected content.
   */
  SelectionSignalType& SelectionDataReceivedSignal();

  /**
   * @brief This signal is emitted when the style is changed.
   */
  StyleSignalType& StyleChangedSignal();

  /**
   * @brief This signal is emitted when an accessibility event is received.
   */
  AccessibilitySignalType& AccessibilitySignal();

  /**
   * @brief This signal is emitted when window's transition animation is started or ended.
   */
  TransitionEffectEventSignalType& TransitionEffectEventSignal();

protected:

  // Undefined
  WindowBase(const WindowBase&) = delete;

  // Undefined
  WindowBase& operator=(const WindowBase& rhs) = delete;

protected:

  IconifySignalType                    mIconifyChangedSignal;
  FocusSignalType                      mFocusChangedSignal;
  OutputSignalType                     mOutputTransformedSignal;
  DeleteSignalType                     mDeleteRequestSignal;
  DamageSignalType                     mWindowDamagedSignal;
  RotationSignalType                   mRotationSignal;
  TouchEventSignalType                 mTouchEventSignal;
  WheelEventSignalType                 mWheelEventSignal;
  KeyEventSignalType                   mKeyEventSignal;
  SelectionSignalType                  mSelectionDataSendSignal;
  SelectionSignalType                  mSelectionDataReceivedSignal;
  StyleSignalType                      mStyleChangedSignal;
  AccessibilitySignalType              mAccessibilitySignal;
  TransitionEffectEventSignalType      mTransitionEffectEventSignal;
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H
