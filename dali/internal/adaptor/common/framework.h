#ifndef DALI_INTERNAL_FRAMEWORK_H
#define DALI_INTERNAL_FRAMEWORK_H

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
#include <dali/public-api/signals/callback.h>
#ifdef APPCORE_WATCH_AVAILABLE
#include <dali/public-api/watch/watch-application.h>
#endif

// INTERNAL INCLUDES
#include <dali/internal/system/common/abort-handler.h>
#include <dali/public-api/adaptor-framework/device-status.h>

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
    NORMAL,       ///< normal appFramework
    WATCH,        ///< watch appFramework
    WIDGET        ///< widget appFramework
  };

  /**
   * Observer class for the framework.
   */
  class Observer
  {
  public:

    /**
     * Invoked when the application is to be initialised.
     */
    virtual void OnInit() {}

    /**
     * Invoked when the application is to be terminated.
     */
    virtual void OnTerminate() {}

    /**
     * Invoked when the application is to be paused.
     */
    virtual void OnPause() {}

    /**
     * Invoked when the application is to be resumed.
     */
    virtual void OnResume() {}

    /**
     * Invoked when the application is to be reset.
     */
    virtual void OnReset() {}

    /**
    * Invoked when the AppControl message is received.
    * @param[in] The bundle data of AppControl message.
    */
    virtual void OnAppControl(void *) {}

#ifdef APPCORE_WATCH_AVAILABLE
    /**
     * Invoked at every second
     */
    virtual void OnTimeTick(WatchTime&) {}

    /**
     * Invoked at every second in ambient mode
     */
    virtual void OnAmbientTick(WatchTime&) {}

    /**
     * Invoked when the device enters or exits ambient mode
     */
    virtual void OnAmbientChanged(bool ambient) {}
#endif

    /**
     * Invoked when the language of the device is changed.
     */
    virtual void OnLanguageChanged() {}

    /**
    * Invoked when the region is changed.
    */
    virtual void OnRegionChanged() {}

    /**
    * Invoked when the battery level of the device is low.
    */
    virtual void OnBatteryLow( Dali::DeviceStatus::Battery::Status status ) {}

    /**
    * Invoked when the memory level of the device is low.
    */
    virtual void OnMemoryLow( Dali::DeviceStatus::Memory::Status status ) {}
  };

public:

  /**
   * Constructor
   * @param[in]  observer  The observer of the Framework.
   * @param[in]  argc      A pointer to the number of arguments.
   * @param[in]  argv      A pointer the the argument list.
   * @param[in]  type      The type of application
   */
  Framework( Observer& observer, int* argc, char ***argv, Type type = NORMAL );

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
  void AddAbortCallback( CallbackBase* callback );

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
  void SetCommandLineOptions( int* argc, char **argv[] )
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
  void SetLanguage( const std::string& language );

  /**
   * Sets system region.
   */
  void SetRegion( const std::string& region );

  /**
   * Gets system language.
   */
  std::string GetLanguage() const;

  /**
   * Gets system region.
   */
  std::string GetRegion() const;

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
   * Called by the App framework when an application lifecycle event occurs.
   * @param[in] type The type of event occurred.
   * @param[in] bundleData The bundle data of event occurred.
   */
  bool AppStatusHandler(int type, void *bundleData);

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
  Observer&          mObserver;
  bool               mInitialised;
  bool               mRunning;
  int*               mArgc;
  char***            mArgv;
  std::string        mBundleName;
  std::string        mBundleId;
  AbortHandler       mAbortHandler;

private: // impl members

  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_FRAMEWORK_H
