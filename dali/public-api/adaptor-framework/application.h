#ifndef DALI_APPLICATION_H
#define DALI_APPLICATION_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/device-status.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
class ObjectRegistry;

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
} // namespace DALI_INTERNAL
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
 *
 * #### UI thread
 * There is the UI thread feature.
 * UI thread is an additional thread that an Application object creates. The thread is for UI events.
 *
 * When the UI thread feature is enabled, you can use the task signals(TaskInit, TaskTerminate, TaskAppControl, TaskLanguageChanged, TaskLowBattery, and TaskLowMemory).
 * The task signals are emitted on the main thread,
 * and the normal signals(Init, Terminate, Pause, Resume, Reset, AppControl, LanguageChanged, Region, LowBattery, and LowMemory) are emitted on the UI thread.
 *
 * If you want to handle windows or actors in cases like when the memory level of the device is low, you have to use the normal signals, not the task signals.
 * Callbacks of all signals in DALi except the task signals are emitted on the UI thread. (e.g., Timer callbacks are emitted on the UI thread.)
 *
 * To enable the UI Thread, you can use this method. you have to set True to the useUiThread.
 * Dali::Application::New(int *argc, char **argv[], const std::string &stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize, bool useUiThread)
 *
 *
 * This class accepts command line arguments as well. The following options are supported:
 *
 * @code
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
  typedef Signal<void(DeviceStatus::Battery::Status)>     LowBatterySignalType;               ///< Application device signal type @SINCE_1_2.62
  typedef Signal<void(DeviceStatus::Memory::Status)>      LowMemorySignalType;                ///< Application device signal type @SINCE_1_2.62
  typedef Signal<void(DeviceStatus::Orientation::Status)> DeviceOrientationChangedSignalType; ///< Application device orientation changed signal type @SINCE_2_2.1
  typedef Signal<void(Application&)>                      AppSignalType;                      ///< Application lifecycle signal and system signal callback type @SINCE_1_0.0
  typedef Signal<void(Application&, void*)>               AppControlSignalType;               ///< Application control signal callback type @SINCE_1_0.0

  /**
   * @brief Enumeration for deciding whether a Dali application window is opaque or transparent.
   * @SINCE_1_0.0
   */
  enum WINDOW_MODE
  {
    OPAQUE      = 0, ///< The window will be opaque @SINCE_1_0.0
    TRANSPARENT = 1  ///< The window transparency will match the alpha value set in Dali::Stage::SetBackgroundcolor() @SINCE_1_0.0
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
  static Application New(int* argc, char** argv[]);

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
  static Application New(int* argc, char** argv[], const std::string& stylesheet);

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
  static Application New(int* argc, char** argv[], const std::string& stylesheet, WINDOW_MODE windowMode);

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
  static Application New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize);

  /**
   * @brief This is the constructor for applications.
   *
   * @SINCE_2_1.20
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   * @param[in,out]  argc         A pointer to the number of arguments
   * @param[in,out]  argv         A pointer to the argument list
   * @param[in]      stylesheet   The path to user defined theme file
   * @param[in]      windowMode   A member of WINDOW_MODE
   * @param[in]      positionSize A position and a size of the window
   * @param[in]      useUiThread  True if the application would create a UI thread
   * @return A handle to the Application
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.<BR>
   * UI thread is an additional thread that DALi creates for UI events.
   * The UI thread isn't blocked from the system events(AppControl, LanguageChanged, RegionChanged, LowMemory, LowBattery task signals).
   */
  static Application New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize, bool useUiThread);

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
  Application(const Application& application);

  /**
   * @brief Assignment operator.
   * @SINCE_1_0.0
   * @param[in] application Handle to an object
   * @return A reference to this
   */
  Application& operator=(const Application& application);

  /**
   * @brief Move constructor.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   */
  Application(Application&& rhs);

  /**
   * @brief Move assignment operator.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  Application& operator=(Application&& rhs);

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
   * On platforms where context loss can occur, the application is responsible for tearing down and
   * re-loading UI.  The application should listen to Stage::ContextLostSignal and
   * Stage::ContextRegainedSignal.
   *
   * @SINCE_1_0.0
   */
  void MainLoop();

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
  bool AddIdle(CallbackBase* callback);

  /**
   * @brief Retrieves the main window used by the Application class.
   *
   * The application writer can use the window to change indicator and orientation
   * properties.
   * @SINCE_1_0.0
   * @return A handle to the window
   */
  Window GetWindow();

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

  /**
   * @brief Gets the Object registry.
   *
   * @SINCE_1_9.21
   * @return The object registry
   * @note This will only be a valid handle after the InitSignal has been emitted.
   */
  ObjectRegistry GetObjectRegistry() const;

