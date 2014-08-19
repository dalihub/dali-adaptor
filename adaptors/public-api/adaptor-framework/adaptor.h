#ifndef __DALI_ADAPTOR_H__
#define __DALI_ADAPTOR_H__

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
#include <boost/function.hpp>
#include "window.h"
#include "tts-player.h"
#include <dali/public-api/signals/dali-signal-v2.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/events/touch-event.h>

namespace Dali DALI_IMPORT_API
{

struct DeviceLayout;
class RenderSurface;

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Adaptor;
}
}

/**
 * @brief An Adaptor object is used to initialize and control how Dali runs.
 *
 * It provides a lifecycle interface that allows the application
 * writer to provide their own main loop and other platform related
 * features.
 *
 * The Adaptor class provides a means for initialising the resources required by the Dali::Core.
 *
 * When dealing with platform events, the application writer MUST ensure that Dali is called in a
 * thread-safe manner.
 *
 * As soon as the Adaptor class is created and started, the application writer can initialise their
 * Dali::Actor objects straight away or as required by the main loop they intend to use (there is no
 * need to wait for an initialise signal as per the Dali::Application class).
 *
 * The Adaptor does emit a Resize signal which informs the user when the surface is resized.
 * Tizen and Linux Adaptors should follow the example below:
 *
 * @code
 * void CreateProgram(DaliAdaptor& adaptor)
 * {
 *   // Create Dali components...
 *   // Can instantiate adaptor here instead, if required
 * }
 *
 * int main ()
 * {
 *   // Initialise platform
 *   MyPlatform.Init();
 *
 *   // Create an 800 by 1280 window positioned at (0,0).
 *   Dali::PositionSize positionSize(0, 0, 800, 1280);
 *   Dali::Window window = Dali::Window::New( positionSize, "My Application" );
 *
 *   // Create an adaptor which uses that window for rendering
 *   Dali::Adaptor adaptor = Dali::Adaptor::New( window );
 *   adaptor.Start();
 *
 *   CreateProgram(adaptor);
 *   // Or use this as a callback function depending on the platform initialisation sequence.
 *
 *   // Start Main Loop of your platform
 *   MyPlatform.StartMainLoop();
 *
 *   return 0;
 * }
 * @endcode
 *
 * If required, you can also connect class member functions to a signal:
 *
 * @code
 * MyApplication application;
 * adaptor.ResizedSignal().Connect(&application, &MyApplication::Resize);
 * @endcode
 *
 * @see RenderSurface
 */
class Adaptor
{
public:

  typedef SignalV2< void (Adaptor&) > AdaptorSignalV2; ///< Generic Type for adaptor signals

public:
  /**
   * @brief Create a new adaptor using the window.
   *
   * @param[in] window The window to draw onto
   * @note The default base layout DeviceLayout::DEFAULT_BASE_LAYOUT will be used.
   * @return a reference to the adaptor handle
   */
  static Adaptor& New( Window window );

  /**
   * @brief Create a new adaptor using the window.
   *
   * @param[in] window The window to draw onto
   * @param[in] baseLayout  The base layout that the application has been written for
   * @return a reference to the adaptor handle
   */
  static Adaptor& New( Window window, const DeviceLayout& baseLayout );

  /**
   * @brief Virtual Destructor.
   */
  virtual ~Adaptor();

public:

  /**
   * @brief Starts the Adaptor.
   */
  void Start();

  /**
   * @brief Pauses the Adaptor.
   */
  void Pause();

  /**
   * @brief Resumes the Adaptor, if previously paused.
   *
   * @note If the adaptor is not paused, this does not do anything.
   */
  void Resume();

  /**
   * @brief Stops the Adaptor.
   */
  void Stop();

  /**
   * @brief Ensures that the function passed in is called from the main loop when it is idle.
   *
   * A callback of the following type may be used:
   * @code
   *   void MyFunction();
   * @endcode
   *
   * @param[in]  callBack  The function to call.
   * @return true if added successfully, false otherwise
   */
  bool AddIdle( boost::function<void(void)> callBack );

  /**
   * @brief Get the render surface the adaptor is using to render to.
   *
   * @return reference to current render surface
   */
  RenderSurface& GetSurface();

  /**
   * @brief Returns a reference to the instance of the adaptor used by the current thread.
   *
   * @return A reference to the adaptor.
   * @pre The adaptor has been initialised.
   * @note This is only valid in the main thread.
   */
  static Adaptor& Get();

  /**
   * @brief Checks whether the adaptor is available.
   *
   * @return true, if it is available, false otherwise.
   */
  static bool IsAvailable();

  /**
   * @brief Registers the singleton of Dali handle with its type info.
   *
   * The singleton will be kept alive for the life time of the
   * adaptor.
   * @note This is not intended for application developers.
   * @param[in] info The type info of the Dali handle generated by the compiler.
   * @param[in] singleton The Dali handle to be registered
   */
  void RegisterSingleton(const std::type_info& info, BaseHandle singleton);

  /**
   * @brief Gets the singleton for the given type.
   *
   * @note This is not intended for application developers.
   * @param[in] info The type info of the given type.
   * @return the Dali handle if it is registered as a singleton or an uninitialized handle.
   */
  BaseHandle GetSingleton(const std::type_info& info) const;

  /**
   * @brief Call this method to notify Dali when the system language changes.
   *
   * Use this only when NOT using Dali::Application, As Application created using
   * Dali::Application will automatically receive notification of language change.
   * When Dali::Application is not used, the application developer should
   * use app-core to receive language change notifications and should update Dali
   * by calling this method.
   */
  void NotifyLanguageChanged();

  /**
   * @brief Sets minimum distance in pixels that the fingers must move towards/away from each other in order to
   * trigger a pinch gesture
   *
   * @param[in] distance The minimum pinch distance in pixels
   */
  void SetMinimumPinchDistance(float distance);

public:  // Signals

  /**
   * @brief The user should connect to this signal if they need to perform any
   * special activities when the surface Dali is being rendered on is resized.
   *
   * @return The signal to connect to
   */
  AdaptorSignalV2& ResizedSignal();

  /**
   * @brief This signal is emitted when the language is changed on the device.
   *
   * @return The signal to connect to
   */
  AdaptorSignalV2& LanguageChangedSignal();

private:

  // Undefined
  Adaptor(const Adaptor&);
  Adaptor& operator=(Adaptor&);

private:

  /**
   * @brief Create an uninitialized Adaptor.
   */
  DALI_INTERNAL Adaptor();

  Internal::Adaptor::Adaptor* mImpl; ///< Implementation object
  friend class Internal::Adaptor::Adaptor;
};

} // namespace Dali

#endif // __DALI_ADAPTOR_H__
