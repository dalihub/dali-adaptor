#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_IMPL_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/ref-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/internal/window-system/common/gl-window-render-thread.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class WindowBase;

class GlWindow;
using GlWindowPtr     = IntrusivePtr<GlWindow>;
using EventHandlerPtr = IntrusivePtr<EventHandler>;

/**
 * Window provides a surface to render onto with orientation.
 */
class GlWindow : public BaseObject, public EventHandler::Observer, public DamageObserver, public ConnectionTracker
{
public:
  using KeyEventSignalType          = Dali::GlWindow::KeyEventSignalType;
  using TouchEventSignalType        = Dali::GlWindow::TouchEventSignalType;
  using FocusChangeSignalType       = Dali::GlWindow::FocusChangeSignalType;
  using ResizeSignalType            = Dali::GlWindow::ResizeSignalType;
  using VisibilityChangedSignalType = Dali::GlWindow::VisibilityChangedSignalType;
  using SignalType                  = Signal<void()>;

  /**
   * @brief Create a new GlWindow. This should only be called once by the Application class
   * @param[in] positionSize The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return A newly allocated Window
   */
  static GlWindow* New(const PositionSize& positionSize, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * @copydoc Dali::GlWindow::SetGraphicsConfig()
   */
  void SetGraphicsConfig(bool depth, bool stencil, int msaa, Dali::GlWindow::GlesVersion version);

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
  std::string GetSupportedAuxiliaryHint(unsigned int index) const;

  /**
   * @copydoc Dali::GlWindow::AddAuxiliaryHint()
   */
  unsigned int AddAuxiliaryHint(const std::string& hint, const std::string& value);

  /**
   * @copydoc Dali::GlWindow::RemoveAuxiliaryHint()
   */
  bool RemoveAuxiliaryHint(unsigned int id);

  /**
   * @copydoc Dali::GlWindow::SetAuxiliaryHintValue()
   */
  bool SetAuxiliaryHintValue(unsigned int id, const std::string& value);

  /**
   * @copydoc Dali::GlWindow::GetAuxiliaryHintValue()
   */
  std::string GetAuxiliaryHintValue(unsigned int id) const;

  /**
   * @copydoc Dali::GlWindow::GetAuxiliaryHintId()
   */
  unsigned int GetAuxiliaryHintId(const std::string& hint) const;

  /**
   * @copydoc Dali::GlWindow::SetInputRegion()
   */
  void SetInputRegion(const Rect<int>& inputRegion);

  /**
   * @copydoc Dali::GlWindow::SetOpaqueState()
   */
  void SetOpaqueState(bool opaque);

  /**
   * @copydoc Dali::GlWindow::IsOpaqueState()
   */
  bool IsOpaqueState() const;

  /**
   * @copydoc Dali::GlWindow::SetPositionSize()
   */
  void SetPositionSize(PositionSize positionSize);

  /**
   * @copydoc Dali::GlWindow::GetPositionSize()
   */
  PositionSize GetPositionSize() const;

  /**
   * @copydoc Dali::GlWindow::GetCurrentOrientation() const
   */
  WindowOrientation GetCurrentOrientation() const;

  /**
   * @copydoc Dali::GlWindow::SetAvailableOrientations()
   */
  void SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations);

  /**
   * @copydoc Dali::GlWindow::SetPreferredOrientation()
   */
  void SetPreferredOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::GlWindow::RegisterGlCallbacks()
   */
  void RegisterGlCallbacks(CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback);

  /**
   * @copydoc Dali::GlWindow::RenderOnce()
   */
  void RenderOnce();

  /**
   * @copydoc Dali::GlWindow::SetRenderingMode()
   */
  void SetRenderingMode(Dali::GlWindow::RenderingMode mode);

  /**
   * @copydoc Dali::GlWindow::GetRenderingMode()
   */
  Dali::GlWindow::RenderingMode GetRenderingMode() const;

public: // For implementation
  /**
   * @brief Sets child window with Dali::Window
   *
   * @param[in] child The child window.
   *
   * Most of cases, child window is the default window in adaptor
   *
   * Currently the child window is default window.
   */
  void SetChild(Dali::Window& child);

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
   *
   * @param[in] positionSize The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   */
  void Initialize(const PositionSize& positionSize, const std::string& name, const std::string& className);

  /**
   * Called when the window becomes iconified or deiconified.
   *
   * @param[in] iconified The flag whether window is iconifed or deiconfied.
   */
  void OnIconifyChanged(bool iconified);

  /**
   * Called when the window focus is changed.
   * @param[in] focusIn The flag whether window is focused or not.
   */
  void OnFocusChanged(bool focusIn);

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
   *
   * @param[in] angles The list of the avaiabled rotation angle.
   */
  void SetAvailableAnlges(const std::vector<int>& angles);

  /**
   * @brief Check available window orientation for Available angle.
   *
   * @param[in] orientation the oritation value of window rotation.
   *
   * @return true is available window orientation. false is not available.
   */
  bool IsOrientationAvailable(WindowOrientation orientation) const;

  /**
   * @brief Convert from window orientation to angle using orientation mode value.
   *
   * @param[in] orientation the oritation value of window rotation.
   *
   * @return The coverted angle value is returned.
   */
  int ConvertToAngle(WindowOrientation orientation);

  /**
   * @brief Convert from angle to window orientation using orientation mode value.
   *
   * @param[in] angle the angle value of window rotation.
   *
   * @return The converted window orientation value is returned.
   */
  WindowOrientation ConvertToOrientation(int angle) const;

