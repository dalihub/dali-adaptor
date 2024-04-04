/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

// HEADER
#include <dali/internal/system/common/time-service.h>

// EXTERNAL INCLUDES
#include <chrono>
#include <thread>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace TimeService
{
void GetNanoseconds(uint64_t& timeInNanoseconds)
{
  // Get the time of a monotonic clock since its epoch.
  auto epoch = std::chrono::steady_clock::now().time_since_epoch();

  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch);

  timeInNanoseconds = static_cast<uint64_t>(duration.count());
}

uint32_t GetMilliSeconds()
{
  // Get the time of a monotonic clock since its epoch.
  auto epoch = std::chrono::steady_clock::now().time_since_epoch();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

  return static_cast<uint32_t>(duration.count());
}

void SleepUntil(uint64_t timeInNanoseconds)
{
  using Clock     = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;

  const Clock::duration duration = std::chrono::nanoseconds(timeInNanoseconds);
  const TimePoint       timePoint(duration);

  std::this_thread::sleep_until(timePoint);
}

} // namespace TimeService

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
