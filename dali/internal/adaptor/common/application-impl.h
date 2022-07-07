#ifndef DALI_INTERNAL_APPLICATION_H
#define DALI_INTERNAL_APPLICATION_H

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
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/public-api/adaptor-framework/application.h>

#include <dali/internal/adaptor/common/adaptor-builder-impl.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/system/common/environment-options.h>

namespace Dali
{
class Adaptor;
class Window;

namespace Internal
{
namespace Adaptor
{
namespace Launchpad
{
/**
 * @brief Launchpad is used to improve application launch performance.
 * When an application is pre-initialized, so files are preloaded, some functions are initialized and a window is made in advance.
 */
enum State
{
  NONE,           ///< The default state
  PRE_INITIALIZED ///< Application is pre-initialized.
};

} // namespace Launchpad

class CommandLineOptions;
class EventLoop;

typedef Dali::Rect<int> PositionSize;

class Application;
typedef IntrusivePtr<Application> ApplicationPtr;

/**
 * Implementation of the Application class.
 */
class Application : public BaseObject, public Framework::Observer, public Framework::TaskObserver
{
public:
  typedef Dali::Application::LowBatterySignalType LowBatterySignalType;
  typedef Dali::Application::LowMemorySignalType  LowMemorySignalType;
  typedef Dali::Application::AppSignalType        AppSignalType;
  typedef Dali::Application::AppControlSignalType AppControlSignalType;
  typedef Dali::Application::WINDOW_MODE          WINDOW_MODE;

  /**
   * Create a new application
   * @param[in]  argc              A pointer to the number of arguments
   * @param[in]  argv              A pointer to the argument list
   * @param[in]  stylesheet        The path to user defined theme file
   * @param[in]  windowMode        A member of Dali::Application::WINDOW_MODE
   * @param[in]  positionSize      A position and a size of the window
   * @param[in]  applicationType   A member of Dali::Framework::Type
   * @param[in]  type              It is window type for default window.
   * @param[in]  useUiThread       True if the application would create a UI thread
   */
  static ApplicationPtr New(int* argc, char** argv[], const std::string& stylesheet, WINDOW_MODE windowMode, const PositionSize& positionSize, Framework::Type applicationType, WindowType type, bool useUiThread);

  /**
   * @copydoc Dali::DevelApplication::PreInitialize()
   */
  static void PreInitialize(int* argc, char** argv[]);

public:
  /**
   * @copydoc Dali::Application::MainLoop()
   */
  void MainLoop();

  /**
   * @copydoc Dali::Application::Lower()
   */
  void Lower();

  /**
   * @copydoc Dali::Application::Quit()
   */
  void Quit();

  /**
   * @copydoc Dali::Application::AddIdle()
   */
  bool AddIdle(CallbackBase* callback, bool hasReturnValue);

  /**
   * @copydoc Dali::Application::GetAdaptor();
   */
  Dali::Adaptor& GetAdaptor();

  /**
   * @copydoc Dali::Application::GetWindow();
   */
  Dali::Window GetWindow();

  /**
   * @copydoc Dali::Application::GetRegion();
   */
  std::string GetRegion() const;

  /**
   * @copydoc Dali::Application::GetLanguage();
   */
  std::string GetLanguage() const;

  /**
   * @copydoc Dali::Application::GetObjectRegistry();
   */
  Dali::ObjectRegistry GetObjectRegistry() const;

  /**
   * @copydoc Dali::Application::GetResourcePath();
   */
  static std::string GetResourcePath();

  /**
   * @copydoc Dali::DevelApplication::GetDataPath()
   */
  static std::string GetDataPath();

  /**
   * Retrieves the pre-initialized application.
   *
   * @return A pointer to the pre-initialized application
   */
  static ApplicationPtr GetPreInitializedApplication();

  /**
   * Stores PositionSize of window
   */
  void StoreWindowPositionSize(PositionSize positionSize);

public: // From Framework::Observer
  /**
   * Called when the framework is initialised.
   */
  void OnInit() override;

  /**
   * Called when the framework is terminated.
   */
  void OnTerminate() override;

  /**
   * Called when the framework is paused.
   */
  void OnPause() override;

  /**
   * Called when the framework resumes from a paused state.
   */
  void OnResume() override;

  /**
   * Called when the framework received AppControlSignal.
   * @param[in] The bundle data of AppControl event.
   */
  void OnAppControl(void* data) override;

  /**
   * Called when the framework informs the application that it should reset itself.
   */
  void OnReset() override;

  /**
   * Called when the framework informs the application that the language of the device has changed.
   */
  void OnLanguageChanged() override;

  /**
   * Called when the framework informs the application that the region of the device has changed.
   */
  void OnRegionChanged() override;