  /**
   * @brief Initialize and create EGL resource
   */
  void InitializeGraphics();

  /**
   * @brief Sets event handler for window's events.
   */
  void SetEventHandler();

  /**
   * @brief calculate screen position for rotation.
   */
  Vector2 RecalculatePosition(const Vector2& position);

  /**
   * @brief Sets window and class name.
   *
   * @param[in] name The name of the window
   * @param[in] className The class of the window
   */
  void SetClass(const std::string& name, const std::string className);

private:
  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnTouchPoint
   */
  void OnTouchPoint(Dali::Integration::Point& point, int timeStamp) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnMouseFrameEvent
   */
  void OnMouseFrameEvent() override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnWheelEvent
   */
  void OnWheelEvent(Dali::Integration::WheelEvent& wheelEvent) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnKeyEvent
   */
  void OnKeyEvent(Dali::Integration::KeyEvent& keyEvent) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnRotation
   */
  void OnRotation(const RotationEvent& rotation) override;

private: // From Dali::Internal::Adaptor::DamageObserver
  /**
   * @copydoc Dali::Internal::Adaptor::DamageObserver::OnDamaged()
   */
  void OnDamaged(const DamageArea& area);

  /**
   * @brief Updates screen rotation value and screen rotation works.
   *
   * @param[in] newAngle new screen rotation angle
   */
  void UpdateScreenRotation(int newAngle);

public: // Signals
  /**
   * @copydoc Dali::GlWindow::FocusChangeSignal()
   */
  FocusChangeSignalType& FocusChangeSignal()
  {
    return mFocusChangeSignal;
  }

  /**
   * @copydoc Dali::GlWindow::ResizeSignal()
   */
  ResizeSignalType& ResizeSignal()
  {
    return mResizeSignal;
  }

  /**
   * @copydoc Dali::GlWindow::KeyEventSignal()
   */
  KeyEventSignalType& KeyEventSignal()
  {
    return mKeyEventSignal;
  }

  /**
   * @copydoc Dali::GlWindow::TouchSignal()
   */
  TouchEventSignalType& TouchedSignal()
  {
    return mTouchedSignal;
  }

  /**
   * @copydoc Dali::GlWindow::VisibilityChangedSignal()
   */
  VisibilityChangedSignalType& VisibilityChangedSignal()
  {
    return mVisibilityChangedSignal;
  }

private:
  std::unique_ptr<WindowBase>                  mWindowBase;
  std::unique_ptr<Graphics::GraphicsInterface> mGraphics;             ///< Graphics interface
  std::unique_ptr<Dali::DisplayConnection>     mDisplayConnection;    ///< The native display connection
  std::unique_ptr<GlWindowRenderThread>        mGlWindowRenderThread; ///< The render thread
  EventHandlerPtr                              mEventHandler;         ///< The window events handler
  Dali::Window                                 mChildWindow;          ///< The default child UI Window
  std::string                                  mName;
  std::string                                  mClassName;
  bool                                         mIsTransparent : 1;
  bool                                         mIsFocusAcceptable : 1;
  bool                                         mIconified : 1;
  bool                                         mOpaqueState : 1;
  bool                                         mResizeEnabled : 1;
  bool                                         mVisible : 1;
  bool                                         mIsWindowRotated : 1;
  bool                                         mIsTouched : 1;
  bool                                         mIsEGLInitialized : 1;
  bool                                         mDepth : 1;
  bool                                         mStencil : 1;

  PositionSize                  mPositionSize; ///< The window position and size
  EnvironmentOptions            mEnvironmentOptions;
  std::vector<int>              mAvailableAngles; ///< The list of available angle
  ColorDepth                    mColorDepth;      ///< The color depth of window
  Dali::GlWindow::RenderingMode mRenderingMode;   ///< The rendering mode

  int mPreferredAngle;      ///< The angle of preferred angle
  int mTotalRotationAngle;  ///< The angle of window + screen rotation angle % 360
  int mWindowRotationAngle; ///< The angle of window rotation angle
  int mScreenRotationAngle; ///< The angle of screen rotation angle
  int mOrientationMode;     ///< 0: Default portrati, 1:Default landscape
  int mWindowWidth;         ///< The width of the window
  int mWindowHeight;        ///< The height of the window
  int mNativeWindowId;      ///< The Native Window Id
  int mMSAA;                ///< The multisample anti-aliasing for EGL Configuration

  // Signals
  KeyEventSignalType          mKeyEventSignal;
  TouchEventSignalType        mTouchedSignal;
  FocusChangeSignalType       mFocusChangeSignal;
  ResizeSignalType            mResizeSignal;
  VisibilityChangedSignalType mVisibilityChangedSignal;
};

} // namespace Adaptor
} // namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::GlWindow& GetImplementation(Dali::GlWindow& window)
{
  DALI_ASSERT_ALWAYS(window && "Window handle is empty");
  BaseObject& object = window.GetBaseObject();
  return static_cast<Internal::Adaptor::GlWindow&>(object);
}

inline const Internal::Adaptor::GlWindow& GetImplementation(const Dali::GlWindow& window)
{
  DALI_ASSERT_ALWAYS(window && "Window handle is empty");
  const BaseObject& object = window.GetBaseObject();
  return static_cast<const Internal::Adaptor::GlWindow&>(object);
}

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_GL_WINDOW_IMPL_H
