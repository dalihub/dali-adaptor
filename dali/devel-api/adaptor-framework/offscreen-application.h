#ifndef DALI_OFFSCREEN_APPLICATION_H
#define DALI_OFFSCREEN_APPLICATION_H

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
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

class OffscreenWindow;

namespace Internal
{
class OffscreenApplication;
}

/**
 * @brief Appliations can draw UI on offscreen surface with the OffscreenApplication.
 * When you use a OffscreenApplication, you don't have to create a Window.
 */
class DALI_ADAPTOR_API OffscreenApplication : public Dali::BaseHandle
{
public:
  using OffscreenApplicationSignalType = Signal<void(void)>;

  /**
   * @brief Enumeration for the render mode
   */
  enum class RenderMode
  {
    AUTO,  // Scene is rendered automatically
    MANUAL // Scene is rendered by RenderOnce()
  };

public:
  /**
   * @brief This is the constructor of OffscreenApplication
   *
   * @param[in] width The initial width of the default OffscreenWindow
   * @param[in] height The initial height of the default OffscreenWindow
   * @param[in] isTranslucent Whether the OffscreenWindow is translucent or not
   * @param[in] renderMode The RenderMode of the OffscreenApplication
   */
  static OffscreenApplication New(uint16_t width, uint16_t height, bool isTranslucent, RenderMode renderMode = RenderMode::AUTO);

  /**
   * @brief This is the constructor of OffscreenApplication
   *
   * @param[in] surface The native surface handle to create the OffscreenWindow
   * @param[in] renderMode The RenderMode of the OffscreenApplication
   */
  static OffscreenApplication New(Dali::Any surface, RenderMode renderMode = RenderMode::AUTO);

  /**
   * @brief Constructs an empty handle
   */
  OffscreenApplication();

  /**
   * @brief Copy constructor
   *
   * @param [in] offscreenApplication A reference to the copied handle
   */
  OffscreenApplication(const OffscreenApplication& offscreenApplication);

  /**
   * @brief Assignment operator
   *
   * @param [in] offscreenApplication A reference to the copied handle
   * @return A reference to this
   */
  OffscreenApplication& operator=(const OffscreenApplication& offscreenApplication);

  /**
   * @brief Destructor
   */
  ~OffscreenApplication();

public:
  /**
   * @brief This starts the application.
   */
  void MainLoop();

  /**
   * @brief This quits the application.
   */
  void Quit();

  /**
   * @brief Get the default OffscreenWindow handle
   * @return The default OffscreenWindow
   */
  OffscreenWindow GetWindow();

  /**
   * @brief Renders once more even if we're paused
   */
  void RenderOnce();

  /**
   * @brief Gets the context of the framwork
   * @return Platform dependent context handle
   */
  Any GetFrameworkContext() const;

public: // Signals
  /**
   * @brief Signal to notify the client when the application is ready to be initialized
   *
   * @note OffscreenApplication::Start() should be called to be initialized
   *
   * @return The signal
   */
  OffscreenApplicationSignalType& InitSignal();

  /**
   * @brief Signal to notify the user when the application is about to be terminated
   *
   * @return The signal
   */
  OffscreenApplicationSignalType& TerminateSignal();

  /**
   * @brief Signal to notify the user when the application is about to be paused
   *
   * @return The signal
   */
  OffscreenApplicationSignalType& PauseSignal();

  /**
   * @brief Signal to notify the user when the application is about to be resumed
   *
   * @return The signal
   */
  OffscreenApplicationSignalType& ResumeSignal();

  /**
   * @brief Signal to notify the user when the application is about to be reinitialized
   *
   * @return The signal
   */
  OffscreenApplicationSignalType& ResetSignal();

  /**
   * @brief Signal to notify the user when the application is about to be language is changed on the device.
   *
   * @return The signal
   */
  OffscreenApplicationSignalType& LanguageChangedSignal();

public: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL OffscreenApplication(Internal::OffscreenApplication* offscreenApplication);
};

/**
 * @}
 */

} // namespace Dali

#endif // DALI_OFFSCREEN_APPLICATION_H
