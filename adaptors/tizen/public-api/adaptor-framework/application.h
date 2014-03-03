#ifndef __DALI_APPLICATION_H__
#define __DALI_APPLICATION_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// EXTERNAL INCLUDES
#include <boost/function.hpp>
#include <string>
#include <dali/public-api/object/base-handle.h>

#include <dali/public-api/adaptor-framework/common/style-monitor.h>
#include <dali/public-api/adaptor-framework/common/device-layout.h>
#include <dali/public-api/adaptor-framework/common/window.h>

namespace Dali DALI_IMPORT_API
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Application;
}
}

/**
 * An Application class object should be created by every application
 * that wishes to use Dali.  It provides a means for initialising the
 * resources required by the Dali::Core.
 *
 * The Application class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * Application and create the Dali objects in the connected callback.
 *
 * Linux Applications should follow the example below:
 *
 * @code
 * void CreateProgram(Application app)
 * {
 *   // Create Dali components...
 *   // Can instantiate here, if required
 * }
 *
 * int main (int argc, char **argv)
 * {
 *   Application app = Application::New(&argc, &argv);
 *   app.InitSignal().Connect(&CreateProgram);
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
 */
class Application : public BaseHandle
{
public:

  typedef SignalV2< void (Application&) > AppSignalV2;

public:

  /**
   * This is the constructor for Linux & SLP applications.
   * @param  argc        A pointer to the number of arguments
   * @param  argv        A pointer the the argument list
   * @note The default base layout (DeviceLayout::DEFAULT_BASE_LAYOUT) will be used with this constructor.
   */
  static Application New( int* argc, char **argv[] );

  /**
   * This is the constructor for Linux & SLP applications with a name
   * @param  argc  A pointer to the number of arguments
   * @param  argv  A pointer the the argument list
   * @param  name  A name of application
   * @note The default base layout (DeviceLayout::DEFAULT_BASE_LAYOUT) will be used with this constructor.
   */
  static Application New( int* argc, char **argv[], const std::string& name );

  /**
   * This is the constructor for Linux & SLP applications when a layout for the application is specified.
   * @param  argc        A pointer to the number of arguments
   * @param  argv        A pointer the the argument list
   * @param  baseLayout  The base layout that the application has been written for
   */
  static Application New( int* argc, char **argv[], const DeviceLayout& baseLayout );

  /**
   * This is the constructor for Linux & SLP applications with a name and when a layout for the application is specified.
   * @param  argc  A pointer to the number of arguments
   * @param  argv  A pointer the the argument list
   * @param  name  A name of application
   * @param  baseLayout  The base layout that the application has been written for
   */
  static Application New( int* argc, char **argv[], const std::string& name, const DeviceLayout& baseLayout );

  /**
   * Construct an empty handle
   */
  Application();

  /**
   * Copy Constructor
   */
  Application( const Application& application );

  /**
   * Assignment operator
   */
  Application& operator=( const Application& applicaton );

  /**
   * Virtual destructor
   */
  virtual ~Application();

public:
  /**
   * This starts the application.
   */
  void MainLoop();

  /**
   * This lowers the application to bottom without actually quitting it
   */
  void Lower();

  /**
   * This quits the application.  Tizen applications should use Lower to improve re-start performance unless they need to Quit completely.
   */
  void Quit();

  /**
   * This returns a handle to the Orientation object used by Application which allows
   * the user to determine the orientation of the device and connect to a
   * signal emitted whenever the orientation changes.
   * @return A handle to the Orientation object used by the Application
   */
  Orientation GetOrientation();

  /**
   * Ensures that the function passed in is called from the main loop when it is idle.
   *
   * A callback of the following type may be used:
   * @code
   *   void MyFunction();
   * @endcode
   *
   * @param[in]  callBack  The function to call.
   * @return true if added successfully, false otherwise
   */
  bool AddIdle(boost::function<void(void)> callBack);

  /**
   * Retrieves the window used by the Application class.
   * The application writer can use the window to change indicator and orientation
   * properties.
   * @return A handle to the window
   */
  Window GetWindow();

  /**
   * Returns the local thread's instance of the Application class.
   * @return A reference to the local thread's Application class instance.
   * @pre The Application class has been initialised.
   * @note This is only valid in the main thread.
   */
  static Application Get();

public:  // Signals

  /**
   * The user should connect to this signal to determine when they should initialise
   * their application
   */
  AppSignalV2& InitSignal();

  /**
   * The user should connect to this signal to determine when they should terminate
   * their application
   */
  AppSignalV2& TerminateSignal();

  /**
   * The user should connect to this signal if they need to perform any special
   * activities when the application is about to be paused.
   */
  AppSignalV2& PauseSignal();

  /**
   * The user should connect to this signal if they need to perform any special
   * activities when the application has resumed.
   */
  AppSignalV2& ResumeSignal();

  /**
   * This signal is sent when the system requires the user to reinitialise itself.
   */
  AppSignalV2& ResetSignal();

  /**
   * This signal is emitted when the window the application is rendering on is resized.
   */
  AppSignalV2& ResizeSignal();

  /**
   * This signal is emitted when the language is changed on the device.
   */
  AppSignalV2& LanguageChangedSignal();

public: // Not intended for application developers
  /**
   * Internal constructor
   */
  explicit DALI_INTERNAL Application(Internal::Adaptor::Application* application);
};

} // namespace Dali

#endif // __DALI_APPLICATION_H__
