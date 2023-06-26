#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_LINUX_TIMER_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_LINUX_TIMER_H

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

// INTERNAL INCLUDES
#include <dali/internal/system/common/timer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class TimerEcore;

typedef IntrusivePtr<TimerEcore> TimerEcorePtr;

/**
 * @brief TimerEcore class provides an Timer Ecore implementation.
 */
class TimerEcore : public Timer
{
public:
  static TimerEcorePtr New(uint32_t milliSec);

  /**
   * Constructor
   * @param[in]  milliSec  Interval in milliseconds.
   */
  TimerEcore(uint32_t milliSec);

  /**
   * Destructor.
   */
  virtual ~TimerEcore();

public:
  /**
   * @copydoc Dali::Timer::Start()
   */
  void Start() override;

  /**
   * @copydoc Dali::Timer::Stop()
   */
  void Stop() override;

  /**
   * @copydoc Dali::Timer::Pause()
   */
  void Pause() override;

  /**
   * @copydoc Dali::Timer::Resume()
   */
  void Resume() override;

  /**
   * @copydoc Dali::Timer::SetInterval()
   */
  void SetInterval(uint32_t interval, bool restart) override;

  /**
   * @copydoc Dali::Timer::GetInterval()
   */
  uint32_t GetInterval() const override;

  /**
   * @copydoc Dali::Timer::IsRunning()
   */
  bool IsRunning() const override;

  /**
   * Tick
   */
  bool Tick();

private: // Implementation
  // not implemented
  TimerEcore(const TimerEcore&) = delete;
  TimerEcore& operator=(const TimerEcore&) = delete;

  /**
   * Resets any stored timer data.
   */
  void ResetTimerData();

private: // Data
  // To hide away implementation details
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_LINUX_TIMER_H
