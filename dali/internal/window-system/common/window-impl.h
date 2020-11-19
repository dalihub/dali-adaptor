#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H

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
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <dali/public-api/object/property-array.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/common/event-handler.h>

namespace Dali
{
class Adaptor;
class Actor;
class RenderSurfaceInterface;

namespace Internal
{
namespace Adaptor
{
class Orientation;
class WindowRenderSurface;
class WindowBase;

class Window;
using WindowPtr = IntrusivePtr< Window >;
using OrientationPtr = IntrusivePtr< Orientation >;
using EventHandlerPtr = IntrusivePtr< EventHandler >;

/**
 * Window provides a surface to render onto with orientation & indicator properties.
 */
class Window : public Dali::Internal::Adaptor::SceneHolder, public EventHandler::Observer, public ConnectionTracker
{
public:
  typedef Dali::Window::FocusChangeSignalType FocusChangeSignalType;
  typedef Dali::Window::ResizeSignalType ResizeSignalType;
  typedef Dali::DevelWindow::VisibilityChangedSignalType VisibilityChangedSignalType;
  typedef Dali::DevelWindow::TransitionEffectEventSignalType TransitionEffectEventSignalType;
  typedef Dali::DevelWindow::KeyboardRepeatSettingsChangedSignalType KeyboardRepeatSettingsChangedSignalType;
  typedef Signal< void () > SignalType;

  /**
   * @brief Create a new Window. This should only be called once by the Application class
   * @param[in] positionSize The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return A newly allocated Window
   */
  static Window* New(const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * @brief Create a new Window. This should only be called once by the Application class
   * @param[in] surface The surface used to render on.
   * @param[in] positionSize The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return A newly allocated Window
   */
  static Window* New(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * @copydoc Dali::Window::SetClass()
   */
  void SetClass( std::string name, std::string className );

  /**
   * @brief Gets the window class name.
   * @return The class of the window
   */
  std::string GetClassName() const;

  /**
   * @copydoc Dali::Window::Raise()
   */
  void Raise();

  /**
   * @copydoc Dali::Window::Lower()
   */
  void Lower();

  /**
   * @copydoc Dali::Window::Activate()
   */
  void Activate();

  /**
   * @copydoc Dali::Window::GetLayerCount()
   */
  uint32_t GetLayerCount() const;

  /**
   * @copydoc Dali::Window::GetLayer()
   */
  Dali::Layer GetLayer( uint32_t depth ) const;

  /**
   * @copydoc Dali::DevelWindow::GetRenderTaskList()
   */
  Dali::RenderTaskList GetRenderTaskList() const;

  /**
   * @copydoc Dali::Window::AddAvailableOrientation()
   */
  void AddAvailableOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::RemoveAvailableOrientation()
   */
  void RemoveAvailableOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::SetPreferredOrientation()
   */
  void SetPreferredOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::GetPreferredOrientation()
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * @copydoc Dali::Window::SetAcceptFocus()
   */
  void SetAcceptFocus( bool accept );

  /**
   * @copydoc Dali::Window::IsFocusAcceptable()
   */
  bool IsFocusAcceptable() const;

  /**
   * @copydoc Dali::Window::Show()
   */
  void Show();

  /**
   * @copydoc Dali::Window::Hide()
   */
  void Hide();

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHintCount()
   */
  unsigned int GetSupportedAuxiliaryHintCount() const;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHint()
   */
  std::string GetSupportedAuxiliaryHint( unsigned int index ) const;

  /**
   * @copydoc Dali::Window::AddAuxiliaryHint()
   */
  unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value );

  /**
   * @copydoc Dali::Window::RemoveAuxiliaryHint()
   */
  bool RemoveAuxiliaryHint( unsigned int id );

  /**
   * @copydoc Dali::Window::SetAuxiliaryHintValue()
   */
  bool SetAuxiliaryHintValue( unsigned int id, const std::string& value );

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintValue()
   */
  std::string GetAuxiliaryHintValue( unsigned int id ) const;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintId()
   */
  unsigned int GetAuxiliaryHintId( const std::string& hint ) const;

  /**
   * @copydoc Dali::Window::SetInputRegion()
   */
  void SetInputRegion( const Rect< int >& inputRegion );

  /**
   * @copydoc Dali::Window::SetType()
   */
  void SetType( WindowType type );

  /**
   * @copydoc Dali::Window::GetType() const
   */
  WindowType GetType() const;

  /**
   * @copydoc Dali::Window::SetNotificationLevel()
   */
  bool SetNotificationLevel( WindowNotificationLevel level );

  /**
   * @copydoc Dali::Window::GetNotificationLevel()
   */
  WindowNotificationLevel GetNotificationLevel() const;

  /**
   * @copydoc Dali::Window::SetOpaqueState()
   */
  void SetOpaqueState( bool opaque );

  /**
   * @copydoc Dali::Window::IsOpaqueState()
   */
  bool IsOpaqueState() const;

  /**
   * @copydoc Dali::Window::SetScreenOffMode()
   */
  bool SetScreenOffMode(WindowScreenOffMode screenOffMode);

  /**
   * @copydoc Dali::Window::GetScreenOffMode()
   */
  WindowScreenOffMode GetScreenOffMode() const;

  /**
   * @copydoc Dali::Window::SetBrightness()
   */
  bool SetBrightness( int brightness );

  /**
   * @copydoc Dali::Window::GetBrightness()
   */
  int GetBrightness() const;

  /**
   * @copydoc Dali::Window::SetSize()
   */
  void SetSize( Dali::Window::WindowSize size );

  /**
   * @copydoc Dali::Window::GetSize()
   */
  Dali::Window::WindowSize GetSize() const;

  /**
   * @copydoc Dali::Window::SetPosition()
   */
  void SetPosition( Dali::Window::WindowPosition position );

  /**
   * @copydoc Dali::Window::GetPosition()
   */
  Dali::Window::WindowPosition GetPosition() const;

  /**
   * @copydoc Dali::DevelWindow::SetPositionSize()
   */
  void SetPositionSize( PositionSize positionSize );

  /**
   * @copydoc Dali::Window::GetRootLayer()
   */
  Dali::Layer GetRootLayer() const;

  /**
   * @copydoc Dali::Window::SetTransparency()
   */
  void SetTransparency( bool transparent );

  /**
   * @copydoc Dali::KeyGrab::GrabKey()
   */
  bool GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode );

