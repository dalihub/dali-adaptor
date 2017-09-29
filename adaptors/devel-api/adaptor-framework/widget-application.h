#ifndef __DALI_WIDGET_APPLICATION_H__
#define __DALI_WIDGET_APPLICATION_H__

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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>
#include <dali/public-api/signals/callback.h>
#include <bundle.h>

// INTERNAL INCLUDES
#ifdef DALI_ADAPTOR_COMPILATION  // full path doesn't exist until adaptor is installed so we have to use relative
#include <device-status.h>
#else
#include <dali/public-api/adaptor-framework/device-status.h>
#endif

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class WidgetApplication;
}
}

class Window;

/**
 * @brief An WidgetApplication class object should be created by every widget application
 * that wishes to use Dali.
 *
 * It provides a means for initializing the
 * resources required by the Dali::Core.
 *
 * The WidgetApplication class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * WidgetApplication and create the Dali Widget object in the connected callback.
 *
 * WidgetApplications should follow the example below:
 *
 * @code
 * class ExampleController: public ConnectionTracker
 * {
 * public:
 *   ExampleController( WidgetApplication& widgetApplication )
 *   : mWidgetApplication( widgetApplication )
 *   {
 *     mWidgetApplication.InitSignal().Connect( this, &ExampleController::Create );
 *   }
 *
 *   void Create( WidgetApplication& widgetApplication )
 *   {
 *     Widget widget = Widget::New( WIDGET_ID );
 *     widget.CreateSignal( this, &ExampleController::WidgetCreate );
 *   }
 *
 *   void WidgetCreate( const std::string& id, bundle* content, Window window )
 *   {
 *     // Do Dali components...
 *   }
 *  ...
 * private:
 *   WidgetApplication& mWidgetApplication;
 * };
 *
 * int main (int argc, char **argv)
 * {
 *   WidgetApplication app = WidgetApplication::New(&argc, &argv);
 *   ExampleController example( app );
 *   app.MainLoop();
 * }
 * @endcode
 *
 * If required, you can also connect class member functions to a signal:
 *
 * @code
 * MyWidgetApplication app;
 * app.ResumeSignal().Connect(&app, &MyWidgetApplication::Resume);
 * @endcode
 *
 * @SINCE_1_2.62
 */
class DALI_IMPORT_API WidgetApplication : public BaseHandle
{
public:

  typedef Signal< void (WidgetApplication&) > AppSignalType;                   ///< Widget application lifecycle signal and system signal callback type
  typedef Signal< void (DeviceStatus::Battery::Status) > LowBatterySignalType; ///< Widget application device signal type
  typedef Signal< void (DeviceStatus::Memory::Status) > LowMemorySignalType;   ///< Widget application device signal type

public:

  /**
   * @brief This is the constructor for WidgetApplications with a name.
   *
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @param[in]      stylesheet  The path to user defined theme file
   * @return A handle to the WidgetApplication
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
   */
  static WidgetApplication New( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * @brief The default constructor.
   *
   */
  WidgetApplication();

  /**
   * @brief Copy Constructor.
   * @param[in] WidgetApplication Handle to an object
   */
  WidgetApplication( const WidgetApplication& widgetApplication );

  /**
   * @brief Assignment operator.
   * @param[in] WidgetApplication Handle to an object
   * @return A reference to this
   */
  WidgetApplication& operator=( const WidgetApplication& widgetApplication );

 /**
   * @brief Destructor
   *
   */
  ~WidgetApplication();

  /**
   * @brief This starts the application.
   */
  void MainLoop();

  /**
   * @brief This quits the application.  Tizen applications should use Lower to improve re-start performance unless they need to Quit completely.
   */
  void Quit();

  /**
   * @brief Get path application resources are stored at
   * @return the full path of the resources
   */
  static std::string GetResourcePath();

  /**
   * @brief this is used to get region information from device.
   *
   * @return region information
   */
  std::string GetRegion();

  /**
   * @brief this is used to get language information from device.
   *
   * @return language information
   */
  std::string GetLanguage();

public:  // Signals

  /**
   * @brief The user should connect to this signal to determine when they should initialize
   * their application.
   * @return The signal to connect to
   */
  AppSignalType& InitSignal();

  /**
   * @brief The user should connect to this signal to determine when they should terminate
   * their application.
   * @return The signal to connect to
   */
  AppSignalType& TerminateSignal();

  /**
   * @brief This signal is emitted when the language is changed on the device.
   * @return The signal to connect to
   */
  AppSignalType& LanguageChangedSignal();

  /**
  * @brief This signal is emitted when the region of the device is changed.
  * @return The signal to connect to
  */
  AppSignalType& RegionChangedSignal();

  /**
  * @brief This signal is emitted when the battery level of the device is low.
  * @return The signal to connect to
  */
  LowBatterySignalType& LowBatterySignal();

  /**
  * @brief This signal is emitted when the memory level of the device is low.
  * @return The signal to connect to
  */
  LowMemorySignalType& LowMemorySignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL WidgetApplication(Internal::Adaptor::WidgetApplication* widgetApplication);
  /// @endcond
};

} // namespace Dali

#endif // ___DALI_WIDGET_APPLICATION_H__
