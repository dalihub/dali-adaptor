#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_WINDOW_BASE_ECORE_WL2_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_WINDOW_BASE_ECORE_WL2_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/window-base.h>

// EXTERNAL HEADERS
#include <Ecore.h>
#include <Ecore_Wl2.h>
#include <tizen-extension-client-protocol.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>

#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class WindowRenderSurface;
class WindowRenderSurfaceEcoreWl2;

/**
 * WindowBaseEcoreWl2 class provides an WindowBase Ecore-Wayland2 implementation.
 */
class WindowBaseEcoreWl2 : public WindowBase
{
public:

  /**
   * @brief Constructor
   */
  WindowBaseEcoreWl2( PositionSize positionSize, Any surface, bool isTransparent );

  /**
   * @brief Destructor
   */
  virtual ~WindowBaseEcoreWl2();

public:

  /**
   * @brief Called when the window iconify state is changed.
   */
  Eina_Bool OnIconifyStateChanged( void* data, int type, void* event );

  /**
   * @brief Called when the window gains focus.
   */
  Eina_Bool OnFocusIn( void* data, int type, void* event );

  /**
   * @brief Called when the window loses focus.
   */
  Eina_Bool OnFocusOut( void* data, int type, void* event );

  /**
   * @brief Called when the output is transformed.
   */
  Eina_Bool OnOutputTransform( void* data, int type, void* event );

  /**
   * @brief Called when the output transform should be ignored.
   */
  Eina_Bool OnIgnoreOutputTransform( void* data, int type, void* event );

  /**
   * @brief Called when a rotation event is recevied.
   */
  void OnRotation( void* data, int type, void* event );

  /**
   * @brief Called when a configure event is recevied.
   */
  void OnConfiguration( void* data, int type, void* event );

  /**
   * @brief Called when a touch down is received.
   */
  void OnMouseButtonDown( void* data, int type, void* event );

  /**
   * @brief Called when a touch up is received.
   */
  void OnMouseButtonUp( void* data, int type, void* event );

  /**
   * @brief Called when a touch motion is received.
   */
  void OnMouseButtonMove( void* data, int type, void* event );

  /**
   * @brief Called when a touch is canceled.
   */
  void OnMouseButtonCancel( void* data, int type, void* event );

  /**
   * @brief Called when a mouse wheel is received.
   */
  void OnMouseWheel( void* data, int type, void* event );

  /**
   * @brief Called when a detent rotation event is recevied.
   */
  void OnDetentRotation( void* data, int type, void* event );

  /**
   * @brief Called when a key down is received.
   */
  void OnKeyDown( void* data, int type, void* event );

  /**
   * @brief Called when a key up is received.
   */
  void OnKeyUp( void* data, int type, void* event );

  /**
   * @brief Called when the source window notifies us the content in clipboard is selected.
   */
  void OnDataSend( void* data, int type, void* event );

  /**
   * @brief Called when the source window sends us about the selected content.
   */
  void OnDataReceive( void* data, int type, void* event );

  /**
   * @brief Called when a font name is changed.
   */
  void OnFontNameChanged();

  /**
   * @brief Called when a font size is changed.
   */
  void OnFontSizeChanged();

  /**
   * @brief Called when a transition effect-start/end event is received.
   */
  void OnTransitionEffectEvent( DevelWindow::EffectState state, DevelWindow::EffectType type );

  /**
   * @brief Called when a keyboard repeat event is changed.
   */
  void OnKeyboardRepeatSettingsChanged();

  /**
   * @brief Called when a window redraw is requested.
   */
  void OnEcoreEventWindowRedrawRequest();

#ifdef DALI_ELDBUS_AVAILABLE
  /**
   * @brief Called when Ecore ElDBus accessibility event is received.
   */
  void OnEcoreElDBusAccessibilityNotification( void* context, const Eldbus_Message* message );

  /**
   * @brief Called when Ecore ElDBus quick panel event for accessibility is received.
   */
  void OnEcoreElDBusAccessibilityQuickpanelChanged( void* context, const Eldbus_Message* message );
#endif

  /**
   * @brief Called when a keymap is changed.
   */
  void KeymapChanged(void *data, int type, void *event);

  /**
   * @brief RegistryGlobalCallback
   */
  void RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version );

  /**
   * @brief RegistryGlobalCallbackRemove
   */
  void RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id );

  /**
   * @brief TizenPolicyNotificationChangeDone
   */
  void TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state );

  /**
   * @brief TizenPolicyScreenModeChangeDone
   */
  void TizenPolicyScreenModeChangeDone( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state );

  /**
   * @brief DisplayPolicyBrightnessChangeDone
   */
  void DisplayPolicyBrightnessChangeDone( void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state );


  /**
   * @brief Gets the key code by keyName.
   */
  void GetKeyCode( std::string keyName, int32_t& keyCode );