public: // Signals
  /**
   * @brief The user should connect to this signal to determine when they should initialize
   * their application.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& InitSignal();

  /**
   * @brief The user should connect to this signal to determine when they should terminate
   * their application.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& TerminateSignal();

  /**
   * @brief The user should connect to this signal if they need to perform any special
   * activities when the application is about to be paused.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& PauseSignal();

  /**
   * @brief The user should connect to this signal if they need to perform any special
   * activities when the application has resumed.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& ResumeSignal();

  /**
   * @brief This signal is sent when the system requires the user to reinitialize itself.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& ResetSignal();

  /**
   * @brief This signal is emitted when another application sends a launch request to the application.
   *
   * When the application is launched, this signal is emitted after the main loop of the application starts up.
   * The passed parameter describes the launch request and contains the information about why the application is launched.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppControlSignalType& AppControlSignal();

  /**
   * @brief This signal is emitted when the language is changed on the device.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& LanguageChangedSignal();

  /**
   * @brief This signal is emitted when the region of the device is changed.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  AppSignalType& RegionChangedSignal();

  /**
   * @brief This signal is emitted when the battery level of the device is low.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  LowBatterySignalType& LowBatterySignal();

  /**
   * @brief This signal is emitted when the memory level of the device is low.
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  LowMemorySignalType& LowMemorySignal();

  /**
   * @brief This signal is emitted when the device orientation is changed
   * Only when the user uses the UI thread, this signal is emitted on the UI thread.
   * Otherwise, it is emitted on the main thread.
   *
   * This signal is only used in Application, it is different to Window's orientation signal.
   * @SINCE_2_2.1
   * @return The signal to connect to
   */
  DeviceOrientationChangedSignalType& DeviceOrientationChangedSignal();

  // TaskSignal
  /**
   * @brief The user should connect to this signal to determine when they should initialize
   * their application.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  AppSignalType& TaskInitSignal();

  /**
   * @brief The user should connect to this signal to determine when they should terminate
   * their application.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  AppSignalType& TaskTerminateSignal();

  /**
   * @brief This signal is emitted when another application sends a launch request to the application.
   *
   * When the application is launched, this signal is emitted after the main loop of the application starts up.
   * The passed parameter describes the launch request and contains the information about why the application is launched.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  AppControlSignalType& TaskAppControlSignal();

  /**
   * @brief This signal is emitted when the language is changed on the device.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  AppSignalType& TaskLanguageChangedSignal();

  /**
   * @brief This signal is emitted when the region of the device is changed.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  AppSignalType& TaskRegionChangedSignal();

  /**
   * @brief This signal is emitted when the battery level of the device is low.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  LowBatterySignalType& TaskLowBatterySignal();

  /**
   * @brief This signal is emitted when the memory level of the device is low.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @return The signal to connect to
   */
  LowMemorySignalType& TaskLowMemorySignal();

  /**
   * @brief This signal is emitted when the device orientation is changed.
   * Only when the user uses the UI thread, this signal is emitted on the main thread.
   * Otherwise, it is not emitted at all.
   * @SINCE_2_2.1
   * @return The signal to connect to
   */
  DeviceOrientationChangedSignalType& TaskDeviceOrientationChangedSignal();

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
