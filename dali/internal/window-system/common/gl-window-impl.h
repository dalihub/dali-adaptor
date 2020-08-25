#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_IMPL_H

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
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/adaptor-framework/window.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/common/event-handler.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class WindowBase;

class GlWindow;
using GlWindowPtr = IntrusivePtr< GlWindow >;
using EventHandlerPtr = IntrusivePtr< EventHandler >;

/**
 * Window provides a surface to render onto with orientation.
 */
class GlWindow : public BaseObject, public EventHandler::Observer, public DamageObserver, public ConnectionTracker
{
public:

  using KeyEventSignalType = Dali::GlWindow::KeyEventSignalType;
  using TouchSignalType = Dali::GlWindow::TouchSignalType;
  using FocusChangeSignalType = Dali::GlWindow::FocusChangeSignalType;
  using ResizedSignalType = Dali::GlWindow::ResizedSignalType;
  using VisibilityChangedSignalType = Dali::GlWindow::VisibilityChangedSignalType;
  using SignalType = Signal< void () >;

  /**
   * @brief Create a new GlWindow. This should only be called once by the Application class
   * @param[in] surface The surface used to render on.
   * @param[in] positionSize The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return A newly allocated Window
   */
  static GlWindow* New( const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent = false );

  /**
   * @copydoc Dali::GlWindow::SetEglConfig()
   */
  void SetEglConfig( bool depth, bool stencil, int msaa, Dali::GlWindow::GlesVersion version );

  /**
   * @copydoc Dali::GlWindow::Raise()
   */
  void Raise();

  /**
   * @copydoc Dali::GlWindow::Lower()
   */
  void Lower();

  /**
   * @copydoc Dali::GlWindow::Activate()
   */
  void Activate();

  /**
   * @copydoc Dali::GlWindow::Show()
   */
  void Show();

  /**
   * @copydoc Dali::GlWindow::Hide()
   */
  void Hide();

  /**
   * @copydoc Dali::GlWindow::GetSupportedAuxiliaryHintCount()
   */
  unsigned int GetSupportedAuxiliaryHintCount() const;

  /**
   * @copydoc Dali::GlWindow::GetSupportedAuxiliaryHint()
   */
  std::string GetSupportedAuxiliaryHint( unsigned int index ) const;

  /**
   * @copydoc Dali::GlWindow::AddAuxiliaryHint()
   */
  unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value );

  /**
   * @copydoc Dali::GlWindow::RemoveAuxiliaryHint()
   */
  bool RemoveAuxiliaryHint( unsigned int id );

  /**
   * @copydoc Dali::GlWindow::SetAuxiliaryHintValue()
   */
  bool SetAuxiliaryHintValue( unsigned int id, const std::string& value );

  /**
   * @copydoc Dali::GlWindow::GetAuxiliaryHintValue()
   */
  std::string GetAuxiliaryHintValue( unsigned int id ) const;

  /**
   * @copydoc Dali::GlWindow::GetAuxiliaryHintId()
   */
  unsigned int GetAuxiliaryHintId( const std::string& hint ) const;

  /**
   * @copydoc Dali::GlWindow::SetInputRegion()
   */
  void SetInputRegion( const Rect< int >& inputRegion );

  /**
   * @copydoc Dali::GlWindow::SetOpaqueState()
   */
  void SetOpaqueState( bool opaque );

  /**
   * @copydoc Dali::GlWindow::IsOpaqueState()
   */
  bool IsOpaqueState() const;

  /**
   * @copydoc Dali::GlWindow::SetPositionSize()
   */
  void SetPositionSize( PositionSize positionSize );

  /**
   * @copydoc Dali::GlWindow::GetPositionSize()
   */
  PositionSize GetPositionSize() const;

  /**
   * @copydoc Dali::GlWindow::GetCurrentOrientation() const
   */
  Dali::GlWindow::GlWindowOrientation GetCurrentOrientation() const;

  /**
   * @copydoc Dali::GlWindow::SetAvailableOrientations()
   */
  void SetAvailableOrientations( const Dali::Vector< Dali::GlWindow::GlWindowOrientation >& orientations );

  /**
   * @copydoc Dali::GlWindow::SetPreferredOrientation()
   */
  void SetPreferredOrientation( Dali::GlWindow::GlWindowOrientation orientation );

  /**
   * @copydoc Dali::GlWindow::RegisterGlCallback()
   */
  void RegisterGlCallback( GlInitialize glInit, GlRenderFrame glRenderFrame, GlTerminate glTerminate );

  /**
   * @copydoc Dali::GlWindow::RenderOnce()
   */
  void RenderOnce();

public: // For implementation
  /**
   * @brief Sets child window with Dali::Window
   *
   * Currently the child window is default window.
   */
  void SetChild( Dali::Window& child );

