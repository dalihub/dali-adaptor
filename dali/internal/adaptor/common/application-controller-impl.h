#ifndef DALI_INTERNAL_APPLICATION_CONTROLLER_H
#define DALI_INTERNAL_APPLICATION_CONTROLLER_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/application-controller.h>
#include <dali/internal/adaptor/common/ui-context-impl.h>

namespace Dali
{
class Adaptor;
class UiContext;

namespace Internal
{
namespace Adaptor
{
class EnvironmentOptions;
class FrameworkFactory;

using ApplicationControllerPtr = IntrusivePtr<ApplicationController>;

/**
 * @brief Implementation of the ApplicationController class.
 */
class ApplicationController : public BaseObject
{
public:
  /**
   * @brief LaunchpadState is used to improve application launch performance.
   * When an application is pre-initialized, resources are preloaded, some functions are initialized and a window is created in advance.
   */
  enum class LaunchpadState
  {
    NONE,           ///< The default state
    PRE_INITIALIZED ///< Application is pre-initialized.
  };

public:
  /**
   * @copydoc Dali::ApplicationController::New()
   */
  static ApplicationControllerPtr New(PositionSize windowPositionSize, bool useUiThread);

  /**
   * @copydoc Dali::ApplicationController::PreInitialize()
   */
  void PreInitialize();

  /**
   * @copydoc Dali::ApplicationController::PostInitialize()
   */
  void PostInitialize();

  /**
   * @copydoc Dali::ApplicationController::PrePause()
   */
  void PrePause();

  /**
   * @copydoc Dali::ApplicationController::PostPause()
   */
  void PostPause();

  /**
   * @copydoc Dali::ApplicationController::PreResume()
   */
  void PreResume();

  /**
   * @copydoc Dali::ApplicationController::PostResume()
   */
  void PostResume();

  /**
   * @copydoc Dali::ApplicationController::PreTerminate()
   */
  void PreTerminate();

  /**
   * @copydoc Dali::ApplicationController::PostTerminate()
   */
  void PostTerminate();

  /**
   * @copydoc Dali::ApplicationController::PreLanguageChanged()
   */
  void PreLanguageChanged(const std::string& language);

protected:
  /**
   * @brief Private Constructor
   */
  ApplicationController(PositionSize windowPositionSize, bool useUiThread);

  /**
   * @brief Destructor
   */
  ~ApplicationController();

private:
  /**
   * Creates the default window
   */
  void CreateWindow();

  /**
   * Creates the adaptor.
   * It should be called after main window created.
   */
  void CreateAdaptor();

  /**
   * @brief Get latest environment options and apply changeness
   */
  void UpdateEnvironmentOptions();

private:
  ApplicationController(const ApplicationController&)            = delete; ///< Deleted copy constructor.
  ApplicationController(ApplicationController&&)                 = delete; ///< Deleted move constructor.
  ApplicationController& operator=(const ApplicationController&) = delete; ///< Deleted copy assignment operator.
  ApplicationController& operator=(ApplicationController&&)      = delete; ///< Deleted move assignment operator.

private:
  Dali::Window    mMainWindow;
  Dali::UiContext mUiContext;

  PositionSize mWindowPositionSize{};

  std::unique_ptr<Dali::Adaptor>      mAdaptor{};
  std::unique_ptr<EnvironmentOptions> mEnvironmentOptions{};
  std::unique_ptr<FrameworkFactory>   mFrameworkFactory{};

  LaunchpadState mLaunchpadState{LaunchpadState::NONE};

  bool mUseUiThread{false};
};

/**
 * @brief Retrieves the internal implementation from the public handle.
 *
 * @param[in] controller The public ApplicationController handle
 * @return Reference to the internal ApplicationController implementation
 */
inline ApplicationController& GetImplementation(Dali::ApplicationController& controller)
{
  DALI_ASSERT_ALWAYS(controller && "ApplicationController handle is empty");

  BaseObject& handle = controller.GetBaseObject();

  return static_cast<Internal::Adaptor::ApplicationController&>(handle);
}

/**
 * @brief Retrieves the internal implementation from the public handle (const version).
 *
 * @param[in] controller The public ApplicationController handle
 * @return Const reference to the internal ApplicationController implementation
 */
inline const ApplicationController& GetImplementation(const Dali::ApplicationController& controller)
{
  DALI_ASSERT_ALWAYS(controller && "ApplicationController handle is empty");

  const BaseObject& handle = controller.GetBaseObject();

  return static_cast<const Internal::Adaptor::ApplicationController&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_APPLICATION_CONTROLLER_H
