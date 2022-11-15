#ifndef DALI_INTERNAL_FRAMEWORK_H
#define DALI_INTERNAL_FRAMEWORK_H

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
 *
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
  ~Framework();

public:
  /**
   * Runs the main loop of framework
   */
  void Run();

  /**
   * Quits the main loop
   */
  void Quit();

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
   * Gets bundle name which was passed in app_reset callback.
   */
  std::string GetBundleName() const;

  /**
   * Gets bundle id which was passed in app_reset callback.
   */
  std::string GetBundleId() const;

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

  /**
   *  Gets the path at which application resources are stored.
   */
  static std::string GetResourcePath();

  /**
   *  Gets the path at which application data are stored.
   */
  static std::string GetDataPath();

  /**
   * Sets system language.
   */
  void SetLanguage(const std::string& language);

  /**
   * Sets system region.
   */
  void SetRegion(const std::string& region);

  /**
   * Gets system language.
   */
  std::string GetLanguage() const;

  /**
   * Gets system region.
   */
  std::string GetRegion() const;

  /**
   * Called by the App framework when an application lifecycle event occurs.
   * @param[in] type The type of event occurred.
   * @param[in] data The data of event occurred.
   */
  bool AppStatusHandler(int type, void* data);

  /**
   * Called by the adaptor when an idle callback is added.
   * @param[in] timeout The timeout of the callback.
   * @param[in] data The data of of the callback.
   * @param[in] callback The callback.
   * @return The callback id.
   */
  unsigned int AddIdle(int timeout, void* data, bool (*callback)(void* data));

  /**
   * Called by the adaptor when an idle callback is removed.
   * @param[in] id The callback id.
   */
  void RemoveIdle(unsigned int id);

private:
  // Undefined
  Framework(const Framework&);
  Framework& operator=(Framework&);

private:
  /**
   * Called when the application is created.
   */
  bool Create();

  /**
   * Called app_reset callback was called with bundle.
   */
  void SetBundleName(const std::string& name);

  /**
   * Called app_reset callback was called with bundle.
   */
  void SetBundleId(const std::string& id);

  /**
   * Called if the application is aborted.
   */
  void AbortCallback();

  /**
   * Called for initializing on specified backend. (X11 or Wayland)
   */
  void InitThreads();

private:
  Observer&     mObserver;
  TaskObserver& mTaskObserver;
  bool          mInitialised;
  bool          mPaused;
  bool          mRunning;
  int*          mArgc;
  char***       mArgv;
  std::string   mBundleName;
  std::string   mBundleId;
  AbortHandler  mAbortHandler;

private: // impl members
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_FRAMEWORK_H
