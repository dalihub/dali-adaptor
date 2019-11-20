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
#include <ctime>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace TimeService
{

namespace
{
const uint64_t NANOSECONDS_PER_SECOND = 1e+9;
}

void GetNanoseconds( uint64_t& timeInNanoseconds )
{
  timespec timeSpec;
  clock_gettime( CLOCK_MONOTONIC, &timeSpec );

  // Convert all values to uint64_t to match our return type
  timeInNanoseconds = ( static_cast< uint64_t >( timeSpec.tv_sec ) * NANOSECONDS_PER_SECOND ) + static_cast< uint64_t >( timeSpec.tv_nsec );
}

void SleepUntil( uint64_t timeInNanoseconds )
{
  timespec timeSpec;
  timeSpec.tv_sec  = timeInNanoseconds / NANOSECONDS_PER_SECOND;
  timeSpec.tv_nsec = timeInNanoseconds % NANOSECONDS_PER_SECOND;

  // clock_nanosleep returns 0 if it sleeps for the period specified, otherwise it returns an error value
  // If an error value is returned, just sleep again till the absolute time specified
  while( clock_nanosleep( CLOCK_MONOTONIC, TIMER_ABSTIME, &timeSpec, NULL ) )
  {
  }
}

} // namespace TimeService

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
