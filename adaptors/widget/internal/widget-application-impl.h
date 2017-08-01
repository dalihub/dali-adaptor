#ifndef __DALI_INTERNAL_WIDGET_APPLICATION_H__
#define __DALI_INTERNAL_WIDGET_APPLICATION_H__

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
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <widget-application.h>
#include <singleton-service.h>

#include <framework.h>
#include <window.h>
#include <base/environment-options.h>

namespace Dali
{
class Adaptor;

namespace Internal
{

namespace Adaptor
{
class CommandLineOptions;
class EventLoop;

class WidgetApplication;
typedef IntrusivePtr<WidgetApplication> WidgetApplicationPtr;

/**
 * Implementation of the WidgetApplication class.
 */
class WidgetApplication : public BaseObject, public Framework::Observer
{
public:
  typedef Dali::WidgetApplication::AppSignalType AppSignalType;

  /**
   * Create a new widget application
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   */
  static WidgetApplicationPtr New( int* argc, char **argv[], const std::string& stylesheet );

public:

  /**
   * @copydoc Dali::WidgetApplication::MainLoop()
   */
  void MainLoop();

  /**
   * @copydoc Dali::WidgetApplication::Quit()
   */
  void Quit();

  /**
   * @copydoc Dali::WidgetApplication::GetWindow();
   */
  Dali::Window GetWindow();

  /**
   * @copydoc Dali::WidgetApplication::GetAdaptor();
   */
  Dali::Adaptor& GetAdaptor();

  /**
   * @copydoc Dali::WidgetApplication::GetResourcePath();
   */
  static std::string GetResourcePath();

public: // Lifecycle functionality

  /**
   * Called when OnInit is called or the framework is initialised.
   */
  void DoInit();

  /**
   * Called after OnInit is called or the framework is started.
   */
  void DoStart();

  /**
   * Called when OnTerminate is called or the framework is terminated.
   */
  void DoTerminate();

  /**
   * Called when OnLanguageChanged is called or the framework informs the application that the language of the device has changed.
   */
  void DoLanguageChange();

public: // From Framework::Observer

  /**
   * Called when the framework is initialised.
   */
  virtual void OnInit();

  /**
   * Called when the framework is terminated.
   */
  virtual void OnTerminate();

  /**
   * Called when the framework informs the application that the language of the device has changed.
   */
  virtual void OnLanguageChanged();

  /**
  * Called when the framework informs the application that the region of the device has changed.
  */
  virtual void OnRegionChanged();

  /**
  * Called when the framework informs the application that the battery level of the device is low.
  */
  virtual void OnBatteryLow();

  /**
  * Called when the framework informs the application that the memory level of the device is low.
  */
  virtual void OnMemoryLow();

public:  // Signals

  /**
   * @copydoc Dali::WidgetApplication::InitSignal()
   */
   Dali::WidgetApplication::AppSignalType& InitSignal() { return mInitSignal; }

  /**
   * @copydoc Dali::WidgetApplication::TerminateSignal()
   */
  Dali::WidgetApplication::AppSignalType& TerminateSignal() { return mTerminateSignal; }

  /**
   * @copydoc Dali::WidgetApplication::LanguageChangedSignal()
   */
  Dali::WidgetApplication::AppSignalType& LanguageChangedSignal() { return mLanguageChangedSignal; }

  /**
  * @copydoc Dali::WidgetApplication::RegionChangedSignal()
  */
  Dali::WidgetApplication::AppSignalType& RegionChangedSignal() { return mRegionChangedSignal; }

  /**
  * @copydoc Dali::WidgetApplication::BatteryLowSignal()
  */
  Dali::WidgetApplication::AppSignalType& BatteryLowSignal() { return mBatteryLowSignal; }

  /**
  * @copydoc Dali::WidgetApplication::MemoryLowSignal()
  */
  Dali::WidgetApplication::AppSignalType& MemoryLowSignal() { return mMemoryLowSignal; }

protected:

  /**
   * Private Constructor
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   */
  WidgetApplication( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * Destructor
   */
  virtual ~WidgetApplication();

  // Undefined
  WidgetApplication(const Application&);
  WidgetApplication& operator=(Application&);

  /**
   * Creates the adaptor
   */
  void CreateAdaptor();

  /**
   * Quits from the main loop
   */
  void QuitFromMainLoop();

  /**
   * Add idle
   */
  bool AddIdle( CallbackBase* callback );

private:

  AppSignalType                         mInitSignal;
  AppSignalType                         mTerminateSignal;
  AppSignalType                         mLanguageChangedSignal;
  AppSignalType                         mRegionChangedSignal;
  AppSignalType                         mBatteryLowSignal;
  AppSignalType                         mMemoryLowSignal;

  Framework*                            mFramework;

  Dali::Configuration::ContextLoss      mContextLossConfiguration;
  CommandLineOptions*                   mCommandLineOptions;

  Dali::SingletonService                mSingletonService;
  Dali::Adaptor*                        mAdaptor;
  Dali::Window                          mWindow;
  std::string                           mName;
  std::string                           mStylesheet;
  EnvironmentOptions                    mEnvironmentOptions;

  SlotDelegate< WidgetApplication >     mSlotDelegate;
};

inline WidgetApplication& GetImplementation(Dali::WidgetApplication& widgetApplication)
{
  DALI_ASSERT_ALWAYS(widgetApplication && "widget application handle is empty");

  BaseObject& handle = widgetApplication.GetBaseObject();

  return static_cast<Internal::Adaptor::WidgetApplication&>(handle);
}

inline const WidgetApplication& GetImplementation(const Dali::WidgetApplication& widgetApplication)
{
  DALI_ASSERT_ALWAYS(widgetApplication && "widget application handle is empty");

  const BaseObject& handle = widgetApplication.GetBaseObject();

  return static_cast<const Internal::Adaptor::WidgetApplication&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_WIDGET_APPLICATION_H__
