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

// CLASS HEADER
#include <dali/public-api/adaptor-framework/timer.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/system-factory.h>
#include <dali/internal/system/common/timer-impl.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
Timer::Timer()
{
}

Timer Timer::New(uint32_t milliSec)
{
  Internal::Adaptor::TimerPtr internal = Dali::Internal::Adaptor::GetSystemFactory()->CreateTimer(milliSec);
  return Timer(internal.Get());
}

Timer::Timer(const Timer& copy) = default;

Timer& Timer::operator=(const Timer& rhs) = default;

Timer::Timer(Timer&& rhs) noexcept = default;

Timer& Timer::operator=(Timer&& rhs) noexcept = default;

Timer::~Timer()
{
}

Timer Timer::DownCast(BaseHandle handle)
{
  return Timer(dynamic_cast<Internal::Adaptor::Timer*>(handle.GetObjectPtr()));
}

void Timer::Start()
{
  Internal::Adaptor::GetImplementation(*this).Start();
}

void Timer::Stop()
{
  Internal::Adaptor::GetImplementation(*this).Stop();
}

void Timer::Pause()
{
  Internal::Adaptor::GetImplementation(*this).Pause();
}

void Timer::Resume()
{
  Internal::Adaptor::GetImplementation(*this).Resume();
}

void Timer::SetInterval(unsigned int interval)
{
  Internal::Adaptor::GetImplementation(*this).SetInterval(interval, true);
}

void Timer::SetInterval(unsigned int interval, bool restart)
{
  Internal::Adaptor::GetImplementation(*this).SetInterval(interval, restart);
}

unsigned int Timer::GetInterval() const
{
  return Internal::Adaptor::GetImplementation(*this).GetInterval();
}

bool Timer::IsRunning() const
{
  return Internal::Adaptor::GetImplementation(*this).IsRunning();
}

Timer::TimerSignalType& Timer::TickSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TickSignal();
}

Timer::Timer(Internal::Adaptor::Timer* timer)
: BaseHandle(timer)
{
}

} // namespace Dali
