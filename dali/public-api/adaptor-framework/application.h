#ifndef DALI_APPLICATION_H
#define DALI_APPLICATION_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/view-mode.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/application-configuration.h>
#include <dali/public-api/adaptor-framework/device-status.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Application;
}
}
/**
 * @brief An Application class object should be created by every application
 * that wishes to use Dali.
 *
 * It provides a means for initializing the
 * resources required by the Dali::Core.
 *
 * The Application class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * Application and create the Dali objects in the connected callback.
 *
 * Applications should follow the example below:
 *
 * @code
 * class ExampleController: public ConnectionTracker
 * {
 * public:
 *   ExampleController( Application& application )
 *   : mApplication( application )
 *   {
 *     mApplication.InitSignal().Connect( this, &ExampleController::Create );
 *   }
 *
 *   void Create( Application& application )
 *   {
 *     // Create Dali components...
 *   }
 *  ...
 * private:
 *   Application& mApplication;
 * };
 *
 * int main (int argc, char **argv)
 * {
 *   Application app = Application::New(&argc, &argv);
 *   ExampleController example( app );
 *   app.MainLoop();
 * }
 * @endcode
 *
 * If required, you can also connect class member functions to a signal:
 *
 * @code
 * MyApplication app;
 * app.ResumeSignal().Connect(&app, &MyApplication::Resume);
 * @endcode
 *
 * This class accepts command line arguments as well. The following options are supported:
 *
 * @code
 *     --no-vsync       Disable VSync on Render
 *  -w|--width          Stage Width
 *  -h|--height         Stage Height
 *  -d|--dpi            Emulated DPI
 *     --help           Help
 * @endcode
 *
 * When the above options are found, they are stripped from argv, and argc is updated appropriately.
 * @SINCE_1_0.0
 */
class DALI_ADAPTOR_API Application : public BaseHandle
{
public:

  typedef Signal< void (DeviceStatus::Battery::Status) > LowBatterySignalType; ///< Application device signal type @SINCE_1_2.62
  typedef Signal< void (DeviceStatus::Memory::Status) > LowMemorySignalType;   ///< Application device signal type @SINCE_1_2.62
  typedef Signal< void (Application&) > AppSignalType;  ///< Application lifecycle signal and system signal callback type @SINCE_1_0.0
  typedef Signal< void (Application&, void *) > AppControlSignalType; ///< Application control signal callback type @SINCE_1_0.0

  /**
   * @brief Enumeration for deciding whether a Dali application window is opaque or transparent.
   * @SINCE_1_0.0
   */
  enum WINDOW_MODE
  {
    OPAQUE = 0,       ///< The window will be opaque @SINCE_1_0.0
    TRANSPARENT = 1   ///< The window transparency will match the alpha value set in Dali::Stage::SetBackgroundcolor() @SINCE_1_0.0
  };

public:

  /**
   * @brief This is the constructor for applications without an argument list.
   * @SINCE_1_0.0
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   * @return A handle to the Application
   */
  static Application New();

  /**
   * @brief This is the constructor for applications.
   *
   * @SINCE_1_0.0
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @return A handle to the Application
   */
  static Application New( int* argc, char **argv[] );

