#ifndef __DALI_INTERNAL_WINDOW_H__
#define __DALI_INTERNAL_WINDOW_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <base/lifecycle-observer.h>
#include <base/interfaces/indicator-interface.h>
#include <adaptor-impl.h>
#include <window.h>
#include <orientation.h>
#include <render-surface.h>
#include <drag-and-drop-detector.h>
#include <window-devel.h>

namespace Dali
{
class Adaptor;
class RenderSurface;

namespace Integration
{
class SystemOverlay;
}

namespace Internal
{
namespace Adaptor
{
class Orientation;

class Window;
typedef IntrusivePtr<Window> WindowPtr;
typedef IntrusivePtr<Orientation> OrientationPtr;

/**
 * Window provides a surface to render onto with orientation & indicator properties.
 */
class Window : public Dali::BaseObject, public IndicatorInterface::Observer, public LifeCycleObserver
{
public:
  typedef Dali::Window::IndicatorSignalType IndicatorSignalType;
  typedef Dali::DevelWindow::FocusSignalType FocusSignalType;
  typedef Signal< void () > SignalType;

  /**
   * Create a new Window. This should only be called once by the Application class
   * @param[in] windowPosition The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return A newly allocated Window
   */
  static Window* New(const PositionSize& posSize, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * Pass the adaptor back to the overlay. This allows the window to access Core's overlay.
   * @param[in] adaptor An initialized adaptor
   */
  void SetAdaptor(Dali::Adaptor& adaptor);

  /**
   * Get the window surface
   * @return The render surface
   */
  RenderSurface* GetSurface();

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
  void SetClass( std::string name, std::string klass );

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
   * @brief Sets whether window accepts focus or not.
   *
   * @param[in] accept If focus is accepted or not. Default is true.
   */
  void SetAcceptFocus( bool accept );

  /**
   * @brief Returns whether window accepts focus or not.
   *
   * @param[in] window The window to accept focus
   * @return True if the window accept focus, false otherwise
   */
  bool IsFocusAcceptable();

  /**
   * @brief Shows the window if it is hidden.
   */
  void Show();

  /**
   * @brief Hides the window if it is showing.
   */
  void Hide();

  /**
   * @brief Returns whether the window is showing or not.
   * @return True if the window is showing, false otherwise.
   */
  bool IsVisible() const;

  /**
   * Called from Orientation after the Change signal has been sent
   */
  void RotationDone( int orientation, int width, int height );

  /**
   * @brief Gets the count of supported auxiliary hints of the window.
   * @return The number of supported auxiliary hints.
   *
   * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
   * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
   */
   unsigned int GetSupportedAuxiliaryHintCount();

  /**
   * @brief Gets the supported auxiliary hint string of the window.
   * @param[in] index The index of the supported auxiliary hint lists
   * @return The auxiliary hint string of the index.
   *
   * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
   * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
   */
  std::string GetSupportedAuxiliaryHint( unsigned int index );

  /**
   * @brief Creates an auxiliary hint of the window.
   * @param[in] hint The auxiliary hint string.
   * @param[in] value The value string.
   * @return The ID of created auxiliary hint, or @c 0 on failure.
   */
  unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value );

  /**
   * @brief Removes an auxiliary hint of the window.
   * @param[in] id The ID of the auxiliary hint.
   * @return True if no error occurred, false otherwise.
   */
  bool RemoveAuxiliaryHint( unsigned int id );

  /**
   * @brief Changes a value of the auxiliary hint.
   * @param[in] id The auxiliary hint ID.
   * @param[in] value The value string to be set.
   * @return True if no error occurred, false otherwise.
   */
  bool SetAuxiliaryHintValue( unsigned int id, const std::string& value );

  /**
   * @brief Gets a value of the auxiliary hint.
   * @param[in] id The auxiliary hint ID.
   * @return The string value of the auxiliary hint ID, or an empty string if none exists.
   */
  std::string GetAuxiliaryHintValue( unsigned int id ) const;

