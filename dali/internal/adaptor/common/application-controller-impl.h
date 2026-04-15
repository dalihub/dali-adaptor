#ifndef DALI_INTERNAL_APPLICATION_CONTROLLER_H
#define DALI_INTERNAL_APPLICATION_CONTROLLER_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-object.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/application-controller.h>
#include <dali/public-api/adaptor-framework/ui-context.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class EnvironmentOptions;
class FrameworkFactory;

using ApplicationControllerPtr = IntrusivePtr<ApplicationController>;

/**
 * @brief Implementation of the ApplicationController class.
 */
class ApplicationController : public BaseObject
{
public:
  /**
   * @brief Pre-initializes resources on behalf of the Launchpad daemon.
   *
   * Called once by the Launchpad process before any real application is launched.
   * Performs font pre-loading and creates the initial ApplicationController with
   * a pre-initialized (hidden) window.
   *
   * @param[in] argc Pointer to the argument count
   * @param[in] argv Pointer to the argument list
   */
  static void LaunchpadApplicationPreInitialize(int* argc, char** argv[]);

  /**
   * @brief Returns the controller created during Launchpad pre-initialization, if any.
   *
   * @return The pre-initialized ApplicationControllerPtr, or an empty ptr if not pre-initialized.
   */
  static ApplicationControllerPtr GetLaunchpadApplicationController();

  /**
   * @copydoc Dali::ApplicationController::New()
   */
  static ApplicationControllerPtr New(PositionSize windowPositionSize);

  /**
   * @copydoc Dali::ApplicationController::PreInitialize()
   */
  void PreInitialize();

  /**
   * @brief Completes the pre-initialization sequence by notifying LifecycleController::OnPreInit().
   *
   * Called automatically at the end of PreInitialize() unless SetDeferCompletePreInitialize(true)
   * has been called. In the deferred case, the caller (e.g. Internal::Application) is responsible
   * for invoking this method after any additional setup work is done.
   */
  void CompletePreInitialize();

  /**
   * @copydoc Dali::ApplicationController::PostInitialize()
   */
  void PostInitialize();

  /**
   * @copydoc Dali::ApplicationController::PrePause()
   */
  void PrePause();

  /**
   * @copydoc Dali::ApplicationController::PostPause()
   */
  void PostPause();

  /**
   * @copydoc Dali::ApplicationController::PreResume()
   */
  void PreResume();

  /**
   * @copydoc Dali::ApplicationController::PostResume()
   */
  void PostResume();

  /**
   * @copydoc Dali::ApplicationController::PreTerminate()
   */
  void PreTerminate();

  /**
   * @copydoc Dali::ApplicationController::PostTerminate()
   */
  void PostTerminate();

  /**
   * @brief Called when the framework informs the application that it should reset itself.
   */
  void PreReset();

  /**
   * @copydoc Dali::ApplicationController::PreLanguageChanged()
   */
  void PreLanguageChanged(const std::string& language);

  /**
   * @brief Controls whether PreInitialize() automatically calls CompletePreInitialize().
   *
   * By default (false), PreInitialize() calls CompletePreInitialize() after adaptor start,
   * which triggers LifecycleController::OnPreInit().
   *
   * Set to true when the caller (e.g. Internal::Application) needs to perform additional
   * work between adaptor start and the OnPreInit notification, and will call
   * CompletePreInitialize() explicitly at the right moment.
   *
   * @param[in] defer If true, PreInitialize() skips the CompletePreInitialize() call.
   */
  void SetDeferCompletePreInitialize(bool defer);

  /**
   * @brief Overrides the window position and size set at construction time.
   *
   * Must be called before PreInitialize().
   * @param[in] windowPositionSize The desired window position and size
   */
  void SetWindowPositionSize(PositionSize windowPositionSize);

  /**
   * @brief Sets the window title (class name) for the main window.
   *
   * @param[in] windowName The window name / title string
   */
  void SetWindowName(const std::string& windowName);

  /**
   * @brief Configures the main window using a WindowData descriptor.
   *
   * Must be called before PreInitialize() so that transparency, window type, and
   * other properties are applied correctly during window creation.
   * @param[in] windowData The window configuration data
   */
  void SetWindowData(const Dali::WindowData& windowData);

  /**
   * @brief Returns the main Window handle managed by this controller.
   *
   * @return The main Dali::Window handle (may be empty before PreInitialize())
   */
  Dali::Window GetWindow() const
  {
    return mMainWindow;
  }

  /**
   * @brief Returns a pointer to the Adaptor managed by this controller.
   *
   * @return Pointer to the Dali::Adaptor, or nullptr before PreInitialize()
   */
  Dali::Adaptor* GetAdaptor() const
  {
    return mAdaptor.get();
  }