private:

  /**
   * Private constructor.
   * @sa Window::New()
   */
  GlWindow();

  /**
   * Destructor
   */
  virtual ~GlWindow();

  /**
   * Second stage initialization
   */
  void Initialize( const PositionSize& positionSize, const std::string& name, const std::string& className );

  /**
   * Called when the window becomes iconified or deiconified.
   */
  void OnIconifyChanged( bool iconified );

  /**
   * Called when the window focus is changed.
   */
  void OnFocusChanged( bool focusIn );

  /**
   * Called when the output is transformed.
   */
  void OnOutputTransformed();

  /**
   * Called when the window receives a delete request.
   */
  void OnDeleteRequest();

  /**
   * @brief Set available rotation angle to window base.
   */
  void SetAvailableAnlges( const std::vector< int >& angles );

  /**
   * @brief Check available window rotation angle for Available angle.
   */
  bool IsOrientationAvailable( Dali::GlWindow::GlWindowOrientation orientation ) const;

  /**
   * @brief Convert from window orientation to angle using orientation mode value.
   */
  int ConvertToAngle( Dali::GlWindow::GlWindowOrientation orientation );

  /**
   * @brief Convert from angle to window orientation using orientation mode value.
   */
  Dali::GlWindow::GlWindowOrientation ConvertToOrientation( int angle ) const;

  /**
   * @brief Run Ui GL callback function.
   */
  bool RunCallback();

  /**
   * @brief Initialize and create EGL resource
   */
  void InitializeGraphics();

  /**
   * @brief Sets event handler for window's events.
   */
  void SetEventHandler() ;

  /**
   * @brief Gets the native window handle
   */
  Dali::Any GetNativeHandle() const;

  /**
   * @brief Gets the native window handle ID
   */
  int32_t GetNativeId() const;

  /**
   * @brief calculate touch position for rotation.
   */
  void RecalculateTouchPosition( Integration::Point& point ) ;

  /**
   * @brief Sets window and class name.
   */
  void SetClass( const std::string name, const std::string className );

private:

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnTouchPoint
   */
  void OnTouchPoint( Dali::Integration::Point& point, int timeStamp ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnWheelEvent
   */
  void OnWheelEvent( Dali::Integration::WheelEvent& wheelEvent ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnKeyEvent
   */
  void OnKeyEvent( Dali::Integration::KeyEvent& keyEvent ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnRotation
   */
  void OnRotation( const RotationEvent& rotation ) override;

private: // From Dali::Internal::Adaptor::DamageObserver

  /**
   * @copydoc Dali::Internal::Adaptor::DamageObserver::OnDamaged()
   */
  void OnDamaged( const DamageArea& area );

public: // Signals

  /**
   * @copydoc Dali::GlWindow::FocusChangeSignal()
   */
  FocusChangeSignalType& FocusChangeSignal() { return mFocusChangeSignal; }

  /**
   * @copydoc Dali::GlWindow::ResizedSignal()
   */
  ResizedSignalType& ResizedSignal() { return mResizedSignal; }

  /**
   * @copydoc Dali::GlWindow::KeyEventSignal()
   */
  KeyEventSignalType& KeyEventSignal() { return mKeyEventSignal; }

  /**
   * @copydoc Dali::GlWindow::TouchSignal()
   */
  TouchSignalType& TouchSignal() { return mTouchSignal; }

  /**
   * @copydoc Dali::GlWindow::VisibilityChangedSignal()
   */
  VisibilityChangedSignalType& VisibilityChangedSignal() { return mVisibilityChangedSignal; }

private:

  std::unique_ptr< WindowBase >         mWindowBase;
  std::unique_ptr< GraphicsInterface >  mGraphics;                    ///< Graphics interface
  Dali::DisplayConnection*              mDisplayConnection;
  std::string                           mName;
  std::string                           mClassName;
  EventHandlerPtr                       mEventHandler;         ///< The window events handler
  PositionSize                          mPositionSize;
  ColorDepth                            mColorDepth;
  Dali::Window                          mChildWindow;
  bool                                  mIsTransparent:1;
  bool                                  mIsFocusAcceptable:1;
  bool                                  mIconified:1;
  bool                                  mOpaqueState:1;
  bool                                  mResizedEnabled:1;
  bool                                  mVisible:1;
  bool                                  mIsRotated:1;
  bool                                  mIsWindowRotated:1;
  bool                                  mIsTouched:1;

  std::vector< int >                    mAvailableAngles;
  int                                   mPreferredAngle;
  int                                   mTotalRotationAngle;   ///< The angle of window + screen rotation angle % 360
  int                                   mWindowRotationAngle;  ///< The angle of window rotation angle
  int                                   mScreenRotationAngle;  ///< The angle of screen rotation angle
  int                                   mOrientationMode;      ///< 0: Default portrati, 1:Default landscape
  int                                   mWindowWidth;          ///< The width of the window
  int                                   mWindowHeight;         ///< The height of the window
  int                                   mNativeWindowId;       ///< The Native Window Id

  // Signals
  KeyEventSignalType                    mKeyEventSignal;
  TouchSignalType                       mTouchSignal;
  FocusChangeSignalType                 mFocusChangeSignal;
  ResizedSignalType                     mResizedSignal;
  VisibilityChangedSignalType           mVisibilityChangedSignal;

  // EGL, GL Resource
  GlInitialize                          mGLInitCallback;
  GlRenderFrame                         mGLRenderFrameCallback;
  GlTerminate                           mGLTerminateCallback;
  CallbackBase*                         mGLRenderCallback;
  EGLSurface                            mEGLSurface;
  EGLContext                            mEGLContext;
  Dali::GlWindow::GlesVersion           mGLESVersion;
  bool                                  mInitCallback:1;
  bool                                  mDepth:1;
  bool                                  mStencil:1;
  bool                                  mIsEGLInitialize:1;
  int                                   mMSAA;
};

} // namespace Adaptor
} // namepsace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::GlWindow& GetImplementation(Dali::GlWindow& window)
{
  DALI_ASSERT_ALWAYS( window && "Window handle is empty" );
  BaseObject& object = window.GetBaseObject();
  return static_cast<Internal::Adaptor::GlWindow&>(object);
}

inline const Internal::Adaptor::GlWindow& GetImplementation(const Dali::GlWindow& window)
{
  DALI_ASSERT_ALWAYS( window && "Window handle is empty" );
  const BaseObject& object = window.GetBaseObject();
  return static_cast<const Internal::Adaptor::GlWindow&>(object);
}

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_IMPL_H
