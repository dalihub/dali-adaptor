#ifndef DALI_DEVEL_ATSPI_ACCESSIBILITY_H
#define DALI_DEVEL_ATSPI_ACCESSIBILITY_H
/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
DALI_ADAPTOR_API void Say(const std::string& text, bool discardable, std::function<void(std::string)> callback);

/**
 * @brief Force accessibility client to pause.
 */
DALI_ADAPTOR_API void Pause();

/**
 * @brief Force accessibility client to resume.
 */
DALI_ADAPTOR_API void Resume();

/**
 * @brief Cancels anything screen-reader is reading / has queued to read
 *
 * @param alsoNonDiscardable whether to cancel non-discardable readings as well
 */
DALI_ADAPTOR_API void StopReading(bool alsoNonDiscardable = false);

/**
 * @brief Suppresses reading of screen-reader
 *
 * @param suppress whether to suppress reading of screen-reader
 * @return true on success, false otherwise
 */
DALI_ADAPTOR_API bool SuppressScreenReader(bool suppress);

/**
 * @brief Returns whether the state of Accessibility is enabled or not.
 *
 * @return True if Accessibility is enabled, false otherwise.
 */
DALI_ADAPTOR_API bool IsEnabled();

/**
 * @brief Returns whether the state of Screen Reader is enabled or not.
 *
 * @return True if Screen Reader is enabled, false otherwise.
 */
DALI_ADAPTOR_API bool IsScreenReaderEnabled();

} //namespace AtspiAccessibility
} //namespace Dali

#endif // DALI_DEVEL_ATSPI_ACCESSIBILITY_H
