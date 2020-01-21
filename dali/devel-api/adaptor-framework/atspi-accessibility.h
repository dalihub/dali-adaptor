#ifndef DALI_DEVEL_ATSPI_ACCESSIBILITY_H
#define DALI_DEVEL_ATSPI_ACCESSIBILITY_H
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
#include <functional>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace AtspiAccessibility
{
/**
 * @brief Reads given text by screen reader
 * @param text The text to read
 * @param discardable If TRUE, reading can be discarded by subsequent reading requests,
 * if FALSE the reading must finish before next reading request can be started
 * @param callback the callback function that is called on reading signals emitted
 * during processing of this reading request.
 * Callback can be one of the following signals:
 * ReadingCancelled, ReadingStopped, ReadingSkipped
 */
DALI_ADAPTOR_API void Say( const std::string &text, bool discardable, std::function<void(std::string)> callback );

/**
 * @brief Force accessibility client to pause.
 */
DALI_ADAPTOR_API void Pause();

/**
 * @brief Force accessibility client to resume.
 */
DALI_ADAPTOR_API void Resume();

/**
 * @brief Set ATSPI to be turned On or Off forcibly.
 *
 * @param[in] turnOn true to turn on, false to turn off.
 * @return The status of ATSPI : 0(ATSPI OFF, ScreenReader OFF), 1(ATSPI ON, ScreenReader OFF),
 * 2 (ATSPI OFF, ScreenReader ON), 3(ATSPI ON, ScreenReader ON)
 */
DALI_ADAPTOR_API int SetForcefully( bool turnOn );

/**
 * @brief Get ATSPI status.
 * @return Status of ATSPI : 0(ATSPI OFF, ScreenReader OFF), 1(ATSPI ON, ScreenReader OFF),
 * 2 (ATSPI OFF, ScreenReader ON), 3(ATSPI ON, ScreenReader ON)
 */
DALI_ADAPTOR_API int GetStatus();

} //namespace AtspiAccessibility
} //namespace Dali

#endif // DALI_DEVEL_ATSPI_ACCESSIBILITY_H