  /**
   * Called when the framework informs the application that the battery level of the device is low.
   */
  void OnBatteryLow(Dali::DeviceStatus::Battery::Status status) override;

  /**
   * Called when the framework informs the application that the memory level of the device is low.
   */
  void OnMemoryLow(Dali::DeviceStatus::Memory::Status status) override;

  /**
   * Called when the framework informs the application that the platform surface is created.
   */
  void OnSurfaceCreated(Any newSurface) override;

  /**
   * Called when the framework informs the application that the platform surface is destroyed.
   */
  void OnSurfaceDestroyed(Any newSurface) override;

public: // From Framework::TaskObserver
  /**
   * Called when the framework is initialised.
   */
  void OnTaskInit() override;

  /**
   * Called when the framework is terminated.
   */
  void OnTaskTerminate() override;

  /**
   * Called when the framework received AppControlSignal.
   * @param[in] The bundle data of AppControl event.
   */
  void OnTaskAppControl(void* data) override;

  /**
   * Called when the framework informs the application that the language of the device has changed.
   */
  void OnTaskLanguageChanged() override;

  /**
   * Called when the framework informs the application that the region of the device has changed.
   */
  void OnTaskRegionChanged() override;

  /**
   * Called when the framework informs the application that the battery level of the device is low.
   */
  void OnTaskBatteryLow(Dali::DeviceStatus::Battery::Status status) override;

  /**
   * Called when the framework informs the application that the memory level of the device is low.
   */
  void OnTaskMemoryLow(Dali::DeviceStatus::Memory::Status status) override;

public:
  /**
   * Sets a user defined theme file.
   * This should be called before initialization.
   * @param[in] stylesheet The path to user defined theme file
   */
  void SetStyleSheet(const std::string& stylesheet);

  /**
   * Sets a command line options.
   * This is used in case of the preinitialized application.
   * @param[in] argc A pointer to the number of arguments
   * @param[in] argv A pointer to the argument list
   */
  void SetCommandLineOptions(int* argc, char** argv[]);

  /**
   * Sets default window type.
   * This is used in case of the preinitialized application.
   * @param[in] type the window type for default window
   */
  void SetDefaultWindowType(WindowType type);

public: // Signals
  /**
   * @copydoc Dali::Application::InitSignal()
   */
  Dali::Application::AppSignalType& InitSignal()
  {
    return mInitSignal;
  }

  /**
   * @copydoc Dali::Application::TerminateSignal()
   */
  Dali::Application::AppSignalType& TerminateSignal()
  {
    return mTerminateSignal;
  }

  /**
   * @copydoc Dali::Application::PauseSignal()
   */
  Dali::Application::AppSignalType& PauseSignal()
  {
    return mPauseSignal;
  }

  /**
   * @copydoc Dali::Application::ResumeSignal()
   */
  Dali::Application::AppSignalType& ResumeSignal()
  {
    return mResumeSignal;
  }

  /**
   * @copydoc Dali::Application::ResetSignal()
   */
  Dali::Application::AppSignalType& ResetSignal()
  {
    return mResetSignal;
  }

  /**
   * @copydoc Dali::Application::AppControlSignal()
   */
  Dali::Application::AppControlSignalType& AppControlSignal()
  {
    return mAppControlSignal;
  }

  /**
   * @copydoc Dali::Application::LanguageChangedSignal()
   */
  Dali::Application::AppSignalType& LanguageChangedSignal()
  {
    return mLanguageChangedSignal;
  }

  /**
   * @copydoc Dali::Application::RegionChangedSignal()
   */
  Dali::Application::AppSignalType& RegionChangedSignal()
  {
    return mRegionChangedSignal;
  }

  /**
   * @copydoc Dali::Application::LowBatterySignal()
   */
  Dali::Application::LowBatterySignalType& LowBatterySignal()
  {
    return mLowBatterySignal;
  }

  /**
   * @copydoc Dali::Application:::LowMemorySignal()
   */
  Dali::Application::LowMemorySignalType& LowMemorySignal()
  {
    return mLowMemorySignal;
  }

  /**
   * @copydoc Dali::Application::TaskInitSignal()
   */
  Dali::Application::AppSignalType& TaskInitSignal()
  {
    return mTaskInitSignal;
  }

  /**
   * @copydoc Dali::Application::TaskTerminateSignal()
   */
  Dali::Application::AppSignalType& TaskTerminateSignal()
  {
    return mTaskTerminateSignal;
  }

  /**
   * @copydoc Dali::Application::TaskAppControlSignal()
   */
  Dali::Application::AppControlSignalType& TaskAppControlSignal()
  {
    return mTaskAppControlSignal;
  }

