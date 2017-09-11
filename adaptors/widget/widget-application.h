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

class DALI_IMPORT_API WidgetApplication : public BaseHandle
{
public:

  typedef Signal< void (WidgetApplication&) > AppSignalType;  ///< Application lifecycle signal and system signal callback type

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
   * @brief Get window.
   * @return the window for widget instance.
   */
  Window GetWindow();

  /**
   * @brief Get path application resources are stored at
   * @return the full path of the resources
   */

  static std::string GetResourcePath();

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
  AppSignalType& BatteryLowSignal();

  /**
  * @brief This signal is emitted when the memory level of the device is low.
  * @return The signal to connect to
  */
  AppSignalType& MemoryLowSignal();

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