  /**
   * @brief Gets a ID of the auxiliary hint string.
   * @param[in] hint The auxiliary hint string.
   * @return The ID of the auxiliary hint string, or @c 0 if none exists.
   */
  unsigned int GetAuxiliaryHintId( const std::string& hint ) const;

  /**
   * @brief Sets a region to get input events.
   * @param[in] inputRegion The rectangle region to get input events.
   * @note To set an empty region, pass width and height as 0. An empty input region means the entire window will accept input events.
   */
  void SetInputRegion( const Rect< int >& inputRegion );

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
  void Initialize(const PositionSize& posSize, const std::string& name, const std::string& className);

  /**
   * Shows / hides the indicator bar.
   * Handles close/open if rotation changes whilst hidden
   */
  void DoShowIndicator( Dali::Window::WindowOrientation lastOrientation );

  /**
   * Close current indicator and open a connection onto the new indicator service.
   * Effect may not be synchronous if waiting for an indicator update on existing connection.
   */
  void DoRotateIndicator( Dali::Window::WindowOrientation orientation );

  /**
   * Change the indicator actor's rotation to match the current orientation
   */
  void SetIndicatorActorRotation();

  /**
   * Set the indicator properties on the window
   */
  void SetIndicatorProperties( bool isShown, Dali::Window::WindowOrientation lastOrientation );

private: // IndicatorInterface::Observer interface

  /**
   * @copydoc Dali::Internal::Adaptor::IndicatorInterface::Observer::IndicatorTypeChanged()
   */
  virtual void IndicatorTypeChanged( IndicatorInterface::Type type );

  /**
   * @copydoc Dali::Internal::Adaptor::IndicatorInterface::Observer::IndicatorClosed()
   */
  virtual void IndicatorClosed( IndicatorInterface* indicator);

  /**
   * @copydoc Dali::Internal::Adaptor::IndicatorInterface::Observer::IndicatorVisibilityChanged()
   */
  virtual void IndicatorVisibilityChanged( bool isVisible );

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
   * The user should connect to this signal to get a timing when window gains focus or loses focus.
   */
  FocusSignalType& FocusChangedSignal() { return mFocusChangedSignal; }

  /**
   * This signal is emitted when the window is requesting to be deleted
   */
  SignalType& DeleteRequestSignal() { return mDeleteRequestSignal; }

private:

  typedef std::vector< std::pair< std::string, std::string > > AuxiliaryHints;

  RenderSurface*                   mSurface;
  Dali::Window::IndicatorVisibleMode mIndicatorVisible; ///< public state
  bool                             mIndicatorIsShown:1; ///< private state
  bool                             mShowRotatedIndicatorOnClose:1;
  bool                             mStarted:1;
  bool                             mIsTransparent:1;
  bool                             mWMRotationAppSet:1;
  bool                             mEcoreEventHander:1;
  bool                             mIsFocusAcceptable:1;
  bool                             mVisible:1;
  IndicatorInterface*              mIndicator;
  Dali::Window::WindowOrientation  mIndicatorOrientation;
  Dali::Window::WindowOrientation  mNextIndicatorOrientation;
  Dali::Window::IndicatorBgOpacity mIndicatorOpacityMode;
  Integration::SystemOverlay*      mOverlay;
  Adaptor*                         mAdaptor;
  Dali::DragAndDropDetector        mDragAndDropDetector;

  struct EventHandler;
  EventHandler*                    mEventHandler;

  OrientationPtr                               mOrientation;
  std::vector<Dali::Window::WindowOrientation> mAvailableOrientations;
  Dali::Window::WindowOrientation              mPreferredOrientation;

  std::vector< std::string >        mSupportedAuxiliaryHints;
  AuxiliaryHints                    mAuxiliaryHints;

  // Signals
  IndicatorSignalType mIndicatorVisibilityChangedSignal;
  FocusSignalType     mFocusChangedSignal;
  SignalType          mDeleteRequestSignal;
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


#endif // __DALI_INTERNAL_WINDOW_H__
