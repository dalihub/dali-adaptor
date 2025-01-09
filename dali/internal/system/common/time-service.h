#ifndef DALI_INTERNAL_TIME_SERVICE_H
#define DALI_INTERNAL_TIME_SERVICE_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <stdint.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace TimeService
{
/**
 * @brief Get the monotonic time since the clock's epoch.
 *
 * @param[out]  timeInNanoseconds  The time in nanoseconds since the reference point.
 *
 * @note The maximum value timeInNanoseconds can hold is 0xFFFFFFFFFFFFFFFF which is 1.844674407e+19. Therefore, this can overflow after approximately 584 years.
 *
 * Needs exporting as required by the GlWindow library
 */
void DALI_ADAPTOR_API GetNanoseconds(uint64_t& timeInNanoseconds);

/**
 * @brief Get the monotonic time since the clock's epoch.
 *
 * @return The time in milliseconds since the reference point.
 *
 * @note The maximum value that can be returned is 0xFFFFFFFF which is 4,294,967,295. Therefore, this can overflow after approximately 49 days.
 *
 * Needs exporting as required by the GlWindow library
 */
uint32_t DALI_ADAPTOR_API GetMilliSeconds();

/**
 * @brief Sleeps until the monotonic time specified since the clock's epoch.
 *
 * If the time specified has already passed, then it returns immediately.
 *
 * @param[in]  timeInNanoseconds  The time to sleep until
 *
 * @note The maximum value timeInNanoseconds can hold is 0xFFFFFFFFFFFFFFFF which is 1.844674407e+19. Therefore, this can overflow after approximately 584 years.
 *
 * Needs exporting as required by the GlWindow library
 */
void DALI_ADAPTOR_API SleepUntil(uint64_t timeInNanoseconds);

} // namespace TimeService

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TIME_SERVICE_H