  /**
   * @brief Returns the FrameworkFactory owned by this controller.
   *
   * @return Pointer to the FrameworkFactory, or nullptr if not yet created.
   */
  FrameworkFactory* GetFrameworkFactory() const
  {
    return mFrameworkFactory.get();
  }

private:
  /**
   * @brief Constructor.
   *
   * @param[in] windowPositionSize The initial position and size of the main window
   */
  ApplicationController(PositionSize windowPositionSize);

  /**
   * @brief Destructor
   */
  ~ApplicationController();

  /**
   * @brief Pre-initializes the application in the Launchpad phase.
   *
   * Creates a Window (hidden) in advance. Depending on compile options, may also
   * create the Adaptor. Does NOT call Adaptor::Start(). Idempotent: a second call
   * when already pre-initialized is a no-op.
   */
  void LaunchpadPreInitialize();

  /**
   * @brief Creates the default window.
   *
   * Resolves the final window size (from mWindowPositionSize, environment options, or
   * screen size), assembles a WindowData descriptor, and creates the Dali Window.
   *
   * @param[in] isPreInitialize If true, the window is created in pre-initialized (hidden) state.
   *                            If false, a normal visible window is created.
   */
  void CreateWindow(bool isPreInitialize);

  /**
   * @brief Creates the Adaptor.
   *
   * Must be called after the main window has been created, and before Show() or Start().
   */
  void CreateAdaptor();

  /**
   * @brief Reads environment variables and updates mEnvironmentOptions accordingly.
   */
  void UpdateEnvironmentOptions();

  /**
   * @brief Updates window properties (size, name, class, front-buffer) after transitioning
   *        from the PRE_INITIALIZED state to the real application lifecycle.
   *
   * Called inside PreInitialize() when the controller is in the PRE_INITIALIZED state.
   * Reconciles the pre-initialized window with the final environment and application settings.
   */
  void UpdatePreInitializedWindowInfo();

  ApplicationController(const ApplicationController&)            = delete; ///< Deleted copy constructor.
  ApplicationController(ApplicationController&&)                 = delete; ///< Deleted move constructor.
  ApplicationController& operator=(const ApplicationController&) = delete; ///< Deleted copy assignment operator.
  ApplicationController& operator=(ApplicationController&&)      = delete; ///< Deleted move assignment operator.

private:
  /**
   * @brief LaunchpadState is used to improve application launch performance.
   * When an application is pre-initialized, resources are preloaded, some functions are initialized and a window is created in advance.
   */
  enum class LaunchpadState
  {
    NONE,           ///< The default state
    PRE_INITIALIZED ///< Application is pre-initialized.
  };

  Dali::Window    mMainWindow; ///< The main application window.
  Dali::UiContext mUiContext;  ///< The UiContext wrapping the main window.

  PositionSize                      mWindowPositionSize{}; ///< Requested window position/size.
  std::string                       mWindowName{};         ///< Window title / application name.
  std::unique_ptr<Dali::WindowData> mWindowData{};         ///< Optional window configuration data.

  std::unique_ptr<Dali::Adaptor>      mAdaptor{};            ///< The DALi Adaptor instance.
  std::unique_ptr<EnvironmentOptions> mEnvironmentOptions{}; ///< Parsed environment options.
  std::unique_ptr<FrameworkFactory>   mFrameworkFactory{};   ///< Factory for creating the Framework.

  LaunchpadState mLaunchpadState{LaunchpadState::NONE}; ///< Current Launchpad lifecycle state.
  bool           mDeferCompletePreInitialize{false};    ///< If true, CompletePreInitialize() is not called automatically in PreInitialize().
};

/**
 * @brief Retrieves the internal implementation from the public handle.
 *
 * @param[in] controller The public ApplicationController handle
 * @return Reference to the internal ApplicationController implementation
 */
inline ApplicationController& GetImplementation(Dali::ApplicationController& controller)
{
  DALI_ASSERT_ALWAYS(controller && "ApplicationController handle is empty");

  BaseObject& handle = controller.GetBaseObject();

  return static_cast<Internal::Adaptor::ApplicationController&>(handle);
}

/**
 * @brief Retrieves the internal implementation from the public handle (const version).
 *
 * @param[in] controller The public ApplicationController handle
 * @return Const reference to the internal ApplicationController implementation
 */
inline const ApplicationController& GetImplementation(const Dali::ApplicationController& controller)
{
  DALI_ASSERT_ALWAYS(controller && "ApplicationController handle is empty");

  const BaseObject& handle = controller.GetBaseObject();

  return static_cast<const Internal::Adaptor::ApplicationController&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_APPLICATION_CONTROLLER_H