  /**
   * @copydoc Dali::Application::TaskLanguageChangedSignal()
   */
  Dali::Application::AppSignalType& TaskLanguageChangedSignal()
  {
    return mTaskLanguageChangedSignal;
  }

  /**
   * @copydoc Dali::Application::TaskRegionChangedSignal()
   */
  Dali::Application::AppSignalType& TaskRegionChangedSignal()
  {
    return mTaskRegionChangedSignal;
  }

  /**
   * @copydoc Dali::Application::TaskLowBatterySignal()
   */
  Dali::Application::LowBatterySignalType& TaskLowBatterySignal()
  {
    return mTaskLowBatterySignal;
  }

  /**
   * @copydoc Dali::Application::TaskLowMemorySignal()
   */
  Dali::Application::LowMemorySignalType& TaskLowMemorySignal()
  {
    return mTaskLowMemorySignal;
  }

protected:
  /**
   * Private Constructor
   * @param[in]  argc               A pointer to the number of arguments
   * @param[in]  argv               A pointer to the argument list
   * @param[in]  stylesheet         The path to user defined theme file
   * @param[in]  windowMode         A member of Dali::Application::WINDOW_MODE
   * @param[in]  positionSize       A position and a size of the window
   * @param[in]  applicationType    A member of Dali::Framework::Type
   * @param[in]  type               The default window's type.
   * @param[in]  useUiThread         True if the application would create UI thread
   */
  Application(int* argc, char** argv[], const std::string& stylesheet, WINDOW_MODE windowMode, const PositionSize& positionSize, Framework::Type applicationType, WindowType type, bool useUiThread);

  /**
   * Destructor
   */
  ~Application() override;

  // Undefined
  Application(const Application&);
  Application& operator=(Application&);

  /**
   * Creates the default window
   */
  void CreateWindow();

  /**
   * Creates the adaptor
   */
  void CreateAdaptor();

  /**
   * Creates the adaptor builder
   */
  void CreateAdaptorBuilder();

  /**
   * Quits from the main loop
   */
  void QuitFromMainLoop();

  /**
   * Changes size of preInitialized window
   */
  void ChangePreInitializedWindowSize();

private:
  AppSignalType        mInitSignal;
  AppSignalType        mTerminateSignal;
  AppSignalType        mPauseSignal;
  AppSignalType        mResumeSignal;
  AppSignalType        mResetSignal;
  AppControlSignalType mAppControlSignal;
  AppSignalType        mLanguageChangedSignal;
  AppSignalType        mRegionChangedSignal;
  LowBatterySignalType mLowBatterySignal;
  LowMemorySignalType  mLowMemorySignal;

  AppSignalType        mTaskInitSignal;
  AppSignalType        mTaskTerminateSignal;
  AppControlSignalType mTaskAppControlSignal;
  AppSignalType        mTaskLanguageChangedSignal;
  AppSignalType        mTaskRegionChangedSignal;
  LowBatterySignalType mTaskLowBatterySignal;
  LowMemorySignalType  mTaskLowMemorySignal;

  EventLoop* mEventLoop;
  Framework* mFramework;

  CommandLineOptions* mCommandLineOptions;

  Dali::Internal::Adaptor::AdaptorBuilder* mAdaptorBuilder; ///< The adaptor builder
  Dali::Adaptor*                           mAdaptor;

  // The Main Window is that window created by the Application during initial startup
  // (previously this was the only window)
  Dali::Window                   mMainWindow;     ///< Main Window instance
  Dali::Application::WINDOW_MODE mMainWindowMode; ///< Window mode of the main window
  std::string                    mMainWindowName; ///< Name of the main window as obtained from environment options

  std::string        mStylesheet;
  EnvironmentOptions mEnvironmentOptions;
  PositionSize       mWindowPositionSize;
  Launchpad::State   mLaunchpadState;
  bool               mUseRemoteSurface;
  WindowType         mDefaultWindowType; ///< Default window's type. It is used when Application is created.
  bool               mUseUiThread;

  SlotDelegate<Application> mSlotDelegate;

  static ApplicationPtr gPreInitializedApplication;
};

inline Application& GetImplementation(Dali::Application& application)
{
  DALI_ASSERT_ALWAYS(application && "application handle is empty");

  BaseObject& handle = application.GetBaseObject();

  return static_cast<Internal::Adaptor::Application&>(handle);
}

inline const Application& GetImplementation(const Dali::Application& application)
{
  DALI_ASSERT_ALWAYS(application && "application handle is empty");

  const BaseObject& handle = application.GetBaseObject();

  return static_cast<const Internal::Adaptor::Application&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_APPLICATION_H