  /**
   * @brief This is the constructor for applications with a name.
   *
   * @SINCE_1_0.0
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @param[in]      stylesheet  The path to user defined theme file
   * @return A handle to the Application
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
   */
  static Application New( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * @brief This is the constructor for applications with a name.
   *
   * @SINCE_1_0.0
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @param[in]      stylesheet  The path to user defined theme file
   * @param[in]      windowMode  A member of WINDOW_MODE
   * @return A handle to the Application
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
   */
  static Application New( int* argc, char **argv[], const std::string& stylesheet, WINDOW_MODE windowMode );

  /**
   * @brief This is the constructor for applications.
   *
   * @SINCE_1_2.60
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   * @param[in,out]  argc         A pointer to the number of arguments
   * @param[in,out]  argv         A pointer to the argument list
   * @param[in]      stylesheet   The path to user defined theme file
   * @param[in]      windowMode   A member of WINDOW_MODE
   * @param[in]      positionSize A position and a size of the window
   * @return A handle to the Application
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
   */
  static Application New( int* argc, char **argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize );

  /**
   * @brief Constructs an empty handle.
   * @SINCE_1_0.0
   */
  Application();

  /**
   * @brief Copy Constructor.
   * @SINCE_1_0.0
   * @param[in] application Handle to an object
   */
  Application( const Application& application );

  /**
   * @brief Assignment operator.
   * @SINCE_1_0.0
   * @param[in] application Handle to an object
   * @return A reference to this
   */
  Application& operator=( const Application& application );

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_1_0.0
   */
  ~Application();

public:
  /**
   * @brief This starts the application.
   *
   * Choosing this form of main loop indicates that the default
   * application configuration of APPLICATION_HANDLES_CONTEXT_LOSS is used. On platforms where
   * context loss can occur, the application is responsible for tearing down and re-loading UI.
   * The application should listen to Stage::ContextLostSignal and Stage::ContextRegainedSignal.
   * @SINCE_1_0.0
   */
  void MainLoop();

  /**
   * @brief This starts the application, and allows the app to choose a different configuration.
   *
   * If the application plans on using the ReplaceSurface or ReplaceWindow API, then this will
   * trigger context loss & regain.
   * The application should listen to Stage::ContextLostSignal and Stage::ContextRegainedSignal.
   * @SINCE_1_0.0
   * @param[in] configuration The context loss configuration
   */
  void MainLoop(Configuration::ContextLoss configuration);

  /**
   * @brief This lowers the application to bottom without actually quitting it.
   * @SINCE_1_0.0
   */
  void Lower();

  /**
   * @brief This quits the application.  Tizen applications should use Lower to improve re-start performance unless they need to Quit completely.
   * @SINCE_1_0.0
   */
  void Quit();

  /**
   * @brief Ensures that the function passed in is called from the main loop when it is idle.
   * @SINCE_1_0.0
   * @param[in] callback The function to call
   * @return @c true if added successfully, @c false otherwise
   *
   * @note Function must be called from main event thread only
   *
   * A callback of the following type may be used:
   * @code
   *   void MyFunction();
   * @endcode
   * This callback will be deleted once it is called.
   *
   * @note Ownership of the callback is passed onto this class.
   */
  bool AddIdle( CallbackBase* callback );

  /**
   * @brief Retrieves the window used by the Application class.
   *
   * The application writer can use the window to change indicator and orientation
   * properties.
   * @SINCE_1_0.0
   * @return A handle to the window
   */
  Window GetWindow();

  /**
   * @brief Replaces the current window.
   *
   * This will force context loss.
   * If you plan on using this API in your application, then you should configure
   * it to prevent discard behavior when handling the Init signal.
   * @SINCE_1_0.0
   * @param[in] windowPosition The position and size parameters of the new window
   * @param[in] name The name of the new window
   */
  void ReplaceWindow(PositionSize windowPosition, const std::string& name);

  /**
   * @brief Get path application resources are stored at
   *
   * @SINCE_1_2.2
   * @return the full path of the resources
   */
  static std::string GetResourcePath();

  /**
   * @brief This is used to get region information from device.
   *
   * @SINCE_1_2.62
   * @return Region information
   */
  std::string GetRegion() const;

  /**
   * @brief This is used to get language information from device.
   *
   * @SINCE_1_2.62
   * @return Language information
   */
  std::string GetLanguage() const;

public: // Stereoscopy

  /**
   * @brief Sets the viewing mode for the application.
   * @SINCE_1_0.0
   * @param[in] viewMode The new viewing mode
   */
  void SetViewMode( ViewMode viewMode );

  /**
   * @brief Gets the current viewing mode.
   * @SINCE_1_0.0
   * @return The current viewing mode
   */
  ViewMode GetViewMode() const;

  /**
   * @brief Sets the stereo base (eye separation) for Stereoscopic 3D.
   *
   * The stereo base is the distance in millimetres between the eyes. Typical values are
   * between 50mm and 70mm. The default value is 65mm.
   * @SINCE_1_0.0
   * @param[in] stereoBase The stereo base (eye separation) for Stereoscopic 3D
   */
  void SetStereoBase( float stereoBase );

  /**
   * @brief Gets the stereo base (eye separation) for Stereoscopic 3D.
   *
   * @SINCE_1_0.0
   * @return The stereo base (eye separation) for Stereoscopic 3D
   */
  float GetStereoBase() const;

public:  // Signals

  /**
   * @brief The user should connect to this signal to determine when they should initialize
   * their application.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& InitSignal();

  /**
   * @brief The user should connect to this signal to determine when they should terminate
   * their application.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& TerminateSignal();

  /**
   * @brief The user should connect to this signal if they need to perform any special
   * activities when the application is about to be paused.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& PauseSignal();

  /**
   * @brief The user should connect to this signal if they need to perform any special
   * activities when the application has resumed.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& ResumeSignal();

  /**
   * @brief This signal is sent when the system requires the user to reinitialize itself.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& ResetSignal();

  /**
   * @DEPRECATED_1_1.43 Use Window::ResizedSignal() instead.
   * @brief This signal is emitted when the window application rendering on is resized.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& ResizeSignal() DALI_DEPRECATED_API;

  /**
  * @brief This signal is emitted when another application sends a launch request to the application.
  *
  * When the application is launched, this signal is emitted after the main loop of the application starts up.
  * The passed parameter describes the launch request and contains the information about why the application is launched.
  * @SINCE_1_0.0
  * @return The signal to connect to
  */
  AppControlSignalType& AppControlSignal();

  /**
   * @brief This signal is emitted when the language is changed on the device.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& LanguageChangedSignal();

  /**
  * @brief This signal is emitted when the region of the device is changed.
  * @SINCE_1_0.0
  * @return The signal to connect to
  */
  AppSignalType& RegionChangedSignal();

  /**
  * @DEPRECATED_1_2.62 Use LowBatterySignal() instead.
  * @brief This signal is emitted when the battery level of the device is low.
  * @SINCE_1_0.0
  * @return The signal to connect to
  */
  AppSignalType& BatteryLowSignal() DALI_DEPRECATED_API;

  /**
  * @DEPRECATED_1_2.62 Use LowMemorySignal() instead.
  * @brief This signal is emitted when the memory level of the device is low.
  * @SINCE_1_0.0
  * @return The signal to connect to
  */
  AppSignalType& MemoryLowSignal() DALI_DEPRECATED_API;

  /**
   * @brief This signal is emitted when the battery level of the device is low.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  LowBatterySignalType& LowBatterySignal();

  /**
   * @brief This signal is emitted when the memory level of the device is low.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  LowMemorySignalType& LowMemorySignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor.
   * @SINCE_1_0.0
   */
  explicit DALI_INTERNAL Application(Internal::Adaptor::Application* application);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_APPLICATION_H
