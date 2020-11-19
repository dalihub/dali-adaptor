#ifndef DALI_INTERNAL_WINDOWSYSTEM_ECOREX_WINDOW_BASE_ECORE_X_H
#define DALI_INTERNAL_WINDOWSYSTEM_ECOREX_WINDOW_BASE_ECORE_X_H

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

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-base.h>

// EXTERNAL HEADERS
#include <dali/internal/system/linux/dali-ecore.h>
#include <dali/internal/system/linux/dali-ecore-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * WindowBaseEcoreX class provides an WindowBase EcoreX implementation.
 */
class WindowBaseEcoreX : public WindowBase
{
public:

  /**
   * @brief Constructor
   */
  WindowBaseEcoreX( PositionSize positionSize, Any surface, bool isTransparent );

  /**
   * @brief Destructor
   */
  virtual ~WindowBaseEcoreX();

public:

  /**
   * @brief Called when the window property is changed.
   */
  Eina_Bool OnWindowPropertyChanged( void* data, int type, void* event );

  /**
   * @brief Called when the window receives a delete request
   */
  void OnDeleteRequest();

  /**
   * @brief Called when the window gains focus.
   */
  void OnFocusIn( void* data, int type, void* event );

  /**
   * @brief Called when the window loses focus.
   */
  void OnFocusOut( void* data, int type, void* event );

  /**
   * @brief Called when the window is damaged.
   */
  void OnWindowDamaged( void* data, int type, void* event );

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
   * @brief Called when a mouse wheel is received.
   */
  void OnMouseWheel( void* data, int type, void* event );

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
  void OnSelectionClear( void* data, int type, void* event );

  /**
   * @brief Called when the source window sends us about the selected content.
   */
  void OnSelectionNotify( void* data, int type, void* event );

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
  void SetType( Dali::WindowType type ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetNotificationLevel()
   */
  bool SetNotificationLevel( Dali::WindowNotificationLevel level ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNotificationLevel()
   */
  Dali::WindowNotificationLevel GetNotificationLevel() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetOpaqueState()
   */
  void SetOpaqueState( bool opaque ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetScreenOffMode()
   */
  bool SetScreenOffMode(WindowScreenOffMode screenOffMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenOffMode()
   */
  WindowScreenOffMode GetScreenOffMode() const override;

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
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenRotationAngle()
   */
  int GetScreenRotationAngle() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowRotationAngle()
   */
  void SetWindowRotationAngle( int degree ) override;

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
   * @brief Get the surface id if the surface parameter is not empty
   * @param surface Any containing a surface id, or can be empty
   * @return surface id, or zero if surface is empty
   */
  unsigned int GetSurfaceId( Any surface ) const;

  /**
   * @brief Create window
   */
  void CreateWindow( PositionSize positionSize, bool isTransparent );

protected:

  // Undefined
  WindowBaseEcoreX(const WindowBaseEcoreX&) = delete;

  // Undefined
  WindowBaseEcoreX& operator=(const WindowBaseEcoreX& rhs) = delete;

private:

  Dali::Vector< Ecore_Event_Handler* > mEcoreEventHandler;
  Ecore_X_Window                       mEcoreWindow;        ///< Native window handle
  bool                                 mOwnSurface:1;       ///< Whether we own the surface (responsible for deleting it)
  bool                                 mIsTransparent;      ///< Whether the window is transparent (32 bit or 24 bit)
  bool                                 mRotationAppSet:1;
};

} // namespace Adaptor

} // namespace internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_ECOREX_WINDOW_BASE_ECORE_X_H
