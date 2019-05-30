#ifndef DALI_WATCH_APPLICATION_H
#define DALI_WATCH_APPLICATION_H

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
#include <string>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/adaptor-framework/application.h>
#include <dali/public-api/watch/watch-time.h>

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
class WatchApplication;
}
}

/**
 * @brief A WatchApplication class object should be created by every watch application
 * that wishes to use Dali.
 *
 * It provides a means for initialising the resources required by the Dali::Core.
 * Like Application class, the WatchApplication class manages Tizen watch application life cycle.
 *
 * The WatchApplication class emits additional signals which are availalble only in the watch application
 * (TimeTick, AmbientTick, AmbientChanged)
 *
 * This feature is supported in wearable applications only.
 *
 * WatchApplication should follow the example below:
 *
 * @code
 * class ExampleController: public ConnectionTracker
 * {
 * public:
 *   ExampleController( WatchApplication& application )
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
 *   WatchApplication&  mApplication;
 * };
 *
 * int DALI_ADAPTOR_API main (int argc, char **argv)
 * {
 *   WatchApplication app = WatchApplication::New(&argc, &argv);
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
 * When the above options are found, they are stripped from argv, and argc is updated appropriately.
 * @SINCE_1_1.37
 */

class DALI_ADAPTOR_API WatchApplication : public Application
{
public:
  typedef Signal< void (Application&, const WatchTime&) > WatchTimeSignal; ///< Watch pointer signal callback type @SINCE_1_1.37
  typedef Signal< void (Application&, bool) > WatchBoolSignal; ///< Watch bool signal callback type @SINCE_1_1.37

public:

  /**
   * @brief This is the constructor for applications without an argument list.
   * @SINCE_1_1.37
   * @return A handle to the WatchApplication
   */
  static WatchApplication New();

  /**
   * @brief This is the constructor for applications.
   *
   * @SINCE_1_1.37
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer the the argument list
   * @return A handle to the WatchApplication
   */
  static WatchApplication New( int* argc, char **argv[] );

  /**
   * @brief This is the constructor for applications with a name
   *
   * @SINCE_1_1.37
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer the the argument list
   * @param[in]      stylesheet  The path to user defined theme file
   * @return A handle to the WatchApplication
   */
  static WatchApplication New( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * @brief Construct an empty handle
   * @SINCE_1_1.37
   */
  WatchApplication();

  /**
   * @brief Copy Constructor
   * @SINCE_1_1.37
   * @param[in] implementation The WatchApplication implementation
   */
  WatchApplication( const WatchApplication& implementation );

  /**
   * @brief Assignment operator
   * @SINCE_1_1.37
   * @param[in] application Handle to an object
   * @return A reference to this
   */
  WatchApplication& operator=( const WatchApplication& application );

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_1_1.37
   */
  ~WatchApplication();

public:
  /**
   * @brief This signal is emitted at every second
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Application& application, const WatchTime& time);
   * @endcode
   * time(watch time handle) will not be available after returning this callback. It will be freed by the framework.
   * @SINCE_1_1.37
   * @return The signal to connect to
   */
  WatchTimeSignal& TimeTickSignal();

  /**
   * @brief This signal is emitted at each minute in ambient mode
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Application& application, const WatchTime& time);
   * @endcode
   * time(watch time handle) will not be available after returning this callback. It will be freed by the framework.
   * @SINCE_1_1.37
   * @remarks http://tizen.org/privilege/alarm.set privilege is needed to receive ambient ticks at each minute.
   * The AmbientTickSignal() will be ignored if your app doesn't have the privilege
   * @return The signal to connect to
   */
  WatchTimeSignal& AmbientTickSignal();

  /**
   * @brief This signal is emitted when the device enters or exits ambient mode
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Application& application, bool ambient);
   * @endcode
   * ambient_mode If true the device enters the ambient mode, otherwise false
   * @SINCE_1_1.37
   * @return The signal to connect to
   */
  WatchBoolSignal& AmbientChangedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor
   * @SINCE_1_1.37
   */
  explicit DALI_INTERNAL WatchApplication(Internal::Adaptor::WatchApplication* implementation);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_WATCH_APPLICATION_H
