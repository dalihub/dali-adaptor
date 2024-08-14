#ifndef DALI_INTERNAL_OFFSCREEN_APPLICATION_IMPL_H
#define DALI_INTERNAL_OFFSCREEN_APPLICATION_IMPL_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/base-object.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/offscreen-application.h>
#include <dali/devel-api/adaptor-framework/offscreen-window.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/system/common/environment-options.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class FrameworkFactory;
} // namespace Adaptor

/**
 * Implementation of the OffscreenApplication class.
 */
class OffscreenApplication : public BaseObject, public Adaptor::Framework::Observer, public Adaptor::Framework::TaskObserver
{
public:
  using OffscreenApplicationSignalType = Dali::OffscreenApplication::OffscreenApplicationSignalType;

  /**
   * @brief Create a new OffscreenApplication
   * @param[in] width The width of the default OffscreenWindow
   * @param[in] height The height of the default OffscreenWindow
   * @param[in] surface The native surface handle to create the default OffscreenWindow
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   * @param[in] renderMode The RenderMode of the OffscreenApplication
   */
  static IntrusivePtr<OffscreenApplication> New(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent, Dali::OffscreenApplication::RenderMode renderMode);

public:
  /**
   * Destructor
   */
  virtual ~OffscreenApplication();

  /**
   * @copydoc Dali::OffscreenApplication::MainLoop()
   */
  void MainLoop();

  /**
   * @copydoc Dali::OffscreenApplication::Quit()
   */
  void Quit();

  /**
   * @copydoc Dali::OffscreenApplication::GetDefaultWindow()
   */
  Dali::OffscreenWindow GetWindow();

  /**
   * @copydoc Dali::OffscreenApplication::RenderOnce()
   */
  void RenderOnce();

  /**
   * @copydoc Dali::OffscreenApplication::GetFrameworkContext()
   */
  Any GetFrameworkContext() const;

public: // Signals
  /**
   * @copydoc Dali::OffscreenApplication::InitSignal()
   */
  OffscreenApplicationSignalType& InitSignal()
  {
    return mInitSignal;
  }

  /**
   * @copydoc Dali::OffscreenApplication::TerminateSignal()
   */
  OffscreenApplicationSignalType& TerminateSignal()
  {
    return mTerminateSignal;
  }

  /**
   * @copydoc Dali::OffscreenApplication::PauseSignal()
   */
  OffscreenApplicationSignalType& PauseSignal()
  {
    return mPauseSignal;
  }

  /**
   * @copydoc Dali::OffscreenApplication::ResumeSignal()
   */
  OffscreenApplicationSignalType& ResumeSignal()
  {
    return mResumeSignal;
  }

  /**
   * @copydoc Dali::OffscreenApplication::ResetSignal()
   */
  OffscreenApplicationSignalType& ResetSignal()
  {
    return mResetSignal;
  }

  /**
   * @copydoc Dali::OffscreenApplication::LanguageChangedSignal()
   */
  OffscreenApplicationSignalType& LanguageChangedSignal()
  {
    return mLanguageChangedSignal;
  }

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
   * Called when the framework informs the application that it should reset itself.
   */
  void OnReset() override;

  /**
   * Called when the framework informs the application that the language of the device has changed.
   */
  void OnLanguageChanged() override;

private:
  /**
   * Private constructor
   * @param[in] width The width of the OffscreenWindow
   * @param[in] height The height of the OffscreenApplication
   * @param[in] surface The native surface handle to create the default OffscreenWindow
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   * @param[in] renderMode The RenderMode of the OffscreenApplication
   */
  OffscreenApplication(uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent, Dali::OffscreenApplication::RenderMode renderMode);

  /**
   * Quits from the main loop
   */
  void QuitFromMainLoop();

  // Undefined
  OffscreenApplication(const OffscreenApplication&) = delete;
  OffscreenApplication& operator=(OffscreenApplication&) = delete;
  OffscreenApplication& operator=(const OffscreenApplication&) = delete;
  OffscreenApplication& operator=(OffscreenApplication&&) = delete;

private:
  std::unique_ptr<Dali::Adaptor>                               mAdaptor;
  std::unique_ptr<Dali::Internal::Adaptor::EnvironmentOptions> mEnvironmentOptions;

  Dali::OffscreenWindow mDefaultWindow;

  std::unique_ptr<Internal::Adaptor::Framework>        mFramework;
  std::unique_ptr<Internal::Adaptor::FrameworkFactory> mFrameworkFactory;

  OffscreenApplicationSignalType mInitSignal;
  OffscreenApplicationSignalType mTerminateSignal;
  OffscreenApplicationSignalType mPauseSignal;
  OffscreenApplicationSignalType mResumeSignal;
  OffscreenApplicationSignalType mResetSignal;
  OffscreenApplicationSignalType mLanguageChangedSignal;
};

inline OffscreenApplication& GetImplementation(Dali::OffscreenApplication& offscreenApplication)
{
  DALI_ASSERT_ALWAYS(offscreenApplication && "OffscreenApplication handle is empty");

  BaseObject& handle = offscreenApplication.GetBaseObject();

  return static_cast<OffscreenApplication&>(handle);
}

inline const OffscreenApplication& GetImplementation(const Dali::OffscreenApplication& offscreenApplication)
{
  DALI_ASSERT_ALWAYS(offscreenApplication && "OffscreenApplication handle is empty");

  const BaseObject& handle = offscreenApplication.GetBaseObject();

  return static_cast<const OffscreenApplication&>(handle);
}

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_OFFSCREEN_APPLICATION_IMPL_H