public:

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeWindow()
   */
  Any GetNativeWindow() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeWindowId()
   */
  int GetNativeWindowId() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeWindowResourceId()
   */
  std::string GetNativeWindowResourceId() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::CreateEglWindow()
   */
  EGLNativeWindowType CreateEglWindow( int width, int height ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::DestroyEglWindow()
   */
  void DestroyEglWindow() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetEglWindowRotation()
   */
  void SetEglWindowRotation( int angle ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetEglWindowBufferTransform()
   */
  void SetEglWindowBufferTransform( int angle ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetEglWindowTransform()
   */
  void SetEglWindowTransform( int angle ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::ResizeEglWindow()
   */
  void ResizeEglWindow( PositionSize positionSize ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsEglWindowRotationSupported()
   */
  bool IsEglWindowRotationSupported() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Move()
   */
  void Move( PositionSize positionSize ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Resize()
   */
  void Resize( PositionSize positionSize ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::MoveResize()
   */
  void MoveResize( PositionSize positionSize ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetClass()
   */
  void SetClass( const std::string& name, const std::string& className ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Raise()
   */
  void Raise() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Lower()
   */
  void Lower() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Activate()
   */
  void Activate() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAvailableAnlges()
   */
  void SetAvailableAnlges( const std::vector< int >& angles ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetPreferredAngle()
   */
  void SetPreferredAngle( int angle ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAcceptFocus()
   */
  void SetAcceptFocus( bool accept ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Show()
   */
  void Show() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Hide()
   */
  void Hide() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetSupportedAuxiliaryHintCount()
   */
  unsigned int GetSupportedAuxiliaryHintCount() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetSupportedAuxiliaryHint()
   */
  std::string GetSupportedAuxiliaryHint( unsigned int index ) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::AddAuxiliaryHint()
   */
  unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RemoveAuxiliaryHint()
   */
  bool RemoveAuxiliaryHint( unsigned int id ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAuxiliaryHintValue()
   */
  bool SetAuxiliaryHintValue( unsigned int id, const std::string& value ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetAuxiliaryHintValue()
   */
  std::string GetAuxiliaryHintValue( unsigned int id ) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetAuxiliaryHintId()
   */
  unsigned int GetAuxiliaryHintId( const std::string& hint ) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetInputRegion()
   */
  void SetInputRegion( const Rect< int >& inputRegion ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetType()
   */
  void SetType( Dali::Window::Type type ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetNotificationLevel()
   */
  bool SetNotificationLevel( Dali::Window::NotificationLevel::Type level ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNotificationLevel()
   */
  Dali::Window::NotificationLevel::Type GetNotificationLevel() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetOpaqueState()
   */
  void SetOpaqueState( bool opaque ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetScreenOffMode()
   */
  bool SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenOffMode()
   */
  Dali::Window::ScreenOffMode::Type GetScreenOffMode() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBrightness()
   */
  bool SetBrightness( int brightness ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetBrightness()
   */
  int GetBrightness() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GrabKey()
   */
  bool GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::UngrabKey()
   */
  bool UngrabKey( Dali::KEY key ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GrabKeyList()
   */
  bool GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::UngrabKeyList()
   */
  bool UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetDpi()
   */
  void GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical ) override;

  /**
   * @brief Return the orientation of the surface.
   * @return The orientation
   */
  virtual int GetOrientation() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenRotationAngle()
   */
  int GetScreenRotationAngle() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowRotationAngle()
   */
  void SetWindowRotationAngle( int degree ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetWindowRotationAngle()
   */
  virtual int GetWindowRotationAngle() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::WindowRotationCompleted()
   */
  void WindowRotationCompleted( int degree, int width, int height ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetTransparency()
   */
  void SetTransparency( bool transparent ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetParent()
   */
  void SetParent( WindowBase* parentWinBase ) override;

  /**
   * @copydoc  Dali::Internal::Adaptor::WindowBase::CreateFrameRenderedSyncFence()
   */
  int CreateFrameRenderedSyncFence() override;

  /**
   * @copydoc  Dali::Internal::Adaptor::WindowBase::CreateFramePresentedSyncFence()
   */
  int CreateFramePresentedSyncFence() override;

private:

  /**
   * Second stage initialization
   */
  void Initialize( PositionSize positionSize, Any surface, bool isTransparent );

  /**
   * Initialize Ecore ElDBus
   */
  void InitializeEcoreElDBus();

  /**
   * @brief Create window
   */
  void CreateWindow( PositionSize positionSize );

protected:

  // Undefined
  WindowBaseEcoreWl2(const WindowBaseEcoreWl2&) = delete;

  // Undefined
  WindowBaseEcoreWl2& operator=(const WindowBaseEcoreWl2& rhs) = delete;

private:

  typedef std::vector< std::pair< std::string, std::string > > AuxiliaryHints;

  Dali::Vector< Ecore_Event_Handler* > mEcoreEventHandler;

  Ecore_Wl2_Window*                    mEcoreWindow;
  wl_surface*                          mWlSurface;
  wl_egl_window*                       mEglWindow;
  wl_display*                          mDisplay;
  wl_event_queue*                      mEventQueue;
  tizen_policy*                        mTizenPolicy;
  tizen_display_policy*                mTizenDisplayPolicy;
  xkb_keymap*                          mKeyMap;

  std::vector< std::string >           mSupportedAuxiliaryHints;
  AuxiliaryHints                       mAuxiliaryHints;

  int                                  mNotificationLevel;
  uint32_t                             mNotificationChangeState;
  bool                                 mNotificationLevelChangeDone;

  int                                  mScreenOffMode;
  uint32_t                             mScreenOffModeChangeState;
  bool                                 mScreenOffModeChangeDone;

  int                                  mBrightness;
  uint32_t                             mBrightnessChangeState;
  bool                                 mBrightnessChangeDone;

  bool                                 mVisible:1;
  Dali::PositionSize                   mWindowPositionSize;

  bool                                 mOwnSurface;

  volatile uint32_t                    mMoveResizeSerial;
  uint32_t                             mLastSubmittedMoveResizeSerial;

  int                                  mWindowRotationAngle;
  int                                  mScreenRotationAngle;
  int                                  mSupportedPreProtation;
#ifdef DALI_ELDBUS_AVAILABLE
  Eldbus_Connection*                   mSystemConnection;
#endif // DALI_ELDBUS_AVAILABLE
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_WINDOW_BASE_ECORE_WL2_H
