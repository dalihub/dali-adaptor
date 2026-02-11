#ifndef DALI_APPLICATION_CONTROLLER_H
#define DALI_APPLICATION_CONTROLLER_H

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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class ApplicationController;
}
} //namespace Internal DALI_INTERNAL

/**
 * @brief Application Controller
 *
 * ApplicationController manages the application lifecycle and handles
 * creation and initialization of Window and Adaptor.
 *
 * @note The controller must follow the lifecycle sequence: Initialize -> Pause/Resume -> Terminate
 */
class DALI_ADAPTOR_API ApplicationController : public BaseHandle
{
public:
  /**
   * @brief Creates an ApplicationController instance.
   *
   * @param[in] windowPositionSize The position and size of the window
   * @param[in] useUiThread Whether to use UI thread
   * @return A handle to the ApplicationController
   */
  static ApplicationController New(PositionSize windowPositionSize, bool useUiThread);

  /**
   * @brief Constructs an empty handle.
   */
  ApplicationController();

  /**
   * @brief Copy Constructor.
   *
   * @param[in] controller Handle to an object
   */
  ApplicationController(const ApplicationController& controller);

  /**
   * @brief Assignment operator.
   *
   * @param[in] controller Handle to an object
   * @return A reference to this
   */
  ApplicationController& operator=(const ApplicationController& controller);

  /**
   * @brief Move constructor.
   *
   * @param[in] rhs A reference to the moved handle
   */
  ApplicationController(ApplicationController&& rhs) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  ApplicationController& operator=(ApplicationController&& rhs) noexcept;

  /**
   * @brief Destructor.
   */
  ~ApplicationController();

  /**
   * @brief Pre-initializes the application.
   *
   * This method should be called during application startup to initialize
   * the window and adaptor. It must be called before GetWindow().
   */
  void PreInitialize();

  /**
   * @brief Post-initializes the application.
   *
   * This method should be called after PreInitialize() to complete the
   * initialization process and ensure all resources are loaded.
   *
   * @pre PreInitialize() must have been called
   */
  void PostInitialize();

  /**
   * @brief Pre-pauses the application.
   *
   * This method should be called when the application is about to be paused.
   */
  void PrePause();

  /**
   * @brief Post-pauses the application.
   *
   * This method should be called after PrePause() to complete the pause process.
   *
   * @pre PrePause() must have been called
   */
  void PostPause();

  /**
   * @brief Pre-resumes the application.
   *
   * This method should be called when the application is about to resume from pause.
   * It prepares the application for foreground execution.
   */
  void PreResume();

  /**
   * @brief Post-resumes the application.
   *
   * This method should be called after PreResume() to complete the resume process.
   *
   * @pre PreResume() must have been called
   */
  void PostResume();

  /**
   * @brief Pre-terminates the application.
   *
   * This method should be called when the application is about to be terminated.
   * It prepares the application for clean shutdown.
   *
   * @pre The application must be initialized
   */
  void PreTerminate();

  /**
   * @brief Post-terminates the application.
   *
   * This method should be called after PreTerminate() to complete the termination process.
   * It stops the adaptor and releases all resources.
   *
   * @pre PreTerminate() must have been called
   */
  void PostTerminate();

  /**
   * @brief Called before the application language is changed.
   *
   * This method should be called when the application language is about to be changed.
   * It prepares the application for the language change event.
   *
   * @param[in] language The new language to be set
   */
  void PreLanguageChanged(const std::string& language);

public: // Not intended for application developers
  /**
   * @brief Internal constructor.
   *
   * @param[in] controller Pointer to the internal implementation
   */
  explicit DALI_INTERNAL ApplicationController(Internal::Adaptor::ApplicationController* controller);
};

} // namespace Dali

#endif // DALI_APPLICATION_CONTROLLER_H