  /**
   * @copydoc Dali::KeyGrab::UngrabKey()
   */
  bool UngrabKey( Dali::KEY key );

  /**
   * @copydoc Dali::KeyGrab::GrabKeyList()
   */
  bool GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result );

  /**
   * @copydoc Dali::KeyGrab::UngrabKeyList()
   */
  bool UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result );

  /**
   * @copydoc Dali::DevelWindow::Get()
   */
  static Dali::Window Get( Dali::Actor actor );

  /**
   * @copydoc Dali::DevelWindow::SetParent()
   */
  void SetParent( Dali::Window& parent );

  /**
   * @copydoc Dali::DevelWindow::Unparent()
   */
  void Unparent();

  /**
   * @copydoc Dali::DevelWindow::GetParent()
   */
  Dali::Window GetParent();

  /**
   * @copydoc Dali::DevelWindow::GetCurrentOrientation()
   */
  WindowOrientation GetCurrentOrientation() const;

  /**
   * @copydoc Dali::DevelWindow::SetAvailableOrientations()
   */
  void SetAvailableOrientations( const Dali::Vector<WindowOrientation>& orientations );

public: // Dali::Internal::Adaptor::SceneHolder

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::GetNativeHandle
   */
  Dali::Any GetNativeHandle() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::IsVisible
   */
  bool IsVisible() const override;

  /**
   * @copydoc Dali::DevelWindow::GetNativeId()
   */
  int32_t GetNativeId() const;

private:

  /**
   * @brief Enumeration for orietation mode.
   * The Orientation Mode is related to screen size.
   * If screen width is longer than height, the Orientation Mode will have LANDSCAPE.
   * Otherwise screen width is shorter than height or same, the Orientation Mode will have PORTRAIT.
   */
  enum class OrientationMode
  {
    PORTRAIT = 0,
    LANDSCAPE
  };

  /**
   * Private constructor.
   * @sa Window::New()
   */
  Window();

  /**
   * Destructor
   */
  ~Window() override;

  /**
   * Second stage initialization
   */
  void Initialize(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className);

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
   * Called when the window receives a Transition effect-start/end event.
   */
  void OnTransitionEffectEvent( WindowEffectState state, WindowEffectType type );

  /**
   * @brief Called when window receives a keyboard repeat event.
   */
  void OnKeyboardRepeatSettingsChanged();

  /**
   * @brief Called when the window redraw is requested.
   */
  void OnWindowRedrawRequest();

