#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H

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
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/render-tasks/render-task-list.h>

#ifdef DALI_ADAPTOR_COMPILATION
#include <dali/integration-api/scene-holder-impl.h>
#else
#include <dali/integration-api/adaptors/scene-holder-impl.h>
#endif

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
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
  typedef Dali::Window::IndicatorSignalType IndicatorSignalType;
  typedef Dali::Window::FocusSignalType FocusSignalType;
  typedef Dali::Window::ResizedSignalType ResizedSignalType;
  typedef Signal< void () > SignalType;

  /**
   * Create a new Window. This should only be called once by the Application class
   * @param[in] positionSize The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return A newly allocated Window
   */
  static Window* New(const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * @copydoc Dali::Window::ShowIndicator()
   */
  void ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode );

  /**
   * @copydoc Dali::Window::SetIndicatorBgOpacity()
   */
  void SetIndicatorBgOpacity( Dali::Window::IndicatorBgOpacity opacity );

  /**
   * @copydoc Dali::Window::RotateIndicator()
   */
  void RotateIndicator( Dali::Window::WindowOrientation orientation );

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
  void AddAvailableOrientation(Dali::Window::WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::RemoveAvailableOrientation()
   */
  void RemoveAvailableOrientation(Dali::Window::WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::SetAvailableOrientations()
   */
  void SetAvailableOrientations(const std::vector<Dali::Window::WindowOrientation>& orientations);

  /**
   * @copydoc Dali::Window::GetAvailableOrientations()
   */
  const std::vector<Dali::Window::WindowOrientation>& GetAvailableOrientations();

  /**
   * @copydoc Dali::Window::SetPreferredOrientation()
   */
  void SetPreferredOrientation(Dali::Window::WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::GetPreferredOrientation()
   */
  Dali::Window::WindowOrientation GetPreferredOrientation();

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
  void SetType( Dali::Window::Type type );

  /**
   * @copydoc Dali::Window::GetType() const
   */
  Dali::Window::Type GetType() const;

  /**
   * @copydoc Dali::Window::SetNotificationLevel()
   */
  bool SetNotificationLevel( Dali::Window::NotificationLevel::Type level );

  /**
   * @copydoc Dali::Window::GetNotificationLevel()
   */
  Dali::Window::NotificationLevel::Type GetNotificationLevel() const;

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
  bool SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode);

  /**
   * @copydoc Dali::Window::GetScreenOffMode()
   */
  Dali::Window::ScreenOffMode::Type GetScreenOffMode() const;

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

public: // Dali::Internal::Adaptor::SceneHolder

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::GetNativeHandle
   */
  Dali::Any GetNativeHandle() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::IsVisible
   */
  bool IsVisible() const override;

private:

  /**
   * Private constructor.
   * @sa Window::New()
   */
  Window();

  /**
   * Destructor
   */
  virtual ~Window();

  /**
   * Second stage initialization
   */
  void Initialize(const PositionSize& positionSize, const std::string& name, const std::string& className);

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
   * The user should connect to this signal to get a timing when indicator was shown / hidden.
   */
  IndicatorSignalType& IndicatorVisibilityChangedSignal() { return mIndicatorVisibilityChangedSignal; }

  /**
   * @copydoc Dali::Window::FocusChangedSignal()
   */
  FocusSignalType& FocusChangedSignal() { return mFocusChangedSignal; }

  /**
   * @copydoc Dali::Window::ResizedSignal()
   */
  ResizedSignalType& ResizedSignal() { return mResizedSignal; }

  /**
   * This signal is emitted when the window is requesting to be deleted
   */
  SignalType& DeleteRequestSignal() { return mDeleteRequestSignal; }

  /**
   * @copydoc Dali::Window::SignalEventProcessingFinished()
   */
  Dali::DevelWindow::EventProcessingFinishedSignalType& EventProcessingFinishedSignal() { return mScene.EventProcessingFinishedSignal(); }

  /**
   * @copydoc Dali::Window::KeyEventSignal()
   */
  Dali::DevelWindow::KeyEventSignalType& KeyEventSignal() { return mScene.KeyEventSignal(); }

  /**
   * @copydoc Dali::Window::KeyEventGeneratedSignal()
   */
  Dali::DevelWindow::KeyEventGeneratedSignalType& KeyEventGeneratedSignal() { return mScene.KeyEventGeneratedSignal(); }

  /**
    * @copydoc Dali::Window::TouchSignal()
    */
  Dali::DevelWindow::TouchSignalType& TouchSignal() { return mScene.TouchSignal(); }

  /**
   * @copydoc Dali::Window::WheelEventSignal()
   */
  Dali::DevelWindow::WheelEventSignalType& WheelEventSignal() { return mScene.WheelEventSignal(); }

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
  Dali::Window::Type                    mType;
  Dali::Window                          mParentWindow;

  OrientationPtr                               mOrientation;
  std::vector<Dali::Window::WindowOrientation> mAvailableOrientations;
  Dali::Window::WindowOrientation              mPreferredOrientation;

  int                                   mRotationAngle;     ///< The angle of the rotation
  int                                   mWindowWidth;       ///< The width of the window
  int                                   mWindowHeight;      ///< The height of the window

  EventHandlerPtr                       mEventHandler;      ///< The window events handler

  // Signals
  IndicatorSignalType                   mIndicatorVisibilityChangedSignal;
  FocusSignalType                       mFocusChangedSignal;
  ResizedSignalType                     mResizedSignal;
  SignalType                            mDeleteRequestSignal;


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
