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
#include <dali/integration-api/scene.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/lifecycle-observer.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/devel-api/adaptor-framework/drag-and-drop-detector.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>

namespace Dali
{
class Adaptor;
class Actor;

namespace Internal
{
namespace Adaptor
{
class EventHandler;
class Orientation;
class RotationObserver;
class WindowRenderSurface;
class WindowBase;

class Window;
using WindowPtr = IntrusivePtr< Window >;
using OrientationPtr = IntrusivePtr< Orientation >;
using EventHandlerPtr = IntrusivePtr< EventHandler >;

/**
 * Window provides a surface to render onto with orientation & indicator properties.
 */
class Window : public Dali::BaseObject, public LifeCycleObserver, public ConnectionTracker
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
   * Pass the adaptor back to the overlay. This allows the window to access Core's overlay.
   * @param[in] adaptor An initialized adaptor
   */
  void SetAdaptor(Dali::Adaptor& adaptor);

  /**
   * Pass the adaptor back to the overlay. This allows the window to access Core's overlay.
   * @param[in] adaptor implementation An initialized adaptor implementation
   */
  void SetAdaptor(Adaptor& adaptor);

  /**
   * Get the window surface
   * @return The render surface
   */
  WindowRenderSurface* GetSurface() const;

  /**
   * Set the window surface
   * @param[in] surface The surface
   */
  void SetSurface(WindowRenderSurface* surface);

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
   * @brief Gets the window name.
   * @return The name of the window
   */
  std::string GetName() const;

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
   * @copydoc Dali::Window::Add()
   */
  void Add( Dali::Actor actor );

  /**
   * @copydoc Dali::Window::Remove()
   */
  void Remove( Dali::Actor remove );

  /**
   * @copydoc Dali::Window::SetBackgroundColor()
   */
  void SetBackgroundColor(Vector4 color);

  /**
   * @copydoc Dali::Window::GetBackgroundColor()
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @copydoc Dali::Window::GetRootLayer()
   */
  Dali::Layer GetRootLayer() const;

  /**
   * @copydoc Dali::Window::GetLayerCount()
   */
  uint32_t GetLayerCount() const;

  /**
   * @copydoc Dali::Window::GetLayer()
   */
  Dali::Layer GetLayer( uint32_t depth ) const;

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
   * @copydoc Dali::Window::GetDragAndDropDetector() const
   */
  Dali::DragAndDropDetector GetDragAndDropDetector() const;

  /**
   * @copydoc Dali::Window::GetNativeHandle() const
   */
  Dali::Any GetNativeHandle() const;

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
   * @copydoc Dali::Window::IsVisible() const
   */
  bool IsVisible() const;

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
   * @copydoc Dali::DevelWindow::GetRootLayer()
   */
  Dali::Layer GetRootLayer();

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
   * Called from Orientation after the Change signal has been sent
   */
  void RotationDone( int orientation, int width, int height );

  /**
   * @brief Retrieves the unique ID of the window.
   * @return The ID
   */
  uint32_t GetId() const;

  /**
   * Feed (Send) touch event to core and gesture manager
   * @param[in] touchEvent  The touch event holding the touch point information.
   */
  void FeedTouchPoint( TouchPoint& point, int timeStamp );

  /**
   * Feed (Send) wheel event to core and gesture manager
   * @param[in]  wheelEvent The wheel event
   */
  void FeedWheelEvent( WheelEvent& wheelEvent );

  /**
   * Feed (Send) key event to core
   * @param[in] keyEvent The key event holding the key information.
   */
  void FeedKeyEvent( KeyEvent& keyEvent );

  /**
   * Called when the adaptor is paused.
   */
  void Pause();

  /**
   * Called when the adaptor is resumed (from pause).
   */
  void Resume();

  /**
   * Set the rotation observer (note, some adaptors may not have a rotation observer)
   * @param[in] observer The rotation observer
   * @return If the rotation observer is set
   */
  bool SetRotationObserver( RotationObserver* observer );

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

private: // Adaptor::Observer interface

  /**
   * @copydoc Dali::Internal::Adaptor::Adaptor::Observer::OnStart()
   */
  virtual void OnStart();

  /**
   * @copydoc Dali::Internal::Adaptor::Adaptor::Observer::OnPause()
   */
  virtual void OnPause();

  /**
   * @copydoc Dali::Internal::Adaptor::Adaptor::Observer::OnResume()
   */
  virtual void OnResume();

  /**
   * @copydoc Dali::Internal::Adaptor::Adaptor::Observer::OnStop()
   */
  virtual void OnStop();

  /**
   * @copydoc Dali::Internal::Adaptor::Adaptor::Observer::OnDestroy()
   */
  virtual void OnDestroy();

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
  Dali::DevelWindow::EventProcessingFinishedSignalType& EventProcessingFinishedSignal() { return mScene.EventProcessingFinishedSignal(); };

  /**
   * @copydoc Dali::Window::KeyEventSignal()
   */
  Dali::DevelWindow::KeyEventSignalType& KeyEventSignal() { return mScene.KeyEventSignal(); };

  /**
    * @copydoc Dali::Window::TouchSignal()
    */
  Dali::DevelWindow::TouchSignalType& TouchSignal() { return mScene.TouchSignal(); };

  /**
   * @copydoc Dali::Window::WheelEventSignal()
   */
  Dali::DevelWindow::WheelEventSignalType& WheelEventSignal() { return mScene.WheelEventSignal(); };

private:

  static uint32_t                       mWindowCounter;    ///< A counter to track the window creation
  uint32_t                              mId;               ///< A unique ID to identify the window starting from 0
  std::unique_ptr< WindowRenderSurface >mSurface;          ///< The window rendering surface
  Dali::Integration::Scene              mScene;
  WindowBase*                           mWindowBase;
  std::string                           mName;
  std::string                           mClassName;
  bool                                  mStarted:1;
  bool                                  mIsTransparent:1;
  bool                                  mIsFocusAcceptable:1;
  bool                                  mVisible:1;
  bool                                  mIconified:1;
  bool                                  mOpaqueState:1;
  bool                                  mResizeEnabled:1;
  Adaptor*                              mAdaptor;
  Dali::DragAndDropDetector             mDragAndDropDetector;
  Dali::Window::Type                    mType;

  OrientationPtr                               mOrientation;
  std::vector<Dali::Window::WindowOrientation> mAvailableOrientations;
  Dali::Window::WindowOrientation              mPreferredOrientation;

  EventHandlerPtr                       mEventHandler;      ///< The window events handler

  Vector4                               mBackgroundColor;

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