  /**
   * @brief Set available orientation to window base.
   */
  void SetAvailableAnlges( const std::vector< int >& angles );

  /**
   * @brief Convert from window orientation to angle using OrientationMode.
   */
  int ConvertToAngle( WindowOrientation orientation );

  /**
   * @brief Convert from angle to window orientation using OrientationMode.
   */
  WindowOrientation ConvertToOrientation( int angle ) const;

  /**
   * @brief Check available window orientation for Available orientation.
   */
  bool IsOrientationAvailable( WindowOrientation orientation ) const;

private: // Dali::Internal::Adaptor::SceneHolder

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnAdaptorSet
   */
  void OnAdaptorSet( Dali::Adaptor& adaptor ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnSurfaceSet
   */
  void OnSurfaceSet( Dali::RenderSurfaceInterface* surface ) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnPause
   */
  void OnPause() override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnResume
   */
  void OnResume() override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::RecalculateTouchPosition
   */
  void RecalculateTouchPosition( Integration::Point& point ) override;

private: // Dali::Internal::Adaptor::EventHandler::Observer

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

public: // Signals

  /**
   * @copydoc Dali::Window::FocusChangeSignal()
   */
  FocusChangeSignalType& FocusChangeSignal() { return mFocusChangeSignal; }

  /**
   * @copydoc Dali::Window::ResizedSignal()
   */
  ResizeSignalType& ResizeSignal() { return mResizeSignal; }

  /**
   * This signal is emitted when the window is requesting to be deleted
   */
  SignalType& DeleteRequestSignal() { return mDeleteRequestSignal; }

  /**
   * @copydoc Dali::DevelWindow::VisibilityChangedSignal()
   */
  VisibilityChangedSignalType& VisibilityChangedSignal() { return mVisibilityChangedSignal; }

  /**
   * @copydoc Dali::Window::SignalEventProcessingFinished()
   */
  Dali::DevelWindow::EventProcessingFinishedSignalType& EventProcessingFinishedSignal() { return mScene.EventProcessingFinishedSignal(); }

  /**
   * @copydoc Dali::DevelWindow::TransitionEffectEventSignal()
   */
  TransitionEffectEventSignalType& TransitionEffectEventSignal() { return mTransitionEffectEventSignal; }

  /**
   * @copydoc Dali::DevelWindow::KeyboardRepeatSettingsChangedSignal()
   */
  KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal() { return mKeyboardRepeatSettingsChangedSignal; }

private:

  WindowRenderSurface*                  mWindowSurface;      ///< The window rendering surface
  WindowBase*                           mWindowBase;
  std::string                           mName;
  std::string                           mClassName;
  bool                                  mIsTransparent:1;
  bool                                  mIsFocusAcceptable:1;
  bool                                  mIconified:1;
  bool                                  mOpaqueState:1;
  bool                                  mResizeEnabled:1;
  WindowType                            mType;
  Dali::Window                          mParentWindow;

  OrientationPtr                        mOrientation;
  std::vector< int >                    mAvailableAngles;
  int                                   mPreferredAngle;

  int                                   mRotationAngle;     ///< The angle of the rotation
  int                                   mWindowWidth;       ///< The width of the window
  int                                   mWindowHeight;      ///< The height of the window

  EventHandlerPtr                       mEventHandler;      ///< The window events handler

  OrientationMode                       mOrientationMode;

  int                                   mNativeWindowId;          ///< The Native Window Id

  // Signals
  SignalType                              mDeleteRequestSignal;
  FocusChangeSignalType                   mFocusChangeSignal;
  ResizeSignalType                        mResizeSignal;
  VisibilityChangedSignalType             mVisibilityChangedSignal;
  TransitionEffectEventSignalType         mTransitionEffectEventSignal;
  KeyboardRepeatSettingsChangedSignalType mKeyboardRepeatSettingsChangedSignal;
};

} // namespace Adaptor
} // namepsace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::Window& GetImplementation(Dali::Window& window)
{
  DALI_ASSERT_ALWAYS( window && "Window handle is empty" );
  BaseObject& object = window.GetBaseObject();
  return static_cast<Internal::Adaptor::Window&>(object);
}

inline const Internal::Adaptor::Window& GetImplementation(const Dali::Window& window)
{
  DALI_ASSERT_ALWAYS( window && "Window handle is empty" );
  const BaseObject& object = window.GetBaseObject();
  return static_cast<const Internal::Adaptor::Window&>(object);
}

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H
