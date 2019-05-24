#ifndef DALI_INTERNAL_TIME_SERVICE_H
#define DALI_INTERNAL_TIME_SERVICE_H

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

#include <stdint.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace TimeService
{

/**
 * @brief Get the monotonic time since some unspecified starting point (usually the boot time).
 *
 * @param[out]  timeInNanoseconds  The time in nanoseconds since the reference point.
 *
 * @note The maximum value timeInNanoseconds can hold is 0xFFFFFFFFFFFFFFFF which is 1.844674407e+19. Therefore, this can overflow after approximately 584 years.
 */
void GetNanoseconds( uint64_t& timeInNanoseconds );

/**
 * @brief Sleeps until the monotonic time specified since some unspecified starting point (usually the boot time).
 *
 * If the time specified has already passed, then it returns immediately.
 *
 * @param[in]  timeInNanoseconds  The time to sleep until
 *
 * @note The maximum value timeInNanoseconds can hold is 0xFFFFFFFFFFFFFFFF which is 1.844674407e+19. Therefore, this can overflow after approximately 584 years.
 */
void SleepUntil( uint64_t timeInNanoseconds );

} // namespace TimeService

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIME_SERVICE_H
