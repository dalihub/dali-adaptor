#ifndef __DALI_INTERNAL_APPLICATION_H__
#define __DALI_INTERNAL_APPLICATION_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <application.h>
#include <singleton-service.h>

#include <framework.h>
#include <window-impl.h>

namespace Dali
{
class Adaptor;
class Window;

namespace Internal
{

namespace Adaptor
{
class CommandLineOptions;
class EventLoop;

typedef Dali::Rect<int> PositionSize;

class Application;
typedef IntrusivePtr<Application> ApplicationPtr;

/**
 * Implementation of the Application class.
 */
class Application : public BaseObject, public Framework::Observer
{
public:

  typedef Dali::Application::AppSignalV2 AppSignalV2;

  /**
   * Constructor
   * @param[in]  app         The public instance of the Application
   * @param[in]  argc        A pointer to the number of arguments
   * @param[in]  argv        A pointer to the argument list
   * @param[in]  name        A name of application
   * @param[in]  baseLayout  The base layout that the application has been written for
   * @param[in]  windowMode  A member of Dali::Application::WINDOW_MODE
   */
  static ApplicationPtr New(int* argc, char **argv[], const std::string& name,
                            const DeviceLayout& baseLayout,
                            Dali::Application::WINDOW_MODE windowMode);

  Application( int* argc, char **argv[], const std::string& name, const DeviceLayout& baseLayout, Dali::Application::WINDOW_MODE windowMode );

  /**
   * Destructor
   */
  virtual ~Application();

public:

  /**
   * @copydoc Dali::Application::MainLoop()
   */
  void MainLoop(Dali::Configuration::ContextLoss configuration);

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
  bool AddIdle(boost::function<void(void)> callBack);

  /**
   * @copydoc Dali::Application::GetAdaptor();
   */
  Dali::Adaptor& GetAdaptor();

  /**
   * @copydoc Dali::Application::GetWindow();
   */
  Dali::Window GetWindow();

  /**
   * @copydoc Dali::Application::GetTheme();
   */
  const std::string& GetTheme();

  /**
   * @copydoc Dali::Application::SetTheme();
   */
  void SetTheme(const std::string& themeFilePath);

  /**
   * @copydoc Dali::Application::ReplaceWindow();
   */
  void ReplaceWindow(PositionSize windowPosition, const std::string& name);

public: // Stereoscopy

  /**
   * @copydoc Dali::Application::SetViewMode()
   */
  void SetViewMode( ViewMode viewMode );

  /**
   * @copydoc Dali::Application::GetViewMode()
   */
  ViewMode GetViewMode() const;

  /**
   * @copydoc Dali::Application::SetStereoBase()
   */
  void SetStereoBase( float stereoBase );

  /**
   * @copydoc Dali::Application::GetStereoBase()
   */
  float GetStereoBase() const;

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
   * Called when the framework is paused.
   */
  virtual void OnPause();

  /**
   * Called when the framework resumes from a paused state.
   */
  virtual void OnResume();

  /**
   * Called when the framework informs the application that it should reset itself.
   */
  virtual void OnReset();

  /**
   * Called when the framework informs the application that the language of the device has changed.
   */
  virtual void OnLanguageChanged();

public:

  /**
   * Signal handler when the adaptor's window resizes itself.
   * @param[in]  adaptor  The adaptor
   */
  void OnResize(Dali::Adaptor& adaptor);

public:  // Signals

  /**
   * @copydoc Dali::Application::InitSignal()
   */
  Dali::Application::AppSignalV2& InitSignal() { return mInitSignalV2; }

  /**
   * @copydoc Dali::Application::TerminateSignal()
   */
  Dali::Application::AppSignalV2& TerminateSignal() { return mTerminateSignalV2; }

  /**
   * @copydoc Dali::Application::PauseSignal()
   */
  Dali::Application::AppSignalV2& PauseSignal() { return mPauseSignalV2; }

  /**
   * @copydoc Dali::Application::ResumeSignal()
   */
  Dali::Application::AppSignalV2& ResumeSignal() { return mResumeSignalV2; }

  /**
   * @copydoc Dali::Application::ResetSignal()
   */
  Dali::Application::AppSignalV2& ResetSignal() { return mResetSignalV2; }

  /**
   * @copydoc Dali::Application::ResizeSignal()
   */
  Dali::Application::AppSignalV2& ResizeSignal() { return mResizeSignalV2; }

  /**
   * @copydoc Dali::Application::LanguageChangedSignal()
   */
  Dali::Application::AppSignalV2& LanguageChangedSignal() { return mLanguageChangedSignalV2; }

private:

  // Undefined
  Application(const Application&);
  Application& operator=(Application&);

private:
  /**
   * Creates the window
   */
  void CreateWindow();

  /**
   * Creates the adaptor
   */
  void CreateAdaptor();

  /**
   * Quits from the main loop
   */
  void QuitFromMainLoop();

private:

  AppSignalV2                           mInitSignalV2;
  AppSignalV2                           mTerminateSignalV2;
  AppSignalV2                           mPauseSignalV2;
  AppSignalV2                           mResumeSignalV2;
  AppSignalV2                           mResetSignalV2;
  AppSignalV2                           mResizeSignalV2;
  AppSignalV2                           mLanguageChangedSignalV2;

  EventLoop*                            mEventLoop;
  Framework*                            mFramework;

  Dali::Configuration::ContextLoss      mContextLossConfiguration;
  CommandLineOptions*                   mCommandLineOptions;

  Dali::SingletonService                mSingletonService;
  Dali::Adaptor*                        mAdaptor;
  Dali::Window                          mWindow;
  Dali::Application::WINDOW_MODE        mWindowMode;
  std::string                           mName;

  bool                                  mInitialized;
  DeviceLayout                          mBaseLayout;

  SlotDelegate< Application >           mSlotDelegate;
};

inline Application& GetImplementation(Dali::Application& application)
{
  DALI_ASSERT_ALWAYS(application && "application handle is empty");

  BaseObject& handle = application.GetBaseObject();

  return static_cast<Internal::Adaptor::Application&>(handle);
}

inline const Application& GetImplementation(const Dali::Application& application)
{
  DALI_ASSERT_ALWAYS(application && "Timre handle is empty");

  const BaseObject& handle = application.GetBaseObject();

  return static_cast<const Internal::Adaptor::Application&>(handle);
}


} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_APPLICATION_H__
