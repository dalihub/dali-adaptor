#ifndef DALI_INTERNAL_ADAPTOR_COMMON_FRAMEWORK_H
#define DALI_INTERNAL_ADAPTOR_COMMON_FRAMEWORK_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/callback.h>
#include <string>
#ifdef APPCORE_WATCH_AVAILABLE
#include <dali/public-api/watch/watch-application.h>
#endif

// INTERNAL INCLUDES
#include <dali/internal/system/common/abort-handler.h>
#include <dali/public-api/adaptor-framework/device-status.h>

#ifdef COMPONENT_APPLICATION_SUPPORT
#include <dali/devel-api/adaptor-framework/component-application.h>
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * The Framework class is ideally placed to provide key API required by Applications.
 *
 * The class is also used to register callbacks with the TIZEN platform so that
 * we know when any of the application lifecycle events occur.  This includes events
 * like when our application is to be initialised, terminated, paused, resumed etc.
 */
class Framework
{
public:
  enum Type
  {
    NORMAL,   ///< normal appFramework
    WATCH,    ///< watch appFramework
    WIDGET,   ///< widget appFramework
    COMPONENT ///< component appFramework
  };

  /**
   * Observer class for the framework.
   * @brief When the UI thread is enabled, the events are emitted on the UI thread.
   * When it is disabled, the events are emitted on the main thread.
   */
  class Observer
  {
  public:
    /**
     * Invoked when the application is to be initialised.
     */
    virtual void OnInit()
    {
    }

    /**
     * Invoked when the application is to be terminated.
     */
    virtual void OnTerminate()
    {
    }

    /**
     * Invoked when the application is to be paused.
     */
    virtual void OnPause()
    {
    }

    /**
     * Invoked when the application is to be resumed.
     */
    virtual void OnResume()
    {
    }

    /**
     * Invoked when the application is to be reset.
     */
    virtual void OnReset()
    {
    }

    /**
     * Invoked when the AppControl message is received.
     * @param[in] The bundle data of AppControl message.
     */
    virtual void OnAppControl(void*)
    {
    }

#ifdef APPCORE_WATCH_AVAILABLE
    /**
     * Invoked at every second
     */
    virtual void OnTimeTick(WatchTime&)
    {
    }

    /**
     * Invoked at every second in ambient mode
     */
    virtual void OnAmbientTick(WatchTime&)
    {
    }

    /**
     * Invoked when the device enters or exits ambient mode
     */
    virtual void OnAmbientChanged(bool ambient)
    {
    }
#endif

    /**
     * Invoked when the language of the device is changed.
     */
    virtual void OnLanguageChanged()
    {
    }

    /**
     * Invoked when the region is changed.
     */
    virtual void OnRegionChanged()
    {
    }

    /**
     * Invoked when the battery level of the device is low.
     */
    virtual void OnBatteryLow(Dali::DeviceStatus::Battery::Status status)
    {
    }

    /**
     * Invoked when the memory level of the device is low.
     */
    virtual void OnMemoryLow(Dali::DeviceStatus::Memory::Status status)
    {
    }

    /**
     * Invoked when the device orientation is changed.
     */
    virtual void OnDeviceOrientationChanged(Dali::DeviceStatus::Orientation::Status status)
    {
    }

    /**
     * Invoked when the platform surface is created.
     */
    virtual void OnSurfaceCreated(Any newSurface)
    {
    }

    /**
     * Invoked when the platform surface is destroyed.
     */
    virtual void OnSurfaceDestroyed(Any newSurface)
    {
    }

#ifdef COMPONENT_APPLICATION_SUPPORT
    /**
     * Invoked when the component application is created.
     */
    virtual Any OnCreate()
    {
      return nullptr;
    }
#endif
  };

  /**
   * TaskObserver class for the framework.
   * @brief This is used only when UiThread is enabled. the events are emitted on the main thread.
   */
  class TaskObserver
  {
  public:
    /**
     * Invoked when the application is to be initialised.
     */
    virtual void OnTaskInit()
    {
    }

    /**
     * Invoked when the application is to be terminated.
     */
    virtual void OnTaskTerminate()
    {
    }

    /**
     * Invoked when the AppControl message is received.
     * @param[in] The bundle data of AppControl message.
     */
    virtual void OnTaskAppControl(void*)
    {
    }

    /**
     * Invoked when the language of the device is changed.
     */
    virtual void OnTaskLanguageChanged()
    {
    }

    /**
     * Invoked when the region is changed.
     */
    virtual void OnTaskRegionChanged()
    {
    }

    /**
     * Invoked when the battery level of the device is low.
     */
    virtual void OnTaskBatteryLow(Dali::DeviceStatus::Battery::Status status)
    {
    }

    /**
     * Invoked when the memory level of the device is low.
     */
    virtual void OnTaskMemoryLow(Dali::DeviceStatus::Memory::Status status)
    {
    }

    /**
     * Invoked when the device orientation is changed.
     *
     * Device orientation changed event is from Application Framework(Sensor Framework), it means it is system event.
     * If UIThreading is enable, DALI application has the main thread and UI thread.
     * This event is emitted in main thread, then it is posted to the UI thread in this callback function.
     */
    virtual void OnTaskDeviceOrientationChanged(Dali::DeviceStatus::Orientation::Status status)
    {
    }
  };

public:
  /**
   * Constructor
   * @param[in]  observer      The observer of the Framework.
   * @param[in]  taskObserver  The task observer of the Framework.
   * @param[in]  argc          A pointer to the number of arguments.
   * @param[in]  argv          A pointer the the argument list.
   * @param[in]  type          The type of application
   * @param[in]  useUiThread   True if the application would create a UI thread
   */
  Framework(Observer& observer, TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread);

  /**
   * Destructor
   */
  virtual ~Framework();

public:
  /**
   * Runs the main loop of framework
   */
  virtual void Run() = 0;

  /**
   * Quits the main loop
   */
  virtual void Quit() = 0;

  /**
   * Gets system language.
   */
  virtual std::string GetLanguage() const;

  /**
   * Gets system region.
   */
  virtual std::string GetRegion() const;

  /**
   * Checks whether the main loop of the framework is running.
   * @return true, if the main loop is running, false otherwise.
   */
  bool IsMainLoopRunning();

  /**
   * If the main loop aborts unexpectedly, then the connected callback function is called.
   * @param[in]  callBack  The function to call.
   * @note Only one callback can be registered.  The last callback to be set will be called on abort.
   * @note The ownership of callback is passed onto this class.
   */
  void AddAbortCallback(CallbackBase* callback);

  /**
   * Sets a command line options.
   * This is used in case of the preinitialized application.
   * @param[in] argc A pointer to the number of arguments
   * @param[in] argv A pointer to the argument list
   */
  void SetCommandLineOptions(int* argc, char** argv[])
  {
    mArgc = argc;
    mArgv = argv;
  }

private:
  /**
   * Called if the application is aborted.
   */
  void AbortCallback();

private:
  // Undefined
  Framework(const Framework&);
  Framework& operator=(Framework&);

protected:
  Observer&                     mObserver;
  TaskObserver&                 mTaskObserver;
  AbortHandler                  mAbortHandler;
  int*                          mArgc;
  char***                       mArgv;
  std::unique_ptr<CallbackBase> mAbortCallBack;
  bool                          mRunning;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_COMMON_FRAMEWORK_H
