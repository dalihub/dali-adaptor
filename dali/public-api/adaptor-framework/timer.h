#ifndef DALI_TIMER_H
#define DALI_TIMER_H

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

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Timer;
}
}

/**
 * @brief Mechanism to issue simple periodic or one-shot events.
 *
 * Timer is provided for application developers to be able to issue
 * simple periodic or one-shot events.  Please note that timer
 * callback functions should return as soon as possible, because they
 * block the next SignalTick.  Please note that timer signals are not
 * in sync with Dali's render timer.
 *
 * This class is a handle class so it can be stack allocated and used
 * as a member.
 * @SINCE_1_0.0
 */
class DALI_ADAPTOR_API Timer : public BaseHandle
{
public: // Signal typedefs

  typedef Signal< bool () > TimerSignalType; ///< Timer finished signal callback type @SINCE_1_0.0

public: // API

  /**
   * @brief Constructor, creates an uninitialized timer.
   *
   * Call New to fully construct a timer.
   * @SINCE_1_0.0
   */
  Timer();

  /**
   * @brief Creates a tick Timer that emits periodic signal.
   *
   * @SINCE_1_0.0
   * @param[in] milliSec Interval in milliseconds
   * @return A new timer
   */
  static Timer New( unsigned int milliSec );

  /**
   * @brief Copy constructor.
   *
   * @SINCE_1_0.0
   * @param[in] timer The handle to copy. The copied handle will point at the same implementation
   */
  Timer( const Timer& timer );

  /**
   * @brief Assignment operator.
   *
   * @SINCE_1_0.0
   * @param[in] timer The handle to copy. This handle will point at the same implementation
   * as the copied handle
   * @return Reference to this timer handle
   */
  Timer& operator=( const Timer& timer );

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_1_0.0
   */
  ~Timer();

  /**
   * @brief Downcasts a handle to Timer handle.
   *
   * If handle points to a Timer object, the downcast produces a valid handle.
   * If not, the returned handle is left uninitialized.
   *
   * @SINCE_1_0.0
   * @param[in] handle to An object
   * @return handle to a Timer object or an uninitialized handle
   */
  static Timer DownCast( BaseHandle handle );

  /**
   * @brief Starts timer.
   *
   * In case a Timer is already running, its time is reset and timer is restarted.
   * @SINCE_1_0.0
   */
  void Start();

  /**
   * @brief Stops timer.
   * @SINCE_1_0.0
   */
  void Stop();

  /**
   * @brief Pauses timer.
   * @SINCE_1_3.41
   */
  void Pause();

  /**
   * @brief Resumes timer.
   * @SINCE_1_3.41
   */
  void Resume();

  /**
   * @brief Sets a new interval on the timer and starts the timer.
   *
   * Cancels the previous timer.
   * @SINCE_1_0.0
   * @param[in] milliSec Interval in milliseconds
   */
  void SetInterval( unsigned int milliSec );

  /**
   * @brief Sets a new interval on the timer with option to restart the timer.
   *
   * Cancels the previous timer.
   * @SINCE_1_3.41
   * @param[in] milliSec Interval in milliseconds
   * @param[in] restart Flag to set enabled to restart or not.
   */
  void SetInterval( unsigned int milliSec, bool restart );

  /**
   * @brief Gets the interval of timer.
   *
   * @SINCE_1_0.0
   * @return Interval in milliseconds
   */
  unsigned int GetInterval() const;

  /**
   * @brief Tells whether timer is running.
   * @SINCE_1_0.0
   * @return Whether Timer is started or not
   */
  bool IsRunning() const;

public: // Signals

  /**
   * @brief Signal emitted after specified time interval.
   *
   * The return of the callback decides whether signal emission stops or continues.
   * If the callback function returns false, emission will stop and if true, it will continue.
   * This return value is ignored for one-shot events, which will always stop after the first execution.
   * @return The signal to Connect() with
   * @SINCE_1_0.0
   */
  TimerSignalType& TickSignal();

public: // Not intended for application developers
  explicit DALI_INTERNAL Timer(Internal::Adaptor::Timer* timer);
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_TIMER_H
